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
  // Shift the origin so that the transcript is aligned with the top edge of a node
  m_origin.x = x;
  m_origin.y = y - (m_nodeHeight/2);
}

CPoint TranscriptPane::GetOrigin(void)
{
  return m_origin;
}

void TranscriptPane::Layout(CDC& dc)
{
  CSize border = GetBorder();
  for (auto& nl : m_nodes)
  {
    nl.draw->SetText(L"");

    // Add the input line for every node except the root node
    if (nl.node->GetParent())
    {
      CStringW line = nl.node->GetLine();
      line.AppendChar(L'\n');
      nl.AddText(line,true);
    }

    nl.AddText(nl.node->GetTranscriptText(),false);

    // Work out the height of the text for this node
    CRect r(0,0,GetWidth(),0);
    r.DeflateRect(border.cx,0);
    nl.draw->SizeText(dc,r);
    nl.height = r.Height() + (border.cy*2);
  }
}

void TranscriptPane::DrawArrows(CDC& dc, CPoint origin, int skeinIndex)
{
  CPen linePen(PS_DOT,1,theApp.GetColour(InformApp::ColourSkeinLine));
  for (auto& nl : m_nodes)
  {
    // Draw a background
    int x = nl.node->GetX(skeinIndex);
    int y = nl.node->GetY(skeinIndex);
    dc.FillSolidRect(origin.x+x,origin.y+y,m_origin.x-x,3,theApp.GetColour(InformApp::ColourBack));

    // Draw a dotted line
    CPen* oldPen = dc.SelectObject(&linePen);
    dc.MoveTo(origin.x+x,origin.y+y);
    dc.LineTo(origin.x+m_origin.x,origin.y+y);

    // Draw the arrow head
    for (int i = 0; i < 4; i++)
      dc.FillSolidRect(origin.x+m_origin.x-2-(i*2),origin.y+y-i,2,(i*2)+1,theApp.GetColour(InformApp::ColourSkeinLine));
  }
}

void TranscriptPane::Draw(CDC& dc, CPoint origin)
{
  origin += m_origin;
  dc.FillSolidRect(origin.x,origin.y,GetWidth(),GetHeight(),theApp.GetColour(InformApp::ColourTranscriptBack));

  CSize border = GetBorder();
  CPen linePen(PS_DASH,1,theApp.GetColour(InformApp::ColourTranscriptLine));
  for (auto& nl : m_nodes)
  {
    // Draw the text in the transcript
    CRect r(origin,CSize(GetWidth(),nl.height));
    r.DeflateRect(border);
    dc.SetBkMode(TRANSPARENT);
    nl.draw->DrawText(dc,r);

    // For all but the last entry, add a dashed line as a separator
    if (nl.node != m_nodes.back().node)
    {
      CPen* oldPen = dc.SelectObject(&linePen);
      int y = origin.y + nl.height;
      dc.MoveTo(origin.x,y);
      dc.LineTo(origin.x+GetWidth(),y);
      dc.SelectObject(oldPen);

      origin.y += nl.height;
    }
  }
}

void TranscriptPane::SetEndNode(Skein::Node* node, CWnd* wnd)
{
  ClearNodes();

  // Find all nodes in the transcript
  while (node != NULL)
  {
    auto it = m_nodes.emplace(m_nodes.begin(),node);

    // Set up a windowless rich edit control for each element
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
  // Delete windowless rich edit controls before clearing the array
  for (auto& nl : m_nodes)
    delete nl.draw;
  m_nodes.clear();
}

CSize TranscriptPane::GetBorder(void)
{
  return CSize(m_fontSize.cx*4/3,m_nodeHeight/6);
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

void TranscriptPane::NodeLayout::AddText(LPCWSTR text, bool bold)
{
  // Get a range at the end of the windowless rich edit control
  CComPtr<ITextRange> range;
  draw->Range(0,0,&range);
  range->MoveStart(tomStory,1,NULL);

  // Set the new text in the range at the end
  range->SetText(CComBSTR(text));

  // Adjust the font for the range
  CComPtr<ITextFont> font;
  range->GetFont(&font);
  font->SetBold(bold ? tomTrue : tomFalse);
}
