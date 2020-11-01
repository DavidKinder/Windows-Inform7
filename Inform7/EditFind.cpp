#include "stdafx.h"
#include "EditFind.h"
#include "FindReplaceDialog.h"
#include "Inform.h"
#include "Messages.h"
#include "SourceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EditFind::EditFind() : m_dialog(NULL), m_edit(NULL)
{
  m_findOnly = true;
}

void EditFind::Create(SourceEdit* edit, bool findOnly)
{
  if (m_dialog != NULL)
  {
    if (findOnly == m_findOnly)
    {
      CStringW findText;
      if (m_dialog->IsWindowVisible() == FALSE)
        findText = GetInitialFindText();
      m_dialog->Show(findText);
      return;
    }
    Destroy();
    //XXXXDK save state for other dialog?
  }

  m_edit = edit;
  m_findOnly = findOnly;

  m_dialog = FindReplaceDialog::Create(findOnly ? IDD_FIND : IDD_REPLACE,edit);
  if (m_dialog)
    m_dialog->Show(GetInitialFindText());
}

void EditFind::Destroy(void)
{
  delete m_dialog;
  m_dialog = NULL;
  m_edit = NULL;
}

LRESULT EditFind::FindReplaceCmd(WPARAM wParam, LPARAM lParam)
{
  bool found = true;

  ASSERT(m_dialog != NULL);
  if (m_dialog)
  {
    switch (wParam)
    {
    case FindCmd_Next:
      found = FindNext(true,true);
      if (!found)
        found = FindNext(false,true);
      break;
    case FindCmd_Previous:
      found = FindNext(true,false);
      if (!found)
        found = FindNext(false,false);
      break;
    }
  }

  if (!found)
    ::MessageBeep(MB_ICONEXCLAMATION);
  return 0;
}

const CStringW& EditFind::GetLastFind(void)
{
  return m_lastFind;
}

void EditFind::RepeatFind(bool forward)
{
  // Search for the text
  CHARRANGE found = m_edit->FindText(m_lastFind,true,forward,FALSE,FALSE);//XXXXDK m_matchCase,m_matchWholeWord);

  // Was there a match?
  if (found.cpMin >= 0)
    Select(found);
}

bool EditFind::FindNext(bool fromSelect, bool forward)
{
  ASSERT(m_edit != NULL);

  // Search for the text
  CHARRANGE found = m_edit->FindText(m_dialog->GetFindString(),fromSelect,forward,
    (m_dialog->MatchCase() != 0),(m_dialog->MatchWholeWord() != 0));

  // Was there a match?
  if (found.cpMin >= 0)
  {
    if (Select(found))
    {
      m_edit->MoveShowSelect(m_dialog);
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
  return FindNext(true,true);
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

  if (currentMatch || FindNext(true,true))
  {
    while (Replace());
    return true;
  }
  return false;
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
