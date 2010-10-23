#pragma once

#include "TabBase.h"
#include "Skein.h"
#include "SkeinWindow.h"
#include "ArrowButton.h"

class TabSkein : public TabBase
{
  DECLARE_DYNAMIC(TabSkein)

public:
  TabSkein();

  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  bool IsProjectEdited(void);

  void SetSkein(Skein* skein);
  void ShowNode(Skein::Node* node, Skein::Show why);

  CString GetToolTip(UINT_PTR id);

private:
  Skein* m_skein;
  SkeinWindow m_window;

  ArrowButton m_label;
  CButton m_trim, m_play;

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM, LPARAM);

  afx_msg void OnSkeinLabel();
  afx_msg void OnSkeinTrim();
  afx_msg void OnSkeinPlay();
};
