#include "stdafx.h"
#include "SkeinEdit.h"
#include "Messages.h"

IMPLEMENT_DYNAMIC(SkeinEdit, UnicodeEdit)

SkeinEdit::SkeinEdit() : m_node(NULL), m_label(false)
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

    if (m_label)
      GetParent()->SendMessage(WM_LABELNODE,(WPARAM)m_node,(LPARAM)(LPCWSTR)line);
    else
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

void SkeinEdit::StartEdit(Skein::Node* node, const CRect& nodeRect, bool label)
{
  m_node = node;
  m_label = label;

  int len = 0;
  if (label)
  {
    const CStringW& line = node->GetLabel();
    len = line.GetLength();
    SetWindowText(line);
  }
  else
  {
    const CStringW& line = node->GetLine();
    len = line.GetLength();
    SetWindowText(line);
  }

  MoveWindow(nodeRect,FALSE);
  ShowWindow(SW_SHOW);
  SetSel(len,len);
  SetFocus();
}

void SkeinEdit::EndEdit(void)
{
  ShowWindow(SW_HIDE);
}

bool SkeinEdit::EditingLabel(Skein::Node* node)
{
  return (m_label && (m_node == node));
}
