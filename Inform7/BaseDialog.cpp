#include "stdafx.h"
#include "Inform.h"

#include "BaseDialog.h"
#include "DarkMode.h"
#include "DpiFunctions.h"

IMPLEMENT_DYNAMIC(I7BaseDialog, BaseDialog)

I7BaseDialog::I7BaseDialog(UINT templateId, CWnd* parent) : BaseDialog(templateId,parent)
{
}

INT_PTR I7BaseDialog::DoModal()
{
  theApp.AddModalDialog(this);
  INT_PTR result = BaseDialog::DoModal();
  theApp.RemoveModalDialog(this);
  return result;
}

void I7BaseDialog::SetFont(CDialogTemplate& dlgTemplate)
{
  dlgTemplate.SetFont(
    theApp.GetFontName(InformApp::FontSystem),theApp.GetFontSize(InformApp::FontSystem));
}

void I7BaseDialog::EraseWithGripper(CDC* dc)
{
  DarkMode* dark = DarkMode::GetActive(this);

  CRect dlgRect;
  GetClientRect(dlgRect);
  dc->FillSolidRect(dlgRect,dark ?
    dark->GetColour(DarkMode::Darkest) : ::GetSysColor(COLOR_BTNFACE));

  CRect gripRect = dlgRect;
  int dpi = DPI::getWindowDPI(this);
  gripRect.left = gripRect.right - DPI::getSystemMetrics(SM_CXHSCROLL,dpi);
  gripRect.top = gripRect.bottom - DPI::getSystemMetrics(SM_CYVSCROLL,dpi);

  // Draw a gripper to show that the dialog can be resized
  bool drawn = false;
  if (::IsAppThemed())
  {
    // Open the status bar theme
    HTHEME theme = ::OpenThemeData(GetSafeHwnd(),L"Status");
    if (theme)
    {
      ::DrawThemeBackground(theme,dc->GetSafeHdc(),SP_GRIPPER,0,gripRect,NULL);
      ::CloseThemeData(theme);
      drawn = true;
    }
  }
  if (!drawn)
    dc->DrawFrameControl(gripRect,DFC_SCROLL,DFCS_SCROLLSIZEGRIP);
}

void I7BaseDialog::UpdateUIState(void)
{
  BOOL always = FALSE;
  ::SystemParametersInfo(SPI_GETKEYBOARDCUES,0,&always,0);
  if (!always)
    SendMessage(WM_UPDATEUISTATE,MAKEWPARAM(UIS_INITIALIZE,UISF_HIDEACCEL),0);
}
