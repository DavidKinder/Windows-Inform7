#include "stdafx.h"
#include "PrefsDialog.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PrefsDialog::PrefsDialog() : I7BaseDialog(PrefsDialog::IDD,FALSE)
{
  // Set the default preferences values
  m_tabSize = 8;
  m_indentWrapped = TRUE;
  m_autoIndent = TRUE;
  m_startWithLast = FALSE;
  m_cleanFiles = TRUE;
  m_cleanIndexes = TRUE;
  m_glulxTerp = "Glulxe";
}

BEGIN_MESSAGE_MAP(PrefsDialog, I7BaseDialog)
  ON_BN_CLICKED(IDC_CLEANFILES, OnBnClickedCleanFiles)
END_MESSAGE_MAP()

void PrefsDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_TABSIZE, m_tabSize);
  DDX_Check(pDX, IDC_INDENT, m_indentWrapped);
  DDX_Check(pDX, IDC_AUTO_INDENT, m_autoIndent);
  DDX_Check(pDX, IDC_STARTLAST, m_startWithLast);
  DDV_MinMaxUInt(pDX, m_tabSize, 1, 32);
  DDX_Check(pDX, IDC_CLEANFILES, m_cleanFiles);
  DDX_Check(pDX, IDC_CLEANINDEX, m_cleanIndexes);
  DDX_Control(pDX, IDC_CLEANFILES, m_cleanFilesCheck);
  DDX_Control(pDX, IDC_CLEANINDEX, m_cleanIndexCheck);
  DDX_CBString(pDX, IDC_GLULX, m_glulxTerp);
}

BOOL PrefsDialog::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

  UpdateControlStates();
  return TRUE;
}

INT_PTR PrefsDialog::DoModal()
{
  // Get the current settings
  {
    CRegKey registryKey;
    if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_READ) == ERROR_SUCCESS)
    {
      DWORD value;
      if (registryKey.QueryDWORDValue("Source Tab Size Chars",value) == ERROR_SUCCESS)
        m_tabSize = value;
      if (registryKey.QueryDWORDValue("Indent Wrapped Lines",value) == ERROR_SUCCESS)
        m_indentWrapped = value;
      if (registryKey.QueryDWORDValue("Auto Indent",value) == ERROR_SUCCESS)
        m_autoIndent = value;

      if (registryKey.QueryDWORDValue("Clean Up Files",value) == ERROR_SUCCESS)
        m_cleanFiles = (value != 0);
      if (registryKey.QueryDWORDValue("Clean Up Indexes",value) == ERROR_SUCCESS)
        m_cleanIndexes = (value != 0);
    }

    m_startWithLast = (theApp.GetProfileInt("Start","Open Last Project",0) != 0);
    m_glulxTerp = theApp.CWinApp::GetProfileString("Game","Glulx Interpreter",m_glulxTerp);
  }

  // Show the dialog
  INT_PTR result = I7BaseDialog::DoModal();
  if (result == IDOK)
  {
    // Store the new settings
    CRegKey registryKey;
    if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
    {
      registryKey.SetDWORDValue("Source Tab Size Chars",m_tabSize);
      registryKey.SetDWORDValue("Indent Wrapped Lines",m_indentWrapped);
      registryKey.SetDWORDValue("Auto Indent",m_autoIndent);
      registryKey.SetDWORDValue("Clean Up Files",m_cleanFiles);
      registryKey.SetDWORDValue("Clean Up Indexes",m_cleanIndexes);
    }
    theApp.WriteProfileInt("Start","Open Last Project",m_startWithLast ? 1 : 0);
    theApp.CWinApp::WriteProfileString("Game","Glulx Interpreter",m_glulxTerp);

    // Notify all project windows
    theApp.SendAllFrames(InformApp::Preferences,0);
  }
  return result;
}

void PrefsDialog::OnBnClickedCleanFiles()
{
  UpdateControlStates();
}

void PrefsDialog::UpdateControlStates(void)
{
  bool cleanFiles = (m_cleanFilesCheck.GetCheck() == BST_CHECKED);
  m_cleanIndexCheck.EnableWindow(cleanFiles);
}
