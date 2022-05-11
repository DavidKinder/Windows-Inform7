#include <cstdio>
#include <ctime>

int main(int, char**)
{
  time_t now;
  time(&now);

  struct tm* timeNow = localtime(&now);

  char* afterNum = NULL;
  switch (timeNow->tm_mday)
  {
  case 1:
  case 21:
  case 31:
    afterNum = "st";
    break;
  case 2:
  case 22:
    afterNum = "nd";
    break;
  case 3:
  case 23:
    afterNum = "rd";
    break;
  default:
    afterNum = "th";
    break;
  }

  char monthYear[256];
  strftime(monthYear,256,"%B %Y",timeNow);

  char informVersion[32], line[256];
  FILE* contentsFile = fopen("../Distribution/inform/inform7/Contents.w","rt");
  if (contentsFile == NULL)
    return 0;
  while (feof(contentsFile) == 0)
  {
    fgets(line,256,contentsFile);
    sscanf(line,"Version Number: %s",informVersion);
  }
  fclose(contentsFile);

  FILE* outFile = fopen("Build.h","wt");
  fprintf(outFile,"#define BUILD_DATE \"%d%s %s\"\n",timeNow->tm_mday,afterNum,monthYear);
  fprintf(outFile,"#define INFORM_VER \"%s\"\n",informVersion);
  fclose(outFile);

  outFile = fopen("..\\Installer\\Inform7.nsh","wt");
  fprintf(outFile,"!define INFORM_VER %s\n",informVersion);
  fclose(outFile);
  return 0;
}
