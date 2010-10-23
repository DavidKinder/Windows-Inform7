#include "I7GlkFile.h"

#include <sstream>
#include <windows.h>

std::set<I7GlkFile*> glkFiles;
char* fileDir = NULL;

extern gidispatch_rock_t (*registerObjFn)(void *obj, glui32 objclass);
extern void (*unregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock);

I7GlkFile::I7GlkFile(glui32 use, glui32 rock, const char* fileName)
{
  m_use = use;
  m_rock = rock;

  std::ostringstream path;
  if (fileDir != NULL)
  {
    path << fileDir << '\\';

    for (const char* f = fileName; *f != '\0'; f++)
    {
      if ((*f == '\\') || (*f == '/') || (*f == ':'))
        path << '-';
      else
        path << *f;
    }
  }
  else
    path << fileName;

  if (strchr(fileName,'.') == NULL)
  {
    if ((m_use & fileusage_TypeMask) == fileusage_Data)
      path << ".glkdata";
  }

  m_fileName = path.str();
  glkFiles.insert(this);

  if (registerObjFn)
    setDispRock((*registerObjFn)(this,gidisp_Class_Fileref));
  else
    m_dispRock.num = 0;
}

I7GlkFile::I7GlkFile(glui32 use, glui32 rock, I7GlkFile* copy)
{
  m_use = use;
  m_rock = rock;
  m_fileName = copy->m_fileName;

  glkFiles.insert(this);

  if (registerObjFn)
    setDispRock((*registerObjFn)(this,gidisp_Class_Fileref));
  else
    m_dispRock.num = 0;
}

I7GlkFile::~I7GlkFile()
{
  if (unregisterObjFn)
    (*unregisterObjFn)(this,gidisp_Class_Fileref,getDispRock());

  glkFiles.erase(this);
}

const char* I7GlkFile::fileName(void)
{
  return m_fileName.c_str();
}

bool I7GlkFile::isText(void)
{
  return (m_use & fileusage_TextMode);
}

bool I7GlkFile::exists(void)
{
  return ::GetFileAttributes(m_fileName.c_str()) != INVALID_FILE_ATTRIBUTES;
}

void I7GlkFile::deleteFile(void)
{
  ::DeleteFile(m_fileName.c_str());
}
