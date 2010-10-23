#include "stdafx.h"
#include "Inform.h"
#include "GameText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(GameText, RichEdit)

BEGIN_MESSAGE_MAP(GameText, RichEdit)
  ON_WM_CREATE()
  ON_WM_MOUSEACTIVATE()
  ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
  ON_NOTIFY_REFLECT(EN_MSGFILTER, OnEnMsgFilter)
  ON_NOTIFY_REFLECT(EN_PROTECTED, OnEnProtected)
  ON_NOTIFY_REFLECT(EN_LINK, OnEnLink)
END_MESSAGE_MAP()

GameText::GameText(MainWindow* main)
{
  ASSERT(main != NULL);

  m_allowEdit = false;
  m_inputPos = -1;
  m_history = 0; 
  m_link = 0;

  m_fore = theApp.GetColour(InformApp::ColourText);
  m_back = theApp.GetColour(InformApp::ColourBack);
  m_reversed = false;

  m_main = main;
  m_drawing = NULL;

  Create(WS_CHILD|WS_VISIBLE,main->GetWnd(),0);

  m_background = theApp.GetColour(InformApp::ColourBack);
  SendMessage(EM_SETBKGNDCOLOR,FALSE,m_background);

  // Allow full justification
  SendMessage(EM_SETTYPOGRAPHYOPTIONS,TO_ADVANCEDTYPOGRAPHY,TO_ADVANCEDTYPOGRAPHY);
}

bool GameText::IsKindOf(const CRuntimeClass* rclass) const
{
  return (RichEdit::IsKindOf(rclass) != FALSE);
}

BOOL GameText::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return RichEdit::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

HWND GameText::GetSafeHwnd(void) const
{
  return RichEdit::GetSafeHwnd();
}

void GameText::Layout(const CRect& r)
{
  m_main->DeferMoveWindow(GetSafeHwnd(),r);
}

void GameText::GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r)
{
  w = size * fontSize.cx;
  h = size * fontSize.cy;
}

void GameText::AddText(const CStringW& text, bool fromSkein)
{
  // Don't update the display until we're done
  long freeze = 0;
  m_textDoc->Freeze(&freeze);

  // Get a range at the end of the text
  CComPtr<ITextRange> range;
  m_textDoc->Range(0,0,&range);
  range->MoveStart(tomStory,1,NULL);

  m_allowEdit = true;

  // Add the text
  range->SetText(CComBSTR(text));

  // Set the appropriate font and paragraph for this range
  if (fromSkein)
    range->SetFont(m_skeinFont);
  else if (m_currentFont != NULL)
    range->SetFont(m_currentFont);
  if (m_currentPara != NULL)
    range->SetPara(m_currentPara);

  // If a hyperlink is active, change the added text to be the link
  if (m_link != 0)
  {
    // Get the current selection, and then make the new text the selection
    CComPtr<ITextSelection> select;
    m_textDoc->GetSelection(&select);
    range->Select();

    // Set the link effect field for the newly added text, and store the link
    // number in the style field
    CHARFORMAT2 format;
    ::ZeroMemory(&format,sizeof format);
    format.cbSize = sizeof format;
    format.dwMask = CFM_LINK|CFM_STYLE;
    format.dwEffects = CFE_LINK;
    format.sStyle = m_link;
    SendMessage(EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&format);

    // Put back the old selection
    select->Select();
  }

  m_allowEdit = false;

  // Let the display update
  m_textDoc->Unfreeze(&freeze);
}

void GameText::ClearText(bool styles, bool reverse)
{
  // Don't update the display until we're done
  long freeze = 0;
  m_textDoc->Freeze(&freeze);

  // Reset the current font
  if (styles)
  {
    m_fore = theApp.GetColour(InformApp::ColourText);
    m_back = theApp.GetColour(InformApp::ColourBack);
    SetStyle(false,false,false,false,0);
  }

  // Get the whole range of text
  CComPtr<ITextRange> range;
  m_textDoc->Range(0,0,&range);
  range->MoveEnd(tomStory,1,NULL);

  m_allowEdit = true;

  // Set the current font and paragraph for this range
  if (m_currentFont != NULL)
    range->SetFont(m_currentFont);
  if (m_currentPara != NULL)
    range->SetPara(m_currentPara);

  // Clear the text
  range->SetText(CComBSTR());
  m_allowEdit = false;

  // Let the display update
  m_textDoc->Unfreeze(&freeze);
}

