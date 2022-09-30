#include "stdafx.h"
#include "stdafx.h"
#include "Skein.h"
#include "TranscriptPane.h"
#include "Inform.h"
#include "TextFormat.h"
#include "Build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Skein::Skein()
{
  for (int i = 0; i < LAYOUTS; i++)
    m_laidOut[i] = false;

  m_inst.skeinFile = "Skein.skein";
  m_inst.root = new Node(L"- start -",L"",L"",L"",false);

  m_playTo = m_inst.root;
  m_played = m_inst.root;
}

Skein::~Skein()
{
  delete m_inst.root;
  for (std::vector<Instance>::iterator it = m_other.begin(); it != m_other.end(); ++it)
    delete it->root;
}

Skein::Node* Skein::GetRoot(void)
{
  return m_inst.root;
}

Skein::Node* Skein::GetPlayTo(void)
{
  return m_playTo;
}

void Skein::SetPlayTo(Node* node)
{
  if (!(node->IsTestSubItem()))
  {
    m_playTo = node;
    NotifyChange(PlayedChanged);
  }
}

bool Skein::InPlayThread(Node* node)
{
  return InThread(node,m_playTo);
}

bool Skein::InThread(Node* node, Node* endNode)
{
  while (endNode != NULL)
  {
    if (endNode == node)
      return true;
    endNode = endNode->GetParent();
  }
  return false;
}

Skein::Node* Skein::GetPlayed(void)
{
  return m_played;
}

void Skein::Load(const char* path)
{
  if (m_inst.skeinFile.IsEmpty())
  {
    Reset();
    return;
  }

  CString fileName;
  fileName.Format("%s\\%s",path,m_inst.skeinFile);

  // Create an XML document instance
  CComPtr<IXMLDOMDocument> doc;
  if (FAILED(doc.CoCreateInstance(CLSID_DOMDocument)))
  {
    Reset();
    return;
  }

  // Load the skein XML into the document
  VARIANT_BOOL success = 0;
  if (doc->load(CComVariant(fileName),&success) != S_OK)
  {
    CComPtr<IXMLDOMParseError> error;
    doc->get_parseError(&error);

    long line = 0;
    error->get_line(&line);
    CComBSTR text;
    error->get_srcText(&text);
    CComBSTR reason;
    error->get_reason(&reason);

    TRACE("Failed to load skein XML\n line: %d\n text: %S\n reason: %S\n",
      line,text.m_str,reason.m_str);
    Reset();
    return;
  }

  // Get all the item nodes
  CComPtr<IXMLDOMNodeList> items;
  doc->selectNodes(L"/Skein/item",&items);

  // Create a node object for each XML item node
  std::map<CStringW,Node*> nodes;
  CComPtr<IXMLDOMNode> item;
  while (items->nextNode(&item) == S_OK)
  {
    CStringW id = StringFromXML(item,L"@nodeId");
    CStringW command = StringFromXML(item,L"command");
    CStringW label = StringFromXML(item,L"annotation");
    CStringW result = StringFromXML(item,L"result");
    CStringW commentary = StringFromXML(item,L"commentary");
    bool played = BoolFromXML(item,L"played/text()",false);
    bool changed = BoolFromXML(item,L"changed/text()",false);
    bool temp = BoolFromXML(item,L"temporary/text()",true);
    int score = IntFromXML(item,L"temporary/@score");
    item = NULL;

    if (IsTestCommand(command))
    {
      std::vector<CStringW> transcripts, expecteds;
      SeparateByBracketedSequentialNumbers(result,transcripts);
      SeparateByBracketedSequentialNumbers(commentary,expecteds);

      result.Empty();
      commentary.Empty();
      if (transcripts.size() > 0)
      {
        result = transcripts[0];
        if (expecteds.size() > 0)
          commentary = expecteds[0];
      }

      Node* node = new Node(command,label,result,commentary,changed);
      nodes[id] = node;

      if (transcripts.size() > 1)
      {
        for (int i = 1; i < transcripts.size(); i++)
        {
          CStringW expected;
          if (i < expecteds.size())
            expected = expecteds[i];
          Node* testNode = new Node(CommandForTestingEntry(transcripts[i]),L"",
            OutputForTestingEntry(transcripts[i]),OutputForTestingEntry(expected),false);
          testNode->SetTestSubItem();
          node->Add(testNode);
          node = testNode;
        }
      }
    }
    else
    {
      nodes[id] = new Node(command,label,result,commentary,changed);
    }
  }

  // Loop through the XML item nodes again, setting up the parent-child links
  items->reset();
  while (items->nextNode(&item) == S_OK)
  {
    Node* parentNode = nodes[StringFromXML(item,L"@nodeId").m_str];

    // If there are test sub-items, find the last one
    if (parentNode->IsTestCommand())
    {
      while (parentNode->GetNumChildren() == 1)
      {
        Node* childNode = parentNode->GetChild(0);
        if (!childNode->IsTestSubItem())
          break;
        parentNode = childNode;
      }
    }

    CComPtr<IXMLDOMNodeList> children;
    item->selectNodes(L"children/child",&children);

    CComPtr<IXMLDOMNode> child;
    while (children->nextNode(&child) == S_OK)
    {
      Node* childNode = nodes[StringFromXML(child,L"@nodeId").m_str];
      parentNode->Add(childNode);
      child = NULL;
    }
    item = NULL;
  }

  // Get the root node
  CStringW root = StringFromXML(doc,L"/Skein/@rootNode");

  // Discard the current skein and replace with the new
  delete m_inst.root;
  m_inst.root = nodes[root];
  m_playTo = m_inst.root;
  m_played = m_inst.root;

  InvalidateLayout();
  NotifyChange(TreeChanged);
  NotifyEdit(false);
}

bool Skein::Save(const char* path)
{
  for (std::vector<Instance>::iterator it = m_other.begin(); it != m_other.end(); ++it)
    it->Save(path);

  if (m_inst.Save(path))
  {
    NotifyEdit(false);
    return true;
  }
  return false;
}

