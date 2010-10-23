#pragma once

#include "I7GlkWindow.h"

class I7GlkBlankWindow : public I7GlkWindow
{
public:
  I7GlkBlankWindow(glui32 rock);

  void layout(const I7Rect& r);
  void getNeededSize(int size, int& w, int& h, const I7Rect& r);
  void getSize(glui32* w, glui32* h);
};
