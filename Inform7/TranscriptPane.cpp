#include "stdafx.h"
#include "TranscriptPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TranscriptPane::TranscriptPane()
{
  m_skeinIndex = -1;
  m_endNode = NULL;
}

void TranscriptPane::SetSkeinIndex(int idx)
{
  m_skeinIndex = idx;
}

void TranscriptPane::SetOrigin(int x, int y)
{
  m_origin.x = x;
  m_origin.y = y;
}

void TranscriptPane::Draw(CDC& dc, CPoint origin, Skein::Node* rootNode)
{
  if (!rootNode->IsAnimated(m_skeinIndex))
  {
    origin += m_origin;
    dc.FillSolidRect(origin.x,origin.y,GetWidth(),500,RGB(0,255,0));
  }
}

Skein::Node* TranscriptPane::GetEnd(void)
{
  return m_endNode;
}

void TranscriptPane::SetEnd(Skein::Node* node)
{
  m_endNode = node;
}

int TranscriptPane::GetWidth(void)
{
  return 300;
}
