#include "I7GlkFile.h"
#include "I7GlkStream.h"
#include "I7GlkWindow.h"
#include "../../Inform7/InterpreterCommands.h"

std::set<I7GlkStream*> glkStreams;
extern I7GlkStream* currentStream;

extern gidispatch_rock_t (*registerObjFn)(void *obj, glui32 objclass);
extern void (*unregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock);
extern gidispatch_rock_t (*registerArrFn)(void *array, glui32 len, char *typecode);
extern void (*unregisterArrFn)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock);

void sendCommand(int command, int dataLength, const void* data);
extern "C" void fatalError(const char* s);

I7GlkStream::I7GlkStream(glui32 rock)
{
  m_rock = rock;
  m_read = 0;
  m_write = 0;

  glkStreams.insert(this);

  if (registerObjFn)
    setDispRock((*registerObjFn)(this,gidisp_Class_Stream));
  else
    m_dispRock.num = 0;
}

I7GlkStream::~I7GlkStream()
{
  // If this is an echo stream, clear the echo
  for (winid_t win = glk_window_iterate(0,NULL); win != 0; win = glk_window_iterate(win,NULL))
  {
    if (((I7GlkWindow*)win)->getEchoStream() == this)
      ((I7GlkWindow*)win)->setEchoStream(NULL);
  }

  if (currentStream == this)
    currentStream = NULL;

  if (unregisterObjFn)
    (*unregisterObjFn)(this,gidisp_Class_Stream,getDispRock());

  glkStreams.erase(this);
}

void I7GlkStream::getResults(stream_result_t *result)
{
  if (result != NULL)
  {
    result->readcount = m_read;
    result->writecount = m_write;
  }
}

glui32 I7GlkStream::getLine(char *buf, glui32 len)
{
  bool get = true;
  glui32 i = 0;

  while (get && (i < len-1))
  {
    glsi32 c = getChar();
    if (c == -1)
      get = false;
    else
    {
      if (c >= 0 && c <= 255)
        buf[i++] = (char)c;
      else
        buf[i++] = '?';

      if (c == '\n')
        get = false;
    }
  }

  buf[i++] = '\0';
  return i-1;
}

glui32 I7GlkStream::getBuffer(char *buf, glui32 len)
{
  bool get = true;
  int i = 0;

  while (get && (i < len))
  {
    glsi32 c = getChar();
    if (c == -1)
      get = false;
    else
    {
      if (c >= 0 && c <= 255)
        buf[i++] = (char)c;
      else
        buf[i++] = '?';
    }
  }
  return i;
}

glui32 I7GlkStream::getLine(glui32 *buf, glui32 len)
{
  bool get = true;
  glui32 i = 0;

  while (get && (i < len-1))
  {
    glsi32 c = getChar();
    if (c == -1)
      get = false;
    else
    {
      buf[i++] = (glui32)c;
      if (c == '\n')
        get = false;
    }
  }

  buf[i++] = '\0';
  return i-1;
}

glui32 I7GlkStream::getBuffer(glui32 *buf, glui32 len)
{
  bool get = true;
  int i = 0;

  while (get && (i < len))
  {
    glsi32 c = getChar();
    if (c == -1)
      get = false;
    else
      buf[i++] = (glui32)c;
  }
  return i;
}

I7GlkFileStream::I7GlkFileStream(glui32 rock) : I7GlkStream(rock)
{
  m_file = NULL;
  m_lastOper = 0;
}

I7GlkFileStream::~I7GlkFileStream()
{
  if (m_file != NULL)
    fclose(m_file);
}

void I7GlkFileStream::setPosition(glsi32 pos, glui32 seekmode)
{
  int origin = SEEK_SET;
  if (seekmode == seekmode_Current)
    origin = SEEK_CUR;
  else if (seekmode == seekmode_End)
    origin = SEEK_END;
  fseek(m_file,pos,origin);
  m_lastOper = 0;
}

