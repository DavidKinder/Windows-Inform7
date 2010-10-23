#include "stdafx.h"
#include "Inform.h"
#include "Messages.h"
#include "RichEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(RichEdit, CWnd)

BEGIN_MESSAGE_MAP(RichEdit, CWnd)
  ON_WM_DESTROY()
  ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
  ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
  ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditChange)
  ON_COMMAND(ID_EDIT_CUT, OnEditCut)
  ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
  ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
  ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditChange)
  ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
END_MESSAGE_MAP()

static RichEdit::RichVersion m_richVersion = RichEdit::RichEditNotLoaded;

RichEdit::RichEdit()
{
  if (m_richVersion != RichEditNotLoaded)
    return;

  // Load the usual RichEdit 2/3 DLL
  ::LoadLibrary("riched20.dll");
  m_richVersion = RichEdit20;

  // Attempt to load RichEdit 4.1
  if (::LoadLibrary("msftedit.dll") != 0)
    m_richVersion = RichEdit41;
}

void RichEdit::OnDestroy()
{
  m_textDoc.Release();
  CWnd::OnDestroy();
}

void RichEdit::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
  CHARRANGE range;
  GetSel(range);
  pCmdUI->Enable(range.cpMin != range.cpMax);
}

void RichEdit::OnEditCopy()
{
  SendMessage(WM_COPY);
}

void RichEdit::OnUpdateEditChange(CCmdUI* pCmdUI)
{
  if (GetStyle() & ES_READONLY)
    pCmdUI->Enable(FALSE);
  else
  {
    CHARRANGE range;
    GetSel(range);
    pCmdUI->Enable(range.cpMin != range.cpMax);
  }
}

void RichEdit::OnEditCut()
{
  SendMessage(WM_CUT);
}

void RichEdit::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(SendMessage(EM_CANPASTE,CF_UNICODETEXT) != 0);
}

void RichEdit::OnEditPaste()
{
  SendMessage(EM_PASTESPECIAL,CF_UNICODETEXT);
}

void RichEdit::OnEditDelete()
{
  CComPtr<ITextSelection> select;
  m_textDoc->GetSelection(&select);
  select->SetText(CComBSTR(""));
}

void RichEdit::OnEditSelectAll()
{
  SetSel(0,-1);
}

BOOL RichEdit::Create(DWORD style, CWnd* parent, UINT id)
{
  CRect zeroRect(0,0,0,0);

  if (m_richVersion == RichEdit41)
    return CWnd::Create("RICHEDIT50W",NULL,style,zeroRect,parent,id);
  return CWnd::Create("RichEdit20A",NULL,style,zeroRect,parent,id);
}

DWORD RichEdit::SetEventMask(DWORD eventMask)
{
  return (DWORD)SendMessage(EM_SETEVENTMASK,0,eventMask);
}

void RichEdit::EmptyUndoBuffer(void)
{
  SendMessage(EM_EMPTYUNDOBUFFER);
}

long RichEdit::StreamIn(int format, EDITSTREAM& es)
{
  return (long)SendMessage(EM_STREAMIN,format,(LPARAM)&es);
}

void RichEdit::GetSel(CHARRANGE& cr) const
{
  ::SendMessage(GetSafeHwnd(),EM_EXGETSEL,0,(LPARAM)&cr);
}

void RichEdit::SetSel(CHARRANGE& cr)
{
  SendMessage(EM_EXSETSEL,0,(LPARAM)&cr);
}

void RichEdit::SetSel(long start, long end)
{
  CHARRANGE cr;
  cr.cpMin = start;
  cr.cpMax = end;
  SendMessage(EM_EXSETSEL,0,(LPARAM)&cr);
}

BOOL RichEdit::SetDefaultCharFormat(CHARFORMAT& cf)
{
  return (BOOL)SendMessage(EM_SETCHARFORMAT,0,(LPARAM)&cf);
}

void RichEdit::SetMargins(int margin)
{
  SendMessage(EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,margin|(margin<<16));
}

void RichEdit::SetRect(LPCRECT rect)
{
  SendMessage(EM_SETRECT,0,(LPARAM)rect);
}

