#pragma once

class ReportHtml : public CWnd
{
  DECLARE_DYNCREATE(ReportHtml)

public:
  static bool InitWebBrowser(void);
  static void ShutWebBrowser(void);
  static void DoWebBrowserWork(void);
  static void UpdateWebBrowserPreferences(void);
  static void UpdateWebBrowserPreferences(CWnd* frame);
  static void RemoveContext(CWnd* frame);

  ReportHtml();
  ~ReportHtml();

  BOOL Create(LPCSTR, LPCSTR, DWORD style,
    const RECT& rect, CWnd* parentWnd, UINT id, CCreateContext* = NULL);

  // Navigate to the given URL, encoded as UTF-8
  void Navigate(const char* url, bool focus, LPCWSTR find = NULL);

  // Get the current URL, encoded as UTF-8
  CString GetURL(void);

  void Refresh(void);
  void RunJavaScript(const char* code);

  void SetFocusOnContent(void);
  void SetFocusFlag(bool focus);

  void Find(LPCWSTR findText, bool findNext, bool forward, bool matchCase);
  void StopFind(void);
  LRESULT OnFindReplaceCmd(WPARAM wParam, LPARAM lParam);

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
  afx_msg void OnEditFind();

private:
  struct Private;
  Private* m_private;

  CString m_url;
  bool m_setFocus;
  CStringW m_find;
  LinkConsumer* m_consumer;
};
