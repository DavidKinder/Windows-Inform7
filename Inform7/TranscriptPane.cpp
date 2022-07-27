#include "stdafx.h"
#include "TranscriptPane.h"
#include "SkeinWindow.h"
#include "Inform.h"
#include "DpiFunctions.h"
#include "TextFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TranscriptPane::TranscriptPane() : m_nodeHeight(0), m_mouseOverButton(-1)
{
}

TranscriptPane::~TranscriptPane()
{
  ClearNodes();
}

void TranscriptPane::SetFontsBitmaps(CWnd* wnd, CDibSection** bitmaps)
{
  CFont* font = theApp.GetFont(wnd,InformApp::FontDisplay);
  m_fontSize = theApp.MeasureFont(wnd,font);
  m_bitmaps = bitmaps;

  // Use a slightly smaller node height to take account of the node image
  // containing a little white space at the top and bottom edges.
  m_nodeHeight = (int)(m_bitmaps[SkeinWindow::BackActive]->GetSize().cy * 0.8);

  // Update the font for any existing windowless rich edit controls
  for (auto& nl : m_nodes)
    nl.draw->FontChanged(DPI::getWindowDPI(wnd));
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

static COLORREF AlphaBlend(COLORREF col1, COLORREF col2, double alpha)
{
  double r = (GetRValue(col1)*alpha)+(GetRValue(col2)*(1.0-alpha));
  double g = (GetGValue(col1)*alpha)+(GetGValue(col2)*(1.0-alpha));
  double b = (GetBValue(col1)*alpha)+(GetBValue(col2)*(1.0-alpha));
  return RGB((BYTE)r,(BYTE)g,(BYTE)b);
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
      nl.AddText(line,true,false,theApp.GetColour(InformApp::ColourTranscriptBack));
    }

    const TranscriptDiff& diffs = nl.node->GetTranscriptDiff();
    if (diffs.HasDiff())
    {
      COLORREF back = theApp.GetColour(InformApp::ColourTranscriptBack);
      COLORREF del = AlphaBlend(theApp.GetColour(InformApp::ColourTranscriptDelete),back,0.45);
      COLORREF ins = AlphaBlend(theApp.GetColour(InformApp::ColourTranscriptInsert),back,0.45);

      // Show the differences between the expected text and the actual transcript
      for (auto& diff : diffs.GetDifferences())
      {
        switch (diff.formOfEdit)
        {
        case TranscriptDiff::DELETE_EDIT:
          nl.AddText(diffs.SubString(diff),false,true,del);
          break;
        case TranscriptDiff::PRESERVE_EDIT:
        case TranscriptDiff::PRESERVE_ACTUAL_EDIT:
          nl.AddText(diffs.SubString(diff),false,false,back);
          break;
        case TranscriptDiff::INSERT_EDIT:
          nl.AddText(diffs.SubString(diff),false,false,ins);
          break;
        }
      }
    }
    else
    {
      // No differences, so show the expected text
      nl.AddText(diffs.GetIdeal(),false,false,theApp.GetColour(InformApp::ColourTranscriptBack));
    }

    // Work out the height of the text for this node
    CRect r(0,0,GetWidth(),0);
    r.DeflateRect(border.cx,0);
    if (m_bitmaps[SkeinWindow::BlessButton]->GetSize().cx > border.cx)
      r.right -= (m_bitmaps[SkeinWindow::BlessButton]->GetSize().cx - border.cx);
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

void TranscriptPane::Draw(CDC& dc, CPoint origin, CDibSection& bitmap)
{
  origin += m_origin;
  dc.FillSolidRect(origin.x,origin.y,GetWidth(),GetHeight(),theApp.GetColour(InformApp::ColourTranscriptBack));

  CSize border = GetBorder();
  CPen linePen(PS_DASH,1,theApp.GetColour(InformApp::ColourTranscriptLine));
  for (int i = 0; i < m_nodes.size(); i++)
  {
    auto& nl = m_nodes[i];

    // Draw the text in the transcript
    CRect r(origin,CSize(GetWidth(),nl.height));
    r.DeflateRect(border);
    if (m_bitmaps[SkeinWindow::BlessButton]->GetSize().cx > border.cx)
      r.right -= (m_bitmaps[SkeinWindow::BlessButton]->GetSize().cx - border.cx);
    dc.SetBkMode(TRANSPARENT);
    nl.draw->DrawText(dc,r);

    // For all but the last entry, add a dashed line as a separator
    int y = origin.y + nl.height;
    if (nl.node != m_nodes.back().node)
    {
      CPen* oldPen = dc.SelectObject(&linePen);
      dc.MoveTo(origin.x,y);
      dc.LineTo(origin.x+GetWidth(),y);
      dc.SelectObject(oldPen);
    }

    // Draw the button to bless or curse the transcript
    CDibSection* btnImage = NULL;
    if (i == m_mouseOverButton)
      btnImage = m_bitmaps[nl.node->GetDiffers() ? SkeinWindow::BlessButtonOver : SkeinWindow::CurseButtonOver];
    else
      btnImage = m_bitmaps[nl.node->GetDiffers() ? SkeinWindow::BlessButton : SkeinWindow::CurseButton];
    CSize btnSize = btnImage->GetSize();
    nl.buttonRect.left = origin.x + GetWidth() - btnSize.cx;
    nl.buttonRect.top = y - btnSize.cy;
    nl.buttonRect.right = nl.buttonRect.left + btnSize.cx;
    nl.buttonRect.bottom = nl.buttonRect.top + btnSize.cy;
    bitmap.AlphaBlend(btnImage,nl.buttonRect.TopLeft().x,nl.buttonRect.TopLeft().y);

    origin.y += nl.height;
  }
}

bool TranscriptPane::MouseMove(CPoint point)
{
  int previousButton = m_mouseOverButton;
  m_mouseOverButton = -1;
  for (int i = 0; i < m_nodes.size(); i++)
  {
    if (m_nodes[i].buttonRect.PtInRect(point))
      m_mouseOverButton = i;
  }
  return (m_mouseOverButton != previousButton);
}

bool TranscriptPane::LButtonUp(CPoint point, Skein* skein)
{
  for (const auto& nl : m_nodes)
  {
    if (nl.buttonRect.PtInRect(point))
    {
      if (nl.node->GetDiffers())
        skein->Bless(nl.node,false);
      else
        skein->SetExpectedText(nl.node,L"");
      return true;
    }
  }
  return false;
}

void TranscriptPane::SetEndNode(Skein::Node* node, CWnd* wnd)
{
  ClearNodes();

  // Find all nodes in the transcript
  while (node != NULL)
  {
    auto it = m_nodes.emplace(m_nodes.begin(),node);

    // Set up a windowless rich edit control for each element
    it->draw = new RichDrawText(InformApp::FontDisplay);
    it->draw->FontChanged(DPI::getWindowDPI(wnd));

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

void TranscriptPane::ValidateNodes(Skein* skein, CWnd* wnd)
{
  Skein::Node* validNode = NULL;

  // Find the last valid node, if any
  for (int i = 0; i < m_nodes.size(); i++)
  {
    if (skein->IsValidNode(m_nodes[i].node))
    {
      if (i > 0)
        validNode = m_nodes[i].node;
    }
  }

  // If there is a valid node still present use it as the new end, otherwise clear everything
  SetEndNode(validNode ? skein->GetThreadEnd(validNode) : NULL,wnd);
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

void TranscriptPane::Shown(bool& anyTick, bool& anyCross)
{
  for (auto& nl : m_nodes)
  {
    if (nl.node->GetDiffers())
      anyTick = true;
    else
      anyCross = true;
  }
}

int TranscriptPane::GetWidth(void)
{
  return m_fontSize.cx*68;
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

void TranscriptPane::NodeLayout::AddText(LPCWSTR text, bool bold, bool strike, COLORREF back)
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
  font->SetStrikeThrough(strike ? tomTrue : tomFalse);
  font->SetBackColor(back);
  font->SetForeColor(theApp.GetColour(InformApp::ColourText));
}
