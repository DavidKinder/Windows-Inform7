// The documentation tab

#include "stdafx.h"
#include "TabDoc.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"
#include "TextFormat.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabDoc, TabBase)

BEGIN_MESSAGE_MAP(TabDoc, TabBase)
  ON_WM_SIZE()
  ON_MESSAGE(WM_USERNAVIGATE, OnUserNavigate)
  ON_REGISTERED_MESSAGE(FINDMSG, OnFindReplaceCmd)
END_MESSAGE_MAP()

const char* TabDoc::m_files[TabDoc::Number_DocTabs] =
{
  "\\Documentation\\index.html",
  "\\Documentation\\examples_alphabetical.html",
  "\\Documentation\\general_index.html"
};

TabDoc::DocData* TabDoc::m_data = NULL;
CWinThread* TabDoc::m_pThread = NULL;

TabDoc::TabDoc() : m_tab(true), m_initialised(false)
{
}

const char* TabDoc::GetName(void)
{
  return "Documentation";
}

void TabDoc::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the tab control
  CRect zeroRect(0,0,0,0);
  m_tab.Create(WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,zeroRect,this,0);
  m_tab.SendMessage(TCM_SETMINTABWIDTH,0,8);

  // Add tabs
  m_tab.InsertItem(DocTab_Home,"?H");
  m_tab.InsertItem(DocTab_Examples,"Examples");
  m_tab.InsertItem(DocTab_Index,"General Index");

  // Create the HTML control window
  if (!m_html.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,1))
  {
    TRACE("Failed to create HTML control\n");
  }
}

void TabDoc::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabDoc::MakeActive(TabState& state)
{
  if (!m_initialised)
  {
    // Show the index page
    Show(theApp.GetAppDir()+m_files[DocTab_Home]);
  }

  // Make the window visible
  ShowWindow(SW_SHOW);
  m_html.SetFocusOnContent();

  // Use and update the given tab state
  if (state.tab == Panel::Tab_Doc)
    Show(state.url);
  GetTabState(state);
}

BOOL TabDoc::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (m_html.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabDoc::CompileProject(CompileStage stage, int code)
{
  if (stage == RanNaturalInform)
  {
    if (code == 0)
    {
      // Reload the current page, in case it is generated by compilation
      m_html.Refresh();
    }
  }
}

void TabDoc::PrefsChanged(CRegKey& key)
{
  m_html.Refresh();
}

void TabDoc::Show(const char* url)
{
  m_html.Navigate(url,true);
  m_initialised = true;
  UpdateActiveTab();
}

void TabDoc::SetFocusFlag(bool set)
{
  m_html.SetFocusFlag(set);
}

static bool SortBySource(SearchWindow::Result& result1, SearchWindow::Result& result2)
{
  return result1.sourceSort < result2.sourceSort;
}

void TabDoc::Search(LPCWSTR text, std::vector<SearchWindow::Result>& results)
{
  CWaitCursor wc;

  CStringW textLow(text);
  textLow.MakeLower();

  while (true)
  {
    {
      CSingleLock lock(&(m_data->lock),TRUE);
      if (m_data->done)
        break;
    }
    ::Sleep(100);
    theApp.RunMessagePump();
  }

  if (m_pThread)
  {
    DWORD code = 0;
    if (::GetExitCodeThread(*m_pThread,&code))
    {
      if (code != STILL_ACTIVE)
      {
        delete m_pThread;
        m_pThread = NULL;
      }
    }
  }

  {
    CSingleLock lock(&(m_data->lock),TRUE);
    for (int i = 0; i < m_data->texts.GetSize(); i++)
    {
      DocText* docText = m_data->texts[i];

      // Make everything lower case
      CStringW bodyLow(docText->body);
      bodyLow.MakeLower();

      // Look for a match
      int found1 = bodyLow.Find(textLow);
      if (found1 != -1)
      {
        int found2 = found1+textLow.GetLength();

        // Create a larger range to extract the context
        int context1 = found1-8;
        if (context1 < 0)
          context1 = 0;
        int context2 = found2+32;
        if (context2 > docText->body.GetLength()-1)
          context2 = docText->body.GetLength()-1;

        // Get the surrounding text as context
        CStringW context = docText->body.Mid(context1,context2-context1);
        context.Replace(L'\n',L' ');
        context.Replace(L'\r',L' ');
        context.Replace(L'\t',L' ');

        SearchWindow::Result result;
        result.context = context;
        result.inContext.cpMin = found1-context1;
        result.inContext.cpMax = found2-context1;
        CString location;
        location.Format("%s: %s",docText->section,docText->title);
        result.sourceLocation = location;
        result.sourceSort = docText->sort;
        result.sourceFile = docText->file;
        result.colourScheme = docText->colourScheme;
        results.push_back(result);
      }
      theApp.RunMessagePump();
    }
  }

  // Sort the search results by the documentation page they are found in
  std::sort(results.begin(),results.end(),SortBySource);
}

void TabDoc::Highlight(const SearchWindow::Result& result)
{
  std::wstring search = result.context.substr(
    result.inContext.cpMin,result.inContext.cpMax-result.inContext.cpMin);
  m_html.Navigate(result.sourceFile.c_str(),false,search.c_str());
  m_initialised = true;
  UpdateActiveTab();
  Panel::GetPanel(this)->SetActiveTab(Panel::Tab_Doc);
}

CString TabDoc::Description(void)
{
  return "documentation";
}

BOOL TabDoc::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  // Pick up a tab change
  if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
  {
    DocTabs tab = GetActiveTab();
    if (tab != No_DocTab)
    {
      Show(theApp.GetAppDir()+m_files[tab]);

      TabState state;
      GetTabState(state);
      Panel::GetPanel(this)->AddToTabHistory(state);
    }
  }

  return TabBase::OnNotify(wParam, lParam, pResult);
}

