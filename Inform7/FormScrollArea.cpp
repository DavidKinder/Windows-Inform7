#include "stdafx.h"
#include "FormScrollArea.h"

#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FormScrollArea, DrawScrollArea)

BEGIN_MESSAGE_MAP(FormScrollArea, DrawScrollArea)
  ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
END_MESSAGE_MAP()

FormScrollArea::FormScrollArea(UINT nIDTemplate)
{
  m_lpszTemplateName = MAKEINTRESOURCE(nIDTemplate);
  EnableActiveAccessibility();
}

LRESULT FormScrollArea::HandleInitDialog(WPARAM, LPARAM)
{
  Default();
  LoadDynamicLayoutResource(m_lpszTemplateName);
  return FALSE;
}

DarkMode::DarkColour FormScrollArea::GetDarkBackground(void)
{
  return DarkMode::Back;
}

void FormScrollArea::OnInitialUpdate()
{
  UpdateData(FALSE);
  DrawScrollArea::OnInitialUpdate();
}

void FormScrollArea::OnDraw(CDC* pDC)
{
  PaintWindowlessControls(pDC);
}

BOOL FormScrollArea::PreTranslateMessage(MSG* pMsg)
{
  if (DrawScrollArea::PreTranslateMessage(pMsg))
    return TRUE;

  CFrameWnd* pFrameWnd = GetParentFrame();
  while (pFrameWnd != NULL)
  {
    if (pFrameWnd->PreTranslateMessage(pMsg))
      return TRUE;
    pFrameWnd = pFrameWnd->GetParentFrame();
  }

  if (::GetWindow(m_hWnd,GW_CHILD) == NULL)
    return FALSE;

  return PreTranslateInput(pMsg);
}

// Copied from MFC sources to enable a call to our CreateDlgIndirect()
BOOL FormScrollArea::Create(DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
  CREATESTRUCT cs;
  memset(&cs,0,sizeof(CREATESTRUCT));
  if (dwRequestedStyle == 0)
    dwRequestedStyle = AFX_WS_DEFAULT_VIEW;
  cs.style = dwRequestedStyle;
  if (!PreCreateWindow(cs))
    return FALSE;

  if (!CreateDlg(m_lpszTemplateName,pParentWnd))
    return FALSE;

  ModifyStyle(WS_BORDER|WS_CAPTION,cs.style & (WS_BORDER|WS_CAPTION));
  ModifyStyleEx(WS_EX_CLIENTEDGE,cs.dwExStyle & WS_EX_CLIENTEDGE);

  SetDlgCtrlID(nID);

  CRect rectTemplate;
  GetWindowRect(rectTemplate);
  SetScrollSizes(MM_TEXT,rectTemplate.Size());

  if (!ExecuteDlgInit(m_lpszTemplateName))
    return FALSE;

  CRect r(rect);
  if (!r.IsRectEmpty())
    SetWindowPos(NULL,r.left,r.top,r.Width(),r.Height(),SWP_NOZORDER|SWP_NOACTIVATE);

  if (dwRequestedStyle & WS_VISIBLE)
    ShowWindow(SW_NORMAL);
  return TRUE;
}

// Copied from MFC sources to enable a call to our CreateDlgIndirect()
BOOL FormScrollArea::CreateDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
  LPCDLGTEMPLATE lpDialogTemplate = NULL;
  HGLOBAL hDialogTemplate = NULL;
  HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName,RT_DIALOG);
  HRSRC hResource = ::FindResource(hInst,lpszTemplateName,RT_DIALOG);
  hDialogTemplate = LoadResource(hInst,hResource);
  if (hDialogTemplate != NULL)
    lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

  BOOL bSuccess = CreateDlgIndirect(lpDialogTemplate,pParentWnd,hInst);
  UnlockResource(hDialogTemplate);
  FreeResource(hDialogTemplate);
  return bSuccess;
}

INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

// Copied from MFC sources to allow the dialog font to be set
BOOL FormScrollArea::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate,
  CWnd* pParentWnd, HINSTANCE hInst)
{
  if (!hInst)
    hInst = AfxGetResourceHandle();

  HGLOBAL hTemplate = NULL;
  HWND hWnd = NULL;

  TRY
  {
    CDialogTemplate dlgTemp(lpDialogTemplate);
    dlgTemp.SetFont(
      theApp.GetFontName(InformApp::FontSystem),theApp.GetFontSize(InformApp::FontSystem));
    hTemplate = dlgTemp.Detach();
    lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);

    m_nModalResult = -1;
    m_nFlags |= WF_CONTINUEMODAL;

    AfxHookWindowCreate(this);
    hWnd = ::CreateDialogIndirect(hInst,lpDialogTemplate,
      pParentWnd->GetSafeHwnd(),AfxDlgProc);
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
