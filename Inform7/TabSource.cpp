// The source tab

#include "stdafx.h"
#include "TabSource.h"
#include "Panel.h"
#include "Inform.h"
#include "Messages.h"
#include "OSLayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SOURCE_FILE "\\Source\\story.ni"

IMPLEMENT_DYNAMIC(TabSource, TabBase)

BEGIN_MESSAGE_MAP(TabSource, TabBase)
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_MESSAGE(WM_SOURCERANGE, OnSourceRange)
  ON_MESSAGE(WM_NEXTRANGE, OnNextRange)
  ON_COMMAND(ID_SOURCE_ALL, OnSourceAll)
END_MESSAGE_MAP()

TabSource::TabSource() : m_tab(true)
{
}

const char* TabSource::GetName(void)
{
  return "Source";
}

void TabSource::CreateTab(CWnd* parent)
{
  // Create the window and editing controls
  Create(parent);

  // Create the tab control
  CRect zeroRect(0,0,0,0);
  m_tab.Create(WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,zeroRect,this,0);
  m_tab.InsertItem(SrcTab_Contents,"Contents");
  m_tab.InsertItem(SrcTab_Source,"Source");

  // Create the source and contents windows
  m_source.Create(this);
  m_contents.Create(NULL,NULL,WS_CHILD|WS_CLIPCHILDREN,CRect(0,0,0,0),this,0);
  SetActiveTab(SrcTab_Source,false);
}

void TabSource::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabSource::MakeActive(TabState& state)
{
  // Make the window visible
  ShowWindow(SW_SHOW);
  m_source.ShowWindow(SW_SHOW);
  m_contents.ShowWindow(SW_HIDE);

  // Always show the source window
  SetActiveTab(SrcTab_Source,true);

  // Update the given tab state
  state.tab = Panel::Tab_Source;
}

void TabSource::MakeInactive(void)
{
  ShowWindow(SW_HIDE);
  m_source.ShowWindow(SW_HIDE);
  m_contents.ShowWindow(SW_HIDE);
}

BOOL TabSource::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Let the active tab process the command first
  switch (GetActiveTab())
  {
  case SrcTab_Contents:
    if (m_contents.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  case SrcTab_Source:
    if (m_source.GetEdit().OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  }

  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

BOOL TabSource::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  // Pick up a tab change
  if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
    SetActiveTab(GetActiveTab(),true);

  return TabBase::OnNotify(wParam, lParam, pResult);
}

void TabSource::OnPaint()
{
  CPaintDC dc(this);

  CRect client;
  GetClientRect(client);

  // Paint the area containing the buttons
  int heading = (int)GetParentFrame()->SendMessage(WM_PANEHEADING);
  dc.FillSolidRect(0,client.top,client.Width(),heading,::GetSysColor(COLOR_BTNFACE));
}

void TabSource::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);

  if (m_source.GetSafeHwnd() != 0)
  {
    CRect client;
    GetClientRect(client);

    // Call the base class to resize and get parameters
    CSize fontSize;
    int heading, h;
    SizeTab(client,fontSize,heading,h);

    // Get the dimensions of the first and last tab buttons
    CRect firstTabItem, lastTabItem;
    m_tab.GetItemRect(SrcTab_Contents,firstTabItem);
    m_tab.GetItemRect(SrcTab_Source,lastTabItem);
    int w = lastTabItem.right - firstTabItem.left + 4;

    // Resize the tab control
    CRect tabSize;
    tabSize.left = (client.Width()-w)/2;
    tabSize.right = tabSize.left+w;
    if (tabSize.left < 0)
      tabSize.left = 0;
    if (tabSize.right > client.right)
      tabSize.right = client.right;
    tabSize.top = 2;
    tabSize.bottom = client.Height()-tabSize.top-2;
    m_tab.MoveWindow(tabSize,TRUE);

    // Resize the contents and edit windows
    m_contents.MoveWindow(client,m_contents.IsWindowVisible());
    m_source.MoveWindow(client,m_source.IsWindowVisible());
  }
}

