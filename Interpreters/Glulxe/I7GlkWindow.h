#pragma once

#include "I7GlkStream.h"

#include <set>

extern "C" {
#include "Glk.h"
#include "gi_dispa.h"
}

struct I7Rect
{
  int left;
  int top;
  int right;
  int bottom;

  I7Rect();
  I7Rect(int l, int t, int r, int b);

  int width() const;
  int height() const;
};

struct I7GlkStyle
{
  I7GlkStyle();
  I7GlkStyle(int weight, bool proportional);
  bool operator==(const I7GlkStyle& style);

  int m_justify;
  int m_size;
  int m_weight;
  bool m_italic;
  bool m_proportional;
  int m_textColour;
  int m_backColour;
  bool m_reverse;
};

class I7GlkPairWindow;

class I7GlkWindow
{
public:
  I7GlkWindow(glui32 rock);
  virtual ~I7GlkWindow();

  int getId(void) { return m_id; }
  glui32 getRock(void) { return m_rock; }
  void setDispRock(const gidispatch_rock_t& rock) { m_dispRock = rock; }
  gidispatch_rock_t& getDispRock(void) { return m_dispRock; }

  I7GlkPairWindow* getParent(void) { return m_parent; }
  void setParent(I7GlkPairWindow* parent) { m_parent = parent; }

  I7GlkWinStream* getStream(void) { return m_stream; }
  void getStreamResults(stream_result_t *result) { m_stream->getResults(result); }

  I7GlkStream* getEchoStream(void) { return m_echo; }
  void setEchoStream(I7GlkStream* stream) { m_echo = stream; }

  enum ReadKey
  {
    ReadKeyNone,
    ReadKeyAscii,
    ReadKeyUnicode,
  };

  virtual void requestLine(char* buf, glui32 maxlen, glui32 initlen) {}
  virtual void requestLine(glui32* buf, glui32 maxlen, glui32 initlen) {}
  virtual void endLine(event_t* event, glui32 len, bool cancel);
  virtual void requestKey(ReadKey readKey) {}
  virtual void endKey(event_t* event, glui32 len, bool cancel);
  virtual void requestMouse(void) {}
  virtual void endMouse(event_t* event, int x, int y);
  virtual void requestLink(void) {}
  virtual void endLink(event_t* event, int link);

  virtual void moveCursor(int x, int y) {}
  virtual void clear(void) {}
  virtual glui32 draw(glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
    { return 0; }

  virtual void layout(const I7Rect& r) = 0;
  virtual void getNeededSize(int size, int& w, int& h, const I7Rect& r) = 0;
  virtual void getSize(glui32* w, glui32* h) = 0;

  virtual I7GlkStyle getStyle(int style);

  virtual void closeWindow(void);

protected:
  int m_id;
  glui32 m_rock;
  gidispatch_rock_t m_dispRock;

  ReadKey m_readKey;
  bool m_readMouse;
  bool m_readLink;
  I7GlkStyle m_styles[style_NUMSTYLES];

  I7GlkPairWindow* m_parent;
  I7GlkWinStream* m_stream;
  I7GlkStream* m_echo;
};

extern std::set<I7GlkWindow*> glkWindows;