long RichEdit::GetTextLength(void) const
{
  GETTEXTLENGTHEX textLenEx;
  textLenEx.flags = GTL_NUMCHARS;
  textLenEx.codepage = CP_ACP;
  return (long)::SendMessage(GetSafeHwnd(),EM_GETTEXTLENGTHEX,(WPARAM)&textLenEx,0);
}

int RichEdit::GetTextRange(int first, int last, CStringW& str) const
{
  if (last == -1)
    last = GetTextLength();

  CComPtr<ITextRange> range;
  m_textDoc->Range(first,last,&range);

  CComBSTR text;
  range->GetText(&text);
  str = text.m_str;
  return str.GetLength();
}

void RichEdit::SetTextRange(int first, int last, LPCWSTR str) const
{
  if (last == -1)
    last = GetTextLength();

  CComPtr<ITextRange> range;
  m_textDoc->Range(first,last,&range);

  range->SetText(CComBSTR(str));
}

int RichEdit::CharFromPos(CPoint point) const
{
  POINTL ptl = { point.x, point.y };
  return (int)::SendMessage(GetSafeHwnd(),EM_CHARFROMPOS,0,(LPARAM)&ptl);
}

void RichEdit::GetIRichEditOle(IRichEditOle** reo) const
{
  *reo = NULL;
  ::SendMessage(GetSafeHwnd(),EM_GETOLEINTERFACE,0,(LPARAM)reo);
}

BOOL RichEdit::SetOLECallback(IRichEditOleCallback* callback)
{
  return (BOOL)SendMessage(EM_SETOLECALLBACK,0,(LPARAM)callback);
}

bool RichEdit::Setup(void)
{
  // Get the ITextDocument interface
  CComPtr<IRichEditOle> reo;
  GetIRichEditOle(&reo);
  m_textDoc = reo;
  if (!m_textDoc)
    return false;

  // Setting the undo limit to zero causes crashes in the rich edit
  // code, which seems to be a Microsoft bug
  SendMessage(EM_SETUNDOLIMIT,1);

  // Effectively remove the limit on the amount of text that can be entered
  SendMessage(EM_EXLIMITTEXT,0,1024*1024*16);

  // Set font and colour
  CHARFORMAT format;
  ::ZeroMemory(&format,sizeof format);
  format.cbSize = sizeof format;
  format.dwMask = CFM_FACE|CFM_SIZE|CFM_EFFECTS;
  format.yHeight = 20*theApp.GetFontPointSize();
  strcpy(format.szFaceName,theApp.GetFontName());
  SetDefaultCharFormat(format);
  SendMessage(EM_SETBKGNDCOLOR,FALSE,theApp.GetColour(InformApp::ColourBack));

  return true;
}

bool RichEdit::RejectKey(MSG* msg)
{
  if ((msg->hwnd == GetSafeHwnd()) && (msg->message == WM_KEYDOWN))
  {
    // Reject inappropriate editing hotkeys
    bool alt = ((::GetKeyState(VK_MENU) & 0x8000) != 0);
    bool ctrl = ((::GetKeyState(VK_CONTROL) & 0x8000) != 0);
    bool shift = ((::GetKeyState(VK_SHIFT) & 0x8000) != 0);
    if (ctrl && !alt)
    {
      switch (msg->wParam)
      {
      case 'E': // Center alignment
      case 'J': // Justify alignment
      case 'R': // Right alignment
      case 'L': // Left alignment
      case '1': // Line spacing = 1 line
      case '2': // Line spacing = 2 lines
      case '5': // Line spacing = 1.5 lines
      case VK_OEM_PLUS: // Superscript and subscript
        return true;
      case 'A': // If shifted, all capitals
      case VK_OEM_7: // If shifted, smart quotes
        return shift;
      }
    }
  }
  return false;
}

