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

  void ShowLauncher(void);

  virtual BOOL OnInitDialog();

  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnClickedAdvice(UINT nID);
  afx_msg void OnClickedLink(UINT nID);

  enum { IDD = IDD_LAUNCHER };

  CFont m_titleFont;
  CommandButton m_links[12];
  ReportHtml m_html;

  CDibSection* m_original;
  CDibSection m_back;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()

private:
  void SetBackBitmap(void);
  void GetRegions(CArray<CRect>& regions);
  void ShowHtml(bool show);
};
