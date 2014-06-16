#pragma once

#include "Dib.h"

class FlatButton : public CButton
{
  DECLARE_DYNAMIC(FlatButton)

protected:
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void DrawItem(LPDRAWITEMSTRUCT dis);

  CDibSection* GetImage(const char* name, const CSize& size, bool light);
};