bool Skein::Instance::Save(const char* path)
{
  if (!edited)
    return true;

  if (skeinFile.IsEmpty())
    return true;
  CString fileName;
  fileName.Format("%s\\%s",path,skeinFile);

  FILE* skeinFile = fopen(fileName,"wt");
  if (skeinFile == NULL)
    return false;

  fprintf(skeinFile,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Skein rootNode=\"%s\">\n"
    "  <generator>Windows Inform " INFORM_VER "</generator>\n",
    root->GetUniqueId());
  root->SaveNodes(skeinFile);
  fprintf(skeinFile,"</Skein>\n");
  fclose(skeinFile);
  edited = false;
  return true;
}

void Skein::Reset()
{
  delete m_inst.root;
  m_inst.root = new Node(L"- start -",L"",L"",L"",false);
  m_playTo = m_inst.root;
  m_played = m_inst.root;

  InvalidateLayout();
  NotifyChange(TreeChanged);
  NotifyEdit(false);
}

void Skein::Import(const char* path)
{
  Node* node = m_inst.root;
  bool added = false;

  CStdioFile recFile;
  if (recFile.Open(path,CFile::modeRead|CFile::typeText))
  {
    CString recLine;
    while (recFile.ReadString(recLine))
    {
      recLine.Trim();
      if (recLine.GetLength() > 0)
      {
        CStringW recLineW = EscapeLine(CStringW(recLine),UsePrintable);
        Node* newNode = node->Find(recLineW);
        if (newNode == NULL)
        {
          newNode = new Node(recLineW,L"",L"",L"",false);
          node->Add(newNode);
          added = true;
        }
        node = newNode;
      }
    }

    if (added)
    {
      InvalidateLayout();
      NotifyChange(TreeChanged);
      NotifyEdit(true);
    }
  }
}

bool Skein::IsActive(void)
{
  return (m_inst.skeinFile.IsEmpty() == FALSE);
}

bool Skein::IsEdited(void)
{
  if (m_inst.edited)
    return true;
  for (std::vector<Instance>::const_iterator it = m_other.begin(); it != m_other.end(); ++it)
  {
    if (it->edited)
      return true;
  }
  return false;
}

void Skein::SetFile(const char* fileName)
{
  m_inst.skeinFile = fileName;
}

bool Skein::ChangeFile(const char* fileName, const char* path)
{
  if (m_inst.skeinFile == fileName)
    return false;

  m_other.push_back(m_inst);
  m_inst = Instance();

  for (std::vector<Instance>::iterator it = m_other.begin(); it != m_other.end(); ++it)
  {
    if (it->skeinFile == fileName)
    {
      m_inst = *it;
      m_other.erase(it);
      m_playTo = m_inst.root;
      m_played = m_inst.root;

      InvalidateLayout();
      NotifyChange(TreeChanged);
      return true;
    }
  }

  m_inst.skeinFile = fileName;
  Load(path);
  return true;
}

void Skein::Reset(bool playTo)
{
  if (playTo)
    m_playTo = m_inst.root;

  m_played = m_inst.root;
  NotifyChange(PlayedChanged);
}

void Skein::InvalidateLayout(void)
{
  for (int i = 0; i < LAYOUTS; i++)
    m_laidOut[i] = false;
}

