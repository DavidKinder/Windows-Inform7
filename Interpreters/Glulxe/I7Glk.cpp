extern "C" {
#include "Glk.h"
#include "gi_blorb.h"
#include "gi_dispa.h"
}

#include "I7GlkChannel.h"
#include "I7GlkCmd.h"
#include "I7GlkFile.h"
#include "I7GlkStream.h"
#include "I7GlkBlankWindow.h"
#include "I7GlkGfxWindow.h"
#include "I7GlkGridWindow.h"
#include "I7GlkPairWindow.h"
#include "I7GlkTextWindow.h"
#include "GlkUnicode.h"
#include "../../Inform7/InterpreterCommands.h"

#include <windows.h>
#include "GlkTime.h"
#include "WinGlk.h"

#include <deque>
#include <map>
#include <malloc.h>
#include <stdlib.h>

gidispatch_rock_t (*registerObjFn)(void *obj, glui32 objclass) = NULL;
void (*unregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock) = NULL;
gidispatch_rock_t (*registerArrFn)(void *array, glui32 len, char *typecode) = NULL;
void (*unregisterArrFn)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock) = NULL;

extern "C" void fatalError(const char* s);

giblorb_map_t* blorbMap = NULL;

I7GlkStream* currentStream = NULL;
I7GlkWindow* mainWindow = NULL;

int displayWidth = 0;
int displayHeight = 0;
int charWidth = 0;
int charHeight = 0;
std::map<int,std::pair<int,int> > imageSizes;

DWORD timerTick = 0;
DWORD timerLast = 0;

std::deque<event_t> inputEvents;
std::deque<event_t> otherEvents;

std::deque<FrontEndCmd> commands;

FrontEndCmd::FrontEndCmd()
{
  cmd = -1;
  len = 0;
  data = NULL;
}

void FrontEndCmd::free(void)
{
  ::free(data);
  len = 0;
  data = NULL;
}

void FrontEndCmd::read(void)
{
  readReturnData(&cmd,sizeof cmd);
  readReturnData(&len,sizeof len);
  data = malloc(len);
  readReturnData(data,len);
}

bool readCommand(void)
{
  // Look for anything to read
  DWORD available = 0;
  HANDLE in = ::GetStdHandle(STD_INPUT_HANDLE);
  ::PeekNamedPipe(in,NULL,0,NULL,&available,NULL);
  if (available == 0)
    return false;

  FrontEndCmd command;
  command.read();
  commands.push_back(command);
  return true;
}

extern "C" void glk_exit(void)
{
  for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
  {
    I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
    if (wstr != NULL)
      wstr->flush();
  }

  if (blorbMap != NULL)
    giblorb_destroy_map(blorbMap);
  exit(0);
}

extern "C" void glk_set_interrupt_handler(void (*func)(void))
{
}

extern "C" void glk_tick(void)
{
}

extern "C" glui32 glk_gestalt(glui32 sel, glui32 val)
{
  return glk_gestalt_ext(sel,val,NULL,0);
}

extern "C" glui32 glk_gestalt_ext(glui32 sel, glui32 val, glui32 *arr, glui32 arrlen)
{
  switch (sel)
  {
  case gestalt_Version:
    return 0x00000705; // Glk 0.7.5

  case gestalt_LineInput:
    if ((val >= 32 && val <= 126) || (val >= 160 && val <= 0xFFFF))
      return 1;
    return 0;

  case gestalt_CharInput:
    switch (val)
    {
    case keycode_Return:
    case keycode_Escape:
    case keycode_Left:
    case keycode_Right:
    case keycode_Up:
    case keycode_Down:
      return 1;
    default:
      if ((val >= 32 && val <= 126) || (val >= 160 && val <= 0xFFFF))
        return 1;
      break;
    }
    return 0;

  case gestalt_CharOutput:
    if (val == L'\n')
    {
      if (arr && (arrlen > 0))
        arr[0] = 0;
      return gestalt_CharOutput_CannotPrint;
    }
    else if ((val >= 32 && val <= 126) || (val >= 160 && val <= 0x10FFFF))
    {
      if (arr && (arrlen > 0))
        arr[0] = 1;
      return gestalt_CharOutput_ExactPrint;
    }
    else
    {
      if (arr && (arrlen > 0))
        arr[0] = (val <= 0xFF) ? 6 : 12;
      return gestalt_CharOutput_CannotPrint;
    }

  case gestalt_Unicode:
  case gestalt_UnicodeNorm:
    return 1;

  case gestalt_Graphics:
  case gestalt_GraphicsTransparency:
    return 1;

  case gestalt_DrawImage:
    if ((val == wintype_TextBuffer) || (val == wintype_Graphics))
      return 1;
    return 0;

  case gestalt_Sound:
  case gestalt_Sound2:
  case gestalt_SoundVolume:
  case gestalt_SoundNotify:
  case gestalt_SoundMusic:
    return 1;

  case gestalt_Timer:
    return 1;

  case gestalt_MouseInput:
    if ((val == wintype_TextGrid) || (val == wintype_Graphics))
      return 1;
    return 0;

  case gestalt_Hyperlinks:
    return 1;

  case gestalt_HyperlinkInput:
    if ((val == wintype_TextBuffer) || (val == wintype_TextGrid))
      return 1;
    return 0;

  case gestalt_LineInputEcho:
    return 1;

  case gestalt_DateTime:
    return 1;

  case gestalt_ResourceStream:
    return 0;

  case gestalt_GraphicsCharInput:
    return 0;

  case gestalt_GarglkText:
    return 1;
  }
  return 0;
}

