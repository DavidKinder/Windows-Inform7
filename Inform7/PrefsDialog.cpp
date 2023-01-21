#include "stdafx.h"
#include "PrefsDialog.h"
#include "Inform.h"
#include "Messages.h"

#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static LPCWSTR previewText =
  L"Section 5 - Tight Passage and Inscription\n"
  L"\n"
  L"[This sample is a jumble, just to demonstrate syntax highlighting.]\n"
  L"\n"
  L"Tight Passage is northeast of Father's Regret. \"The passage through rock ends here, and begins to tunnel through soil instead.\" The inscription is a fixed in place thing in Tight Passage. \"You have the words by heart now: [italic type][printing of the inscription][roman type].\"\n"
  L"\n"
  L"Table of Tour de France Jerseys\n"
  L"jersey\tyear established\tcitation\n"
  L"a yellow jersey\t1919\t\"race leader\"\n"
  L"a polkadot jersey\t1933\t\"King of the Mountains\"\n"
  L"a green jersey\t1953\t\"highest point scorer on sprints\"\n"
  L"a white jersey\t1975\t\"best cyclist aged 25 or less\"";

#define NUM_PREVIEW_TABS 60

IMPLEMENT_DYNAMIC(PrefsEditPage, CPropertyPage)

BEGIN_MESSAGE_MAP(PrefsEditPage, CPropertyPage)
  ON_BN_CLICKED(IDC_RESTORE, OnClickedRestore)
  ON_CBN_SELCHANGE(IDC_FONT, OnChangeFont)
  ON_CBN_SELCHANGE(IDC_FONTSIZE, OnChangeFont)
  ON_CBN_EDITCHANGE(IDC_FONTSIZE, OnChangeFont)
  ON_BN_CLICKED(IDC_ENABLE_STYLES, OnClickedEnableStyles)
  ON_BN_CLICKED(IDC_HEAD_BOLD, OnChangeStyle)
  ON_BN_CLICKED(IDC_HEAD_ITALIC, OnChangeStyle)
  ON_BN_CLICKED(IDC_HEAD_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_HEAD_SIZE, OnChangeStyle)
  ON_BN_CLICKED(IDC_MAIN_BOLD, OnChangeStyle)
  ON_BN_CLICKED(IDC_MAIN_ITALIC, OnChangeStyle)
  ON_BN_CLICKED(IDC_MAIN_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_MAIN_SIZE, OnChangeStyle)
  ON_BN_CLICKED(IDC_COMMENT_BOLD, OnChangeStyle)
  ON_BN_CLICKED(IDC_COMMENT_ITALIC, OnChangeStyle)
  ON_BN_CLICKED(IDC_COMMENT_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_COMMENT_SIZE, OnChangeStyle)
  ON_BN_CLICKED(IDC_QUOTE_BOLD, OnChangeStyle)
  ON_BN_CLICKED(IDC_QUOTE_ITALIC, OnChangeStyle)
  ON_BN_CLICKED(IDC_QUOTE_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_QUOTE_SIZE, OnChangeStyle)
  ON_BN_CLICKED(IDC_SUBST_BOLD, OnChangeStyle)
  ON_BN_CLICKED(IDC_SUBST_ITALIC, OnChangeStyle)
  ON_BN_CLICKED(IDC_SUBST_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_SUBST_SIZE, OnChangeStyle)
  ON_BN_CLICKED(IDC_ELASTIC_TABS, OnChangeStyle)
  ON_WM_HSCROLL()
  ON_MESSAGE(WM_AFTERFONTSET, OnAfterFontSet)
  ON_MESSAGE(WM_UPDATEPREVIEW, OnUpdatePreview)
END_MESSAGE_MAP()

PrefsEditPage::PrefsEditPage(PrefsDialog* dlg)
  : CPropertyPage(PrefsEditPage::IDD), m_dialog(dlg)
{
  SetDefaults();
}

void PrefsEditPage::ReadSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
  {
    char fontName[MAX_PATH];
    DWORD value = sizeof fontName;
    if (registryKey.QueryStringValue("Font Name",fontName,&value) == ERROR_SUCCESS)
      m_fontName = fontName;
    if (registryKey.QueryDWORDValue("Font Size",value) == ERROR_SUCCESS)
      m_fontSize.Format("%d",value);

    if (registryKey.QueryDWORDValue("Syntax Highlighting",value) == ERROR_SUCCESS)
      m_styles = (value != 0);
    if (registryKey.QueryDWORDValue("Headings Style",value) == ERROR_SUCCESS)
    {
      m_boldHead.SetIsChecked((value & 2) != 0);
      m_italicHead.SetIsChecked((value & 1) != 0);
    }
    if (registryKey.QueryDWORDValue("Main Text Style",value) == ERROR_SUCCESS)
    {
      m_boldMain.SetIsChecked((value & 2) != 0);
      m_italicMain.SetIsChecked((value & 1) != 0);
    }
    if (registryKey.QueryDWORDValue("Comments Style",value) == ERROR_SUCCESS)
    {
      m_boldComment.SetIsChecked((value & 2) != 0);
      m_italicComment.SetIsChecked((value & 1) != 0);
    }
    if (registryKey.QueryDWORDValue("Quoted Text Style",value) == ERROR_SUCCESS)
    {
      m_boldQuote.SetIsChecked((value & 2) != 0);
      m_italicQuote.SetIsChecked((value & 1) != 0);
    }
    if (registryKey.QueryDWORDValue("Substitutions Style",value) == ERROR_SUCCESS)
    {
      m_boldSubst.SetIsChecked((value & 2) != 0);
      m_italicSubst.SetIsChecked((value & 1) != 0);
    }
    if (registryKey.QueryDWORDValue("Headings Underline",value) == ERROR_SUCCESS)
      m_underHead.SetIsChecked(value != 0);
    if (registryKey.QueryDWORDValue("Main Text Underline",value) == ERROR_SUCCESS)
      m_underMain.SetIsChecked(value != 0);
    if (registryKey.QueryDWORDValue("Comments Underline",value) == ERROR_SUCCESS)
      m_underComment.SetIsChecked(value != 0);
    if (registryKey.QueryDWORDValue("Quoted Text Underline",value) == ERROR_SUCCESS)
      m_underQuote.SetIsChecked(value != 0);
    if (registryKey.QueryDWORDValue("Substitutions Underline",value) == ERROR_SUCCESS)
      m_underSubst.SetIsChecked(value != 0);
    if (registryKey.QueryDWORDValue("Headings Size",value) == ERROR_SUCCESS)
      m_sizeHead = (int)value;
    if (registryKey.QueryDWORDValue("Main Text Size",value) == ERROR_SUCCESS)
      m_sizeMain = (int)value;
    if (registryKey.QueryDWORDValue("Comments Size",value) == ERROR_SUCCESS)
      m_sizeComment = (int)value;
    if (registryKey.QueryDWORDValue("Quoted Text Size",value) == ERROR_SUCCESS)
      m_sizeQuote = (int)value;
    if (registryKey.QueryDWORDValue("Substitutions Size",value) == ERROR_SUCCESS)
      m_sizeSubst = (int)value;

    if (registryKey.QueryDWORDValue("Source Tab Size Chars",value) == ERROR_SUCCESS)
      m_tabSize = value;
    if (registryKey.QueryDWORDValue("Auto Indent",value) == ERROR_SUCCESS)
      m_autoIndent = value;
    if (registryKey.QueryDWORDValue("Auto Space Tables",value) == ERROR_SUCCESS)
      m_autoSpaceTables = value;
    if (registryKey.QueryDWORDValue("Auto Number Sections",value) == ERROR_SUCCESS)
      m_autoNumber = value;
  }
}

