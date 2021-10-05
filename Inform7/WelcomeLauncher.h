#pragma once

#include "BaseDialog.h"
#include "CommandButton.h"
#include "ReportHtml.h"
#include "Dib.h"
#include "Resource.h"

class WelcomeLauncher : public I7BaseDialog
{
  DECLARE_DYNAMIC(WelcomeLauncher)

public:
  WelcomeLauncher(CWnd* pParent = NULL);

  void ShowModalLauncher(void);
  void CloseLauncher(void);

  virtual BOOL OnInitDialog();

  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnOpenProject(UINT nID);
  afx_msg void OnCreateProject();
  afx_msg void OnCreateExtProject();
  afx_msg void OnClickedAdvice(UINT nID);
  afx_msg void OnClickedLink(UINT nID);

  enum { IDD = IDD_LAUNCHER };

  bool m_modal;

  CDibSection m_banner;
  CFont m_bigFont;
  CFont m_titleFont;
  CommandButton m_cmds[27];
  ReportHtml m_html;

protected:
  DECLARE_MESSAGE_MAP()

private:
  void SetBannerBitmap(void);
  void GetRegions(CArray<CRect>& regions);
  void ShowHtml(bool show);
};
