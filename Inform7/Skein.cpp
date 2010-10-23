#include "stdafx.h"
#include "Skein.h"
#include "Inform.h"
#include "TextFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SKEIN_FILE "\\Skein.skein"

Skein::Skein() : m_layout(false), m_edited(false)
{
  m_root = new Node(L"- start -",L"",L"",L"",false,false,false,0);
  m_current = m_root;
  m_played = m_root;
  m_maxSaveTemp = theApp.GetProfileInt("Skein","Save Temp",100);
}

Skein::~Skein()
{
  delete m_root;
}

Skein::Node* Skein::GetRoot(void)
{
  return m_root;
}

Skein::Node* Skein::GetCurrent(void)
{
  return m_current;
}

void Skein::SetCurrent(Node* node)
{
  m_current = node;
  NotifyChange(ThreadChanged);
}

bool Skein::InCurrentThread(Node* node)
{
  return InThread(node,m_current);
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
  CString fileName = path;
  fileName += SKEIN_FILE;

  // Create an XML document instance
  CComPtr<IXMLDOMDocument> doc;
  if (FAILED(doc.CoCreateInstance(CLSID_DOMDocument)))
    return;

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

    nodes[id] = new Node(command,label,result,commentary,played,changed,temp,score);
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

  // Get the root and current nodes
  CStringW root = StringFromXML(doc,L"/Skein/@rootNode");
  CStringW current = StringFromXML(doc,L"/Skein/activeNode/@nodeId");

  // Discard the current skein and replace with the new
  delete m_root;
  m_root = nodes[root];
  m_current = nodes[current];
  m_played = m_root;

  m_layout = false;
  NotifyChange(TreeChanged);
  NotifyEdit(false);
}

bool Skein::Save(const char* path)
{
  std::set<Node*> tempNodes;
  m_root->GetTempNodes(tempNodes,m_maxSaveTemp);
  Node* current = m_current->WillSaveNode(tempNodes) ? m_current : m_root;

  CString fileName = path;
  fileName += SKEIN_FILE;

  FILE* skeinFile = fopen(fileName,"wt");
  if (skeinFile == NULL)
    return false;

  fprintf(skeinFile,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Skein rootNode=\"%s\" xmlns=\"http://www.logicalshift.org.uk/IF/Skein\">\n"
    "  <generator>Inform 7 for Windows</generator>\n"
    "  <activeNode nodeId=\"%s\"/>\n",
    m_root->GetUniqueId(),current->GetUniqueId());
  m_root->SaveNodes(skeinFile,tempNodes);
  fprintf(skeinFile,"</Skein>\n");
  fclose(skeinFile);
  NotifyEdit(false);
  return true;
}

