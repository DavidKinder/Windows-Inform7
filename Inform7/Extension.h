#pragma once

#include "Process.h"

#include <set>
#include <string>
#include <vector>

namespace Extension
{
  struct Location
  {
    std::string author;
    std::string title;
    std::string path;

    Location(const char* a, const char* t, const char* p);
    bool operator<(const Location& el) const;
  };

  typedef std::set<Location> Set;
  typedef std::vector<Location> Array;

  void GetAll(Set& extensions, const char* materialsDir);
  void Find(const char* dir, Set& extensions);
  bool UnpackZip(const CString& zipPath, const CString& destPath, CString& extPath);

  bool IsBuiltIn(const char* path);
  bool IsLegacy(const char* path);

  namespace Legacy
  {
    const Array& Get(void);
    void Install(CFrameWnd* parent);
    bool ReadFirstLine(const char* path, CStringW& name, CStringW& author, CStringW& version);

    class Census
    {
    public:
      void Run(void);
      void HandleEvent(void);
      const Extension::Array& GetExtensions(void);

    protected:
      Process m_proc;
      HANDLE m_read = INVALID_HANDLE_VALUE;
      HANDLE m_write = INVALID_HANDLE_VALUE;
      CString m_output;

      Extension::Array m_extensions;
    };
  }
};