glui32 I7GlkFileStream::getPosition(void)
{
  return (glui32)ftell(m_file);
}

glsi32 I7GlkFileStream::getChar(void)
{
  setNextOperation(filemode_Read);
  int c = fgetc(m_file);
  if (c == EOF)
    return -1;
  m_read++;
  return c;
}

void I7GlkFileStream::putStr(char* s, glui32 len, bool check)
{
  setNextOperation(filemode_Write);
  m_write += fwrite(s,1,len,m_file);
  fflush(m_file);
}

void I7GlkFileStream::putStr(glui32* s, glui32 len, bool check)
{
  setNextOperation(filemode_Write);
  for (glui32 i = 0; i < len; i++)
  {
    if (fputc((s[i] > 255) ? '?' : s[i],m_file) != EOF)
      m_write++;
  }
  fflush(m_file);
}

bool I7GlkFileStream::open(I7GlkFile* file, glui32 mode)
{
  if ((mode == filemode_ReadWrite) || (mode == filemode_WriteAppend))
  {
    FILE* f = fopen(file->fileName(),"ab");
    if (f != NULL)
      fclose(f);
  }

  std::string modeStr;
  switch (mode)
  {
  case filemode_Write:
    modeStr = "w";
    break;
  case filemode_Read:
    modeStr = "r";
    break;
  case filemode_ReadWrite:
    modeStr = "r+";
    break;
  case filemode_WriteAppend:
    modeStr = "r+";
    break;
  }

  if (file->isText())
    modeStr += "t";
  else
    modeStr += "b";

  m_file = fopen(file->fileName(),modeStr.c_str());
  if ((m_file != NULL) && (mode == filemode_WriteAppend))
      fseek(m_file,0,SEEK_END);

  return (m_file != NULL) ? true : false;
}

void I7GlkFileStream::setNextOperation(glui32 oper)
{
  /* If switching between reading and writing, force an fseek() */
  if ((m_lastOper != 0) && (m_lastOper != oper))
  {
    long pos = ftell(m_file);
    fseek(m_file,pos,SEEK_SET);
  }
  m_lastOper = oper;
}

I7GlkUniFileStream::I7GlkUniFileStream(glui32 rock) : I7GlkStream(rock)
{
  m_text = false;
  m_file = NULL;
  m_lastOper = 0;
}

I7GlkUniFileStream::~I7GlkUniFileStream()
{
  if (m_file != NULL)
    fclose(m_file);
}

void I7GlkUniFileStream::setPosition(glsi32 pos, glui32 seekmode)
{
  int origin = SEEK_SET;
  if (seekmode == seekmode_Current)
    origin = SEEK_CUR;
  else if (seekmode == seekmode_End)
    origin = SEEK_END;

  if (m_text)
    fseek(m_file,pos*2,origin);
  else
    fseek(m_file,pos*4,origin);
  m_lastOper = 0;
}

glui32 I7GlkUniFileStream::getPosition(void)
{
  if (m_text)
    return (glui32)ftell(m_file)/2;
  else
    return (glui32)ftell(m_file)/4;
}

glsi32 I7GlkUniFileStream::getChar(void)
{
  setNextOperation(filemode_Read);
  glsi32 c = -1;
  if (m_text)
  {
    glui32 uc = 0;
    int c1 = fgetc(m_file);
    if (c1 != EOF)
    {
      uc |= (c1 & 0xFF);
      c1 = fgetc(m_file);
      if (c1 != EOF)
      {
        uc |= ((c1 & 0xFF) << 8);
        c = uc;
      }
    }
  }
  else
  {
    glui32 uc = 0;
    for (int i = 0; i < 4; i++)
    {
      uc <<= 8;
      int c1 = fgetc(m_file);
      if (c1 == EOF)
        break;
      uc |= (c1 & 0xFF);
      if (i == 3)
        c = uc;
    }
  }

  if (c != -1)
    m_read++;
  return c;
}

