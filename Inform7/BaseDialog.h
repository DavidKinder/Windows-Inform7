#pragma once

#include "Dialogs.h"

class I7BaseDialog : public BaseDialog
{
  DECLARE_DYNAMIC(I7BaseDialog)

protected:
  I7BaseDialog(UINT templateId, CWnd* parent = NULL);
  void SetFont(CDialogTemplate& dlgTemplate);

  void EraseWithGripper(CDC* dc);
  void UpdateUIState(void);
};
