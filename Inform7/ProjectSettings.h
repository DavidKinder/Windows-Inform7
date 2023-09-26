#pragma once

#include <set>

class ProjectSettings
{
public:
  ProjectSettings();

  void Load(const char* path);
  bool Save(const char* path);
  CString GetSettingsPath(const char* path);

  CString GetInformSwitches(bool release, bool debugFile);
  CString GetOutputExtension(void);
  CString GetOutputFormat(bool release);
  CString GetCompilerVersion(void);

  enum Output
  {
    OutputZ8 = 8,
    OutputGlulx = 256
  };

  Output m_output;
  bool m_blorb;
  bool m_predictable;
  bool m_basic;
  bool m_legacyExtensions;
  CString m_compilerVersion;
  int m_testingTabShownCount;

  bool m_changed;

private:
  void SetDefaults(void);
};
