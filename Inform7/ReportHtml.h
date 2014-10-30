#pragma once

class ReportHtml;

class ScriptExternal : public CCmdTarget
{
public:
  ScriptExternal(ReportHtml* html);
  LPUNKNOWN GetProject(void);
  void SetProject(LPUNKNOWN);

private:
  DECLARE_DISPATCH_MAP()
  ReportHtml* m_html;
};

class ScriptProject : public CCmdTarget
{
public:
  ScriptProject(ReportHtml* html);
  void SelectView(LPCSTR view);
  void PasteCode(LPCWSTR code);
  void CreateNewProject(LPCWSTR title, LPCWSTR code);
  void OpenFile(LPCWSTR path);
  void OpenUrl(LPCWSTR url);
  BSTR ExtCompareVersion(LPCWSTR author, LPCWSTR title, LPCWSTR compare);
  BSTR ExtGetVersion(LPCWSTR author, LPCWSTR title);
  void ExtDownload(VARIANT& extArray);

private:
  DECLARE_DISPATCH_MAP()
  ReportHtml* m_html;
};

class ReportHtml : public CHtmlView
{
  DECLARE_DYNCREATE(ReportHtml)

public:
  class LinkConsumer
  {
  public:
    virtual void SourceLink(const char* url) = 0;
    virtual void LibraryLink(const char* url) = 0;
    virtual bool DocLink(const wchar_t* url) = 0;
    virtual bool LinkError(const char* url) = 0;
  };

  class PageRewriter
  {
  public:
    virtual void ModifyPage(const char* url, IHTMLDocument2* doc) = 0;
  };

  static void SetIEPreferences(const char* path);

  void SetLinkConsumer(LinkConsumer* consumer);
  void SetPageRewriter(PageRewriter* rewriter);
  void SetFocusOnContent(void);
  void SetFocusFlag(bool focus);
  void Navigate(const char* url, bool focus, const wchar_t* find = NULL);
  CString GetURL(void);
  void Invoke(LPCWSTR method, VARIANT* arg);

protected:
  ReportHtml();

  virtual HRESULT OnGetOptionKeyPath(LPOLESTR* pchKey, DWORD dwReserved);
  virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
  virtual void OnNavigateError(LPCTSTR lpszURL, LPCTSTR lpszFrame, DWORD dwError, BOOL *pbCancel);
  virtual void OnDocumentComplete(LPCTSTR lpszURL);
  virtual void OnStatusTextChange(LPCTSTR lpszText);
  virtual HRESULT OnShowContextMenu(DWORD dwID,  LPPOINT ppt, LPUNKNOWN pcmdTarget, LPDISPATCH);
  virtual HRESULT OnGetExternal(LPDISPATCH *lppDispatch);
  virtual HRESULT OnTranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnEditSelectAll();
  afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
  afx_msg void OnEditFind();
  afx_msg void OnTimer(UINT nIDEvent);

private:
  void HighlightFound(bool goToFound);

  static LONG WINAPI HookRegOpenKeyExW(
    HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);

  static CString m_registryPath;
  LinkConsumer* m_consumer;
  PageRewriter* m_rewriter;

  CString m_url;
  bool m_setFocus;
  bool m_notify;

  CStringW m_find;
  int m_findTimer;

  ScriptExternal m_scriptExternal;
  ScriptProject m_scriptProject;

  friend class ScriptExternal;
  friend class ScriptProject;
};
