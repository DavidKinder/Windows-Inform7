#pragma once

#include "TabBase.h"
#include "ReportHtml.h"
#include "FlatTab.h"
#include "ReportEdit.h"

class TabErrors : public TabBase, public ReportHtml::LinkConsumer
{
  DECLARE_DYNAMIC(TabErrors)

public:
  TabErrors();

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

  // Implementation of ReportHtml::LinkConsumer
  void SourceLink(const char* url);
  void DocLink(const wchar_t* url);

  void SetLinkNotify(LinkNotify* notify);
  int GetTabHeight(void);

  void ShowRuntimeProblem(int problem);
  void ShowTerpFailed(void);

private:
  enum ErrorTabs
  {
    ErrTab_Progress = 0,
    ErrTab_Problems,
    Number_ErrTabs,
    No_ErrTab = -1
  };

  ErrorTabs GetActiveTab(void);
  void SetActiveTab(ErrorTabs tab, bool focus);
  void GetTabState(TabState& state);

  void SetFocusOnContent(void);

  FlatTab m_tab;
  ReportEdit m_progress;
  ReportHtml* m_problems;

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
