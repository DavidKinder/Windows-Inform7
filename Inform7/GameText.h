#pragma once

#include "GameBase.h"
#include "RichEdit.h"

class GameText : public RichEdit, public GameBase
{
  DECLARE_DYNAMIC(GameText)

public:
  GameText(MainWindow* main);

  bool IsKindOf(const CRuntimeClass* rclass) const;
	BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  HWND GetSafeHwnd(void) const;

  void Layout(const CRect& r);
  void GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r);
  void AddText(const CStringW& text, bool fromSkein);
  void ClearText(bool styles, bool reverse);
  void SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size);
  void SetColours(COLORREF fore, COLORREF back);
  void SetCursor(int x, int y);
  void MoveToEnd(void);
  void Draw(CDibSection* image, int val1, int val2, int width, int height);
  COLORREF GetAlphaColour(void);
  void SetLink(int link);
  void SetParagraph(Justify justify);
  void SetBackColour(COLORREF colour);

  void AllowLineInput(int initial);
  CStringW StopLineInput(bool discard);
  void AllowCharInput(void);

protected:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
  afx_msg void OnEnChange();
  afx_msg void OnEnMsgFilter(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnEnProtected(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnEnLink(NMHDR *pNMHDR, LRESULT *pResult);

private:
  bool GetLineFromHistory(int history);
  void ReplaceInputLine(LPCWSTR line);
  void CaretToEnd(void);

  CComPtr<ITextFont> m_defaultFont, m_currentFont, m_skeinFont;
  CComPtr<ITextPara> m_defaultPara, m_currentPara;

  bool m_allowEdit;
  long m_inputPos;
  int m_history;
  int m_link;

  MainWindow* m_main;

  bool m_reversed;
  COLORREF m_fore;
  COLORREF m_back;
  COLORREF m_background;

  CDibSection* m_drawing;

// IRichEditOleCallback implementation
public:
  DECLARE_INTERFACE_MAP()

  BEGIN_INTERFACE_PART(RichEditOleCallback, IRichEditOleCallback)
    STDMETHOD(GetNewStorage) (LPSTORAGE*);
    STDMETHOD(GetInPlaceContext) (LPOLEINPLACEFRAME*, LPOLEINPLACEUIWINDOW*, LPOLEINPLACEFRAMEINFO);
    STDMETHOD(ShowContainerUI) (BOOL);
    STDMETHOD(QueryInsertObject) (LPCLSID, LPSTORAGE, LONG);
    STDMETHOD(DeleteObject) (LPOLEOBJECT);
    STDMETHOD(QueryAcceptData) (LPDATAOBJECT, CLIPFORMAT*, DWORD,BOOL, HGLOBAL);
    STDMETHOD(ContextSensitiveHelp) (BOOL);
    STDMETHOD(GetClipboardData) (CHARRANGE*, DWORD, LPDATAOBJECT*);
    STDMETHOD(GetDragDropEffect) (BOOL, DWORD, LPDWORD);
    STDMETHOD(GetContextMenu) (WORD, LPOLEOBJECT, CHARRANGE*, HMENU*);
  END_INTERFACE_PART(RichEditOleCallback)

  BEGIN_INTERFACE_PART(DataObject, IDataObject)
    STDMETHOD(GetData) (FORMATETC*, STGMEDIUM*);
    STDMETHOD(GetDataHere) (FORMATETC*, STGMEDIUM*);
    STDMETHOD(QueryGetData) (FORMATETC*);
    STDMETHOD(GetCanonicalFormatEtc) (FORMATETC*, FORMATETC*);
    STDMETHOD(SetData) (FORMATETC*, STGMEDIUM*, BOOL);
    STDMETHOD(EnumFormatEtc) (DWORD, IEnumFORMATETC**);
    STDMETHOD(DAdvise) (FORMATETC*, DWORD, IAdviseSink*, DWORD*);
    STDMETHOD(DUnadvise) (DWORD);
    STDMETHOD(EnumDAdvise) (IEnumSTATDATA**);
  END_INTERFACE_PART(DataObject)
};