void I7GlkUniFileStream::putStr(char* s, glui32 len, bool check)
{
  setNextOperation(filemode_Write);
  for (glui32 i = 0; i < len; i++)
    addChar((unsigned char)s[i]);
  fflush(m_file);
}

void I7GlkUniFileStream::putStr(glui32* s, glui32 len, bool check)
{
  setNextOperation(filemode_Write);
  for (glui32 i = 0; i < len; i++)
    addChar(s[i]);
  fflush(m_file);
}

bool I7GlkUniFileStream::open(I7GlkFile* file, glui32 mode)
{
  if ((mode == filemode_ReadWrite) || (mode == filemode_WriteAppend))
  {
    FILE* f = fopen(file->fileName(),"ab");
    if (f != NULL)
      fclose(f);
  }

  std::string modeStr;
  switch (mode)
  {
  case filemode_Write:
    modeStr = "w";
    break;
  case filemode_Read:
    modeStr = "r";
    break;
  case filemode_ReadWrite:
    modeStr = "r+";
    break;
  case filemode_WriteAppend:
    modeStr = "r+";
    break;
  }

  if (file->isText())
  {
    m_text = true;
    modeStr += "t";
  }
  else
  {
    m_text = false;
    modeStr += "b";
  }

  m_file = fopen(file->fileName(),modeStr.c_str());
  if ((m_file != NULL) && (mode == filemode_WriteAppend))
      fseek(m_file,0,SEEK_END);

  return (m_file != NULL) ? true : false;
}

void I7GlkUniFileStream::addChar(glui32 c)
{
  if (m_text)
  {
    if (c > 0xFFFF)
      c = L'?';
    fputc(c & 0xFF,m_file);
    fputc((c>>8) & 0xFF,m_file);
  }
  else
  {
    fputc((c>>24) & 0xFF,m_file);
    fputc((c>>16) & 0xFF,m_file);
    fputc((c>>8) & 0xFF,m_file);
    fputc(c & 0xFF,m_file);
  }
  m_write++;
}

void I7GlkUniFileStream::setNextOperation(glui32 oper)
{
  /* If switching between reading and writing, force an fseek() */
  if ((m_lastOper != 0) && (m_lastOper != oper))
  {
    long pos = ftell(m_file);
    fseek(m_file,pos,SEEK_SET);
  }
  m_lastOper = oper;
}

I7GlkMemoryStream::I7GlkMemoryStream(char* buf, glui32 len, glui32 rock) : I7GlkStream(rock)
{
  m_buffer = buf;
  m_length = len;
  m_position = 0;

  if (registerArrFn && m_buffer && (m_length > 0))
    m_arrayRock = ((*registerArrFn)(m_buffer,m_length,(char*)"&+#!Cn"));
  else
    m_arrayRock.num = 0;
}

I7GlkMemoryStream::~I7GlkMemoryStream()
{
  if (unregisterArrFn && m_buffer && (m_length > 0))
    (*unregisterArrFn)(m_buffer,m_length,(char*)"&+#!Cn",m_arrayRock);
}

void I7GlkMemoryStream::setPosition(glsi32 pos, glui32 mode)
{
  if (mode == seekmode_Current)
    m_position += pos;
  else if (mode == seekmode_End)
    m_position = m_length + pos;
  else
    m_position = pos;

  if (m_position < 0)
    m_position = 0;
  if (m_position > m_length)
    m_position = m_length;
}

glui32 I7GlkMemoryStream::getPosition(void)
{
  return m_position;
}

glsi32 I7GlkMemoryStream::getChar(void)
{
  if (m_buffer == NULL)
    return -1;

  if (m_position < m_length)
  {
    m_read++;
    return (unsigned char)m_buffer[m_position++];
  }
  return -1;
}

