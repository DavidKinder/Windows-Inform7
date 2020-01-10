#pragma once

#include "TabInterface.h"
#include "FlatTab.h"

class Panel : public CWnd, public FlatTab::TabController
{
protected: 
  DECLARE_DYNCREATE(Panel)

  Panel();
  ~Panel();

public:
  enum Tabs
  {
    Tab_Source = 0,
    Tab_Results,
    Tab_Index,
    Tab_Skein,
    Tab_Transcript,
    Tab_Story,
    Tab_Doc,
    Tab_Extensions,
    Tab_Settings,
    Number_Tabs,
    No_Tab = -1
  };

protected:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void PostNcDestroy();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

public:
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);

public:
  TabInterface* GetTab(Tabs tab);
  Tabs GetActiveTab(void);
  void SetActiveTab(Tabs tab);
  bool ContainsTab(TabInterface* tab);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  void CompileProject(TabInterface::CompileStage stage, int code);
  bool IsProjectEdited(void);
  void Progress(const char* msg);
  void LoadSettings(CRegKey& key, bool primary);
  void SaveSettings(CRegKey& key, bool primary);
  void PrefsChanged(CRegKey& key);

  virtual bool IsTabEnabled(int tab);

  bool CanTabNavigate(bool forward);
  const char* TabNavigateName(bool forward);
  void TabNavigate(bool forward);

protected:
  TabState GetTabNavigate(bool forward);
  CRect GetTabSize(void);

  FlatTab m_tab;
  CFont m_tabFont;
  TabInterface* m_tabs[Number_Tabs];

  // Position in tab history, counting from the end of the array
  int m_tabHistoryPos;
  CArray<TabState> m_tabHistory;
  bool m_tabHistoryFrozen;

public:
  void AddToTabHistory(TabState state);
  static Panel* GetPanel(CWnd* wnd);
  static double GetFontScale(CWnd* wnd, CDC* dc);

  class FreezeHistory
  {
    Panel* m_panel;

  public:
    FreezeHistory(Panel* panel);
    ~FreezeHistory();
  };
  friend class FreezeHistory;
};
