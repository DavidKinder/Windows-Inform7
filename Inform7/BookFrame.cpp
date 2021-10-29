#include "stdafx.h"
#include "BookFrame.h"
#include "DpiFunctions.h"
#include "Inform.h"
#include "ReportHtml.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class BookContentsView : public CTreeView
{
protected:
  DECLARE_DYNCREATE(BookContentsView)
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSelectionChanged(NMHDR *hdr, LRESULT *result)
  {
    NMTREEVIEW* nmtv = (NMTREEVIEW*)hdr;
    if (nmtv->itemNew.lParam)
    {
      CString* url = (CString*)(nmtv->itemNew.lParam);
      CFrameWnd* frame = GetParentFrame();
      if (frame->IsKindOf(RUNTIME_CLASS(BookFrame)))
        ((BookFrame*)frame)->ShowPage(*url);
    }
  }
};

IMPLEMENT_DYNCREATE(BookContentsView, CTreeView)

BEGIN_MESSAGE_MAP(BookContentsView, CTreeView)
  ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelectionChanged)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(BookFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(BookFrame, CFrameWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_CLOSE()
  ON_WM_SIZE()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_COMMAND(ID_WINDOW_SWITCH, OnWindowSwitchPanes)
END_MESSAGE_MAP()

BookFrame::BookFrame(const char* dir) : m_dir(dir), m_splitter(true)
{
}

void BookFrame::ShowBook(const char* dir)
{
  // If the book is already open, bring it to the front
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  for (int i = 0; i < frames.GetSize(); i++)
  {
    if (frames[i]->IsKindOf(RUNTIME_CLASS(BookFrame)))
    {
      BookFrame* frame = (BookFrame*)frames[i];
      if (frame->m_dir.CompareNoCase(dir) == 0)
      {
        frame->ActivateFrame();
        return;
      }
    }
  }

  // Open a new book
  BookFrame* frame = new BookFrame(dir);
  theApp.NewFrame(frame);
  frame->LoadFrame(IDR_BOOKFRAME,WS_OVERLAPPEDWINDOW,NULL,NULL);
  frame->SetFromRegistryPath(REGISTRY_INFORM_WINDOW);
  frame->ShowWindow(SW_SHOW);
  frame->UpdateWindow();
}

void BookFrame::ShowPage(const char* page)
{
  CWnd* wnd = m_splitter.GetPane(0,1);
  ASSERT(wnd != NULL);
  ASSERT(wnd->IsKindOf(RUNTIME_CLASS(ReportHtml)));
  ReportHtml* html = (ReportHtml*)wnd;

  CString pagePath;
  pagePath.Format("%s\\OEBPS\\%s",(LPCSTR)m_dir,page);
  html->Navigate(theApp.PathToUrl(pagePath),false);
}

CString BookFrame::GetDisplayName(void)
{
  return m_title;
}

BOOL BookFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
  const RECT& rect, CWnd* pParentWnd, LPCTSTR lpszMenuName, DWORD dwExStyle, CCreateContext* pContext)
{
  m_strTitle = lpszWindowName;
  if (!CreateEx(dwExStyle,lpszClassName,lpszWindowName,dwStyle,
    rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,pParentWnd->GetSafeHwnd(),0,pContext))
  {
    return FALSE;
  }
  return TRUE;
}

BOOL BookFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  if (!CFrameWnd::PreCreateWindow(cs))
    return FALSE;

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass = AfxRegisterWndClass(0);
  return TRUE;
}

int BookFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Set the application icon
  theApp.SetIcon(this);

  // Create a splitter to occupy the client area of the frame
  if (!m_splitter.CreateStatic(this,1,2,WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS))
    return -1;
  if (!m_splitter.CreateView(0,1,RUNTIME_CLASS(ReportHtml),CSize(0,0),NULL))
    return -1;
  if (!m_splitter.CreateView(0,0,RUNTIME_CLASS(BookContentsView),CSize(0,0),NULL))
    return -1;

  // Set the splitter as 1/3 on the left
  CRect client;
  GetClientRect(client);
  m_splitter.SetColumnInfo(0,client.Width()/3,16);

  // Set up the tree control for the book contents
  CTreeCtrl& tree = GetTreeCtrl();
  tree.ModifyStyle(0,TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT|TVS_SHOWSELALWAYS);
  tree.SetFont(theApp.GetFont(this,InformApp::FontSystem));

  // Read the book contents
  m_title = ReadContents();
  if (m_title.IsEmpty())
    return -1;
  SetWindowText(m_title);

  // Show the first item in the book
  tree.SelectItem(tree.GetChildItem(TVI_ROOT));
  tree.SetScrollPos(SB_HORZ,0);
  tree.SetScrollPos(SB_VERT,0);
  tree.SetFocus();
  return 0;
}

void BookFrame::OnDestroy()
{
  DeleteItemData(TVI_ROOT);
  ReportHtml::RemoveContext(this);
  CFrameWnd::OnDestroy();
}

void BookFrame::OnClose()
{
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  if (frames.GetSize() == 1)
    theApp.WriteOpenProjectsOnExit();

  theApp.FrameClosing(this);
  CFrameWnd::OnClose();
}

BOOL BookFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  CPushRoutingFrame push(this);
  CWnd* focusWnd = GetFocus();

  // Let the panels in the splitter process the command
  for (int i = 0; i < 2; i++)
  {
    CWnd* wnd = m_splitter.GetPane(0,i);
    if (wnd->IsChild(focusWnd))
    {
      if (wnd->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
        return TRUE;
    }
  }

  // Then pump through frame
  if (CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;

  // Then pump through application
  if (AfxGetApp()->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return FALSE;
}

void BookFrame::OnSize(UINT nType, int cx, int cy)
{
  // Get the existing splitter position before the default action for this message
  double split = 0.0;
  if (m_splitter.GetSafeHwnd() != 0)
    split = m_splitter.GetColumnFraction(0);

  CFrameWnd::OnSize(nType,cx,cy);

  if ((m_splitter.GetSafeHwnd() != 0) && (split > 0.0))
  {
    // Adjust the splitter so that the fractional position is constant
    m_splitter.SetColumnFraction(0,split,16);
    m_splitter.RecalcLayout();
  }
}

LRESULT BookFrame::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  double split = 0.0;
  if (m_splitter.GetSafeHwnd() != 0)
    split = m_splitter.GetColumnFraction(0);

  MoveWindow((LPRECT)lparam,TRUE);

  GetTreeCtrl().SetFont(theApp.GetFont(this,InformApp::FontSystem));
  ReportHtml::UpdateWebBrowserPreferences(this);

  if ((m_splitter.GetSafeHwnd() != 0) && (split > 0.0))
  {
    m_splitter.SetColumnFraction(0,split,16);
    m_splitter.RecalcLayout();
  }
  return 0;
}

void BookFrame::OnWindowSwitchPanes()
{
  CWnd* focusWnd = GetFocus();
  CWnd* leftWnd = m_splitter.GetPane(0,0);

  if ((leftWnd == focusWnd) || leftWnd->IsChild(focusWnd))
    m_splitter.GetPane(0,1)->SetFocus();
  else
    leftWnd->SetFocus();
}

void BookFrame::SetFromRegistryPath(const char* path)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,path,KEY_READ) == ERROR_SUCCESS)
  {
    // Restore the window state
    WINDOWPLACEMENT place;
    ULONG len = sizeof WINDOWPLACEMENT;
    if (registryKey.QueryBinaryValue("Placement 96dpi",&place,&len) == ERROR_SUCCESS)
    {
      DPI::ContextUnaware dpiUnaware;
      SetWindowPlacement(&place);
    }
    else if (registryKey.QueryBinaryValue("Placement",&place,&len) == ERROR_SUCCESS)
      SetWindowPlacement(&place);
  }
}