void Skein::Import(const char* path)
{
  Node* node = m_root;
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
          newNode = new Node(recLineW,L"",L"",L"",false,false,true,0);
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

bool Skein::IsEdited(void)
{
  return m_edited;
}

bool Skein::NeedSaveWarn(int& maxTemp)
{
  maxTemp = m_maxSaveTemp;
  std::set<Node*> tempNodes;
  m_root->GetTempNodes(tempNodes,maxTemp+1);
  return ((int)tempNodes.size() > maxTemp);
}

void Skein::Reset(bool current)
{
  if (current)
    m_current = m_root;

  m_played = m_root;
  NotifyChange(ThreadChanged);
}

void Skein::Layout(CDC& dc, CFont* labelFont, int spacing)
{
  if (m_layout == false)
    m_root->Layout(dc,labelFont,0,spacing);
  m_layout = true;
}

void Skein::NewLine(const CStringW& line)
{
  bool nodeAdded = false;
  CStringW nodeLine = EscapeLine(line,UsePrintable);

  // Is there a child node with the same line?
  Node* node = m_current->Find(nodeLine);
  if (node == NULL)
  {
    node = new Node(nodeLine,L"",L"",L"",true,false,true,0);
    m_current->Add(node);
    nodeAdded = true;
  }

  // Make this the new current node
  m_current = node;
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
  Node* next = m_played->FindAncestor(m_current);
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
  m_played->SetPlayed();
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
  Node* node = m_current;
  while (--history > 0)
  {
    node = node->GetParent();
    if (node == NULL)
      return false;
  }

  // Don't return the root node
  if (node == m_root)
    return false;

  line = EscapeLine(node->GetLine(),RemoveEscapes);
  return true;
}

Skein::Node* Skein::AddNew(Node* node)
{
  Node* newNode = new Node(L"",L"",L"",L"",false,false,true,0);
  node->Add(newNode);

  m_layout = false;
  NotifyChange(TreeChanged);
  NotifyEdit(true);

  return newNode;
}

Skein::Node* Skein::AddNewParent(Node* node)
{
  Node* newNode = new Node(L"",L"",L"",L"",false,false,true,0);
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
    bool inCurrent = InCurrentThread(node);
    removed = parent->Remove(node);

    if (inCurrent)
    {
      m_current = m_root;
      m_played = m_root;
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
    bool inCurrent = InCurrentThread(node);
    removed = parent->RemoveSingle(node);

    if (inCurrent)
    {
      m_current = m_root;
      m_played = m_root;
    }

    m_layout = false;
    NotifyChange(TreeChanged);
  }
  if (removed)
    NotifyEdit(true);
  return removed;
}

void Skein::SetLine(Node* node, LPWSTR line)
{
  if (node->SetLine(line))
    NotifyEdit(true);
  m_layout = false;
  NotifyChange(NodeTextChanged);
}

void Skein::SetLabel(Node* node, LPWSTR label)
{
  if (node->SetLabel(label))
    NotifyEdit(true);
  m_layout = false;
  NotifyChange(NodeTextChanged);
}

void Skein::Lock(Node* node)
{
  while (node != NULL)
  {
    if (node->SetTemporary(false))
      NotifyEdit(true);
    node = node->GetParent();
  }
  NotifyChange(LockChanged);
}

void Skein::Unlock(Node* node, bool notify)
{
  if (node->SetTemporary(true))
    NotifyEdit(true);
  for (int i = 0; i < node->GetNumChildren(); i++)
    Unlock(node->GetChild(i),false);

  if (notify)
    NotifyChange(LockChanged);
}

void Skein::Trim(Node* node, bool running, bool notify)
{
  int i = 0;
  while (i < node->GetNumChildren())
  {
    Node* child = node->GetChild(i);
    bool inCurrent = InCurrentThread(child);

    // Only delete unlocked nodes. If the game is running, only delete
    // if the node is not in the current thread as well.
    if (child->GetTemporary() && !(running && inCurrent))
    {
      if (inCurrent)
      {
        m_current = m_root;
        m_played = m_root;
      }

      if (RemoveAll(child,false) == false)
        i++;
    }
    else
    {
      Trim(child,running,false);
      i++;
    }
  }

  if (notify)
    NotifyChange(TreeChanged);
}

void Skein::GetLabels(std::map<CStringW,Node*>& labels)
{
  m_root->GetLabels(labels);
}

bool Skein::HasLabels(void)
{
  return m_root->HasLabels();
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

Skein::Node* Skein::GetThreadTop(Node* node)
{
  while (true)
  {
    Node* parent = node->GetParent();
    if (parent == NULL)
    {
      ASSERT(FALSE);
      return node;
    }
    if (parent->GetNumChildren() != 1)
      return node;
    if (parent->GetParent() == NULL)
      return node;
    node = parent;
  }
}

Skein::Node* Skein::GetThreadBottom(Node* node)
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
    node = m_root;

  if (testNode == node)
    return true;

  for (int i = 0; i < node->GetNumChildren(); i++)
  {
    if (IsValidNode(testNode,node->GetChild(i)))
      return true;
  }
  return false;
}

int Skein::GetBlessedThreadEnds(std::vector<Node*>& nodes, Node* node)
{
  if (node == NULL)
    node = m_root;

  // Count the number of blessed descendants
  int count = 0;
  for (int i = 0; i < node->GetNumChildren(); i++)
    count += GetBlessedThreadEnds(nodes,node->GetChild(i));

  // Is this node blessed?
  if (node->GetExpectedText().IsEmpty() == FALSE)
  {
    // If this node has no blessed descendants, add to the set
    if (count == 0)
      nodes.push_back(node);
    count++;
  }
  return count;
}

Skein::Node* Skein::GetFirstDifferent(Node* node)
{
  if (node == NULL)
    node = m_root;

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
    node = m_root;
  nodes.Add(node);

  for (int i = 0; i < node->GetNumChildren(); i++)
    GetAllNodes(nodes,node->GetChild(i));
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
  if (m_edited != edited)
  {
    m_edited = edited;
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
  const CStringW& expected, bool played, bool changed, bool temp, int score)
  : m_parent(NULL), m_line(line), m_label(label), m_textTranscript(transcript),
    m_textExpected(expected), m_played(played), m_changed(changed), m_temp(temp),
    m_differs(ExpectedDifferent), m_score(score), m_width(-1), m_lineWidth(-1),
    m_labelWidth(0), m_x(0)
{
  m_id.Format("node-0x%p",this);
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

bool Skein::Node::SetLine(LPWSTR line)
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

bool Skein::Node::SetLabel(LPWSTR label)
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

bool Skein::Node::GetTemporary(void)
{
  return m_temp;
}

void Skein::Node::SetPlayed(void)
{
  m_played = true;
}

bool Skein::Node::SetTemporary(bool temp)
{
  bool change = (m_temp != temp);
  m_temp = temp;
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

int Skein::Node::GetLineWidth(CDC& dc, CFont* labelFont)
{
  if (m_width < 0)
  {
    SIZE size;
    ::GetTextExtentPoint32W(dc.GetSafeHdc(),m_line,(UINT)wcslen(m_line),&size);
    m_width = size.cx;
    m_lineWidth = size.cx;

    if (wcslen(m_label) > 0)
    {
      CFont* oldFont = dc.SelectObject(labelFont);
      ::GetTextExtentPoint32W(dc.GetSafeHdc(),m_label,(UINT)wcslen(m_label),&size);
      dc.SelectObject(oldFont);
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

int Skein::Node::GetLineTextWidth(void)
{
  return m_lineWidth;
}

int Skein::Node::GetLabelTextWidth(void)
{
  return m_labelWidth;
}

int Skein::Node::GetTreeWidth(CDC& dc, CFont* labelFont, int spacing)
{
  // Get the tree width of all children
  int total = 0;
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    total += m_children[i]->GetTreeWidth(dc,labelFont,spacing);
    if (i > 0)
      total += spacing;
  }

  // Return the largest of the above, the width of this node's line
  // and the width of this node's label
  int width = max(total,GetLineWidth(dc,labelFont));
  return max(width,GetLabelTextWidth());
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

int Skein::Node::GetMaxDepth(void)
{
  int max = 0;
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    int depth = m_children[i]->GetMaxDepth();
    if (depth > max)
      max = depth;
  }
  return max+1;
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

void Skein::Node::GetTempNodes(std::set<Node*>& nodes, int max)
{
  if (m_temp)
  {
    if ((int)nodes.size() >= max)
      return;
    nodes.insert(this);
  }
  for (int i = 0; i < m_children.GetSize(); i++)
    m_children[i]->GetTempNodes(nodes,max);
}

bool Skein::Node::WillSaveNode(const std::set<Node*>& tempNodes)
{
  if (m_temp)
    return (tempNodes.count(this) > 0);
  return true;
}

void Skein::Node::SaveNodes(FILE* skeinFile, const std::set<Node*>& tempNodes)
{
  fprintf(skeinFile,
    "  <item nodeId=\"%s\">\n"
    "    <command xml:space=\"preserve\">%s</command>\n"
    "    <result xml:space=\"preserve\">%s</result>\n"
    "    <commentary xml:space=\"preserve\">%s</commentary>\n"
    "    <played>%s</played>\n"
    "    <changed>%s</changed>\n"
    "    <temporary score=\"%d\">%s</temporary>\n",
    m_id,
    (LPCTSTR)TextFormat::ToXML_UTF8(m_line),
    (LPCTSTR)TextFormat::ToXML_UTF8(m_textTranscript),
    (LPCTSTR)TextFormat::ToXML_UTF8(m_textExpected),
    ToXML_UTF8(m_played),
    ToXML_UTF8(m_changed),
    m_score,
    ToXML_UTF8(m_temp));

  if (m_label.GetLength() > 0)
  {
    fprintf(skeinFile,
      "    <annotation xml:space=\"preserve\">%s</annotation>\n",
      (LPCTSTR)TextFormat::ToXML_UTF8(m_label));
  }

  CArray<Node*> children;
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    if (m_children[i]->WillSaveNode(tempNodes))
      children.Add(m_children[i]);
  }

  if (children.GetSize() > 0)
  {
    fprintf(skeinFile,"    <children>\n");
    for (int i = 0; i < children.GetSize(); i++)
      fprintf(skeinFile,"      <child nodeId=\"%s\"/>\n",children[i]->m_id);
    fprintf(skeinFile,"    </children>\n");
  }
  fprintf(skeinFile,"  </item>\n");

  for (int i = 0; i < children.GetSize(); i++)
    children[i]->SaveNodes(skeinFile,tempNodes);
}

void Skein::Node::Layout(CDC& dc, CFont* labelFont, int x, int spacing)
{
  // Store the centre x co-ordinate for this node
  m_x = x;

  // Find the total width of all descendant nodes
  int total = 0;
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    total += m_children[i]->GetTreeWidth(dc,labelFont,spacing);
    if (i > 0)
      total += spacing;
  }

  // Lay out each child node
  int cx = 0;
  for (int i = 0; i < m_children.GetSize(); i++)
  {
    int tw = m_children[i]->GetTreeWidth(dc,labelFont,spacing);
    m_children[i]->Layout(dc,labelFont,x-(total/2)+cx+(tw/2),spacing);
    cx += tw+spacing;
  }
}

int Skein::Node::GetX(void)
{
  return m_x;
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
  m_differs = !(m_textTranscript == m_textExpected) ? ExpectedDifferent : ExpectedSame;
  if (m_differs == ExpectedDifferent)
  {
    if ((m_textTranscript.IsEmpty() == FALSE) && (m_textExpected.IsEmpty() == FALSE))
    {
      if (StripWhite(m_textTranscript).CompareNoCase(StripWhite(m_textExpected)) == 0)
        m_differs = ExpectedNearlySame;
    }
  }

  m_diffExpected.clear();
  m_diffTranscript.clear();
  if ((m_differs == ExpectedDifferent) && (m_textExpected.IsEmpty() == FALSE))
    Diff::DiffStrings(m_textExpected,m_textTranscript,m_diffExpected,m_diffTranscript);
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