void Skein::Layout(CDC& dc, int idx, LayoutMode mode, const CSize& spacing, TranscriptPane& transcript)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  if (mode == LayoutRecalculate)
    m_inst.root->ClearWidths(idx);

  if ((mode > LayoutDefault) || (m_laidOut[idx] == false))
  {
    std::vector<std::vector<Node*> > nodesByDepth;
    m_inst.root->GetNodesByDepth(0,nodesByDepth);

    // Visit each level, from the bottom to the top, left to right
    for (size_t row = nodesByDepth.size(); row > 0; --row)
    {
      int x = 0;
      const std::vector<Node*>& rowNodes = nodesByDepth[row-1];
      for (size_t col = 0; col < rowNodes.size(); ++col)
      {
        Node* node = rowNodes[col];
        int numc = node->GetNumChildren();
        Node* transcriptChild = transcript.ContainsChildNode(node);

        // Set the initial position of the node
        int width = node->CalcLineWidth(dc,idx);
        width = max(width,node->GetLabelTextWidth(idx));
        int x_next = x + (width/2);
        node->SetX(idx,x_next);
        node->SetY(idx,(int)(row-1) * spacing.cy);

        if (transcriptChild != NULL)
        {
          // Put all transcript nodes on the same vertical line
          int x_child = transcriptChild->GetX(idx);
          node->SetX(idx,x_child);
          if (x_next > x_child)
          {
            // Move this node and all its children to the right
            node->ShiftX(idx,x_next - x_child);

            // Move all nodes after this node to the right
            for (size_t col2 = col+1; col2 < rowNodes.size(); ++col2)
              rowNodes[col2]->ShiftX(idx,x_next - x_child);
          }
        }
        else if (numc > 0)
        {
          // Centre a parent node between its children
          int x_first_child = node->GetChild(0)->GetX(idx);
          int x_last_child = node->GetChild(numc-1)->GetX(idx);
          int x_centre = (x_first_child + x_last_child)/2;
          node->SetX(idx,x_centre);
          if (x_next > x_centre)
          {
            // Move this node and all its children to the right
            node->ShiftX(idx,x_next - x_centre);

            // Move all nodes after this node to the right
            for (size_t col2 = col+1; col2 < rowNodes.size(); ++col2)
              rowNodes[col2]->ShiftX(idx,x_next - x_centre);
          }
        }

        x = node->GetX(idx) + (width/2) + spacing.cx;
      }
    }

    // Shift the entire tree so that the origin is the left edge
    int x_leftmost = 0;
    for (size_t row = 0; row < nodesByDepth.size(); ++row)
    {
      const std::vector<Node*>& rowNodes = nodesByDepth[row];
      if (rowNodes.size() > 0)
      {
        int x = rowNodes[0]->GetX(idx);
        if (x < x_leftmost)
          x_leftmost = x;
      }
    }
    m_inst.root->ShiftX(idx,-x_leftmost);

    // Add space for the transcript
    if (transcript.IsActive())
    {
      // Layout the transcript
      transcript.Layout(dc);

      // Get all nodes in the transcript
      std::vector<Node*> transcriptNodes;
      transcript.GetNodes(transcriptNodes);

      // Find the right-most extent of the nodes in the the transcript.
      int x_right = INT_MIN;
      for (size_t i = 0; i < transcriptNodes.size(); i++)
      {
        Node* node = transcriptNodes[i];
        int xr = node->GetX(idx) + (node->GetLayoutWidth(idx)/2);
        if (xr > x_right)
          x_right = xr;
      }

      // Set the origin of the transcript
      int transcriptMarginX = (int)(spacing.cx*0.7);
      transcript.SetOrigin(x_right+transcriptMarginX,m_inst.root->GetY(idx));

      // Adjust the layout to make space for the transcript vertically
      for (size_t row = 0; row < transcriptNodes.size(); row++)
      {
        if (row+1 < nodesByDepth.size())
        {
          const std::vector<Node*>& rowNodes = nodesByDepth[row+1];
          if (rowNodes.size() > 0)
          {
            int y_spacing = rowNodes[0]->GetY(idx) - transcriptNodes[row]->GetY(idx);
            int t_height = transcript.GetRowHeight((int)row);

            // Add a bit more space for the last row of the transcript
            if (row == transcriptNodes.size()-1)
              t_height += spacing.cx / 2;

            if (y_spacing < t_height)
            {
              int y_shift = t_height - y_spacing;
              for (size_t col = 0; col < rowNodes.size(); ++col)
                rowNodes[col]->ShiftY(idx,y_shift);
            }
          }
        }
      }

      // Find the left-most extent of the nodes after the transcript
      int x_after_left = m_inst.root->GetLeftmostAfterX(idx,transcriptNodes[0]->GetX(idx));

      // Adjust the layout to make space for the transcript horizontally
      int transcriptWidth = transcript.GetWidth() + (transcriptMarginX*2);
      if (x_after_left < INT_MAX)
        transcriptWidth += (x_right - x_after_left);

      // For each node in the transcript, shift any child nodes to the right of the
      // child node that is also in the transcript, if any.
      for (size_t row = 0; row < transcriptNodes.size()-1; row++)
      {
        bool afterTranscript = false;
        Node* node = transcriptNodes[row];
        for (int i = 0; i < node->GetNumChildren(); i++)
        {
          Node* child = node->GetChild(i);
          if (afterTranscript)
            child->ShiftX(idx,transcriptWidth);
          else if (child == transcriptNodes[row+1])
            afterTranscript = true;
        }
      }
    }
  }
  m_laidOut[idx] = true;
}

CSize Skein::GetTreeExtent(int idx)
{
  std::vector<std::vector<Node*> > nodesByDepth;
  m_inst.root->GetNodesByDepth(0,nodesByDepth);

  int xmin = 0, xmax = 0, height = 0;
  bool valid = false;
  for (size_t row = 0; row < nodesByDepth.size(); ++row)
  {
    const std::vector<Node*>& rowNodes = nodesByDepth[row];
    if (rowNodes.size() > 0)
    {
      Node* node1 = rowNodes[0];
      Node* node2 = rowNodes[rowNodes.size()-1];
      int x1 = node1->GetX(idx) - (node1->GetLayoutWidth(idx)/2);
      int x2 = node2->GetX(idx) + (node2->GetLayoutWidth(idx)/2);
      if (!valid || (x1 < xmin))
        xmin = x1;
      if (!valid || (x2 > xmax))
        xmax = x2;
      valid = true;

      if (node1->GetY(idx) > height)
        height = node1->GetY(idx);
    }
  }
  return CSize(valid ? (xmax - xmin) : 0,height);
}

void Skein::NewLine(const CStringW& line, bool test)
{
  bool nodeAdded = false;
  CStringW nodeLine = EscapeLine(line,UsePrintable);

  // Is there a child node with the same line?
  Node* node = m_played->Find(nodeLine);
  if (node == NULL)
  {
    node = new Node(nodeLine,L"",L"",L"",false);
    m_played->Add(node);
    m_played->SortChildren();
    nodeAdded = true;
  }

  // Make this the new node being played
  if (test)
    node->SetTestSubItem();
  else
    m_playTo = node;
  m_played = node;

  // Notify any listeners
  if (nodeAdded)
  {
    InvalidateLayout();
    NotifyChange(TreeChanged);
    NotifyEdit(true);
  }
  else
    NotifyChange(PlayedChanged);

  NotifyShowNode(node);
}

bool Skein::NextLine(CStringW& line)
{
  // Find the next node to use
  Node* next = m_played->FindAncestor(m_playTo);
  if (next != NULL)
  {
    line = EscapeLine(next->GetLine(),UseCharacters);
    m_played = next;
    NotifyChange(PlayedChanged);
    NotifyShowNode(next);
    return true;
  }
  return false;
}

void Skein::UpdateAfterPlaying(const CStringW& transcript)
{
  Change change = NodeTranscriptChanged;

  // If the transcript is empty, just notify any listeners but do not change anything,
  // as the game has most likely stopped without printing anything else.
  if (!transcript.IsEmpty())
  {
    if (m_played->IsTestCommand())
    {
      std::vector<CStringW> transcripts;
      SeparateByBracketedSequentialNumbers(transcript,transcripts);

      if (transcripts.size() > 0)
        m_played->NewTranscriptText(transcripts[0]);
      if (transcripts.size() > 1)
      {
        // Go through the remaining entries, inserting test child items as we go
        for (int i = 1; i < transcripts.size(); i++)
        {
          Node* parent = m_played;
          NewLine(CommandForTestingEntry(transcripts[i]),true);
          parent->RemoveAllExcept(m_played);
          m_played->NewTranscriptText(OutputForTestingEntry(transcripts[i]));
          change = TreeChanged;
        }

        // The node being played to may have been removed by the above
        if ((change == TreeChanged) && !IsValidNode(m_playTo))
          m_playTo = m_played;
      }
    }
    else
    {
      // Update the status of the last played node
      m_played->NewTranscriptText(transcript);
    }
  }

  NotifyChange(change);
  NotifyShowNode(m_played);
}

