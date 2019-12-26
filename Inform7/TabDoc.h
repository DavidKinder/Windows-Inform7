#pragma once

#include "TabBase.h"
#include "ReportHtml.h"
#include "SearchWindow.h"
#include "FlatTab.h"

class TabDoc : public TabBase, public SearchWindow::Source
{
  DECLARE_DYNAMIC(TabDoc)

public:
  TabDoc();

  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void CompileProject(CompileStage stage, int code);
  void PrefsChanged(CRegKey& key);

  void Show(const char* url);
  void SetFocusFlag(bool set);

  // Implementation of SearchWindow::Source
  void Search(LPCWSTR text, std::vector<SearchWindow::Result>& results);
  void Highlight(const SearchWindow::Result& result);
  CString Description(void);
  
  static void InitInstance(void);
  static void ExitInstance(void);

protected:
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnUserNavigate(WPARAM, LPARAM);
  afx_msg LRESULT OnFindReplaceCmd(WPARAM, LPARAM);

private:
  enum DocTabs
  {
    DocTab_Home = 0,
    DocTab_Examples,
    DocTab_Index,
    Number_DocTabs,
    No_DocTab = -1
  };

  static const char* m_files[TabDoc::Number_DocTabs];

  DocTabs GetActiveTab(void);
  void UpdateActiveTab(void);
  void GetTabState(TabState& state);
  void ShowFile(DocTabs tab);

  FlatTab m_tab;
  ReportHtml m_html;
  bool m_initialised;

  struct DocText
  {
    DocText();
    void AddToBody(WCHAR ch);

    CString section;
    CString title;
    CString sort;
    CStringW body;
    CString file;
    int colourScheme;
  };

  static void DecodeHTML(const char* filename, int scheme);
  static UINT BackgroundDecodeThread(LPVOID);

  struct DocData
  {
    CCriticalSection lock;
    bool done;
    CArray<TabDoc::DocText*> texts;

    DocData() : done(false)
    {
    }
  };

  static DocData* m_data;
  static CWinThread* m_pThread;
};
