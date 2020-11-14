#pragma once

#include "BaseDialog.h"
#include "FindInFiles.h"

class FindReplaceDialog : public I7BaseDialog
{
  DECLARE_DYNAMIC(FindReplaceDialog)

public:
  static FindReplaceDialog* Create(UINT id, CWnd* parentWnd);
  void Show(LPCWSTR findText);

  CStringW GetFindString(void) const;
  CStringW GetReplaceString(void) const;
  bool MatchCase(void) const;
  FindRule GetFindRule(void) const;

protected:
  FindReplaceDialog(UINT id, CWnd* parentWnd);

  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnCancel();

  DECLARE_MESSAGE_MAP()

  afx_msg void OnClose();
  afx_msg void OnFindNext();
  afx_msg void OnFindPrevious();
  afx_msg void OnChangeFindText();
  afx_msg void OnReplace();
  afx_msg void OnReplaceAll();
  afx_msg void OnChangeReplaceWith();

private:
  void EnableActions(void);

  CStringW m_findText, m_replaceWith;
  BOOL m_ignoreCase;
  FindRule m_findRule;
};
