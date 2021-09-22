#pragma once

#include "BaseDialog.h"
#include "Dib.h"
#include "Resource.h"

class WelcomeLauncher : public I7BaseDialog
{
  DECLARE_DYNAMIC(WelcomeLauncher)

public:
  WelcomeLauncher(CWnd* pParent = NULL);

  void ShowLauncher(void);

  virtual BOOL OnInitDialog();

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  enum { IDD = IDD_LAUNCHER };

  CDibSection* m_original;
  CDibSection m_back;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()

private:
  void SetBackBitmap(void);
};