void PrefsEditPage::WriteSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
  {
    registryKey.SetStringValue("Font Name",m_fontName);
    int fontSize = 0;
    if (sscanf(m_fontSize,"%d",&fontSize) == 1)
      registryKey.SetDWORDValue("Font Size",fontSize);

    registryKey.SetDWORDValue("Syntax Highlighting",m_styles);
    registryKey.SetDWORDValue("Headings Style",(m_boldHead.GetIsChecked() ? 2 : 0) | (m_italicHead.GetIsChecked() ? 1 : 0));
    registryKey.SetDWORDValue("Main Text Style",(m_boldMain.GetIsChecked() ? 2 : 0) | (m_italicMain.GetIsChecked() ? 1 : 0));
    registryKey.SetDWORDValue("Comments Style",(m_boldComment.GetIsChecked() ? 2 : 0) | (m_italicComment.GetIsChecked() ? 1 : 0));
    registryKey.SetDWORDValue("Quoted Text Style",(m_boldQuote.GetIsChecked() ? 2 : 0) | (m_italicQuote.GetIsChecked() ? 1 : 0));
    registryKey.SetDWORDValue("Substitutions Style",(m_boldSubst.GetIsChecked() ? 2 : 0) | (m_italicSubst.GetIsChecked() ? 1 : 0));
    registryKey.SetDWORDValue("Headings Underline",m_underHead.GetIsChecked() ? 1 : 0);
    registryKey.SetDWORDValue("Main Text Underline",m_underMain.GetIsChecked() ? 1 : 0);
    registryKey.SetDWORDValue("Comments Underline",m_underComment.GetIsChecked() ? 1 : 0);
    registryKey.SetDWORDValue("Quoted Text Underline",m_underQuote.GetIsChecked() ? 1 : 0);
    registryKey.SetDWORDValue("Substitutions Underline",m_underSubst.GetIsChecked() ? 1 : 0);
    registryKey.SetDWORDValue("Headings Size",m_sizeHead);
    registryKey.SetDWORDValue("Main Text Size",m_sizeMain);
    registryKey.SetDWORDValue("Comments Size",m_sizeComment);
    registryKey.SetDWORDValue("Quoted Text Size",m_sizeQuote);
    registryKey.SetDWORDValue("Substitutions Size",m_sizeSubst);

    registryKey.SetDWORDValue("Source Tab Size Chars",m_tabSize);
    registryKey.SetDWORDValue("Auto Indent",m_autoIndent);
    registryKey.SetDWORDValue("Auto Space Tables",m_autoSpaceTables);
    registryKey.SetDWORDValue("Auto Number Sections",m_autoNumber);
  }
}

void PrefsEditPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FONT, m_font);
  DDX_CBString(pDX, IDC_FONTSIZE, m_fontSize);
  DDX_Check(pDX, IDC_ENABLE_STYLES, m_styles);
  DDX_Control(pDX,IDC_ENABLE_STYLES, m_stylesCheck);
  DDX_CBIndex(pDX, IDC_HEAD_SIZE, m_sizeHead);
  DDX_CBIndex(pDX, IDC_MAIN_SIZE, m_sizeMain);
  DDX_CBIndex(pDX, IDC_COMMENT_SIZE, m_sizeComment);
  DDX_CBIndex(pDX, IDC_QUOTE_SIZE, m_sizeQuote);
  DDX_CBIndex(pDX, IDC_SUBST_SIZE, m_sizeSubst);
  DDX_Control(pDX, IDC_TABSIZE, m_tabSizeCtrl);
  DDX_Slider(pDX, IDC_TABSIZE, m_tabSize);
  DDX_Check(pDX, IDC_AUTO_INDENT, m_autoIndent);
  DDX_Check(pDX, IDC_ELASTIC_TABS, m_autoSpaceTables);
  DDX_Check(pDX, IDC_AUTO_NUMBER, m_autoNumber);
}

BOOL PrefsEditPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // Get all the possible fonts
  CDC* dc = GetDC();
  LOGFONT font;
  ::ZeroMemory(&font,sizeof font);
  font.lfCharSet = ANSI_CHARSET;
  ::EnumFontFamiliesEx(dc->GetSafeHdc(),&font,(FONTENUMPROC)ListFonts,(LPARAM)this,0);
  ReleaseDC(dc);

  // Initialize the font control
  if (m_font.SelectString(-1,m_fontName) == CB_ERR)
    m_font.SetCurSel(0);

  // Subclass dialog controls
  m_boldHead.SubclassDlgItem(IDC_HEAD_BOLD,this);
  m_boldMain.SubclassDlgItem(IDC_MAIN_BOLD,this);
  m_boldComment.SubclassDlgItem(IDC_COMMENT_BOLD,this);
  m_boldQuote.SubclassDlgItem(IDC_QUOTE_BOLD,this);
  m_boldSubst.SubclassDlgItem(IDC_SUBST_BOLD,this);
  m_italicHead.SubclassDlgItem(IDC_HEAD_ITALIC,this);
  m_italicMain.SubclassDlgItem(IDC_MAIN_ITALIC,this);
  m_italicComment.SubclassDlgItem(IDC_COMMENT_ITALIC,this);
  m_italicQuote.SubclassDlgItem(IDC_QUOTE_ITALIC,this);
  m_italicSubst.SubclassDlgItem(IDC_SUBST_ITALIC,this);
  m_underHead.SubclassDlgItem(IDC_HEAD_UNDER,this);
  m_underMain.SubclassDlgItem(IDC_MAIN_UNDER,this);
  m_underComment.SubclassDlgItem(IDC_COMMENT_UNDER,this);
  m_underQuote.SubclassDlgItem(IDC_QUOTE_UNDER,this);
  m_underSubst.SubclassDlgItem(IDC_SUBST_UNDER,this);

  // Initialize the tab width slider
  m_tabSizeCtrl.SetRange(2,32);
  m_tabSizeCtrl.SetTicFreq(1);

  // Create the preview window
  m_preview.Create(this,Project_I7,SourceWindow::Border);
  CRect previewRect;
  GetDlgItem(IDC_PREVIEW)->GetWindowRect(previewRect);
  ScreenToClient(previewRect);
  m_preview.MoveWindow(previewRect);
  SourceEdit& previewEdit = m_preview.GetEdit();
  m_preview.LoadSettings(*m_dialog);
  previewEdit.ReplaceSelect(previewText);
  previewEdit.SetReadOnly(true);
  previewEdit.HideCaret();
  CHARRANGE startRange = { 0,0 };
  previewEdit.SetSelect(startRange);
  m_preview.ShowWindow(SW_SHOW);

  // Create the tab preview window
  m_tabPreview.Create(this,Project_I7,SourceWindow::SingleLine);
  GetDlgItem(IDC_TABPREVIEW)->GetWindowRect(previewRect);
  ScreenToClient(previewRect);
  m_tabPreview.MoveWindow(previewRect);
  SourceEdit& tabPreviewEdit = m_tabPreview.GetEdit();
  tabPreviewEdit.ReplaceSelect(
    L"\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|"
    L"\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|"
    L"\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|\t|");
  tabPreviewEdit.SetCustomTabStops(NUM_PREVIEW_TABS,
    previewEdit.GetTabWidthPixels());
  tabPreviewEdit.SetReadOnly(true);
  tabPreviewEdit.SetShowScrollBars(false);
  tabPreviewEdit.SetLineWrap(false);
  tabPreviewEdit.HideCaret();
  tabPreviewEdit.DisableUserControl();
  tabPreviewEdit.SetSelect(startRange);
  m_tabPreview.ShowWindow(SW_SHOW);

  UpdateControlStates();
  return TRUE;
}