static const wchar_t* escapes[] =
{
  L"\r",    L"[enter]",
  L"\n",    L"[enter]",
  L"\t",    L"[tab]",
  L"\b",    L"[back]",
  L"\033",  L"[esc]",
  L"\xFFFE",L"[left]",  // The 0XFFFX constants are defined
  L"\xFFFD",L"[right]", // in InterpreterCommands.h
  L"\xFFFC",L"[up]",
  L"\xFFFB",L"[down]",
};

CStringW Skein::EscapeLine(const CStringW& line, EscapeAction action)
{
  CStringW escLine(line);

  for (int i = 0; i < sizeof escapes/sizeof escapes[0]; i += 2)
  {
    switch (action)
    {
    case UsePrintable:
      escLine.Replace(escapes[i],escapes[i+1]);
      break;
    case UseCharacters:
      escLine.Replace(escapes[i+1],escapes[i]);
      break;
    case RemoveEscapes:
      escLine.Replace(escapes[i+1],L"");
      break;
    }
  }
  return escLine;
}

bool Skein::GetLineFromHistory(CStringW& line, int history)
{
  // Find the node to return the line from
  Node* node = m_playTo;
  while (--history > 0)
  {
    node = node->GetParent();
    if (node == NULL)
      return false;
  }

  // Don't return the root node
  if (node == m_inst.root)
    return false;

  line = EscapeLine(node->GetLine(),RemoveEscapes);
  return true;
}

Skein::Node* Skein::AddNew(Node* node)
{
  Node* newNode = new Node(L"",L"",L"",L"",false);
  node->Add(newNode);

  InvalidateLayout();
  NotifyChange(TreeChanged);
  NotifyEdit(true);

  return newNode;
}

Skein::Node* Skein::AddNewParent(Node* node)
{
  Node* newNode = new Node(L"",L"",L"",L"",false);
  node->GetParent()->Replace(node,newNode);
  newNode->Add(node);

  InvalidateLayout();
  NotifyChange(TreeChanged);
  NotifyEdit(true);

  return newNode;
}

bool Skein::RemoveAll(Node* node, bool notify)
{
  bool removed = false;
  Node* parent = node->GetParent();
  if (parent != NULL)
  {
    bool inPlay = InPlayThread(node);
    removed = parent->Remove(node);

    if (inPlay)
    {
      m_playTo = m_inst.root;
      m_played = m_inst.root;
    }

    InvalidateLayout();
    if (notify)
      NotifyChange(TreeChanged);
  }
  if (removed)
    NotifyEdit(true);
  return removed;
}

bool Skein::RemoveSingle(Node* node)
{
  bool removed = false;
  Node* parent = node->GetParent();
  if (parent != NULL)
  {
    bool inPlay = InPlayThread(node);
    removed = parent->RemoveSingle(node);
    if (removed)
      parent->SortChildren();

    if (inPlay)
    {
      m_playTo = m_inst.root;
      m_played = m_inst.root;
    }

    InvalidateLayout();
    NotifyChange(TreeChanged);
  }
  if (removed)
    NotifyEdit(true);
  return removed;
}

void Skein::SortSiblings(Node* node)
{
  Node* parent = node->GetParent();
  if (parent != NULL)
  {
    if (parent->SortChildren())
    {
      InvalidateLayout();
      NotifyChange(TreeChanged);
    }
  }
}

void Skein::SetLine(Node* node, LPCWSTR line)
{
  if (node->SetLine(line))
    NotifyEdit(true);

  InvalidateLayout();
  NotifyChange(NodeTextChanged);
}

void Skein::SetLabel(Node* node, LPCWSTR label)
{
  if (node->SetLabel(label))
    NotifyEdit(true);

  InvalidateLayout();
  NotifyChange(NodeTextChanged);
}

void Skein::Lock(Node* node)
{
  while (node != NULL)
  {
    if (node->SetLocked(true))
      NotifyEdit(true);
    node = node->GetParent();
  }
  NotifyChange(LockChanged);
}

void Skein::Unlock(Node* node, bool notify)
{
  if (node->SetLocked(false))
    NotifyEdit(true);
  for (int i = 0; i < node->GetNumChildren(); i++)
    Unlock(node->GetChild(i),false);
  if (notify)
    NotifyChange(LockChanged);
}

void Skein::Trim(Node* node, bool running, bool notify)
{
  // Only delete unlocked nodes. If the game is running, only delete
  // if the node is not in the currently played thread as well.
  bool inPlay = InPlayThread(node);
  if ((node->GetParent() != NULL) && !node->GetLocked() && !(running && inPlay))
  {
    if (inPlay)
    {
      m_playTo = m_inst.root;
      m_played = m_inst.root;
    }
    RemoveAll(node,false);
  }
  else
  {
    // Since this may remove child nodes, run through in reverse
    for (int i = node->GetNumChildren()-1; i >= 0; i--)
      Trim(node->GetChild(i),false);
  }

  if (notify)
    NotifyChange(TreeChanged);
}

void Skein::Bless(Node* node, bool all)
{
  while (node != NULL)
  {
    if (node->Bless())
      NotifyEdit(true);
    node = all ? node->GetParent() : NULL;
  }
  NotifyChange(NodeTranscriptChanged);
}

bool Skein::CanBless(Node* node, bool all)
{
  bool canBless = false;
  while (node != NULL)
  {
    canBless |= node->GetDiffers();
    node = all ? node->GetParent() : NULL;
  }
  return canBless;
}

void Skein::SetExpectedText(Node* node, LPCWSTR text)
{
  if (node->SetExpectedText(text))
    NotifyEdit(true);
  NotifyChange(NodeTranscriptChanged);
}