void GameText::SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size)
{
  // Create a new current font
  m_currentFont.Release();
  m_defaultFont->GetDuplicate(&m_currentFont);

  // Set its style appropriately
  if (bold)
    m_currentFont->SetBold(tomTrue);
  if (italic)
    m_currentFont->SetItalic(tomTrue);
  if (fixed)
    m_currentFont->SetName(CComBSTR(theApp.GetFixedFontName()));

  if (size != 0)
  {
    float pointSize = 0.0;
    m_currentFont->GetSize(&pointSize);
    m_currentFont->SetSize(pointSize + size);
  }

  m_reversed = reverse;
  if (m_reversed)
  {
    m_currentFont->SetForeColor(m_back);
    m_currentFont->SetBackColor(m_fore);
  }
  else
  {
    m_currentFont->SetForeColor(m_fore);
    m_currentFont->SetBackColor(m_back);
  }
}

void GameText::SetColours(COLORREF fore, COLORREF back)
{
  m_fore = fore;
  m_back = back;
  if (m_reversed)
  {
    m_currentFont->SetForeColor(m_back);
    m_currentFont->SetBackColor(m_fore);
  }
  else
  {
    m_currentFont->SetForeColor(m_fore);
    m_currentFont->SetBackColor(m_back);
  }
}

void GameText::SetCursor(int x, int y)
{
}

void GameText::MoveToEnd(void)
{
  // Get the end range of text
  CComPtr<ITextRange> range;
  long end = GetTextLength();
  m_textDoc->Range(end,end,&range);
  range->MoveEnd(tomStory,1,NULL);

  // Move the caret
  range->Collapse(tomEnd);
  range->Select();

  // Scroll into view
  SendMessage(EM_SCROLLCARET);
}

void GameText::Draw(CDibSection* image, int val1, int val2, int width, int height)
{
  // Get the edit control's IRichEditOle interface
  CComPtr<IRichEditOle> reo;
  GetIRichEditOle(&reo);

  // Get a client site for the object
  CComPtr<IOleClientSite> site;
  reo->GetClientSite(&site);
  if (site == NULL)
    return;

  // Create a compound file storage object
  CComPtr<IStorage> store;
  if (FAILED(::StgCreateDocfile(NULL,
    STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE|STGM_DELETEONRELEASE,0,&store)))
    return;

  // Prepare a format description for the picture
  FORMATETC format;
  format.cfFormat = CF_BITMAP;
  format.ptd = NULL;
  format.dwAspect = DVASPECT_CONTENT;
  format.lindex = -1;
  format.tymed = TYMED_GDI;

  // Create an IOleObject instance of the picture
  CComPtr<IOleObject> obj;
  m_drawing = image;
  HRESULT hr = ::OleCreateStaticFromData(&m_xDataObject,IID_IOleObject,OLERENDER_FORMAT,
    &format,site,store,(LPVOID*)&obj);
  m_drawing = NULL;
  if (FAILED(hr))
    return;
  ::OleSetContainedObject(obj,TRUE);

  // Insert the picture object
  REOBJECT ro;
  ::ZeroMemory(&ro,sizeof ro);
  ro.cbStruct = sizeof ro;
  ro.cp = GetTextLength();
  obj->GetUserClassID(&ro.clsid);
  ro.poleobj = obj;
  ro.pstg = store;
  ro.polesite = site;
  ro.dvaspect = DVASPECT_CONTENT;
  reo->InsertObject(&ro);
}

COLORREF GameText::GetAlphaColour(void)
{
  return m_background;
}

void GameText::SetLink(int link)
{
  m_link = link;
}

void GameText::SetParagraph(Justify justify)
{
  m_currentPara.Release();
  m_defaultPara->GetDuplicate(&m_currentPara);

  switch (justify)
  {
  case JustifyLeft:
    m_currentPara->SetAlignment(tomAlignLeft);
    break;
  case JustifyRight:
    m_currentPara->SetAlignment(tomAlignRight);
    break;
  case JustifyCentre:
    m_currentPara->SetAlignment(tomAlignCenter);
    break;
  case JustifyFull:
    m_currentPara->SetAlignment(tomAlignJustify);
    break;
  }
}