TabSource::SourceTabs TabSource::GetActiveTab(void)
{
  return (SourceTabs)m_tab.GetCurSel();
}

void TabSource::SetActiveTab(SourceTabs tab, bool focus)
{
  if (tab != No_SrcTab)
  {
    // Set the tab control
    if (GetActiveTab() != tab)
      m_tab.SetCurSel(tab);

    // Show the appropriate control
    switch (tab)
    {
    case SrcTab_Contents:
      {
        // Initialize the contents window
        CArray<SourceLexer::Heading> headings;
        m_source.GetAllHeadings(headings);
        m_contents.SetHeadings(headings,m_source.GetHeading());

        // Remove focus from the edit control before animating the contents window
        // to prevent the editing caret appearing over the animation
        if (focus)
          m_contents.SetFocus();
        m_contents.SlideIn(&m_source);
      }
      break;
    case SrcTab_Source:
      m_contents.SlideOut(&m_source);
      if (focus)
        m_source.GetEdit().SetFocus();
      break;
    }
  }
}

LRESULT TabSource::OnSourceRange(WPARAM wp, LPARAM)
{
  std::auto_ptr<SourceRange> sr((SourceRange*)wp);
  if (sr->full)
  {
    // Go to the selected heading
    m_source.ShowBetween(0,0,NULL);
    m_source.Highlight(sr->startLine,-1,false);
  }
  else
  {
    // Show only the selected section
    m_source.ShowBetween(sr->startLine,sr->endLine,&(sr->heading));
  }

  SetActiveTab(SrcTab_Source,false);
  return 0;
}

namespace {
int FindHeading(const CArray<SourceLexer::Heading>& headings, const CStringW& find, int i)
{
  while (i < headings.GetSize())
  {
    if (headings.GetAt(i).name.Compare(find) == 0)
      return i;
    i++;
  }
  return -1;
}

int FindNext(const CArray<SourceLexer::Heading>& headings, bool next, int i)
{
  SourceLexer::HeadingLevel level = headings.GetAt(i).level;
  if (next)
  {
    i++;
    while (i < headings.GetSize())
    {
      if (headings.GetAt(i).level <= level)
        return i;
      i++;
    }
  }
  else
  {
    i--;
    while (i >= 0)
    {
      if (headings.GetAt(i).level <= level)
        return i;
      i--;
    }
  }
  return -1;
}

} // unnamed namespace

LRESULT TabSource::OnNextRange(WPARAM wp, LPARAM)
{
  // Get a list of headings
  CArray<SourceLexer::Heading> headings;
  m_source.GetAllHeadings(headings);

  // Find the currently selected heading
  int idx = 0;
  const SourceHeading& heading = m_source.GetHeading();
  for (int i = (int)heading.GetSize()-1; i >= 0; i--)
  {
    idx = FindHeading(headings,heading.GetAt(i),idx);
    if (idx < 0)
      break;
  }

  // Find the next (or previous) heading at the same or higher level
  if (idx > 0)
    idx = FindNext(headings,(wp == 0),idx);

  // If no next heading, use the entire source
  if (idx < 0)
    idx = 0;

  // Create the new heading to select
  SourceHeading newHeading;
  SourceLexer::HeadingLevel level = SourceLexer::Title;
  for (int i = idx; i >= 0; i--)
  {
    const SourceLexer::Heading& head = headings.GetAt(i);
    if ((i == idx) || (head.level < level))
    {
      level = head.level;
      if (head.level > SourceLexer::Title)
        newHeading.Add(CStringW(head.name));
    }
  }
  int endIdx = FindNext(headings,true,idx);

  // Select the new heading
  int startLine = (idx > 0) ? headings.GetAt(idx).line : 0;
  int endLine = (endIdx > 0) ? headings.GetAt(endIdx).line-1 : 0;
  m_source.ShowBetween(startLine,endLine,&newHeading);
  SetActiveTab(SrcTab_Source,false);
  return 0;
}

