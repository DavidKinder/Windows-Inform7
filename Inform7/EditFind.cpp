#include "stdafx.h"
#include "EditFind.h"
#include "FindReplaceDialog.h"
#include "Inform.h"
#include "Messages.h"
#include "SourceEdit.h"
#include "TextFormat.h"

#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EditFind::EditFind() : m_dialogFind(NULL), m_dialogReplace(NULL), m_edit(NULL)
{
  m_lastMatchCase = false;
  m_lastFindRule = FindRule_Contains;
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
    try
    {
      switch (wParam)
      {
      case FindCmd_Close:
        m_lastFind = current->GetFindString();
        m_lastMatchCase = current->MatchCase();
        m_lastFindRule = current->GetFindRule();
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
      case FindCmd_Replace:
        found = Replace(current);
        break;
      case FindCmd_ReplaceAll:
        found = ReplaceAll(current);
        break;
      }
    }
    catch (std::regex_error& ex)
    {
      current->MessageBox(FindAllHelper::RegexError(ex),INFORM_TITLE,MB_ICONERROR|MB_OK);
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
  try
  {
    // Search for the text
    CHARRANGE found = FindText(m_lastFind,true,forward,m_lastMatchCase,m_lastFindRule);

    // Was there a match?
    if (found.cpMin >= 0)
      Select(found);
  }
  catch (std::regex_error& ex)
  {
    m_edit->MessageBox(FindAllHelper::RegexError(ex),INFORM_TITLE,MB_ICONERROR|MB_OK);
  }
}

void EditFind::SourceChanged(void)
{
  // Discard the cached source code text, if any
  m_lastSource.Empty();
}

bool EditFind::FindNext(FindReplaceDialog* current, bool fromSelect, bool forward)
{
  // Search for the text
  CHARRANGE found = FindText(current->GetFindString(),
    fromSelect,forward,current->MatchCase(),current->GetFindRule());

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

CHARRANGE EditFind::FindText(LPCWSTR text, bool fromSelect, bool down, bool matchCase, FindRule findRule)
{
  ASSERT(m_edit != NULL);

  if (findRule != FindRule_Regex)
    return m_edit->FindText(text,fromSelect,down,matchCase,findRule);

  // If not already present, cache the source code text
  if (m_lastSource.IsEmpty())
    m_lastSource = m_edit->GetSource();

  // Set up a regular expression
  CString textUtf = TextFormat::UnicodeToUTF8(text);
  std::regex::flag_type flags = std::regex::ECMAScript;
  if (!matchCase)
    flags |= std::regex::icase;
  std::regex regexp;
  regexp.assign(textUtf,flags);

  // Work out where to start searching from
  int start = 0;
  if (fromSelect)
  {
    CHARRANGE select = m_edit->GetSelect();
    start = down ? select.cpMax : select.cpMin;
  }
  else
    start = down ? 0 : -1;

  // Search for the text
  CHARRANGE result = { -1,-1 };
  int len = m_lastSource.GetLength();
  if (start < len)
  {
    if (down)
    {
      std::cregex_iterator regexIt(m_lastSource.GetString()+start,m_lastSource.GetString()+len,regexp);
      if ((regexIt != std::cregex_iterator()) && (regexIt->length() > 0))
      {
        result.cpMin = start+(int)regexIt->position();
        result.cpMax = start+(int)(regexIt->position() + regexIt->length());
      }
    }
    else
    {
      // Search from the beginning for the last result before the start position
      std::cregex_iterator regexIt(m_lastSource.GetString(),m_lastSource.GetString()+len,regexp);
      for (; regexIt != std::cregex_iterator(); ++regexIt)
      {
        if (regexIt->length() > 0)
        {
          int matchStart = (int)regexIt->position();
          int matchEnd = (int)(regexIt->position() + regexIt->length());
          if ((start == -1) || (matchEnd < start))
          {
            result.cpMin = matchStart;
            result.cpMax = matchEnd;
          }
        }
      }
    }
  }
  return result;
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

// In a regular expression replacement string, we want:
//  \0 through \9 to be replaced with the corresponding found group
//  \\ to \
//  \t \r \v \f \n to newline or tab characters 8, 10, 11, 12, 13
//  \x09 etc. to UTF-8 code
void RegexReplaceString(CStringW& replace, const std::cmatch& match)
{
  bool foundSlash = false;
  for (int i = 0; i < replace.GetLength(); i++)
  {
    WCHAR c = replace.GetAt(i);
    if (c == L'\\')
    {
      if (foundSlash)
      {
        replace.Delete(i,1);
        i--;
        foundSlash = false;
      }
      else
        foundSlash = true;
    }
    else if (foundSlash)
    {
      if ((c >= L'0') && (c <= L'9'))
      {
        int group = (int)(c - L'0');
        if (group < match.size())
        {
          CStringW groupStr = TextFormat::UTF8ToUnicode(match[group].str().c_str());
          replace.Delete(i-1,2);
          replace.Insert(i-1,groupStr);
          i = (i-1) + (int)(groupStr.GetLength()-1);
        }
      }
      else if ((c == L't') || (c == L'r') || (c == L'v') || (c == L'f') || (c == L'n'))
      {
        if (c == L't')
          c = 9;
        else if (c == L'r')
          c = 10;
        else if (c == L'v')
          c = 11;
        else if (c == L'f')
          c = 12;
        else if (c == L'n')
          c = 13;
        replace.SetAt(i-1,c);
        replace.Delete(i,1);
        i--;
      }
      else if (c == L'x')
      {
        if (i < replace.GetLength()-2)
        {
          int hexInt = 0;
          if (swscanf(replace.Mid(i+1,2),L"%x",&hexInt) == 1)
          {
            replace.SetAt(i-1,hexInt);
            replace.Delete(i,3);
            i--;
          }
        }
      }
      foundSlash = false;
    }
  }
}

bool EditFind::Replace(FindReplaceDialog* current)
{
  // Get the text in the current selection
  CHARRANGE sel = m_edit->GetSelect();
  CStringW selText = m_edit->GetTextRange(sel.cpMin,sel.cpMax);

  // If the current selection matches the text in the dialog, replace it
  if (current->GetFindRule() == FindRule_Regex)
  {
    // Set up a regular expression
    CString findUtf = TextFormat::UnicodeToUTF8(current->GetFindString());
    std::regex::flag_type flags = std::regex::ECMAScript;
    if (!current->MatchCase())
      flags |= std::regex::icase;
    std::regex regexp;
    regexp.assign(findUtf,flags);

    // Run the regular expression on the selected text
    CString selTextUtf = TextFormat::UnicodeToUTF8(selText);
    std::cmatch matchResults;
    if (std::regex_match(selTextUtf.GetString(),selTextUtf.GetString()+selTextUtf.GetLength(),matchResults,regexp))
    {
      CStringW replace = current->GetReplaceString();
      RegexReplaceString(replace,matchResults);
      m_edit->ReplaceSelect(replace);
    }
  }
  else
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
  }

  // Find the next occurence
  return FindNext(current,true,true);
}

bool EditFind::ReplaceAll(FindReplaceDialog* current)
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
    while (Replace(current));
    return true;
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
