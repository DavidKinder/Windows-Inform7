#pragma once

class RichEdit : public CWnd
{
  DECLARE_DYNAMIC(RichEdit)

public:
  RichEdit();
  BOOL Create(DWORD style, CWnd* parent, UINT id);

  DWORD SetEventMask(DWORD eventMask);
  void EmptyUndoBuffer(void);
  long StreamIn(int format, EDITSTREAM& es);

  void GetSel(CHARRANGE& cr) const;
  void SetSel(CHARRANGE& cr);
  void SetSel(long start, long end);

  BOOL SetDefaultCharFormat(CHARFORMAT& cf);
  void SetMargins(int margin);
  void SetRect(LPCRECT rect);

  long GetTextLength(void) const;
  int GetTextRange(int first, int last, CStringW& str) const;
  void SetTextRange(int first, int last, LPCWSTR str) const;
  int CharFromPos(CPoint point) const;

  void GetIRichEditOle(IRichEditOle** reo) const;
  BOOL SetOLECallback(IRichEditOleCallback* callback);

  enum RichVersion
  {
    RichEditNotLoaded,
    RichEdit20,
    RichEdit41
  };

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnDestroy();
  afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
  afx_msg void OnEditCopy();
  afx_msg void OnUpdateEditChange(CCmdUI* pCmdUI);
  afx_msg void OnEditCut();
  afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
  afx_msg void OnEditPaste();
  afx_msg void OnEditDelete();
  afx_msg void OnEditSelectAll();

  bool Setup(void);
  bool RejectKey(MSG* msg);

  CComQIPtr<ITextDocument> m_textDoc;
};

class RichDrawText : public CCmdTarget
{
public:
  RichDrawText();

  void SetText(LPCWSTR text);
  void Range(long cpFirst, long cpLim, ITextRange** ppRange);

  void SizeText(CDC& dc, CRect& rect);
  void DrawText(CDC& dc, const CRect& rect);

  DECLARE_INTERFACE_MAP()

  BEGIN_INTERFACE_PART(TextHost, ITextHost)
    HDC TxGetDC();
    INT TxReleaseDC(HDC hdc);
    BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
    BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);
    BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
    BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);
    void TxInvalidateRect(LPCRECT prc, BOOL fMode);
    void TxViewChange(BOOL fUpdate);
    BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
    BOOL TxShowCaret(BOOL fShow);
    BOOL TxSetCaretPos(INT x, INT y);
    BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
    void TxKillTimer(UINT idTimer);
    void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip,
      HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
    void TxSetCapture(BOOL fCapture);
    void TxSetFocus();
    void TxSetCursor(HCURSOR hcur, BOOL fText);
    BOOL TxScreenToClient(LPPOINT lppt);
    BOOL TxClientToScreen(LPPOINT lppt);
    HRESULT TxActivate(LONG * plOldState);
    HRESULT TxDeactivate(LONG lNewState);
    HRESULT TxGetClientRect(LPRECT prc);
    HRESULT TxGetViewInset(LPRECT prc);
    HRESULT TxGetCharFormat(const CHARFORMATW **ppCF);
    HRESULT TxGetParaFormat(const PARAFORMAT **ppPF);
    COLORREF TxGetSysColor(int nIndex);
    HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle);
    HRESULT TxGetMaxLength(DWORD *plength);
    HRESULT TxGetScrollBars(DWORD *pdwScrollBar);
    HRESULT TxGetPasswordChar(TCHAR *pch);
    HRESULT TxGetAcceleratorPos(LONG *pcp);
    HRESULT TxGetExtent(LPSIZEL lpExtent);
    HRESULT OnTxCharFormatChange(const CHARFORMATW * pcf);
    HRESULT OnTxParaFormatChange(const PARAFORMAT * ppf);
    HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);
    HRESULT TxNotify(DWORD iNotify, void *pv);
    HIMC TxImmGetContext();
    void TxImmReleaseContext(HIMC himc);
    HRESULT TxGetSelectionBarWidth(LONG *lSelBarWidth);
  END_INTERFACE_PART(TextHost)

protected:
  CComQIPtr<ITextDocument> m_textDoc;
  CComQIPtr<ITextServices,&IID_ITextServices> m_textServ;

  CHARFORMATW m_charFormat;
  PARAFORMAT m_paraFormat;
};
