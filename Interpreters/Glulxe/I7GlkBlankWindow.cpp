#include "I7GlkBlankWindow.h"

I7GlkBlankWindow::I7GlkBlankWindow(glui32 rock) : I7GlkWindow(rock)
{
}

void I7GlkBlankWindow::layout(const I7Rect& r)
{
}

void I7GlkBlankWindow::getNeededSize(int size, int& w, int& h, const I7Rect& r)
{
  w = 0;
  h = 0;
}

void I7GlkBlankWindow::getSize(glui32* w, glui32* h)
{
  if (w != NULL)
    *w = 0;
  if (h != NULL)
    *h = 0;
}