extern "C" unsigned char glk_char_to_lower(unsigned char ch)
{
  static const char* pszLoTable1 =
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@abcdefghijklmnopqrstuvwxyz[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~ ";

  static const char* pszLoTable2 =
    "��������������������������������"
    "��������������������������������"
    "��������������������������������";

  unsigned char new_ch = ch;

  if (ch >= 32 && ch <= 126)
    new_ch = pszLoTable1[ch-32];
  if (ch >= 160)
    new_ch = pszLoTable2[ch-160];

  return new_ch;
}

extern "C" unsigned char glk_char_to_upper(unsigned char ch)
{
  static const char* hiTable1 =
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~ ";

  static const char* hiTable2 =
    "��������������������������������"
    "��������������������������������"
    "��������������������������������";

  unsigned char newCh = ch;
  if (ch >= 32 && ch <= 126)
    newCh = hiTable1[ch-32];
  if (ch >= 160)
    newCh = hiTable2[ch-160];
  return newCh;
}

extern "C" winid_t glk_window_get_root(void)
{
  return (winid_t)mainWindow;
}

extern "C" winid_t glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock)
{
  if ((split == 0) && (mainWindow != NULL))
    fatalError("glk_window_open() was called without a window to split, but a window is already open.");
  if ((split != 0) && (mainWindow == NULL))
    fatalError("glk_window_open() was called with a window to split, but no windows are open.");

  I7GlkWindow* win = NULL;
  switch (wintype)
  {
  case wintype_Blank:
    win = new I7GlkBlankWindow(rock);
    break;
  case wintype_TextBuffer:
    win = new I7GlkTextWindow(rock);
    break;
  case wintype_TextGrid:
    win = new I7GlkGridWindow(rock);
    break;
  case wintype_Graphics:
    win = new I7GlkGfxWindow(rock);
    break;
  }
  if (win == NULL)
    return 0;

  int data[6];
  data[0] = win->getId();
  data[1] = -1;
  data[2] = -1;
  data[3] = method;
  data[4] = size;
  data[5] = wintype;

  if (split == 0)
    mainWindow = win;
  else
  {
    I7GlkPairWindow* parent = ((I7GlkWindow*)split)->getParent();
    I7GlkPairWindow* pair = new I7GlkPairWindow((I7GlkWindow*)split,win,method,size);

    if (parent != NULL)
    {
      parent->replace((I7GlkWindow*)split,pair);
      pair->setParent(parent);
    }
    else
      mainWindow = pair;

    data[1] = ((I7GlkWindow*)split)->getId();
    data[2] = pair->getId();
  }

  sendCommand(Command_CreateWindow,sizeof data,data);

  if (mainWindow != NULL)
    mainWindow->layout(I7Rect(0,0,displayWidth,displayHeight));
  win->getStream()->setStyle(style_Normal);

  if (wintype == wintype_TextBuffer)
  {
    int data[4];
    data[0] = win->getId();
    int colour = win->getStyle(style_Normal).m_backColour;
    data[1] = (colour & 0x00FF0000) >> 16;
    data[2] = (colour & 0x0000FF00) >> 8;
    data[3] = (colour & 0x000000FF);
    sendCommand(Command_BackColour,sizeof data,data);
  }

  return (winid_t)win;
}

extern "C" void glk_window_close(winid_t win, stream_result_t *result)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_close() was called for an invalid window.");

  ((I7GlkWindow*)win)->getStreamResults(result);
  ((I7GlkWindow*)win)->closeWindow();

  if (mainWindow != NULL)
    mainWindow->layout(I7Rect(0,0,displayWidth,displayHeight));
}

extern "C" void glk_window_get_size(winid_t win, glui32 *widthptr, glui32 *heightptr)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_size() was called for an invalid window.");

  ((I7GlkWindow*)win)->getSize(widthptr,heightptr);
}

extern "C" void glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_set_arrangement() was called for an invalid window.");
  if ((keywin != 0) && (glkWindows.find((I7GlkWindow*)keywin) == glkWindows.end()))
    fatalError("glk_window_set_arrangement() was called for an invalid key window.");

  I7GlkPairWindow* pwin = dynamic_cast<I7GlkPairWindow*>((I7GlkWindow*)win);
  if (pwin == NULL)
    fatalError("glk_window_set_arrangement() was called for a window that is not a pair window.");

  pwin->setArrangement(method,size,(I7GlkWindow*)keywin);
  if (mainWindow != NULL)
    mainWindow->layout(I7Rect(0,0,displayWidth,displayHeight));
}

extern "C" void glk_window_get_arrangement(winid_t win, glui32 *methodptr, glui32 *sizeptr, winid_t *keywinptr)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_arrangement() was called for an invalid window.");

  I7GlkPairWindow* pwin = dynamic_cast<I7GlkPairWindow*>((I7GlkWindow*)win);
  if (pwin == NULL)
    fatalError("glk_window_get_arrangement() was called for a window that is not a pair window.");

  pwin->getArrangement(methodptr,sizeptr,keywinptr);
}

extern "C" winid_t glk_window_iterate(winid_t win, glui32 *rockptr)
{
  winid_t last = 0;
  std::set<I7GlkWindow*>::iterator it;
  for (it = glkWindows.begin(); it != glkWindows.end(); ++it)
  {
    if (last == win)
    {
      if (rockptr != NULL)
        *rockptr = (*it)->getRock();
      return (winid_t)*it;
    }
    last = (winid_t)*it;
  }
  return 0;
}

extern "C" glui32 glk_window_get_rock(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_rock() was called for an invalid window.");

  return ((I7GlkWindow*)win)->getRock();
}

extern "C" glui32 glk_window_get_type(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_type() was called for an invalid window.");

  I7GlkWindow* window = (I7GlkWindow*)win;

  if (dynamic_cast<I7GlkPairWindow*>(window) != NULL)
    return wintype_Pair;
  if (dynamic_cast<I7GlkTextWindow*>(window) != NULL)
    return wintype_TextBuffer;
  if (dynamic_cast<I7GlkGridWindow*>(window) != NULL)
    return wintype_TextGrid;
  if (dynamic_cast<I7GlkGfxWindow*>(window) != NULL)
    return wintype_Graphics;
  return wintype_Blank;
}

extern "C" winid_t glk_window_get_parent(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_parent() was called for an invalid window.");

  return (winid_t)((I7GlkWindow*)win)->getParent();
}

extern "C" winid_t glk_window_get_sibling(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_sibling() was called for an invalid window.");

  I7GlkPairWindow* parent = ((I7GlkWindow*)win)->getParent();
  if (parent != NULL)
    return (winid_t)parent->getSibling((I7GlkWindow*)win);
  return 0;
}

extern "C" void glk_window_clear(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_clear() was called for an invalid window.");

  ((I7GlkWindow*)win)->clear();
}

