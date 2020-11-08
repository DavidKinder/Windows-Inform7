#include "stdafx.h"
#include "EditFind.h"
#include "FindReplaceDialog.h"
#include "Inform.h"
#include "Messages.h"
#include "SourceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EditFind::EditFind() : m_dialogFind(NULL), m_dialogReplace(NULL), m_edit(NULL)
{
  m_matchCase = false;
}

void EditFind::CreateFind(SourceEdit* edit)
{
  m_edit = edit;

  // If the find dialog is currently shown, just activate it
  if (m_dialogFind && m_dialogFind->IsWindowVisible())
  {
    m_dialogFind->Show(L"");
    return;
  }

  // If the replace dialog is currently shown, hide it
  if (m_dialogReplace && m_dialogReplace->IsWindowVisible())
    m_dialogReplace->ShowWindow(SW_HIDE);

  // Create the find dialog if it does not already exist
  if (m_dialogFind == NULL)
    m_dialogFind = FindReplaceDialog::Create(IDD_FIND,edit);

  // Show the find dialog
  m_dialogFind->Show(GetInitialFindText());
}

void EditFind::CreateReplace(SourceEdit* edit)
{
  m_edit = edit;

  // If the replace dialog is currently shown, just activate it
  if (m_dialogReplace && m_dialogReplace->IsWindowVisible())
  {
    m_dialogReplace->Show(L"");
    return;
  }

  // If the find dialog is currently shown, hide it
  if (m_dialogFind && m_dialogFind->IsWindowVisible())
    m_dialogFind->ShowWindow(SW_HIDE);

  // Create the replace dialog if it does not already exist
  if (m_dialogReplace == NULL)
    m_dialogReplace = FindReplaceDialog::Create(IDD_REPLACE,edit);

  // Show the replace dialog
  m_dialogReplace->Show(GetInitialFindText());
}

void EditFind::Destroy(void)
{
  delete m_dialogFind;
  m_dialogFind = NULL;
  delete m_dialogReplace;
  m_dialogReplace = NULL;
  m_edit = NULL;
}

LRESULT EditFind::FindReplaceCmd(WPARAM wParam, LPARAM lParam)
{
  bool found = true;

  FindReplaceDialog* current = GetCurrentDialog();
  ASSERT(current != NULL);
  if (current)
  {
    switch (wParam)
    {
    case FindCmd_Close:
      m_lastFind = current->GetFindString();
      m_matchCase = current->MatchCase();
      break;
    case FindCmd_Next:
      found = FindNext(current,true,true);
      if (!found)
        found = FindNext(current,false,true);
      break;
    case FindCmd_Previous:
      found = FindNext(current,true,false);
      if (!found)
        found = FindNext(current,false,false);
      break;
    }
  }

  if (!found)
    ::MessageBeep(MB_ICONEXCLAMATION);
  return 0;
}

const CStringW& EditFind::GetLastFind(void)
{
  FindReplaceDialog* current = GetCurrentDialog();
  if (current != NULL)
    m_lastFind = current->GetFindString();
  return m_lastFind;
}

void EditFind::RepeatFind(bool forward)
{
  // Search for the text
  CHARRANGE found = m_edit->FindText(m_lastFind,true,forward,m_matchCase,FALSE);//XXXXDK m_matchWholeWord

  // Was there a match?
  if (found.cpMin >= 0)
    Select(found);
}

bool EditFind::FindNext(FindReplaceDialog* current, bool fromSelect, bool forward)
{
  ASSERT(m_edit != NULL);

  // Search for the text
  CHARRANGE found = m_edit->FindText(current->GetFindString(),
    fromSelect,forward,current->MatchCase(),(current->MatchWholeWord() != 0));

  // Was there a match?
  if (found.cpMin >= 0)
  {
    if (Select(found))
    {
      m_edit->MoveShowSelect(current);
      return true;
    }
  }
  return false;
}

bool EditFind::Select(const CHARRANGE& range)
{
  // Is the range visible?
  CHARRANGE lines = m_edit->GetRangeLines(range);
  if (m_edit->IsLineShown(lines.cpMin) && m_edit->IsLineShown(lines.cpMax))
  {
    // Select the text in the range
    m_edit->SetSelect(range);
    return true;
  }
  return false;
}

bool EditFind::Replace(void)
{
  // Get the text in the current selection
  CHARRANGE sel = m_edit->GetSelect();
  CStringW selText = m_edit->GetTextRange(sel.cpMin,sel.cpMax);

  // If the current selection matches the text in the dialog, replace it
  FindReplaceDialog* current = GetCurrentDialog();
  ASSERT(current != NULL);
  if (current != NULL)
  {
    if (current->MatchCase())
    {
      if (selText.Compare(current->GetFindString()) == 0)
        m_edit->ReplaceSelect(current->GetReplaceString());
    }
    else
    {
      if (selText.CompareNoCase(current->GetFindString()) == 0)
        m_edit->ReplaceSelect(current->GetReplaceString());
    }

    // Find the next occurence
    return FindNext(current,true,true);
  }
  return false;
}

bool EditFind::ReplaceAll(void)
{
  FindReplaceDialog* current = GetCurrentDialog();
  ASSERT(current != NULL);
  if (current != NULL)
  {
    // Does the current selection match?
    bool currentMatch = false;
    {
      CHARRANGE sel = m_edit->GetSelect();
      CStringW selText = m_edit->GetTextRange(sel.cpMin,sel.cpMax);

      if (current->MatchCase())
      {
        if (selText.Compare(current->GetFindString()) == 0)
          currentMatch = true;
      }
      else
      {
        if (selText.CompareNoCase(current->GetFindString()) == 0)
          currentMatch = true;
      }
    }

    if (currentMatch || FindNext(current,true,true))
    {
      while (Replace());
      return true;
    }
  }
  return false;
}

FindReplaceDialog* EditFind::GetCurrentDialog(void)
{
  if (m_dialogFind && m_dialogFind->IsWindowVisible())
    return m_dialogFind;
  if (m_dialogReplace && m_dialogReplace->IsWindowVisible())
    return m_dialogReplace;
  return NULL;
}

CStringW EditFind::GetInitialFindText(void)
{
  // Return the current selection or the previous search text
  CStringW findText;
  if (theApp.GetProfileInt("Window","Find Uses Selection",0) != 0)
  {
    CHARRANGE sel = m_edit->GetSelect();
    findText = m_edit->GetTextRange(sel.cpMin,sel.cpMax);
  }
  if (findText.IsEmpty())
    findText = m_lastFind;
  return findText;
}
