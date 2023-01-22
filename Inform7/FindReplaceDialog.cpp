#include "stdafx.h"
#include "FindReplaceDialog.h"
#include "ExtensionFrame.h"
#include "Inform.h"
#include "Messages.h"
#include "ProjectFrame.h"
#include "Resource.h"
#include "RichEdit.h"
#include "SourceEdit.h"
#include "UnicodeEdit.h"

#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FindReplaceDialog, I7BaseDialog)

BEGIN_MESSAGE_MAP(FindReplaceDialog, I7BaseDialog)
  ON_WM_CLOSE()
  ON_WM_DESTROY()
  ON_WM_DRAWITEM()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
  ON_BN_CLICKED(IDC_FIND_PREVIOUS, OnFindPrevious)
  ON_BN_CLICKED(IDC_FIND_ALL, OnFindAll)
  ON_EN_CHANGE(IDC_FIND, OnChangeFindText)
  ON_BN_CLICKED(IDC_REPLACE, OnReplace)
  ON_BN_CLICKED(IDC_REPLACE_ALL, OnReplaceAll)
  ON_EN_CHANGE(IDC_REPLACE_WITH, OnChangeReplaceWith)
  ON_CBN_SELCHANGE(IDC_FIND_RULE, OnChangeFindRule)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_RESULTS, OnResultsDraw)
  ON_NOTIFY(NM_DBLCLK, IDC_RESULTS, OnResultsSelect)
  ON_NOTIFY(NM_RETURN, IDC_RESULTS, OnResultsSelect)
  ON_MESSAGE(WM_RESIZERESULTS, OnResultsResize)
END_MESSAGE_MAP()

FindReplaceDialog* FindReplaceDialog::Create(UINT id, CWnd* parentWnd)
{
  FindReplaceDialog* dialog = new FindReplaceDialog(id,parentWnd);
  if (dialog->I7BaseDialog::Create(dialog->m_lpszTemplateName,parentWnd) == FALSE)
  {
    ASSERT(FALSE);
    delete dialog;
    return NULL;
  }
  dialog->InitDialog();
  return dialog;
}

void FindReplaceDialog::Show(LPCWSTR findText)
{
  SetActiveWindow();
  ShowWindow(SW_SHOW);
  
  UpdateData(TRUE);
  if (wcslen(findText) > 0)
  {
    m_findText = CStringW(findText).Trim();
    UpdateData(FALSE);
  }

  EnableActions();
  CWnd* find = GetDlgItem(IDC_FIND);
  find->SendMessage(EM_SETSEL,0,-1);
  find->SendMessage(EM_SCROLLCARET);
  find->SetFocus();
}

FindReplaceDialog::FindReplaceDialog(UINT id, CWnd* parentWnd) : I7BaseDialog(id,parentWnd)
{
  m_dpi = 96;
  m_ignoreCase = TRUE;
  m_findRule = FindRule_Contains;
  m_richText = NULL;
  m_heightNormal = 0;
  m_heightLong = 0;
}

void FindReplaceDialog::InitDialog(void)
{
  m_dpi = DPI::getWindowDPI(this);
  theApp.SetIcon(this);
  if (m_resultsList.GetSafeHwnd() != 0)
    m_findHelper.InitResultsCtrl(&m_resultsList,false);

  CRect dlgRect;
  GetWindowRect(dlgRect);
  m_heightNormal = dlgRect.Height();
  m_heightLong = m_heightNormal;

  // Figure out sizes for showing regular expression help
  if (m_regexHelp.GetSafeHwnd())
  {
    if (m_richText == NULL)
      m_richText = new RichDrawText(InformApp::FontSystem);

    CSize textSize = theApp.MeasureText(this,"HELP");
    int helpLines = GetDlgItem(IDC_REPLACE) ? 11 : 10;

    CRect helpRect;
    m_regexHelp.GetWindowRect(helpRect);
    int gap = dlgRect.bottom - helpRect.bottom;

    ScreenToClient(helpRect);
    helpRect.top = gap + helpRect.bottom;
    helpRect.right = dlgRect.Width() - (2*helpRect.left);
    helpRect.bottom = helpRect.top + (textSize.cy*helpLines);
    m_regexHelp.MoveWindow(helpRect);
    m_heightLong += (gap + helpRect.Height());

    if (m_resultsList.GetSafeHwnd() != 0)
    {
      CRect resultsRect(helpRect), findNextRect;
      m_findNext.GetWindowRect(findNextRect);
      ScreenToClient(findNextRect);
      resultsRect.right = findNextRect.right;
      m_resultsList.MoveWindow(resultsRect);
    }
  }
}

void FindReplaceDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_TextW(pDX, IDC_FIND, m_findText);
  DDX_Control(pDX, IDC_FIND_NEXT, m_findNext);
  DDX_Control(pDX, IDC_FIND_PREVIOUS, m_findPrev);
  if (GetDlgItem(IDC_FIND_ALL))
    DDX_Control(pDX, IDC_FIND_ALL, m_findAll);
  if (GetDlgItem(IDC_REPLACE))
    DDX_Control(pDX, IDC_REPLACE, m_replace);
  if (GetDlgItem(IDC_REPLACE_ALL))
    DDX_Control(pDX, IDC_REPLACE_ALL, m_replaceAll);
  DDX_Control(pDX, IDC_HOW_GROUP, m_howGroup);
  DDX_Check(pDX,IDC_IGNORE_CASE,m_ignoreCase);
  if (GetDlgItem(IDC_FIND_RULE))
  {
    DDX_Control(pDX,IDC_FIND_RULE,m_findRuleCtrl);
    DDX_CBIndex(pDX,IDC_FIND_RULE,(int&)m_findRule);
  }
  if (GetDlgItem(IDC_REPLACE_WITH))
    DDX_TextW(pDX, IDC_REPLACE_WITH, m_replaceWith);
  if (GetDlgItem(IDC_RESULTS))
    DDX_Control(pDX, IDC_RESULTS, m_resultsList);
  if (GetDlgItem(IDC_REGEX_HELP))
    DDX_Control(pDX, IDC_REGEX_HELP, m_regexHelp);
}

BOOL FindReplaceDialog::OnInitDialog()
{
  if (I7BaseDialog::OnInitDialog())
  {
    m_ignoreCaseCtrl.SubclassDlgItem(IDC_IGNORE_CASE,this,IDR_DARK_CHECK,DarkMode::Darkest);
    return TRUE;
  }
  return FALSE;
}

void FindReplaceDialog::OnCancel()
{
  SendMessage(WM_CLOSE);
}

CStringW FindReplaceDialog::GetFindString(void) const
{
  return m_findText;
}

CStringW FindReplaceDialog::GetReplaceString(void) const
{
  return m_replaceWith;
}

bool FindReplaceDialog::MatchCase(void) const
{
  return (m_ignoreCase == FALSE);
}

FindRule FindReplaceDialog::GetFindRule(void) const
{
  return m_findRule;
}

void FindReplaceDialog::OnClose()
{
  UpdateData(TRUE);
  m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Close);

  ShowWindow(SW_HIDE);
  GetParentFrame()->SetActiveWindow();
}

void FindReplaceDialog::OnDestroy()
{
  delete m_richText;
  m_richText = NULL;
}

void FindReplaceDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di)
{
  if (nIDCtl == IDC_REGEX_HELP)
  {
    DarkMode* dark = DarkMode::GetActive(this);

    CRect helpRect(di->rcItem);
    HDC hdc = 0;
    HANDLE pb = ::BeginBufferedPaint(di->hDC,helpRect,BPBF_COMPATIBLEBITMAP,NULL,&hdc);
    if (pb == 0)
      return;
    CDC* dc = CDC::FromHandle(hdc);

    if (dark)
    {
      dc->FillSolidRect(helpRect,dark->GetColour(DarkMode::Darkest));
      m_richText->TextColourChanged(dark->GetColour(DarkMode::Fore));
    }
    else
    {
      dc->FillSolidRect(helpRect,::GetSysColor(COLOR_BTNFACE));
      m_richText->TextColourChanged(theApp.GetColour(InformApp::ColourText));
    }

    const char* text1 =
      "\\b Characters\\b0\\par\\par"
      " .\\tab Any character\\par"
      " \\\\b\\tab Word boundary\\par"
      " \\\\d\\tab Any digit\\par"
      " \\\\n\\tab Newline\\par"
      " \\\\s\\tab Whitespace\\par"
      " \\\\S\\tab Non-whitespace\\par"
      " \\\\t\\tab Tab\\par"
      " [A-Z]\\tab Character ranges\\par";
    SetRichTextRTF(text1);
    m_richText->DrawText(*dc,helpRect);

    const char* text2 =
      "\\b Patterns\\b0\\par\\par"
      " x?\\tab 0 or 1 of 'x'\\par"
      " x*\\tab 0 or more of 'x'\\par"
      " x+\\tab 1 or more of 'x'\\par"
      " x\\{2\\}\\tab Exactly 2 of 'x'\\par"
      " x\\{2,\\}\\tab 2 or more of 'x'\\par"
      " x\\{,2\\}\\tab 0-2 of 'x'\\par"
      " x\\{2,4\\}\\tab 2-4 of 'x'\\par";
    SetRichTextRTF(text2);
    CRect textRect;
    textRect.SetRectEmpty();
    textRect.right = helpRect.Width();
    m_richText->SizeText(*dc,textRect);
    textRect.MoveToX((helpRect.right - textRect.Width())/2);
    m_richText->DrawText(*dc,textRect);

    const char* text3 = GetDlgItem(IDC_REPLACE) ?
      "\\b Expressions\\b0\\par\\par"
      " (x)\\tab Group expression\\par"
      " x|y\\tab x or y\\par"
      " ^\\tab Start of the line\\par"
      " $\\tab End of the line\\par\\par"
      "\\b Replace With\\b0\\par\\par"
      " \\\\1\\tab First group\\par"
      " \\\\2\\tab Second group\\par"
      :
      "\\b Expressions\\b0\\par\\par"
      " (x)\\tab Group expression\\par"
      " x|y\\tab x or y\\par"
      " ^\\tab Start of the line\\par"
      " $\\tab End of the line\\par";
    SetRichTextRTF(text3);
    textRect.SetRectEmpty();
    textRect.right = helpRect.Width();
    m_richText->SizeText(*dc,textRect);
    textRect.MoveToX(helpRect.right - textRect.Width());
    m_richText->DrawText(*dc,textRect);

    ::EndBufferedPaint(pb,TRUE);
  }
  else
    CWnd::OnDrawItem(nIDCtl,di);
}

