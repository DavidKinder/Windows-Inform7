#include "stdafx.h"
#include "PickExtensionDialog.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Implementation of the extension picker dialog
class PickExtensionDialogImpl
{
public:
  PickExtensionDialogImpl(const char* title, bool legacyDir, CWnd* parentWnd) : m_events(this)
  {
    m_legacy = legacyDir;
    m_parent = parentWnd->GetSafeHwnd();
    m_eventsCookie = 0;

    // Create the file dialog object
    if (SUCCEEDED(m_dialog.CoCreateInstance(__uuidof(FileOpenDialog))))
    {
      // Set up the dialog
      COMDLG_FILTERSPEC filter[] =
      {
        { L"Inform extensions", L"*.i7x;*.i7xd;*.zip" },
        { L"All Files", L"*.*" }
      };
      m_dialog->SetFileTypes(2,filter);
      m_dialog->SetTitle(CStringW(title));
      DWORD options = 0;
      m_dialog->GetOptions(&options);
      m_dialog->SetOptions(options|FOS_DONTADDTORECENT);

      // Choose a directory to open the file dialog in
      CString dir;
      if (legacyDir)
        dir.Format("%s\\Inform\\Extensions",(LPCSTR)theApp.GetHomeDir());
      else
      {
        CRegKey registryKey;
        if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
        {
          char settingDir[MAX_PATH];
          ULONG len = sizeof settingDir;
          if (registryKey.QueryStringValue("Install Extensions From",settingDir,&len) == ERROR_SUCCESS)
            dir = settingDir;
        }
      }

      // Open the file dialog in the appropriate directory
      if (!dir.IsEmpty())
      {
        CComPtr<IShellItem> si;
        if (SUCCEEDED(::SHCreateItemFromParsingName(CStringW(dir),
          NULL,__uuidof(IShellItem),(void**)&si)))
        {
          m_dialog->SetFolder(si);
        }
      }

      // Listen for events
      m_dialog->Advise(&m_events,&m_eventsCookie);

      // Get an IOleWindow pointer from the dialog
      m_window = m_dialog;
    }
  }

  ~PickExtensionDialogImpl()
  {
    // Stop listening to events
    if (m_dialog != NULL)
      m_dialog->Unadvise(m_eventsCookie);
  }

  INT_PTR ShowDialog(void)
  {
    HRESULT result = E_NOTIMPL;
    if (m_dialog != NULL)
    {
      result = m_dialog->Show(m_parent);

      // If the return code is cancel but an extension path is set,
      // the user successfully picked an extension directory.
      if (result == MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_CANCELLED))
      {
        if (!m_extPath.IsEmpty())
          result = S_OK;
      }

      if (!m_legacy && (result == S_OK))
      {
        // If a file or directory was chosen, save the directory it is in.
        CString extPath = GetExtensionPath();
        int i = extPath.ReverseFind('\\');
        if (i > 0)
        {
          CRegKey registryKey;
          if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
            registryKey.SetStringValue("Install Extensions From",extPath.Left(i));
        }
      }
    }
    return (result == S_OK) ? IDOK : IDCANCEL;
  }

  void OnFolderChange()
  {
    m_extPath.Empty();
    CString path;
    {
      CComPtr<IShellItem> si;
      if (FAILED(m_dialog->GetFolder(&si)))
        return;
      path = GetPathFromShellItem(si);
    }

    // Is the window up yet?
    HWND wnd;
    if (FAILED(m_window->GetWindow(&wnd)))
      return;
    if (!::IsWindowVisible(wnd))
      return;

    // Is the new folder an extension?
    if (IsExtensionDir(path))
    {
      // Got an Inform extension
      m_extPath = path;

      // Close the dialog. Cancel is used to make sure that
      // the dialog is closed without any validation.
      ::PostMessage(wnd,WM_COMMAND,IDCANCEL,0);
    }
  }

  CString GetExtensionPath(void)
  {
    if (!m_extPath.IsEmpty())
      return m_extPath;

    CComPtr<IShellItem> si;
    if (SUCCEEDED(m_dialog->GetResult(&si)))
      return GetPathFromShellItem(si);
    return "";
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

  bool IsExtensionDir(const CString& dir)
  {
    // Get the last part of the path
    int i = dir.ReverseFind('\\');
    if (i == -1)
      return false;
    CString last = dir.Mid(i+1);

    // Look at the extension on the last part
    i = last.ReverseFind('.');
    if (i == -1)
      return false;
    if (last.Mid(i).CompareNoCase(".i7xd") == 0)
      return true;
    return false;
  }

  bool m_legacy;
  HWND m_parent;
  CString m_extPath;

  CComPtr<IFileDialog> m_dialog;
  CComQIPtr<IOleWindow> m_window;

  friend class PickExtensionDialog;

  class DialogEvents : public CCmdTarget, public IFileDialogEvents
  {
  public:
    DialogEvents(PickExtensionDialogImpl* dialog) : m_dialog(dialog)
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
      return E_NOTIMPL;
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
    PickExtensionDialogImpl* m_dialog;
  };

  DialogEvents m_events;
  DWORD m_eventsCookie;
};

// Implementation of facade class

PickExtensionDialog::PickExtensionDialog(const char* title, bool legacyDir, CWnd* parentWnd)
{
  m_impl = new PickExtensionDialogImpl(title,legacyDir,parentWnd);
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
