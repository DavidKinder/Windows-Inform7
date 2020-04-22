#include "stdafx.h"
#include "FindInFiles.h"
#include "ProjectFrame.h"
#include "OSLayer.h"
#include "RichEdit.h"
#include "TextFormat.h"
#include "resource.h"

#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FIND_HISTORY_LENGTH 30

// Implementation of IEnumString for auto-completion
class FindEnumString : public CCmdTarget
{
public:
  FindEnumString(FindInFiles* parent) : m_parent(parent), m_iter(0)
  {
  }

  DECLARE_INTERFACE_MAP()

  BEGIN_INTERFACE_PART(EnumString, IEnumString)
    HRESULT Next(ULONG, LPOLESTR*, ULONG*);
    HRESULT Skip(ULONG);
    HRESULT Reset(void);
    HRESULT Clone(IEnumString**);
  END_INTERFACE_PART(EnumString)

private:
  FindInFiles* m_parent;
  int m_iter;
};

BEGIN_INTERFACE_MAP(FindEnumString, CCmdTarget)
  INTERFACE_PART(FindEnumString, IID_IEnumString, EnumString)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) FindEnumString::XEnumString::AddRef()
{
  METHOD_PROLOGUE(FindEnumString, EnumString)
  return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) FindEnumString::XEnumString::Release()
{
  METHOD_PROLOGUE(FindEnumString, EnumString)
  return pThis->ExternalRelease();
}

STDMETHODIMP FindEnumString::XEnumString::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE(FindEnumString, EnumString)
  return (HRESULT)pThis->InternalQueryInterface(&iid,ppvObj);
}

HRESULT FindEnumString::XEnumString::Next(ULONG count, LPOLESTR* strs, ULONG* pDone)
{
  METHOD_PROLOGUE(FindEnumString, EnumString)

  ULONG done = 0;
  if (pDone)
    *pDone = done;

  for (ULONG i = 0; i < count; i++)
  {
    LPCWSTR str = pThis->m_parent->GetAutoComplete(pThis->m_iter);
    if (str)
    {
      strs[done] = (LPOLESTR)::CoTaskMemAlloc(sizeof(WCHAR) * (wcslen(str)+1));
      wcscpy(strs[done],str);

      pThis->m_iter++;
      done++;
      if (pDone)
        *pDone = done;
    }
    else
      return S_FALSE;
  }
  return S_OK;
}

HRESULT FindEnumString::XEnumString::Skip(ULONG count)
{
  METHOD_PROLOGUE(FindEnumString, EnumString)
  pThis->m_iter += count;
  return S_OK;
}

HRESULT FindEnumString::XEnumString::Reset(void)
{
  METHOD_PROLOGUE(FindEnumString, EnumString)
  pThis->m_iter = 0;
  return S_OK;
}

HRESULT FindEnumString::XEnumString::Clone(IEnumString**)
{
  return E_FAIL;
}

FindResultsCtrl::FindResultsCtrl()
{
}

BEGIN_MESSAGE_MAP(FindResultsCtrl, CListCtrl)
  ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHeaderDividerDblClick)
  ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHeaderDividerDblClick)
END_MESSAGE_MAP()

void FindResultsCtrl::OnHeaderDividerDblClick(NMHDR* pNotifyStruct, LRESULT* result)
{
  NMHEADER* hdr = (NMHEADER*)pNotifyStruct;
  if (hdr->iItem == 0)
    GetParent()->SendMessage(WM_RESIZERESULTS);
  else
    Default();
}

// The one and only FindInFiles object
FindInFiles theFinder;

BEGIN_MESSAGE_MAP(FindInFiles, I7BaseDialog)
  ON_WM_CLOSE()
  ON_WM_DRAWITEM()
  ON_WM_ERASEBKGND()
  ON_WM_GETMINMAXINFO()
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_FIND_ALL, OnFindAll)
  ON_CBN_SELCHANGE(IDC_FIND_RULE, OnChangeFindRule)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_RESULTS, OnResultsDraw)
  ON_MESSAGE(WM_RESIZERESULTS, OnResultsResize)
END_MESSAGE_MAP()

