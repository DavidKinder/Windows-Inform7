#include "stdafx.h"
#include "PrefsDialog.h"
#include "Inform.h"
#include "Messages.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NUM_PREVIEW_TABS 60

PrefsEditPage::PrefsEditPage() : CPropertyPage(PrefsEditPage::IDD)
{
  SetDefaults();
}

BEGIN_MESSAGE_MAP(PrefsEditPage, CPropertyPage)
  ON_BN_CLICKED(IDC_RESTORE, OnClickedRestore)
  ON_CBN_SELCHANGE(IDC_FONT, OnChangeFont)
  ON_CBN_SELCHANGE(IDC_FONTSIZE, OnChangeFont)
  ON_CBN_EDITCHANGE(IDC_FONTSIZE, OnChangeFont)
  ON_BN_CLICKED(IDC_ENABLE_HIGHLIGHT, OnClickedEnableHighlight)
  ON_CBN_SELCHANGE(IDC_HEAD_STYLE, OnChangeStyle)
  ON_BN_CLICKED(IDC_HEAD_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_HEAD_SIZE, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_MAIN_STYLE, OnChangeStyle)
  ON_BN_CLICKED(IDC_MAIN_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_MAIN_SIZE, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_COMMENT_STYLE, OnChangeStyle)
  ON_BN_CLICKED(IDC_COMMENT_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_COMMENT_SIZE, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_QUOTE_STYLE, OnChangeStyle)
  ON_BN_CLICKED(IDC_QUOTE_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_QUOTE_SIZE, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_SUBST_STYLE, OnChangeStyle)
  ON_BN_CLICKED(IDC_SUBST_UNDER, OnChangeStyle)
  ON_CBN_SELCHANGE(IDC_SUBST_SIZE, OnChangeStyle)
  ON_BN_CLICKED(IDC_INDENT, OnChangeStyle)
  ON_BN_CLICKED(IDC_ELASTIC_TABS, OnChangeStyle)
  ON_WM_HSCROLL()
  ON_MESSAGE(WM_AFTERFONTSET, OnAfterFontSet)
  ON_MESSAGE(WM_UPDATEPREVIEW, OnUpdatePreview)
END_MESSAGE_MAP()