LRESULT FindReplaceDialog::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  int newDpi = (int)HIWORD(wparam);

  // Get the positon of the results control before default processing
  CRect resultsRectBefore;
  if (m_resultsList.GetSafeHwnd() != 0)
  {
    m_resultsList.GetWindowRect(resultsRectBefore);
    ScreenToClient(resultsRectBefore);
  }

  Default();

  if (newDpi != m_dpi)
  {
    m_heightNormal = MulDiv(m_heightNormal,newDpi,m_dpi);
    m_heightLong = MulDiv(m_heightLong,newDpi,m_dpi);

    if (m_resultsList.GetSafeHwnd() != 0)
    {
      CRect resultsRectAfter, findNextRect;
      m_findNext.GetWindowRect(findNextRect);
      ScreenToClient(findNextRect);
      m_resultsList.GetWindowRect(resultsRectAfter);
      ScreenToClient(resultsRectAfter);

      resultsRectAfter.right = findNextRect.right;
      m_resultsList.MoveWindow(resultsRectAfter);

      double scaleX = (double)resultsRectAfter.right / (double)resultsRectBefore.right;
      m_resultsList.SetFont(theApp.GetFont(this,InformApp::FontSmall));
      m_resultsList.SetColumnWidth(0,(int)(scaleX*m_resultsList.GetColumnWidth(0)));
    }

    m_dpi = newDpi;
  }
  return 0;
}

void FindReplaceDialog::OnFindNext()
{
  // Handle the return key being pressed on the results list as a select action
  if ((GetFocus() == &m_resultsList) && (::GetKeyState(VK_RETURN) != 0))
  {
    int item = m_resultsList.GetNextItem(-1,LVNI_SELECTED);
    if ((item >= 0) && (item < (int)m_findHelper.results.size()))
      ShowResult(m_findHelper.results.at(item));
    return;
  }

  UpdateData(TRUE);
  m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Next);
}

void FindReplaceDialog::OnFindPrevious()
{
  UpdateData(TRUE);
  m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Previous);
}

void FindReplaceDialog::OnFindAll()
{
  UpdateData(TRUE);
  if (m_findText.IsEmpty())
    return;

  // Search for the text
  m_findHelper.results.clear();
  try
  {
    if (m_pParentWnd->IsKindOf(RUNTIME_CLASS(SourceEdit)))
    {
      SourceEdit* edit = (SourceEdit*)m_pParentWnd;
      CFrameWnd* frame = edit->GetParentFrame();

      CString docName;
      if (frame->IsKindOf(RUNTIME_CLASS(ProjectFrame)))
        docName = ((ProjectFrame*)frame)->GetDisplayName(true);
      else if (frame->IsKindOf(RUNTIME_CLASS(ExtensionFrame)))
        docName = ((ExtensionFrame*)frame)->GetDisplayName(true);

      m_findHelper.Find(edit->GetSource(),m_findText,m_ignoreCase,m_findRule,
        docName,"","","",FoundIn_Source);
    }
  }
  catch (std::regex_error& ex)
  {
    MessageBox(FindAllHelper::RegexError(ex),INFORM_TITLE,MB_ICONERROR|MB_OK);
  }
  std::sort(m_findHelper.results.begin(),m_findHelper.results.end());
  m_findHelper.UpdateResultsCtrl(&m_resultsList,false);

  // Set the height of the dialog
  CRect dlgRect;
  GetWindowRect(dlgRect);
  dlgRect.bottom = dlgRect.top + m_heightLong;
  MoveWindow(dlgRect);

  // Show the results and hide the regular expression help
  m_regexHelp.ModifyStyle(WS_VISIBLE,0);
  m_resultsList.ModifyStyle(0,WS_VISIBLE);
  m_resultsList.SetFocus();
  Invalidate();
}

