#pragma once

#include "TabBase.h"
#include "ReportHtml.h"
#include "FlatTab.h"

class TabIndex : public TabBase, public ReportHtml::LinkConsumer
{
  DECLARE_DYNAMIC(TabIndex)

public:
  TabIndex();

  // Implementation of TabInterface
  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  void CompileProject(CompileStage stage, int code);

  // Implementation of ReportHtml::LinkConsumer
  void SourceLink(const char* url);
  void DocLink(const wchar_t* url);

  void SetLinkNotify(LinkNotify* notify);

  void ShowIndex(int index);

private:
  enum IndexTabs
  {
    IdxTab_Contents = 0,
    IdxTab_Actions,
    IdxTab_Kinds,
    IdxTab_Phrasebook,
    IdxTab_Rules,
    IdxTab_Scenes,
    IdxTab_World,
    Number_IdxTabs,
    No_IdxTab = -1
  };

  static const char* m_files[TabIndex::Number_IdxTabs];

  IndexTabs GetActiveTab(void);
  void SetActiveTab(IndexTabs tab, bool focus);
  void GetTabState(TabState& state);

  FlatTab m_tab;
  ReportHtml* m_index;

  CString m_projectDir;
  LinkNotify* m_notify;

protected:
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnUserNavigate(WPARAM, LPARAM);
};