FindInFiles::FindInFiles() : I7BaseDialog(IDD_FIND_FILES)
{
  m_project = NULL;

  m_lookSource = TRUE;
  m_lookExts = TRUE;
  m_lookDocPhrases = TRUE;
  m_lookDocMain = TRUE;
  m_lookDocExamples = TRUE;

  m_ignoreCase = TRUE;
  m_findRule = 0;
}

void FindInFiles::Show(ProjectFrame* project)
{
  if (GetSafeHwnd() == 0)
  {
    m_richText = new RichDrawText();
    Create(m_lpszTemplateName,CWnd::GetDesktopWindow());
    theApp.SetIcon(this);

    bool placementSet = false;
    CRegKey registryKey;
    if (registryKey.Create(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW) == ERROR_SUCCESS)
    {
      // Restore the window state
      WINDOWPLACEMENT place;
      ULONG len = sizeof WINDOWPLACEMENT;
      if (registryKey.QueryBinaryValue("Find in Files Placement",&place,&len) == ERROR_SUCCESS)
        SetWindowPlacement(&place);
      placementSet = true;
    }
    if (!placementSet)
      CenterWindow(project);
  }

  if (GetSafeHwnd() != 0)
  {
    m_project = project;
    ShowWindow(SW_SHOW);
    GetDlgItem(IDC_FIND)->SetFocus();
  }
}

void FindInFiles::Hide(ProjectFrame* project)
{
  if (m_project == project)
  {
    ShowWindow(SW_HIDE);
    m_project = NULL;
  }
}

void FindInFiles::FindInSource(LPCWSTR text)
{
  UpdateData(TRUE);
  m_lookSource = TRUE;
  m_lookExts = FALSE;
  m_lookDocPhrases = FALSE;
  m_lookDocMain = FALSE;
  m_lookDocExamples = FALSE;
  m_ignoreCase = TRUE;
  m_findRule = 0;
  m_findText = text;
  UpdateData(FALSE);
  PostMessage(WM_COMMAND,IDC_FIND_ALL);
}

void FindInFiles::FindInDocs(LPCWSTR text)
{
  UpdateData(TRUE);
  m_lookSource = FALSE;
  m_lookExts = FALSE;
  m_lookDocPhrases = TRUE;
  m_lookDocMain = TRUE;
  m_lookDocExamples = TRUE;
  m_ignoreCase = TRUE;
  m_findRule = 0;
  m_findText = text;
  UpdateData(FALSE);
  PostMessage(WM_COMMAND,IDC_FIND_ALL);
}

void FindInFiles::InitInstance(void)
{
  try
  {
    // Read the serialized auto complete history from the registry
    CRegKey registryKey;
    if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_READ) == ERROR_SUCCESS)
    {
      ULONG historyLen = 0;
      if (registryKey.QueryBinaryValue("Find in Files History",NULL,&historyLen) == ERROR_SUCCESS)
      {
        BYTE* historyPtr = new BYTE[historyLen];
        if (registryKey.QueryBinaryValue("Find in Files History",historyPtr,&historyLen) == ERROR_SUCCESS)
        {
          CMemFile historyFile(historyPtr,historyLen);
          CArchive historyArchive(&historyFile,CArchive::load);
          INT_PTR historyCount = 0;
          historyArchive >> historyCount;
          for (INT_PTR i = 0; i < historyCount; i++)
          {
            CStringW historyItem;
            historyArchive >> historyItem;
            m_findHistory.AddTail(historyItem);
          }
        }
      }
    }
  }
  catch (CException* ex)
  {
    ex->Delete();
  }
}

void FindInFiles::ExitInstance(void)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
  {
    // Save the window state
    if (GetSafeHwnd() != 0)
    {
      WINDOWPLACEMENT place;
      place.length = sizeof place;
      GetWindowPlacement(&place);
      registryKey.SetBinaryValue("Find in Files Placement",&place,sizeof WINDOWPLACEMENT);
    }

    // Save the auto complete history
    try
    {
      if (m_findHistory.GetCount() > 0)
      {
        // Write the list of the auto complete history to a memory file
        CMemFile historyFile;
        CArchive historyArchive(&historyFile,CArchive::store);
        historyArchive << m_findHistory.GetCount();
        POSITION pos = m_findHistory.GetHeadPosition();
        for (INT_PTR i = 0; i < m_findHistory.GetCount(); i++)
          historyArchive << m_findHistory.GetNext(pos);
        historyArchive.Close();
 
        // Write the above buffer to the registry
        ULONG historyLen = (ULONG)historyFile.GetLength();
        void* historyPtr = historyFile.Detach();
        registryKey.SetBinaryValue("Find in Files History",historyPtr,historyLen);
        free(historyPtr);
      }
    }
    catch (CException* ex)
    {
      ex->Delete();
    }
  }

  // Close the window
  if (GetSafeHwnd() != 0)
    DestroyWindow();
  delete m_richText;
  m_richText = NULL;
  m_project = NULL;
}