extern "C" void glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_move_cursor() was called for an invalid window.");

  return ((I7GlkWindow*)win)->moveCursor(xpos,ypos);
}

extern "C" strid_t glk_window_get_stream(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_stream() was called for an invalid window.");

  return (strid_t)((I7GlkWindow*)win)->getStream();
}

extern "C" void glk_window_set_echo_stream(winid_t win, strid_t str)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_set_echo_stream() was called for an invalid window.");

  ((I7GlkWindow*)win)->setEchoStream((I7GlkStream*)str);
}

extern "C" strid_t glk_window_get_echo_stream(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_get_echo_stream() was called for an invalid window.");

  return (strid_t)((I7GlkWindow*)win)->getEchoStream();
}

extern "C" void glk_set_window(winid_t win)
{
  if (win == 0)
    glk_stream_set_current(0);
  else
  {
    if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
      fatalError("glk_set_window() was called for an invalid window.");
    glk_stream_set_current(glk_window_get_stream(win));
  }
}

extern "C" strid_t glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock)
{
  if (glkFiles.find((I7GlkFile*)fileref) == glkFiles.end())
    fatalError("glk_stream_open_file() was called for an invalid file reference.");

  I7GlkFileStream* str = new I7GlkFileStream(rock);
  if (str->open((I7GlkFile*)fileref,fmode) == false)
  {
    delete str;
    str = NULL;
  }
  return (strid_t)str;
}

extern "C" strid_t glk_stream_open_memory(char *buf, glui32 buflen, glui32 fmode, glui32 rock)
{
  return (strid_t)new I7GlkMemoryStream(buf,buflen,rock);
}

extern "C" void glk_stream_close(strid_t str, stream_result_t *result)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_stream_close() was called for an invalid stream.");

  I7GlkStream* stream = (I7GlkStream*)str;
  if (dynamic_cast<I7GlkWinStream*>(stream) != NULL)
    fatalError("glk_stream_close() was called for a window stream.");

  stream->getResults(result);
  delete stream;
}

extern "C" strid_t glk_stream_iterate(strid_t str, glui32 *rockptr)
{
  strid_t last = 0;
  std::set<I7GlkStream*>::iterator it;
  for (it = glkStreams.begin(); it != glkStreams.end(); ++it)
  {
    if (last == str)
    {
      if (rockptr != NULL)
        *rockptr = (*it)->getRock();
      return (strid_t)*it;
    }
    last = (strid_t)*it;
  }
  return 0;
}

extern "C" glui32 glk_stream_get_rock(strid_t str)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_stream_get_rock() was called for an invalid stream.");

  return ((I7GlkStream*)str)->getRock();
}

extern "C" void glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_stream_set_position() was called for an invalid stream.");

  ((I7GlkStream*)str)->setPosition(pos,seekmode);
}

extern "C" glui32 glk_stream_get_position(strid_t str)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_stream_get_position() was called for an invalid stream.");

  return ((I7GlkStream*)str)->getPosition();
}

extern "C" void glk_stream_set_current(strid_t str)
{
  currentStream = (I7GlkStream*)str;
}

extern "C" strid_t glk_stream_get_current(void)
{
  return (strid_t)currentStream;
}

extern "C" void glk_put_char(unsigned char ch)
{
  glk_put_char_stream(glk_stream_get_current(),ch);
}

extern "C" void glk_put_char_stream(strid_t str, unsigned char ch)
{
  if (str == 0)
  {
    static std::string zeroOut;

    if (ch == '\n')
    {
      if (zeroOut.size() > 0)
        sendCommand(Command_NullOutput,zeroOut.size(),&zeroOut[0]);
      zeroOut.resize(0);
    }
    else
      zeroOut += ch;

    return; // Otherwise an early Run-time Problem halts the game
  }

  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_put_char_stream() was called for an invalid stream.");

  ((I7GlkStream*)str)->putStr((char*)&ch,1,true);
}

extern "C" void glk_put_string(char *s)
{
  glk_put_string_stream(glk_stream_get_current(),s);
}

extern "C" void glk_put_string_stream(strid_t str, char *s)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_put_string_stream() was called for an invalid stream.");

  ((I7GlkStream*)str)->putStr(s,strlen(s),true);
}

extern "C" void glk_put_buffer(char *buf, glui32 len)
{
  glk_put_buffer_stream(glk_stream_get_current(),buf,len);
}

extern "C" void glk_put_buffer_stream(strid_t str, char *buf, glui32 len)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_put_buffer_stream() was called for an invalid stream.");

  ((I7GlkStream*)str)->putStr(buf,len,true);
}

extern "C" void glk_set_style(glui32 styl)
{
  glk_set_style_stream(glk_stream_get_current(),styl);
}

extern "C" void glk_set_style_stream(strid_t str, glui32 styl)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_set_style_stream() was called for an invalid stream.");

  I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
  if (wstr != NULL)
    wstr->setStyle(styl);
}

extern "C" glsi32 glk_get_char_stream(strid_t str)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_get_char_stream() was called for an invalid stream.");

  glsi32 c = ((I7GlkStream*)str)->getChar();
  if ((c == -1) || (c >= 0 && c <= 255))
    return c;
  return '?';
}

extern "C" glui32 glk_get_line_stream(strid_t str, char *buf, glui32 len)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_get_line_stream() was called for an invalid stream.");

  return ((I7GlkStream*)str)->getLine(buf,len);
}

extern "C" glui32 glk_get_buffer_stream(strid_t str, char *buf, glui32 len)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_get_buffer_stream() was called for an invalid stream.");

  return ((I7GlkStream*)str)->getBuffer(buf,len);
}

