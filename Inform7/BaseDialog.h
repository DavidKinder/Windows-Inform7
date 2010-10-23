#pragma once

#include "Dialogs.h"

class I7BaseDialog : public BaseDialog
{
  DECLARE_DYNAMIC(I7BaseDialog)

public:
  I7BaseDialog(UINT templateId, BOOL appFontSize, CWnd* parent = NULL);

protected:
  I7BaseDialog(BOOL appFontSize);
  void SetFont(CDialogTemplate& dlgTemplate);

  BOOL m_appFontSize;
};