void PrefsEditPage::OnOK()
{
  m_font.GetWindowText(m_fontName);
  CPropertyPage::OnOK();
}

void PrefsEditPage::OnClickedRestore()
{
  LPCWSTR head = L"Reset the editing preferences?";
  LPCWSTR msg = L"This action cannot be undone.";
  int btn = 0;
  if (SUCCEEDED(::TaskDialog(GetSafeHwnd(),0,L_INFORM_TITLE,head,msg,
    TDCBF_YES_BUTTON|TDCBF_NO_BUTTON,TD_WARNING_ICON,&btn)))
  {
    if (btn == IDYES)
    {
      SetDefaults();

      // Update the controls to match the defaults
      UpdateData(FALSE);
      m_font.SelectString(-1,m_fontName);
      UpdateControlStates();
      m_dialog->UpdatePreviews();
      m_tabPreview.GetEdit().SetCustomTabStops(NUM_PREVIEW_TABS,
        m_preview.GetEdit().GetTabWidthPixels());
    }
  }
}

void PrefsEditPage::OnChangeFont()
{
  PostMessage(WM_UPDATEPREVIEW);
}

void PrefsEditPage::OnChangeStyle()
{
  PostMessage(WM_UPDATEPREVIEW);
}

void PrefsEditPage::OnClickedEnableStyles()
{
  UpdateControlStates();
  UpdatePreview();
}

void PrefsEditPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  PostMessage(WM_UPDATEPREVIEW);
}

LRESULT PrefsEditPage::OnAfterFontSet(WPARAM, LPARAM)
{
  // Font scaling can leave uneven spacing between the syntax highlighting controls ...
  CRect topRect, bottomRect;
  GetDlgItem(IDC_HEAD_SIZE)->GetWindowRect(topRect);
  ScreenToClient(topRect);
  GetDlgItem(IDC_SUBST_SIZE)->GetWindowRect(bottomRect);
  ScreenToClient(bottomRect);
  int rowh = topRect.Height();
  int space = (bottomRect.bottom - topRect.top - (5*rowh) + 3) / 5;
  AdjustControlRow(IDC_MAIN_SIZE,topRect.top + rowh + space,IDC_MAIN_LABEL,IDC_MAIN_SIZE);
  AdjustControlRow(IDC_COMMENT_SIZE,topRect.top + 2*(rowh + space),IDC_COMMENT_LABEL,IDC_COMMENT_SIZE);
  AdjustControlRow(IDC_QUOTE_SIZE,topRect.top + 3*(rowh + space),IDC_QUOTE_LABEL,IDC_QUOTE_SIZE);
  AdjustControlRow(IDC_SUBST_SIZE,topRect.top + 4*(rowh + space),IDC_SUBST_LABEL,IDC_SUBST_SIZE);

  UpdatePreview();
  return 0;
}

LRESULT PrefsEditPage::OnUpdatePreview(WPARAM, LPARAM)
{
  UpdatePreview();
  return 0;
}

void PrefsEditPage::UpdateControlStates(void)
{
  bool stylesEnabled = (m_stylesCheck.GetCheck() == BST_CHECKED);
  for (int id = IDC_HEAD_LABEL; id <= IDC_SUBST_SIZE; id++)
    GetDlgItem(id)->EnableWindow(stylesEnabled);
}

void PrefsEditPage::UpdatePreview(void)
{
  UpdateData(TRUE);
  m_font.GetWindowText(m_fontName);

  m_dialog->UpdatePreviews();
  m_tabPreview.GetEdit().SetCustomTabStops(NUM_PREVIEW_TABS,
    m_preview.GetEdit().GetTabWidthPixels());
}

void PrefsEditPage::PreviewChanged(void)
{
  m_preview.LoadSettings(*m_dialog);
  m_preview.PrefsChanged();
}

// Set the default preferences values
void PrefsEditPage::SetDefaults(void)
{
  m_fontName = theApp.GetFontName(InformApp::FontDisplay);
  m_fontSize.Format("%d",theApp.GetFontSize(InformApp::FontDisplay));

  m_styles = TRUE;
  m_boldHead.SetIsChecked(true);
  m_boldMain.SetIsChecked(false);
  m_boldComment.SetIsChecked(true);
  m_boldQuote.SetIsChecked(true);
  m_boldSubst.SetIsChecked(false);
  m_italicHead.SetIsChecked(false);
  m_italicMain.SetIsChecked(false);
  m_italicComment.SetIsChecked(false);
  m_italicQuote.SetIsChecked(false);
  m_italicSubst.SetIsChecked(false);
  m_underHead.SetIsChecked(false);
  m_underMain.SetIsChecked(false);
  m_underComment.SetIsChecked(false);
  m_underQuote.SetIsChecked(false);
  m_underSubst.SetIsChecked(false);
  m_sizeHead = 0; // Normal
  m_sizeMain = 0;
  m_sizeComment = 1; // Small
  m_sizeQuote = 0;
  m_sizeSubst = 0;

  m_tabSize = 8;
  m_autoIndent = TRUE;
  m_autoSpaceTables = TRUE;
  m_autoNumber = TRUE;
}

void PrefsEditPage::AdjustControlRow(int ctrlId, int top, int ctrlId1, int ctrlId2)
{
  CRect ctrlRect;
  GetDlgItem(ctrlId)->GetWindowRect(ctrlRect);
  ScreenToClient(ctrlRect);
  int adjust = top - ctrlRect.top;

  for (int id = ctrlId1; id <= ctrlId2; id++)
  {
    CWnd* wnd = GetDlgItem(id);
    wnd->GetWindowRect(ctrlRect);
    ScreenToClient(ctrlRect);
    ctrlRect.OffsetRect(0,adjust);
    wnd->MoveWindow(ctrlRect);
  }
}

