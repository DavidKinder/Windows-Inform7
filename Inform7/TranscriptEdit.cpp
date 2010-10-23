#include "stdafx.h"
#include "TranscriptEdit.h"
#include "Messages.h"

IMPLEMENT_DYNAMIC(TranscriptEdit, RichEdit)

TranscriptEdit::TranscriptEdit() : m_node(NULL)
{
}

BEGIN_MESSAGE_MAP(TranscriptEdit, RichEdit)
  ON_WM_CREATE()
  ON_WM_KILLFOCUS()
  ON_WM_KEYDOWN()
END_MESSAGE_MAP()

int TranscriptEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (RichEdit::OnCreate(lpCreateStruct) == -1)
    return -1;
  if (!Setup())
    return -1;
  return 0;
}

void TranscriptEdit::OnKillFocus(CWnd* pNewWnd)
{
  RichEdit::OnKillFocus(pNewWnd);
  ShowWindow(SW_HIDE);

  if (m_node != NULL)
  {
    CStringW text;
    GetTextRange(0,-1,text);
    GetParent()->SendMessage(WM_SETEXPECTED,(WPARAM)m_node,(LPARAM)(LPCWSTR)text);
  }
  GetParent()->Invalidate();
  m_node = NULL;
}

void TranscriptEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  switch (nChar)
  {
  case VK_ESCAPE:
    RichEdit::OnKeyDown(nChar,nRepCnt,nFlags);
    m_node = NULL;
    EndEdit();
    break;
  case VK_RETURN:
    if ((::GetKeyState(VK_CONTROL) & 0x8000) == 0)
      EndEdit();
    else
      RichEdit::OnKeyDown(nChar,nRepCnt,nFlags);
    break;
  default:
    RichEdit::OnKeyDown(nChar,nRepCnt,nFlags);
    break;
  }
}

void TranscriptEdit::StartEdit(Skein::Node* node, const CRect& nodeRect, CPoint point)
{
  m_node = node;
  const CStringW& text = node->GetExpectedText();
  SetTextRange(0,-1,text);

  MoveWindow(nodeRect,FALSE);
  ShowWindow(SW_SHOW);

  ScreenToClient(&point);
  int pos = CharFromPos(point);
  SetSel(pos,pos);

  SetFocus();
}

void TranscriptEdit::EndEdit(void)
{
  ShowWindow(SW_HIDE);
}