LPCWSTR FindInFiles::GetAutoComplete(int index)
{
  if (index < m_findHistory.GetSize())
  {
    POSITION pos = m_findHistory.FindIndex(index);
    if (pos != NULL)
      return m_findHistory.GetAt(pos).GetString();
  }
  return NULL;
}

void FindInFiles::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FIND, m_find);
  DDX_Control(pDX, IDC_REGEX_HELP, m_regexHelp);
  DDX_TextW(pDX, IDC_FIND, m_findText);
  DDX_Check(pDX,IDC_LOOK_SOURCE,m_lookSource);
  DDX_Check(pDX,IDC_LOOK_EXTENSIONS,m_lookExts);
  DDX_Check(pDX,IDC_LOOK_DOC_PHRASES,m_lookDocPhrases);
  DDX_Check(pDX,IDC_LOOK_DOC_MAIN,m_lookDocMain);
  DDX_Check(pDX,IDC_LOOK_DOC_EXAMPLES,m_lookDocExamples);
  DDX_Check(pDX,IDC_IGNORE_CASE,m_ignoreCase);
  DDX_CBIndex(pDX,IDC_FIND_RULE,m_findRule);
  DDX_Control(pDX, IDC_RESULTS, m_resultsList);
}

BOOL FindInFiles::OnInitDialog()
{
  if (I7BaseDialog::OnInitDialog())
  {
    // Initialize auto-completion for the find string
    if (FAILED(m_findAutoComplete.CoCreateInstance(CLSID_AutoComplete)))
      return FALSE;
    FindEnumString* fes = new FindEnumString(this);
    CComQIPtr<IEnumString> ies(fes->GetInterface(&IID_IEnumString));
    m_findAutoComplete->Init(m_find.GetSafeHwnd(),ies,NULL,NULL);
    m_findAutoComplete->SetOptions(ACO_AUTOSUGGEST);
    fes->ExternalRelease();

    m_resultsList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
    m_resultsList.InsertColumn(0,"Result");
    m_resultsList.InsertColumn(1,"Document");
    m_resultsList.InsertColumn(2,"Type");

    CRect windowRect, resultsRect;
    GetWindowRect(windowRect);
    m_minSize = windowRect.Size();
    m_resultsList.GetWindowRect(resultsRect);
    m_resultsBottomRight = windowRect.BottomRight() - resultsRect.BottomRight();

    ScreenToClient(resultsRect);
    m_regexHelp.MoveWindow(resultsRect);
    return TRUE;
  }
  return FALSE;
}

void FindInFiles::OnCancel()
{
  SendMessage(WM_CLOSE);
}

void FindInFiles::OnClose()
{
  ShowWindow(SW_HIDE);
  if (m_project)
  {
    m_project->SetActiveWindow();
    m_project = NULL;
  }
}

