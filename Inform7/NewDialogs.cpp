#include "stdafx.h"
#include "NewDialogs.h"
#include "Inform.h"
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
  : I7BaseDialog(AbstractNewDialog::IDD,parent)
{
  m_author = theApp.GetProfileString("User",L"Name",L"");
}

void AbstractNewDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_DIR, m_dirEdit);
  DDX_Control(pDX, IDC_DIRPOPUP, m_dirPopup);
  DDX_Text(pDX, IDC_DIR, m_dir);
  DDX_Control(pDX, IDC_NAME, m_nameEdit);
  DDX_Text(pDX, IDC_NAME, m_name);
  DDX_Control(pDX, IDC_AUTHOR, m_authorEdit);
  DDX_TextW(pDX, IDC_AUTHOR, m_author);
  DDX_Control(pDX, IDOK, m_start);
  DDX_Control(pDX, IDCANCEL, m_cancel);
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
  if (m_author.FindOneOf(invalidW) != -1)
  {
    CString msg;
    msg.Format("The author's name cannot contain any of the following:\n%s",invalid);
    MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }
  if (m_author.CompareNoCase(L"Reserved") == 0)
  {
    MessageBox("The author cannot be called 'Reserved'",INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Check if the project directory is valid
  if (CheckPath() == false)
    return;

  // Save the name of the author
  if (GetDlgItem(IDC_AUTHOR)->IsWindowEnabled())
    theApp.WriteProfileString("User",L"Name",m_author);
  I7BaseDialog::OnOK();
}

BOOL AbstractNewDialog::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

  // Add auto-completion to the directory edit control
  ::SHAutoComplete(GetDlgItem(IDC_DIR)->GetSafeHwnd(),SHACF_FILESYSTEM);

  // The create button isn't enabled until all fields are filled in
  GetDlgItem(IDOK)->EnableWindow(
    !(m_dir.IsEmpty() || m_name.IsEmpty() || m_author.IsEmpty()));

  // Adjust all texts
  AddType(this);
  AddType(GetDlgItem(IDC_DIR_TEXT));
  AddType(GetDlgItem(IDC_NAME_TEXT));

  return TRUE;
}

void AbstractNewDialog::OnClickedDirPopup()
{
  UpdateData(TRUE);

  CString title;
  title.Format("Choose the directory to create the new %s in",GetType());
  CString dir = theApp.PickDirectory(title,NULL,NULL,m_dir,this);
  if (!dir.IsEmpty())
  {
    m_dir = dir;
    UpdateData(FALSE);

    // Update the create button state
    OnChangedEdit();
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

  // Curly quotes and double quotes cause problems with mapping between the Windows code page,
  // ISO Latin-1 and UTF-8, so we just avoid this by replacing them with 'ordinary' quotes.
  name.Replace((char)0x91,'\'');
  name.Replace((char)0x92,'\'');
  name.Replace((char)0x93,'\"');
  name.Replace((char)0x94,'\"');
  return name;
}

CStringW AbstractNewDialog::GetAuthor(void)
{
  // Quote the author's name if it contains a period
  if (m_author.Find(L'.') != -1)
  {
    CStringW author;
    author.Format(L"\"%s\"",m_author);
    return author;
  }
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

NewProjectDialog::NewProjectDialog(ProjectType projectType, LPCSTR dir, CWnd* parent)
  : AbstractNewDialog(parent), m_projectType(projectType), m_fromExt(false)
{
  // Find the parent directory of the default project directory
  m_dir = dir;
  int i = m_dir.ReverseFind('\\');
  if (i != -1)
    m_dir.Truncate(i);
  else
    m_dir.Empty();
}

void NewProjectDialog::FromExt(const char* name, const char* author)
{
  ASSERT(m_projectType == Project_I7XP);
  m_name = name;
  m_author = author;
  m_fromExt = true;
}

BOOL NewProjectDialog::OnInitDialog()
{
  if (AbstractNewDialog::OnInitDialog() == FALSE)
    return FALSE;

  GetDlgItem(IDC_AUTHOR)->EnableWindow(m_fromExt ? FALSE : TRUE);
  if (m_fromExt)
    SetWindowText("Start an extension project from an extension");
  return TRUE;
}

const char* NewProjectDialog::GetType(void)
{
  switch (m_projectType)
  {
  case Project_I7:
    return "project";
  case Project_I7XP:
    return "extension project";
  default:
    ASSERT(0);
    break;
  }
  return 0;
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
  switch (m_projectType)
  {
  case Project_I7:
    path.Append(".inform");
    break;
  case Project_I7XP:
    path.Append(".i7xp");
    break;
  default:
    ASSERT(0);
    break;
  }
  return path;
}

bool NewProjectDialog::CheckPath(void)
{
  CString path = GetPath();
  if (::GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
  {
    // Create the project directory
    if (::SHCreateDirectoryEx(GetSafeHwnd(),path,NULL) != ERROR_SUCCESS)
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
    if (::SHCreateDirectoryEx(GetSafeHwnd(),path,NULL) != ERROR_SUCCESS)
    {
      CString msg;
      msg.Format("Failed to create the %s directory\n%s",GetType(),(LPCSTR)path);
      MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
      return false;
    }
  }
  return true;
}