void FindReplaceDialog::OnChangeFindText()
{
  UpdateData(TRUE);
  EnableActions();
}

void FindReplaceDialog::OnReplace()
{
  UpdateData(TRUE);
  m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Replace);
}

void FindReplaceDialog::OnReplaceAll()
{
  UpdateData(TRUE);
  m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_ReplaceAll);
}

void FindReplaceDialog::OnChangeReplaceWith()
{
  UpdateData(TRUE);
  EnableActions();
}

void FindReplaceDialog::OnChangeFindRule()
{
  UpdateData(TRUE);
  if (m_findRule == FindRule_Regex)
  {
    CRect dlgRect;
    GetWindowRect(dlgRect);
    dlgRect.bottom = dlgRect.top + m_heightLong;
    MoveWindow(dlgRect);

    if (m_resultsList.GetSafeHwnd() != 0)
      m_resultsList.ModifyStyle(WS_VISIBLE,0);
    m_regexHelp.ModifyStyle(0,WS_VISIBLE);
    Invalidate();
  }
  else if (m_regexHelp.IsWindowVisible())
  {
    CRect dlgRect;
    GetWindowRect(dlgRect);
    dlgRect.bottom = dlgRect.top + m_heightNormal;
    MoveWindow(dlgRect);

    m_regexHelp.ModifyStyle(WS_VISIBLE,0);
    Invalidate();
  }
}

void FindReplaceDialog::OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result)
{
  m_findHelper.OnResultsDraw(&m_resultsList,(NMLVCUSTOMDRAW*)pNotifyStruct,result);
}

void FindReplaceDialog::OnResultsSelect(NMHDR*, LRESULT* result)
{
  int item = m_resultsList.GetNextItem(-1,LVNI_SELECTED);
  if ((item >= 0) && (item < (int)m_findHelper.results.size()))
    ShowResult(m_findHelper.results.at(item));
  *result = 0;
}

LRESULT FindReplaceDialog::OnResultsResize(WPARAM, LPARAM)
{
  m_findHelper.OnResultsResize(&m_resultsList);
  return 0;
}

void FindReplaceDialog::EnableActions(void)
{
  BOOL canFind = !m_findText.IsEmpty();
  m_findNext.EnableWindow(canFind);
  m_findPrev.EnableWindow(canFind);
  if (m_findAll.GetSafeHwnd() != 0)
    m_findAll.EnableWindow(canFind);

  BOOL canReplace = !m_findText.IsEmpty() && !m_replaceWith.IsEmpty();
  if (m_replace.GetSafeHwnd() != 0)
    m_replace.EnableWindow(canReplace);
  if (m_replaceAll.GetSafeHwnd() != 0)
    m_replaceAll.EnableWindow(canReplace);
}

void FindReplaceDialog::SetRichTextRTF(const char* fragment)
{
  int sysDpi = DPI::getSystemDPI();
  int wndDpi = DPI::getWindowDPI(this);

  CString rtfText;
  rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
    (2*theApp.GetFontSize(InformApp::FontSystem)*wndDpi)/sysDpi,fragment);
  m_richText->SetTextRTF(rtfText);
}

void FindReplaceDialog::ShowResult(const FindResult& result)
{
  if (m_pParentWnd->IsKindOf(RUNTIME_CLASS(SourceEdit)))
  {
    SourceEdit* edit = (SourceEdit*)m_pParentWnd;

    // If the SourceEdit is in a SourceWindow, use that to highlight the result, as that will
    // show the whole of the source if needed, if only part of it is currently visible.
    CWnd* editParent = edit->GetParent();
    if (editParent->IsKindOf(RUNTIME_CLASS(SourceWindow)))
    {
      SourceWindow* source = (SourceWindow*)editParent;
      source->Highlight(result.loc,true);
    }
    else
      edit->Select(result.loc,true);

    // Move the dialog if necessary, so that the result is visible
    edit->MoveShowSelect(this);
  }
}
