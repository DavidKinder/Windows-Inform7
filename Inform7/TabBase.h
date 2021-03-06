#pragma once

#include "TabInterface.h"
#include "FlatButton.h"

class TabBase : public CWnd, public TabInterface
{
  DECLARE_DYNAMIC(TabBase)

public:
  void MakeInactive(void);
  bool IsEnabled(void);
  CWnd* GetWindow(void);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  void CompileProject(CompileStage stage, int code);
  bool IsProjectEdited(void);
  void Progress(const char* msg);
  void LoadSettings(CRegKey& key, bool primary);
  void SaveSettings(CRegKey& key, bool primary);
  void PrefsChanged(CRegKey& key);
  void UpdateDPI(void);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg void OnBackward();
  afx_msg void OnForward();
  afx_msg BOOL OnToolTipText(UINT, NMHDR*, LRESULT*);
  afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM, LPARAM);

  void Create(CWnd* parent);
  void SizeTab(CRect& client, CSize& fontSize, int& heading);

  virtual CString GetToolTip(UINT_PTR id);

private:
  FlatButton m_navigate[2];
};
