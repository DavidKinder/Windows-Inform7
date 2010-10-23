#pragma once

#include "Platform.h"
#include "Scintilla.h"

class ReportEdit : public CWnd
{
  DECLARE_DYNAMIC(ReportEdit)

public:
  ReportEdit();
  BOOL Create(CWnd* parent, UINT id);

  void AppendText(const char* text);
  void ClearText(void);
  void SetFormat(bool fixed);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
  afx_msg void OnEditCopy();
  afx_msg void OnEditSelectAll();

private:
  LONG_PTR CallEdit(UINT msg, DWORD wp = 0, LONG_PTR lp = 0);

  sptr_t m_editPtr;
  bool m_fixed;
};
