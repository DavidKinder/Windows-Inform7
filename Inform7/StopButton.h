#pragma once

class CDibSection;

class StopButton : public CButton
{
  DECLARE_DYNAMIC(StopButton)

public:
  BOOL Create(DWORD style, const RECT& rect, CWnd* parent, UINT id);
  CSize GetButtonSize(void);

protected:
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void DrawItem(LPDRAWITEMSTRUCT dis);

  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

  CDibSection* GetImage(bool hot);
  void ReverseImage(CDibSection* image);
};