void FindInFiles::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di)
{
  if (nIDCtl == IDC_REGEX_HELP)
  {
    CRect helpRect(di->rcItem);
    HANDLE pb = 0;
    CDC* dc = theOS.BeginBufferedPaint(di->hDC,helpRect,BPBF_COMPATIBLEBITMAP,&pb);
    dc->FillSolidRect(helpRect,::GetSysColor(COLOR_BTNFACE));

    const char* text1 =
      "\\b Characters\\b0\\par\\par"
      " .\\tab Any character\\par"
      " \\\\b\\tab Word boundary\\par"
      " \\\\d\\tab Any digit\\par"
      " \\\\n\\tab Newline\\par"
      " \\\\s\\tab Whitespace\\par"
      " \\\\S\\tab Non-whitespace\\par"
      " \\\\t\\tab Tab\\par"
      " [A-Z]\\tab Character ranges\\par";
    CString rtfText;
    rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
      theApp.GetFontSize(InformApp::FontSystem)*2,text1);
    m_richText->SetTextRTF(rtfText);
    m_richText->DrawText(*dc,helpRect);

    const char* text2 =
      "\\b Patterns\\b0\\par\\par"
      " x?\\tab 0 or 1 of 'x'\\par"
      " x*\\tab 0 or more of 'x'\\par"
      " x+\\tab 1 or more of 'x'\\par"
      " x\\{2\\}\\tab Exactly 2 of 'x'\\par"
      " x\\{2,\\}\\tab 2 or more of 'x'\\par"
      " x\\{,2\\}\\tab 0-2 of 'x'\\par"
      " x\\{2,4\\}\\tab 2-4 of 'x'\\par";
    rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
      theApp.GetFontSize(InformApp::FontSystem)*2,text2);
    m_richText->SetTextRTF(rtfText);
    CRect textRect;
    textRect.SetRectEmpty();
    textRect.right = helpRect.Width();
    m_richText->SizeText(*dc,textRect);
    textRect.MoveToX((helpRect.right - textRect.Width())/2);
    m_richText->DrawText(*dc,textRect);

    const char* text3 =
      "\\b Expressions\\b0\\par\\par"
      " (x)\\tab Group expression\\par"
      " x|y\\tab x or y\\par"
      " ^\\tab Start of the line\\par"
      " $\\tab End of the line\\par";
    rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
      theApp.GetFontSize(InformApp::FontSystem)*2,text3);
    m_richText->SetTextRTF(rtfText);
    textRect.SetRectEmpty();
    textRect.right = helpRect.Width();
    m_richText->SizeText(*dc,textRect);
    textRect.MoveToX(helpRect.right - textRect.Width());
    m_richText->DrawText(*dc,textRect);

    theOS.EndBufferedPaint(pb,TRUE);
  }
  else
    CWnd::OnDrawItem(nIDCtl,di);
}

BOOL FindInFiles::OnEraseBkgnd(CDC* dc)
{
  EraseWithGripper(dc);
  return TRUE;
}

void FindInFiles::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  I7BaseDialog::OnGetMinMaxInfo(lpMMI);
  lpMMI->ptMinTrackSize.x = m_minSize.cx;
  lpMMI->ptMinTrackSize.y = m_minSize.cy;
}

void FindInFiles::OnSize(UINT nType, int cx, int cy)
{
  I7BaseDialog::OnSize(nType,cx,cy);

  if (m_resultsList.GetSafeHwnd() != 0)
  {
    CRect windowRect, resultsRect;
    GetWindowRect(windowRect);
    m_resultsList.GetWindowRect(resultsRect);
    resultsRect.right = windowRect.right - m_resultsBottomRight.cx;
    resultsRect.bottom = windowRect.bottom - m_resultsBottomRight.cy;
    ScreenToClient(resultsRect);
    m_resultsList.MoveWindow(resultsRect);
  }

  // Make sure that the sizing gripper is redrawn
  Invalidate();
}

