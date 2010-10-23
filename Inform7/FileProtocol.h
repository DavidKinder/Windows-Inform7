#pragma once

#include <map>
#include <string>
#include <vector>

class FileProtocol : public CCmdTarget
{
public:
  ~FileProtocol();

  void Install(const wchar_t* protocol);
  void AddDirectory(const char* dir);
  void AddDirectory(const wchar_t* prefix, const char* dir);

  CStringW TranslateUrl(const wchar_t* url);

  DECLARE_INTERFACE_MAP()

  BEGIN_INTERFACE_PART(InternetProtocolInfo, IInternetProtocolInfo)
    STDMETHOD(ParseUrl) (LPCWSTR, PARSEACTION, DWORD, LPWSTR, DWORD, DWORD*, DWORD);
    STDMETHOD(CombineUrl) (LPCWSTR, LPCWSTR, DWORD, LPWSTR, DWORD, DWORD*, DWORD);
    STDMETHOD(CompareUrl) (LPCWSTR, LPCWSTR, DWORD);
    STDMETHOD(QueryInfo) (LPCWSTR, QUERYOPTION, DWORD, LPVOID, DWORD, DWORD*, DWORD);
  END_INTERFACE_PART(InternetProtocolInfo)

  BEGIN_INTERFACE_PART(ClassFactory, IClassFactory)
    STDMETHOD(CreateInstance)(LPUNKNOWN, REFIID, LPVOID*);
    STDMETHOD(LockServer)(BOOL);
  END_INTERFACE_PART(ClassFactory)

private:
  CStringW Unescape(const wchar_t* input);

  CStringW m_protocol;
  std::vector<std::string> m_dirs;
  std::map<std::wstring,std::vector<std::string> > m_prefixDirs;

  CComPtr<IInternetSession> m_session;
};