void PrefsEditPage::ReadSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
  {
    char fontName[MAX_PATH];
    DWORD value = sizeof fontName;
    if (registryKey.QueryStringValue("Source Font Name",fontName,&value) == ERROR_SUCCESS)
      m_fontName = fontName;
    else
    {
      value = sizeof fontName;
      if (registryKey.QueryStringValue("Font Name",fontName,&value) == ERROR_SUCCESS)
        m_fontName = fontName;
    }
    if (registryKey.QueryDWORDValue("Source Font Size",value) == ERROR_SUCCESS)
      m_fontSize.Format("%d",value);
    else if (registryKey.QueryDWORDValue("Font Size",value) == ERROR_SUCCESS)
      m_fontSize.Format("%d",value);
    if (registryKey.QueryDWORDValue("Source Paper Colour",value) == ERROR_SUCCESS)
      m_colourSource.SetCurrentColour((COLORREF)value);
    if (registryKey.QueryDWORDValue("Ext Paper Colour",value) == ERROR_SUCCESS)
      m_colourExt.SetCurrentColour((COLORREF)value);

    if (registryKey.QueryDWORDValue("Syntax Highlighting",value) == ERROR_SUCCESS)
      m_highlight = (value != 0);
    if (registryKey.QueryDWORDValue("Headings Colour",value) == ERROR_SUCCESS)
      m_colourHead.SetCurrentColour((COLORREF)value);
    if (registryKey.QueryDWORDValue("Main Text Colour",value) == ERROR_SUCCESS)
      m_colourMain.SetCurrentColour((COLORREF)value);
    if (registryKey.QueryDWORDValue("Comments Colour",value) == ERROR_SUCCESS)
      m_colourComment.SetCurrentColour((COLORREF)value);
    if (registryKey.QueryDWORDValue("Quoted Text Colour",value) == ERROR_SUCCESS)
      m_colourQuote.SetCurrentColour((COLORREF)value);
    if (registryKey.QueryDWORDValue("Substitutions Colour",value) == ERROR_SUCCESS)
      m_colourSubst.SetCurrentColour((COLORREF)value);
    if (registryKey.QueryDWORDValue("Headings Style",value) == ERROR_SUCCESS)
      m_styleHead = (int)value;
    if (registryKey.QueryDWORDValue("Main Text Style",value) == ERROR_SUCCESS)
      m_styleMain = (int)value;
    if (registryKey.QueryDWORDValue("Comments Style",value) == ERROR_SUCCESS)
      m_styleComment = (int)value;
    if (registryKey.QueryDWORDValue("Quoted Text Style",value) == ERROR_SUCCESS)
      m_styleQuote = (int)value;
    if (registryKey.QueryDWORDValue("Substitutions Style",value) == ERROR_SUCCESS)
      m_styleSubst = (int)value;
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
    if (registryKey.QueryDWORDValue("Indent Wrapped Lines",value) == ERROR_SUCCESS)
      m_indentWrapped = value;
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
    registryKey.SetStringValue("Source Font Name",m_fontName);
    int fontSize = 0;
    if (sscanf(m_fontSize,"%d",&fontSize) == 1)
      registryKey.SetDWORDValue("Source Font Size",fontSize);
    registryKey.SetDWORDValue("Source Paper Colour",m_colourSource.GetCurrentColour());
    registryKey.SetDWORDValue("Ext Paper Colour",m_colourExt.GetCurrentColour());

    registryKey.SetDWORDValue("Syntax Highlighting",m_highlight);
    registryKey.SetDWORDValue("Headings Colour",m_colourHead.GetCurrentColour());
    registryKey.SetDWORDValue("Main Text Colour",m_colourMain.GetCurrentColour());
    registryKey.SetDWORDValue("Comments Colour",m_colourComment.GetCurrentColour());
    registryKey.SetDWORDValue("Quoted Text Colour",m_colourQuote.GetCurrentColour());
    registryKey.SetDWORDValue("Substitutions Colour",m_colourSubst.GetCurrentColour());
    registryKey.SetDWORDValue("Headings Style",m_styleHead);
    registryKey.SetDWORDValue("Main Text Style",m_styleMain);
    registryKey.SetDWORDValue("Comments Style",m_styleComment);
    registryKey.SetDWORDValue("Quoted Text Style",m_styleQuote);
    registryKey.SetDWORDValue("Substitutions Style",m_styleSubst);
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
    registryKey.SetDWORDValue("Indent Wrapped Lines",m_indentWrapped);
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
  DDX_Check(pDX, IDC_ENABLE_HIGHLIGHT, m_highlight);
  DDX_Control(pDX, IDC_ENABLE_HIGHLIGHT, m_highlightCheck);
  DDX_CBIndex(pDX, IDC_HEAD_STYLE, m_styleHead);
  DDX_CBIndex(pDX, IDC_MAIN_STYLE, m_styleMain);
  DDX_CBIndex(pDX, IDC_COMMENT_STYLE, m_styleComment);
  DDX_CBIndex(pDX, IDC_QUOTE_STYLE, m_styleQuote);
  DDX_CBIndex(pDX, IDC_SUBST_STYLE, m_styleSubst);
  DDX_CBIndex(pDX, IDC_HEAD_SIZE, m_sizeHead);
  DDX_CBIndex(pDX, IDC_MAIN_SIZE, m_sizeMain);
  DDX_CBIndex(pDX, IDC_COMMENT_SIZE, m_sizeComment);
  DDX_CBIndex(pDX, IDC_QUOTE_SIZE, m_sizeQuote);
  DDX_CBIndex(pDX, IDC_SUBST_SIZE, m_sizeSubst);
  DDX_Control(pDX, IDC_TABSIZE, m_tabSizeCtrl);
  DDX_Slider(pDX, IDC_TABSIZE, m_tabSize);
  DDX_Check(pDX, IDC_INDENT, m_indentWrapped);
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
  m_colourSource.SubclassDlgItem(IDC_COLOUR_SOURCE,this,WM_UPDATEPREVIEW);
  m_colourExt.SubclassDlgItem(IDC_COLOUR_EXT,this,WM_UPDATEPREVIEW);
  m_colourHead.SubclassDlgItem(IDC_HEAD_COLOUR,this,WM_UPDATEPREVIEW);
  m_colourMain.SubclassDlgItem(IDC_MAIN_COLOUR,this,WM_UPDATEPREVIEW);
  m_colourComment.SubclassDlgItem(IDC_COMMENT_COLOUR,this,WM_UPDATEPREVIEW);
  m_colourQuote.SubclassDlgItem(IDC_QUOTE_COLOUR,this,WM_UPDATEPREVIEW);
  m_colourSubst.SubclassDlgItem(IDC_SUBST_COLOUR,this,WM_UPDATEPREVIEW);
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
  m_preview.LoadSettings(*this);
  previewEdit.ReplaceSelect(
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
    L"a white jersey\t1975\t\"best cyclist aged 25 or less\"");
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
  SetDefaults();

  // Update the controls to match the defaults
  UpdateData(FALSE);
  m_font.SelectString(-1,m_fontName);
  UpdateControlStates();
  m_colourSource.Invalidate();
  m_colourExt.Invalidate();
  m_colourHead.Invalidate();
  m_colourMain.Invalidate();
  m_colourComment.Invalidate();
  m_colourQuote.Invalidate();
  m_colourSubst.Invalidate();
  m_preview.LoadSettings(*this);
  m_preview.PrefsChanged();
  m_tabPreview.GetEdit().SetCustomTabStops(NUM_PREVIEW_TABS,
    m_preview.GetEdit().GetTabWidthPixels());
}

