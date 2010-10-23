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

  char buildCount[16], line[256];
  FILE* bcFile = fopen("../Distribution/Inform7/Contents.w","rt");
  while (feof(bcFile) == 0)
  {
    fgets(line,256,bcFile);
    sscanf(line,"Build Number: %s",buildCount);
  }
  fclose(bcFile);
  buildCount[4] = 0;

  FILE* outFile = fopen("Build.h","wt");
  fprintf(outFile,"#define BUILD_DATE \"%d%s %s\"\n",timeNow->tm_mday,afterNum,monthYear);
  fprintf(outFile,"#define NI_BUILD \"%s\"\n",buildCount);
  fclose(outFile);

  outFile = fopen("..\\Installer\\Inform7.nsh","wt");
  fprintf(outFile,"!define BUILD %s\n",buildCount);
  fclose(outFile);
}
