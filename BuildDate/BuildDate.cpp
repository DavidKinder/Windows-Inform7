#include <stdio.h>
#include <time.h>
#include <string>

char informVersion[32] = "?";
std::string buildHeader;

void createOutput(void)
{
  time_t now;
  time(&now);
  struct tm* timeNow = localtime(&now);

  char line[256];
  FILE* contentsFile = fopen("../Distribution/inform/inform7/Contents.w","rt");
  if (contentsFile == NULL)
    exit(1);
  while (feof(contentsFile) == 0)
  {
    fgets(line,256,contentsFile);
    sscanf(line,"Version Number: %s",informVersion);
  }
  fclose(contentsFile);

  sprintf(line,"#define BUILD_DATE \"%04d%02d%02d\"\n",
    timeNow->tm_year+1900,timeNow->tm_mon+1,timeNow->tm_mday);
  buildHeader = line;
  sprintf(line,"#define INFORM_VER \"%s\"\n",informVersion);
  buildHeader += line;
}

std::string getCurrent(void)
{
  std::string currentHeader;

  FILE* inFile = fopen("Build.h","rt");
  if (inFile != NULL)
  {
    char line[256];

    fgets(line,256,inFile);
    currentHeader = line;
    fgets(line,256,inFile);
    currentHeader += line;
    fclose(inFile);
  }
  return currentHeader;
}

int main(int, char**)
{
  createOutput();
  std::string currentHeader = getCurrent();
  if (currentHeader != buildHeader)
  {
    printf("Creating new Build.h...\n");
    FILE* outFile = fopen("Build.h","wt");
    fputs(buildHeader.c_str(),outFile);
    fclose(outFile);

    outFile = fopen("..\\Installer\\Inform7.nsh","wt");
    fprintf(outFile,"!define INFORM_VER %s\n",informVersion);
    fclose(outFile);
  }
  return 0;
}
