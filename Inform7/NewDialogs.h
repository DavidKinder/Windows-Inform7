#pragma once

#include "BaseDialog.h"
#include "Resource.h"

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

  static int CALLBACK BrowseDirCB(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
  void AddType(CWnd* wnd);

  CString m_dir;
  CString m_name;
  CStringW m_author;
};

class NewProjectDialog : public AbstractNewDialog
{
  DECLARE_DYNAMIC(NewProjectDialog)

public:
  NewProjectDialog(LPCSTR dir, CWnd* parent);

  virtual const char* GetType(void);
  virtual CString GetPath(void);
  virtual bool CheckPath(void);
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
