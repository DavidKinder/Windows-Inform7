#pragma once

class ProgressWnd : public CWnd
{
  DECLARE_DYNAMIC(ProgressWnd)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);

public:
  BOOL Create(CWnd* parentWnd, DWORD style);
  void ShowProgress(const char* text, int progress);

private:
  CStatic m_text;
  CProgressCtrl m_progress;
};