void FindInFiles::OnFindAll()
{
  ASSERT(m_project);
  if (m_project == NULL)
    return;

  UpdateData(TRUE);
  if (m_findText.IsEmpty())
    return;

  // Update the find history
  POSITION pos = m_findHistory.Find(m_findText);
  if (pos == NULL)
    m_findHistory.AddTail(m_findText);
  else if (pos != m_findHistory.GetTailPosition())
  {
    m_findHistory.RemoveAt(pos);
    m_findHistory.AddTail(m_findText);
  }
  while (m_findHistory.GetSize() > FIND_HISTORY_LENGTH)
    m_findHistory.RemoveHead();

  // Update the find history dropdown
  CComQIPtr<IAutoCompleteDropDown> dropdown(m_findAutoComplete);
  if (dropdown)
    dropdown->ResetEnumerator();

  // Search for the text
  CWaitCursor wc;
  m_results.clear();
  if (m_lookSource)
    Find(m_project->GetSource(),m_project->GetDisplayName(false),"Source");

  // Update the results
  m_resultsList.DeleteAllItems();
  for (int i = 0; i < (int)m_results.size(); i++)
  {
    m_resultsList.InsertItem(i,"");
    m_resultsList.SetItemText(i,1,m_results[i].sourceDocument);
    m_resultsList.SetItemText(i,2,m_results[i].sourceType);
  }

  // Resize the results columns
  CRect resultsRect;
  m_resultsList.GetWindowRect(resultsRect);
  m_resultsList.SetColumnWidth(0,(int)(0.65 * resultsRect.Width()));
  m_resultsList.SetColumnWidth(1,(int)(0.2 * resultsRect.Width()));
  m_resultsList.SetColumnWidth(2,LVSCW_AUTOSIZE_USEHEADER);

  // Update the found status message and what is visible
  CString foundMsg;
  switch (m_results.size())
  {
  case 0:
    foundMsg = "No matches were found";
    m_resultsList.ModifyStyle(WS_VISIBLE,0);
    break;
  case 1:
    foundMsg = "Found 1 result:";
    m_resultsList.ModifyStyle(0,WS_VISIBLE);
    m_resultsList.SetFocus();
    break;
  default:
    foundMsg.Format("Found %d results:",m_results.size());
    m_resultsList.ModifyStyle(0,WS_VISIBLE);
    m_resultsList.SetFocus();
    break;
  }
  GetDlgItem(IDC_FOUND)->SetWindowText(foundMsg);
  m_regexHelp.ModifyStyle(WS_VISIBLE,0);
  Invalidate();
}

void FindInFiles::OnChangeFindRule()
{
  UpdateData(TRUE);
  if (m_findRule == 3)
  {
    m_resultsList.ModifyStyle(WS_VISIBLE,0);
    m_regexHelp.ModifyStyle(0,WS_VISIBLE);
    GetDlgItem(IDC_FOUND)->SetWindowText("");
    Invalidate();
  }
  else
  {
    if (m_regexHelp.IsWindowVisible())
    {
      m_regexHelp.ModifyStyle(WS_VISIBLE,0);
      Invalidate();
    }
  }
}

