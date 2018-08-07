#include "stdafx.h"
#include "SourceSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SourceSettingsRegistry::SourceSettingsRegistry(CRegKey& key) : m_key(key)
{
}

bool SourceSettingsRegistry::GetDWord(const char* name, DWORD& value)
{
  return (m_key.QueryDWORDValue(name,value) == ERROR_SUCCESS);
}

bool SourceSettingsRegistry::GetString(const char* name, char* value, ULONG len)
{
  return (m_key.QueryStringValue(name,value,&len) == ERROR_SUCCESS);
}
