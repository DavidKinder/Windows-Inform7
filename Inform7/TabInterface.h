#pragma once

// Structure describing the tab state for the history
struct TabState
{
  int tab;
  int section;
  CString url;

  TabState()
  {
    // Equivalent to Panel::No_Tab
    tab = -1;
    section = -1;
  }

  bool operator==(const TabState& ts) const
  {
    if (tab != ts.tab)
      return false;
    if (section != ts.section)
      return false;
    if (url != ts.url)
      return false;
    return true;
  }
};

// Interface for implementations of each tab
class TabInterface
{
public:
  virtual ~TabInterface() {};

  // Get the name of the tab
  virtual const char* GetName(void) = 0;
  // Create the tab
  virtual void CreateTab(CWnd* parent) = 0;
  // Resize the tab
  virtual void MoveTab(CRect& rect) = 0;
  // Make this tab active
  virtual void MakeActive(TabState& state) = 0;
  // Make this tab inactive
  virtual void MakeInactive(void) = 0;
  // Check if the tab is enabled
  virtual bool IsEnabled(void) = 0;
  // Get the colour for the tab highlight
  virtual COLORREF GetTabColour(void) = 0;

  // Command routing
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) = 0;

  enum CompileStage
  {
    CompileStart,
    RanNaturalInform,
    RanInform6,
    RanCBlorb,
  };

  // Notification that a project is being opened
  virtual void OpenProject(const char* path, bool primary) = 0;
  // Notification that a project is being saved
  virtual bool SaveProject(const char* path, bool primary) = 0;
  // Notification that a project is being compiled
  virtual void CompileProject(CompileStage stage, int code) = 0;
  // Determine if a project has been edited since it was last saved
  virtual bool IsProjectEdited(void) = 0;
  // Notification of a progress message
  virtual void Progress(const char* msg) = 0;
  // Load settings from the registry
  virtual void LoadSettings(CRegKey& key) = 0;
  // Save settings to the registry
  virtual void SaveSettings(CRegKey& key) = 0;

  class LinkNotify
  {
  public:
    virtual void OnSourceLink(const char* url, TabInterface* from, COLORREF highlight) = 0;
    virtual void OnDocLink(const wchar_t* url, TabInterface* from) = 0;
  };
};
