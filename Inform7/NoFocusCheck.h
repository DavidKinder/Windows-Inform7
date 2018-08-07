#pragma once

class NoFocusCheck : public CButton
{
  DECLARE_DYNAMIC(NoFocusCheck)

public:
  NoFocusCheck();

  bool GetIsChecked(void);
  void SetIsChecked(bool checked);

  BOOL SubclassDlgItem(UINT id, CWnd* parent);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnClicked();
  afx_msg void OnCustomDraw(NMHDR*, LRESULT*);

private:
  bool m_checked;
};
