#include "I7GlkGridWindow.h"
#include "../../Inform7/InterpreterCommands.h"

extern int charWidth;
extern int charHeight;

void sendCommand(int command, int dataLength, const void* data);

I7GlkStyle I7GlkGridWindow::defaultStyles[2][style_NUMSTYLES];

I7GlkGridWindow::I7GlkGridWindow(glui32 rock) : I7GlkWindow(rock)
{
  for (int i = 0; i < style_NUMSTYLES; i++)
    m_styles[i] = defaultStyles[1][i];
}

void I7GlkGridWindow::requestKey(ReadKey readKey)
{
  m_readKey = readKey;
  sendCommand(Command_ReadKey,0,NULL);
}

void I7GlkGridWindow::requestLink(void)
{
  m_readLink = true;
}

void I7GlkGridWindow::requestMouse(void)
{
  m_readMouse = true;
}

void I7GlkGridWindow::moveCursor(int x, int y)
{
  m_stream->flush();

  int data[3];
  data[0] = m_id;
  data[1] = x;
  data[2] = y;
  sendCommand(Command_SetCursor,sizeof data,data);
}

void I7GlkGridWindow::clear(void)
{
  m_stream->flush();
  I7GlkStyle theStyle = getStyle(style_Normal);

  int data[2];
  data[0] = m_id;
  data[1] = theStyle.m_reverse ? 1 : 0;
  sendCommand(Command_Clear,sizeof data,data);
}

void I7GlkGridWindow::layout(const I7Rect& r)
{
  m_rect = r;
}

void I7GlkGridWindow::getNeededSize(int size, int& w, int& h, const I7Rect& r)
{
  w = charWidth * size;
  h = charHeight * size;
  if ((r.top == 0) && (size > 0))
    h += charHeight/4;
}

void I7GlkGridWindow::getSize(glui32* w, glui32* h)
{
  if (w != NULL)
    *w = m_rect.width() / charWidth;
  if (h != NULL)
    *h = m_rect.height() / charHeight;
}

I7GlkStyle I7GlkGridWindow::getStyle(int style)
{
  I7GlkStyle theStyle = I7GlkWindow::getStyle(style);
  theStyle.m_justify = stylehint_just_LeftFlush;
  theStyle.m_size = 0;
  theStyle.m_weight = 0;
  theStyle.m_italic = false;
  theStyle.m_proportional = false;
  return theStyle;
}
