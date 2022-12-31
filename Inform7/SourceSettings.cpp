#include "stdafx.h"
#include "SourceSettings.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SourceSettingsRegistry::SourceSettingsRegistry(CRegKey& key) : m_key(key)
{
  // Get colours from the appropriate colour scheme
  char schemeName[MAX_PATH];
  ULONG schemeNameLen = sizeof schemeName;
  if (GetString("Colour Scheme",schemeName,schemeNameLen))
  {
    CString schemeKeyName;
    schemeKeyName.Format("%s\\Colour Schemes\\%s",REGISTRY_INFORM,schemeName);
    CRegKey schemeKey;
    if (schemeKey.Open(HKEY_CURRENT_USER,schemeKeyName,KEY_READ) == ERROR_SUCCESS)
    {
      const char* colourNames[] =
      {
        "Source Paper Colour",
        "Ext Paper Colour",
        "Headings Colour",
        "Main Text Colour",
        "Comments Colour",
        "Quoted Text Colour",
        "Substitutions Colour"
      };
      for (auto colourName : colourNames)
      {
        DWORD value = 0;
        if (schemeKey.QueryDWORDValue(colourName,value) == ERROR_SUCCESS)
          m_colours[colourName] = value;
      }
    }
  }
}

bool SourceSettingsRegistry::GetDWord(const char* name, DWORD& value)
{
  if (m_colours.count(name) > 0)
  {
    value = m_colours[name];
    return true;
  }
  return (m_key.QueryDWORDValue(name,value) == ERROR_SUCCESS);
}

bool SourceSettingsRegistry::GetString(const char* name, char* value, ULONG len)
{
  return (m_key.QueryStringValue(name,value,&len) == ERROR_SUCCESS);
}