void PrefsEditPage::OnChangeFont()
{
  PostMessage(WM_UPDATEPREVIEW);
}

void PrefsEditPage::OnChangeStyle()
{
  PostMessage(WM_UPDATEPREVIEW);
}

void PrefsEditPage::OnClickedEnableHighlight()
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
  GetDlgItem(IDC_HEAD_STYLE)->GetWindowRect(topRect);
  ScreenToClient(topRect);
  GetDlgItem(IDC_SUBST_STYLE)->GetWindowRect(bottomRect);
  ScreenToClient(bottomRect);
  int rowh = topRect.Height();
  int space = (bottomRect.bottom - topRect.top - (5*rowh) + 3) / 5;
  AdjustControlRow(IDC_MAIN_STYLE,topRect.top + rowh + space,IDC_MAIN_LABEL,IDC_MAIN_SIZE);
  AdjustControlRow(IDC_COMMENT_STYLE,topRect.top + 2*(rowh + space),IDC_COMMENT_LABEL,IDC_COMMENT_SIZE);
  AdjustControlRow(IDC_QUOTE_STYLE,topRect.top + 3*(rowh + space),IDC_QUOTE_LABEL,IDC_QUOTE_SIZE);
  AdjustControlRow(IDC_SUBST_STYLE,topRect.top + 4*(rowh + space),IDC_SUBST_LABEL,IDC_SUBST_SIZE);

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
  bool highlight = (m_highlightCheck.GetCheck() == BST_CHECKED);
  for (int id = IDC_HEAD_COLOUR; id <= IDC_SUBST_SIZE; id++)
    GetDlgItem(id)->EnableWindow(highlight);
}

void PrefsEditPage::UpdatePreview(void)
{
  UpdateData(TRUE);
  m_font.GetWindowText(m_fontName);

  m_preview.LoadSettings(*this);
  m_preview.PrefsChanged();
  m_tabPreview.GetEdit().SetCustomTabStops(NUM_PREVIEW_TABS,
    m_preview.GetEdit().GetTabWidthPixels());
}

