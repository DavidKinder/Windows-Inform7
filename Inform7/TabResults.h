#pragma once

#include "TabBase.h"
#include "ReportHtml.h"
#include "FlatTab.h"
#include "ReportEdit.h"

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

  // Implementation of ReportHtml::LinkConsumer
  void SourceLink(const char* url);
  void LibraryLink(const char* url);
  void SkeinLink(const char* url);
  bool DocLink(const wchar_t* url);
  bool LinkError(const char* url);

  void SetLinkNotify(LinkNotify* notify);
  int GetTabHeight(void);

  void ShowRuntimeProblem(int problem);
  void ShowTerpFailed(void);

private:
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

  FlatTab m_tab;
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
};