extern "C" void glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val)
{
  if (wintype == wintype_AllTypes)
  {
    glk_stylehint_set(wintype_TextBuffer,styl,hint,val);
    glk_stylehint_set(wintype_TextGrid,styl,hint,val);
    return;
  }

  I7GlkStyle* style = NULL;
  if ((styl >= 0) && (styl < style_NUMSTYLES))
  {
    switch (wintype)
    {
    case wintype_TextBuffer:
      style = I7GlkTextWindow::defaultStyles[1]+styl;
      break;
    case wintype_TextGrid:
      style = I7GlkGridWindow::defaultStyles[1]+styl;
      break;
    }
  }

  if (style != NULL)
  {
    switch (hint)
    {
    case stylehint_Justification:
      style->m_justify = val;
      break;
    case stylehint_Size:
      style->m_size = val;
      break;
    case stylehint_Weight:
      style->m_weight = val;
      break;
    case stylehint_Oblique:
      style->m_italic = (val != 0);
      break;
    case stylehint_Proportional:
      style->m_proportional = (val != 0);
      break;
    case stylehint_TextColor:
      style->m_textColour = val;
      break;
    case stylehint_BackColor:
      style->m_backColour = val;
      break;
    case stylehint_ReverseColor:
      style->m_reverse = (val != 0);
      break;
    }
  }
}

extern "C" void glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint)
{
  if (wintype == wintype_AllTypes)
  {
    glk_stylehint_clear(wintype_TextBuffer,styl,hint);
    glk_stylehint_clear(wintype_TextGrid,styl,hint);
    return;
  }

  I7GlkStyle* from = NULL;
  I7GlkStyle* style = NULL;
  if ((styl >= 0) && (styl < style_NUMSTYLES))
  {
    switch (wintype)
    {
    case wintype_TextBuffer:
      from = I7GlkTextWindow::defaultStyles[0]+styl;
      style = I7GlkTextWindow::defaultStyles[1]+styl;
      break;
    case wintype_TextGrid:
      from = I7GlkGridWindow::defaultStyles[0]+styl;
      style = I7GlkGridWindow::defaultStyles[1]+styl;
      break;
    }
  }

  if (style != NULL)
  {
    switch (hint)
    {
    case stylehint_Justification:
      style->m_justify = from->m_justify;
      break;
    case stylehint_Size:
      style->m_size = from->m_size;
      break;
    case stylehint_Weight:
      style->m_weight = from->m_weight;
      break;
    case stylehint_Oblique:
      style->m_italic = from->m_italic;
      break;
    case stylehint_Proportional:
      style->m_proportional = from->m_proportional;
      break;
    case stylehint_TextColor:
      style->m_textColour = from->m_textColour;
      break;
    case stylehint_BackColor:
      style->m_backColour = from->m_backColour;
      break;
    case stylehint_ReverseColor:
      style->m_reverse = from->m_reverse;
      break;
    }
  }
}

extern "C" glui32 glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_style_distinguish() was called for an invalid window.");

  I7GlkStyle style1 = ((I7GlkWindow*)win)->getStyle(styl1);
  I7GlkStyle style2 = ((I7GlkWindow*)win)->getStyle(styl2);
  return (style1 == style2) ? 0 : 1;
}

extern "C" glui32 glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32 *result)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_style_measure() was called for an invalid window.");

  I7GlkStyle style = ((I7GlkWindow*)win)->getStyle(styl);
  switch (hint)
  {
  case stylehint_Indentation:
    if (result != NULL)
      *result = 0;
    return 1;
  case stylehint_ParaIndentation:
    if (result != NULL)
      *result = 0;
    return 1;
  case stylehint_Justification:
    if (result != NULL)
      *result = style.m_justify;
    return 1;
  case stylehint_Size:
    if (result != NULL)
      *result = style.m_size;
    return 1;
  case stylehint_Weight:
    if (result != NULL)
      *result = style.m_weight;
    return 1;
  case stylehint_Oblique:
    if (result != NULL)
      *result = style.m_italic ? 1 : 0;
    return 1;
  case stylehint_Proportional:
    if (result != NULL)
      *result = style.m_proportional ? 1 : 0;
    return 1;
  case stylehint_TextColor:
    if (result != NULL)
      *result = style.m_textColour;
    return 1;
  case stylehint_BackColor:
    if (result != NULL)
      *result = style.m_backColour;
    return 1;
  case stylehint_ReverseColor:
    if (result != NULL)
      *result = style.m_reverse ? 1 : 0;
    return 1;
  }
  return 0;
}

extern "C" frefid_t glk_fileref_create_temp(glui32 usage, glui32 rock)
{
  return 0;
}

extern "C" frefid_t glk_fileref_create_by_name(glui32 usage, char *name, glui32 rock)
{
  return (frefid_t)(new I7GlkFile(usage,rock,name,true));
}

extern "C" frefid_t glk_fileref_create_by_prompt(glui32 usage, glui32 fmode, glui32 rock)
{
  std::string name = I7GlkFile::fileDialog(usage,fmode);
  if (!name.empty())
    return (frefid_t)(new I7GlkFile(usage,rock,name.c_str(),false));
  return 0;
}

extern "C" frefid_t glk_fileref_create_from_fileref(glui32 usage, frefid_t fref, glui32 rock)
{
  if (glkFiles.find((I7GlkFile*)fref) == glkFiles.end())
    fatalError("glk_fileref_create_from_fileref() was called for an invalid file reference.");

  return (frefid_t)(new I7GlkFile(usage,rock,(I7GlkFile*)fref));
}

extern "C" void glk_fileref_destroy(frefid_t fref)
{
  if (glkFiles.find((I7GlkFile*)fref) == glkFiles.end())
    fatalError("glk_fileref_destroy() was called for an invalid file reference.");

  delete (I7GlkFile*)fref;
}

extern "C" frefid_t glk_fileref_iterate(frefid_t fref, glui32 *rockptr)
{
  frefid_t last = 0;
  std::set<I7GlkFile*>::iterator it;
  for (it = glkFiles.begin(); it != glkFiles.end(); ++it)
  {
    if (last == fref)
    {
      if (rockptr != NULL)
        *rockptr = (*it)->getRock();
      return (frefid_t)*it;
    }
    last = (frefid_t)*it;
  }
  return 0;
}

extern "C" glui32 glk_fileref_get_rock(frefid_t fref)
{
  if (glkFiles.find((I7GlkFile*)fref) == glkFiles.end())
    return 0;

  return ((I7GlkFile*)fref)->getRock();
}

extern "C" void glk_fileref_delete_file(frefid_t fref)
{
  if (glkFiles.find((I7GlkFile*)fref) == glkFiles.end())
    fatalError("glk_fileref_delete_file() was called for an invalid file reference.");

  ((I7GlkFile*)fref)->deleteFile();
}