void GameText::SetBackColour(COLORREF colour)
{
  m_background = colour;
  SendMessage(EM_SETBKGNDCOLOR,FALSE,m_background);
}

BOOL GameText::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style |= ES_MULTILINE|ES_DISABLENOSCROLL|ES_NOHIDESEL|WS_VSCROLL;
  return RichEdit::PreCreateWindow(cs);
}

BOOL GameText::PreTranslateMessage(MSG* pMsg)
{
  if (RejectKey(pMsg))
    return TRUE;

  if ((pMsg->hwnd == GetSafeHwnd()) && (pMsg->message == WM_KEYDOWN))
  {
    // If game input is active ...
    if (m_inputPos >= 0)
    {
      // Find out where the cursor is
      CHARRANGE range;
      GetSel(range);

      // Handle cursor keys
      switch (pMsg->wParam)
      {
      case VK_UP:
        if (GetLineFromHistory(m_history+1))
          m_history++;
        return TRUE;
      case VK_DOWN:
        if (m_history > 1)
        {
          if (GetLineFromHistory(m_history-1))
            m_history--;
        }
        else
        {
          ReplaceInputLine(L"");
          m_history = 0;
        }
        return TRUE;
      case VK_LEFT:
        if (range.cpMin < m_inputPos)
        {
          SetSel(m_inputPos,m_inputPos);
          return TRUE;
        }
        else if (range.cpMin == m_inputPos)
          return TRUE;
        break;
      case VK_RIGHT:
        if (range.cpMax < m_inputPos)
        {
          CaretToEnd();
          return TRUE;
        }
        break;
      case VK_HOME:
        SetSel(m_inputPos,m_inputPos);
        return TRUE;
      case VK_END:
        if (range.cpMax < m_inputPos)
        {
          CaretToEnd();
          return TRUE;
        }
        break;
      }
    }
  }
  return RichEdit::PreTranslateMessage(pMsg);
}

int GameText::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (RichEdit::OnCreate(lpCreateStruct) == -1)
    return -1;
  if (!Setup())
    return -1;
  if (!SetOLECallback(&m_xRichEditOleCallback))
    return -1;

  // Set events to listen for
  SetEventMask(ENM_CHANGE|ENM_PROTECTED|ENM_KEYEVENTS|ENM_LINK);

  // Set margins
  CSize fontSize = theApp.MeasureFont(&(theApp.GetFont()));
  SetMargins(fontSize.cx);

  // Set the default foreground colour
  CHARFORMAT format;
  format.cbSize = sizeof format;
  format.dwMask = CFM_COLOR|CFM_PROTECTED;
  format.dwEffects = CFE_PROTECTED;
  format.crTextColor = m_fore;
  SetDefaultCharFormat(format);

  // Get the default font
  CComPtr<ITextRange> range;
  m_textDoc->Range(0,0,&range);
  CComPtr<ITextFont> font;
  range->GetFont(&font);
  font->GetDuplicate(&m_defaultFont);

  // Create a font for skein input
  font->GetDuplicate(&m_skeinFont);
  m_skeinFont->SetForeColor(theApp.GetColour(InformApp::ColourSkeinInput));
  m_skeinFont->SetBold(tomTrue);

  // Get the default paragraph
  CComPtr<ITextPara> para;
  range->GetPara(&para);
  para->GetDuplicate(&m_defaultPara);

  ClearText(true,false);
  return 0;
}

int GameText::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
  // Check if this window is being activated, and isn't already active
  int result = RichEdit::OnMouseActivate(pDesktopWnd,nHitTest,message);
  if ((result == MA_ACTIVATE) && (GetFocus() != this))
  {
    // Put the caret at the end of the text
    long end = GetTextLength();
    PostMessage(EM_SETSEL,end,end);
  }
  return result;
}

void GameText::OnEnChange()
{
  // Don't allow undo
  EmptyUndoBuffer();
}

void GameText::OnEnMsgFilter(NMHDR *pNMHDR, LRESULT *pResult)
{
  MSGFILTER* filter = reinterpret_cast<MSGFILTER*>(pNMHDR);
  *pResult = 0;

  // Catch keyboard events
  if (filter->msg == WM_KEYDOWN)
  {
    if (!m_main->GameKeyEvent(this,filter->wParam,filter->lParam))
      *pResult = 1;
  }
}

