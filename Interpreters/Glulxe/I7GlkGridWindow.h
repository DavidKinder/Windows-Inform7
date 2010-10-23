#pragma once

#include "I7GlkWindow.h"

class I7GlkGridWindow : public I7GlkWindow
{
public:
  I7GlkGridWindow(glui32 rock);

  void requestKey(ReadKey readKey);
  void requestLink(void);
  void requestMouse(void);

  void moveCursor(int x, int y);
  void clear(void);

  void layout(const I7Rect& r);
  void getNeededSize(int size, int& w, int& h, const I7Rect& r);
  void getSize(glui32* w, glui32* h);

  I7GlkStyle getStyle(int style);

  static I7GlkStyle defaultStyles[2][style_NUMSTYLES];

protected:
  I7Rect m_rect;
};
