#pragma once

extern "C" {
#include "Glk.h"
#include "gi_dispa.h"
}

#include <string>
#include <set>

class I7GlkFile
{
public:
  I7GlkFile(glui32 use, glui32 rock, const char* fileName);
  I7GlkFile(glui32 use, glui32 rock, I7GlkFile* copy);
  virtual ~I7GlkFile();

  glui32 getRock(void) { return m_rock; }
  void setDispRock(const gidispatch_rock_t& rock) { m_dispRock = rock; }
  gidispatch_rock_t& getDispRock(void) { return m_dispRock; }

  const char* fileName(void);
  bool isText(void);
  bool exists(void);
  void deleteFile(void);

protected:
  glui32 m_use;
  glui32 m_rock;
  gidispatch_rock_t m_dispRock;
  std::string m_fileName;
};

extern std::set<I7GlkFile*> glkFiles;
