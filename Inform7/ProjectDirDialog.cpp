#include "stdafx.h"
#include "ProjectDirDialog.h"
#include "OSLayer.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Logic used by both file dialogs

enum DirResult
{
  NotProject,
  IsProject,
  NoExtension
};

DirResult IsProjectDir(const CString& dir)
{
  // Get the last part of the path
  int i = dir.ReverseFind('\\');
  if (i == -1)
    return NotProject;
  CString last = dir.Mid(i+1);

  // Look at the extension on the last part
  i = last.ReverseFind('.');
  if (i == -1)
    return NoExtension;
  if (last.Mid(i).CompareNoCase(".inform") == 0)
    return IsProject;
  if (last.Mid(i).CompareNoCase(".i7xp") == 0)
    return IsProject;
  return NotProject;
}

class FixedTextButton : public CWnd
{
protected:
  DECLARE_MESSAGE_MAP()

  afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam)
  {
    return TRUE;
  }
};

BEGIN_MESSAGE_MAP(FixedTextButton, CWnd)
  ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

// Implementation of the project directory dialog
class ProjectDirDialogImpl
{
public:
  ProjectDirDialogImpl(bool open, const char* dir, const char* title, const char* saveExt, CWnd* parentWnd) : m_events(this)
  {
    m_open = open;
    m_title = title;
    m_saveExt = saveExt;
    m_parent = parentWnd->GetSafeHwnd();
    m_eventsCookie = 0;

    // Create the file dialog object
    if (SUCCEEDED(m_dialog.CoCreateInstance(open ? __uuidof(FileOpenDialog) : __uuidof(FileSaveDialog))))
    {
      // Set up the dialog
      COMDLG_FILTERSPEC filter[] = { { L"Inform 7 projects", L"" } };
      m_dialog->SetFileTypes(1,filter);
      m_dialog->SetTitle(CStringW(title));
      m_dialog->SetFileNameLabel(L"Project &name:");
      DWORD options = 0;
      m_dialog->GetOptions(&options);
      m_dialog->SetOptions(options|FOS_DONTADDTORECENT);

      // Listen for events
      m_dialog->Advise(&m_events,&m_eventsCookie);

      // Get an IOleWindow pointer from the dialog
      m_window = m_dialog;

      // Find the parent directory of the default project directory
      CString initialDir = dir;
      int i = initialDir.ReverseFind('\\');
      if (i != -1)
        initialDir.Truncate(i);
      else
        initialDir.Empty();

      // Open the file dialog in the parent directory
      CComPtr<IShellItem> si;
      if (SUCCEEDED(theOS.SHCreateItemFromParsingName(
        CStringW(initialDir),NULL,__uuidof(IShellItem),(void**)&si)))
      {
        m_dialog->SetFolder(si);
      }
    }
  }

  ~ProjectDirDialogImpl()
  {
    // Stop listening to events
    if (m_dialog != NULL)
      m_dialog->Unadvise(m_eventsCookie);

    // Since the OK button belongs to the file dialog,
    // don't try to destroy it here
    m_okButton.Detach();
  }

  INT_PTR ShowDialog(void)
  {
    HRESULT result = E_NOTIMPL;
    if (m_dialog != NULL)
    {
      // Hook window procedures to get a notification when the dialog opens
      m_hook = ::SetWindowsHookEx(WH_CALLWNDPROCRET,MsgProc,NULL,::GetCurrentThreadId());
      m_instance = this;

      // Show the file dialog
      result = m_dialog->Show(m_parent);

      // Remove the window procedure hook
      ::UnhookWindowsHookEx(m_hook);
      m_hook = 0;
      m_instance = NULL;

      // If the return code is cancel but the project directory is set,
      // the user successfully picked a directory.
      if (result == MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_CANCELLED))
      {
        if (!m_projectDir.IsEmpty())
          result = S_OK;
      }
    }
    return (result == S_OK) ? IDOK : IDCANCEL;
  }

  void OnFolderChange()
  {
    m_projectDir.Empty();
    CString dir;
    {
      CComPtr<IShellItem> dirShellItem;
      if (FAILED(m_dialog->GetFolder(&dirShellItem)))
        return;
      dir = GetPathFromShellItem(dirShellItem);
    }

    // Is the window up yet?
    HWND wnd;
    if (FAILED(m_window->GetWindow(&wnd)))
      return;
    if (!::IsWindowVisible(wnd))
      return;

    // Is the new folder an Inform project?
    if (IsProjectDir(dir) == IsProject)
    {
      // Got an Inform project
      m_projectDir = dir;

      // If the project already exists, check with the user
      CheckOverwrite();

      // Close the dialog. Cancel is used to make sure that
      // the dialog is closed without any validation.
      ::PostMessage(wnd,WM_COMMAND,IDCANCEL,0);
    }
  }

  bool OnFileNameOK()
  {
    if (m_open)
    {
      // Prevent file selection, only directory selection is wanted
      return false;
    }
    else
    {
      // For the save dialog, the user can enter a name
      m_projectDir.Empty();
      CString dir;
      {
        CComPtr<IShellItem> dirShellItem;
        if (FAILED(m_dialog->GetResult(&dirShellItem)))
          return false;
        dir = GetPathFromShellItem(dirShellItem);
      }

      // Is the name an Inform project?
      switch (IsProjectDir(dir))
      {
      case IsProject:
        m_projectDir = dir;
        return CheckOverwrite();
      case NoExtension:
        // Add the standard extension
        m_projectDir = dir + m_saveExt;
        return CheckOverwrite();
      }
    }
    return false;
  }

  void OnInitDone(HWND wnd)
  {
    // Subclass the OK button to prevent its text changing
    // to "Open" when a directory is selected.
    m_okButton.SubclassDlgItem(IDOK,CWnd::FromHandle(wnd));
  }

