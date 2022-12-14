#pragma once

class SourceSettings : public CObject
{
public:
  virtual bool GetDWord(const char* name, DWORD& value) = 0;
  virtual bool GetString(const char* name, char* value, ULONG len) = 0;
};

class SourceSettingsRegistry : public SourceSettings
{
public:
  SourceSettingsRegistry(CRegKey& key);
  virtual bool GetDWord(const char* name, DWORD& value);
  virtual bool GetString(const char* name, char* value, ULONG len);

private:
  CRegKey& m_key;
};
