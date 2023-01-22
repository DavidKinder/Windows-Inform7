#pragma once

#include "BaseDialog.h"
#include "FindAllHelper.h"

#include "DarkMode.h"

class RichDrawText;

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
  void InitDialog(void);

  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnCancel();

  DECLARE_MESSAGE_MAP()

  afx_msg void OnClose();
  afx_msg void OnDestroy();
  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  afx_msg void OnFindNext();
  afx_msg void OnFindPrevious();
  afx_msg void OnFindAll();
  afx_msg void OnChangeFindText();
  afx_msg void OnReplace();
  afx_msg void OnReplaceAll();
  afx_msg void OnChangeReplaceWith();
  afx_msg void OnChangeFindRule();

  afx_msg void OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnResultsSelect(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg LRESULT OnResultsResize(WPARAM, LPARAM);

private:
  void EnableActions(void);
  void SetRichTextRTF(const char* fragment);
  void ShowResult(const FindResult& result);

  CStringW m_findText, m_replaceWith;
  DarkModeButton m_findNext, m_findPrev, m_findAll;
  DarkModeButton m_replace, m_replaceAll;

  DarkModeGroupBox m_howGroup;
  DarkModeCheckButton m_ignoreCaseCtrl;
  BOOL m_ignoreCase;
  DarkModeComboBox m_findRuleCtrl;
  FindRule m_findRule;

  FindResultsCtrl m_resultsList;
  CStatic m_regexHelp;
  RichDrawText* m_richText;

  FindAllHelper m_findHelper;
  int m_heightNormal, m_heightLong;
  UINT m_dpi;
};