private:
  bool CheckOverwrite(void)
  {
    // Only do anything if this is a save dialog
    if (m_open)
      return true;

    // If the directory doesn't exist, don't ask the user
    if (::GetFileAttributes(m_projectDir) == INVALID_FILE_ATTRIBUTES)
      return true;

    // Get the file dialog window
    HWND wnd;
    if (FAILED(m_window->GetWindow(&wnd)))
      return true;

    // Ask the user
    CString message;
    message.Format("%s already exists.\nDo you want to replace it?",(LPCSTR)m_projectDir);
    if (::MessageBox(wnd,message,m_title,MB_YESNO|MB_DEFBUTTON2|MB_ICONWARNING) == IDYES)
      return true;

    m_projectDir.Empty();
    return false;
  }

  CString GetPathFromShellItem(IShellItem* shellItem)
  {
    LPOLESTR path;
    if (SUCCEEDED(shellItem->GetDisplayName(SIGDN_FILESYSPATH,&path)))
    {
      CString pathStr(path);
      ::CoTaskMemFree(path);
      return pathStr;
    }
    return "";
  }

  bool m_open;
  CString m_title;

  HWND m_parent;
  CString m_projectDir;
  CString m_saveExt;
  FixedTextButton m_okButton;

  CComPtr<IFileDialog> m_dialog;
  CComQIPtr<IOleWindow> m_window;

  friend class ProjectDirDialog;

  class DialogEvents : public CCmdTarget, public IFileDialogEvents
  {
  public:
    DialogEvents(ProjectDirDialogImpl* dialog) : m_dialog(dialog)
    {
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
      return 1;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
      return 1;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppvObj)
    {
      return (HRESULT)InternalQueryInterface(&iid,ppvObj);
    }

    HRESULT STDMETHODCALLTYPE OnFileOk(IFileDialog*)
    {
      return m_dialog->OnFileNameOK() ? S_OK : S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE OnFolderChanging(IFileDialog*, IShellItem*)
    {
      return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE OnFolderChange(IFileDialog*)
    {
      m_dialog->OnFolderChange();
      return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnSelectionChange(IFileDialog*)
    {
      return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*)
    {
      return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE OnTypeChange(IFileDialog*)
    {
      return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*)
    {
      return E_NOTIMPL;
    }

  private:
    ProjectDirDialogImpl* m_dialog;
  };

  DialogEvents m_events;
  DWORD m_eventsCookie;

  static LRESULT CALLBACK MsgProc(int code, WPARAM wparam, LPARAM lparam)
  {
    if (code == HC_ACTION)
    {
      CWPRETSTRUCT* msg = (CWPRETSTRUCT*)lparam;
      if (msg->message == WM_INITDIALOG)
        m_instance->OnInitDone(msg->hwnd);
    }
    return ::CallNextHookEx(m_hook,code,wparam,lparam);
  }

  static HHOOK m_hook;
  static ProjectDirDialogImpl* m_instance;
};

HHOOK ProjectDirDialogImpl::m_hook = 0;
ProjectDirDialogImpl* ProjectDirDialogImpl::m_instance = NULL;

// Implementation of facade class

ProjectDirDialog::ProjectDirDialog(bool open, const char* dir, const char* title, const char* saveExt, CWnd* parentWnd)
{
  m_impl = new ProjectDirDialogImpl(open,dir,title,saveExt,parentWnd);
}

ProjectDirDialog::~ProjectDirDialog()
{
  delete m_impl;
}

INT_PTR ProjectDirDialog::ShowDialog(void)
{
  if (m_impl != NULL)
    return m_impl->ShowDialog();
  return IDCANCEL;
}

CString ProjectDirDialog::GetProjectDir(void)
{
  if (m_impl != NULL)
    return m_impl->m_projectDir;
  return "";
}
