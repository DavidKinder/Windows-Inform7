#pragma once

class ProjectDirDialogImpl;

class ProjectDirDialog
{
public:
  ProjectDirDialog(bool open, const char* dir, const char* title, const char* saveExt, CWnd* parentWnd);
  ~ProjectDirDialog();

  INT_PTR ShowDialog(void);
  CString GetProjectDir(void);

private:
  ProjectDirDialogImpl* m_impl;
};
