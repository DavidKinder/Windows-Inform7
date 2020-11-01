#include "stdafx.h"
#include "FindReplaceDialog.h"
#include "Inform.h"
#include "Messages.h"
#include "UnicodeEdit.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*XXXXDK
typedef UINT_PTR (CALLBACK* COMMDLGPROC)(HWND, UINT, WPARAM, LPARAM);

class FindReplaceDialogImpl : public FindReplaceDialog
{
  DECLARE_DYNAMIC(FindReplaceDialogImpl)

public:
  CStringW GetReplaceString(void) const
  {
    return CStringW(m_fr.lpstrReplaceWith);
  }

  CStringW GetFindString(void) const
  {
    return CStringW(m_fr.lpstrFindWhat);
  }

  BOOL SearchDown(void) const
  {
    return m_fr.Flags & FR_DOWN;
  }

  BOOL FindNext(void) const
  {
    return m_fr.Flags & FR_FINDNEXT;
  }

  BOOL MatchCase(void) const
  {
    return m_fr.Flags & FR_MATCHCASE;
  }

  BOOL MatchWholeWord(void) const
  {
    return m_fr.Flags & FR_WHOLEWORD;
  }

  BOOL ReplaceCurrent(void) const
  {
    return m_fr. Flags & FR_REPLACE;
  }

  BOOL ReplaceAll(void) const
  {
    return m_fr.Flags & FR_REPLACEALL;
  }

  BOOL IsTerminating(void) const
  {
    return m_fr.Flags & FR_DIALOGTERM;
  }

  FindReplaceDialogImpl(BOOL findOnly, LPCWSTR findWhat,
    BOOL searchDown, BOOL matchCase, const BOOL* matchWholeWord, CWnd* parentWnd)
  {
    memset(&m_fr,0,sizeof(m_fr));
    m_findWhat[0] = L'\0';
    m_replaceWith[0] = L'\0';

    m_fr.Flags = FR_ENABLEHOOK;
    if (searchDown)
      m_fr.Flags |= FR_DOWN;
    if (matchCase)
      m_fr.Flags |= FR_MATCHCASE;
    if (matchWholeWord && *matchWholeWord)
      m_fr.Flags |= FR_WHOLEWORD;
    else if (!matchWholeWord)
      m_fr.Flags |= FR_HIDEWHOLEWORD;

    m_fr.lpfnHook = (COMMDLGPROC)_AfxCommDlgProc;
    m_fr.lStructSize = sizeof m_fr;
    m_fr.hwndOwner = parentWnd->GetSafeHwnd();
    m_nIDHelp = findOnly ? AFX_IDD_FIND : AFX_IDD_REPLACE;

    m_fr.lpstrFindWhat = (LPWSTR)m_findWhat;
    m_fr.wFindWhatLen = sizeof m_findWhat / sizeof m_findWhat[0];
    m_fr.lpstrReplaceWith = (LPWSTR)m_replaceWith;
    m_fr.wReplaceWithLen = sizeof m_replaceWith / sizeof m_replaceWith[0];

    if (findWhat != NULL)
      ::lstrcpynW(m_findWhat,findWhat,m_fr.wFindWhatLen);

    AfxHookWindowCreate(this);
    if (findOnly)
      ::FindTextW(&m_fr);
    else
      ::ReplaceTextW(&m_fr);
    if (!AfxUnhookWindowCreate())
      PostNcDestroy();
  }

  FINDREPLACEW m_fr;
  WCHAR m_findWhat[128];
  WCHAR m_replaceWith[128];
};

IMPLEMENT_DYNAMIC(FindReplaceDialogImpl, FindReplaceDialog)
*/

IMPLEMENT_DYNAMIC(FindReplaceDialog, I7BaseDialog)

BEGIN_MESSAGE_MAP(FindReplaceDialog, I7BaseDialog)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
  ON_BN_CLICKED(IDC_FIND_PREVIOUS, OnFindPrevious)
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
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
  
  if (wcslen(findText) > 0)
  {
    UpdateData(TRUE);
    m_findText = CStringW(findText).Trim();
    UpdateData(FALSE);
  }

  CWnd* find = GetDlgItem(IDC_FIND);
  find->SendMessage(EM_SETSEL,0,-1);
  find->SendMessage(EM_SCROLLCARET);
  find->SetFocus();
}

FindReplaceDialog::FindReplaceDialog(UINT id, CWnd* parentWnd) : I7BaseDialog(id,parentWnd)
{
  m_ignoreCase = TRUE;
}

void FindReplaceDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_TextW(pDX, IDC_FIND, m_findText);
  DDX_Check(pDX,IDC_IGNORE_CASE,m_ignoreCase);
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
  return L"";//XXXXDK
}

bool FindReplaceDialog::MatchCase(void) const
{
  return (m_ignoreCase == FALSE);
}

BOOL FindReplaceDialog::MatchWholeWord(void) const
{
  return FALSE;//XXXXDK
}

BOOL FindReplaceDialog::ReplaceCurrent(void) const
{
  return FALSE;//XXXXDK
}

BOOL FindReplaceDialog::ReplaceAll(void) const
{
  return FALSE;//XXXXDK
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
  if (!m_findText.IsEmpty())
    m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Next);
}

void FindReplaceDialog::OnFindPrevious()
{
  UpdateData(TRUE);
  if (!m_findText.IsEmpty())
    m_pParentWnd->SendMessage(WM_FINDREPLACECMD,FindCmd_Previous);
}

//XXXXDK doesn't work!
LRESULT FindReplaceDialog::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
  if (GetSafeHwnd() != 0)
  {
    UpdateData(TRUE);
    BOOL hasFindText = !m_findText.IsEmpty();
    GetDlgItem(IDC_FIND_NEXT)->EnableWindow(hasFindText);
    GetDlgItem(IDC_FIND_PREVIOUS)->EnableWindow(hasFindText);
  }
  return 0;
}
