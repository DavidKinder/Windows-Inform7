#pragma once

#include "BaseDialog.h"
#include "Dib.h"
#include "Resource.h"

class SplashScreen : public I7BaseDialog
{
  DECLARE_DYNAMIC(SplashScreen)

public:
  SplashScreen(CWnd* pParent = NULL);

  void ShowSplash(void);

  virtual BOOL OnInitDialog();

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

  afx_msg void OnNewProject();
  afx_msg void OnReopenLast();
  afx_msg void OnOpenProject();

  enum { IDD = IDD_SPLASH };

  CStatic m_intro;
  CDibSection m_back;

  CButton m_newProject;
  CButton m_reopenLast;
  CButton m_openProject;
  CFont m_buttonFont;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()
};