extern "C" glui32 glk_fileref_does_file_exist(frefid_t fref)
{
  if (glkFiles.find((I7GlkFile*)fref) == glkFiles.end())
    fatalError("glk_fileref_does_file_exist() was called for an invalid file reference.");

  return ((I7GlkFile*)fref)->exists() ? 1 : 0;
}

extern "C" void glk_select(event_t *event)
{
  for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
  {
    I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
    if (wstr != NULL)
      wstr->flush();
  }

  // Wait for an event to be added to one of the event queues
  while (inputEvents.empty() && otherEvents.empty())
  {
    // Read in any new commands
    while (readCommand());

    // Process the queue of commands
    while (!commands.empty())
    {
      FrontEndCmd command = commands.front();
      commands.pop_front();

      switch (command.cmd)
      {
      case Return_ReadLine:
        if (command.len >= sizeof(int))
        {
          int wndId = ((int*)command.data)[0];
          for (winid_t win = glk_window_iterate(0,NULL); win != 0; win = glk_window_iterate(win,NULL))
          {
            if (((I7GlkWindow*)win)->getId() == wndId)
            {
              void* lineData = ((int*)command.data)+1;
              int lineLen = (command.len - sizeof(int)) / sizeof(wchar_t);

              event_t lineEvent = { 0 };
              ((I7GlkWindow*)win)->endLine(&lineEvent,false,(wchar_t*)lineData,lineLen);
              if (lineEvent.type == evtype_LineInput)
                inputEvents.push_back(lineEvent);
              break;
            }
          }
        }
        break;

      case Return_ReadKey:
        if (command.len >= sizeof(int))
        {
          int key = ((int*)command.data)[0];
          for (winid_t win = glk_window_iterate(0,NULL); win != 0; win = glk_window_iterate(win,NULL))
          {
            event_t charEvent = { 0 };
            ((I7GlkWindow*)win)->endKey(&charEvent,false,key);
            if (charEvent.type == evtype_CharInput)
            {
              inputEvents.push_back(charEvent);
              break;
            }
          }
        }
        break;

      case Return_Size:
        if (command.len == 4*sizeof(int))
        {
          int* size = (int*)command.data;

          displayWidth = size[0];
          displayHeight = size[1];
          charWidth = size[2];
          charHeight = size[3];

          if (mainWindow != NULL)
          {
            mainWindow->layout(I7Rect(0,0,displayWidth,displayHeight));

            bool addEvent = true;
            for (std::deque<event_t>::const_iterator it = otherEvents.begin(); it != otherEvents.end(); ++it)
            {
              if (it->type == evtype_Arrange)
                addEvent = false;
            }

            if (addEvent)
            {
              event_t arrangeEvent;
              arrangeEvent.type = evtype_Arrange;
              arrangeEvent.win = 0;
              arrangeEvent.val1 = 0;
              arrangeEvent.val2 = 0;
              otherEvents.push_back(arrangeEvent);
            }
          }
        }
        break;

      case Return_SoundOver:
        if (command.len == sizeof(int))
        {
          int notify = ((int*)command.data)[0];
          for (schanid_t chan = glk_schannel_iterate(0,NULL); chan != NULL; chan = glk_schannel_iterate(chan,NULL))
          {
            if (((I7GlkChannel*)chan)->getId() == notify)
            {
              event_t notifyEvent = { 0 };
              ((I7GlkChannel*)chan)->getSoundNotify(notifyEvent);
              if (notifyEvent.type == evtype_SoundNotify)
                otherEvents.push_back(notifyEvent);
              break;
            }
          }
        }
        break;

      case Return_VolumeOver:
        if (command.len == sizeof(int))
        {
          int notify = ((int*)command.data)[0];
          for (schanid_t chan = glk_schannel_iterate(0,NULL); chan != NULL; chan = glk_schannel_iterate(chan,NULL))
          {
            if (((I7GlkChannel*)chan)->getId() == notify)
            {
              event_t notifyEvent = { 0 };
              ((I7GlkChannel*)chan)->getVolumeNotify(notifyEvent);
              if (notifyEvent.type == evtype_VolumeNotify)
                otherEvents.push_back(notifyEvent);
              break;
            }
          }
        }
        break;

      case Return_Mouse:
        if (command.len == 3*sizeof(int))
        {
          int* mouse = (int*)command.data;
          for (winid_t win = glk_window_iterate(0,NULL); win != 0; win = glk_window_iterate(win,NULL))
          {
            if (((I7GlkWindow*)win)->getId() == mouse[0])
            {
              event_t mouseEvent = { 0 };
              ((I7GlkWindow*)win)->endMouse(&mouseEvent,mouse[1],mouse[2]);
              if (mouseEvent.type == evtype_MouseInput)
                inputEvents.push_back(mouseEvent);
              break;
            }
          }
        }
        break;

      case Return_Link:
        if (command.len == 2*sizeof(int))
        {
          int* link = (int*)command.data;
          for (winid_t win = glk_window_iterate(0,NULL); win != 0; win = glk_window_iterate(win,NULL))
          {
            if (((I7GlkWindow*)win)->getId() == link[0])
            {
              event_t linkEvent = { 0 };
              ((I7GlkWindow*)win)->endLink(&linkEvent,link[1]);
              if (linkEvent.type == evtype_Hyperlink)
                inputEvents.push_back(linkEvent);
              break;
            }
          }
        }
        break;
      }

      // Discard the memory allocated for the command
      command.free();
    }

    // Check if a timer event should occur
    if ((timerTick > 0) && (::GetTickCount() > timerLast+timerTick))
    {
      timerLast = ::GetTickCount();

      event_t timerEvent;
      timerEvent.type = evtype_Timer;
      timerEvent.win = 0;
      timerEvent.val1 = 0;
      timerEvent.val2 = 0;
      otherEvents.push_back(timerEvent);
    }

    if (inputEvents.empty() && otherEvents.empty())
      ::Sleep(50);
  }

  // Get an event from one of the event queues
  event->type = evtype_None;
  if (inputEvents.empty() == false)
  {
    *event = inputEvents.front();
    inputEvents.pop_front();
  }
  else if (otherEvents.empty() == false)
  {
    *event = otherEvents.front();
    otherEvents.pop_front();
  }
}

