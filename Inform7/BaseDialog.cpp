#include "stdafx.h"
#include "BaseDialog.h"
#include "Inform.h"

IMPLEMENT_DYNAMIC(I7BaseDialog, BaseDialog)

I7BaseDialog::I7BaseDialog(UINT templateId, CWnd* parent) : BaseDialog(templateId,parent)
{
}

void I7BaseDialog::SetFont(CDialogTemplate& dlgTemplate)
{
  dlgTemplate.SetFont(
    theApp.GetFontName(InformApp::FontSystem),theApp.GetFontSize(InformApp::FontSystem));
}
