#pragma once

#include "FindInFiles.h"
#include "FlatSplitter.h"
#include "GameWindow.h"
#include "Inform.h"
#include "Panel.h"
#include "ProjectSettings.h"
#include "ProgressWnd.h"
#include "SearchBar.h"
#include "Skein.h"
#include "TabSettings.h"
#include "TabSource.h"

#include "MenuBar.h"

#include <queue>

class ExampleComboBox : public DarkModeComboBox
{
public:
  ExampleComboBox();
  virtual HRESULT get_accName(VARIANT child, BSTR* name);
};

class ProjectFrame :
  public MenuBarFrameWnd,
  public InformApp::OutputSink,
  public TabInterface::LinkNotify,
  public TabSettings::SettingsTabNotify
{
protected:
  DECLARE_DYNAMIC(ProjectFrame)

public:
  ProjectFrame(ProjectType projectType);

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

  virtual void SetDarkMode(DarkMode* dark);

protected:
  int m_toolBarSize;
  DarkModeToolBar m_toolBar;
  SearchBar m_searchBar;
  ExampleComboBox m_exampleList;
  ProgressWnd m_progress;
  FlatSplitter m_splitter;

  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnDestroy();
  afx_msg void OnClose();
  afx_msg void OnActivate(UINT nState, CWnd*, BOOL);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnChangedExample();
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

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
  afx_msg LRESULT OnShowSkein(WPARAM, LPARAM);
  afx_msg LRESULT OnAnimateSkein(WPARAM, LPARAM);
  afx_msg LRESULT OnTerpFailed(WPARAM, LPARAM);
  afx_msg LRESULT OnProjectDir(WPARAM, LPARAM);
  afx_msg LRESULT OnPlayNextThread(WPARAM, LPARAM);
  afx_msg LRESULT OnCanPlayAll(WPARAM, LPARAM);
  afx_msg LRESULT OnProjectEdited(WPARAM, LPARAM);
  afx_msg LRESULT OnExtDownload(WPARAM, LPARAM);
  afx_msg LRESULT OnProgress(WPARAM, LPARAM);
  afx_msg LRESULT OnCreateNewProject(WPARAM, LPARAM);
  afx_msg LRESULT OnProjectExt(WPARAM, LPARAM);
  afx_msg LRESULT OnProjectType(WPARAM, LPARAM);
  afx_msg LRESULT OnStoryActive(WPARAM, LPARAM);
  afx_msg LRESULT OnWantStop(WPARAM, LPARAM);
  afx_msg LRESULT OnRunCensus(WPARAM, LPARAM);
  afx_msg LRESULT OnStoryName(WPARAM, LPARAM);
  afx_msg LRESULT OnReplayAll(WPARAM, LPARAM);
  afx_msg LRESULT OnTestingTabShown(WPARAM, LPARAM);
  afx_msg LRESULT OnIsBuildFile(WPARAM, LPARAM);

  afx_msg void OnFileNew();
  afx_msg void OnFileOpen();
  afx_msg void OnFileInstallExt();
  afx_msg void OnFileInstallFolder();
  afx_msg void OnFileInstallExtProject();
  afx_msg void OnFileNewExt();
  afx_msg void OnFileNewExtProject();
  afx_msg void OnFileOpenExt(UINT nID);
  afx_msg void OnFileNewXPFromExt(UINT nID);
  afx_msg void OnFileClose();
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();
  afx_msg void OnFileImportSkein();
  afx_msg void OnFileExportExtProject();

  afx_msg void OnEditFindInFiles();

  afx_msg void OnUpdateIfNotBusy(CCmdUI *pCmdUI);
  afx_msg void OnUpdateCompile(CCmdUI *pCmdUI);
  afx_msg void OnPlayGo();
  afx_msg void OnPlayTest();
  afx_msg void OnPlayRefresh();
  afx_msg void OnPlayLoad();
  afx_msg void OnPlayReplay();

  afx_msg void OnUpdateReleaseGame(CCmdUI *pCmdUI);
  afx_msg void OnReleaseGame(UINT nID);
  afx_msg void OnReleaseMaterials();
  afx_msg void OnReleaseIFiction();

  afx_msg void OnWindowLeftPane();
  afx_msg void OnWindowRightPane();
  afx_msg void OnWindowSwitchPanes();
  afx_msg void OnWindowShowTab(UINT nID);
  afx_msg void OnUpdateWindowShowTab(CCmdUI *pCmdUI);
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
  static bool StartNewExtProject(const char* dir, CWnd* parent, const InformApp::ExtLocation* fromExt);
  static bool StartExistingProject(const char* dir, CWnd* parent);
  static bool StartNamedProject(const char* project);
  static ProjectType TypeFromDir(const CString& projectDir);

  CString GetDisplayName(bool fullName);
  void SendChanged(InformApp::Changed changed, int value);

  CString GetSource(void);
  void SelectInSource(const CHARRANGE& range);
  void SelectInDocumentation(const char* link, LPCWSTR find);

  const ProjectSettings& GetSettings(void);

  enum ProcessAction
  {
    ProcessNoAction,
    ProcessHelpExtensions
  };
  void MonitorProcess(InformApp::CreatedProcess cp, ProcessAction action, LPCSTR name);
  bool IsProcessRunning(LPCSTR name);

protected:
  // Implementation of InformApp::OutputSink
  void Output(const char* msg);
  bool WantStop(void);

  // Implementation of TabSettings::SettingsTabNotify
  void OnSettingsChange(TabSettings* changed);

  // Implementation of TabInterface::LinkTabNotify
  void OnSourceLink(const char* url, TabInterface* from, COLORREF highlight);
  void OnDocLink(const char* url, TabInterface* from);
  void OnSkeinLink(const char* url, TabInterface* from);

  static ProjectFrame* NewFrame(ProjectType projectType);
  void SetFromRegistryPath(const char* path);
  void SaveSettings(void);

  void OpenProject(const char* project);
  bool SaveProject(const char* project);
  bool CompileProject(bool release, bool test, bool force);
  void RunProject(void);
  void CleanProject(void);
  bool IsProjectEdited(void);
  const char* GetProjectFileExt(void);

  void UpdateExtensionsMenu(void);
  bool UpdateExampleList(void);
  void SetExampleListLocation(void);
  bool GetExtensionInfo(CString& path, CStringW& name, CStringW& author);
  CString GetMaterialsFolder(void);
  bool CopyExtensionToMaterials(void);

  struct Example
  {
    char id;
    int index;
    CString name;

    Example() : id(0), index(0)
    {
    }
  };
  Example GetCurrentExample(void);
  void TestCurrentExample(bool testAll);

  CString Inform7CommandLine(bool release);
  CString Inform6CommandLine(bool release);
  CString IntestSourceCommandLine(void);
  void GenerateIntestReport(CString result);
  void GenerateIntestCombinedReport(void);
  bool BusyWantStop(void);
  LONGLONG GetMaxLast5Seconds(void);

  Panel* GetPanel(int column) const;
  int ChoosePanel(Panel::Tabs newTab);

  bool LoadToolBar(void);
  void UpdateToolBarFont(void);
  void UpdateToolBarImages(void);

  enum SkeinAction
  {
    PlaySkeinThread,
    ShowFirstSkeinError,
    ShowTestReport,
    RunNextTest,
    ReportThenRunNextTest
  };

  struct PlaySkein
  {
    SkeinAction action;
    Skein::Node* node;

    PlaySkein(SkeinAction sa) : action(sa), node(NULL)
    {
    }
  };

  GameWindow m_game;
  Skein m_skein;
  std::queue<PlaySkein> m_playThreads;

  FindInFiles m_finder;
  HWND m_focus;

  const ProjectType m_projectType;
  CString m_projectDir;
  CRegKey m_registryKey;
  bool m_needCompile;
  CTime m_startTime;
  CTime m_last5StartTime;

  ProjectSettings m_settings;
  bool m_busy;
  bool m_I6debug;

  CString m_outputFileLoc;
  int m_loadFilter;

  struct SubProcess
  {
    SubProcess() : action(ProcessNoAction)
    {
    }

    InformApp::CreatedProcess cp;
    ProcessAction action;
    CString name;
  };
  CArray<SubProcess> m_processes;

  CArray<Example> m_examples;
  Example m_exampleCompiled;
  CString m_materialsExtPath;

  struct ExLineOffset
  {
    char id;
    int from;
    int offset;

    ExLineOffset() : id(0), from(0), offset(0)
    {
    }
    ExLineOffset(char i, int f, int o) : id(i), from(f), offset(o)
    {
    }
  };
  CArray<ExLineOffset> m_exLineOffsets;

  class BusyProject
  {
  public:
    BusyProject(ProjectFrame* frame) : m_frame(frame)
    {
      frame->m_busy = true;
    }

    ~BusyProject()
    {
      m_frame->m_busy = false;
    }

  private:
    ProjectFrame* m_frame;
  };
  friend class BusyProject;
};
