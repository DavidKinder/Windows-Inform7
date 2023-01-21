#include "stdafx.h"
#include "SourceSettings.h"
#include "ColourScheme.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SourceSettingsRegistry::SourceSettingsRegistry(CRegKey& key, CWnd* wnd) : m_key(key)
{
  // Get the colour scheme name
  char schemeName[MAX_PATH] = { 0 };
  ULONG schemeNameLen = sizeof schemeName;
  GetString("Colour Scheme",schemeName,schemeNameLen);

  // First get an appropriate set of default colours
  auto schemeIt = ColourScheme::DefaultSchemes.find(schemeName);
  if (schemeIt == ColourScheme::DefaultSchemes.end())
    schemeIt = ColourScheme::DefaultSchemes.find(ColourScheme::GetDefaultSchemeName(wnd));
  ASSERT(schemeIt != ColourScheme::DefaultSchemes.end());
  if (schemeIt != ColourScheme::DefaultSchemes.end())
  {
    m_colours["Source Paper Colour"] = schemeIt->second.source;
    m_colours["Ext Paper Colour"] = schemeIt->second.ext;
    m_colours["Headings Colour"] = schemeIt->second.head;
    m_colours["Main Text Colour"] = schemeIt->second.main;
    m_colours["Comments Colour"] = schemeIt->second.comment;
    m_colours["Quoted Text Colour"] = schemeIt->second.quote;
    m_colours["Substitutions Colour"] = schemeIt->second.subst;
  }

  // Then get the specified colours for the colour scheme
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
