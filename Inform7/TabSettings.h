#pragma once

#include "TabInterface.h"
#include "ProjectSettings.h"

class TabSettings : public CFormView, public TabInterface
{
  DECLARE_DYNAMIC(TabSettings)

public:
  TabSettings();

  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);
  void MakeInactive(void);
  bool IsEnabled(void);
  CWnd* GetWindow(void);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  void CompileProject(CompileStage stage, int code);
  bool IsProjectEdited(void);
  void Progress(const char* msg);
  void LoadSettings(CRegKey& key, bool primary);
  void SaveSettings(CRegKey& key, bool primary);
  void PrefsChanged(CRegKey& key);
  void BeforeUpdateDPI(std::map<CWnd*,double>& layout);
  void UpdateDPI(const std::map<CWnd*,double>& layout);

  void SetSettings(ProjectSettings* settings);
  void UpdateSettings(void);
  void UpdateFromSettings(void);

  class SettingsTabNotify
  {
  public:
    virtual void OnSettingsChange(TabSettings* changed) = 0;
  };

  void SetNotify(SettingsTabNotify* notify);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnChangedVersion();

  virtual void PostNcDestroy();
  virtual void OnInitialUpdate();
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  virtual void OnDraw(CDC* pDC);

  void AddVersions(void);
  void Layout(void);

  BOOL Create(DWORD, const RECT&, CWnd*, UINT);
  BOOL CreateDlg(LPCTSTR, CWnd*);
  BOOL CreateDlgIndirect(LPCDLGTEMPLATE, CWnd*, HINSTANCE);

private:
  CButton m_outputZ8, m_outputGlulx;
  CButton m_blorb, m_predictable;
  CComboBox m_version;
  CFont m_labelFont;

  ProjectSettings* m_settings;
  SettingsTabNotify* m_notify;

  static CString m_labelTexts[4];
  CRect m_labelRects[4];
};