// The IID in riched20.lib in the Plaform SDK appears to be incorrect.
// This one is from the Microsoft Knowledge Base, article Q270161.
const IID IID_ITextServices = {
  // 8d33f740-cf58-11ce-a89d-00aa006cadc5
  0x8d33f740, 0xcf58, 0x11ce, {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
};

RichDrawText::RichDrawText()
{
  ::ZeroMemory(&m_charFormat,sizeof m_charFormat);
  m_charFormat.cbSize = sizeof m_charFormat;
  m_charFormat.dwMask = CFM_BOLD|CFM_CHARSET|CFM_COLOR|CFM_FACE|CFM_ITALIC|CFM_OFFSET|
    CFM_PROTECTED|CFM_SIZE|CFM_STRIKEOUT|CFM_UNDERLINE;
  m_charFormat.yHeight = 20 * theApp.GetFontPointSize();
  m_charFormat.crTextColor = theApp.GetColour(InformApp::ColourText);
  m_charFormat.bPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
  CStringW fontName(theApp.GetFontName());
  wcscpy(m_charFormat.szFaceName,fontName);

  ::ZeroMemory(&m_paraFormat,sizeof m_paraFormat);
  m_paraFormat.cbSize = sizeof m_paraFormat;
  m_paraFormat.dwMask = PFM_ALIGNMENT|PFM_NUMBERING|PFM_OFFSET|PFM_OFFSETINDENT|
    PFM_RIGHTINDENT|PFM_RTLPARA|PFM_STARTINDENT|PFM_TABSTOPS;
  m_paraFormat.wAlignment = PFA_LEFT;

  CComPtr<IUnknown> unknown;
  HRESULT hr = ::CreateTextServices(NULL,&m_xTextHost,&unknown);
  ASSERT(SUCCEEDED(hr));

  m_textDoc = unknown;
  m_textServ = unknown;
}

void RichDrawText::SetText(LPCWSTR text)
{
  // Get a range for the whole of the text in the control
  CComPtr<ITextRange> range;
  m_textDoc->Range(0,0,&range);
  range->MoveEnd(tomStory,1,NULL);

  range->SetText(CComBSTR(text));
}

void RichDrawText::Range(long cpFirst, long cpLim, ITextRange** ppRange)
{
  m_textDoc->Range(cpFirst,cpLim,ppRange);
}

void RichDrawText::SizeText(CDC& dc, CRect& rect)
{
  LONG w = rect.Width();
  LONG h = rect.Height();

  SIZEL extent = { -1, -1 };
  HRESULT hr = m_textServ->TxGetNaturalSize(DVASPECT_CONTENT,
    dc.GetSafeHdc(),0,NULL,TXTNS_FITTOCONTENT,&extent,&w,&h);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
    h = 0;

  rect.bottom = rect.top+h;
}

void RichDrawText::DrawText(CDC& dc, const CRect& rect)
{
  RECTL rc = { rect.left, rect.top, rect.right, rect.bottom };
  HRESULT hr = m_textServ->TxDraw(DVASPECT_CONTENT,0,NULL,NULL,dc.GetSafeHdc(),0,
    &rc,NULL,NULL,NULL,0,0);
  ASSERT(SUCCEEDED(hr));
}

BEGIN_INTERFACE_MAP(RichDrawText, CCmdTarget)
  INTERFACE_PART(RichDrawText, IID_ITextHost, TextHost)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) RichDrawText::XTextHost::AddRef()
{
  return 1;
}

STDMETHODIMP_(ULONG) RichDrawText::XTextHost::Release()
{
  return 1;
}

STDMETHODIMP RichDrawText::XTextHost::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE(RichDrawText, TextHost)
  return (HRESULT)pThis->InternalQueryInterface(&iid,ppvObj);
}

HDC RichDrawText::XTextHost::TxGetDC()
{
  return 0;
}

INT RichDrawText::XTextHost::TxReleaseDC(HDC hdc)
{
  return 0;
}

BOOL RichDrawText::XTextHost::TxShowScrollBar(INT fnBar, BOOL fShow)
{
  return FALSE;
}

BOOL RichDrawText::XTextHost::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
  return FALSE;
}

BOOL RichDrawText::XTextHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
  return FALSE;
}

BOOL RichDrawText::XTextHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
  return FALSE;
}

void RichDrawText::XTextHost::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
}

void RichDrawText::XTextHost::TxViewChange(BOOL fUpdate)
{
}

BOOL RichDrawText::XTextHost::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
  return FALSE;
}