bool PrefsEditPage::GetDWord(const char* name, DWORD& value)
{
  CString n(name);
  if (n == "Font Size")
  {
    int fontSize = 9;
    sscanf(m_fontSize,"%d",&fontSize);
    value = fontSize;
    return true;
  }
  else if (n == "Syntax Highlighting")
  {
    value = m_styles;
    return true;
  }
  else if (n == "Headings Style")
  {
    value = (m_boldHead.GetIsChecked() ? 2 : 0) | (m_italicHead.GetIsChecked() ? 1 : 0);
    return true;
  }
  else if (n == "Main Text Style")
  {
    value = (m_boldMain.GetIsChecked() ? 2 : 0) | (m_italicMain.GetIsChecked() ? 1 : 0);
    return true;
  }
  else if (n == "Comments Style")
  {
    value = (m_boldComment.GetIsChecked() ? 2 : 0) | (m_italicComment.GetIsChecked() ? 1 : 0);
    return true;
  }
  else if (n == "Quoted Text Style")
  {
    value = (m_boldQuote.GetIsChecked() ? 2 : 0) | (m_italicQuote.GetIsChecked() ? 1 : 0);
    return true;
  }
  else if (n == "Substitutions Style")
  {
    value = (m_boldSubst.GetIsChecked() ? 2 : 0) | (m_italicSubst.GetIsChecked() ? 1 : 0);
    return true;
  }
  else if (n == "Headings Underline")
  {
    value = m_underHead.GetIsChecked() ? 1 : 0;
    return true;
  }
  else if (n == "Main Text Underline")
  {
    value = m_underMain.GetIsChecked() ? 1 : 0;
    return true;
  }
  else if (n == "Comments Underline")
  {
    value = m_underComment.GetIsChecked() ? 1 : 0;
    return true;
  }
  else if (n == "Quoted Text Underline")
  {
    value = m_underQuote.GetIsChecked() ? 1 : 0;
    return true;
  }
  else if (n == "Substitutions Underline")
  {
    value = m_underSubst.GetIsChecked() ? 1 : 0;
    return true;
  }
  else if (n == "Headings Size")
  {
    value = m_sizeHead;
    return true;
  }
  else if (n == "Main Text Size")
  {
    value = m_sizeMain;
    return true;
  }
  else if (n == "Comments Size")
  {
    value = m_sizeComment;
    return true;
  }
  else if (n == "Quoted Text Size")
  {
    value = m_sizeQuote;
    return true;
  }
  else if (n == "Substitutions Size")
  {
    value = m_sizeSubst;
    return true;
  }
  else if (n == "Source Tab Size Chars")
  {
    value = m_tabSize;
    return true;
  }
  else if (n == "Auto Indent")
  {
    value = m_autoIndent;
    return true;
  }
  else if (n == "Auto Space Tables")
  {
    value = m_autoSpaceTables;
    return true;
  }
  else if (n == "Auto Number Sections")
  {
    value = m_autoNumber;
    return true;
  }
  return false;
}

bool PrefsEditPage::GetString(const char* name, char* value, ULONG len)
{
  CString n(name);
  if (n == "Font Name")
  {
    strncpy(value,m_fontName,len-1);
    value[len-1] = 0;
    return true;
  }
  return false;
}

// Called when enumerating fonts, populates the font drop down list in the dialog
int CALLBACK PrefsEditPage::ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param)
{
  PrefsEditPage* page = (PrefsEditPage*)param;
  if (font->elfLogFont.lfFaceName[0] != '@')
    page->m_font.AddString(font->elfLogFont.lfFaceName);
  return 1;
}

IMPLEMENT_DYNAMIC(PrefsColourPage, CPropertyPage)

BEGIN_MESSAGE_MAP(PrefsColourPage, CPropertyPage)
  ON_BN_CLICKED(IDC_NEW_SCHEME, OnClickedNewScheme)
  ON_BN_CLICKED(IDC_DELETE_SCHEME, OnClickedDeleteScheme)
  ON_BN_CLICKED(IDC_RESTORE, OnClickedRestore)
  ON_BN_CLICKED(IDC_ENABLE_COLOURS, OnClickedEnableColours)
  ON_CBN_SELCHANGE(IDC_COLOUR_SCHEME, OnChangeColourScheme)
  ON_WM_HSCROLL()
  ON_MESSAGE(WM_UPDATEPREVIEW, OnUpdatePreview)
  ON_MESSAGE(WM_COLOURCHANGED, OnColourChanged)
END_MESSAGE_MAP()

PrefsColourPage::PrefsColourPage(PrefsDialog* dlg)
  : CPropertyPage(PrefsColourPage::IDD), m_dialog(dlg)
{
  SetDefaults();
}

void PrefsColourPage::ReadSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
  {
    DWORD value = 0;
    if (registryKey.QueryDWORDValue("Syntax Colouring",value) == ERROR_SUCCESS)
      m_colours = (value != 0);

    char schemeName[MAX_PATH];
    value = sizeof schemeName;
    if (registryKey.QueryStringValue("Colour Scheme",schemeName,&value) == ERROR_SUCCESS)
      m_colourScheme = schemeName;
  }
  registryKey.Close();

  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM "\\Colour Schemes",KEY_READ) == ERROR_SUCCESS)
  {
    int i = 0;
    char schemeName[MAX_PATH];
    DWORD schemeNameLen = sizeof schemeName;
    while (registryKey.EnumKey(i,schemeName,&schemeNameLen) == ERROR_SUCCESS)
    {
      i++;
      schemeNameLen = sizeof schemeName;

      CRegKey schemeKey;
      if (schemeKey.Open(registryKey,schemeName) == ERROR_SUCCESS)
      {
        DWORD value = 0;
        ColourScheme& scheme = m_schemes[schemeName];
        if (schemeKey.QueryDWORDValue("Source Paper Colour",value) == ERROR_SUCCESS)
          scheme.source = (COLORREF)value;
        if (schemeKey.QueryDWORDValue("Ext Paper Colour",value) == ERROR_SUCCESS)
          scheme.ext = (COLORREF)value;
        if (schemeKey.QueryDWORDValue("Headings Colour",value) == ERROR_SUCCESS)
          scheme.head = (COLORREF)value;
        if (schemeKey.QueryDWORDValue("Main Text Colour",value) == ERROR_SUCCESS)
          scheme.main = (COLORREF)value;
        if (schemeKey.QueryDWORDValue("Comments Colour",value) == ERROR_SUCCESS)
          scheme.comment = (COLORREF)value;
        if (schemeKey.QueryDWORDValue("Quoted Text Colour",value) == ERROR_SUCCESS)
          scheme.quote = (COLORREF)value;
        if (schemeKey.QueryDWORDValue("Substitutions Colour",value) == ERROR_SUCCESS)
          scheme.subst = (COLORREF)value;
      }
    }
  }
 }

