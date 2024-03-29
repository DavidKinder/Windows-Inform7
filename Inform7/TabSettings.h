#pragma once

#include "FormScrollArea.h"
#include "ProjectSettings.h"
#include "TabInterface.h"

#include "DarkMode.h"

class TabSettings : public FormScrollArea, public TabInterface
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
  void SetDarkMode(DarkMode* dark);

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
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg LRESULT OnPrintClient(CDC* pDC, UINT nFlags);

  void AddVersions(void);
  void Layout(void);
  void DrawLabels(CDC& dc, const CPoint& p);

private:
  DarkModeGroupBox m_boxStory, m_boxRandom, m_boxVersion, m_boxBasic, m_boxLegacy;
  DarkModeRadioButton m_outputZ8, m_outputGlulx;
  DarkModeCheckButton m_blorb, m_predictable, m_basic, m_legacyExts;
  DarkModeComboBox m_version;
  CFont m_labelFont;

  ProjectSettings* m_settings;
  SettingsTabNotify* m_notify;

  static CString m_labelTexts[5];
  CRect m_labelRects[5];
};
