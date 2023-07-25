#pragma once

class PickExtensionDialogImpl;

class PickExtensionDialog
{
public:
  PickExtensionDialog(const char* title, bool legacyDir, CWnd* parentWnd);
  ~PickExtensionDialog();

  INT_PTR ShowDialog(void);
  CString GetExtensionPath(void);

private:
  PickExtensionDialogImpl* m_impl;
};
