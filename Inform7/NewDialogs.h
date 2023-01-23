#pragma once

#include "BaseDialog.h"
#include "Inform.h"
#include "UnicodeEdit.h"

#include "DarkMode.h"

class AbstractNewDialog : public I7BaseDialog
{
  DECLARE_DYNAMIC(AbstractNewDialog)

public:
  AbstractNewDialog(CWnd* parent);

  virtual BOOL OnInitDialog();

  enum { IDD = IDD_NEWPROJECT };

  virtual const char* GetType(void) = 0;
  virtual CString GetPath(void) = 0;
  virtual bool CheckPath(void) = 0;

  CString GetName(void);
  CStringW GetAuthor(void);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnOK();
  afx_msg void OnClickedDirPopup();
  afx_msg void OnChangedEdit();

  DECLARE_MESSAGE_MAP()

  void AddType(CWnd* wnd);

  CString m_dir;
  CString m_name;
  CStringW m_author;

  DarkModeEdit m_dirEdit, m_nameEdit;
  UnicodeEdit m_authorEdit;
  DarkModeButton m_start, m_cancel, m_dirPopup;
};

class NewProjectDialog : public AbstractNewDialog
{
  DECLARE_DYNAMIC(NewProjectDialog)

public:
  NewProjectDialog(ProjectType projectType, LPCSTR dir, CWnd* parent);
  void FromExt(const char* name, const char* author);

  virtual BOOL OnInitDialog();

  virtual const char* GetType(void);
  virtual CString GetPath(void);
  virtual bool CheckPath(void);

protected:
  ProjectType m_projectType;
  bool m_fromExt;
};

class NewExtensionDialog : public AbstractNewDialog
{
  DECLARE_DYNAMIC(NewExtensionDialog)

public:
  NewExtensionDialog(CWnd* parent);

  virtual BOOL OnInitDialog();

  virtual const char* GetType(void);
  virtual CString GetPath(void);
  virtual bool CheckPath(void);
};