void PrefsColourPage::WriteSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
  {
    registryKey.SetDWORDValue("Syntax Colouring",m_colours);
    registryKey.SetStringValue("Colour Scheme",m_colourScheme);
  }
  registryKey.Close();

  if (registryKey.Create(HKEY_CURRENT_USER,REGISTRY_INFORM "\\Colour Schemes") == ERROR_SUCCESS)
  {
    for (auto schemeIt = m_schemes.begin(); schemeIt != m_schemes.end(); ++schemeIt)
    {
      CRegKey schemeKey;
      if (schemeKey.Create(registryKey,schemeIt->first.c_str()) == ERROR_SUCCESS)
      {
        schemeKey.SetDWORDValue("Source Paper Colour",schemeIt->second.source);
        schemeKey.SetDWORDValue("Ext Paper Colour",schemeIt->second.ext);
        schemeKey.SetDWORDValue("Headings Colour",schemeIt->second.head);
        schemeKey.SetDWORDValue("Main Text Colour",schemeIt->second.main);
        schemeKey.SetDWORDValue("Comments Colour",schemeIt->second.comment);
        schemeKey.SetDWORDValue("Quoted Text Colour",schemeIt->second.quote);
        schemeKey.SetDWORDValue("Substitutions Colour",schemeIt->second.subst);
      }
    }

    // Remove any deleted colour schemes
    std::set<std::string> schemesToRemove;
    int i = 0;
    char schemeName[MAX_PATH];
    DWORD schemeNameLen = sizeof schemeName;
    while (registryKey.EnumKey(i,schemeName,&schemeNameLen) == ERROR_SUCCESS)
    {
      i++;
      schemeNameLen = sizeof schemeName;

      if (m_schemes.count(schemeName) == 0)
        schemesToRemove.insert(schemeName);
    }
    for (auto schemeToRemove : schemesToRemove)
      registryKey.DeleteSubKey(schemeToRemove.c_str());
  }
}

void PrefsColourPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_ENABLE_COLOURS, m_colours);
  DDX_Control(pDX,IDC_ENABLE_COLOURS, m_coloursCheck);
  DDX_CBString(pDX, IDC_COLOUR_SCHEME, m_colourScheme);
  DDX_Control(pDX, IDC_COLOUR_SCHEME, m_colourSchemeCombo);
}

BOOL PrefsColourPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // Subclass dialog controls
  m_colourSource.SubclassDlgItem(IDC_COLOUR_SOURCE,this,WM_COLOURCHANGED);
  m_colourExt.SubclassDlgItem(IDC_COLOUR_EXT,this,WM_COLOURCHANGED);
  m_colourHead.SubclassDlgItem(IDC_HEAD_COLOUR,this,WM_COLOURCHANGED);
  m_colourMain.SubclassDlgItem(IDC_MAIN_COLOUR,this,WM_COLOURCHANGED);
  m_colourComment.SubclassDlgItem(IDC_COMMENT_COLOUR,this,WM_COLOURCHANGED);
  m_colourQuote.SubclassDlgItem(IDC_QUOTE_COLOUR,this,WM_COLOURCHANGED);
  m_colourSubst.SubclassDlgItem(IDC_SUBST_COLOUR,this,WM_COLOURCHANGED);

  m_colourSource.SetShowDisabled(false);
  m_colourExt.SetShowDisabled(false);
  m_colourHead.SetShowDisabled(false);
  m_colourMain.SetShowDisabled(false);
  m_colourComment.SetShowDisabled(false);
  m_colourQuote.SetShowDisabled(false);
  m_colourSubst.SetShowDisabled(false);

  // Create the preview window
  m_preview.Create(this,Project_I7,SourceWindow::Border);
  CRect previewRect;
  GetDlgItem(IDC_PREVIEW)->GetWindowRect(previewRect);
  ScreenToClient(previewRect);
  m_preview.MoveWindow(previewRect);
  SourceEdit& previewEdit = m_preview.GetEdit();
  m_preview.LoadSettings(*m_dialog);
  previewEdit.ReplaceSelect(previewText);
  previewEdit.SetReadOnly(true);
  previewEdit.HideCaret();
  CHARRANGE startRange = { 0,0 };
  previewEdit.SetSelect(startRange);
  m_preview.ShowWindow(SW_SHOW);

  // Set the list of colour scheme names
  UpdateSchemeChoices();

  // Update controls after adding the colour schemes
  UpdateData(FALSE);
  UpdateColourButtons();
  UpdateControlStates();
  UpdatePreview();
  return TRUE;
}

void PrefsColourPage::OnClickedNewScheme()
{
  NewColourSchemeDialog dialog;
  if (dialog.DoModal() != IDOK)
    return;

  CString name = dialog.GetName();
  if (name.IsEmpty())
  {
    ASSERT(FALSE);
    return;
  }

  // Is the name already used?
  bool used = false;
  for (auto schemeIt = m_schemes.begin(); schemeIt != m_schemes.end(); ++schemeIt)
  {
    if (name.CompareNoCase(schemeIt->first.c_str()) == 0)
      used = true;
  }
  if (used)
  {
    LPCWSTR head = L"Name already used";
    LPCWSTR msg = L"The name you have chosen is already used. Try another name.";
    int btn = 0;
    ::TaskDialog(GetSafeHwnd(),0,L_INFORM_TITLE,head,msg,TDCBF_OK_BUTTON,TD_ERROR_ICON,NULL);
    return;
  }

  // Add a new scheme, based on the current scheme
  UpdateData(TRUE);
  auto schemeIt = m_schemes.find((LPCSTR)m_colourScheme);
  ASSERT(schemeIt != m_schemes.end());
  if (schemeIt != m_schemes.end())
  {
    ColourScheme newScheme(schemeIt->second);
    newScheme.sortIndex = CUSTOM_SORT_INDEX;
    m_schemes[(LPCSTR)name] = newScheme;
    m_colourScheme = name;
  }
  UpdateSchemeChoices();
  UpdateData(FALSE);
  UpdateColourButtons();
  UpdateControlStates();
  UpdatePreview();
}

void PrefsColourPage::OnClickedDeleteScheme()
{
  UpdateData(TRUE);
  auto schemeIt = m_schemes.find((LPCSTR)m_colourScheme);
  ASSERT(schemeIt != m_schemes.end());
  if (schemeIt != m_schemes.end())
  {
    if (schemeIt->second.sortIndex >= CUSTOM_SORT_INDEX)
    {
      m_schemes.erase((LPCSTR)m_colourScheme);
      m_colourScheme = ColourScheme::GetDefaultSchemeName(this);
    }
  }

  UpdateSchemeChoices();
  UpdateData(FALSE);
  UpdateColourButtons();
  UpdateControlStates();
  UpdatePreview();
}

void PrefsColourPage::OnClickedRestore()
{
  LPCWSTR head = L"Reset the current colours back to their original values?";
  LPCWSTR msg = L"This action cannot be undone.";
  int btn = 0;
  if (SUCCEEDED(::TaskDialog(GetSafeHwnd(),0,L_INFORM_TITLE,head,msg,
    TDCBF_YES_BUTTON|TDCBF_NO_BUTTON,TD_WARNING_ICON,&btn)))
  {
    if (btn == IDYES)
    {
      SetDefaults();

      // Update the controls to match the defaults
      UpdateData(FALSE);
      UpdateColourButtons();
      UpdateControlStates();
      m_dialog->UpdatePreviews();
    }
  }
}

