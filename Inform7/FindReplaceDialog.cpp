#include "stdafx.h"
#include "FindReplaceDialog.h"
#include "Inform.h"
#include "Messages.h"
#include "RichEdit.h"
#include "UnicodeEdit.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FindReplaceDialog, I7BaseDialog)

BEGIN_MESSAGE_MAP(FindReplaceDialog, I7BaseDialog)
  ON_WM_CLOSE()
  ON_WM_DESTROY()
  ON_WM_DRAWITEM()
  ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
  ON_BN_CLICKED(IDC_FIND_PREVIOUS, OnFindPrevious)
  ON_EN_CHANGE(IDC_FIND, OnChangeFindText)
  ON_BN_CLICKED(IDC_REPLACE, OnReplace)
  ON_BN_CLICKED(IDC_REPLACE_ALL, OnReplaceAll)
  ON_EN_CHANGE(IDC_REPLACE_WITH, OnChangeReplaceWith)
  ON_CBN_SELCHANGE(IDC_FIND_RULE, OnChangeFindRule)
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
  theApp.SetIcon(dialog);
  dialog->PrepareHelp();
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
  m_ignoreCase = TRUE;
  m_findRule = FindRule_Contains;
  m_richText = NULL;
  m_heightNormal = 0;
  m_heightHelp = 0;
}

void FindReplaceDialog::PrepareHelp(void)
{
  CRect dlgRect;
  GetWindowRect(dlgRect);
  m_heightNormal = dlgRect.Height();
  m_heightHelp = m_heightNormal;

  // Figure out sizes for showing regular expression help
  if (m_regexHelp.GetSafeHwnd())
  {
    if (m_richText == NULL)
      m_richText = new RichDrawText();

    CSize textSize = theApp.MeasureText(this,"HELP");
    int helpLines = GetDlgItem(IDC_REPLACE) ? 11 : 10;

    CRect helpRect;
    m_regexHelp.GetWindowRect(helpRect);
    int gap = dlgRect.bottom - helpRect.bottom;

    ScreenToClient(helpRect);
    helpRect.top = gap + helpRect.bottom;
    helpRect.right = dlgRect.Width() - helpRect.left*2;
    helpRect.bottom = helpRect.top + textSize.cy*helpLines;
    m_regexHelp.MoveWindow(helpRect);
    m_heightHelp += (gap + helpRect.Height());
  }
}

void FindReplaceDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_TextW(pDX, IDC_FIND, m_findText);
  DDX_Check(pDX,IDC_IGNORE_CASE,m_ignoreCase);
  if (GetDlgItem(IDC_FIND_RULE))
    DDX_CBIndex(pDX,IDC_FIND_RULE,(int&)m_findRule);
  if (GetDlgItem(IDC_REPLACE_WITH))
    DDX_TextW(pDX, IDC_REPLACE_WITH, m_replaceWith);
  if (GetDlgItem(IDC_REGEX_HELP))
    DDX_Control(pDX, IDC_REGEX_HELP, m_regexHelp);
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
    CRect helpRect(di->rcItem);
    HDC hdc = 0;
    HANDLE pb = ::BeginBufferedPaint(di->hDC,helpRect,BPBF_COMPATIBLEBITMAP,NULL,&hdc);
    if (pb == 0)
      return;
    CDC* dc = CDC::FromHandle(hdc);
    dc->FillSolidRect(helpRect,::GetSysColor(COLOR_BTNFACE));

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
    CString rtfText;
    rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
      theApp.GetFontSize(InformApp::FontSystem)*2,text1);
    m_richText->SetTextRTF(rtfText);
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
    rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
      theApp.GetFontSize(InformApp::FontSystem)*2,text2);
    m_richText->SetTextRTF(rtfText);
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
    rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
      theApp.GetFontSize(InformApp::FontSystem)*2,text3);
    m_richText->SetTextRTF(rtfText);
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

void FindReplaceDialog::OnFindNext()
{
  UpdateData(TRUE);
  m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Next);
}

void FindReplaceDialog::OnFindPrevious()
{
  UpdateData(TRUE);
  m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Previous);
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
    dlgRect.bottom = dlgRect.top + m_heightHelp;
    MoveWindow(dlgRect);

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

void FindReplaceDialog::EnableActions(void)
{
  BOOL canFind = !m_findText.IsEmpty();
  GetDlgItem(IDC_FIND_NEXT)->EnableWindow(canFind);
  GetDlgItem(IDC_FIND_PREVIOUS)->EnableWindow(canFind);

  BOOL canReplace = !m_findText.IsEmpty() && !m_replaceWith.IsEmpty();
  CWnd* button = GetDlgItem(IDC_REPLACE);
  if (button)
    button->EnableWindow(canReplace);
  button = GetDlgItem(IDC_REPLACE_ALL);
  if (button)
    button->EnableWindow(canReplace);
}
