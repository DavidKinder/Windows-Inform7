#pragma once

#include <map>
#include <string>

class SourceSettings : public CObject
{
public:
  virtual bool GetDWord(const char* name, DWORD& value) = 0;
  virtual bool GetString(const char* name, char* value, ULONG len) = 0;
};

class SourceSettingsRegistry : public SourceSettings
{
public:
  SourceSettingsRegistry(CRegKey& key, CWnd* wnd);
  virtual bool GetDWord(const char* name, DWORD& value);
  virtual bool GetString(const char* name, char* value, ULONG len);

private:
  CRegKey& m_key;
  std::map<std::string,DWORD> m_colours;
};