void PrefsColourPage::OnClickedEnableColours()
{
  UpdateControlStates();
  UpdatePreview();
}

void PrefsColourPage::OnChangeColourScheme()
{
  UpdateData(TRUE);
  UpdateColourButtons();
  UpdateControlStates();
  UpdatePreview();
}

void PrefsColourPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  PostMessage(WM_UPDATEPREVIEW);
}

LRESULT PrefsColourPage::OnUpdatePreview(WPARAM, LPARAM)
{
  UpdatePreview();
  return 0;
}

LRESULT PrefsColourPage::OnColourChanged(WPARAM, LPARAM)
{
  UpdateData(TRUE);
  auto schemeIt = m_schemes.find((LPCSTR)m_colourScheme);
  ASSERT(schemeIt != m_schemes.end());
  if (schemeIt != m_schemes.end())
  {
    schemeIt->second.source = m_colourSource.GetCurrentColour();
    schemeIt->second.ext = m_colourExt.GetCurrentColour();
    schemeIt->second.head = m_colourHead.GetCurrentColour();
    schemeIt->second.main = m_colourMain.GetCurrentColour();
    schemeIt->second.comment = m_colourComment.GetCurrentColour();
    schemeIt->second.quote = m_colourQuote.GetCurrentColour();
    schemeIt->second.subst = m_colourSubst.GetCurrentColour();
  }

  UpdatePreview();
  return 0;
}

void PrefsColourPage::UpdateSchemeChoices(void)
{
  m_colourSchemeCombo.ResetContent();

  std::vector<std::string> schemeNames;
  for (auto schemeIt = m_schemes.begin(); schemeIt != m_schemes.end(); ++schemeIt)
    schemeNames.push_back(schemeIt->first);
  std::sort(schemeNames.begin(),schemeNames.end(),[&](auto& name1, auto& name2)
  {
    ColourScheme& scheme1 = m_schemes[name1];
    ColourScheme& scheme2 = m_schemes[name2];

    if (scheme1.sortIndex != scheme2.sortIndex)
      return scheme1.sortIndex < scheme2.sortIndex;
    return name1 < name2;
  });
  for (auto nameIt = schemeNames.begin(); nameIt != schemeNames.end(); ++nameIt)
    m_colourSchemeCombo.AddString(nameIt->c_str());
}

void PrefsColourPage::UpdateColourButtons(void)
{
  auto schemeIt = m_schemes.find((LPCSTR)m_colourScheme);
  ASSERT(schemeIt != m_schemes.end());
  if (schemeIt != m_schemes.end())
  {
    m_colourSource.SetCurrentColour(schemeIt->second.source);
    m_colourExt.SetCurrentColour(schemeIt->second.ext);
    m_colourHead.SetCurrentColour(schemeIt->second.head);
    m_colourMain.SetCurrentColour(schemeIt->second.main);
    m_colourComment.SetCurrentColour(schemeIt->second.comment);
    m_colourQuote.SetCurrentColour(schemeIt->second.quote);
    m_colourSubst.SetCurrentColour(schemeIt->second.subst);

    if (GetSafeHwnd() != 0)
    {
      m_colourSource.Invalidate();
      m_colourExt.Invalidate();
      m_colourHead.Invalidate();
      m_colourMain.Invalidate();
      m_colourComment.Invalidate();
      m_colourQuote.Invalidate();
      m_colourSubst.Invalidate();
    }
  }
}

void PrefsColourPage::UpdateControlStates(void)
{
  bool coloursEnabled = (m_coloursCheck.GetCheck() == BST_CHECKED);
  const int ids[] =
  {
    IDC_SCHEME_LABEL,
    IDC_COLOUR_SCHEME,
    IDC_NEW_SCHEME,
    IDC_DELETE_SCHEME,
    IDC_HEAD_LABEL,
    IDC_MAIN_LABEL,
    IDC_COMMENT_LABEL,
    IDC_QUOTE_LABEL,
    IDC_SUBST_LABEL,
    IDC_HEAD_COLOUR,
    IDC_MAIN_COLOUR,
    IDC_COMMENT_COLOUR,
    IDC_QUOTE_COLOUR,
    IDC_SUBST_COLOUR,
    IDC_COLOUR_SOURCE_LABEL,
    IDC_COLOUR_EXT_LABEL,
    IDC_COLOUR_SOURCE,
    IDC_COLOUR_EXT
  };
  for (int i = 0; i < sizeof ids / sizeof ids[0]; i++)
    GetDlgItem(ids[i])->EnableWindow(coloursEnabled);

  auto schemeIt = m_schemes.find((LPCSTR)m_colourScheme);
  ASSERT(schemeIt != m_schemes.end());
  if (schemeIt != m_schemes.end())
  {
    if (schemeIt->second.sortIndex < CUSTOM_SORT_INDEX)
      GetDlgItem(IDC_DELETE_SCHEME)->EnableWindow(FALSE);
  }
}

void PrefsColourPage::UpdatePreview(void)
{
  UpdateData(TRUE);
  m_dialog->UpdatePreviews();
}

void PrefsColourPage::PreviewChanged(void)
{
  m_preview.LoadSettings(*m_dialog);
  m_preview.PrefsChanged();
}

// Set the default preferences values
void PrefsColourPage::SetDefaults(void)
{
  m_colours = TRUE;
  m_colourScheme = ColourScheme::GetDefaultSchemeName(
    GetSafeHwnd() != 0 ? this : AfxGetMainWnd());
  m_schemes = ColourScheme::DefaultSchemes;
  UpdateColourButtons();
}

bool PrefsColourPage::GetDWord(const char* name, DWORD& value)
{
  CString n(name);
  if (n == "Syntax Colouring")
  {
    value = m_colours;
    return true;
  }
  else if (n == "Source Paper Colour")
  {
    value = m_colourSource.GetCurrentColour();
    return true;
  }
  else if (n == "Ext Paper Colour")
  {
    value = m_colourExt.GetCurrentColour();
    return true;
  }
  else if (n == "Headings Colour")
  {
    value = m_colourHead.GetCurrentColour();
    return true;
  }
  else if (n == "Main Text Colour")
  {
    value = m_colourMain.GetCurrentColour();
    return true;
  }
  else if (n == "Comments Colour")
  {
    value = m_colourComment.GetCurrentColour();
    return true;
  }
  else if (n == "Quoted Text Colour")
  {
    value = m_colourQuote.GetCurrentColour();
    return true;
  }
  else if (n == "Substitutions Colour")
  {
    value = m_colourSubst.GetCurrentColour();
    return true;
  }
  return false;
}

bool PrefsColourPage::GetString(const char* name, char* value, ULONG len)
{
  return false;
}

BEGIN_MESSAGE_MAP(PrefsAdvancedPage, CPropertyPage)
  ON_BN_CLICKED(IDC_CLEANFILES, OnClickedCleanFiles)
  ON_MESSAGE(WM_AFTERFONTSET, OnAfterFontSet)
END_MESSAGE_MAP()