BOOL RichDrawText::XTextHost::TxShowCaret(BOOL fShow)
{
  return FALSE;
}

BOOL RichDrawText::XTextHost::TxSetCaretPos(INT x, INT y)
{
  return FALSE;
}

BOOL RichDrawText::XTextHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
  return FALSE;
}

void RichDrawText::XTextHost::TxKillTimer(UINT idTimer)
{
}

void RichDrawText::XTextHost::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll,
  LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{
}

void RichDrawText::XTextHost::TxSetCapture(BOOL fCapture)
{
}

void RichDrawText::XTextHost::TxSetFocus()
{
}

void RichDrawText::XTextHost::TxSetCursor(HCURSOR hcur, BOOL fText)
{
}

BOOL RichDrawText::XTextHost::TxScreenToClient(LPPOINT lppt)
{
  return FALSE;
}

BOOL RichDrawText::XTextHost::TxClientToScreen(LPPOINT lppt)
{
  return FALSE;
}

HRESULT RichDrawText::XTextHost::TxActivate(LONG* plOldState)
{
  return E_FAIL;
}

HRESULT RichDrawText::XTextHost::TxDeactivate(LONG lNewState)
{
  return E_FAIL;
}

HRESULT RichDrawText::XTextHost::TxGetClientRect(LPRECT prc)
{
  return E_FAIL;
}

HRESULT RichDrawText::XTextHost::TxGetViewInset(LPRECT prc)
{
  *prc = CRect(0,0,0,0);
  return S_OK;
}

HRESULT RichDrawText::XTextHost::TxGetCharFormat(const CHARFORMATW **ppCF)
{
  METHOD_PROLOGUE(RichDrawText, TextHost)
  *ppCF = &(pThis->m_charFormat);
  return S_OK;
}

HRESULT RichDrawText::XTextHost::TxGetParaFormat(const PARAFORMAT **ppPF)
{
  METHOD_PROLOGUE(RichDrawText, TextHost)
  *ppPF = &(pThis->m_paraFormat);
  return S_OK;
}

COLORREF RichDrawText::XTextHost::TxGetSysColor(int nIndex)
{
  return ::GetSysColor(nIndex);
}

HRESULT RichDrawText::XTextHost::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
  *pstyle = TXTBACK_TRANSPARENT;
  return S_OK;
}

HRESULT RichDrawText::XTextHost::TxGetMaxLength(DWORD *plength)
{
  *plength = 1024*1024*16;
  return S_OK;
}

HRESULT RichDrawText::XTextHost::TxGetScrollBars(DWORD *pdwScrollBar)
{
  *pdwScrollBar = 0;
  return S_OK;
}

HRESULT RichDrawText::XTextHost::TxGetPasswordChar(TCHAR *pch)
{
  return S_FALSE;
}

HRESULT RichDrawText::XTextHost::TxGetAcceleratorPos(LONG *pcp)
{
  *pcp = -1;
  return S_OK;
}

HRESULT RichDrawText::XTextHost::TxGetExtent(LPSIZEL lpExtent)
{
  return E_NOTIMPL;
}

HRESULT RichDrawText::XTextHost::OnTxCharFormatChange(const CHARFORMATW * pcf)
{
  return E_FAIL;
}

HRESULT RichDrawText::XTextHost::OnTxParaFormatChange(const PARAFORMAT * ppf)
{
  return E_FAIL;
}

HRESULT RichDrawText::XTextHost::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
{
  DWORD bits = TXTBIT_MULTILINE|TXTBIT_RICHTEXT|TXTBIT_WORDWRAP;
  *pdwBits = bits & dwMask;
  return S_OK;
}

HRESULT RichDrawText::XTextHost::TxNotify(DWORD iNotify, void *pv)
{
  return S_OK;
}

HIMC RichDrawText::XTextHost::TxImmGetContext()
{
  return 0;
}

void RichDrawText::XTextHost::TxImmReleaseContext(HIMC himc)
{
}

HRESULT RichDrawText::XTextHost::TxGetSelectionBarWidth(LONG *lSelBarWidth)
{
  *lSelBarWidth = 0;
  return S_OK;
}
