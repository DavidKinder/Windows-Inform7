#pragma once

#include "EditFind.h"
#include "SearchWindow.h"
#include "SpellCheck.h"
#include "SourceLexer.h"
#include "SourceSettings.h"

class SourceEdit : public CWnd
{
  DECLARE_DYNAMIC(SourceEdit)

public:
  SourceEdit();
  BOOL Create(CWnd* parent, UINT id, COLORREF back, bool includeExt);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnDestroy();
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  afx_msg UINT OnGetDlgCode();

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
  afx_msg void OnUpdateNeedFindText(CCmdUI* pCmdUI);
  afx_msg void OnEditFindNext();
  afx_msg void OnEditFindPrev();
  afx_msg void OnEditScroll();
  afx_msg void OnUpdateEditUseSel(CCmdUI *pCmdUI);
  afx_msg void OnEditUseSel();
  afx_msg void OnEditSpelling();
  afx_msg void OnEditDelete();
  afx_msg void OnFormatShift(UINT id);
  afx_msg void OnFormatComment(UINT id);
  afx_msg void OnFormatRenumber();

  afx_msg void OnSavePointReached(NMHDR*, LRESULT*);
  afx_msg void OnSavePointLeft(NMHDR*, LRESULT*);
  afx_msg void OnStyleNeeded(NMHDR*, LRESULT*);
  afx_msg void OnCharAdded(NMHDR* hdr, LRESULT* res);
  afx_msg void OnDocModified(NMHDR* hdr, LRESULT* res);
  afx_msg void OnConvertPaste(NMHDR*, LRESULT*);
  afx_msg void OnConvertCopyToClip(NMHDR*, LRESULT*);

  afx_msg LRESULT OnFindReplaceCmd(WPARAM wParam, LPARAM lParam);

  virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
  void SetStyles(COLORREF back);
  void SetReadOnly(bool readOnly);
  void SetShowScrollBars(bool show);
  void SetLineWrap(bool wrap);
  void HideCaret(void);
  void DisableUserControl(void);

  void SetDocument(SourceEdit* master);
  void OpenFile(CFile* file);
  bool SaveFile(CFile* file);
  bool IsEdited(void);
  const CTime& GetFileTime(void);

  void Search(LPCWSTR text, std::vector<SearchWindow::Result>& results, const char* sourceFile);
  void Select(CHARRANGE range, bool centre);
  void Highlight(int line, COLORREF colour, bool centre);
  void ShowBetween(int startLine, int endLine);
  bool IsLineShown(int line);

  void PasteCode(const wchar_t* code);
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

  void LoadSettings(SourceSettings& set, COLORREF back);
  void PrefsChanged(void);
  bool GetElasticTabStops(void);
  void SetElasticTabStops(bool enable);
  void SetCustomTabStops(int num, int tabPixels);
  int GetTabWidthPixels(void);

  void GetAllHeadings(CArray<SourceLexer::Heading>& headings);
  int GetLineHeight(void);
  CHARRANGE GetRangeLines(CHARRANGE range);

  CString GetSource(void);

private:
  LONG_PTR CallEdit(UINT msg, DWORD wp = 0, LONG_PTR lp = 0);

  bool GetNextLine(const CStringW& text, CStringW& line, int& i);
  void TokenizeLine(const CStringW& line, CArray<CStringW>& tokens);
  void RenumberHeadings(const CArray<SourceLexer::Heading>& headings);
  bool IsLineInExtDoc(const CArray<SourceLexer::Heading>& headings, int line);
  void SetSourceStyle(int style, int boldItalic, bool underline, int size);

private:
  LONG_PTR m_editPtr;
  CTime m_fileTime;

  int m_marker;
  CHARRANGE m_markSel;

  bool m_includeExt;

  CString m_fontName;
  int m_fontSize;

  bool m_syntaxHighlight;
  COLORREF m_colourHead;
  COLORREF m_colourMain;
  COLORREF m_colourComment;
  COLORREF m_colourQuote;
  COLORREF m_colourSubst;
  int m_styleHead;
  int m_styleMain;
  int m_styleComment;
  int m_styleQuote;
  int m_styleSubst;
  bool m_underHead;
  bool m_underMain;
  bool m_underComment;
  bool m_underQuote;
  bool m_underSubst;
  int m_sizeHead;
  int m_sizeMain;
  int m_sizeComment;
  int m_sizeQuote;
  int m_sizeSubst;

  bool m_autoIndent;
  bool m_autoNumber;
  bool m_elasticTabStops;

  EditFind m_find;
  SpellCheck m_spell;

  friend class SourceLexer;
};
