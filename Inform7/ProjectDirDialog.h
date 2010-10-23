#pragma once

class ProjectDirDialogXP;
class ProjectDirDialogVista;

class ProjectDirDialog
{
public:
  ProjectDirDialog(bool open, const char* dir, const char* title, CWnd* parentWnd);
  ~ProjectDirDialog();

  INT_PTR ShowDialog(void);
  CString GetProjectDir(void);

private:
  ProjectDirDialogXP* m_dialogXP;
  ProjectDirDialogVista* m_dialogVista;
};
