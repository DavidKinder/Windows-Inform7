#include "stdafx.h"
#include "NoFocusCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(NoFocusCheck, DarkModeCheckButton)

BEGIN_MESSAGE_MAP(NoFocusCheck, DarkModeCheckButton)
  ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

NoFocusCheck::NoFocusCheck()
{
  m_checked = false;
}

bool NoFocusCheck::GetIsChecked(void)
{
  return m_checked;
}

void NoFocusCheck::SetIsChecked(bool checked)
{
  m_checked = checked;
  if (GetSafeHwnd())
    SetCheck(checked ? BST_CHECKED : BST_UNCHECKED);
}

BOOL NoFocusCheck::SubclassDlgItem(UINT id, CWnd* parent, UINT imageId, DarkMode::DarkColour back)
{
  if (DarkModeCheckButton::SubclassDlgItem(id,parent,imageId,back))
  {
    SetCheck(m_checked ? BST_CHECKED : BST_UNCHECKED);
    return TRUE;
  }
  return FALSE;
}

BOOL NoFocusCheck::OnClicked() 
{
  m_checked = (GetCheck() == BST_CHECKED);
  return FALSE;
}

void NoFocusCheck::OnCustomDraw(NMHDR* nmhdr, LRESULT* result)
{
  DarkMode* dark = DarkMode::GetActive(this);
  if (dark)
    DarkModeCheckButton::OnCustomDraw(nmhdr,result);
  else
  {
    // Skip the pre-paint stage to avoid drawing the label and focus rectangle
    NMCUSTOMDRAW* nmcd = (NMCUSTOMDRAW*)nmhdr;
    if (nmcd->dwDrawStage == CDDS_PREPAINT)
      *result = CDRF_SKIPDEFAULT;
    else
      *result = CDRF_DODEFAULT;
  }
}

bool NoFocusCheck::HasFocusRect(UINT uiState)
{
  return false;
}