void FindInFiles::OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result)
{
  // Default to letting Windows draw the control
  *result = CDRF_DODEFAULT;

  // Work out where we are in the drawing process
  NMLVCUSTOMDRAW* custom = (NMLVCUSTOMDRAW*)pNotifyStruct;
  switch (custom->nmcd.dwDrawStage)
  {
  case CDDS_PREPAINT:
    // Tell us when an item is drawn
    *result = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    // Tell us when a sub-item is drawn
    *result = CDRF_NOTIFYSUBITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
  {
    // Work out the background colour
    int item = (int)custom->nmcd.dwItemSpec;
    COLORREF backColour = theApp.GetColour(m_results[item].colour);
    if ((item % 2) != 0)
      backColour = Darken(backColour);

    // Get if the item is selected
    bool selected = false;
    if (GetFocus() == &m_resultsList)
    {
      if (item == m_resultsList.GetNextItem(-1,LVNI_SELECTED))
        selected = true;
    }

    // Get the bounding rectangle for drawing the text
    CRect rect;
    m_resultsList.GetSubItemRect(item,custom->iSubItem,LVIR_LABEL,rect);
    CRect textRect = rect;
    textRect.DeflateRect(2,0);

    // Set up the device context
    HANDLE pb = 0;
    CDC* dc = theOS.BeginBufferedPaint(custom->nmcd.hdc,rect,BPBF_COMPATIBLEBITMAP,&pb);
    dc->SetTextColor(::GetSysColor(selected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
    dc->SetBkMode(TRANSPARENT);
    CFont* oldFont = dc->SelectObject(m_resultsList.GetFont());

    // Draw the background
    dc->FillSolidRect(rect,selected ? ::GetSysColor(COLOR_HIGHLIGHT) : backColour);

    // Special case painting of the first column
    if (custom->iSubItem == 0)
    {
      // Create a bold font
      LOGFONT logFont;
      m_resultsList.GetFont()->GetLogFont(&logFont);
      logFont.lfWeight = FW_BOLD;
      CFont boldFont;
      boldFont.CreateFontIndirect(&logFont);

      // Get the text
      const CStringW& text = m_results[item].context;
      int high1 = m_results[item].inContext.cpMin;
      int high2 = m_results[item].inContext.cpMax;

      // Draw the text
      DrawText(dc,text.GetString(),high1,textRect,DT_VCENTER|DT_NOPREFIX);
      dc->SelectObject(&boldFont);
      DrawText(dc,text.GetString()+high1,high2-high1,textRect,DT_VCENTER|DT_NOPREFIX);
      dc->SelectObject(m_resultsList.GetFont());
      DrawText(dc,text.GetString()+high2,text.GetLength()-high2,textRect,
        DT_VCENTER|DT_WORD_ELLIPSIS|DT_NOPREFIX);
    }
    else
    {
      CString text = m_resultsList.GetItemText(item,custom->iSubItem);
      dc->DrawText(text,textRect,DT_VCENTER|DT_SINGLELINE|DT_WORD_ELLIPSIS|DT_NOPREFIX);
    }

    dc->SelectObject(oldFont);
    theOS.EndBufferedPaint(pb,TRUE);
    *result = CDRF_SKIPDEFAULT;
  }
  break;
  }
}

LRESULT FindInFiles::OnResultsResize(WPARAM, LPARAM)
{
  // Set up a device context
  CDC* dc = m_resultsList.GetDC();
  CFont* oldFont = dc->SelectObject(m_resultsList.GetFont());

  // Create a bold font
  LOGFONT logFont;
  m_resultsList.GetFont()->GetLogFont(&logFont);
  logFont.lfWeight = FW_BOLD;
  CFont boldFont;
  boldFont.CreateFontIndirect(&logFont);

  int colWidth = 0;
  for (int i = 0; i < (int)m_results.size(); i++)
  {
    // Get the text
    const CStringW& text = m_results[i].context;
    int high1 = m_results[i].inContext.cpMin;
    int high2 = m_results[i].inContext.cpMax;

    // Measure the text
    dc->SelectObject(m_resultsList.GetFont());
    int width = MeasureText(dc,text.GetString(),high1);
    dc->SelectObject(&boldFont);
    width += MeasureText(dc,text.GetString()+high1,high2-high1);
    dc->SelectObject(m_resultsList.GetFont());
    width += MeasureText(dc,text.GetString()+high2,text.GetLength()-high2);

    if (width > colWidth)
      colWidth = width;
  }

  // Free the device context
  dc->SelectObject(oldFont);
  m_resultsList.ReleaseDC(dc);

  m_resultsList.SetColumnWidth(0,colWidth+8);
  return 0;
}

void FindInFiles::Find(const CString& text, const CString& doc, const char* type)
{
  try
  {
    // Set up a regular expression
    CString findUtf = TextFormat::UnicodeToUTF8(m_findText);
    std::regex::flag_type flags = std::regex::ECMAScript;
    if (m_ignoreCase)
      flags |= std::regex::icase;
    if (m_findRule != 3) // Not "Regular expression"
    {
      // Escape any characters with a special meaning in regular expressions
      for (int i = 0; i < findUtf.GetLength(); i++)
      {
        if (strchr(".^$|()[]{}*+?\\",findUtf.GetAt(i)))
        {
          findUtf.Insert(i,'\\');
          i++;
        }
      }      
    }
    switch (m_findRule)
    {
    case 1: // "Starts with"
      findUtf.Insert(0,"\\b");
      break;
    case 2: // "Full word"
      findUtf.Insert(0,"\\b");
      findUtf.Append("\\b");
      break;
    }
    std::regex regexp;
    regexp.assign(findUtf,flags);

    // Search for the text
    auto regexBegin = std::cregex_iterator(text.GetString(),text.GetString()+text.GetLength(),regexp);
    for (auto it = regexBegin; it != std::cregex_iterator(); ++it)
    {
      if (it->length() <= 0)
        return;

      int matchStart = (int)it->position();
      int matchEnd = (int)(it->position() + it->length());

      // Get the surrounding text as context. Note that we get the leading, matching and
      // trailing texts separately so that we can count the number of Unicode characters in each.
      int lineStart = FindLineStart(text,matchStart);
      int lineEnd = FindLineEnd(text,matchEnd);
      CStringW leading = GetMatchRange(text,lineStart,matchStart);
      CStringW match = GetMatchRange(text,matchStart,matchEnd);
      CStringW trailing = GetMatchRange(text,matchEnd,lineEnd);
      CStringW context = leading + match + trailing;
      context.Replace(L'\n',L' ');
      context.Replace(L'\r',L' ');
      context.Replace(L'\t',L' ');

      // Store the found result
      FindResult result;
      result.context = context;
      result.inContext.cpMin = leading.GetLength();
      result.inContext.cpMax = leading.GetLength() + match.GetLength();
      result.sourceDocument = doc;
      result.sourceType = type;
      result.inSource.cpMin = matchStart;
      result.inSource.cpMax = matchEnd;
      m_results.push_back(result);
    }
  }
  catch (std::regex_error& ex)
  {
    const char* msg = NULL;
    switch (ex.code())
    {
    case std::regex_constants::error_collate:
      msg = "The find expression contained an invalid collating element name.";
      break;
    case std::regex_constants::error_ctype:
      msg = "The find expression contained an invalid character class name.";
      break;
    case std::regex_constants::error_escape:
      msg = "The find expression contained an invalid escaped character, or a trailing escape.";
      break;
    case std::regex_constants::error_backref:
      msg = "The find expression contained an invalid back reference.";
      break;
    case std::regex_constants::error_brack:
      msg = "The find expression contained mismatched [ and ].";
      break;
    case std::regex_constants::error_paren:
      msg = "The find expression contained mismatched ( and ).";
      break;
    case std::regex_constants::error_brace:
      msg = "The find expression contained mismatched { and }.";
      break;
    case std::regex_constants::error_badbrace:
      msg = "The find expression contained an invalid range in a { expression }.";
      break;
    case std::regex_constants::error_range:
      msg = "The find expression contained an invalid character range, such as [b-a].";
      break;
    case std::regex_constants::error_space:
      msg = "Insufficient memory to process the find expression.";
      break;
    case std::regex_constants::error_badrepeat:
      msg = "One of *?+{ was not preceded by a valid regular expression.";
      break;
    case std::regex_constants::error_complexity:
      msg = "The complexity of an attempted match against a regular expression was too much.";
      break;
    case std::regex_constants::error_stack:
      msg = "There was insufficient memory to determine whether the regular expression could match the specified character sequence.";
      break;
    case std::regex_constants::error_parse:
      msg = "Failed to parse find expression.";
      break;
    case std::regex_constants::error_syntax:
      msg = "Syntax error in find expression.";
      break;
    default:
      msg = "Error in find expression.";
      break;
    }
    MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
  }
}

int FindInFiles::FindLineStart(const CString& text, int pos)
{
  while (pos > 0)
  {
    char c = text.GetAt(pos-1);
    if ((c == '\n') || (c == '\r'))
      return pos;
    pos--;
  }
  return pos;
}

int FindInFiles::FindLineEnd(const CString& text, int pos)
{
  int len = text.GetLength();
  while (pos < len)
  {
    char c = text.GetAt(pos);
    if ((c == '\n') || (c == '\r'))
      return pos;
    pos++;
  }
  return pos;
}

CStringW FindInFiles::GetMatchRange(const CString& text, int start, int end)
{
  return TextFormat::UTF8ToUnicode(text.Mid(start,end-start));
}

void FindInFiles::DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format)
{
  if (length > 0)
  {
    theOS.DrawText(dc,text,length,rect,format|DT_SINGLELINE);

    CRect measure(rect);
    theOS.DrawText(dc,text,length,measure,format|DT_SINGLELINE|DT_CALCRECT);
    rect.left += measure.Width();
  }
}

int FindInFiles::MeasureText(CDC* dc, LPCWSTR text, int length)
{
  if (length > 0)
  {
    SIZE textSize;
    ::GetTextExtentPoint32W(dc->GetSafeHdc(),text,length,&textSize);
    return textSize.cx;
  }
  return 0;
}

COLORREF FindInFiles::Darken(COLORREF colour)
{
  BYTE r = GetRValue(colour);
  BYTE g = GetGValue(colour);
  BYTE b = GetBValue(colour);
  r = (BYTE)(r * 0.9333);
  g = (BYTE)(g * 0.9333);
  b = (BYTE)(b * 0.9333);
  return RGB(r,g,b);
}

FindInFiles::FindResult::FindResult()
{
  inContext.cpMin = 0;
  inContext.cpMax = 0;
  inSource.cpMin = 0;
  inSource.cpMax = 0;
  colour = InformApp::ColourBack;
}