// Set the default preferences values
void PrefsEditPage::SetDefaults(void)
{
  m_fontName = theApp.GetFontName(InformApp::FontDisplay);
  m_fontSize.Format("%d",theApp.GetFontSize(InformApp::FontDisplay));
  m_colourSource.SetCurrentColour(theApp.GetColour(InformApp::ColourBack));
  m_colourExt.SetCurrentColour(theApp.GetColour(InformApp::ColourI7XP));

  m_highlight = TRUE;
  m_colourHead.SetCurrentColour(theApp.GetColour(InformApp::ColourText));
  m_colourMain.SetCurrentColour(theApp.GetColour(InformApp::ColourText));
  m_colourComment.SetCurrentColour(theApp.GetColour(InformApp::ColourComment));
  m_colourQuote.SetCurrentColour(theApp.GetColour(InformApp::ColourQuote));
  m_colourSubst.SetCurrentColour(theApp.GetColour(InformApp::ColourSubstitution));
  m_styleHead = 2; // Bold
  m_styleMain = 0; // Regular
  m_styleComment = 2;
  m_styleQuote = 2;
  m_styleSubst = 0;
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
  m_indentWrapped = TRUE;
  m_autoIndent = TRUE;
  m_autoSpaceTables = TRUE;
  m_autoNumber = FALSE;
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
  if (n == "Source Font Size")
  {
    int fontSize = 9;
    sscanf(m_fontSize,"%d",&fontSize);
    value = fontSize;
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
  else if (n == "Syntax Highlighting")
  {
    value = m_highlight;
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
  else if (n == "Headings Style")
  {
    value = m_styleHead;
    return true;
  }
  else if (n == "Main Text Style")
  {
    value = m_styleMain;
    return true;
  }
  else if (n == "Comments Style")
  {
    value = m_styleComment;
    return true;
  }
  else if (n == "Quoted Text Style")
  {
    value = m_styleQuote;
    return true;
  }
  else if (n == "Substitutions Style")
  {
    value = m_styleSubst;
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
  else if (n == "Indent Wrapped Lines")
  {
    value = m_indentWrapped;
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
  if (n == "Source Font Name")
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

PrefsTextPage::PrefsTextPage() : CPropertyPage(PrefsTextPage::IDD)
{
  // Set the default preferences values
  m_fontName = theApp.GetFontName(InformApp::FontDisplay);
  m_fixedFontName = theApp.GetFontName(InformApp::FontFixedWidth);
  m_fontSize.Format("%d",theApp.GetFontSize(InformApp::FontDisplay));
}

BEGIN_MESSAGE_MAP(PrefsTextPage, CPropertyPage)
END_MESSAGE_MAP()

void PrefsTextPage::ReadSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
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
  }
}

void PrefsTextPage::WriteSettings(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
  {
    registryKey.SetStringValue("Font Name",m_fontName);
    registryKey.SetStringValue("Fixed Font Name",m_fixedFontName);
    int fontSize = 0;
    if (sscanf(m_fontSize,"%d",&fontSize) == 1)
      registryKey.SetDWORDValue("Font Size",fontSize);
  }
}

void PrefsTextPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FONT, m_font);
  DDX_Control(pDX, IDC_FIXEDFONT, m_fixedFont);
  DDX_CBString(pDX, IDC_FONTSIZE, m_fontSize);
}

BOOL PrefsTextPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

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
  return TRUE;
}

void PrefsTextPage::OnOK()
{
  m_font.GetWindowText(m_fontName);
  m_fixedFont.GetWindowText(m_fixedFontName);
  CPropertyPage::OnOK();
}

// Called when enumerating fonts, populates the font drop down lists in the dialog
int CALLBACK PrefsTextPage::ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param)
{
  PrefsTextPage* page = (PrefsTextPage*)param;
  if (font->elfLogFont.lfFaceName[0] != '@')
  {
    page->m_font.AddString(font->elfLogFont.lfFaceName);
    if (font->elfLogFont.lfPitchAndFamily & FIXED_PITCH)
      page->m_fixedFont.AddString(font->elfLogFont.lfFaceName);
  }
  return 1;
}

PrefsAdvancedPage::PrefsAdvancedPage() : CPropertyPage(PrefsAdvancedPage::IDD)
{
  // Set the default preferences values
  m_cleanFiles = TRUE;
  m_cleanIndexes = TRUE;
  m_glulxTerp = "Glulxe";
  m_tabsHorizontal = FALSE;
  m_I6debug = FALSE;
}

BEGIN_MESSAGE_MAP(PrefsAdvancedPage, CPropertyPage)
  ON_BN_CLICKED(IDC_CLEANFILES, OnClickedCleanFiles)
  ON_MESSAGE(WM_AFTERFONTSET, OnAfterFontSet)
END_MESSAGE_MAP()

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
    if (registryKey.QueryDWORDValue("Generate I6 Debug",value) == ERROR_SUCCESS)
      m_I6debug = (value != 0);
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
    registryKey.SetDWORDValue("Generate I6 Debug",m_I6debug);
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
  DDX_Check(pDX, IDC_I6DEBUGGING, m_I6debug);
}

BOOL PrefsAdvancedPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();
  UpdateControlStates();
  return TRUE;
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

PrefsDialog::PrefsDialog(void) : CPropertySheet("Preferences")
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
  PrefsEditPage editPage;
  editPage.ReadSettings();
  AddPage(&editPage);

  PrefsTextPage textPage;
  textPage.ReadSettings();
  AddPage(&textPage);

  PrefsAdvancedPage advPage;
  advPage.ReadSettings();
  AddPage(&advPage);

  // Show the dialog
  if (DoModal() == IDOK)
  {
    // Store the new settings
    editPage.WriteSettings();
    textPage.WriteSettings();
    advPage.WriteSettings();

    // Notify all project windows
    theApp.SendAllFrames(InformApp::Preferences,0);
  }
}
