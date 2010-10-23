#include "stdafx.h"
#include "EditFind.h"
#include "FindReplaceDialog.h"
#include "SourceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT EditFind::FINDMSG = ::RegisterWindowMessage(FINDMSGSTRING);

EditFind::EditFind() : m_dialog(NULL)
{
  m_findOnly = true;
  m_searchDown = TRUE;
  m_matchCase = FALSE;
  m_matchWholeWord = FALSE;
}

void EditFind::Create(SourceEdit* edit, bool findOnly)
{
  if (m_dialog != NULL)
  {
    if (findOnly == m_findOnly)
    {
      m_dialog->SetActiveWindow();
      m_dialog->ShowWindow(SW_SHOW);
      return;
    }
    Destroy();
  }

  m_edit = edit;
  m_findOnly = findOnly;

  // Use the current selection or, if empty, the previous search text to
  // populate the find dialog.
  CHARRANGE sel = edit->GetSelect();
  CStringW findText = edit->GetTextRange(sel.cpMin,sel.cpMax);
  if (findText.IsEmpty())
    findText = m_lastFind;

  m_dialog = FindReplaceDialog::Create(findOnly ? TRUE : FALSE,findText,
    m_searchDown,m_matchCase,m_matchWholeWord,edit);
  m_dialog->SetActiveWindow();
  m_dialog->ShowWindow(SW_SHOW);
}

void EditFind::Destroy(void)
{
  if ((m_dialog != NULL) && ::IsWindow(m_dialog->m_hWnd))
    m_dialog->SendMessage(WM_CLOSE);
  m_dialog = NULL;
  m_edit = NULL;
}

LRESULT EditFind::FindReplaceCmd(WPARAM, LPARAM lParam)
{
  FindReplaceDialog* dialog = FindReplaceDialog::GetNotifier(lParam);
  ASSERT(dialog == m_dialog);

  bool found = true;

  if (dialog->IsTerminating())
  {
    m_lastFind = dialog->GetFindString();
    m_searchDown = dialog->SearchDown();
    m_matchCase = dialog->MatchCase();
    m_matchWholeWord = dialog->MatchWholeWord();
    m_dialog = NULL;
  }
  else if (dialog->FindNext())
  {
    found = FindNext(true);
    if (!found)
      found = FindNext(false);
  }
  else if (dialog->ReplaceCurrent())
    found = Replace();
  else if (dialog->ReplaceAll())
    found = ReplaceAll();

  if (!found)
    ::MessageBeep(MB_ICONEXCLAMATION);
  return 0;
}

bool EditFind::FindNext(bool fromSelect)
{
  ASSERT(m_edit != NULL);

  // Search for the text
  CHARRANGE found = m_edit->FindText(m_dialog->GetFindString(),fromSelect,
    (m_dialog->SearchDown() != 0),(m_dialog->MatchCase() != 0),(m_dialog->MatchWholeWord() != 0));

  // Was there a match?
  if (found.cpMin >= 0)
  {
    // Is the match visible?
    CHARRANGE lines = m_edit->GetRangeLines(found);
    if (m_edit->IsLineShown(lines.cpMin) && m_edit->IsLineShown(lines.cpMax))
    {
      // Select the newly found text
      m_edit->SetSelect(found);
      m_edit->MoveShowSelect(m_dialog);
      return true;
    }
  }
  return false;
}

bool EditFind::Replace(void)
{
  ASSERT(m_findOnly == false);

  // Get the text in the current selection
  CHARRANGE sel = m_edit->GetSelect();
  CStringW selText = m_edit->GetTextRange(sel.cpMin,sel.cpMax);

  // If the current selection matches the text in the dialog, replace it
  if (m_dialog->MatchCase())
  {
    if (selText.Compare(m_dialog->GetFindString()) == 0)
      m_edit->ReplaceSelect(m_dialog->GetReplaceString());
  }
  else
  {
    if (selText.CompareNoCase(m_dialog->GetFindString()) == 0)
      m_edit->ReplaceSelect(m_dialog->GetReplaceString());
  }

  // Find the next occurence
  return FindNext(true);
}

bool EditFind::ReplaceAll(void)
{
  ASSERT(m_findOnly == false);

  // Does the current selection match?
  bool currentMatch = false;
  {
    CHARRANGE sel = m_edit->GetSelect();
    CStringW selText = m_edit->GetTextRange(sel.cpMin,sel.cpMax);

    if (m_dialog->MatchCase())
    {
      if (selText.Compare(m_dialog->GetFindString()) == 0)
        currentMatch = true;
    }
    else
    {
      if (selText.CompareNoCase(m_dialog->GetFindString()) == 0)
        currentMatch = true;
    }
  }

  if (currentMatch || FindNext(true))
  {
    while (Replace());
    return true;
  }
  return false;
}
