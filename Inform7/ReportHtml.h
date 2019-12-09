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
  void RunJavaScript(const char* code);

  void SetFocusOnContent(void);
  void SetFocusFlag(bool focus);

  bool OnBeforeBrowse(const char* url, bool user);
  void OnLoadEnd(void);
  void OnLoadError(const char* url);

  class LinkConsumer
  {
  public:
    virtual void SourceLink(const char* url) = 0;
    virtual void LibraryLink(const char* url) = 0;
    virtual void SkeinLink(const char* url) = 0;
    virtual bool DocLink(const char* url) = 0;
    virtual void LinkError(const char* url) = 0;
    virtual void LinkDone(void) = 0;
  };

  void SetLinkConsumer(LinkConsumer* consumer);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnEditCopy();
  afx_msg void OnEditSelectAll();

private:
  struct Private;
  Private* m_private;

  CString m_url;
  bool m_setFocus;
  LinkConsumer* m_consumer;

///////////////////////////////////////////////////////////////////////////////

/*
protected:
  afx_msg void OnEditFind();
  afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
  void HighlightFound(bool goToFound);

  CStringW m_find;
  int m_findTimer;
*/
};
