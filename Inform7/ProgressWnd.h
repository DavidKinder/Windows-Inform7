#pragma once

class ProgressWnd : public CWnd
{
  DECLARE_DYNAMIC(ProgressWnd)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);

public:
  ProgressWnd();
  BOOL Create(CWnd* parentWnd, DWORD style);

  void ToFront();
  void TaskProgress(const char* text, int progress);
  void TaskDone();

  void LongTaskProgress(const char* text, int step, int stepTotal);
  void LongTaskAdvance();
  void LongTaskDone();

private:
  CStatic m_text;
  CProgressCtrl m_progress;
  CString m_long;
  int m_longStep;
  int m_longStepTotal;
};
