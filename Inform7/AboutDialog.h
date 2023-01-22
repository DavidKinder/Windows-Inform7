#pragma once

#include "BaseDialog.h"
#include "RichEdit.h"
#include "Dib.h"
#include "Resource.h"

#include <memory>

class AboutCreditsEdit : public RichEdit
{
  DECLARE_DYNAMIC(AboutCreditsEdit)

  AboutCreditsEdit();

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSetFocus(CWnd* pOldWnd);

  virtual BOOL PreTranslateMessage(MSG* pMsg);
};

class AboutDialog : public I7BaseDialog
{
public:
  AboutDialog();

  enum { IDD = IDD_ABOUTBOX };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();

  DECLARE_MESSAGE_MAP()

  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* mmi);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnCreditsResize(NMHDR* notify, LRESULT*);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  void LayoutControls(void);
  void SetTitleFont(void);
  void SetCredits(void);

private:
  UINT m_dpi;

  CStatic m_logo;
  CStatic m_title, m_version, m_copyright;

  AboutCreditsEdit m_credits;
  CString m_creditsRTF;
  int m_creditHeight;

  CSize m_initialSize;
  CFont m_titleFont;
  std::auto_ptr<CDibSection> m_bitmap;
};