Skein::Node* Skein::GetThreadEnd(Node* node)
{
  while (true)
  {
    if (node->GetNumChildren() != 1)
      return node;
    node = node->GetChild(0);
  }
}

bool Skein::IsValidNode(Node* testNode, Node* node)
{
  if (node == NULL)
    node = m_inst.root;

  if (testNode == node)
    return true;

  for (int i = 0; i < node->GetNumChildren(); i++)
  {
    if (IsValidNode(testNode,node->GetChild(i)))
      return true;
  }
  return false;
}

void Skein::GetThreadEnds(std::vector<Node*>& nodes, Node* node)
{
  if (node == NULL)
    node = m_inst.root;

  int num = node->GetNumChildren();
  if (num > 0)
  {
    for (int i = 0; i < num; i++)
      GetThreadEnds(nodes,node->GetChild(i));
  }
  else
    nodes.push_back(node);
}

Skein::Node* Skein::GetFirstDifferent(Node* node)
{
  if (node == NULL)
    node = m_inst.root;

  // If this node differs, return it
  if (node->GetDiffers())
    return node;

  // Look for differing child nodes
  for (int i = 0; i < node->GetNumChildren(); i++)
  {
    Node* diffNode = GetFirstDifferent(node->GetChild(i));
    if (diffNode != NULL)
      return diffNode;
  }

  return NULL;
}

void Skein::GetAllNodes(CArray<Skein::Node*,Skein::Node*>& nodes, Node* node)
{
  if (node == NULL)
    node = m_inst.root;
  nodes.Add(node);

  for (int i = 0; i < node->GetNumChildren(); i++)
    GetAllNodes(nodes,node->GetChild(i));
}

Skein::Node* Skein::FindNode(const char* id, Node* node)
{
  if (node == NULL)
    node = m_inst.root;
  if (strcmp(node->GetUniqueId(),id) == 0)
    return node;
  for (int i = 0; i < node->GetNumChildren(); i++)
  {
    Node* foundNode = FindNode(id,node->GetChild(i));
    if (foundNode != NULL)
      return foundNode;
  }
  return NULL;
}

void Skein::CopyNode(Node* node, Node* parentNode)
{
  parentNode->Add(node->Clone());
  parentNode->SortChildren();

  InvalidateLayout();
  NotifyChange(TreeChanged);
  NotifyEdit(true);
}

bool Skein::MoveNode(Node* node, Node* parentNode, Skein* fromSkein)
{
  ASSERT(node->GetParent() != NULL);
  if (node->GetParent()->ReleaseChild(node))
  {
    parentNode->Add(node);
    parentNode->SortChildren();

    InvalidateLayout();
    NotifyChange(TreeChanged);
    NotifyEdit(true);

    if (fromSkein != this)
    {
      fromSkein->InvalidateLayout();
      fromSkein->NotifyChange(TreeChanged);
      fromSkein->NotifyEdit(true);
    }
    return true;
  }
  return false;
}

void Skein::AddListener(Listener* listener)
{
  m_listeners.push_back(listener);
}

