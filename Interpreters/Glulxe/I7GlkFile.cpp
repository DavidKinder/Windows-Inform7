#include "I7GlkCmd.h"
#include "I7GlkFile.h"
#include "../../Inform7/InterpreterCommands.h"

#include <sstream>
#include <windows.h>

std::set<I7GlkFile*> glkFiles;
char* fileDir = NULL;

extern gidispatch_rock_t (*registerObjFn)(void *obj, glui32 objclass);
extern void (*unregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock);

I7GlkFile::I7GlkFile(glui32 use, glui32 rock, const char* fileName, bool checkName)
{
  m_use = use;
  m_rock = rock;

  // During startup always use the exact path as given
  if (fileDir == NULL)
    checkName = false;

  std::ostringstream path;
  if (checkName)
  {
    path << fileDir << '\\';

    // Copy the file name, removing any illegal characters, and
    // stopping at the file extension, if any
    bool isNull = true;
    const char* illegal = "/\\<>:\"|?*";
    for (const char* f = fileName; *f != '\0'; f++)
    {
      if (strchr(illegal,*f) == NULL)
      {
        if (*f == '.')
          break;
        path << *f;
        isNull = false;
      }
    }

    // If the filename is now empty, use 'null'
    if (isNull)
      path << "null";

    // Add the appropriate file extension
    switch (use & fileusage_TypeMask)
    {
    case fileusage_Data:
      path << ".glkdata";
      break;
    case fileusage_SavedGame:
      path << ".glksave";
      break;
    case fileusage_Transcript:
    case fileusage_InputRecord:
      path << ".txt";
      break;
    }
  }
  else
    path << fileName;

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

static FrontEndCmd readFilePath(void)
{
  for (;;)
  {
    while (readCommand());

    for (std::deque<FrontEndCmd>::iterator it = commands.begin(); it != commands.end(); ++it)
    {
      if (it->cmd == Return_FilePath)
      {
        FrontEndCmd cmd = *it;
        commands.erase(it);
        return cmd;
      }
    }

    ::Sleep(50);
  }
}

std::string I7GlkFile::fileDialog(glui32 use, glui32 fmode)
{
  int data[2];
  switch (use & fileusage_TypeMask)
  {
  case fileusage_Data:
    data[0] = File_GlkData;
    break;
  case fileusage_SavedGame:
    data[0] = File_GlkSave;
    break;
  case fileusage_Transcript:
  case fileusage_InputRecord:
  default:
    data[0] = File_Text;
    break;
  }
  data[1] = (fmode != filemode_Read);
  sendCommand(Command_FileDialog,sizeof data,data);

  FrontEndCmd pathCmd = readFilePath();
  if (pathCmd.len > 0)
    return std::string((char*)pathCmd.data,pathCmd.len);
  pathCmd.free();
  return "";
}
