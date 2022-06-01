#include "stdafx.h"
#include "Skein.h"
#include "Inform.h"
#include "TextFormat.h"
#include "Build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Skein::Skein() : m_layout(false)
{
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
  m_playTo = node;
  NotifyChange(ThreadChanged);
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

    nodes[id] = new Node(command,label,result,commentary,changed);
    item = NULL;
  }

  // Loop through the XML item nodes again, setting up the parent-child links
  items->reset();
  while (items->nextNode(&item) == S_OK)
  {
    Node* parentNode = nodes[StringFromXML(item,L"@nodeId").m_str];

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

  m_layout = false;
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

  m_layout = false;
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
      m_layout = false;
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

      m_layout = false;
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
  NotifyChange(ThreadChanged);
}

void Skein::Layout(CDC& dc, int spacing, bool force)
{
  if (force)
    m_inst.root->ClearWidths();
  if (force || (m_layout == false))
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
        int width = node->GetLineWidth(dc);
        width = max(width,node->GetLabelTextWidth());
        int x_next = x + (width/2);

        // Centre a parent node between its children
        int numc = node->GetNumChildren();
        if (numc > 0)
        {
          int x_first_child = node->GetChild(0)->GetX();
          int x_last_child = node->GetChild(numc-1)->GetX();
          int x_centre = (x_first_child + x_last_child)/2;
          node->SetX(x_centre);
          if (x_next > x_centre)
          {
            // Move this node and all its children to the right
            node->ShiftX(x_next - x_centre);

            // Move all nodes after this node to the right
            for (size_t col2 = col+1; col2 < rowNodes.size(); ++col2)
              rowNodes[col2]->ShiftX(x_next - x_centre);
          }
        }
        else
          node->SetX(x_next);

        x = node->GetX() + (width/2) + spacing;
      }
    }

    // Shift the entire tree so that is centered horizontally
    m_inst.root->ShiftX(-m_inst.root->GetX());
    int xmin = 0, xmax = 0;
    bool valid = false;
    for (size_t row = 0; row < nodesByDepth.size(); ++row)
    {
      const std::vector<Node*>& rowNodes = nodesByDepth[row];
      if (rowNodes.size() > 0)
      {
        Node* node1 = rowNodes[0];
        Node* node2 = rowNodes[rowNodes.size()-1];
        int x1 = node1->GetX() - (node1->GetLayoutWidth()/2);
        int x2 = node2->GetX() + (node2->GetLayoutWidth()/2);
        if (!valid || (x1 < xmin))
          xmin = x1;
        if (!valid || (x2 > xmax))
          xmax = x2;
        valid = true;
      }
    }
    if (valid)
      m_inst.root->ShiftX((xmin + xmax) / -2);
  }
  m_layout = true;
}

void Skein::GetTreeExtent(int& width, int& depth)
{
  std::vector<std::vector<Node*> > nodesByDepth;
  m_inst.root->GetNodesByDepth(0,nodesByDepth);

  width = 0;
  depth = (int)nodesByDepth.size();

  int xmin = 0, xmax = 0;
  bool valid = false;
  for (size_t row = 0; row < nodesByDepth.size(); ++row)
  {
    const std::vector<Node*>& rowNodes = nodesByDepth[row];
    if (rowNodes.size() > 0)
    {
      Node* node1 = rowNodes[0];
      Node* node2 = rowNodes[rowNodes.size()-1];
      int x1 = node1->GetX() - (node1->GetLayoutWidth()/2);
      int x2 = node2->GetX() + (node2->GetLayoutWidth()/2);
      if (!valid || (x1 < xmin))
        xmin = x1;
      if (!valid || (x2 > xmax))
        xmax = x2;
      valid = true;
    }
  }
  if (valid)
    width = (xmax - xmin);
}

void Skein::NewLine(const CStringW& line)
{
  bool nodeAdded = false;
  CStringW nodeLine = EscapeLine(line,UsePrintable);

  // Is there a child node with the same line?
  Node* node = m_playTo->Find(nodeLine);
  if (node == NULL)
  {
    node = new Node(nodeLine,L"",L"",L"",false);
    m_playTo->Add(node);
    nodeAdded = true;
  }

  // Make this the new node being played
  m_playTo = node;
  m_played = node;

  // Notify any listeners
  if (nodeAdded)
  {
    m_layout = false;
    NotifyChange(TreeChanged);
    NotifyEdit(true);
  }
  else
    NotifyChange(ThreadChanged);

  NotifyShowNode(node,ShowNewLine);
}

