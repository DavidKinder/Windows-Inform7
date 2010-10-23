// Base class implementation for most tabs

#include "stdafx.h"
#include "TabBase.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabBase, CWnd)

BEGIN_MESSAGE_MAP(TabBase, CWnd)
  ON_WM_PAINT()
  ON_COMMAND(ID_NAVIGATE_BACK, OnBackward)
  ON_COMMAND(ID_NAVIGATE_FORE, OnForward)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

void TabBase::Create(CWnd* parent)
{
  // Create this window
  CWnd::Create(NULL,NULL,WS_CHILD,CRect(0,0,0,0),parent,0);
  EnableToolTips();

  // Create the font for buttons
  m_font.CreatePointFont(theApp.GetDialogFontSize(),theApp.GetFontName());

  // Create the navigation buttons
  m_navigate[0].Create("<",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_NAVIGATE_BACK);
  m_navigate[0].SetFont(&m_font);
  m_navigate[1].Create(">",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_NAVIGATE_FORE);
  m_navigate[1].SetFont(&m_font);
}

void TabBase::SizeTab(CRect& client, CSize& fontSize, int& heading, int& h)
{
  // Make space for the buttons
  heading = (int)GetParentFrame()->SendMessage(WM_PANEHEADING);
  client.top += heading;

  // Resize the navigation buttons
  fontSize = theApp.MeasureFont(m_navigate[0].GetFont());
  h = fontSize.cy/5;
  int nw = theApp.MeasureText(m_navigate).cx+(fontSize.cx*2);
  m_navigate[0].MoveWindow(fontSize.cx/3,h,nw,heading-(2*h),TRUE);
  m_navigate[1].MoveWindow(nw+(fontSize.cx/3),h,nw,heading-(2*h),TRUE);
}

CString TabBase::GetToolTip(UINT_PTR id)
{
  if ((id == ID_NAVIGATE_BACK) || (id == ID_NAVIGATE_FORE))
  {
    const char* name = Panel::GetPanel(this)->TabNavigateName(id == ID_NAVIGATE_FORE);
    if (name != NULL)
    {
      CString displayName(name);
      displayName.MakeLower();

      CString text;
      text.Format("Go %s to the %s tab",
        (id == ID_NAVIGATE_FORE) ? "forward" : "back",(LPCSTR)displayName);
      return text;
    }
  }
  return "";
}

void TabBase::MakeInactive(void)
{
  ShowWindow(SW_HIDE);
}

bool TabBase::IsEnabled(void)
{
  return true;
}

COLORREF TabBase::GetTabColour(void)
{
  return ::GetSysColor(COLOR_BTNFACE);
}

void TabBase::OpenProject(const char* path, bool primary)
{
}

bool TabBase::SaveProject(const char* path, bool primary)
{
  return true;
}

void TabBase::CompileProject(CompileStage stage, int code)
{
}

bool TabBase::IsProjectEdited(void)
{
  return false;
}

void TabBase::Progress(const char* msg)
{
}

void TabBase::LoadSettings(CRegKey& key)
{
}

void TabBase::SaveSettings(CRegKey& key)
{
}

void TabBase::OnPaint()
{
  CPaintDC dc(this);

  CRect client;
  GetClientRect(client);

  // Only paint the area containing the buttons
  int heading = (int)GetParentFrame()->SendMessage(WM_PANEHEADING);
  client.bottom = client.top + heading;
  dc.FillSolidRect(client,::GetSysColor(COLOR_BTNFACE));
}

BOOL TabBase::OnToolTipText(UINT, NMHDR* hdr, LRESULT* result)
{
  TOOLTIPTEXTA* ttta = (TOOLTIPTEXTA*)hdr;
  TOOLTIPTEXTW* tttw = (TOOLTIPTEXTW*)hdr;

  UINT_PTR id = hdr->idFrom;
  if (hdr->code == TTN_NEEDTEXTA && (ttta->uFlags & TTF_IDISHWND) ||
      hdr->code == TTN_NEEDTEXTW && (tttw->uFlags & TTF_IDISHWND))
  {
    id = ((UINT)(WORD)::GetDlgCtrlID((HWND)id));
  }

  CString tipText = GetToolTip(id);
  if (hdr->code == TTN_NEEDTEXTA)
    lstrcpyn(ttta->szText,tipText,sizeof ttta->szText / sizeof ttta->szText[0]);
  else
    _mbstowcsz(tttw->szText,tipText,sizeof tttw->szText / sizeof tttw->szText[0]);

  *result = 0;
  ::SetWindowPos(hdr->hwndFrom,HWND_TOP,0,0,0,0,
    SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);
  return TRUE;
}

LRESULT TabBase::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
  if (GetSafeHwnd() != 0)
  {
    Panel* panel = Panel::GetPanel(this);
    m_navigate[0].EnableWindow(panel->CanTabNavigate(false));
    m_navigate[1].EnableWindow(panel->CanTabNavigate(true));
  }
  return 0;
}

void TabBase::OnBackward()
{
  Panel::GetPanel(this)->TabNavigate(false);
}

void TabBase::OnForward()
{
  Panel::GetPanel(this)->TabNavigate(true);
}
