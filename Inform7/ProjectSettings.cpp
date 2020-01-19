#include "stdafx.h"
#include "ProjectSettings.h"
#include "PropList.h"
#include "Inform.h"
#include "Build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SETTINGS_FILE "\\Settings.plist"

ProjectSettings::ProjectSettings()
{
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
  m_blorb = propList.GetBoolean(L"IFOutputSettings",L"IFSettingCreateBlorb",true);
  m_predictable = propList.GetBoolean(L"IFOutputSettings",L"IFSettingNobbleRng",false);
  switch (propList.GetNumber(L"IFOutputSettings",L"IFSettingZCodeVersion"))
  {
  case 5:
  case 6:
  case 8:
    m_output = OutputZ8;
    break;
  case 256:
    m_output = OutputGlulx;
    break;
  default:
    m_output = OutputGlulx;
    break;
  }
  m_compilerVersion = propList.GetString(L"IFOutputSettings",L"IFSettingCompilerVersion");

  // Cope with old settings files
  if (propList.Exists(L"IFCompilerOptions",L"IFSettingNobbleRng"))
    m_predictable = propList.GetBoolean(L"IFCompilerOptions",L"IFSettingNobbleRng",false);
}

bool ProjectSettings::Save(const char* path)
{
  CString fileName = path;
  fileName += SETTINGS_FILE;

  if (!m_changed && (::GetFileAttributes(fileName) != INVALID_FILE_ATTRIBUTES))
    return true;

  FILE* settingsFile = fopen(fileName,"wt");
  if (settingsFile == NULL)
    return false;

  fprintf(settingsFile,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
      "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
    "<plist version=\"1.0\">\n"
    "<dict>\n"
    "\t<key>IFCompilerOptions</key>\n"
    "\t<dict>\n"
    "\t\t<key>IFSettingNaturalInform</key>\n"
    "\t\t<true/>\n"
    "\t</dict>\n"
    "\t<key>IFI7OutputSettings</key>\n"
    "\t<dict/>\n"
    "\t<key>IFLibrarySettings</key>\n"
    "\t<dict>\n"
    "\t\t<key>IFSettingLibraryToUse</key>\n"
    "\t\t<string>Natural</string>\n"
    "\t</dict>\n");

  fprintf(settingsFile,
    "\t<key>IFMiscSettings</key>\n"
    "\t<dict>\n"
    "\t\t<key>IFSettingInfix</key>\n"
    "\t\t<false/>\n"
    "\t</dict>\n");

  fprintf(settingsFile,
    "\t<key>IFOutputSettings</key>\n"
    "\t<dict>\n"
    "\t\t<key>IFSettingCreateBlorb</key>\n"
    "\t\t<%s/>\n"
    "\t\t<key>IFSettingNobbleRng</key>\n"
    "\t\t<%s/>\n"
    "\t\t<key>IFSettingZCodeVersion</key>\n"
    "\t\t<integer>%d</integer>\n"
    "\t\t<key>IFSettingCompilerVersion</key>\n"
    "\t\t<string>%s</string>\n"
    "\t</dict>\n",
    m_blorb ? "true" : "false",
    m_predictable ? "true" : "false",
    (int)m_output,
    (LPCSTR)m_compilerVersion);

  fprintf(settingsFile,
    "\t<key>IFRandomSettings</key>\n"
    "\t<dict/>\n"
    "</dict>\n"
    "</plist>\n");
  fclose(settingsFile);
  m_changed = false;
  return true;
}

CTime ProjectSettings::GetFileTimestamp(const char* path)
{
  CString fileName = path;
  fileName += SETTINGS_FILE;

  CFileStatus status;
  if (CFile::GetStatus(fileName,status))
    return status.m_mtime;
  return CTime(0);
}

CString ProjectSettings::GetInformSwitches(bool release, bool debugFile)
{
  CString switches("-w");
  switches += (!release) ? "SD" : "~S~D";
  switch (m_output)
  {
  case OutputZ8:
    switches += "v8";
    break;
  case OutputGlulx:
    switches += "G";
    break;
  }
  if (debugFile)
    switches += "k";
  return switches;
}

CString ProjectSettings::GetOutputFormat(void)
{
  switch (m_output)
  {
  case OutputZ8:
    return "z8";
  case OutputGlulx:
    return "ulx";
  }
  return "";
}

CString ProjectSettings::GetCompilerVersion(void)
{
  if (m_compilerVersion.IsEmpty() || (m_compilerVersion == "****"))
    return NI_BUILD;
  return m_compilerVersion;
}

void ProjectSettings::SetDefaults(void)
{
  m_output = OutputGlulx;
  m_blorb = true;
  m_predictable = false;
  m_changed = false;
}
