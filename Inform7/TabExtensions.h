#pragma once

#include "TabBase.h"
#include "ReportHtml.h"
#include "FlatTab.h"

class TabExtensions : public TabBase, public ReportHtml::LinkConsumer
{
  DECLARE_DYNAMIC(TabExtensions)

public:
  TabExtensions();

  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void CompileProject(CompileStage stage, int code);
  void PrefsChanged(CRegKey& key);
  void UpdateDPI(void);

  void Show(const char* url);
  void DownloadedExt(int id);

  // Implementation of ReportHtml::LinkConsumer
  void SourceLink(const char* url);
  void LibraryLink(const char* url);
  void SkeinLink(const char* url);
  bool DocLink(const char* url);
  void LinkError(const char* url);
  void LinkDone(void);

  void SetLinkNotify(LinkNotify* notify);

protected:
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnPubLibError(WPARAM, LPARAM);
  afx_msg LRESULT OnUserNavigate(WPARAM, LPARAM);
  afx_msg LRESULT OnFindReplaceCmd(WPARAM, LPARAM);

private:
  void Resize(void);

  enum ExtTabs
  {
    ExtTab_Home = 0,
    ExtTab_Definitions,
    ExtTab_Library,
    Number_ExtTabs,
    No_ExtTab = -1
  };

  static const char* m_files[TabExtensions::Number_ExtTabs];

  ExtTabs GetActiveTab(void);
  void UpdateActiveTab(void);
  void GetTabState(TabState& state);
  CString GetUrlForTab(ExtTabs tab);

  FlatTab m_tab;
  ReportHtml m_html;
  bool m_initialised;
  LinkNotify* m_notify;
};
