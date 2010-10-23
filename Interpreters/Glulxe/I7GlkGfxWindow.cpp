#include "I7GlkGfxWindow.h"
#include "../../Inform7/InterpreterCommands.h"

void sendCommand(int command, int dataLength, const void* data);
void readReturnData(void* data, int length);

I7GlkGfxWindow::I7GlkGfxWindow(glui32 rock) : I7GlkWindow(rock)
{
  m_backColour = 0x00FFFFFF;
}

void I7GlkGfxWindow::requestMouse(void)
{
  m_readMouse = true;
}

void I7GlkGfxWindow::clear(void)
{
  fillRect(m_rect,m_backColour);
}

glui32 I7GlkGfxWindow::draw(glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
{
  int data[6];
  data[0] = m_id;
  data[1] = image;
  data[2] = val1;
  data[3] = val2;
  data[4] = width;
  data[5] = height;
  sendCommand(Command_Draw,sizeof data,data);
  return 1;
}

void I7GlkGfxWindow::layout(const I7Rect& r)
{
  m_rect = r;
}

void I7GlkGfxWindow::getNeededSize(int size, int& w, int& h, const I7Rect& r)
{
  w = size;
  h = size;
}

void I7GlkGfxWindow::getSize(glui32* w, glui32* h)
{
  if (w != NULL)
    *w = m_rect.width();
  if (h != NULL)
    *h = m_rect.height();
}

void I7GlkGfxWindow::fillRect(const I7Rect& r, int colour)
{
  int data[8];
  data[0] = m_id;
  data[1] = r.left;
  data[2] = r.top;
  data[3] = r.right;
  data[4] = r.bottom;
  data[5] = (colour & 0x00FF0000) >> 16;
  data[6] = (colour & 0x0000FF00) >> 8;
  data[7] = (colour & 0x000000FF);
  sendCommand(Command_FillRect,sizeof data,data);
}

void I7GlkGfxWindow::setBackColour(int colour)
{
  m_backColour = colour;

  int data[4];
  data[0] = m_id;
  data[1] = (colour & 0x00FF0000) >> 16;
  data[2] = (colour & 0x0000FF00) >> 8;
  data[3] = (colour & 0x000000FF);
  sendCommand(Command_BackColour,sizeof data,data);
}