void GameText::OnEnProtected(NMHDR *pNMHDR, LRESULT *pResult)
{
  ENPROTECTED* protect = reinterpret_cast<ENPROTECTED*>(pNMHDR);

  // Assume text should not change
  *pResult = 1;

  switch (protect->msg)
  {
  case WM_COPY:
    // Always allow copying to the clipboard
    *pResult = 0;
    break;
  default:
    if (m_allowEdit)
    {
      // If the allow edit flag is set, the change is from the program
      // and should always be allowed
      *pResult = 0;
    }
    else if (m_inputPos >= 0)
    {
      if (protect->chrg.cpMin > m_inputPos)
      {
        // Allow a change only if the range of the edit is after the
        // input line's start position
        *pResult = 0;
      }
      else if (protect->chrg.cpMin == m_inputPos)
      {
        // Allow a change if not a backspace
        if ((protect->msg != WM_KEYDOWN) || (protect->wParam != VK_BACK))
          *pResult = 0;
      }
    }
    break;
  }
}

void GameText::OnEnLink(NMHDR *pNMHDR, LRESULT *pResult)
{
  ENLINK* link = reinterpret_cast<ENLINK*>(pNMHDR);
  *pResult = 0;

  if (link->msg == WM_LBUTTONUP)
  {
    // Get a range in the selected hyperlink
    CPoint p = GetCurrentMessage()->pt;
    ScreenToClient(&p);
    int cp = CharFromPos(p);
    if ((cp < link->chrg.cpMin) || (cp > link->chrg.cpMax))
      cp = link->chrg.cpMin;
    if (cp > link->chrg.cpMin)
      cp--;
    CComPtr<ITextRange> range;
    m_textDoc->Range(cp,cp,&range);

    // Get the link number from the style field of the font
    CComPtr<ITextFont> font;
    range->GetFont(&font);
    long linkValue = 0;
    font->GetStyle(&linkValue);
    if ((linkValue != 0) && (linkValue != tomUndefined))
      m_main->GameLinkEvent(this,linkValue);
  }
}

void GameText::AllowLineInput(int initial)
{
  // Get the end range of text
  CComPtr<ITextRange> range;
  m_inputPos = GetTextLength() - initial;
  if (m_inputPos < 0)
    m_inputPos = 0;
  m_textDoc->Range(m_inputPos,m_inputPos,&range);
  range->MoveEnd(tomStory,1,NULL);

  // Set the current font and paragraph for the range
  m_allowEdit = true;
  if (m_currentFont != NULL)
    range->SetFont(m_currentFont);
  if (m_currentPara != NULL)
    range->SetPara(m_currentPara);
  m_allowEdit = false;

  // Move the caret
  range->Collapse(tomEnd);
  range->Select();

  // Move the focus to the input line and scroll into view
  SetFocus();
  SendMessage(EM_SCROLLCARET);

  // Reset the command history counter
  m_history = 0;
}

CStringW GameText::StopLineInput(bool discard)
{
  // Get the input range of text
  CComPtr<ITextRange> range;
  m_textDoc->Range(m_inputPos,m_inputPos,&range);
  range->MoveEnd(tomStory,1,NULL);
  m_inputPos = -1;

  // Get the input text
  CComBSTR input;
  range->GetText(&input);

  CStringW line(input.m_str);
  int pos = line.Find(L'\r');
  if (pos != -1)
    line = line.Left(pos);

  if (discard)
  {
    m_allowEdit = true;
    range->Delete(tomCharacter,0,NULL);
    m_allowEdit = false;
  }

  return line;
}

void GameText::AllowCharInput(void)
{
  // Get the end range of text
  CComPtr<ITextRange> range;
  m_textDoc->Range(0,0,&range);
  range->MoveStart(tomStory,1,NULL);

  // Set the current font and paragraph for the range
  m_allowEdit = true;
  if (m_currentFont != NULL)
    range->SetFont(m_currentFont);
  if (m_currentPara != NULL)
    range->SetPara(m_currentPara);
  m_allowEdit = false;

  // Move the end and scroll into view
  range->Select();
  SetFocus();
  SendMessage(EM_SCROLLCARET);
}

