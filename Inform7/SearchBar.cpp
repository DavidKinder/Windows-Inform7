#include "stdafx.h"
#include "SearchBar.h"
#include "Inform.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(SearchBar, CDialogBar)

BEGIN_MESSAGE_MAP(SearchBar, CDialogBar)
  ON_WM_ERASEBKGND()
  ON_WM_MOVE()
END_MESSAGE_MAP()

SearchBar::SearchBar()
  : m_source(WM_SEARCHSOURCE,L"Source"), m_docs(WM_SEARCHDOC,L"Documentation")
{
}

void SearchBar::SearchSource(void)
{
  m_source.SetFocus();
}

void SearchBar::SearchDocs(void)
{
  m_docs.SetFocus();
}

BOOL SearchBar::Create(CWnd* parent, UINT style, UINT id)
{
  BOOL created = DialogBar_Create(parent,MAKEINTRESOURCE(IDD_SEARCH),style,id);
  if (created)
  {
    m_source.Init(IDC_SEARCH_SOURCE,this);
    m_docs.Init(IDC_SEARCH_DOCS,this);
  }
  return created;
}

int SearchBar::OnToolHitTest(CPoint point, TOOLINFO* ti) const
{
  const CWnd* wnd = ChildWindowFromPoint(point);
  if ((wnd == &m_source) || (wnd == &m_docs))
  {
    if (ti != NULL)
    {
      ti->hwnd = GetSafeHwnd();
      ti->uId = (UINT_PTR)wnd->GetSafeHwnd();
      ti->uFlags |= TTF_IDISHWND;
      ti->lpszText = LPSTR_TEXTCALLBACK;
    }
    return wnd->GetDlgCtrlID();
  }
  return -1;
}

BOOL SearchBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  CWnd* focusWnd = GetFocus();
  if (focusWnd == &m_source)
  {
    if (m_source.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  }
  if (focusWnd == &m_docs)
  {
    if (m_docs.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  }
  return CDialogBar::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

BOOL SearchBar::OnEraseBkgnd(CDC* pDC)
{
  CWnd* parent = GetParent();
  CPoint pt(0,0);
  MapWindowPoints(parent,&pt,1);

  pt = pDC->OffsetWindowOrg(pt.x,pt.y);
  LRESULT result = parent->SendMessage(WM_ERASEBKGND,(WPARAM)pDC->GetSafeHdc());
  pDC->SetWindowOrg(pt.x,pt.y);

  return (BOOL)result;
}

void SearchBar::OnMove(int, int)
{
  Invalidate();
}

// Copied from afximpl.h in the MFC sources
extern const TCHAR _afxWndControlBar[];
INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

// Copied from MFC sources to enable a call to our CreateDlgIndirect()
BOOL SearchBar::DialogBar_Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName, UINT nStyle, UINT nID)
{
  m_dwStyle = (nStyle & CBRS_ALL);
  CREATESTRUCT cs;
  memset(&cs, 0, sizeof(cs));
  cs.lpszClass = _afxWndControlBar;
  cs.style = (DWORD)nStyle | WS_CHILD;
  cs.hMenu = (HMENU)(UINT_PTR)nID;
  cs.hInstance = AfxGetInstanceHandle();
  cs.hwndParent = pParentWnd->GetSafeHwnd();
  if (!PreCreateWindow(cs))
    return FALSE;

  m_lpszTemplateName = lpszTemplateName;
  BOOL bSuccess = CreateDlg(lpszTemplateName, pParentWnd);
  m_lpszTemplateName = NULL;

  if (!bSuccess)
    return FALSE;

  SetDlgCtrlID(nID);
  CRect rect;
  GetWindowRect(&rect);
  m_sizeDefault = rect.Size();

  ModifyStyle(0, WS_CLIPSIBLINGS);

  if (!ExecuteDlgInit(lpszTemplateName))
    return FALSE;

  SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
  return TRUE;
}

// Copied from MFC sources to enable a call to our CreateDlgIndirect()
BOOL SearchBar::CreateDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
  LPCDLGTEMPLATE lpDialogTemplate = NULL;
  HGLOBAL hDialogTemplate = NULL;
  HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
  HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
  hDialogTemplate = LoadResource(hInst, hResource);
  if (hDialogTemplate != NULL)
    lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

  BOOL bSuccess = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);

  UnlockResource(hDialogTemplate);
  FreeResource(hDialogTemplate);
  return bSuccess;
}

BOOL SearchBar::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd, HINSTANCE hInst)
{
  if(!hInst)
    hInst = AfxGetResourceHandle();

  HGLOBAL hTemplate = NULL;
  HWND hWnd = NULL;

  TRY
  {
    CDialogTemplate dlgTemp(lpDialogTemplate);
    dlgTemp.SetFont(theApp.GetFontName(),theApp.GetDialogFontSize()/10);
    hTemplate = dlgTemp.Detach();
    lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);

    m_nModalResult = -1;
    m_nFlags |= WF_CONTINUEMODAL;

    AfxHookWindowCreate(this);
    hWnd = ::CreateDialogIndirect(hInst, lpDialogTemplate,
      pParentWnd->GetSafeHwnd(), AfxDlgProc);
  }
  CATCH_ALL(e)
  {
    e->Delete();
    m_nModalResult = -1;
  }
  END_CATCH_ALL

  if (!AfxUnhookWindowCreate())
    PostNcDestroy();

  if (hWnd != NULL && !(m_nFlags & WF_CONTINUEMODAL))
  {
    ::DestroyWindow(hWnd);
    hWnd = NULL;
  }

  if (hTemplate != NULL)
  {
    GlobalUnlock(hTemplate);
    GlobalFree(hTemplate);
  }

  if (hWnd == NULL)
    return FALSE;
  return TRUE;
}
