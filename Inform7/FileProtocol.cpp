#include "stdafx.h"
#include "FileProtocol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const IID CLSID_CustomFileProtocol =
{ /* 7352b380-339a-40be-8d0d-6767bf57c711 */
  0x7352b380,
  0x339a,
  0x40be,
  {0x8d, 0x0d, 0x67, 0x67, 0xbf, 0x57, 0xc7, 0x11}
};

FileProtocol::~FileProtocol()
{
  if (m_session != NULL)
  {
    if (FAILED(m_session->UnregisterNameSpace(&m_xClassFactory,m_protocol)))
    {
      TRACE("Failed to unregister namespace handler");
    }
  }
}

void FileProtocol::Install(const wchar_t* protocol)
{
  m_protocol = protocol;

  // Get the internet session object
  if (FAILED(::CoInternetGetSession(0,&m_session,0)))
  {
    TRACE("Failed to get internet session object");
    return;
  }

  // Register this protocol
  if (FAILED(m_session->RegisterNameSpace(
    &m_xClassFactory,CLSID_CustomFileProtocol,m_protocol,0,NULL,0)))
  {
    TRACE("Failed to register namespace handler");
    return;
  }
}

void FileProtocol::AddDirectory(const char* dir)
{
  m_dirs.push_back(dir);
}

void FileProtocol::AddDirectory(const wchar_t* prefix, const char* dir)
{
  m_prefixDirs[prefix].push_back(dir);
}

CStringW FileProtocol::TranslateUrl(const wchar_t* url)
{
  size_t protocolLen = (size_t)m_protocol.GetLength();
  if (wcslen(url) <= protocolLen+1)
    return L"";
  if (wcsncmp(url,m_protocol,protocolLen) != 0)
    return L"";
  if (url[protocolLen] != L':')
    return L"";
  CStringW fileName = Unescape(url+protocolLen+1);

  std::map<std::wstring,std::vector<std::string> >::const_iterator it;
  for (it = m_prefixDirs.begin(); it != m_prefixDirs.end(); ++it)
  {
    size_t plen = it->first.size();
    if (wcslen(fileName) > plen)
    {
      if (wcsncmp(fileName,it->first.c_str(),plen) == 0)
      {
        for (size_t i = 0; i < it->second.size(); i++)
        {
          CString path;
          path.Format("%s%S",it->second[i].c_str(),fileName.GetString()+plen);

          if (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
          {
            CStringW url;
            url.Format(L"file://%S%s",it->second[i].c_str(),fileName.GetString()+plen);
            return url;
          }
        }
      }
    }
  }

  for (size_t i = 0; i < m_dirs.size(); i++)
  {
    CString path;
    path.Format("%s%S",m_dirs[i].c_str(),fileName.GetString());
    path.TrimRight("/");

    if (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
    {
      CStringW url;
      url.Format(L"file://%S%s",m_dirs[i].c_str(),fileName.GetString());
      url.TrimRight(L"/");
      return url;
    }
  }

  return L"";
}

CStringW FileProtocol::Unescape(const wchar_t* input)
{
  int len = (int)wcslen(input);
  CStringW output;
  output.Preallocate(len);

  static const wchar_t hex[] = L"0123456789ABCDEF";

  int i = 0;
  while (i < len)
  {
    if ((input[i] == L'%') && (i < len-2))
    {
      const wchar_t* hex1 = wcschr(hex,towupper(input[i+1]));
      const wchar_t* hex2 = wcschr(hex,towupper(input[i+2]));
      if ((hex1 != NULL) && (hex2 != NULL))
      {
        wchar_t ch = (int)(((hex1-hex)<<4)+(hex2-hex));
        output.AppendChar(ch);
        i += 3;
        continue;
      }
    }
    output.AppendChar(input[i++]);
  }
  return output;
}

BEGIN_INTERFACE_MAP(FileProtocol, CCmdTarget)
  INTERFACE_PART(FileProtocol, IID_IInternetProtocolInfo, InternetProtocolInfo)
  INTERFACE_PART(FileProtocol, IID_IClassFactory, ClassFactory)
END_INTERFACE_MAP()

// IInternetProtocolInfo implementation

STDMETHODIMP_(ULONG) FileProtocol::XInternetProtocolInfo::AddRef()
{
  METHOD_PROLOGUE(FileProtocol, InternetProtocolInfo)
  return (ULONG)pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) FileProtocol::XInternetProtocolInfo::Release()
{
  METHOD_PROLOGUE(FileProtocol, InternetProtocolInfo)
  return (ULONG)pThis->InternalRelease();
}

STDMETHODIMP FileProtocol::XInternetProtocolInfo::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE(FileProtocol, InternetProtocolInfo)
  return (HRESULT)pThis->InternalQueryInterface(&iid,ppvObj);
}

STDMETHODIMP FileProtocol::XInternetProtocolInfo::ParseUrl(
  LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD dwParseFlags,
  LPWSTR pwzResult, DWORD cchResult, DWORD* pcchResult, DWORD)
{
  METHOD_PROLOGUE(FileProtocol, InternetProtocolInfo)

  CStringW result;

  if (ParseAction == PARSE_CANONICALIZE)
    result = pThis->TranslateUrl(pwzUrl);

  int len = result.GetLength();
  if (len > 0)
  {
    *pcchResult = len+1;

    if (len+1 > (int)cchResult)
      return S_FALSE;

    wcscpy(pwzResult,result);
    return S_OK;
  }

  return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP FileProtocol::XInternetProtocolInfo::CombineUrl(
  LPCWSTR, LPCWSTR, DWORD, LPWSTR, DWORD, DWORD*, DWORD)
{
  return E_NOTIMPL;
}

STDMETHODIMP FileProtocol::XInternetProtocolInfo::CompareUrl(LPCWSTR, LPCWSTR, DWORD)
{
  return E_NOTIMPL;
}

STDMETHODIMP FileProtocol::XInternetProtocolInfo::QueryInfo(
  LPCWSTR, QUERYOPTION, DWORD, LPVOID, DWORD, DWORD*, DWORD)
{
  return INET_E_QUERYOPTION_UNKNOWN;
}

// IClassFactory implementation

STDMETHODIMP_(ULONG) FileProtocol::XClassFactory::AddRef()
{
  METHOD_PROLOGUE(FileProtocol, ClassFactory)
  return (ULONG)pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) FileProtocol::XClassFactory::Release()
{
  METHOD_PROLOGUE(FileProtocol, ClassFactory)
  return (ULONG)pThis->InternalRelease();
}

STDMETHODIMP FileProtocol::XClassFactory::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE(FileProtocol, ClassFactory)
  return (HRESULT)pThis->InternalQueryInterface(&iid,ppvObj);
}

STDMETHODIMP FileProtocol::XClassFactory::CreateInstance(
  IUnknown* pUnkOuter, REFIID riid, LPVOID* ppvObject)
{
  *ppvObject = NULL;
  return E_NOINTERFACE;
}

STDMETHODIMP FileProtocol::XClassFactory::LockServer(BOOL fLock)
{
  return S_OK;
}
