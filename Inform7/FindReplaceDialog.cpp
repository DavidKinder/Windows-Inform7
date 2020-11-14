#include "stdafx.h"
#include "FindReplaceDialog.h"
#include "Inform.h"
#include "Messages.h"
#include "UnicodeEdit.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FindReplaceDialog, I7BaseDialog)

BEGIN_MESSAGE_MAP(FindReplaceDialog, I7BaseDialog)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
  ON_BN_CLICKED(IDC_FIND_PREVIOUS, OnFindPrevious)
  ON_EN_CHANGE(IDC_FIND, OnChangeFindText)
  ON_BN_CLICKED(IDC_REPLACE, OnReplace)
  ON_BN_CLICKED(IDC_REPLACE_ALL, OnReplaceAll)
  ON_EN_CHANGE(IDC_REPLACE_WITH, OnChangeReplaceWith)
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
