#include "stdafx.h"
#include "NewDialogs.h"
#include "Inform.h"
#include "OSLayer.h"
#include "UnicodeEdit.h"

namespace {
  const char* invalid = "\\/:*?\"<>|";
}

IMPLEMENT_DYNAMIC(AbstractNewDialog, I7BaseDialog)

BEGIN_MESSAGE_MAP(AbstractNewDialog, I7BaseDialog)
  ON_BN_CLICKED(IDC_DIRPOPUP, OnClickedDirPopup)
  ON_EN_CHANGE(IDC_DIR, OnChangedEdit)
  ON_EN_CHANGE(IDC_NAME, OnChangedEdit)
  ON_EN_CHANGE(IDC_AUTHOR, OnChangedEdit)
END_MESSAGE_MAP()

AbstractNewDialog::AbstractNewDialog(CWnd* parent)
  : I7BaseDialog(AbstractNewDialog::IDD,FALSE,parent)
{
  m_author = theApp.GetProfileString("User",L"Name",L"");
}

void AbstractNewDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_DIR, m_dir);
  DDX_Text(pDX, IDC_NAME, m_name);
  DDX_TextW(pDX, IDC_AUTHOR, m_author);
}

void AbstractNewDialog::OnOK()
{
  UpdateData(TRUE);

  // Check that the project name is valid
  CString name = GetName();
  if (name.FindOneOf(invalid) != -1)
  {
    CString msg;
    msg.Format("The %s name cannot contain any of the following:\n%s",GetType(),invalid);
    MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Check that the project author is valid
  CStringW invalidW(invalid);
  if (GetAuthor().FindOneOf(invalidW) != -1)
  {
    CString msg;
    msg.Format("The author's name cannot contain any of the following:\n%s",invalid);
    MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }
  if (GetAuthor().CompareNoCase(L"Reserved") == 0)
  {
    MessageBox("The author cannot be called 'Reserved'",INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Check if the project directory is valid
  if (CheckPath() == false)
    return;

  // Save the name of the author
  theApp.WriteProfileString("User",L"Name",m_author);
  I7BaseDialog::OnOK();
}

BOOL AbstractNewDialog::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

  // Add auto-completion to the directory edit control
  theOS.SHAutoComplete(GetDlgItem(IDC_DIR),SHACF_FILESYSTEM);

  // The create button isn't enabled until all fields are filled in
  GetDlgItem(IDOK)->EnableWindow(FALSE);

  // Adjust all texts
  AddType(this);
  AddType(GetDlgItem(IDC_DIR_TEXT));
  AddType(GetDlgItem(IDC_NAME_TEXT));

  return TRUE;
}

int CALLBACK AbstractNewDialog::BrowseDirCB(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  AbstractNewDialog* dlg = (AbstractNewDialog*)lpData;
  if (uMsg == BFFM_INITIALIZED)
    ::SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)(LPCSTR)dlg->m_dir);
  return 0;
}

void AbstractNewDialog::OnClickedDirPopup()
{
  CString title;
  title.Format("Choose the directory to create the new %s in",GetType());

  // Use the the new "pick folders" file dialog mode in Vista,
  // and the earlier folder browser for XP and ealier.
  if ((theOS.GetWindowsVersion() >= 6) && theOS.IsAppThemed())
  {
    CComPtr<IFileDialog> dialog;
    if (FAILED(dialog.CoCreateInstance(__uuidof(FileOpenDialog))))
      return;
    dialog->SetTitle(CStringW(title));
    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options|FOS_PICKFOLDERS);

    CComPtr<IShellItem> si;
    if (SUCCEEDED(theOS.SHCreateItemFromParsingName(
      CStringW(m_dir),NULL,__uuidof(IShellItem),(void**)&si)))
    {
      dialog->SetFolder(si);
    }
    si.Release();

    if (FAILED(dialog->Show(GetSafeHwnd())))
      return;
    if (FAILED(dialog->GetResult(&si)))
      return;
    LPOLESTR path = NULL;
    if (FAILED(si->GetDisplayName(SIGDN_FILESYSPATH,&path)))
      return;

    UpdateData(TRUE);
    m_dir = CString(path);
    ::CoTaskMemFree(path);
    UpdateData(FALSE);

    // Update the create button state
    OnChangedEdit();
  }
  else
  {
    title.AppendChar('.');

    char dir[MAX_PATH];
    BROWSEINFO browse;
    ::ZeroMemory(&browse,sizeof browse);
    browse.hwndOwner = GetSafeHwnd();
    browse.pszDisplayName = dir;
    browse.lpszTitle = title;
    browse.ulFlags = BIF_USENEWUI|BIF_RETURNONLYFSDIRS;
    browse.lpfn = BrowseDirCB;
    browse.lParam = (LPARAM)this;

    LPITEMIDLIST item = ::SHBrowseForFolder(&browse);
    if (item == NULL)
      return;

    if (::SHGetPathFromIDList(item,dir))
    {
      UpdateData(TRUE);
      m_dir = dir;
      UpdateData(FALSE);

      // Update the create button state
      OnChangedEdit();
    }

    CComPtr<IMalloc> malloc;
    ::SHGetMalloc(&malloc);
    malloc->Free(item);
  }
}