extern "C" void glk_select_poll(event_t *event)
{
  memset(event,0,sizeof(event_t));
  event->type = evtype_None;
  if (otherEvents.empty() == false)
  {
    *event = otherEvents.front();
    otherEvents.pop_front();
  }
}

extern "C" void glk_request_timer_events(glui32 millisecs)
{
  timerTick = millisecs;
  timerLast = ::GetTickCount();
}

extern "C" void glk_request_line_event(winid_t win, char *buf, glui32 maxlen, glui32 initlen)
{
  for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
  {
    I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
    if (wstr != NULL)
      wstr->flush();
  }

  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_request_line_event() was called for an invalid window.");

  ((I7GlkWindow*)win)->requestLine(buf,maxlen,initlen);
}

extern "C" void glk_request_char_event(winid_t win)
{
  for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
  {
    I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
    if (wstr != NULL)
      wstr->flush();
  }

  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_request_char_event() was called for an invalid window.");

  ((I7GlkWindow*)win)->requestKey(I7GlkWindow::ReadKeyAscii);
}

extern "C" void glk_request_mouse_event(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_request_mouse_event() was called for an invalid window.");

  ((I7GlkWindow*)win)->requestMouse();
}

extern "C" void glk_cancel_line_event(winid_t win, event_t *event)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_cancel_line_event() was called for an invalid window.");

  ((I7GlkWindow*)win)->endLine(event,true,NULL,0);
}

extern "C" void glk_cancel_char_event(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_cancel_char_event() was called for an invalid window.");

  ((I7GlkWindow*)win)->endKey(NULL,true,0);
}

extern "C" void glk_cancel_mouse_event(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_cancel_mouse_event() was called for an invalid window.");

  ((I7GlkWindow*)win)->endMouse(NULL,0,0);
}

extern "C" glui32 glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_image_draw() was called for an invalid window.");

  return ((I7GlkWindow*)win)->draw(image,val1,val2,(glui32)-1,(glui32)-1);
}

extern "C" glui32 glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_image_draw_scaled() was called for an invalid window.");

  return ((I7GlkWindow*)win)->draw(image,val1,val2,width,height);
}

extern "C" glui32 glk_image_get_info(glui32 image, glui32 *width, glui32 *height)
{
  std::map<int,std::pair<int,int> >::const_iterator it = imageSizes.find(image);
  if (it == imageSizes.end())
    return 0;

  if (width != NULL)
    *width = it->second.first;
  if (height != NULL)
    *height = it->second.second;
  return 1;
}

extern "C" void glk_window_flow_break(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_flow_break() was called for an invalid window.");
}

extern "C" void glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_erase_rect() was called for an invalid window.");

  I7GlkGfxWindow* gwin = dynamic_cast<I7GlkGfxWindow*>((I7GlkWindow*)win);
  if (gwin != NULL)
  {
    I7Rect r(left,top,left+width,top+height);
    gwin->fillRect(r,gwin->getBackColour());
  }
}

extern "C" void glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_fill_rect() was called for an invalid window.");

  I7GlkGfxWindow* gwin = dynamic_cast<I7GlkGfxWindow*>((I7GlkWindow*)win);
  if (gwin != NULL)
  {
    I7Rect r(left,top,left+width,top+height);
    gwin->fillRect(r,color);
  }
}

extern "C" void glk_window_set_background_color(winid_t win, glui32 color)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_window_set_background_color() was called for an invalid window.");

  I7GlkGfxWindow* gwin = dynamic_cast<I7GlkGfxWindow*>((I7GlkWindow*)win);
  if (gwin != NULL)
    gwin->setBackColour(color);
}

extern "C" schanid_t glk_schannel_create(glui32 rock)
{
  return glk_schannel_create_ext(rock,0x10000);
}

extern "C" schanid_t glk_schannel_create_ext(glui32 rock, glui32 volume)
{
  I7GlkChannel* chan = new I7GlkChannel(rock);
  chan->setVolume(volume,0,0);
  chan->pause(false);
  return (schanid_t)chan;
}

extern "C" void glk_schannel_destroy(schanid_t chan)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_destroy() was called for an invalid sound channel.");

  ((I7GlkChannel*)chan)->stop(true);
  delete (I7GlkChannel*)chan;
}

extern "C" schanid_t glk_schannel_iterate(schanid_t chan, glui32 *rockptr)
{
  schanid_t last = 0;
  std::set<I7GlkChannel*>::iterator it;
  for (it = glkChannels.begin(); it != glkChannels.end(); ++it)
  {
    if (last == chan)
    {
      if (rockptr != NULL)
        *rockptr = (*it)->getRock();
      return (schanid_t)*it;
    }
    last = (schanid_t)*it;
  }
  return 0;

}

extern "C" glui32 glk_schannel_get_rock(schanid_t chan)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_get_rock() was called for an invalid sound channel.");

  return ((I7GlkChannel*)chan)->getRock();
}

extern "C" glui32 glk_schannel_play(schanid_t chan, glui32 snd)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_play() was called for an invalid sound channel.");

  return glk_schannel_play_ext(chan,snd,1,0);
}

extern "C" glui32 glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_play_ext() was called for an invalid sound channel.");

  if (repeats == 0)
  {
    ((I7GlkChannel*)chan)->stop(false);
    return 1;
  }

  return ((I7GlkChannel*)chan)->play(snd,repeats,notify);
}

extern "C" glui32 glk_schannel_play_multi(schanid_t *chanarray, glui32 chancount, glui32 *sndarray, glui32 soundcount, glui32 notify)
{
  if (chancount != soundcount)
    fatalError("glk_schannel_play_multi() was called with channel and sound arrays of different lengths.");

  int* data = (int*)alloca(3*sizeof(int)*chancount);
  glui32 playing = 0;
  for (glui32 i = 0; i < chancount; i++)
  {
    if (glkChannels.find((I7GlkChannel*)chanarray[i]) == glkChannels.end())
      fatalError("glk_schannel_play_multi() was called for an invalid sound channel.");

    if (((I7GlkChannel*)chanarray[i])->multiPlay(sndarray[i],1,notify,data+(3*playing)))
      playing++;
  }

  if (playing > 0)
    sendCommand(Command_PlaySounds,3*sizeof(int)*playing,data);
  return playing;
}

