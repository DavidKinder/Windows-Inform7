#pragma once

#include "GameWindow.h"
#include "Panel.h"
#include "Skein.h"
#include "TabSettings.h"
#include "TabSource.h"
#include "Inform.h"
#include "ProjectSettings.h"
#include "FlatSplitter.h"
#include "SearchBar.h"
#include "SearchWindow.h"
#include "MenuBar.h"

#include <queue>

class ProjectFrame :
  public MenuBarFrameWnd,
  public InformApp::OutputSink,
  public TabInterface::LinkNotify,
  public TabSettings::SettingsTabNotify
{
protected:
  DECLARE_DYNAMIC(ProjectFrame)

public:
  ProjectFrame();

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
  virtual void GetMessageString(UINT nID, CString& rMessage) const;

protected:
  CToolBar m_toolBar;
  CToolBar m_helpBar;
  SearchBar m_searchBar;
  CStatusBar m_statusBar;
  CProgressCtrl m_progress;
  FlatSplitter m_splitter;

  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnDestroy();
  afx_msg void OnClose();
  afx_msg void OnActivate(UINT nState, CWnd*, BOOL);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT mi);
  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di);
  afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

  afx_msg LRESULT OnPlaySkein(WPARAM, LPARAM);
  afx_msg LRESULT OnGameRunning(WPARAM, LPARAM);
  afx_msg LRESULT OnGameWaiting(WPARAM, LPARAM);
  afx_msg LRESULT OnPaneHeading(WPARAM, LPARAM);
  afx_msg LRESULT OnSelectSide(WPARAM, LPARAM);
  afx_msg LRESULT OnSelectView(WPARAM, LPARAM);
  afx_msg LRESULT OnPasteCode(WPARAM, LPARAM);
  afx_msg LRESULT OnRuntimeProblem(WPARAM, LPARAM);
  afx_msg LRESULT OnSearchSource(WPARAM, LPARAM);
  afx_msg LRESULT OnSearchDoc(WPARAM, LPARAM);
  afx_msg LRESULT OnShowTranscript(WPARAM, LPARAM);
  afx_msg LRESULT OnShowSkein(WPARAM, LPARAM);
  afx_msg LRESULT OnTerpFailed(WPARAM, LPARAM);
  afx_msg LRESULT OnProjectDir(WPARAM, LPARAM);
  afx_msg LRESULT OnTranscriptEnd(WPARAM, LPARAM);
  afx_msg LRESULT OnPlayNextThread(WPARAM, LPARAM);
  afx_msg LRESULT OnCanPlayAll(WPARAM, LPARAM);
  afx_msg LRESULT OnProjectEdited(WPARAM wparam, LPARAM lparam);

  afx_msg void OnFileNew();
  afx_msg void OnFileOpen();
  afx_msg void OnFileInstallExt();
  afx_msg void OnFileNewExt();
  afx_msg void OnFileOpenExt(UINT nID);
  afx_msg void OnFileClose();
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();
  afx_msg void OnFileImportSkein();

  afx_msg void OnUpdateCompile(CCmdUI *pCmdUI);
  afx_msg void OnPlayGo();
  afx_msg void OnPlayTest();
  afx_msg void OnUpdatePlayStop(CCmdUI *pCmdUI);
  afx_msg void OnPlayStop();
  afx_msg void OnPlayRefresh();
  afx_msg void OnPlayLoad();

  afx_msg void OnReplayLast();
  afx_msg void OnUpdateReplayBlessed(CCmdUI *pCmdUI);
  afx_msg void OnReplayBlessed();
  afx_msg void OnReplayShowLast();
  afx_msg void OnReplayShowLastSkein();
  afx_msg void OnUpdateReplayChanged(CCmdUI *pCmdUI);
  afx_msg void OnReplayChanged(UINT nID);
  afx_msg void OnUpdateReplayDiffer(CCmdUI *pCmdUI);
  afx_msg void OnReplayDiffer(UINT nID);
  afx_msg void OnUpdateReplayDifferSkein(CCmdUI *pCmdUI);
  afx_msg void OnReplayDifferSkein();

  afx_msg void OnUpdateReleaseGame(CCmdUI *pCmdUI);
  afx_msg void OnReleaseGame(UINT nID);
  afx_msg void OnReleaseMaterials();
  afx_msg void OnReleaseIFiction();

  afx_msg void OnWindowLeftPane();
  afx_msg void OnWindowRightPane();
  afx_msg void OnWindowSwitchPanes();
  afx_msg void OnWindowShowTab(UINT nID);
  afx_msg void OnWindowShowIndex(UINT nID);
  afx_msg void OnUpdateWindowList(CCmdUI *pCmdUI);
  afx_msg void OnWindowList(UINT nID);

  afx_msg void OnHelpIndex();
  afx_msg void OnHelpExtensions();
  afx_msg void OnHelpRecipes();
  afx_msg void OnHelpLicence();
  afx_msg void OnHelpWindows();

  afx_msg void OnSearchDocs();
  afx_msg void OnSearchSource();

public:
  static bool StartNewProject(const char* dir, CWnd* parent);
  static bool StartExistingProject(const char* dir, CWnd* parent);
  static bool StartLastProject(void);

  CString GetDisplayName(bool showEdited);
  void SendChanged(InformApp::Changed changed, int value);

protected:
  // Implementation of InformApp::OutputSink
  void Output(const char* msg);

  // Implementation of TabSettings::SettingsTabNotify
  void OnSettingsChange(TabSettings* changed);

  // Implementation of TabInterface::LinkTabNotify
  void OnSourceLink(const char* url, TabInterface* from, COLORREF highlight);
  void OnDocLink(const wchar_t* url, TabInterface* from);

  static ProjectFrame* NewFrame(void);
  void SetFromRegistryPath(const char* path);
  void SaveSettings(void);

  void OpenProject(const char* project);
  bool SaveProject(const char* project);
  bool CompileProject(bool release);
  void RunProject(void);
  void CleanProject(void);
  bool IsProjectEdited(void);

  void UpdateMenuParams(void);
  void UpdateExtensionsMenu(void);

  CString NaturalCommandLine(bool release);
  CString InformCommandLine(bool release);

  Panel* GetPanel(int column) const;
  int ChoosePanel(Panel::Tabs newTab);

  bool LoadToolBar(void);
  bool LoadHelpBar(void);
  bool SetToolbarText(CToolBar& toolbar);

  void WatchSourceDir(void);
  bool CheckSourceDir(void);

  CRect GetInitialSearchRect(void);

  enum SkeinAction
  {
    PlaySkeinThread,
    ShowFirstSkeinError
  };

  struct PlaySkein
  {
    SkeinAction action;
    Skein::Node* node;
  };

  GameWindow m_game;
  Skein m_skein;
  std::queue<PlaySkein> m_playThreads;

  SearchWindow m_search;
  HWND m_focus;

  CRegKey m_registryKey;
  CString m_projectDir;
  HANDLE m_watchSourceDir;

  ProjectSettings m_settings;
  bool m_compiling;

  CString m_outputFileLoc;

  CFont m_menuFonts[2];
  int m_menuGutter;
  CSize m_menuTextGap;
};