bool GameText::GetLineFromHistory(int history)
{
  CStringW line;
  if (m_main->GetLineFromHistory(line,history))
  {
    ReplaceInputLine(line);
    return true;
  }
  return false;
}

void GameText::ReplaceInputLine(LPCWSTR line)
{
  long len = GetTextLength();

  // Replace the current input line with the line from the skein
  CComPtr<ITextRange> range;
  m_textDoc->Range(m_inputPos,len,&range);
  range->SetText(CComBSTR(line));

  CaretToEnd();
}

void GameText::CaretToEnd(void)
{
  long len = GetTextLength();
  CComPtr<ITextRange> range;
  m_textDoc->Range(len,len,&range);
  range->Select();
}

// IRichEditOleCallback implementation

BEGIN_INTERFACE_MAP(GameText, RichEdit)
  INTERFACE_PART(GameText, IID_IUnknown, RichEditOleCallback)
  INTERFACE_PART(GameText, IID_IDataObject, DataObject)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) GameText::XRichEditOleCallback::AddRef()
{
  return 1;
}

STDMETHODIMP_(ULONG) GameText::XRichEditOleCallback::Release()
{
  return 1;
}

STDMETHODIMP GameText::XRichEditOleCallback::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE(GameText, RichEditOleCallback)
  return (HRESULT)pThis->InternalQueryInterface(&iid,ppvObj);
}

STDMETHODIMP GameText::XRichEditOleCallback::GetNewStorage(LPSTORAGE*)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::GetInPlaceContext(LPOLEINPLACEFRAME*, LPOLEINPLACEUIWINDOW*, LPOLEINPLACEFRAMEINFO)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::ShowContainerUI(BOOL)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::QueryInsertObject(LPCLSID clsid, LPSTORAGE, LONG)
{
  // Allow pictures to be inserted
  RPC_STATUS status;
  if (::UuidCompare(clsid,(UUID*)&CLSID_Picture_Dib,&status) == 0)
    return S_OK;
  return S_FALSE;
}

STDMETHODIMP GameText::XRichEditOleCallback::DeleteObject(LPOLEOBJECT)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::QueryAcceptData(LPDATAOBJECT data, CLIPFORMAT*, DWORD, BOOL really, HGLOBAL)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::ContextSensitiveHelp(BOOL)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::GetClipboardData(CHARRANGE*, DWORD, LPDATAOBJECT*)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::GetDragDropEffect(BOOL, DWORD, LPDWORD effect)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XRichEditOleCallback::GetContextMenu(WORD seltype, LPOLEOBJECT, CHARRANGE*, HMENU* menu)
{
  return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) GameText::XDataObject::AddRef()
{
  return 1;
}

STDMETHODIMP_(ULONG) GameText::XDataObject::Release()
{
  return 1;
}

STDMETHODIMP GameText::XDataObject::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE(GameText, DataObject)
  return (HRESULT)pThis->InternalQueryInterface(&iid,ppvObj);
}

STDMETHODIMP GameText::XDataObject::GetData(FORMATETC*, STGMEDIUM* medium)
{
  METHOD_PROLOGUE(GameText, DataObject)

  if (pThis->m_drawing == NULL)
    return E_FAIL;

  // Pass the bitmap handle and this object as the release IUnknown,
  // so that the bitmap handle is not discarded
  medium->hBitmap = pThis->m_drawing->GetSafeHandle();
  medium->tymed = TYMED_GDI;
  medium->pUnkForRelease = this;
  return S_OK;
}

STDMETHODIMP GameText::XDataObject::GetDataHere(FORMATETC*, STGMEDIUM*)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XDataObject::QueryGetData(FORMATETC*)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XDataObject::GetCanonicalFormatEtc(FORMATETC*, FORMATETC*)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XDataObject::SetData(FORMATETC*, STGMEDIUM*, BOOL)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XDataObject::EnumFormatEtc(DWORD, IEnumFORMATETC**)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XDataObject::DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XDataObject::DUnadvise(DWORD)
{
  return E_NOTIMPL;
}

STDMETHODIMP GameText::XDataObject::EnumDAdvise(IEnumSTATDATA**)
{
  return E_NOTIMPL;
}
