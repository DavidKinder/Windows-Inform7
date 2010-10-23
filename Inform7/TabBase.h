#pragma once

#include "TabInterface.h"

class TabBase : public CWnd, public TabInterface
{
  DECLARE_DYNAMIC(TabBase)

public:
  void MakeInactive(void);
  bool IsEnabled(void);
  COLORREF GetTabColour(void);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  void CompileProject(CompileStage stage, int code);
  bool IsProjectEdited(void);
  void Progress(const char* msg);
  void LoadSettings(CRegKey& key);
  void SaveSettings(CRegKey& key);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg void OnBackward();
  afx_msg void OnForward();
  afx_msg BOOL OnToolTipText(UINT, NMHDR*, LRESULT*);
  afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM, LPARAM);

  void Create(CWnd* parent);
  void SizeTab(CRect& client, CSize& fontSize, int& heading, int& h);

  virtual CString GetToolTip(UINT_PTR id);

  CFont m_font;

private:
  CButton m_navigate[2];
};