bool Skein::NextLine(CStringW& line)
{
  // Find the next node to use
  Node* next = m_played->FindAncestor(m_playTo);
  if (next != NULL)
  {
    line = EscapeLine(next->GetLine(),UseCharacters);
    m_played = next;
    NotifyChange(ThreadChanged);
    NotifyShowNode(next,ShowNewLine);
    return true;
  }
  return false;
}

void Skein::UpdateAfterPlaying(const CStringW& transcript)
{
  // Update the status of the last played node
  if (transcript.IsEmpty() == FALSE)
    m_played->NewTranscriptText(transcript);

  NotifyChange(NodeColourChanged);
  if (transcript.IsEmpty() == FALSE)
    NotifyShowNode(m_played,ShowNewTranscript);
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

  m_layout = false;
  NotifyChange(TreeChanged);
  NotifyEdit(true);

  return newNode;
}

Skein::Node* Skein::AddNewParent(Node* node)
{
  Node* newNode = new Node(L"",L"",L"",L"",false);
  node->GetParent()->Replace(node,newNode);
  newNode->Add(node);

  m_layout = false;
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

    m_layout = false;
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

    if (inPlay)
    {
      m_playTo = m_inst.root;
      m_played = m_inst.root;
    }

    m_layout = false;
    NotifyChange(TreeChanged);
  }
  if (removed)
    NotifyEdit(true);
  return removed;
}

void Skein::SetLine(Node* node, LPCWSTR line)
{
  if (node->SetLine(line))
    NotifyEdit(true);
  m_layout = false;
  NotifyChange(NodeTextChanged);
}

void Skein::SetLabel(Node* node, LPCWSTR label)
{
  if (node->SetLabel(label))
    NotifyEdit(true);
  m_layout = false;
  NotifyChange(NodeTextChanged);
}

void Skein::GetLabels(std::map<CStringW,Node*>& labels)
{
  m_inst.root->GetLabels(labels);
}

bool Skein::HasLabels(void)
{
  return m_inst.root->HasLabels();
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
  NotifyChange(NodeColourChanged);
}

bool Skein::CanBless(Node* node, bool all)
{
  bool canBless = false;
  while (node != NULL)
  {
    canBless |= node->CanBless();
    node = all ? node->GetParent() : NULL;
  }
  return canBless;
}

