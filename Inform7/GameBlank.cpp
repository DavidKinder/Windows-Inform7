#include "stdafx.h"
#include "Inform.h"
#include "GameBlank.h"

#include "DarkMode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(GameBlank, CWnd)

BEGIN_MESSAGE_MAP(GameBlank, CWnd)
  ON_WM_PAINT()
END_MESSAGE_MAP()

GameBlank::GameBlank(MainWindow* main)
{
  ASSERT(main != NULL);

  m_main = main;
  Create(NULL,"",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),main->GetWnd(),0);
}

bool GameBlank::IsKindOf(const CRuntimeClass* rclass) const
{
  return (CWnd::IsKindOf(rclass) != FALSE);
}

BOOL GameBlank::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

HWND GameBlank::GetSafeHwnd(void) const
{
  return CWnd::GetSafeHwnd();
}

void GameBlank::Layout(const CRect& r)
{
  m_main->DeferMoveWindow(GetSafeHwnd(),r);
}

void GameBlank::FontChanged(void)
{
}

void GameBlank::SetDarkMode(DarkMode* dark)
{
}

void GameBlank::GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r)
{
  w = 0;
  h = 0;
}

void GameBlank::AddText(const CStringW& text)
{
}

void GameBlank::ClearText(bool styles, bool reverse, COLORREF fore, COLORREF back)
{
}

void GameBlank::SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size)
{
}

void GameBlank::SetColours(COLORREF fore, COLORREF back)
{
}

void GameBlank::SetCursor(int x, int y)
{
}

void GameBlank::MoveToEnd(void)
{
}

void GameBlank::Draw(CDibSection* image, int val1, int val2, int width, int height)
{
}

COLORREF GameBlank::GetAlphaColour(void)
{
  return -1;
}

void GameBlank::SetLink(int link)
{
}

void GameBlank::SetParagraph(Justify justify)
{
}

void GameBlank::SetBackColour(COLORREF colour)
{
}

void GameBlank::OnPaint()
{
  CPaintDC dc(this);

  CRect client;
  GetClientRect(client);

  DarkMode* dark = DarkMode::GetActive(this);
  dc.FillSolidRect(client,
    dark ? dark->GetColour(DarkMode::Back) : theApp.GetColour(InformApp::ColourBack));
}