void I7GlkMemoryStream::putStr(char* s, glui32 len, bool check)
{
  if (m_buffer != NULL)
  {
    for (glui32 i = 0; i < len; i++)
    {
      if (m_position < m_length)
        m_buffer[m_position++] = (unsigned char)s[i];
    }
  }
  m_write += len;
}

void I7GlkMemoryStream::putStr(glui32* s, glui32 len, bool check)
{
  if (m_buffer != NULL)
  {
    for (glui32 i = 0; i < len; i++)
    {
      if (m_position < m_length)
      {
        if (s[i] > 255)
          m_buffer[m_position++] = '?';
        else
          m_buffer[m_position++] = (unsigned char)s[i];
      }
    }
  }
  m_write += len;
}

I7GlkUniMemoryStream::I7GlkUniMemoryStream(glui32* buf, glui32 len, glui32 rock) : I7GlkStream(rock)
{
  m_buffer = buf;
  m_length = len;
  m_position = 0;

  if (registerArrFn && m_buffer && (m_length > 0))
    m_arrayRock = ((*registerArrFn)(m_buffer,m_length,(char*)"&+#!Iu"));
  else
    m_arrayRock.num = 0;
}

I7GlkUniMemoryStream::~I7GlkUniMemoryStream()
{
  if (unregisterArrFn && m_buffer && (m_length > 0))
    (*unregisterArrFn)(m_buffer,m_length,(char*)"&+#!Iu",m_arrayRock);
}

void I7GlkUniMemoryStream::setPosition(glsi32 pos, glui32 mode)
{
  if (mode == seekmode_Current)
    m_position += pos;
  else if (mode == seekmode_End)
    m_position = m_length + pos;
  else
    m_position = pos;

  if (m_position < 0)
    m_position = 0;
  if (m_position > m_length)
    m_position = m_length;
}

glui32 I7GlkUniMemoryStream::getPosition(void)
{
  return m_position;
}

glsi32 I7GlkUniMemoryStream::getChar(void)
{
  if (m_buffer == NULL)
    return -1;

  if (m_position < m_length)
  {
    m_read++;
    return (glsi32)m_buffer[m_position++];
  }
  return -1;
}

void I7GlkUniMemoryStream::putStr(char* s, glui32 len, bool check)
{
  if (m_buffer != NULL)
  {
    for (glui32 i = 0; i < len; i++)
    {
      if (m_position < m_length)
        m_buffer[m_position++] = (unsigned char)s[i];
    }
  }
  m_write += len;
}

void I7GlkUniMemoryStream::putStr(glui32* s, glui32 len, bool check)
{
  if (m_buffer != NULL)
  {
    for (glui32 i = 0; i < len; i++)
    {
      if (m_position < m_length)
        m_buffer[m_position++] = s[i];
    }
  }
  m_write += len;
}

I7GlkWinStream::I7GlkWinStream(I7GlkWindow* win, glui32 rock) : I7GlkStream(rock)
{
  m_win = win;
  m_style = 0;

  m_lastJustify = -1;
  m_newParagraph = true;

  m_buffered.reserve(1024);
  m_buffered.push_back(win->getId());
}

void I7GlkWinStream::setPosition(glsi32 pos, glui32 seekmode)
{
}

glui32 I7GlkWinStream::getPosition(void)
{
  return 0;
}

glsi32 I7GlkWinStream::getChar(void)
{
  return -1;
}

void I7GlkWinStream::putStr(char* s, glui32 len, bool check)
{
  if (check && m_win->inputActive())
  {
    fatalError("Printing text to a window that is waiting for line or character input is not allowed.");
    return;
  }

  if (m_write == 0)
    setParagraph();

  for (glui32 i = 0; i < len; i++)
  {
    addChar((unsigned char)s[i]);
    if (s[i] == '\n')
    {
      flush();
      setParagraph();
      m_newParagraph = true;
    }
    else
      m_newParagraph = false;
  }
  m_write += len;

  I7GlkStream* echo = m_win->getEchoStream();
  if (echo != NULL)
    echo->putStr(s,len,check);
}

