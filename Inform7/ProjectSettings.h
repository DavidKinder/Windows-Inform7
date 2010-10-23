#pragma once

#include <set>

class ProjectSettings
{
public:
  ProjectSettings();

  void Load(const char* path);
  bool Save(const char* path);

  CString GetInformSwitches(bool release);
  CString GetOutputExtension(void);

  enum Output
  {
    OutputZ5 = 5,
    OutputZ8 = 8,
    OutputGlulx = 256
  };

  Output m_output;
  bool m_blorb;
  bool m_predictable;

  int m_compiler;
  static std::set<int> m_compilers;

private:
  void SetDefaults(void);
};
