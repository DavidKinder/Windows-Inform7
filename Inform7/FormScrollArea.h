#pragma once

#include "DrawScrollArea.h"

// Replacement for CFormView
class FormScrollArea : public DrawScrollArea
{
  DECLARE_DYNAMIC(FormScrollArea)

public:
  virtual void OnInitialUpdate();

  virtual DarkMode::DarkColour GetDarkBackground(void);

protected:
  DECLARE_MESSAGE_MAP()

  FormScrollArea(UINT nIDTemplate);

  afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);

  virtual void OnDraw(CDC* pDC);
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  virtual BOOL Create(DWORD, const RECT&, CWnd*, UINT);
  BOOL CreateDlg(LPCTSTR, CWnd*);
  BOOL CreateDlgIndirect(LPCDLGTEMPLATE, CWnd*, HINSTANCE);

  LPCTSTR m_lpszTemplateName;
};
