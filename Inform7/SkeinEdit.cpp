#include "stdafx.h"
#include "SkeinEdit.h"
#include "Messages.h"

IMPLEMENT_DYNAMIC(SkeinEdit, UnicodeEdit)

SkeinEdit::SkeinEdit() : m_node(NULL)
{
}

SkeinEdit::~SkeinEdit()
{
}

BEGIN_MESSAGE_MAP(SkeinEdit, UnicodeEdit)
  ON_WM_KILLFOCUS()
  ON_WM_CHAR()
  ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void SkeinEdit::OnKillFocus(CWnd* pNewWnd)
{
  UnicodeEdit::OnKillFocus(pNewWnd);
  ShowWindow(SW_HIDE);

  if (m_node != NULL)
  {
    CStringW line;
    GetWindowText(line);
    GetParent()->SendMessage(WM_RENAMENODE,(WPARAM)m_node,(LPARAM)(LPCWSTR)line);
  }
  GetParent()->Invalidate();
  m_node = NULL;
}

void SkeinEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  UnicodeEdit::OnChar(nChar,nRepCnt,nFlags);

  if (nChar == '\r')
    EndEdit();
}

void SkeinEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  UnicodeEdit::OnKeyDown(nChar,nRepCnt,nFlags);

  if (nChar == VK_ESCAPE)
  {
    m_node = NULL;
    EndEdit();
  }
}

void SkeinEdit::StartEdit(Skein::Node* node, const CRect& nodeRect)
{
  m_node = node;

  const CStringW& line = node->GetLine();
  int len = line.GetLength();
  SetWindowText(line);

  MoveWindow(nodeRect,FALSE);
  ShowWindow(SW_SHOW);
  SetSel(len,len);
  SetFocus();
}

void SkeinEdit::EndEdit(void)
{
  ShowWindow(SW_HIDE);
}
