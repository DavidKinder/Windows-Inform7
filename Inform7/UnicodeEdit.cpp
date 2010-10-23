#include "stdafx.h"
#include "UnicodeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(UnicodeEdit, CEdit)

UnicodeEdit::UnicodeEdit()
{
  m_isUnicode = FALSE;
}

BOOL UnicodeEdit::Create(DWORD style, CWnd* parent, UINT id)
{
  CREATESTRUCT cs;
  cs.dwExStyle = 0;
  cs.lpszClass = "EDIT";
  cs.lpszName = NULL;
  cs.style = style|WS_CHILD;
  cs.x = 0;
  cs.y = 0;
  cs.cx = 0;
  cs.cy = 0;
  cs.hwndParent = parent->GetSafeHwnd();
  cs.hMenu = (HMENU)(UINT_PTR)id;
  cs.hInstance = AfxGetInstanceHandle();
  cs.lpCreateParams = NULL;

  if (!PreCreateWindow(cs))
  {
    PostNcDestroy();
    return FALSE;
  }

  HWND wnd = ::CreateWindowEx(cs.dwExStyle,cs.lpszClass,cs.lpszName,cs.style,
    cs.x,cs.y,cs.cx,cs.cy,cs.hwndParent,cs.hMenu,cs.hInstance,cs.lpCreateParams);
  if (wnd == NULL)
    return FALSE;

  return SubclassWindow(wnd);
}

BOOL UnicodeEdit::SubclassDlgItem(UINT id, CWnd* parent)
{
  // Check whether the current window procedure is Unicode or ANSI
  m_isUnicode = ::IsWindowUnicode(parent->GetDlgItem(id)->GetSafeHwnd());
  return CEdit::SubclassDlgItem(id,parent);
}

BOOL UnicodeEdit::SubclassWindow(HWND wnd)
{
  // Check whether the current window procedure is Unicode or ANSI
  m_isUnicode = ::IsWindowUnicode(wnd);
  return CEdit::SubclassWindow(wnd);
}

void UnicodeEdit::SetWindowText(LPCWSTR string)
{
  // Was the original window procedure Unicode?
  if (m_isUnicode)
  {
    // Call the original window procedure directly
    ::CallWindowProcW(m_pfnSuper,m_hWnd,WM_SETTEXT,0,(LPARAM)string);
  }
  else
  {
    // Call the ANSI MFC method
    CString stringA(string);
    CEdit::SetWindowText(stringA);
  }
}

void UnicodeEdit::GetWindowText(CStringW& string) const
{
  if (m_isUnicode)
  {
    int len = (int)::CallWindowProcW(m_pfnSuper,m_hWnd,WM_GETTEXTLENGTH,0,0);
    LPWSTR buf = string.GetBufferSetLength(len);
    ::CallWindowProcW(m_pfnSuper,m_hWnd,WM_GETTEXT,len+1,(LPARAM)buf);
    string.ReleaseBuffer();
  }
  else
  {
    CString stringA;
    CEdit::GetWindowText(stringA);
    string = stringA;
  }
}

void AFXAPI DDX_TextW(CDataExchange* dx, int idc, CStringW& value)
{
  HWND wndCtrl = dx->PrepareEditCtrl(idc);
  if (::IsWindowUnicode(wndCtrl))
  {
    if (dx->m_bSaveAndValidate)
    {
      int len = ::GetWindowTextLengthW(wndCtrl);
      ::GetWindowTextW(wndCtrl,value.GetBufferSetLength(len),len+1);
      value.ReleaseBuffer();
    }
    else
      ::SetWindowTextW(wndCtrl,value);
  }
  else
  {
    CString valueA(value);
    DDX_Text(dx,idc,valueA);
    value = valueA;
  }
}

void AFXAPI DDX_Control(CDataExchange* dx, int idc, UnicodeEdit& control)
{
  if ((control.GetSafeHwnd() == 0) && (control.GetControlUnknown() == NULL))
  {
    dx->PrepareCtrl(idc);
    HWND wnd;
    dx->m_pDlgWnd->GetDlgItem(idc,&wnd);
    if ((wnd != 0) && !control.SubclassWindow(wnd))
    {
      ASSERT(FALSE);
      AfxThrowNotSupportedException();
    }
  }
}
