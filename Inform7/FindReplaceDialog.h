#pragma once

class FindReplaceDialog : public CCommonDialog
{
  DECLARE_DYNAMIC(FindReplaceDialog)

public:
  static FindReplaceDialog* Create(BOOL findOnly, LPCWSTR findWhat,
    BOOL searchDown, BOOL matchCase, BOOL matchWholeWord, CWnd* parentWnd);
  static FindReplaceDialog* PASCAL GetNotifier(LPARAM lparam);

  virtual CStringW GetReplaceString(void) const = 0;
  virtual CStringW GetFindString(void) const = 0;
  virtual BOOL SearchDown(void) const = 0;
  virtual BOOL FindNext(void) const = 0;
  virtual BOOL MatchCase(void) const = 0;
  virtual BOOL MatchWholeWord(void) const = 0;
  virtual BOOL ReplaceCurrent(void) const = 0;
  virtual BOOL ReplaceAll(void) const = 0;
  virtual BOOL IsTerminating(void) const = 0;

protected:
  FindReplaceDialog();
  virtual void PostNcDestroy();
};
