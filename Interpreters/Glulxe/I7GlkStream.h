#pragma once

#include <cstdio>
#include <string>
#include <set>
#include <vector>

extern "C" {
#include "Glk.h"
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
  virtual glui32 getLine(char *buf, glui32 len) = 0;
  virtual glui32 getBuffer(char *buf, glui32 len) = 0;

  virtual void putStr(char* s, glui32 len) = 0;
  virtual void putStr(glui32* s, glui32 len) = 0;

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
  glui32 getLine(char *buf, glui32 len);
  glui32 getBuffer(char *buf, glui32 len);

  void putStr(char* s, glui32 len);
  void putStr(glui32* s, glui32 len);

  bool open(I7GlkFile* file, glui32 mode);

protected:
  FILE* m_file;
};

class I7GlkMemoryStream : public I7GlkStream
{
public:
  I7GlkMemoryStream(char* buf, glui32 len, glui32 rock);
  virtual ~I7GlkMemoryStream();

  void setPosition(glsi32 pos, glui32 seekmode);
  glui32 getPosition(void);

  glsi32 getChar(void);
  glui32 getLine(char *buf, glui32 len);
  glui32 getBuffer(char *buf, glui32 len);

  void putStr(char* s, glui32 len);
  void putStr(glui32* s, glui32 len);

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
  glui32 getLine(char *buf, glui32 len);
  glui32 getBuffer(char *buf, glui32 len);

  void putStr(char* s, glui32 len);
  void putStr(glui32* s, glui32 len);

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
  glui32 getLine(char *buf, glui32 len);
  glui32 getBuffer(char *buf, glui32 len);

  void putStr(char* s, glui32 len);
  void putStr(glui32* s, glui32 len);

  void setStyle(glui32 style);
  void setParagraph(void);
  void setHyperlink(glui32 link);

  void flush(void);

protected:
  I7GlkWindow* m_win;
  glui32 m_style;
  std::vector<wchar_t> m_buffered;

  int m_lastJustify;
  bool m_newParagraph;
};
