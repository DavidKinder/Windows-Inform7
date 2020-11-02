#pragma once

#include "BaseDialog.h"

class FindReplaceDialog : public I7BaseDialog
{
  DECLARE_DYNAMIC(FindReplaceDialog)

public:
  static FindReplaceDialog* Create(UINT id, CWnd* parentWnd);
  void Show(LPCWSTR findText);

  CStringW GetFindString(void) const;
  CStringW GetReplaceString(void) const;
  bool MatchCase(void) const;
  BOOL MatchWholeWord(void) const;
  BOOL ReplaceCurrent(void) const;
  BOOL ReplaceAll(void) const;

protected:
  FindReplaceDialog(UINT id, CWnd* parentWnd);

  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnCancel();

  DECLARE_MESSAGE_MAP()

  afx_msg void OnClose();
  afx_msg void OnFindNext();
  afx_msg void OnFindPrevious();
  afx_msg void OnChangeFindText();

private:
  CStringW m_findText;
  BOOL m_ignoreCase;
};
