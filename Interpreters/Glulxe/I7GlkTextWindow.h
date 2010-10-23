#pragma once

#include "I7GlkWindow.h"

class I7GlkTextWindow : public I7GlkWindow
{
public:
  I7GlkTextWindow(glui32 rock);
  ~I7GlkTextWindow();

  void requestLine(char* buf, glui32 maxlen, glui32 initlen);
  void requestLine(glui32* buf, glui32 maxlen, glui32 initlen);
  void endLine(event_t* event, glui32 len, bool cancel);
  void requestKey(ReadKey readKey);
  void requestLink(void);

  void clear(void);
  glui32 draw(glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height);

  void layout(const I7Rect& r);
  void getNeededSize(int size, int& w, int& h, const I7Rect& r);
  void getSize(glui32* w, glui32* h);

  static I7GlkStyle defaultStyles[2][style_NUMSTYLES];

protected:
  gidispatch_rock_t m_arrayRock;

  char* m_lineBuffer;
  glui32* m_lineUBuffer;
  glui32 m_lineLength;

  I7Rect m_rect;
};
