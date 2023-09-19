#pragma once

#include "PageTab.h"
#include "ReportEdit.h"
#include "ReportHtml.h"
#include "TabBase.h"

class TabResults : public TabBase, public ReportHtml::LinkConsumer
{
  DECLARE_DYNAMIC(TabResults)

public:
  TabResults();

  // Implementation of TabInterface
  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  void CompileProject(CompileStage stage, int code);
  void Progress(const char* msg);
  void PrefsChanged(CRegKey& key);
  void UpdateDPI(const std::map<CWnd*,double>& layout);
  void SetDarkMode(DarkMode* dark);

  // Implementation of ReportHtml::LinkConsumer
  void SourceLink(const char* url);
  void LibraryLink(const char* url);
  void SkeinLink(const char* url);
  bool DocLink(const char* url);
  void LinkError(const char* url);
  void LinkDone(void);

  void SetLinkNotify(LinkNotify* notify);
  void ShowPage(LPCSTR path);

private:
  void Resize(void);

  enum ResultTabs
  {
    ResTab_Report = 0,
    ResTab_Console,
    Number_ResTabs,
    No_ResTab = -1
  };

  ResultTabs GetActiveTab(void);
  void SetActiveTab(ResultTabs tab, bool focus);
  void GetTabState(TabState& state);

  void SetFocusOnContent(void);

  PageTab m_tab;
  ReportHtml m_report;
  ReportEdit m_console;

  CString m_projectDir;
  LinkNotify* m_notify;

  enum I6FatalError
  {
    NoError,
    MemorySetting,
    ReadableMemory,
    StoryFileLimit,
  };

  I6FatalError m_inform6;

protected:
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnUserNavigate(WPARAM, LPARAM);
  afx_msg LRESULT OnFindReplaceCmd(WPARAM, LPARAM);
};