PrefsAdvancedPage::PrefsAdvancedPage() : CPropertyPage(PrefsAdvancedPage::IDD)
{
  // Set the default preferences values
  m_cleanFiles = TRUE;
  m_cleanIndexes = TRUE;
  m_glulxTerp = "Glulxe";
  m_tabsHorizontal = FALSE;
  m_fixedFontName = theApp.GetFontName(InformApp::FontFixedWidth);
}

void PrefsAdvancedPage::ReadSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
  {
    DWORD value = 0;

    if (registryKey.QueryDWORDValue("Clean Up Files",value) == ERROR_SUCCESS)
      m_cleanFiles = (value != 0);
    if (registryKey.QueryDWORDValue("Clean Up Indexes",value) == ERROR_SUCCESS)
      m_cleanIndexes = (value != 0);
    if (registryKey.QueryDWORDValue("Tabs Horizontal",value) == ERROR_SUCCESS)
      m_tabsHorizontal = (value != 0);

    char fontName[MAX_PATH];
    value = sizeof fontName;
    if (registryKey.QueryStringValue("Fixed Font Name",fontName,&value) == ERROR_SUCCESS)
      m_fixedFontName = fontName;
  }

  m_glulxTerp = theApp.CWinApp::GetProfileString("Game","Glulx Interpreter",m_glulxTerp);
}

void PrefsAdvancedPage::WriteSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
  {
    registryKey.SetDWORDValue("Clean Up Files",m_cleanFiles);
    registryKey.SetDWORDValue("Clean Up Indexes",m_cleanIndexes);
    registryKey.SetDWORDValue("Tabs Horizontal",m_tabsHorizontal);
    registryKey.SetStringValue("Fixed Font Name",m_fixedFontName);
  }
  theApp.CWinApp::WriteProfileString("Game","Glulx Interpreter",m_glulxTerp);
}

void PrefsAdvancedPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CLEANFILES, m_cleanFiles);
  DDX_Check(pDX, IDC_CLEANINDEX, m_cleanIndexes);
  DDX_Control(pDX, IDC_CLEANFILES, m_cleanFilesCheck);
  DDX_Control(pDX, IDC_CLEANINDEX, m_cleanIndexCheck);
  DDX_CBString(pDX, IDC_GLULX, m_glulxTerp);
  DDX_Check(pDX, IDC_TABS_HORIZONTAL, m_tabsHorizontal);
  DDX_Control(pDX, IDC_FIXEDFONT, m_fixedFont);
}

BOOL PrefsAdvancedPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // Get all the possible fonts
  CDC* dc = GetDC();
  LOGFONT font;
  ::ZeroMemory(&font,sizeof font);
  font.lfCharSet = ANSI_CHARSET;
  ::EnumFontFamiliesEx(dc->GetSafeHdc(),&font,(FONTENUMPROC)ListFonts,(LPARAM)this,0);
  ReleaseDC(dc);

  // Initialize the font control
  if (m_fixedFont.SelectString(-1,m_fixedFontName) == CB_ERR)
    m_fixedFont.SetCurSel(0);

  UpdateControlStates();
  return TRUE;
}

void PrefsAdvancedPage::OnOK()
{
  m_fixedFont.GetWindowText(m_fixedFontName);
  CPropertyPage::OnOK();
}

void PrefsAdvancedPage::OnClickedCleanFiles()
{
  UpdateControlStates();
}

LRESULT PrefsAdvancedPage::OnAfterFontSet(WPARAM, LPARAM)
{
  // Set a smaller font for the "Also clean out ..." checkbox
  LOGFONT smallFont;
  m_cleanIndexCheck.GetFont()->GetLogFont(&smallFont);
  smallFont.lfHeight = (LONG)(smallFont.lfHeight*0.9);
  m_smallFont.DeleteObject();
  m_smallFont.CreateFontIndirect(&smallFont);
  m_cleanIndexCheck.SetFont(&m_smallFont);
  return 0;
}

void PrefsAdvancedPage::UpdateControlStates(void)
{
  bool cleanFiles = (m_cleanFilesCheck.GetCheck() == BST_CHECKED);
  m_cleanIndexCheck.EnableWindow(cleanFiles);
}

// Called when enumerating fonts, populates the font drop down list in the dialog
int CALLBACK PrefsAdvancedPage::ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param)
{
  PrefsAdvancedPage* page = (PrefsAdvancedPage*)param;
  if (font->elfLogFont.lfFaceName[0] != '@')
  {
    if (font->elfLogFont.lfPitchAndFamily & FIXED_PITCH)
      page->m_fixedFont.AddString(font->elfLogFont.lfFaceName);
  }
  return 1;
}

PrefsDialog::PrefsDialog(void)
  : CPropertySheet("Preferences"), m_editPage(this), m_colourPage(this)
{
  m_dpi = 96;
  m_fontHeightPerDpi = 1.0;
  m_psh.dwFlags |= PSH_NOAPPLYNOW;
}

BEGIN_MESSAGE_MAP(PrefsDialog, CPropertySheet)
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_MESSAGE(WM_RESIZEPAGE, OnResizePage)  
END_MESSAGE_MAP()

BOOL PrefsDialog::OnInitDialog() 
{
  CPropertySheet::OnInitDialog();
  m_dpi = DPI::getWindowDPI(this);

  // Get the default font for the dialog
  CFont* sysFont = theApp.GetFont(this,InformApp::FontSystem);
  sysFont->GetLogFont(&m_logFont);
  CSize fontSize = theApp.MeasureFont(this,sysFont);

  // Is a smaller font needed?
  int monHeight = DPI::getMonitorWorkRect(this).Height();
  if (monHeight < 45*fontSize.cy)
    m_logFont.lfHeight = monHeight/45;
  m_font.CreateFontIndirect(&m_logFont);
  m_fontHeightPerDpi = (double)m_logFont.lfHeight / (double)m_dpi;
  double scaleX = (double)fontSize.cx / (double)theApp.MeasureFont(this,&m_font).cx;

  // Change the font of the property sheet and its pages
  ChangeDialogFont(this,&m_font,0.0,scaleX);
  CPropertyPage* page = GetActivePage();
  for (int i = 0; i < GetPageCount(); i++)
  {
    SetActivePage(i);
    CPropertyPage* page = GetActivePage();
    DPI::disableDialogDPI(page);
    ChangeDialogFont(page,&m_font,0.0,scaleX);
    page->SendMessage(WM_AFTERFONTSET);
  }
  SetActivePage(page);

  CTabCtrl* tab = GetTabControl();
  tab->SetMinTabWidth(8);
  tab->GetWindowRect(&m_page);
  ScreenToClient(&m_page);
  tab->AdjustRect(FALSE,&m_page);
  page->MoveWindow(&m_page);
  return TRUE;
}

BOOL PrefsDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
  NMHDR* pnmh = (LPNMHDR) lParam;
  if (pnmh->code == TCN_SELCHANGE)
    PostMessage(WM_RESIZEPAGE);
  return CPropertySheet::OnNotify(wParam, lParam, pResult);
}