void TabDoc::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);

  if (m_tab.GetSafeHwnd() == 0)
    return;

  CRect client;
  GetClientRect(client);

  // Call the base class to resize and get parameters
  CSize fontSize;
  int heading;
  SizeTab(client,fontSize,heading);

  // Get the dimensions of the first and last tab buttons
  CRect firstTabItem, lastTabItem;
  m_tab.GetItemRect(DocTab_Home,firstTabItem);
  m_tab.GetItemRect(DocTab_Index,lastTabItem);
  int w = lastTabItem.right - firstTabItem.left + 4;

  // Resize the tab control
  CRect tabSize;
  tabSize.right = client.Width();
  tabSize.left = tabSize.right-w;
  if (tabSize.left < 0)
    tabSize.left = 0;
  tabSize.top = 0;
  tabSize.bottom = client.Height()-tabSize.top;
  m_tab.MoveWindow(tabSize,TRUE);

  // Work out the display area of the tab control
  CRect tabArea = tabSize;
  m_tab.AdjustRect(FALSE,tabArea);
  client.top = tabArea.top;

  m_html.MoveWindow(client,TRUE);
}

LRESULT TabDoc::OnUserNavigate(WPARAM, LPARAM)
{
  if (IsWindowVisible())
  {
    UpdateActiveTab();

    TabState state;
    GetTabState(state);
    Panel::GetPanel(this)->AddToTabHistory(state);
  }
  return 0;
}

LRESULT TabDoc::OnFindReplaceCmd(WPARAM wParam, LPARAM lParam)
{
  return m_html.OnFindReplaceCmd(wParam,lParam);
}

TabDoc::DocTabs TabDoc::GetActiveTab(void)
{
  return (DocTabs)m_tab.GetCurSel();
}

void TabDoc::UpdateActiveTab(void)
{
  CString url = m_html.GetURL();
  int idx = No_DocTab;
  for (int i = 0; i < sizeof m_files / sizeof m_files[0]; i++)
  {
    CString check(m_files[i]);
    if (url.Find(check) > 0)
      idx = i;
    check.Replace('\\','/');
    if (url.Find(check) > 0)
      idx = i;
  }
  if (idx != GetActiveTab())
    m_tab.SetCurSel(idx);
}

void TabDoc::GetTabState(TabState& state)
{
  state.tab = Panel::Tab_Doc;
  state.url = m_html.GetURL();
}

struct Tag
{
  const wchar_t* name;
  int len;
  bool remove;
  bool cr;
};

