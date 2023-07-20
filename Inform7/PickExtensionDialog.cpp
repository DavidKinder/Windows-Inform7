#include "stdafx.h"
#include "PickExtensionDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Implementation of the extension picker dialog
class PickExtensionDialogImpl
{
public:
  PickExtensionDialogImpl(const char* dir, const char* title, CWnd* parentWnd)
  {
    m_parent = parentWnd->GetSafeHwnd();

    // Create the file dialog object
    if (SUCCEEDED(m_dialog.CoCreateInstance(__uuidof(FileOpenDialog))))
    {
      // Set up the dialog
      COMDLG_FILTERSPEC filter[] =
      {
        { L"Inform extensions", L"*.i7x" },
        { L"All Files", L"*.*" }
      };
      m_dialog->SetFileTypes(2,filter);
      m_dialog->SetTitle(CStringW(title));
      DWORD options = 0;
      m_dialog->GetOptions(&options);
      m_dialog->SetOptions(options|FOS_DONTADDTORECENT);

      // Open the file dialog in the given directory
      if (dir != NULL)
      {
        CComPtr<IShellItem> si;
        if (SUCCEEDED(::SHCreateItemFromParsingName(CStringW(dir),
          NULL,__uuidof(IShellItem),(void**)&si)))
        {
          m_dialog->SetFolder(si);
        }
      }
    }
  }

  INT_PTR ShowDialog(void)
  {
    HRESULT result = E_NOTIMPL;
    if (m_dialog != NULL)
      result = m_dialog->Show(m_parent);
    return (result == S_OK) ? IDOK : IDCANCEL;
  }

  CString GetExtensionPath(void)
  {
    CComPtr<IShellItem> si;
    if (SUCCEEDED(m_dialog->GetResult(&si)))
    {
      LPOLESTR path;
      if (SUCCEEDED(si->GetDisplayName(SIGDN_FILESYSPATH,&path)))
      {
        CString pathStr(path);
        ::CoTaskMemFree(path);
        return pathStr;
      }
    }
    return "";
  }

  HWND m_parent;
  CComPtr<IFileDialog> m_dialog;

  friend class PickExtensionDialog;
};

// Implementation of facade class

PickExtensionDialog::PickExtensionDialog(const char* dir, const char* title, CWnd* parentWnd)
{
  m_impl = new PickExtensionDialogImpl(dir,title,parentWnd);
}

PickExtensionDialog::~PickExtensionDialog()
{
  delete m_impl;
}

INT_PTR PickExtensionDialog::ShowDialog(void)
{
  if (m_impl != NULL)
    return m_impl->ShowDialog();
  return IDCANCEL;
}

CString PickExtensionDialog::GetExtensionPath(void)
{
  if (m_impl != NULL)
    return m_impl->GetExtensionPath();
  return "";
}