void TabSource::OnSourceAll()
{
  // Show all source
  m_source.ShowBetween(0,0,NULL);
  SetActiveTab(SrcTab_Source,false);
}

void TabSource::OpenProject(const char* path, bool primary)
{
  // Show the whole of the source in the editor
  m_source.ShowBetween(0,0,NULL);
  if (!primary)
    return;

  // Work out the path of the source file
  CString fileName = path;
  fileName += SOURCE_FILE;

  // Open the file
  CFile sourceFile;
  if (sourceFile.Open(fileName,CFile::modeRead))
  {
    // Update the control
    m_source.GetEdit().OpenFile(&sourceFile);
  }
  else
  {
    // Tell the user
    CString msg;
    msg.Format(
      "Could not open the project's source file,\n"
      "\"%s\".\n\n"
      "Make sure that this file has not been deleted or renamed.",
      (LPCSTR)fileName);
    m_source.MessageBox(msg,INFORM_TITLE,MB_ICONWARNING|MB_OK);
  }
}

bool TabSource::SaveProject(const char* path, bool primary)
{
  CString fileName = path;
  fileName += SOURCE_FILE;
  bool saved = false;

  if (primary)
  {
    CFile sourceFile;
    if (sourceFile.Open(fileName,CFile::modeCreate|CFile::modeWrite))
      saved = m_source.GetEdit().SaveFile(&sourceFile);
  }
  else
    saved = true;

  return saved;
}

bool TabSource::IsProjectEdited(void)
{
  return m_source.GetEdit().IsEdited();
}

void TabSource::SaveToPath(const char* path)
{
  CFile sourceFile;
  if (sourceFile.Open(path,CFile::modeCreate|CFile::modeWrite))
    m_source.GetEdit().SaveFile(&sourceFile);
}

void TabSource::LoadSettings(CRegKey& key)
{
  m_source.GetEdit().LoadSettings(key,false);
  m_contents.LoadSettings(key);
}

void TabSource::SaveSettings(CRegKey& key)
{
  m_source.GetEdit().SaveSettings(key);
  m_contents.SaveSettings(key);
}

void TabSource::PrefsChanged(CRegKey& key)
{
  m_source.GetEdit().LoadSettings(key,true);
}

void TabSource::SetDocument(TabSource* master)
{
  m_source.GetEdit().SetDocument(&(master->m_source.GetEdit()));
}

bool TabSource::Highlight(const char* url, COLORREF colour)
{
  int line = 0;
  if (sscanf(url,"source:Source\\story.ni#line%d",&line) != 1)
  {
    if (sscanf(url,"source:story.ni#line%d",&line) != 1)
      return false;
  }

  if (line > 0)
  {
    SetActiveTab(SrcTab_Source,false);
    m_source.Highlight(line-1,colour,true);
    return true;
  }
  return false;
}

void TabSource::PasteCode(const wchar_t* code)
{
  SetActiveTab(SrcTab_Source,false);
  m_source.GetEdit().PasteCode(code);
}

void TabSource::SetTextSize(int size)
{
  m_source.GetEdit().SetTextSize(size);
}

void TabSource::UpdateSpellCheck(void)
{
  m_source.GetEdit().UpdateSpellCheck();
}

void TabSource::Search(LPCWSTR text, std::vector<SearchWindow::Result>& results)
{
  m_source.GetEdit().Search(text,results);
}

void TabSource::Highlight(const SearchWindow::Result& result)
{
  SetActiveTab(SrcTab_Source,false);
  m_source.Highlight(result.inSource,true);
  Panel::GetPanel(this)->SetActiveTab(Panel::Tab_Source);
}

CString TabSource::Description(void)
{
  return "source";
}

CRect TabSource::WindowRect(void)
{
  CRect r;
  m_source.GetEdit().GetWindowRect(r);
  return r;
}
