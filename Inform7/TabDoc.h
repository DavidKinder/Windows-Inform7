#pragma once

#include "TabBase.h"
#include "ReportHtml.h"
#include "FlatTab.h"

class TabDoc : public TabBase
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
  void UpdateDPI(const std::map<CWnd*,double>& layout);

  void Show(const char* url, LPCWSTR find = NULL);
  void SetFocusFlag(bool set);

protected:
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnUserNavigate(WPARAM, LPARAM);
  afx_msg LRESULT OnFindReplaceCmd(WPARAM, LPARAM);

private:
  void Resize(void);

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
};
