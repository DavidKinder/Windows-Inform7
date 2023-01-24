#include <map>
#include <math.h>
#include <windows.h>

#include "I7GlkStream.h"
#include "../../Inform7/InterpreterCommands.h"

extern "C" {
#include "glk.h"

int locate_gamefile(int isblorb);
extern strid_t gamefile;
}

extern int displayWidth;
extern int displayHeight;
extern int charWidth;
extern int charHeight;
extern std::map<int,std::pair<int,int> > imageSizes;
extern char* fileDir;

void setDarkMode(int mode);

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
  if (length == 0)
    return;

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

extern "C" void fatalError(const char* s)
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

int main(int argc, char** argv)
{
  if (argc < 8)
    exit(1);

  frefid_t gameRef = glk_fileref_create_by_name(fileusage_BinaryMode|fileusage_Data,argv[1],0);
  if (gameRef == 0)
    exit(2);

  gamefile = glk_stream_open_file(gameRef,filemode_Read,0);
  glk_fileref_destroy(gameRef);
  if (gamefile == 0)
    exit(3);

  glk_stream_set_position(gamefile,0,seekmode_Start);
  char start[12];
  if (glk_get_buffer_stream(gamefile,start,12) < 12)
    exit(4);

  if (memcmp(start,"Glul",4) == 0)
  {
    if (locate_gamefile(0) == 0)
      exit(5);
  }
  else if ((memcmp(start,"FORM",4) == 0) && (memcmp(start+8,"IFRS",4) == 0))
  {
    if (locate_gamefile(1) == 0)
      exit(6);
  }
  else
    exit(7);

  displayWidth = atoi(argv[2]);
  displayHeight = atoi(argv[3]);
  charWidth = atoi(argv[4]);
  charHeight = atoi(argv[5]);
  setDarkMode(atoi(argv[6]));
  fileDir = argv[7];
  readImageSizes(argv[1]);

  glk_main();
  glk_exit();
}
