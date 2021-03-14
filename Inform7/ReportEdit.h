#pragma once

class ReportEdit : public CWnd
{
  DECLARE_DYNAMIC(ReportEdit)

public:
  ReportEdit();
  BOOL Create(CWnd* parent, UINT id);
  void FontChanged(void);
  void PrefsChanged(void);

  void AppendText(const char* text);
  void ClearText(void);
  void SetFormat(bool fixed);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

  afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
  afx_msg void OnEditCopy();
  afx_msg void OnEditSelectAll();

private:
  void SetFonts(void);
  LONG_PTR CallEdit(UINT msg, DWORD wp = 0, LONG_PTR lp = 0);

  LONG_PTR m_editPtr;
  bool m_fixed;
};
