#pragma once

#include "Dib.h"
#include "UnicodeEdit.h"

class SearchEdit : public UnicodeEdit
{
  DECLARE_DYNAMIC(SearchEdit)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg HBRUSH CtlColor(CDC*, UINT);
  afx_msg void OnEnKillFocus();
  afx_msg void OnEnSetFocus();
  afx_msg void OnEditSelectAll();

  UINT m_msg;

  bool m_editing;
  CStringW m_editText;
  const CStringW m_displayText;

  CDibSection* m_image;

public:
  SearchEdit(UINT msg, LPCWSTR displayText);
  void Init(UINT id, CWnd* parent);

  virtual BOOL PreTranslateMessage(MSG* pMsg);
};