void I7GlkWinStream::putStr(glui32* s, glui32 len, bool check)
{
  if (check && m_win->inputActive())
  {
    fatalError("Printing text to a window that is waiting for line or character input is not allowed.");
    return;
  }

  if (m_write == 0)
    setParagraph();

  for (glui32 i = 0; i < len; i++)
  {
    addChar(s[i]);
    if (s[i] == '\n')
    {
      flush();
      setParagraph();
    }
  }
  m_write += len;

  I7GlkStream* echo = m_win->getEchoStream();
  if (echo != NULL)
    echo->putStr(s,len,check);
}

void I7GlkWinStream::setStyle(glui32 style)
{
  flush();
  I7GlkStyle theStyle = m_win->getStyle(style);
  m_style = style;

  {
    int data[3];
    data[0] = m_win->getId();
    data[1] = StyleNormal;
    if (theStyle.m_weight == 1)
      data[1] |= StyleBold;
    if (theStyle.m_italic)
      data[1] |= StyleItalic;
    if (theStyle.m_proportional == false)
      data[1] |= StyleFixed;
    if (theStyle.m_reverse)
      data[1] |= StyleReverse;
    data[2] = theStyle.m_size;
    sendCommand(Command_SetStyle,sizeof data,data);
  }
  {
    int data[7];
    data[0] = m_win->getId();
    data[1] = (theStyle.m_textColour & 0x00FF0000) >> 16;
    data[2] = (theStyle.m_textColour & 0x0000FF00) >> 8;
    data[3] = (theStyle.m_textColour & 0x000000FF);
    data[4] = (theStyle.m_backColour & 0x00FF0000) >> 16;
    data[5] = (theStyle.m_backColour & 0x0000FF00) >> 8;
    data[6] = (theStyle.m_backColour & 0x000000FF);
    sendCommand(Command_SetColour,sizeof data,data);
  }
  if (m_newParagraph)
    setParagraph();
}

void I7GlkWinStream::setParagraph(void)
{
  I7GlkStyle theStyle = m_win->getStyle(m_style);
  if (m_lastJustify == theStyle.m_justify)
    return;

  int data[2];
  data[0] = m_win->getId();
  data[1] = theStyle.m_justify;
  sendCommand(Command_SetParagraph,sizeof data,data);
  m_lastJustify = theStyle.m_justify;
}

void I7GlkWinStream::setHyperlink(glui32 link)
{
  flush();

  int data[2];
  data[0] = m_win->getId();
  data[1] = link;
  sendCommand(Command_SetLink,sizeof data,data);
}

void I7GlkWinStream::flush(void)
{
  if (m_buffered.size() > 1)
  {
    sendCommand(Command_PrintOutput,m_buffered.size() * sizeof(wchar_t),&m_buffered[0]);
    m_buffered.resize(1);
  }
}

void I7GlkWinStream::addChar(glui32 c)
{
  if ((c == '\n') || (c >= 32 && c <= 126) || (c >= 160 && c <= 0xFFFF))
    m_buffered.push_back((wchar_t)c);
  else if (c >= 0x10000 && c <= 0x10FFFF)
  {
    // Unicode high-plane character
    const unsigned long LEAD_OFFSET = 0xD800 - (0x10000 >> 10);
    unsigned short hi = (unsigned short)(LEAD_OFFSET + (c >> 10));
    unsigned short lo = 0xDC00 + (c & 0x3FF);
    m_buffered.push_back((wchar_t)hi);
    m_buffered.push_back((wchar_t)lo);
  }
  else
  {
    char error[16];
    if (c <= 0xFF)
      sprintf(error,"[0x%02X]",(int)c);
    else
      sprintf(error,"[0x%08X]",(int)c);
    for (int i = 0; i < (int)strlen(error); i++)
      addChar((unsigned char)error[i]);
  }
}