LRESULT PrefsDialog::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  int newDpi = (int)HIWORD(wparam);
  if (m_dpi != newDpi)
  {
    if (GetTabControl() != NULL)
    {
      // Use the top-left corner of the suggested window rectangle
      CRect windowRect;
      GetWindowRect(windowRect);
      windowRect.left = ((LPRECT)lparam)->left;
      windowRect.top = ((LPRECT)lparam)->top;
      MoveWindow(windowRect,TRUE);

      // Update the font
      m_logFont.lfHeight = (long)(m_fontHeightPerDpi * newDpi);
      CFont oldFont;
      oldFont.Attach(m_font.Detach());
      m_font.CreateFontIndirect(&m_logFont);

      // Update the dialog to use the new font
      double scaleDpi = (double)newDpi / (double)m_dpi;
      ChangeDialogFont(this,&m_font,scaleDpi,1.0);
      CPropertyPage* page = GetActivePage();
      for (int i = 0; i < GetPageCount(); i++)
      {
        SetActivePage(i);
        CPropertyPage* page = GetActivePage();
        ChangeDialogFont(page,&m_font,scaleDpi,1.0);
        page->SendMessage(WM_AFTERFONTSET);
      }
      SetActivePage(page);

      // Resize the property page
      CTabCtrl* tab = GetTabControl();
      tab->GetWindowRect(&m_page);
      ScreenToClient(&m_page);
      tab->AdjustRect(FALSE,&m_page);
      page->MoveWindow(&m_page);
    }

    m_dpi = newDpi;
  }
  return 0;
}

LRESULT PrefsDialog::OnResizePage(WPARAM, LPARAM)
{
  CPropertyPage* page = GetActivePage();
  page->MoveWindow(&m_page);
  return 0;
}

bool PrefsDialog::GetDWord(const char* name, DWORD& value)
{
  if (m_editPage.GetDWord(name,value))
    return true;
  else if (m_colourPage.GetDWord(name,value))
    return true;
  return false;
}

bool PrefsDialog::GetString(const char* name, char* value, ULONG len)
{
  if (m_editPage.GetString(name,value,len))
    return true;
  else if (m_colourPage.GetString(name,value,len))
    return true;
  return false;
}

void PrefsDialog::ChangeDialogFont(CWnd* wnd, CFont* font, double scale, double extScaleX)
{
  CRect windowRect;

  double scaleW = 1.0, scaleH = 1.0;
  if (scale > 0.0)
  {
    scaleW = scale;
    scaleH = scale;
  }
  else
  {
    TEXTMETRIC tmOld, tmNew;
    CDC* dc = wnd->GetDC();
    CFont* oldFont = dc->SelectObject(wnd->GetFont());
    dc->GetTextMetrics(&tmOld);
    dc->SelectObject(font);
    dc->GetTextMetrics(&tmNew);
    dc->SelectObject(oldFont);
    wnd->ReleaseDC(dc);

    scaleW = (double)tmNew.tmAveCharWidth / (double)tmOld.tmAveCharWidth;
    scaleH = (double)(tmNew.tmHeight+tmNew.tmExternalLeading) /
      (double)(tmOld.tmHeight+tmOld.tmExternalLeading);
  }
  scaleW *= extScaleX;

  // Calculate new dialog window rectangle
  CRect clientRect, newClientRect, newWindowRect;

  wnd->GetWindowRect(windowRect);
  wnd->GetClientRect(clientRect);
  long xDiff = windowRect.Width() - clientRect.Width();
  long yDiff = windowRect.Height() - clientRect.Height();

  newClientRect.left = newClientRect.top = 0;
  newClientRect.right = (long)(clientRect.right * scaleW);
  newClientRect.bottom = (long)(clientRect.bottom * scaleH);

  newWindowRect.left = windowRect.left - (newClientRect.right - clientRect.right)/2;
  newWindowRect.top = windowRect.top - (newClientRect.bottom - clientRect.bottom)/2;
  newWindowRect.right = newWindowRect.left + newClientRect.right + xDiff;
  newWindowRect.bottom = newWindowRect.top + newClientRect.bottom + yDiff;

  wnd->MoveWindow(newWindowRect);
  wnd->SetFont(font);

  CWnd* childWnd = wnd->GetWindow(GW_CHILD);
  while (childWnd)
  {
    childWnd->SetFont(font);
    childWnd->GetWindowRect(windowRect);

    CString strClass;
    ::GetClassName(childWnd->GetSafeHwnd(),strClass.GetBufferSetLength(32),31);
    strClass.MakeUpper();
    if (strClass == "COMBOBOX")
    {
      CRect rect;
      childWnd->SendMessage(CB_GETDROPPEDCONTROLRECT,0,(LPARAM)&rect);
      windowRect.right = rect.right;
      windowRect.bottom = rect.bottom;
    }

    wnd->ScreenToClient(windowRect);
    windowRect.left = (long)(windowRect.left * scaleW);
    windowRect.right = (long)(windowRect.right * scaleW);
    windowRect.top = (long)(windowRect.top * scaleH);
    windowRect.bottom = (long)(windowRect.bottom * scaleH);
    childWnd->MoveWindow(windowRect);

    childWnd = childWnd->GetWindow(GW_HWNDNEXT);
  }
}

void PrefsDialog::ShowDialog(void)
{
  m_editPage.ReadSettings();
  AddPage(&m_editPage);
  m_colourPage.ReadSettings();
  AddPage(&m_colourPage);
  m_advPage.ReadSettings();
  AddPage(&m_advPage);

  // Show the dialog
  if (DoModal() == IDOK)
  {
    // Store the new settings
    m_editPage.WriteSettings();
    m_colourPage.WriteSettings();
    m_advPage.WriteSettings();

    // Notify all project windows
    theApp.SendAllFrames(InformApp::Preferences,0);
  }
}

void PrefsDialog::UpdatePreviews(void)
{
  if (m_editPage.GetSafeHwnd() != 0)
    m_editPage.PreviewChanged();
  if (m_colourPage.GetSafeHwnd() != 0)
    m_colourPage.PreviewChanged();
}

BEGIN_MESSAGE_MAP(NewColourSchemeDialog, I7BaseDialog)
  ON_EN_CHANGE(IDC_NAME, OnChangedEdit)
END_MESSAGE_MAP()

NewColourSchemeDialog::NewColourSchemeDialog() : I7BaseDialog(NewColourSchemeDialog::IDD)
{
}

CString NewColourSchemeDialog::GetName(void)
{
  return m_name;
}

BOOL NewColourSchemeDialog::OnInitDialog()
{
  if (!I7BaseDialog::OnInitDialog())
    return FALSE;

  GetDlgItem(IDOK)->EnableWindow(FALSE);
  return TRUE;
}

void NewColourSchemeDialog::OnOK()
{
  GetDlgItem(IDC_NAME)->GetWindowText(m_name);
  I7BaseDialog::OnOK();
}

void NewColourSchemeDialog::OnChangedEdit()
{
  GetDlgItem(IDC_NAME)->GetWindowText(m_name);
  GetDlgItem(IDOK)->EnableWindow(!m_name.IsEmpty());
}
