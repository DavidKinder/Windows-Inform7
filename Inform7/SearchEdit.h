#pragma once

#include "Dib.h"
#include "UnicodeEdit.h"

class SearchEdit : public UnicodeEdit
{
  DECLARE_DYNAMIC(SearchEdit)

public:
  SearchEdit(UINT msg, LPCWSTR displayText, LPCSTR accName);
  void Init(UINT id, CWnd* parent);
  void SetEditRect(void);

  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual HRESULT get_accName(VARIANT child, BSTR* name);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg HBRUSH CtlColor(CDC*, UINT);
	afx_msg void OnNcPaint();
  afx_msg void OnEnKillFocus();
  afx_msg void OnEnSetFocus();
  afx_msg void OnEditSelectAll();

  CDibSection* GetImage(COLORREF back, bool dark);

  UINT m_msg;

  bool m_editing;
  CStringW m_editText;
  const CStringW m_displayText;
  const CString m_accName;
};
