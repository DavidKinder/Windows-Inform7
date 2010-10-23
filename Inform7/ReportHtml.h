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
  void OpenFile(LPCWSTR path);
  void OpenUrl(LPCWSTR url);

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
    virtual void DocLink(const wchar_t* url) = 0;
  };

  static void SetRegistryPath(const char* path);
  void SetLinkConsumer(LinkConsumer* consumer);
  void SetFocusOnContent(void);
  void SetFocusFlag(bool focus);
  void Navigate(const char* url, bool focus, const wchar_t* find = NULL);
  CString GetURL(void);

protected:
  ReportHtml();

  virtual HRESULT OnGetOptionKeyPath(LPOLESTR* pchKey, DWORD dwReserved);
  virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
  virtual void OnDocumentComplete(LPCTSTR lpszURL);
  virtual void OnStatusTextChange(LPCTSTR lpszText);
  virtual HRESULT OnShowContextMenu(DWORD dwID,  LPPOINT ppt, LPUNKNOWN pcmdTarget, LPDISPATCH);
  virtual HRESULT OnGetExternal(LPDISPATCH *lppDispatch);
  virtual HRESULT OnTranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnEditSelectAll();
  afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
  afx_msg void OnEditFind();

private:
  static CString m_registryPath;
  LinkConsumer* m_consumer;

  CString m_url;
  bool m_setFocus;
  bool m_goToFound;
  CStringW m_find;
  bool m_notify;

  ScriptExternal m_scriptExternal;
  ScriptProject m_scriptProject;

  friend class ScriptExternal;
  friend class ScriptProject;
};
