#pragma once

class ReportHtml : public CWnd
{
  DECLARE_DYNCREATE(ReportHtml)

public:
  static bool InitWebBrowser(void);
  static void ShutWebBrowser(void);
  static void DoWebBrowserWork(void);
  static void UpdateWebBrowserPreferences(void);

  ReportHtml();
  ~ReportHtml();

  BOOL Create(LPCSTR, LPCSTR, DWORD style,
    const RECT& rect, CWnd* parentWnd, UINT id, CCreateContext* = NULL);

  void Navigate(const char* url, bool focus, const wchar_t* find = NULL);
  CString GetURL(void);
  void Refresh(void);

  bool OnBeforeBrowse(const char* url, bool user);

  class LinkConsumer
  {
  public:
    virtual void SourceLink(const char* url) = 0;
    virtual void LibraryLink(const char* url) = 0;
    virtual void SkeinLink(const char* url) = 0;
    virtual bool DocLink(const wchar_t* url) = 0;
    virtual bool LinkError(const char* url) = 0;
  };

  void SetLinkConsumer(LinkConsumer* consumer);

protected:
  DECLARE_MESSAGE_MAP()

private:
  struct Private;
  Private* m_private;

  CString m_url;
  LinkConsumer* m_consumer;

///////////////////////////////////////////////////////////////////////////////

public:
  class PageRewriter
  {
  public:
    virtual void ModifyPage(const char* url, IHTMLDocument2* doc) = 0;
  };

  void SetPageRewriter(PageRewriter* rewriter);
  void SetFocusOnContent(void);
  void SetFocusFlag(bool focus);
  void Invoke(LPCWSTR method, VARIANT* arg);

/*
protected:
  afx_msg void OnEditSelectAll();
  afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
  afx_msg void OnEditFind();
  afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
  void HighlightFound(bool goToFound);

  PageRewriter* m_rewriter;

  bool m_setFocus;

  CStringW m_find;
  int m_findTimer;
*/
};