void Skein::SetExpectedText(Node* node, LPCWSTR text)
{
  if (node->SetExpectedText(text))
    NotifyEdit(true);
  NotifyChange(NodeColourChanged);
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
  if (node->GetDiffers() != Skein::Node::ExpectedSame)
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

void Skein::SaveTranscript(Node* node, const char* path)
{
  std::vector<Node*> list;
  do
  {
    list.push_back(node);
    node = node->GetParent();
  }
  while (node != NULL);

  FILE* transcript = fopen(path,"wt");
  if (transcript == NULL)
    return;

  for (std::vector<Node*>::reverse_iterator it = list.rbegin(); it != list.rend(); ++it)
  {
    if (it == list.rbegin())
    {
      fprintf(transcript,"%s",
        (LPCTSTR)TextFormat::UnicodeToUTF8((*it)->GetTranscriptText()));
    }
    else
    {
      fprintf(transcript,"%s\n%s",
        (LPCTSTR)TextFormat::UnicodeToUTF8((*it)->GetLine()),
        (LPCTSTR)TextFormat::UnicodeToUTF8((*it)->GetTranscriptText()));
    }
  }
  fclose(transcript);
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

void Skein::NotifyShowNode(Node* node, Show why)
{
  for (std::vector<Listener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
    (*it)->SkeinShowNode(node,why);
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
    m_locked(false), m_changed(changed), m_differs(ExpectedDifferent),
    m_width(-1), m_lineWidth(-1), m_labelWidth(-1), m_x(0),
    m_anim(false), m_animX(0), m_animDepth(0)
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
  m_width = -1;
  m_lineWidth = -1;
  m_labelWidth = -1;
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
  m_width = -1;
  m_lineWidth = -1;
  m_labelWidth = -1;
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

Skein::Node::ExpectedCompare Skein::Node::GetDiffers(void)
{
  return m_differs;
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

void Skein::Node::NewTranscriptText(const CStringW& transcript)
{
  if (!m_textTranscript.IsEmpty())
    m_changed = !(m_textTranscript == transcript);
  m_textTranscript = transcript;
  m_textTranscript.Replace('\r','\n');
  CompareWithExpected();
}

bool Skein::Node::Bless(void)
{
  bool differs = (m_differs != ExpectedSame);
  m_textExpected = m_textTranscript;
  m_differs = ExpectedSame;

  m_diffExpected.clear();
  m_diffTranscript.clear();
  return differs;
}

bool Skein::Node::CanBless(void)
{
  return (m_differs != ExpectedSame);
}

bool Skein::Node::SetExpectedText(LPCWSTR text)
{
  CStringW oldExpected = m_textExpected;
  m_textExpected = text;
  m_textExpected.Replace('\r','\n');
  CompareWithExpected();
  return (oldExpected != m_textExpected);
}

int Skein::Node::GetLineWidth(CDC& dc)
{
  if (m_width < 0)
  {
    SIZE size;
    ::GetTextExtentPoint32W(dc.GetSafeHdc(),m_line,(UINT)wcslen(m_line),&size);
    m_width = size.cx;
    m_lineWidth = size.cx;

    if (wcslen(m_label) > 0)
    {
      ::GetTextExtentPoint32W(dc.GetSafeHdc(),m_label,(UINT)wcslen(m_label),&size);
      m_labelWidth = size.cx;
    }
    else
      m_labelWidth = 0;

    CSize minSize = dc.GetTextExtent("    ",4);
    if (m_width < minSize.cx)
      m_width = minSize.cx;
    if (m_labelWidth < minSize.cx)
      m_labelWidth = minSize.cx;
  }
  return m_width;
}

int Skein::Node::GetLayoutWidth(void)
{
  return max(m_width,m_labelWidth);
}

int Skein::Node::GetLineTextWidth(void)
{
  return m_lineWidth;
}

int Skein::Node::GetLabelTextWidth(void)
{
  return m_labelWidth;
}

void Skein::Node::ClearWidths(void)
{
  m_width = -1;
  m_lineWidth = -1;
  m_labelWidth = -1;

  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->ClearWidths();
}

int Skein::Node::GetDepth(void)
{
  int depth = 0;

  Node* node = this;
  while (node != NULL)
  {
    node = node->GetParent();
    if (node != NULL)
      depth++;
  }

  return depth;
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
  fprintf(skeinFile,
    "  <item nodeId=\"%s\">\n"
    "    <command xml:space=\"preserve\">%s</command>\n"
    "    <result xml:space=\"preserve\">%s</result>\n"
    "    <commentary xml:space=\"preserve\">%s</commentary>\n"
    "    <changed>%s</changed>\n",
    (LPCTSTR)m_id,
    (LPCTSTR)TextFormat::ToXML_UTF8(m_line),
    (LPCTSTR)TextFormat::ToXML_UTF8(m_textTranscript),
    (LPCTSTR)TextFormat::ToXML_UTF8(m_textExpected),
    ToXML_UTF8(m_changed));

  if (m_label.GetLength() > 0)
  {
    fprintf(skeinFile,
      "    <annotation xml:space=\"preserve\">%s</annotation>\n",
      (LPCTSTR)TextFormat::ToXML_UTF8(m_label));
  }

  if (m_children.GetSize() > 0)
  {
    fprintf(skeinFile,"    <children>\n");
    for (int i = 0; i < m_children.GetSize(); i++)
    {
      fprintf(skeinFile,"      <child nodeId=\"%s\"/>\n",
        (LPCTSTR)m_children[i]->m_id);
    }
    fprintf(skeinFile,"    </children>\n");
  }
  fprintf(skeinFile,"  </item>\n");

  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->SaveNodes(skeinFile);
}

void Skein::Node::GetLabels(std::map<CStringW,Node*>& labels)
{
  if (m_label.GetLength() > 0)
    labels[m_label] = this;
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->GetLabels(labels);
}

bool Skein::Node::HasLabels(void)
{
  if (m_label.GetLength() > 0)
    return true;
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i]->HasLabels())
      return true;
  }
  return false;
}

void Skein::Node::GetNodesByDepth(int depth, std::vector<std::vector<Node*> >& nodesByDepth)
{
  if (depth >= (int)nodesByDepth.size())
    nodesByDepth.resize(depth+1);
  nodesByDepth[depth].push_back(this);
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->GetNodesByDepth(depth+1,nodesByDepth);
}

int Skein::Node::GetX(void)
{
  return m_x;
}

void Skein::Node::SetX(int x)
{
  m_x = x;
}

void Skein::Node::ShiftX(int shift)
{
  m_x += shift;
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->ShiftX(shift);
}

void Skein::Node::AnimatePrepare(int depth)
{
  m_anim = true;
  m_animX = m_x;
  m_animDepth = depth;

  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->AnimatePrepare(depth+1);
}

void Skein::Node::AnimateClear(void)
{
  m_anim = false;
  m_animX = 0;
  m_animDepth = 0;

  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->AnimateClear();
}

int Skein::Node::GetAnimateX(int pct)
{
  if (m_anim && (pct >= 0) && (pct < 100))
    return m_animX + (((m_x - m_animX) * pct) / 100);
  return m_x;
}

int Skein::Node::GetAnimateY(int depth, int spacing, int pct)
{
  if (m_anim && (pct >= 0) && (pct < 100))
    return ((m_animDepth - depth) * spacing * (100 - pct)) / 100;
  return 0;
}

const CStringW& Skein::Node::GetTranscriptText(void)
{
  return m_textTranscript;
}

const CStringW& Skein::Node::GetExpectedText(void)
{
  return m_textExpected;
}

const Diff::DiffResults& Skein::Node::GetTranscriptDiffs(void)
{
  return m_diffTranscript;
}

const Diff::DiffResults& Skein::Node::GetExpectedDiffs(void)
{
  return m_diffExpected;
}

void Skein::Node::CompareWithExpected(void)
{
  CStringW textExpected(m_textExpected);
  CStringW textTranscript(m_textTranscript);
  OverwriteBanner(textExpected);
  OverwriteBanner(textTranscript);

  m_differs = !(textTranscript == textExpected) ? ExpectedDifferent : ExpectedSame;
  if (m_differs == ExpectedDifferent)
  {
    if ((textTranscript.IsEmpty() == FALSE) && (textExpected.IsEmpty() == FALSE))
    {
      if (StripWhite(textTranscript).CompareNoCase(StripWhite(textExpected)) == 0)
        m_differs = ExpectedNearlySame;
    }
  }

  m_diffExpected.clear();
  m_diffTranscript.clear();
  if ((m_differs == ExpectedDifferent) && (textExpected.IsEmpty() == FALSE))
  {
    Diff::DiffStrings(textExpected,textTranscript,m_diffExpected,m_diffTranscript);
  }
}

void Skein::Node::OverwriteBanner(CStringW& inStr)
{
  // Does this text contain an Inform banner?
  int i = inStr.Find(L"\nRelease ");
  if (i >= 0)
  {
    int release, serial;
    if (swscanf((LPCWSTR)inStr+i,L"\nRelease %d / Serial number %d / Inform 7 ",&release,&serial) == 2)
    {
      // Replace the banner line with asterisks
      for (int j = i+1; j < inStr.GetLength(); j++)
      {
        if (inStr.GetAt(j) == '\n')
          break;
        inStr.SetAt(j,'*');
      }
    }
  }
}

CStringW Skein::Node::StripWhite(const CStringW& inStr)
{
  CStringW outStr;
  outStr.Preallocate(inStr.GetLength()+1);
  for (int i = 0; i < inStr.GetLength(); i++)
  {
    WCHAR c = inStr.GetAt(i);
    switch (c)
    {
    case L'\n':
    case L'\r':
    case L' ':
    case L'\t':
      break;
    case '>':
      if (i < inStr.GetLength()-1)
        outStr.AppendChar(c);
    default:
      outStr.AppendChar(c);
      break;
    }
  }
  return outStr;
}
