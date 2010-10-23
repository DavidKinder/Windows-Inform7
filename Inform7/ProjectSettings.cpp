#include "stdafx.h"
#include "ProjectSettings.h"
#include "PropList.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SETTINGS_FILE "\\Settings.plist"

std::set<int> ProjectSettings::m_compilers;

ProjectSettings::ProjectSettings()
{
  if (m_compilers.empty())
  {
    // Find versions of the Inform 6 compiler
    CFileFind compilers;
    BOOL found = compilers.FindFile(theApp.GetAppDir()+"\\Compilers\\inform-*.exe");
    while (found)
    {
      found = compilers.FindNextFile();

      // Extract the version number from the executable name
      int version = 0;
      if (sscanf(compilers.GetFileTitle(),"inform-%d",&version) == 1)
        m_compilers.insert(version);
    }
  }

  SetDefaults();
}

void ProjectSettings::Load(const char* path)
{
  SetDefaults();

  CString fileName = path;
  fileName += SETTINGS_FILE;

  // Read in the XML file
  PropList propList;
  if (propList.Load(fileName) == false)
    return;

  // Get the settings from the XML
  m_predictable = propList.GetBoolean(L"IFCompilerOptions",L"IFSettingNobbleRng",false);
  m_blorb = propList.GetBoolean(L"IFOutputSettings",L"IFSettingCreateBlorb",true);
  switch (propList.GetNumber(L"IFOutputSettings",L"IFSettingZCodeVersion"))
  {
  case 5:
    m_output = OutputZ5;
    break;
  case 8:
    m_output = OutputZ8;
    break;
  case 256:
    m_output = OutputGlulx;
    break;
  default:
    m_output = OutputZ5;
    break;
  }
}

bool ProjectSettings::Save(const char* path)
{
  CString fileName = path;
  fileName += SETTINGS_FILE;

  FILE* settingsFile = fopen(fileName,"wt");
  if (settingsFile == NULL)
    return false;

  fprintf(settingsFile,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "
      "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
    "<plist version=\"1.0\">\n"
    "<dict>\n");

  fprintf(settingsFile,
    "\t<key>IFCompilerOptions</key>\n"
    "\t<dict>\n"
    "\t\t<key>IFSettingNaturalInform</key>\n"
    "\t\t<true/>\n"
    "\t\t<key>IFSettingNobbleRng</key>\n"
    "\t\t<%s/>\n"
    "\t</dict>\n",
    m_predictable ? "true" : "false");

  fprintf(settingsFile,
    "\t<key>IFLibrarySettings</key>\n"
    "\t<dict>\n"
    "\t\t<key>IFSettingLibraryToUse</key>\n"
    "\t\t<string>Natural</string>\n"
    "\t</dict>\n");

  fprintf(settingsFile,
    "\t<key>IFOutputSettings</key>\n"
    "\t<dict>\n"
    "\t\t<key>IFSettingCreateBlorb</key>\n"
    "\t\t<%s/>\n"
    "\t\t<key>IFSettingZCodeVersion</key>\n"
    "\t\t<integer>%d</integer>\n"
    "\t</dict>\n",
    m_blorb ? "true" : "false",
    (int)m_output);

  fprintf(settingsFile,
    "</dict>\n"
    "</plist>\n");
  fclose(settingsFile);
  return true;
}

CString ProjectSettings::GetInformSwitches(bool release)
{
  CString switches("-w");
  switches += (!release) ? "SD" : "~S~D";

  switch (m_output)
  {
  case OutputZ5:
    switches += "v5";
    break;
  case OutputZ8:
    switches += "v8";
    break;
  case OutputGlulx:
    switches += "G";
    break;
  }

  return switches;
}

CString ProjectSettings::GetOutputExtension(void)
{
  switch (m_output)
  {
  case OutputZ5:
    return "z5";
  case OutputZ8:
    return "z8";
  case OutputGlulx:
    return "ulx";
  }
  return "";
}

void ProjectSettings::SetDefaults(void)
{
  m_output = OutputZ5;
  m_blorb = true;
  m_predictable = false;

  m_compiler = 0;
  if (!m_compilers.empty())
  {
    // Use the latest available compiler
    m_compiler = *m_compilers.rbegin();
  }
}
