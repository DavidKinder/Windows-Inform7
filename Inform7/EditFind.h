#pragma once

#include "FindInFiles.h"

class SourceEdit;
class FindReplaceDialog;

class EditFind
{
public:
  EditFind();

  void CreateFind(SourceEdit* edit);
  void CreateReplace(SourceEdit* edit);
  void Destroy(void);

  LRESULT FindReplaceCmd(WPARAM wParam, LPARAM lParam);

  const CStringW& GetLastFind(void);
  void RepeatFind(bool forward);
  void SourceChanged(void);

private:
  bool FindNext(FindReplaceDialog* current, bool fromSelect, bool forward);
  CHARRANGE FindText(LPCWSTR text, bool fromSelect, bool down, bool matchCase, FindRule findRule);
  bool Select(const CHARRANGE& range);
  bool Replace(void);
  bool ReplaceAll(void);

  FindReplaceDialog* GetCurrentDialog(void);
  CStringW GetInitialFindText(void);

  FindReplaceDialog* m_dialogFind;
  FindReplaceDialog* m_dialogReplace;

  SourceEdit* m_edit;
  CString m_lastSource;

  CStringW m_lastFind;
  bool m_lastMatchCase;
  FindRule m_lastFindRule;
};
