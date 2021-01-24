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
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  afx_msg void OnNewProject();
  afx_msg void OnReopenLast();
  afx_msg void OnOpenProject();

  enum { IDD = IDD_SPLASH };

  CDibSection* m_original;
  CDibSection m_back;

  CStatic m_intro;
  CButton m_newProject;
  CButton m_reopenLast;
  CButton m_openProject;
  CFont m_buttonFont;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()

private:
  void SetButtonFont(void);
  void SetBackBitmap(void);
};