extern "C" void glk_schannel_stop(schanid_t chan)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_stop() was called for an invalid sound channel.");

  ((I7GlkChannel*)chan)->stop(false);
}

extern "C" void glk_schannel_pause(schanid_t chan)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_pause() was called for an invalid sound channel.");

  ((I7GlkChannel*)chan)->pause(true);
}

extern "C" void glk_schannel_unpause(schanid_t chan)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_unpause() was called for an invalid sound channel.");

  ((I7GlkChannel*)chan)->pause(false);
}

extern "C" void glk_schannel_set_volume(schanid_t chan, glui32 vol)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_set_volume() was called for an invalid sound channel.");

  glk_schannel_set_volume_ext(chan,vol,0,0);
}

extern "C" void glk_schannel_set_volume_ext(schanid_t chan, glui32 vol, glui32 duration, glui32 notify)
{
  if (glkChannels.find((I7GlkChannel*)chan) == glkChannels.end())
    fatalError("glk_schannel_set_volume_ext() was called for an invalid sound channel.");

  ((I7GlkChannel*)chan)->setVolume(vol,duration,notify);
}

extern "C" void glk_sound_load_hint(glui32 snd, glui32 flag)
{
}

extern "C" void glk_set_hyperlink(glui32 linkval)
{
  glk_set_hyperlink_stream(glk_stream_get_current(),linkval);
}

extern "C" void glk_set_hyperlink_stream(strid_t str, glui32 linkval)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_set_hyperlink_stream() was called for an invalid stream.");

  return ((I7GlkStream*)str)->setHyperlink(linkval);
}

extern "C" void glk_request_hyperlink_event(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_request_hyperlink_event() was called for an invalid stream.");

  ((I7GlkWindow*)win)->requestLink();
}

extern "C" void glk_cancel_hyperlink_event(winid_t win)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_cancel_hyperlink_event() was called for an invalid stream.");

  ((I7GlkWindow*)win)->endLink(NULL,0);
}

extern "C" glui32 glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  return buffer_change_case(buf,len,numchars,CASE_LOWER,COND_ALL,TRUE);
}

extern "C" glui32 glk_buffer_to_upper_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  return buffer_change_case(buf,len,numchars,CASE_UPPER,COND_ALL,TRUE);
}

extern "C" glui32 glk_buffer_to_title_case_uni(glui32 *buf, glui32 len, glui32 numchars, glui32 lowerrest)
{
  return buffer_change_case(buf,len,numchars,CASE_TITLE,COND_LINESTART,lowerrest);
}

extern "C" void glk_put_char_uni(glui32 ch)
{
  glk_put_char_stream_uni(glk_stream_get_current(),ch);
}

extern "C" void glk_put_string_uni(glui32 *s)
{
  glk_put_string_stream_uni(glk_stream_get_current(),s);
}

extern "C" void glk_put_buffer_uni(glui32 *buf, glui32 len)
{
  glk_put_buffer_stream_uni(glk_stream_get_current(),buf,len);
}

extern "C" void glk_put_char_stream_uni(strid_t str, glui32 ch)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_put_char_stream_uni() was called for an invalid stream.");

  ((I7GlkStream*)str)->putStr((glui32*)&ch,1,true);
}

extern "C" void glk_put_string_stream_uni(strid_t str, glui32 *s)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_put_string_stream_uni() was called for an invalid stream.");

  int len = 0;
  while (s[len] != 0)
    len++;
  ((I7GlkStream*)str)->putStr(s,len,true);
}

extern "C" void glk_put_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_put_buffer_stream_uni() was called for an invalid stream.");

  ((I7GlkStream*)str)->putStr(buf,len,true);
}

extern "C" glsi32 glk_get_char_stream_uni(strid_t str)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_get_char_stream_uni() was called for an invalid stream.");

  return ((I7GlkStream*)str)->getChar();
}

extern "C" glui32 glk_get_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_get_buffer_stream_uni() was called for an invalid stream.");

  return ((I7GlkStream*)str)->getBuffer(buf,len);
}

extern "C" glui32 glk_get_line_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("glk_get_line_stream_uni() was called for an invalid stream.");

  return ((I7GlkStream*)str)->getLine(buf,len);
}

extern "C" strid_t glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock)
{
  if (glkFiles.find((I7GlkFile*)fileref) == glkFiles.end())
    fatalError("glk_stream_open_file_uni() was called for an invalid file reference.");

  I7GlkUniFileStream* str = new I7GlkUniFileStream(rock);
  if (str->open((I7GlkFile*)fileref,fmode) == false)
  {
    delete str;
    str = NULL;
  }
  return (strid_t)str;
}

extern "C" strid_t glk_stream_open_memory_uni(glui32 *buf, glui32 buflen, glui32 fmode, glui32 rock)
{
  return (strid_t)new I7GlkUniMemoryStream(buf,buflen,rock);
}

extern "C" void glk_request_char_event_uni(winid_t win)
{
  for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
  {
    I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
    if (wstr != NULL)
      wstr->flush();
  }

  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_request_char_event_uni() was called for an invalid window.");

  ((I7GlkWindow*)win)->requestKey(I7GlkWindow::ReadKeyUnicode);
}

extern "C" void glk_request_line_event_uni(winid_t win, glui32 *buf, glui32 maxlen, glui32 initlen)
{
  for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
  {
    I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
    if (wstr != NULL)
      wstr->flush();
  }

  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_request_line_event_uni() was called for an invalid window.");

  ((I7GlkWindow*)win)->requestLine(buf,maxlen,initlen);
}

extern "C" void glk_set_echo_line_event(winid_t win, glui32 val)
{
  if (glkWindows.find((I7GlkWindow*)win) == glkWindows.end())
    fatalError("glk_set_echo_line_event() was called for an invalid window.");

  ((I7GlkWindow*)win)->setNextEchoInput(val != 0);
}

extern "C" void glk_set_terminators_line_event(winid_t win, glui32 *keycodes, glui32 count)
{
}