CString BookFrame::ReadContents(void)
{
  // Open the XML file
  CFile tocFile;
  CString tocPath(m_dir);
  tocPath.Append("\\OEBPS\\toc.ncx");
  if (tocFile.Open(tocPath,CFile::modeRead) == FALSE)
    return "";

  // Read in the XML file
  int tocSize = (int)tocFile.GetLength();
  CString xml;
  LPSTR xmlBuffer = xml.GetBufferSetLength(tocSize);
  tocFile.Read(xmlBuffer,tocSize);
  xml.ReleaseBuffer(tocSize);

  // Create an XML document instance
  CComPtr<IXMLDOMDocument> doc;
  if (FAILED(doc.CoCreateInstance(CLSID_DOMDocument)))
    return "";

  // Load the XML into the document
  VARIANT_BOOL success = 0;
  CStreamOnCString xmlStream(xml);
  if (doc->load(CComVariant(&xmlStream),&success) != S_OK)
  {
    CComPtr<IXMLDOMParseError> error;
    doc->get_parseError(&error);

    long line = 0;
    error->get_line(&line);
    CComBSTR text;
    error->get_srcText(&text);
    CComBSTR reason;
    error->get_reason(&reason);

    TRACE("Failed to load book contents XML\n line: %d\n text: %S\n reason: %S\n",
      line,text.m_str,reason.m_str);
    return "";
  }

  // Iterate over all the contents nodes
  CComPtr<IXMLDOMNodeList> navPoints;
  doc->selectNodes(L"/ncx/navMap/navPoint",&navPoints);
  ReadContentsPoints(navPoints,TVI_ROOT);

  // Return the book title
  return StringFromXML(doc,L"/ncx/docTitle/text");
}

void BookFrame::ReadContentsPoints(IXMLDOMNodeList* navPoints, HTREEITEM parentItem)
{
  CTreeCtrl& tree = GetTreeCtrl();

  CComPtr<IXMLDOMNode> navPoint;
  while (navPoints->nextNode(&navPoint) == S_OK)
  {
    CString text = StringFromXML(navPoint,L"navLabel/text");
    CString url = StringFromXML(navPoint,L"content/@src");

    if (!text.IsEmpty() && !url.IsEmpty())
    {
      HTREEITEM item = tree.InsertItem(text,parentItem);
      tree.SetItemData(item,(DWORD_PTR)(new CString(url)));

      CComPtr<IXMLDOMNodeList> childNavPoints;
      navPoint->selectNodes(L"navPoint",&childNavPoints);
      ReadContentsPoints(childNavPoints,item);

      tree.Expand(item,TVE_EXPAND);
    }

    navPoint = NULL;
  }
}

CString BookFrame::StringFromXML(IXMLDOMNode* node, LPWSTR query)
{
  CComPtr<IXMLDOMNode> resultNode;
  if (node->selectSingleNode(query,&resultNode) == S_OK)
  {
    CComBSTR text;
    if (resultNode->get_text(&text) == S_OK)
      return CString(CStringW(text));
  }
  return "";
}

CTreeCtrl& BookFrame::GetTreeCtrl(void)
{
  CWnd* wnd = m_splitter.GetPane(0,0);
  ASSERT(wnd != NULL);
  ASSERT(wnd->IsKindOf(RUNTIME_CLASS(BookContentsView)));

  BookContentsView* treeView = (BookContentsView*)wnd;
  return treeView->GetTreeCtrl();
}

void BookFrame::DeleteItemData(HTREEITEM item)
{
  CTreeCtrl& tree = GetTreeCtrl();

  if (item != TVI_ROOT)
  {
    DWORD_PTR data = tree.GetItemData(item);
    if (data)
    {
      delete (CString*)data;
      tree.SetItemData(item,0);
    }
  }

  HTREEITEM childItem = tree.GetChildItem(item);
  while (childItem)
  {
    DeleteItemData(childItem);
    childItem = tree.GetNextItem(childItem,TVGN_NEXT);
  }
}
