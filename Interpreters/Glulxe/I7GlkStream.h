#pragma once

#include <string>
#include <set>
#include <vector>
#include <stdio.h>

extern "C" {
#include "glk.h"
#include "gi_dispa.h"
}

class I7GlkStream
{
public:
  I7GlkStream(glui32 rock);
  virtual ~I7GlkStream();

  glui32 getRock(void) { return m_rock; }
  void setDispRock(const gidispatch_rock_t& rock) { m_dispRock = rock; }
  gidispatch_rock_t& getDispRock(void) { return m_dispRock; }

  void getResults(stream_result_t *result);

  virtual void setPosition(glsi32 pos, glui32 seekmode) = 0;
  virtual glui32 getPosition(void) = 0;

  virtual glsi32 getChar(void) = 0;
  virtual glui32 getLine(char *buf, glui32 len);
  virtual glui32 getBuffer(char *buf, glui32 len);
  virtual glui32 getLine(glui32 *buf, glui32 len);
  virtual glui32 getBuffer(glui32 *buf, glui32 len);

  virtual void putStr(char* s, glui32 len, bool check) = 0;
  virtual void putStr(glui32* s, glui32 len, bool check) = 0;

  virtual void setHyperlink(glui32 link) {}

protected:
  glui32 m_rock;
  gidispatch_rock_t m_dispRock;

  int m_read;
  int m_write;
};

extern std::set<I7GlkStream*> glkStreams;

class I7GlkFile;

class I7GlkFileStream : public I7GlkStream
{
public:
  I7GlkFileStream(glui32 rock);
  virtual ~I7GlkFileStream();

  void setPosition(glsi32 pos, glui32 seekmode);
  glui32 getPosition(void);

  glsi32 getChar(void);

  void putStr(char* s, glui32 len, bool check);
  void putStr(glui32* s, glui32 len, bool check);

  bool open(I7GlkFile* file, glui32 mode);

protected:
  void setNextOperation(glui32 oper);

  FILE* m_file;
  glui32 m_lastOper;
};

class I7GlkUniFileStream : public I7GlkStream
{
public:
  I7GlkUniFileStream(glui32 rock);
  virtual ~I7GlkUniFileStream();

  void setPosition(glsi32 pos, glui32 seekmode);
  glui32 getPosition(void);

  glsi32 getChar(void);

  void putStr(char* s, glui32 len, bool check);
  void putStr(glui32* s, glui32 len, bool check);

  bool open(I7GlkFile* file, glui32 mode);

protected:
  void addChar(glui32 c);
  void setNextOperation(glui32 oper);

  bool m_text;
  FILE* m_file;
  glui32 m_lastOper;
};

class I7GlkMemoryStream : public I7GlkStream
{
public:
  I7GlkMemoryStream(char* buf, glui32 len, glui32 rock);
  virtual ~I7GlkMemoryStream();

  void setPosition(glsi32 pos, glui32 seekmode);
  glui32 getPosition(void);

  glsi32 getChar(void);

  void putStr(char* s, glui32 len, bool check);
  void putStr(glui32* s, glui32 len, bool check);

protected:
  gidispatch_rock_t m_arrayRock;

  char* m_buffer;
  glui32 m_length;
  glui32 m_position;
};

class I7GlkUniMemoryStream : public I7GlkStream
{
public:
  I7GlkUniMemoryStream(glui32* buf, glui32 len, glui32 rock);
  virtual ~I7GlkUniMemoryStream();

  void setPosition(glsi32 pos, glui32 seekmode);
  glui32 getPosition(void);

  glsi32 getChar(void);

  void putStr(char* s, glui32 len, bool check);
  void putStr(glui32* s, glui32 len, bool check);

protected:
  gidispatch_rock_t m_arrayRock;

  glui32* m_buffer;
  glui32 m_length;
  glui32 m_position;
};

class I7GlkWindow;

class I7GlkWinStream : public I7GlkStream
{
public:
  I7GlkWinStream(I7GlkWindow* win, glui32 rock);

  void setPosition(glsi32 pos, glui32 seekmode);
  glui32 getPosition(void);

  glsi32 getChar(void);

  void putStr(char* s, glui32 len, bool check);
  void putStr(glui32* s, glui32 len, bool check);

  void setStyle(glui32 style);
  void setColours(glui32 fg, glui32 bg);
  void setReverse(bool reverse);
  void setParagraph(void);
  void setHyperlink(glui32 link);

  void flush(void);
  void sendStyle(void);
  void sendColours(void);

protected:
  void addChar(glui32 c);

  I7GlkWindow* m_win;
  glui32 m_style;
  glui32 m_textColour;
  glui32 m_backColour;
  bool m_reverse;
  std::vector<wchar_t> m_buffered;

  int m_lastJustify;
  bool m_newParagraph;
};