void Skein::NotifyChange(Change change)
{
  for (std::vector<Listener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
    (*it)->SkeinChanged(change);
}

void Skein::NotifyEdit(bool edited)
{
  if (m_inst.edited != edited)
  {
    m_inst.edited = edited;
    for (std::vector<Listener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
      (*it)->SkeinEdited(edited);
  }
}

void Skein::NotifyShowNode(Node* node)
{
  for (std::vector<Listener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
    (*it)->SkeinShowNode(node,false);
}

bool Skein::IsTestCommand(const CStringW& line)
{
  return (line.Left(5) == L"test ");
}

void Skein::SeparateByBracketedSequentialNumbers(const CStringW& text, std::vector<CStringW>& results)
{
  if (text.IsEmpty())
    return;

  int commandIndexToFind = 1;
  int searchFrom = 0;
  while (searchFrom < text.GetLength())
  {
    CStringW stringToFind;
    stringToFind.Format(L"[%d]",commandIndexToFind);

    int found = text.Find(stringToFind,searchFrom);
    if (found < 0)
      break;
    results.push_back(text.Mid(searchFrom,found-searchFrom));

    // Move beyond the separator to the remainder of the string
    searchFrom = found + stringToFind.GetLength();

    // Look for the next number
    commandIndexToFind++;
  }

  // Add the remainder of the string as the final component of the results
  if (searchFrom < text.GetLength())
    results.push_back(text.Mid(searchFrom));
}

CStringW Skein::CommandForTestingEntry(const CStringW& entry)
{
  CStringW command(entry);
  command.Trim();

  int returnIndex = command.Find(L"\n");
  if (returnIndex >= 0)
    return command.Left(returnIndex);
  return command;
}

CStringW Skein::OutputForTestingEntry(const CStringW& entry)
{
  int returnIndex = entry.Find(L"\n");
  if (returnIndex >= 0)
    return entry.Mid(returnIndex+1);
  return entry;
}

LPCTSTR Skein::ToXML_UTF8(bool value)
{
  return value ? "YES" : "NO";
}

CComBSTR Skein::StringFromXML(IXMLDOMNode* node, LPWSTR query)
{
  CComPtr<IXMLDOMNode> resultNode;
  if (node->selectSingleNode(query,&resultNode) == S_OK)
  {
    DOMNodeType nt;
    if (resultNode->get_nodeType(&nt) == S_OK)
    {
      if (nt == NODE_TEXT)
      {
        CComVariant text;
        if (resultNode->get_nodeValue(&text) == S_OK)
        {
          if (text.vt == VT_BSTR)
            return text.bstrVal;
        }
      }
      else
      {
        CComBSTR text;
        if (resultNode->get_text(&text) == S_OK)
          return text;
      }
    }
  }
  return L"";
}

bool Skein::BoolFromXML(IXMLDOMNode* node, LPWSTR query, bool ifNon)
{
  CComBSTR text = StringFromXML(node,query);
  if (text.Length() > 0)
    return wcscmp(text.m_str,L"YES") == 0;
  return ifNon;
}

int Skein::IntFromXML(IXMLDOMNode* node, LPWSTR query)
{
  CComBSTR text = StringFromXML(node,query);
  if (text.Length() > 0)
    return _wtoi(text.m_str);
  return 0;
}

Skein::Node::Node(const CStringW& line, const CStringW& label, const CStringW& transcript,
  const CStringW& expected, bool changed)
  : m_parent(NULL), m_line(line), m_label(label), 
    m_textTranscript(transcript), m_textExpected(expected),
    m_locked(false), m_changed(changed), m_testSubItem(false)
{
  static unsigned long counter = 0;
  m_id.Format("node-%lu",counter++);
  CompareWithExpected();
}

Skein::Node::~Node()
{
  for (int i = 0; i < m_children.GetSize(); i++)
    delete m_children[i];
}

Skein::Node* Skein::Node::Clone(void)
{
  Node* clone = new Node(m_line,m_label,m_textTranscript,m_textExpected,false);
  clone->m_testSubItem = m_testSubItem;
  for (int i = 0; i < m_children.GetSize(); i++)
    clone->Add(m_children[i]->Clone());
  return clone;
}

Skein::Node* Skein::Node::GetParent(void)
{
  return m_parent;
}

void Skein::Node::SetParent(Node* parent)
{
  m_parent = parent;
}

int Skein::Node::GetNumChildren(void)
{
  return (int)m_children.GetSize();
}

Skein::Node* Skein::Node::GetChild(int index)
{
  return m_children[index];
}

const CStringW& Skein::Node::GetLine(void)
{
  return m_line;
}

bool Skein::Node::SetLine(LPCWSTR line)
{
  bool change = (m_line != line);
  m_line = line;
  for (int i = 0; i < LAYOUTS; i++)
    m_layout[i].ClearWidths();
  return change;
}

const CStringW& Skein::Node::GetLabel(void)
{
  return m_label;
}

bool Skein::Node::SetLabel(LPCWSTR label)
{
  bool change = (m_label != label);
  m_label = label;
  for (int i = 0; i < LAYOUTS; i++)
    m_layout[i].ClearWidths();
  return change;
}

bool Skein::Node::HasLabel(void)
{
  return ((m_parent != NULL) && (m_label.GetLength() > 0));
}

bool Skein::Node::GetChanged(void)
{
  return m_changed;
}

bool Skein::Node::GetDiffers(void)
{
  return m_diff.HasDiff();
}

bool Skein::Node::GetLocked(void)
{
  return m_locked;
}

bool Skein::Node::SetLocked(bool locked)
{
  bool change = (m_locked != locked);
  m_locked = locked;
  return change;
}

bool Skein::Node::IsTestCommand(void)
{
  return Skein::IsTestCommand(m_line);
}

bool Skein::Node::IsTestSubItem(void)
{
  return m_testSubItem;
}

void Skein::Node::SetTestSubItem(void)
{
  m_testSubItem = true;
}

int Skein::Node::GetNumTestSubChildren(void)
{
  int num = 0;
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i]->IsTestSubItem())
      num++;
  }
  return num;
}

void Skein::Node::NewTranscriptText(LPCWSTR text)
{
  CStringW newTranscript(text);
  newTranscript.Replace('\r','\n');

  if (m_textTranscript != newTranscript)
  {
    m_textTranscript = newTranscript;
    CompareWithExpected();
  }
}

bool Skein::Node::Bless(void)
{
  if (m_textExpected != m_textTranscript)
  {
    m_textExpected = m_textTranscript;
    CompareWithExpected();
    return true;
  }
  return false;
}

bool Skein::Node::SetExpectedText(LPCWSTR text)
{
  CStringW newExpected(text);
  newExpected.Replace('\r','\n');

  if (m_textExpected != newExpected)
  {
    m_textExpected = newExpected;
    CompareWithExpected();
    return true;
  }
  return false;
}

int Skein::Node::CalcLineWidth(CDC& dc, int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));
  LayoutInfo& info = m_layout[idx];

  if (info.width < 0)
  {
    SIZE size;
    ::GetTextExtentPoint32W(dc.GetSafeHdc(),m_line,(UINT)wcslen(m_line),&size);
    info.width = size.cx;
    info.lineWidth = size.cx;

    if (wcslen(m_label) > 0)
    {
      ::GetTextExtentPoint32W(dc.GetSafeHdc(),m_label,(UINT)wcslen(m_label),&size);
      info.labelWidth = size.cx;
    }
    else
      info.labelWidth = 0;

    CSize minSize = dc.GetTextExtent("    ",4);
    if (info.width < minSize.cx)
      info.width = minSize.cx;
    if (info.labelWidth < minSize.cx)
      info.labelWidth = minSize.cx;
  }
  return info.width;
}

int Skein::Node::GetLayoutWidth(int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));
  LayoutInfo& info = m_layout[idx];

  return max(info.width,info.labelWidth);
}

int Skein::Node::GetLineTextWidth(int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  return m_layout[idx].lineWidth;
}

int Skein::Node::GetLabelTextWidth(int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  return m_layout[idx].labelWidth;
}

void Skein::Node::ClearWidths(int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  m_layout[idx].ClearWidths();
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->ClearWidths(idx);
}

int Skein::Node::GetLeftmostAfterX(int idx, int x)
{
  int leftmostX = INT_MAX;

  int leftX = GetX(idx) - (GetLayoutWidth(idx)/2);
  if (leftX > x)
    leftmostX = leftX;

  for (int i = 0; i < m_children.GetSize(); i++)
  {
    int childLeftX = m_children[i]->GetLeftmostAfterX(idx,x);
    if (childLeftX < leftmostX)
      leftmostX = childLeftX;
  }
  return leftmostX;
}

void Skein::Node::Add(Node* child)
{
  m_children.Add(child);
  child->SetParent(this);
}

bool Skein::Node::Remove(Node* child)
{
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i] == child)
    {
      m_children.RemoveAt(i);
      delete child;
      return true;
    }
  }
  return false;
}

