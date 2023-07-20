#pragma once

class PickExtensionDialogImpl;

class PickExtensionDialog
{
public:
  PickExtensionDialog(const char* dir, const char* title, CWnd* parentWnd);
  ~PickExtensionDialog();

  INT_PTR ShowDialog(void);
  CString GetExtensionPath(void);

private:
  PickExtensionDialogImpl* m_impl;
};
