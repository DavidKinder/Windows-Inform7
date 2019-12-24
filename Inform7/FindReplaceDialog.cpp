#include "stdafx.h"
#include "FindReplaceDialog.h"
#include "OSLayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

IMPLEMENT_DYNAMIC(FindReplaceDialog, CDialog)
IMPLEMENT_DYNAMIC(FindReplaceDialogImpl, FindReplaceDialog)

FindReplaceDialog* FindReplaceDialog::Create(BOOL findOnly, LPCWSTR findWhat,
  BOOL searchDown, BOOL matchCase, const BOOL* matchWholeWord, CWnd* parentWnd)
{
  return new FindReplaceDialogImpl(findOnly,findWhat,searchDown,matchCase,matchWholeWord,parentWnd);
}

FindReplaceDialog* PASCAL FindReplaceDialog::GetNotifier(LPARAM lparam)
{
  return (FindReplaceDialogImpl*)(lparam - offsetof(FindReplaceDialogImpl,m_fr));
}

FindReplaceDialog::FindReplaceDialog() : CCommonDialog(NULL)
{
}

void FindReplaceDialog::PostNcDestroy()
{
  delete this;
}