void Skein::Node::RemoveAll(void)
{
  for (int i = 0; i < m_children.GetSize(); i++)
    delete m_children[i];
  m_children.RemoveAll();
}


void Skein::Node::RemoveAllExcept(Node* keep)
{
  for (int i = 0; i < m_children.GetSize();)
  {
    if (m_children[i] == keep)
      i++;
    else
    {
      Node* child = m_children[i];
      m_children.RemoveAt(i);
      delete child;
    }
  }
}

bool Skein::Node::RemoveSingle(Node* child)
{
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i] == child)
    {
      m_children.RemoveAt(i);
      for (int j = 0; j < child->m_children.GetSize(); j++)
        child->m_children[j]->SetParent(this);
      m_children.InsertAt(i,&(child->m_children));
      child->m_children.RemoveAll();
      delete child;
      return true;
    }
  }
  return false;
}

void Skein::Node::Replace(Node* oldNode, Node* newNode)
{
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i] == oldNode)
    {
      m_children[i] = newNode;
      newNode->SetParent(this);
    }
  }
}

static int CompareByLine(const void* element1, const void* element2)
{
  Skein::Node* node1 = *((Skein::Node**)element1);
  Skein::Node* node2 = *((Skein::Node**)element2);
  return node1->GetLine().Compare(node2->GetLine());
}

bool Skein::Node::SortChildren(void)
{
  if (!m_children.IsEmpty())
  {
    // Sort a copy of the children array
    CArray<Node*> sorted;
    sorted.Copy(m_children);
    qsort(sorted.GetData(),sorted.GetSize(),sizeof (Node*),CompareByLine);

    // Has the order changed?
    for (int i = 0; i < m_children.GetSize(); i++)
    {
      if (sorted[i] != m_children[i])
      {
        // Use the sorted array
        m_children.RemoveAll();
        m_children.Copy(sorted);
        return true;
      }
    }
  }
  return false;
}

bool Skein::Node::ReleaseChild(Node* child)
{
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i] == child)
    {
      m_children.RemoveAt(i);
      return true;
    }
  }
  return false;
}

Skein::Node* Skein::Node::Find(const CStringW& line)
{
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i]->m_line.Compare(line) == 0)
      return m_children[i];
  }
  return NULL;
}

Skein::Node* Skein::Node::FindAncestor(Node* descendant)
{
  while (descendant != NULL)
  {
    if (descendant->GetParent() == this)
      return descendant;
    descendant = descendant->GetParent();
  }
  return NULL;
}

const char* Skein::Node::GetUniqueId(void)
{
  return m_id;
}

void Skein::Node::SaveNodes(FILE* skeinFile)
{
  CStringW saveTranscript = m_textTranscript;
  CStringW saveExpected = m_textExpected;
  Node* leafItem = this;

  // If there are test sub-items, include them in the transcript and expected text
  if (IsTestCommand())
  {
    int commandTotal = 1;
    while (leafItem->m_children.GetSize() == 1)
    {
      Node* child = leafItem->m_children[0];
      if (!child->IsTestSubItem())
        break;

      leafItem = child;
      commandTotal++;
    }

    Node* loopItem = this;
    for (int commandIndex = 1; commandIndex < commandTotal; commandIndex++)
    {
      loopItem = loopItem->m_children[0];
      saveTranscript.AppendFormat(L"[%d] %s\n%s",commandIndex,loopItem->m_line,loopItem->m_textTranscript);
      saveExpected.AppendFormat(L"[%d] %s\n%s",commandIndex,loopItem->m_line,loopItem->m_textExpected);
    }
  }

  fprintf(skeinFile,
    "  <item nodeId=\"%s\">\n"
    "    <command xml:space=\"preserve\">%s</command>\n"
    "    <result xml:space=\"preserve\">%s</result>\n"
    "    <commentary xml:space=\"preserve\">%s</commentary>\n"
    "    <changed>%s</changed>\n",
    (LPCTSTR)m_id,
    (LPCTSTR)TextFormat::ToXML_UTF8(m_line),
    (LPCTSTR)TextFormat::ToXML_UTF8(saveTranscript),
    (LPCTSTR)TextFormat::ToXML_UTF8(saveExpected),
    ToXML_UTF8(m_changed));

  if (m_label.GetLength() > 0)
  {
    fprintf(skeinFile,
      "    <annotation xml:space=\"preserve\">%s</annotation>\n",
      (LPCTSTR)TextFormat::ToXML_UTF8(m_label));
  }

  // Write out children, but only if there are non-test children. Note that if this is a test
  // node, the children written out are the non-test children of the leaf item of the test nodes,
  // i.e. any command that comes after the testing sequence.
  if (leafItem->GetNumChildren() > leafItem->GetNumTestSubChildren())
  {
    fprintf(skeinFile,"    <children>\n");
    for (int i = 0; i < leafItem->GetNumChildren(); i++)
    {
      if (!leafItem->GetChild(i)->IsTestSubItem())
      {
        fprintf(skeinFile,"      <child nodeId=\"%s\"/>\n",
          (LPCTSTR)leafItem->GetChild(i)->m_id);
      }
    }
    fprintf(skeinFile,"    </children>\n");
  }

  // End the XML description of this node
  fprintf(skeinFile,"  </item>\n");

  // Save any non-test children of the leaf node
  for (int i = 0; i < leafItem->GetNumChildren(); i++)
  {
    if (!leafItem->GetChild(i)->IsTestSubItem())
      leafItem->GetChild(i)->SaveNodes(skeinFile);
  }
}

void Skein::Node::GetNodesByDepth(int depth, std::vector<std::vector<Node*> >& nodesByDepth)
{
  if (depth >= (int)nodesByDepth.size())
    nodesByDepth.resize(depth+1);
  nodesByDepth[depth].push_back(this);
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->GetNodesByDepth(depth+1,nodesByDepth);
}

int Skein::Node::GetX(int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  return m_layout[idx].pos.x;
}

void Skein::Node::SetX(int idx, int x)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  m_layout[idx].pos.x = x;
}

