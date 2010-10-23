#include <math.h>
#include <stdio.h>
#include <map>
#include <windows.h>

#include "../Glulxe/I7GlkStream.h"
#include "../../Inform7/InterpreterCommands.h"

extern "C" {
#include "Glk.h"
#include "git.h"
}

extern int displayWidth;
extern int displayHeight;
extern int charWidth;
extern int charHeight;
extern std::map<int,std::pair<int,int> > imageSizes;
extern char* fileDir;

void sendCommand(int command, int dataLength, const void* data)
{
  DWORD written = 0;
  HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);

  ::WriteFile(out,&command,sizeof command,&written,NULL);
  ::WriteFile(out,&dataLength,sizeof dataLength,&written,NULL);
  if (dataLength > 0)
    ::WriteFile(out,data,dataLength,&written,NULL);
}

void readReturnData(void* data, int length)
{
  HANDLE in = ::GetStdHandle(STD_INPUT_HANDLE);
  DWORD read = 0;

  ::ReadFile(in,data,length,&read,NULL);
  if (read != length)
    exit(1);
}

void readImageSizes(const char* gamePath)
{
  char sizePath[_MAX_PATH];
  strcpy(sizePath,gamePath);
  char* sizeExt = strrchr(sizePath,'\\');
  if (sizeExt == NULL)
    return;
  strcpy(sizeExt+1,"image.txt");

  FILE* sizeFile = fopen(sizePath,"rt");
  if (sizeFile == NULL)
    return;

  while (true)
  {
    char sizeLine[256];
    if (fgets(sizeLine,sizeof sizeLine,sizeFile) == NULL)
      break;
    int num, w, h;
    if (sscanf(sizeLine,"%d %d %d",&num,&w,&h) != 3)
      break;
    imageSizes[num] = std::make_pair(w,h);
  }
  fclose(sizeFile);
}

void fatalError(const char* s)
{
  for (strid_t str = glk_stream_iterate(0,NULL); str != 0; str = glk_stream_iterate(str,NULL))
  {
    I7GlkWinStream* wstr = dynamic_cast<I7GlkWinStream*>((I7GlkStream*)str);
    if (wstr != NULL)
      wstr->flush();
  }

  sendCommand(Command_FatalError,strlen(s) * sizeof s[0],s);
  exit(0);
}

#define CACHE_SIZE (256 * 1024)
#define UNDO_SIZE (768 * 1024)

int main(int argc, char** argv)
{
  if (argc < 7)
    exit(1);

  displayWidth = atoi(argv[2]);
  displayHeight = atoi(argv[3]);
  charWidth = atoi(argv[4]);
  charHeight = atoi(argv[5]);
  fileDir = argv[6];
  readImageSizes(argv[1]);

  size_t size = 0;
  void* mapping = NULL;
  void* file = CreateFile(argv[1],
    GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if (file != INVALID_HANDLE_VALUE)
  {
    size = GetFileSize(file,0);
    mapping = CreateFileMapping(file,0,PAGE_READONLY,0,0,0);
  }

  void* ptr = NULL;
  if (mapping != NULL)
    ptr = MapViewOfFile(mapping,FILE_MAP_READ,0,0,0);

  if (ptr != NULL)
    git((const git_uint8*)ptr,size,CACHE_SIZE,UNDO_SIZE);

  if (ptr != NULL)
    UnmapViewOfFile(ptr);
  if (mapping != NULL)
    CloseHandle(mapping);
  if (file != INVALID_HANDLE_VALUE)
    CloseHandle(file);

  glk_exit();
}

extern "C" float git_powf(float x, float y)
{
  if (x == 1.0f)
    return 1.0f;
  else if ((y == 0.0f) || (y == -0.0f))
    return 1.0f;
  else if ((x == -1.0f) && isinf(y))
    return 1.0;
  return powf(x,y);
}

