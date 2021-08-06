#pragma once

class CDibSection;

class ButtonTab : public CTabCtrl
{
  DECLARE_DYNAMIC(ButtonTab)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();

public:
  void UpdateDPI(void);

protected:
  CDibSection* GetImage(const char* name, const CSize& size);
};
