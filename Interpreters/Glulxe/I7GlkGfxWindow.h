#pragma once

#include "I7GlkWindow.h"

class I7GlkGfxWindow : public I7GlkWindow
{
public:
  I7GlkGfxWindow(glui32 rock);

  void requestMouse(void);

  void clear(void);
  glui32 draw(glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height);

  void layout(const I7Rect& r);
  void getNeededSize(int size, int& w, int& h, const I7Rect& r);
  void getSize(glui32* w, glui32* h);

  void fillRect(const I7Rect& r, int colour);
  void setBackColour(int colour);

protected:
  I7Rect m_rect;
  int m_backColour;
};