static struct Tag tags[] =
{
  L"a",          1,false,false,
  L"B>",         2,false,false,
  L"b>",         2,false,false,
  L"blockquote",10,false,false,
  L"br",         2,false,true,
  L"font",       4,false,false,
  L"h",          1,false,false,
  L"i>",         2,false,false,
  L"img",        3,false,false,
  L"p>",         2,false,true,
  L"p ",         2,false,true,
  L"script",     6,true, false,
  L"table",      5,false,false,
  L"TABLE",      5,false,false,
  L"td",         2,false,false,
  L"TD",         2,false,false,
  L"tr",         2,false,false,
  L"TR",         2,false,false,
  L"div",        3,false,false,
  L"pre",        3,false,false,
  L"span",       4,true, false,
};

struct Literal
{
  const wchar_t* name;
  int len;
  char replace;
};

static struct Literal literals[] =
{
  L"quot;",5,'\"',
  L"nbsp;",5,' ',
  L"lt;",3,'<',
  L"gt;",3,'>',
  L"amp;",4,'&',
  L"#160;",5,' ',
};

void TabDoc::DecodeHTML(const char* filename, int scheme)
{
  // Open the file
  CFile htmlFile;
  if (htmlFile.Open(filename,CFile::modeRead) == 0)
    return;

  // Read it into memory
  CString htmlText;
  int len = (int)htmlFile.GetLength();
  htmlFile.Read(htmlText.GetBuffer(len),len);
  htmlText.ReleaseBuffer(len);

  // Convert from UTF-8 to Unicode
  CStringW html = TextFormat::UTF8ToUnicode(htmlText);

  // Get the body text
  int body1 = html.Find(L"<body");
  if (body1 == -1)
    return;
  body1 = html.Find(L">",body1);
  if (body1 == -1)
    return;
  int body2 = html.Find(L"</body>");
  if (body2 <= body1)
    return;
  CStringW bodyHtml = html.Mid(body1+1,body2-body1-1);

  // Create a DocText instance for this file
  DocText* mainDocText = new DocText();
  mainDocText->file = filename;
  mainDocText->colourScheme = scheme;
  {
    CSingleLock lock(&(m_data->lock),TRUE);
    m_data->texts.Add(mainDocText);
  }

  // Reserve space for the main text
  len = bodyHtml.GetLength();
  mainDocText->body.Preallocate(len);

  // Scan the text, removing markup
  DocText* docText = mainDocText;
  bool ignore = false;
  bool white = false;
  const wchar_t* p1 = bodyHtml;
  const wchar_t* p2 = p1+len;
  while (p1 < p2)
  {
    // Look for a markup element
    if ((*p1 == L'<') && (iswalpha(*(p1+1)) || (*(p1+1) == L'/')))
    {
      // Check for a closing markup element
      bool closing = false;
      if (*(p1+1) == L'/')
      {
        closing = true;
        p1++;
      }

      // Scan for a known markup element
      bool found = false;
      int i = 0;
      while (!found && (i < sizeof tags / sizeof tags[0]))
      {
        if (wcsncmp(p1+1,tags[i].name,tags[i].len) == 0)
          found = true;
        if (!found)
          i++;
      }
      ASSERT(found);

      // Remove the markup
      if (found && tags[i].remove)
      {
        ASSERT(!closing);

        // Remove everything until the closing element
        CStringW search;
        search.Format(L"</%s>",tags[i].name);
        p1 = wcsstr(p1,search);
        if (p1 != NULL)
          p1 += search.GetLength()-1;
        else
          p1 = p2;
      }
      else
      {
        // Remove just the element
        while ((p1 < p2) && (*p1 != L'>'))
          p1++;
      }
      ASSERT(*p1 == L'>');

      // Add a carriage return for appropriate markup
      if (found && !closing && tags[i].cr && !ignore)
        docText->AddToBody(L'\n');
      white = false;
    }
    else if ((*p1 == L'<') && (*(p1+1) == L'!'))
    {
      // Extract metadata from comments
      wchar_t meta1[256], meta2[256];
      if (swscanf(p1,L"<!-- SEARCH TITLE \"%[^\"]",meta1) == 1)
        docText->title = meta1;
      else if (swscanf(p1,L"<!-- SEARCH SECTION \"%[^\"]",meta1) == 1)
        docText->section = meta1;
      else if (swscanf(p1,L"<!-- SEARCH SORT \"%[^\"]",meta1) == 1)
        docText->sort = meta1;
      else if (swscanf(p1,L"<!-- START EXAMPLE \"%[^\"]\" \"%[^\"]",meta1,meta2) == 2)
      {
        docText = new DocText();
        docText->file = mainDocText->file + "#" + CStringA(meta2);
        docText->colourScheme = mainDocText->colourScheme;
        docText->title = "Example " + CStringA(meta1);
        docText->section = mainDocText->section;
        docText->sort = mainDocText->sort;
        docText->body.Preallocate(len/2);
        {
          CSingleLock lock(&(m_data->lock),TRUE);
          m_data->texts.Add(docText);
        }
      }
      else if (wcsncmp(p1,L"<!-- END EXAMPLE -->",20) == 0)
        docText = mainDocText;
      else if (wcsncmp(p1,L"<!-- START IGNORE ",18) == 0)
        ignore = true;
      else if (wcsncmp(p1,L"<!-- END IGNORE -->",19) == 0)
        ignore = false;

      p1 = wcsstr(p1,L"-->");
      if (p1 != NULL)
        p1 += 2;
      else
        p1 = p2;
    }
    else if (*p1 == L'&')
    {
      // Scan for a known literal
      bool found = false;
      int i = 0;
      while (!found && (i < sizeof literals / sizeof literals[0]))
      {
        if (wcsncmp(p1+1,literals[i].name,literals[i].len) == 0)
          found = true;
        if (!found)
          i++;
      }

      // Replace the literal
      if (found)
      {
        if (!ignore)
          docText->AddToBody(literals[i].replace);
        p1 += literals[i].len;
      }
      else
      {
        ASSERT(FALSE);
        if (!ignore)
          docText->AddToBody(*p1);
      }
      white = false;
    }
    else if (iswspace(*p1))
    {
      if (!white && !ignore)
        docText->AddToBody(L' ');
      white = true;
    }
    else
    {
      if (!ignore)
        docText->AddToBody(*p1);
      white = false;
    }
    p1++;
  }
}

