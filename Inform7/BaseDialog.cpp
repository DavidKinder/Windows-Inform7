#include "stdafx.h"
#include "BaseDialog.h"
#include "Inform.h"

IMPLEMENT_DYNAMIC(I7BaseDialog, BaseDialog)

I7BaseDialog::I7BaseDialog(UINT templateId, BOOL appFontSize, CWnd* parent)
 : BaseDialog(templateId,parent), m_appFontSize(appFontSize)
{
}

I7BaseDialog::I7BaseDialog(BOOL appFontSize) : m_appFontSize(appFontSize)
{
}

void I7BaseDialog::SetFont(CDialogTemplate& dlgTemplate)
{
  WORD fontSize = 0;
  if (m_appFontSize)
    fontSize = theApp.GetFontPointSize();
  else
    fontSize = theApp.GetDialogFontSize()/10;

  dlgTemplate.SetFont(theApp.GetFontName(),fontSize);
}
