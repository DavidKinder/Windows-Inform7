#pragma once

#include "Dib.h"

class FlatButton : public CButton
{
  DECLARE_DYNAMIC(FlatButton)

protected:
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void DrawItem(LPDRAWITEMSTRUCT dis);

public:
  FlatButton();

  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);

protected:
  CDibSection* GetImage(const char* name, const CSize& size, bool light);

  bool m_mouseOver;

public:
  virtual HRESULT get_accName(VARIANT child, BSTR* name);
};