UINT TabDoc::BackgroundDecodeThread(LPVOID)
{
  CFileFind findDoc;
  for (int i = 0; i < 2; i++)
  {
    CString findPath;
    switch (i)
    {
    case 0:
      findPath.Format("%s\\Documentation\\doc*.html",theApp.GetAppDir());
      break;
    case 1:
      findPath.Format("%s\\Documentation\\rdoc*.html",theApp.GetAppDir());
      break;
    }

    BOOL found = findDoc.FindFile(findPath);
    while (found)
    {
      // Get the filename of a documentation file
      found = findDoc.FindNextFile();

      // Extract the title and text
      DecodeHTML(findDoc.GetFilePath(),i);
    }
  }

  {
    CSingleLock lock(&(m_data->lock),TRUE);
    m_data->done = true;
  }
  TRACE("Background thread finished processing HTML for searching\n");
  return 0;
}

void TabDoc::InitInstance(void)
{
  m_data = new TabDoc::DocData();
  m_pThread = AfxBeginThread(BackgroundDecodeThread,
    NULL,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
  if (m_pThread)
  {
    m_pThread->m_bAutoDelete = FALSE;
    m_pThread->ResumeThread();
  }
}

void TabDoc::ExitInstance(void)
{
  if (m_data)
  {
    CSingleLock lock(&(m_data->lock),TRUE);
    if (m_data->done)
    {
      for (int i = 0; i < m_data->texts.GetSize(); i++)
        delete m_data->texts[i];
      m_data->texts.RemoveAll();
    }
  }

  if (m_pThread)
  {
    DWORD code = 0;
    if (::GetExitCodeThread(*m_pThread,&code))
    {
      if (code == STILL_ACTIVE)
      {
        // Stop the thread as we are shutting down
        m_pThread->SuspendThread();
      }
      else
      {
        delete m_pThread;
        m_pThread = NULL;
      }
    }
  }

  if (m_data)
  {
    delete m_data;
    m_data = NULL;
  }
}

TabDoc::DocText::DocText()
{
  colourScheme = 0;
}

void TabDoc::DocText::AddToBody(WCHAR ch)
{
  body += ch;
}