void Skein::Node::ShiftX(int idx, int shift)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  m_layout[idx].pos.x += shift;
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->ShiftX(idx,shift);
}

int Skein::Node::GetY(int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  return m_layout[idx].pos.y;
}

void Skein::Node::SetY(int idx, int y)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  m_layout[idx].pos.y = y;
}

void Skein::Node::ShiftY(int idx, int shift)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  m_layout[idx].pos.y += shift;
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->ShiftY(idx,shift);
}

void Skein::Node::AnimatePrepare(int idx)
{
  if (idx < 0)
  {
    for (int i = 0; i < LAYOUTS; i++)
    {
      LayoutInfo& info = m_layout[i];
      info.anim = true;
      info.animPos = info.pos;
    }
  }
  else
  {
    ASSERT((idx >= 0) && (idx < LAYOUTS));

    LayoutInfo& info = m_layout[idx];
    info.anim = true;
    info.animPos = info.pos;
  }
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->AnimatePrepare(idx);
}

void Skein::Node::AnimateClear(void)
{
  for (int i = 0; i < LAYOUTS; i++)
  {
    LayoutInfo& info = m_layout[i];
    info.anim = false;
    info.animPos = CSize();
  }
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->AnimateClear();
}

CPoint Skein::Node::GetAnimatePos(int idx, int pct)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));
  LayoutInfo& info = m_layout[idx];

  int x, y;
  if (info.anim && (pct >= 0) && (pct < 100))
  {
    x = info.animPos.x + (((info.pos.x - info.animPos.x) * pct) / 100);
    y = info.animPos.y + (((info.pos.y - info.animPos.y) * pct) / 100);
  }
  else
  {
    x = info.pos.x;
    y = info.pos.y;
  }
  return CSize(x,y);
}

bool Skein::Node::IsAnimated(int idx)
{
  ASSERT((idx >= 0) && (idx < LAYOUTS));

  return m_layout[idx].anim;
}

const CStringW& Skein::Node::GetTranscriptText(void)
{
  return m_textTranscript;
}

const CStringW& Skein::Node::GetExpectedText(void)
{
  return m_textExpected;
}

const TranscriptDiff& Skein::Node::GetTranscriptDiff(void)
{
  return m_diff;
}

static CStringW PromptForString(const CStringW& str)
{
  int lastCarriageReturnIndex = str.ReverseFind('\n');
  if (lastCarriageReturnIndex < 0)
    return L"";
  return str.Mid(lastCarriageReturnIndex + 1);
}

static CStringW StringByRemovingPrompt(const CStringW& str)
{
  int lastCarriageReturnIndex = str.ReverseFind('\n');
  if (lastCarriageReturnIndex < 0)
    return str;
  return str.Left(lastCarriageReturnIndex);
}

static CStringW TrailingWhitespace(const CStringW& str)
{
  // Search backwards from the end of the string for a non-whitespace character
  for (int i = str.GetLength() - 1; i >= 0; i--)
  {
    if (!isspace(str[i]))
      return str.Mid(i+1);
  }
  return str;
}

static CStringW LeadingWhitespace(const CStringW& str)
{
  // Find the first non-whitespace character
  for (int i = 0; i < str.GetLength(); i++)
  {
    if (!isspace(str[i]))
      return str.Left(i);
  }
  return str;
}

CStringW StringByRemovingTrailingWhitespace(const CStringW& str)
{
  CStringW trailing = TrailingWhitespace(str);
  return str.Left(str.GetLength() - trailing.GetLength());
}

CStringW StringByRemovingLeadingWhitespace(const CStringW& str)
{
  CStringW leading = LeadingWhitespace(str);
  return str.Mid(leading.GetLength());
}

void Skein::Node::CompareWithExpected(void)
{
  CStringW localIdeal = m_textExpected;
  CStringW localActual = m_textTranscript;

  // Remove matching prompts
  bool promptsMatch = PromptForString(localActual).Compare(PromptForString(localIdeal)) == 0;
  bool idealHasStandardPrompt  = localIdeal.Right(2).Compare(L"\n>") == 0;
  bool actualHasStandardPrompt = localActual.Right(2).Compare(L"\n>") == 0;
  bool hasIdealOutput  = localIdeal.GetLength() > 0;
  bool hasActualOutput = localActual.GetLength() > 0;
  bool canRemovePrompt = promptsMatch || (!hasActualOutput && idealHasStandardPrompt) || (!hasIdealOutput && actualHasStandardPrompt);
  if (canRemovePrompt)
  {
    localIdeal  = StringByRemovingPrompt(localIdeal);
    localActual = StringByRemovingPrompt(localActual);
  }

  // Remove matching trailing whitespace
  CStringW idealTrailingWhitespace  = TrailingWhitespace(localIdeal);
  CStringW actualTrailingWhitespace = TrailingWhitespace(localActual);
  if ((idealTrailingWhitespace.Compare(actualTrailingWhitespace) == 0) || !hasIdealOutput || !hasActualOutput)
  {
    localIdeal  = StringByRemovingTrailingWhitespace(localIdeal);
    localActual = StringByRemovingTrailingWhitespace(localActual);
  }

  // Remove matching leading whitespace
  CStringW idealLeadingWhitespace  = LeadingWhitespace(localIdeal);
  CStringW actualLeadingWhitespace = LeadingWhitespace(localActual);
  if ((idealLeadingWhitespace.Compare(actualLeadingWhitespace) == 0) || !hasIdealOutput || !hasActualOutput)
  {
    localIdeal  = StringByRemovingLeadingWhitespace(localIdeal);
    localActual = StringByRemovingLeadingWhitespace(localActual);
  }

  m_diff.Diff(localIdeal,localActual);
}

Skein::Node::LayoutInfo::LayoutInfo()
  : width(-1), lineWidth(-1), labelWidth(-1), anim(false)
{
}

void Skein::Node::LayoutInfo::ClearWidths()
{
  width = -1;
  lineWidth = -1;
  labelWidth = -1;
}
