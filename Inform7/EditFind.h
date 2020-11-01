#pragma once

class SourceEdit;
class FindReplaceDialog;

class EditFind
{
public:
  EditFind();

  void Create(SourceEdit* edit, bool findOnly);
  void Destroy(void);
  LRESULT FindReplaceCmd(WPARAM wParam, LPARAM lParam);

  const CStringW& GetLastFind(void);
  void RepeatFind(bool forward);

private:
  bool FindNext(bool fromSelect, bool forward);
  bool Select(const CHARRANGE& range);
  bool Replace(void);
  bool ReplaceAll(void);

  CStringW GetInitialFindText(void);

  FindReplaceDialog* m_dialog;
  bool m_findOnly;
  CStringW m_lastFind;

  SourceEdit* m_edit;
};
