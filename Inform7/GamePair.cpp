#include "stdafx.h"
#include "Inform.h"
#include "GamePair.h"
#include "InterpreterCommands.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(GamePair, CObject)

GamePair::GamePair(MainWindow* main, int child1, int child2, int method, int size)
{
  ASSERT(main != NULL);

  m_main = main;
  m_child1 = child1;
  m_child2 = child2;
  m_method = method;
  m_size = size;
  m_key = child2;
}

int GamePair::GetChild1(void)
{
  return m_child1;
}

int GamePair::GetChild2(void)
{
  return m_child2;
}

void GamePair::ReplaceChild(int oldChild, int newChild)
{
  if (m_child1 == oldChild)
    m_child1 = newChild;
  if (m_child2 == oldChild)
    m_child2 = newChild;
}

void GamePair::RemoveChildren(void)
{
  m_child1 = -1;
  m_child2 = -1;
}

void GamePair::RemoveKey(int key)
{
  if (m_key == key)
    m_key = -1;
}

void GamePair::SetArrangement(int method, int size, int key, bool swap)
{
  m_method = method;
  m_size = size;
  m_key = key;

  if (swap)
    std::swap(m_child1,m_child2);
}

bool GamePair::IsKindOf(const CRuntimeClass* rclass) const
{
  return (CObject::IsKindOf(rclass) != FALSE);
}

BOOL GamePair::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return FALSE;
}

HWND GamePair::GetSafeHwnd(void) const
{
  return 0;
}

void GamePair::Layout(const CRect& r)
{
  CRect r1 = r;
  CRect r2 = r;

  switch (m_method & Method_DivMask)
  {
  case Method_Prop:
    switch (m_method & Method_DirMask)
    {
    case Method_Above:
      r2.bottom = r2.top + (int)(r.Height() * m_size * 0.01);
      r1.top = r2.bottom;
      break;
    case Method_Below:
      r2.top = r2.bottom - (int)(r.Height() * m_size * 0.01);
      r1.bottom = r2.top;
      break;
    case Method_Left:
      r2.right = r2.left + (int)(r.Width() * m_size * 0.01);
      r1.left = r2.right;
      break;
    case Method_Right:
      r2.left = r2.right - (int)(r.Width() * m_size * 0.01);
      r1.right = r2.left;
      break;
    }
    break;
  case Method_Fixed:
    {
      int w = 0;
      int h = 0;
      if (m_key != -1)
        m_main->GetNeededSize(m_key,m_size,w,h,r);

      switch (m_method & Method_DirMask)
      {
      case Method_Above:
        r2.bottom = r2.top + h;
        r1.top = r2.bottom;
        break;
      case Method_Below:
        r2.top = r2.bottom - h;
        r1.bottom = r2.top;
        break;
      case Method_Left:
        r2.right = r2.left + w;
        r1.left = r2.right;
        break;
      case Method_Right:
        r2.left = r2.right - w;
        r1.right = r2.left;
        break;
      }
    }
    break;
  }

  m_main->Layout(m_child1,r1);
  m_main->Layout(m_child2,r2);
}

void GamePair::GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r)
{
}

void GamePair::AddText(const CStringW& text, bool fromSkein)
{
}

void GamePair::ClearText(bool styles, bool reverse)
{
}

void GamePair::SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size)
{
}

void GamePair::SetColours(COLORREF fore, COLORREF back)
{
}

void GamePair::SetCursor(int x, int y)
{
}

void GamePair::MoveToEnd(void)
{
}

void GamePair::Draw(CDibSection* image, int val1, int val2, int width, int height)
{
}

COLORREF GamePair::GetAlphaColour(void)
{
  return -1;
}

void GamePair::SetLink(int link)
{
}

void GamePair::SetParagraph(Justify justify)
{
}

void GamePair::SetBackColour(COLORREF colour)
{
}
