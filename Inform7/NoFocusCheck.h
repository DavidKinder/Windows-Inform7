#pragma once

#include "DarkMode.h"

class NoFocusCheck : public DarkModeCheckButton
{
  DECLARE_DYNAMIC(NoFocusCheck)

public:
  NoFocusCheck();

  bool GetIsChecked(void);
  void SetIsChecked(bool checked);

  BOOL SubclassDlgItem(UINT id, CWnd* parent, UINT imageId, DarkMode::DarkColour back);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnClicked();
  afx_msg void OnCustomDraw(NMHDR*, LRESULT*);

  bool HasFocusRect(UINT uiState);

private:
  bool m_checked;
};