extern "C" glui32 glk_buffer_canon_decompose_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  glui32 *dest = buffer_canon_decompose(buf,&numchars);
  if (dest == NULL)
    return 0;

  glui32 newlen = numchars;
  if (newlen > len)
    newlen = len;
  if (newlen > 0)
    memcpy(buf,dest,newlen * sizeof(glui32));
  free(dest);

  return numchars;
}

extern "C" glui32 glk_buffer_canon_normalize_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  glui32 *dest = buffer_canon_decompose(buf,&numchars);
  if (dest == NULL)
    return 0;
  numchars = buffer_canon_compose(dest,numchars);

  glui32 newlen = numchars;
  if (newlen > len)
    newlen = len;
  if (newlen)
    memcpy(buf,dest,newlen * sizeof(glui32));
  free(dest);

  return numchars;
}

extern "C" void glk_current_time(glktimeval_t* time)
{
  ToGlkTime(GetNow(),time);
}

extern "C" glsi32 glk_current_simple_time(glui32 factor)
{
  if (factor == 0)
    return 0;
  return ToSimpleTime(GetNow(),factor);
}

extern "C" void glk_time_to_date_utc(glktimeval_t* time, glkdate_t* date)
{
  ToGlkDate(FromGlkTime(time),date);
}

extern "C" void glk_time_to_date_local(glktimeval_t* time, glkdate_t* date)
{
  ToGlkDate(ToLocal(FromGlkTime(time)),date);
}

extern "C" void glk_simple_time_to_date_utc(glsi32 time, glui32 factor, glkdate_t* date)
{
  ToGlkDate(FromSimpleTime(time,factor),date);
}

extern "C" void glk_simple_time_to_date_local(glsi32 time, glui32 factor, glkdate_t* date)
{
  ToGlkDate(ToLocal(FromSimpleTime(time,factor)),date);
}

extern "C" void glk_date_to_time_utc(glkdate_t* date, glktimeval_t* time)
{
  ToGlkTime(FromGlkDate(date),time);
}

extern "C" void glk_date_to_time_local(glkdate_t* date, glktimeval_t* time)
{
  ToGlkTime(ToLocal(FromGlkDate(date)),time);
}

extern "C" glsi32 glk_date_to_simple_time_utc(glkdate_t* date, glui32 factor)
{
  if (factor == 0)
    return 0;
  return ToSimpleTime(FromGlkDate(date),factor);
}

extern "C" glsi32 glk_date_to_simple_time_local(glkdate_t* date, glui32 factor)
{
  if (factor == 0)
    return 0;
  return ToSimpleTime(ToLocal(FromGlkDate(date)),factor);
}

extern "C" strid_t glk_stream_open_resource(glui32 filenum, glui32 rock)
{
  return 0;
}

extern "C" strid_t glk_stream_open_resource_uni(glui32 filenum, glui32 rock)
{
  return 0;
}

extern "C" void gidispatch_set_object_registry(
  gidispatch_rock_t (*reg)(void *obj, glui32 objclass),
  void (*unreg)(void *obj, glui32 objclass, gidispatch_rock_t objrock))
{
  registerObjFn = reg;
  unregisterObjFn = unreg;

  if (registerObjFn != NULL)
  {
    for (winid_t win = glk_window_iterate(0,NULL); win != 0; win = glk_window_iterate(win,NULL))
      ((I7GlkWindow*)win)->setDispRock((*registerObjFn)(win, gidisp_Class_Window));

    for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
      ((I7GlkStream*)str)->setDispRock((*registerObjFn)(str, gidisp_Class_Stream));

    for (frefid_t fref = glk_fileref_iterate(0,NULL); fref != NULL; fref = glk_fileref_iterate(fref,NULL))
      ((I7GlkFile*)fref)->setDispRock((*registerObjFn)(fref, gidisp_Class_Fileref));

    for (schanid_t chan = glk_schannel_iterate(0,NULL); chan != NULL; chan = glk_schannel_iterate(chan,NULL))
      ((I7GlkChannel*)chan)->setDispRock((*registerObjFn)(chan, gidisp_Class_Schannel));
  }
}

extern "C" gidispatch_rock_t gidispatch_get_objrock(void *obj, glui32 objclass)
{
  gidispatch_rock_t rock;
  rock.num = 0;

  switch (objclass)
  {
  case gidisp_Class_Window:
    rock = ((I7GlkWindow*)obj)->getDispRock();
    break;
  case gidisp_Class_Stream:
    rock = ((I7GlkStream*)obj)->getDispRock();
    break;
  case gidisp_Class_Fileref:
    rock = ((I7GlkFile*)obj)->getDispRock();
    break;
  case gidisp_Class_Schannel:
    rock = ((I7GlkChannel*)obj)->getDispRock();
    break;
  }
  return rock;
}

extern "C" void gidispatch_set_retained_registry(
  gidispatch_rock_t (*reg)(void *array, glui32 len, char *typecode), 
  void (*unreg)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock))
{
  registerArrFn = reg;
  unregisterArrFn = unreg;
}

giblorb_err_t giblorb_set_resource_map(strid_t file)
{
  if (blorbMap != NULL)
  {
    giblorb_destroy_map(blorbMap);
    blorbMap = NULL;
  }
  return giblorb_create_map(file,&blorbMap);
}

extern "C" giblorb_map_t* giblorb_get_resource_map(void)
{
  return blorbMap;
}

extern "C" void garglk_set_zcolors(glui32 fg, glui32 bg)
{
  garglk_set_zcolors_stream(glk_stream_get_current(),fg,bg);
}

extern "C" void garglk_set_zcolors_stream(strid_t str, glui32 fg, glui32 bg)
{
  if ((fg == zcolor_Current) && (bg == zcolor_Current))
    return;

  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("garglk_set_zcolors_stream() was called for an invalid stream.");

  I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
  if (wstr != NULL)
    wstr->setColours(fg, bg);
}

extern "C" void garglk_set_reversevideo(glui32 reverse)
{
  garglk_set_reversevideo_stream(glk_stream_get_current(),reverse);
}

extern "C" void garglk_set_reversevideo_stream(strid_t str, glui32 reverse)
{
  if (glkStreams.find((I7GlkStream*)str) == glkStreams.end())
    fatalError("garglk_set_reversevideo_stream() was called for an invalid stream.");

  I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
  if (wstr != NULL)
    wstr->setReverse(reverse);
}
