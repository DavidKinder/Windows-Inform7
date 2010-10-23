#pragma once

#include "I7GlkWindow.h"

class I7GlkPairWindow : public I7GlkWindow
{
public:
  I7GlkPairWindow(I7GlkWindow* win1, I7GlkWindow* win2, glui32 method, glui32 size);
  ~I7GlkPairWindow();

  void layout(const I7Rect& r);
  void getNeededSize(int size, int& w, int& h, const I7Rect& r);
  void getSize(glui32* w, glui32* h);

  void setArrangement(glui32 method, glui32 size, I7GlkWindow* keywin);
  void getArrangement(glui32* method, glui32* size, winid_t* keywin);
  void replace(I7GlkWindow* oldWin, I7GlkWindow* newWin);
  void removeChildren(void);
  void removeKey(I7GlkWindow* win);

  I7GlkWindow* getSibling(I7GlkWindow* win);
  I7GlkWindow* getChild1(void) { return m_win1; }
  I7GlkWindow* getChild2(void) { return m_win2; }

protected:
  I7GlkWindow* m_win1;
  I7GlkWindow* m_win2;
  I7GlkWindow* m_key;

  glui32 m_method;
  glui32 m_size;
};
