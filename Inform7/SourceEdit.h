#pragma once

#include "EditFind.h"
#include "SearchWindow.h"
#include "SpellCheck.h"
#include "SourceLexer.h"

#include "Platform.h"
#include "Scintilla.h"

class SourceEdit : public CWnd
{
  DECLARE_DYNAMIC(SourceEdit)

public:
  SourceEdit();
  BOOL Create(CWnd* parent, UINT id);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnDestroy();
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

  afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
  afx_msg void OnEditUndo();
  afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
  afx_msg void OnEditRedo();
  afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
  afx_msg void OnEditCut();
  afx_msg void OnEditCopy();
  afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
  afx_msg void OnEditPaste();
  afx_msg void OnEditSelectAll();
  afx_msg void OnUpdateNeedText(CCmdUI* pCmdUI);
  afx_msg void OnEditFind();
  afx_msg void OnEditReplace();
  afx_msg void OnEditSpelling();
  afx_msg void OnEditDelete();
  afx_msg void OnFormatShift(UINT id);
  afx_msg void OnFormatComment(UINT id);
  afx_msg void OnFormatRenumber();
  afx_msg void OnTextSize(UINT id);

  afx_msg void OnSavePointReached(NMHDR*, LRESULT*);
  afx_msg void OnSavePointLeft(NMHDR*, LRESULT*);
  afx_msg void OnStyleNeeded(NMHDR*, LRESULT*);
  afx_msg void OnCharAdded(NMHDR* hdr, LRESULT* res);
  afx_msg void OnConvertPaste(NMHDR*, LRESULT*);
  afx_msg void OnConvertCopyToClip(NMHDR*, LRESULT*);

  afx_msg LRESULT OnFindReplaceCmd(WPARAM wParam, LPARAM lParam);

  virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
  void SetDocument(SourceEdit* master);
  void OpenFile(CFile* file);
  bool SaveFile(CFile* file);
  bool IsEdited(void);

  void Search(LPCWSTR text, std::vector<SearchWindow::Result>& results);
  void Highlight(CHARRANGE range, bool centre);
  void Highlight(int line, COLORREF colour, bool centre);
  void ShowBetween(int startLine, int endLine);
  bool IsLineShown(int line);

  void PasteCode(const wchar_t* code);
  void SetTextSize(int size);

  void UpdateSpellCheck(void);
  void MoveShowSelect(CWnd* child);

  CHARRANGE GetSelect(void);
  RECT GetSelectRect(void);
  void SetSelect(CHARRANGE select);
  void ReplaceSelect(LPCWSTR text);
  CHARRANGE GetCurrentWord(void);
  CHARRANGE GetNextWord(CHARRANGE word);
  CStringW GetTextRange(int cpMin, int cpMax, int len = -1);
  CHARRANGE FindText(LPCWSTR text, bool fromSelect, bool down, bool matchCase, bool wholeWord);

  void LoadSettings(CRegKey& key, bool prefsChange);
  void SaveSettings(CRegKey& key);

  void GetAllHeadings(CArray<SourceLexer::Heading>& headings);
  int GetLineHeight(void);
  CHARRANGE GetRangeLines(CHARRANGE range);

  void ConvertPasteText(LPDATAOBJECT obj, wchar_t* uptr, unsigned int ulen, char*& ptr, unsigned int& len);

private:
  LONG_PTR CallEdit(UINT msg, DWORD wp = 0, LONG_PTR lp = 0);

  bool GetNextLine(const CStringW& text, CStringW& line, int& i);
  void TokenizeLine(const CStringW& line, CArray<CStringW>& tokens);

private:
  sptr_t m_editPtr;

  int m_marker;
  CHARRANGE m_markSel;

  int m_textSize;
  bool m_autoIndent;

  EditFind m_find;
  SpellCheck m_spell;

  friend class SourceLexer;
};
