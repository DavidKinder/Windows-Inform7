#pragma once

#include <set>

class ProjectSettings
{
public:
  ProjectSettings();

  void Load(const char* path);
  bool Save(const char* path);

  CString GetInformSwitches(bool release, bool debugFile);
  CString GetOutputExtension(void);

  enum Output
  {
    OutputZ8 = 8,
    OutputGlulx = 256
  };

  Output m_output;
  bool m_blorb;
  bool m_predictable;
  bool m_elasticTabStops;

  bool m_changed;

private:
  void SetDefaults(void);
};
