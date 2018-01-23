#pragma once

#include "StopButton.h"

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
  int GetProgress();
  void TaskProgress(const char* text, int progress);
  void TaskDone();

  void LongTaskProgress(const char* text, int step, int stepTotal);
  void LongTaskAdvance();
  void LongTaskDone();

  void ShowStop();
  bool WantStop();

private:
  afx_msg void OnStopClicked();

  CStatic m_text;
  StopButton m_stop;
  CProgressCtrl m_progress;
  CString m_long;
  int m_longStep;
  int m_longStepTotal;
  bool m_wantStop;
};