void AbstractNewDialog::OnChangedEdit()
{
  UpdateData(TRUE);
  GetDlgItem(IDOK)->EnableWindow(
    !(m_dir.IsEmpty() || m_name.IsEmpty() || m_author.IsEmpty()));
}

CString AbstractNewDialog::GetName(void)
{
  CString name = m_name;
  int ext = name.Find('.');
  if (ext != -1)
    name.Truncate(ext);
  return name;
}

CStringW AbstractNewDialog::GetAuthor(void)
{
  return m_author;
}

void AbstractNewDialog::AddType(CWnd* wnd)
{
  CString format;
  wnd->GetWindowText(format);

  CString result;
  result.Format(format,GetType());
  wnd->SetWindowText(result);
}

IMPLEMENT_DYNAMIC(NewProjectDialog, AbstractNewDialog)

NewProjectDialog::NewProjectDialog(LPCSTR dir, CWnd* parent) : AbstractNewDialog(parent)
{
  // Find the parent directory of the default project directory
  m_dir = dir;
  int i = m_dir.ReverseFind('\\');
  if (i != -1)
    m_dir.Truncate(i);
  else
    m_dir.Empty();
}

const char* NewProjectDialog::GetType(void)
{
  return "project";
}

CString NewProjectDialog::GetPath(void)
{
  CString path(m_dir);
  int len = path.GetLength();
  if (len > 0)
  {
    if (path.GetAt(len-1) != '\\')
      path.AppendChar('\\');
  }
  path.Append(GetName());
  path.Append(".inform");
  return path;
}

bool NewProjectDialog::CheckPath(void)
{
  CString path = GetPath();
  if (::GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
  {
    // Create the project directory
    if (theOS.SHCreateDirectoryEx(this,path) != ERROR_SUCCESS)
    {
      CString msg;
      msg.Format("Failed to create the %s directory\n%s",GetType(),(LPCSTR)path);
      MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
      return false;
    }
  }
  else
  {
    int ask = MessageBox(
      "This project already exists. Do you want to continue, which will destroy the existing project "
      "and create a new one?",INFORM_TITLE,MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2);
    if (ask == IDNO)
      return false;
  }
  return true;
}

IMPLEMENT_DYNAMIC(NewExtensionDialog, AbstractNewDialog)

NewExtensionDialog::NewExtensionDialog(CWnd* parent) : AbstractNewDialog(parent)
{
  m_dir.Format("%s\\Inform\\Extensions",theApp.GetHomeDir());
}

BOOL NewExtensionDialog::OnInitDialog()
{
  if (AbstractNewDialog::OnInitDialog() == FALSE)
    return FALSE;

  GetDlgItem(IDC_DIR)->EnableWindow(FALSE);
  GetDlgItem(IDC_DIRPOPUP)->EnableWindow(FALSE);
  return TRUE;
}

const char* NewExtensionDialog::GetType(void)
{
  return "extension";
}

CString NewExtensionDialog::GetPath(void)
{
  CString path(m_dir);
  int len = path.GetLength();
  if (len > 0)
  {
    if (path.GetAt(len-1) != '\\')
      path.AppendChar('\\');
  }
  path.Append(CString(m_author));
  path.AppendChar('\\');
  path.Append(m_name);
  path.Append(".i7x");
  return path;
}

bool NewExtensionDialog::CheckPath(void)
{
  CString path(m_dir);
  int len = path.GetLength();
  if (len > 0)
  {
    if (path.GetAt(len-1) != '\\')
      path.AppendChar('\\');
  }
  path.Append(CString(m_author));

  if (::GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
  {
    // Create the extension author directory
    if (theOS.SHCreateDirectoryEx(this,path) != ERROR_SUCCESS)
    {
      CString msg;
      msg.Format("Failed to create the %s directory\n%s",GetType(),(LPCSTR)path);
      MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
      return false;
    }
  }
  return true;
}
