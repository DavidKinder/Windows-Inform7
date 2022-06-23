#include "stdafx.h"
#include "TranscriptPane.h"
#include "Inform.h"
#include "DpiFunctions.h"
#include "TextFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TranscriptPane::TranscriptPane() : m_nodeHeight(0)
{
}

TranscriptPane::~TranscriptPane()
{
  ClearNodes();
}

void TranscriptPane::SetFontsBitmaps(CWnd* wnd, int nodeHeight)
{
  CFont* font = theApp.GetFont(wnd,InformApp::FontDisplay);
  m_fontSize = theApp.MeasureFont(wnd,font);

  // Use a slightly smaller node height to take account of the node image
  // containing a little white space at the top and bottom edges.
  m_nodeHeight = (int)(nodeHeight * 0.8);
}

void TranscriptPane::SetOrigin(int x, int y)
{
  m_origin.x = x;
  m_origin.y = y - (m_nodeHeight/2);
}

CPoint TranscriptPane::GetOrigin(void)
{
  return m_origin;
}

void TranscriptPane::Layout(CDC& dc)
{
  for (auto& nl : m_nodes)
  {
    nl.draw->SetText(nl.node->GetTranscriptText());

    CRect r(0,0,GetWidth(),0);
    r.DeflateRect(m_fontSize.cx*4/3,0);
    nl.draw->SizeText(dc,r);
    nl.height = r.Height() + (m_nodeHeight/2);
  }
}

void TranscriptPane::Draw(CDC& dc, CPoint origin, Skein::Node* rootNode, int skeinIndex)
{
  if (!rootNode->IsAnimated(skeinIndex))
  {
    origin += m_origin;
    dc.FillSolidRect(origin.x,origin.y,GetWidth(),GetHeight(),theApp.GetColour(InformApp::ColourTranscriptBack));

    CPen dashPen(PS_DASH,1,theApp.GetColour(InformApp::ColourTranscriptLine));
    for (auto& nl : m_nodes)
    {
      CRect r(origin,CSize(GetWidth(),nl.height));
      r.DeflateRect(m_fontSize.cx*4/3,m_nodeHeight/4);
      nl.draw->DrawText(dc,r);

      if (nl.node != m_nodes.back().node)
      {
        CPen* oldPen = dc.SelectObject(&dashPen);
        int y = origin.y + nl.height;
        dc.MoveTo(origin.x,y);
        dc.LineTo(origin.x+GetWidth(),y);
        dc.SelectObject(oldPen);

        origin.y += nl.height;
      }
    }
  }
}

void TranscriptPane::SetEndNode(Skein::Node* node, CWnd* wnd)
{
  ClearNodes();
  while (node != NULL)
  {
    auto it = m_nodes.emplace(m_nodes.begin(),node);
    it->draw = new RichDrawText();
    it->draw->FontChanged(DPI::getWindowDPI(wnd));//XXXXDK dpi or prefs change too

    node = node->GetParent();
  }
}

bool TranscriptPane::ContainsNode(Skein::Node* node)
{
  for (const auto& nl : m_nodes)
  {
    if (nl.node == node)
      return true;
  }
  return false;
}

Skein::Node* TranscriptPane::ContainsChildNode(Skein::Node* node)
{
  for (int i = 0; i < node->GetNumChildren(); i++)
  {
    Skein::Node* childNode = node->GetChild(i);
    if (ContainsNode(childNode))
      return childNode;
  }
  return NULL;
}

void TranscriptPane::GetNodes(std::vector<Skein::Node*>& nodes)
{
  for (const auto& nl : m_nodes)
    nodes.push_back(nl.node);
}

bool TranscriptPane::AreNodesValid(Skein* skein)
{
  for (const auto& nl : m_nodes)
  {
    if (!skein->IsValidNode(nl.node))
      return false;
  }
  return true;
}

void TranscriptPane::ClearNodes(void)
{
  for (auto& nl : m_nodes)
    delete nl.draw;
  m_nodes.clear();
}

bool TranscriptPane::IsActive(void)
{
  return m_nodes.size() > 0;
}

int TranscriptPane::GetWidth(void)
{
  return m_fontSize.cx*66;
}

int TranscriptPane::GetHeight(void)
{
  int height = 0;
  for (auto& nl : m_nodes)
    height += nl.height;
  return height;
}

int TranscriptPane::GetRowHeight(int row)
{
  ASSERT((row >= 0) && (row < m_nodes.size()));
  return m_nodes[row].height;
}

void TranscriptPane::SaveTranscript(const char* path)
{
  FILE* transcript = fopen(path,"wt");
  if (transcript == NULL)
    return;

  for (const auto& nl : m_nodes)
  {
    if (nl.node == m_nodes[0].node)
    {
      fprintf(transcript,"%s",
        (LPCTSTR)TextFormat::UnicodeToUTF8(nl.node->GetTranscriptText()));
    }
    else
    {
      fprintf(transcript,"%s\n%s",
        (LPCTSTR)TextFormat::UnicodeToUTF8(nl.node->GetLine()),
        (LPCTSTR)TextFormat::UnicodeToUTF8(nl.node->GetTranscriptText()));
    }
  }
  fclose(transcript);
}

TranscriptPane::NodeLayout::NodeLayout(Skein::Node* n) : node(n), draw(NULL), height(0)
{
}
