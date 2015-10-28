#include "stdafx.h"
#include "PrefsDialog.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PrefsDialog::PrefsDialog() : I7BaseDialog(PrefsDialog::IDD)
{
  // Set the default preferences values
  m_fontName = theApp.GetFontName(InformApp::FontDisplay);
  m_fixedFontName = theApp.GetFontName(InformApp::FontFixedWidth);
  m_fontSize.Format("%d",theApp.GetFontSize(InformApp::FontDisplay));
  m_tabSize = 8;
  m_indentWrapped = TRUE;
  m_autoIndent = TRUE;
  m_autoSpaceTables = TRUE;
  m_autoNumber = FALSE;
  m_startWithLast = FALSE;
  m_cleanFiles = TRUE;
  m_cleanIndexes = TRUE;
  m_I6debug = FALSE;
  m_glulxTerp = "Glulxe";
}

BEGIN_MESSAGE_MAP(PrefsDialog, I7BaseDialog)
  ON_BN_CLICKED(IDC_CLEANFILES, OnBnClickedCleanFiles)
END_MESSAGE_MAP()

void PrefsDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FONT, m_font);
  DDX_Control(pDX, IDC_FIXEDFONT, m_fixedFont);
  DDX_CBString(pDX, IDC_FONTSIZE, m_fontSize);
  DDX_Text(pDX, IDC_TABSIZE, m_tabSize);
  DDX_Check(pDX, IDC_INDENT, m_indentWrapped);
  DDX_Check(pDX, IDC_AUTO_INDENT, m_autoIndent);
  DDX_Check(pDX, IDC_ELASTIC_TABS, m_autoSpaceTables);
  DDX_Check(pDX, IDC_AUTO_NUMBER, m_autoNumber);
  DDX_Check(pDX, IDC_STARTLAST, m_startWithLast);
  DDV_MinMaxUInt(pDX, m_tabSize, 1, 32);
  DDX_Check(pDX, IDC_CLEANFILES, m_cleanFiles);
  DDX_Check(pDX, IDC_CLEANINDEX, m_cleanIndexes);
  DDX_Check(pDX, IDC_I6DEBUGGING, m_I6debug);
  DDX_Control(pDX, IDC_CLEANFILES, m_cleanFilesCheck);
  DDX_Control(pDX, IDC_CLEANINDEX, m_cleanIndexCheck);
  DDX_CBString(pDX, IDC_GLULX, m_glulxTerp);
}

BOOL PrefsDialog::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

  // Set a smaller font for the "Also clean out ..." checkbox
  LOGFONT smallFont;
  GetFont()->GetLogFont(&smallFont);
  smallFont.lfHeight = (LONG)(smallFont.lfHeight*0.9);
  m_smallFont.CreateFontIndirect(&smallFont);
  m_cleanIndexCheck.SetFont(&m_smallFont);

  // Get all the possible fonts
  CDC* dc = GetDC();
  LOGFONT font;
  ::ZeroMemory(&font,sizeof font);
  font.lfCharSet = ANSI_CHARSET;
  ::EnumFontFamiliesEx(dc->GetSafeHdc(),&font,(FONTENUMPROC)ListFonts,(LPARAM)this,0);
  ReleaseDC(dc);

  // Initialize the font controls
  if (m_font.SelectString(-1,m_fontName) == CB_ERR)
    m_font.SetCurSel(0);
  if (m_fixedFont.SelectString(-1,m_fixedFontName) == CB_ERR)
    m_fixedFont.SetCurSel(0);

  UpdateControlStates();
  return TRUE;
}

void PrefsDialog::OnOK()
{
  m_font.GetWindowText(m_fontName);
  m_fixedFont.GetWindowText(m_fixedFontName);
  I7BaseDialog::OnOK();
}

INT_PTR PrefsDialog::DoModal()
{
  // Get the current settings
  {
    CRegKey registryKey;
    if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_READ) == ERROR_SUCCESS)
    {
      char fontName[MAX_PATH];
      DWORD value = sizeof fontName;
      if (registryKey.QueryStringValue("Font Name",fontName,&value) == ERROR_SUCCESS)
        m_fontName = fontName;
      value = sizeof fontName;
      if (registryKey.QueryStringValue("Fixed Font Name",fontName,&value) == ERROR_SUCCESS)
        m_fixedFontName = fontName;
      if (registryKey.QueryDWORDValue("Font Size",value) == ERROR_SUCCESS)
        m_fontSize.Format("%d",value);

      if (registryKey.QueryDWORDValue("Source Tab Size Chars",value) == ERROR_SUCCESS)
        m_tabSize = value;
      if (registryKey.QueryDWORDValue("Indent Wrapped Lines",value) == ERROR_SUCCESS)
        m_indentWrapped = value;
      if (registryKey.QueryDWORDValue("Auto Indent",value) == ERROR_SUCCESS)
        m_autoIndent = value;
      if (registryKey.QueryDWORDValue("Auto Space Tables",value) == ERROR_SUCCESS)
        m_autoSpaceTables = value;
      if (registryKey.QueryDWORDValue("Auto Number Sections",value) == ERROR_SUCCESS)
        m_autoNumber = value;

      if (registryKey.QueryDWORDValue("Clean Up Files",value) == ERROR_SUCCESS)
        m_cleanFiles = (value != 0);
      if (registryKey.QueryDWORDValue("Clean Up Indexes",value) == ERROR_SUCCESS)
        m_cleanIndexes = (value != 0);
      if (registryKey.QueryDWORDValue("Generate I6 Debug",value) == ERROR_SUCCESS)
        m_I6debug = (value != 0);
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
      registryKey.SetStringValue("Font Name",m_fontName);
      registryKey.SetStringValue("Fixed Font Name",m_fixedFontName);
      int fontSize = 0;
      if (sscanf(m_fontSize,"%d",&fontSize) == 1)
        registryKey.SetDWORDValue("Font Size",fontSize);

      registryKey.SetDWORDValue("Source Tab Size Chars",m_tabSize);
      registryKey.SetDWORDValue("Indent Wrapped Lines",m_indentWrapped);
      registryKey.SetDWORDValue("Auto Indent",m_autoIndent);
      registryKey.SetDWORDValue("Auto Space Tables",m_autoSpaceTables);
      registryKey.SetDWORDValue("Auto Number Sections",m_autoNumber);
      registryKey.SetDWORDValue("Clean Up Files",m_cleanFiles);
      registryKey.SetDWORDValue("Clean Up Indexes",m_cleanIndexes);
      registryKey.SetDWORDValue("Generate I6 Debug",m_I6debug);
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

// Called when enumerating fonts, populates the font drop down lists in the dialog
int CALLBACK PrefsDialog::ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param)
{
  PrefsDialog* dialog = (PrefsDialog*)param;
  if (font->elfLogFont.lfFaceName[0] != '@')
  {
    dialog->m_font.AddString(font->elfLogFont.lfFaceName);
    if (font->elfLogFont.lfPitchAndFamily & FIXED_PITCH)
      dialog->m_fixedFont.AddString(font->elfLogFont.lfFaceName);
  }
  return 1;
}
