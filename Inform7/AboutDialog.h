#pragma once

#include "BaseDialog.h"
#include "RichEdit.h"
#include "Dib.h"
#include "Resource.h"

#include <memory>

class AboutCreditsEdit : public RichEdit
{
  DECLARE_DYNAMIC(AboutCreditsEdit)

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

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* mmi);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnCreditsResize(NMHDR* notify, LRESULT*);

  void LayoutControls(void);

private:
  CStatic m_logo;
  CStatic m_title, m_version, m_copyright;

  AboutCreditsEdit m_credits;
  int m_creditHeight;

  CSize m_initialSize;
  CFont m_titleFont;
  std::auto_ptr<CDibSection> m_bitmap;
};
