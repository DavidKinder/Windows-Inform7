#include "stdafx.h"
#include "FindInFiles.h"
#include "ExtensionFrame.h"
#include "ProjectFrame.h"
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
  FindEnumString() : m_iter(0)
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
    LPCWSTR str = FindInFiles::GetAutoComplete(pThis->m_iter);
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

BEGIN_MESSAGE_MAP(FindInFiles, I7BaseDialog)
  ON_WM_CLOSE()
  ON_WM_DESTROY()
  ON_WM_DRAWITEM()
  ON_WM_ERASEBKGND()
  ON_WM_GETMINMAXINFO()
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_FIND_ALL, OnFindAll)
  ON_CBN_SELCHANGE(IDC_FIND_RULE, OnChangeFindRule)
  ON_EN_CHANGE(IDC_FIND, OnChangeFindText)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_RESULTS, OnResultsDraw)
  ON_NOTIFY(NM_DBLCLK, IDC_RESULTS, OnResultsSelect)
  ON_NOTIFY(NM_RETURN, IDC_RESULTS, OnResultsSelect)
  ON_MESSAGE(WM_RESIZERESULTS, OnResultsResize)
END_MESSAGE_MAP()

CList<CStringW> FindInFiles::m_findHistory;

FindInFiles::FindInFiles(ProjectFrame* project)
  : I7BaseDialog(IDD_FIND_FILES), m_project(project)
{
  m_lookSource = TRUE;
  m_lookExts = TRUE;
  m_lookDocPhrases = TRUE;
  m_lookDocMain = TRUE;
  m_lookDocCode = TRUE;

  m_ignoreCase = TRUE;
  m_findRule = 0;
  m_richText = NULL;
}

void FindInFiles::Show(void)
{
  if (GetSafeHwnd() == 0)
  {
    if (m_richText == NULL)
      m_richText = new RichDrawText();
    Create(m_lpszTemplateName,m_project);
    theApp.SetIcon(this);

    bool placementSet = false;
    CRegKey registryKey;
    if (registryKey.Create(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW) == ERROR_SUCCESS)
    {
      // Restore the window state
      WINDOWPLACEMENT place;
      ULONG len = sizeof WINDOWPLACEMENT;
      if (registryKey.QueryBinaryValue("Find in Files Placement",&place,&len) == ERROR_SUCCESS)
      {
        SetWindowPlacement(&place);
        placementSet = true;
      }
    }
    if (!placementSet)
      CenterWindow(m_project);
  }

  if (GetSafeHwnd() != 0)
  {
    UpdateData(TRUE);

    ShowWindow(SW_SHOW);
    GetDlgItem(IDC_FIND_ALL)->EnableWindow(!m_findText.IsEmpty());
    GetDlgItem(IDC_FIND)->SetFocus();
  }
}

void FindInFiles::Hide(void)
{
  if (GetSafeHwnd() != 0)
    ShowWindow(SW_HIDE);
}

void FindInFiles::FindInSource(LPCWSTR text)
{
  UpdateData(TRUE);
  m_lookSource = TRUE;
  m_lookExts = FALSE;
  m_lookDocPhrases = FALSE;
  m_lookDocMain = FALSE;
  m_lookDocCode = FALSE;
  m_ignoreCase = TRUE;
  m_findRule = 0;
  m_findText = text;
  UpdateData(FALSE);
  GetDlgItem(IDC_FIND_ALL)->EnableWindow(!m_findText.IsEmpty());
  PostMessage(WM_COMMAND,IDC_FIND_ALL);
}

void FindInFiles::FindInDocs(LPCWSTR text)
{
  UpdateData(TRUE);
  m_lookSource = FALSE;
  m_lookExts = FALSE;
  m_lookDocPhrases = TRUE;
  m_lookDocMain = TRUE;
  m_lookDocCode = TRUE;
  m_ignoreCase = TRUE;
  m_findRule = 0;
  m_findText = text;
  UpdateData(FALSE);
  GetDlgItem(IDC_FIND_ALL)->EnableWindow(!m_findText.IsEmpty());
  PostMessage(WM_COMMAND,IDC_FIND_ALL);
}

void FindInFiles::InitInstance(void)
{
  // Start decoding the text in the documentation
  m_data = new FindInFiles::DocData();
  m_pThread = AfxBeginThread(BackgroundDecodeThread,NULL,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
  if (m_pThread)
  {
    m_pThread->m_bAutoDelete = FALSE;
    m_pThread->ResumeThread();
  }

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
        delete[] historyPtr;
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
  // Save the auto complete history
  try
  {
    CRegKey registryKey;
    if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
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
  }
  catch (CException* ex)
  {
    ex->Delete();
  }

  // Free the memory for the text extracted from the documentation if the
  // background thread is finished.
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

  // If the background thread is still running, suspend it, otherwise clean up.
  if (m_pThread)
  {
    DWORD code = 0;
    if (::GetExitCodeThread(*m_pThread,&code))
    {
      if (code == STILL_ACTIVE)
        m_pThread->SuspendThread();
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
  DDX_Control(pDX, IDC_REGEX_HELP, m_regexHelp);
  DDX_TextW(pDX, IDC_FIND, m_findText);
  DDX_Check(pDX,IDC_LOOK_SOURCE,m_lookSource);
  DDX_Check(pDX,IDC_LOOK_EXTENSIONS,m_lookExts);
  DDX_Check(pDX,IDC_LOOK_DOC_PHRASES,m_lookDocPhrases);
  DDX_Check(pDX,IDC_LOOK_DOC_MAIN,m_lookDocMain);
  DDX_Check(pDX,IDC_LOOK_DOC_CODE,m_lookDocCode);
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
    FindEnumString* fes = new FindEnumString();
    CComQIPtr<IEnumString> ies(fes->GetInterface(&IID_IEnumString));
    m_findAutoComplete->Init(GetDlgItem(IDC_FIND)->GetSafeHwnd(),ies,NULL,NULL);
    m_findAutoComplete->SetOptions(ACO_AUTOSUGGEST);
    fes->ExternalRelease();

    m_resultsList.SetFont(theApp.GetFont(InformApp::FontSmall));
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

BOOL FindInFiles::OnCommand(WPARAM wParam, LPARAM lParam)
{
  // Do nothing if an operation is in progress
  if (theApp.IsWaitCursor())
    return TRUE;
  return I7BaseDialog::OnCommand(wParam,lParam);
}

void FindInFiles::OnCancel()
{
  SendMessage(WM_CLOSE);
}

void FindInFiles::OnClose()
{
  ShowWindow(SW_HIDE);
  m_project->SetActiveWindow();
}

void FindInFiles::OnDestroy()
{
  delete m_richText;
  m_richText = NULL;

  // Save the window state
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
  {
    WINDOWPLACEMENT place;
    place.length = sizeof place;
    GetWindowPlacement(&place);
    registryKey.SetBinaryValue("Find in Files Placement",&place,sizeof WINDOWPLACEMENT);
  }

  I7BaseDialog::OnDestroy();
}

void FindInFiles::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di)
{
  if (nIDCtl == IDC_REGEX_HELP)
  {
    CRect helpRect(di->rcItem);
    HDC hdc = 0;
    HANDLE pb = ::BeginBufferedPaint(di->hDC,helpRect,BPBF_COMPATIBLEBITMAP,NULL,&hdc);
    if (pb == 0)
      return;
    CDC* dc = CDC::FromHandle(hdc);
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

    ::EndBufferedPaint(pb,TRUE);
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
  // Handle the return key being pressed on the results list as a select action
  if ((GetFocus() == &m_resultsList) && (GetKeyState(VK_RETURN) != 0))
  {
    int item = m_resultsList.GetNextItem(-1,LVNI_SELECTED);
    if ((item >= 0) && (item < (int)m_results.size()))
      ShowResult(m_results.at(item));
    return;
  }

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
    Find(m_project->GetSource(),m_project->GetDisplayName(false),"","","",FoundInSource);
  if (m_lookExts)
    FindInExtensions();
  if (m_lookDocMain || m_lookDocPhrases || m_lookDocCode)
    FindInDocumentation();
  std::sort(m_results.begin(),m_results.end());

  // Update the results
  m_resultsList.DeleteAllItems();
  for (int i = 0; i < (int)m_results.size(); i++)
  {
    m_resultsList.InsertItem(i,"");
    m_resultsList.SetItemText(i,1,m_results[i].doc);
    m_resultsList.SetItemText(i,2,m_results[i].TypeName());
  }

  // Resize the results columns
  CRect resultsRect;
  m_resultsList.GetClientRect(resultsRect);
  m_resultsList.SetColumnWidth(0,(int)(0.54 * resultsRect.Width()));
  m_resultsList.SetColumnWidth(1,(int)(0.24 * resultsRect.Width()));
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

void FindInFiles::OnChangeFindText()
{
  UpdateData(TRUE);
  GetDlgItem(IDC_FIND_ALL)->EnableWindow(!m_findText.IsEmpty());
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
    // Make sure that we have a result for the given item
    int item = (int)custom->nmcd.dwItemSpec;
    if (item >= m_results.size())
      return;

    // Work out the background colour
    COLORREF backColour = m_results[item].Colour();
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
    HDC hdc = 0;
    HANDLE pb = ::BeginBufferedPaint(custom->nmcd.hdc,rect,BPBF_COMPATIBLEBITMAP,NULL,&hdc);
    if (pb == 0)
      return;
    CDC* dc = CDC::FromHandle(hdc);
    dc->SetTextColor(::GetSysColor(selected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
    dc->SetBkMode(TRANSPARENT);
    CFont* oldFont = dc->SelectObject(m_resultsList.GetFont());

    // Draw the background
    dc->FillSolidRect(rect,selected ? ::GetSysColor(COLOR_HIGHLIGHT) : backColour);

    // Special case painting of the first column
    if (custom->iSubItem == 0)
    {
      // Get the text
      const CStringW& prefix = m_results[item].prefix;
      const CStringW& text = m_results[item].context;
      int high1 = m_results[item].inContext.cpMin;
      int high2 = m_results[item].inContext.cpMax;

      // Draw the text
      if (!prefix.IsEmpty())
      {
        LOGFONT logFont;
        m_resultsList.GetFont()->GetLogFont(&logFont);
        logFont.lfItalic = TRUE;
        CFont italicFont;
        italicFont.CreateFontIndirect(&logFont);
        CFont* previousFont = dc->SelectObject(&italicFont);
        DrawText(dc,prefix,prefix.GetLength(),textRect,DT_VCENTER|DT_NOPREFIX);
        dc->SelectObject(previousFont);
      }
      DrawText(dc,text.GetString(),high1,textRect,DT_VCENTER|DT_NOPREFIX);
      if (textRect.left < textRect.right)
      {
        LOGFONT logFont;
        m_resultsList.GetFont()->GetLogFont(&logFont);
        logFont.lfWeight = FW_BOLD;
        CFont boldFont;
        boldFont.CreateFontIndirect(&logFont);
        CFont* previousFont = dc->SelectObject(&boldFont);
        DrawText(dc,text.GetString()+high1,high2-high1,textRect,DT_VCENTER|DT_NOPREFIX);
        dc->SelectObject(previousFont);
      }
      if (textRect.left < textRect.right)
      {
        DrawText(dc,text.GetString()+high2,text.GetLength()-high2,textRect,
          DT_VCENTER|DT_WORD_ELLIPSIS|DT_NOPREFIX);
      }
    }
    else
    {
      CString text = m_resultsList.GetItemText(item,custom->iSubItem);
      dc->DrawText(text,textRect,DT_VCENTER|DT_SINGLELINE|DT_WORD_ELLIPSIS|DT_NOPREFIX);
    }

    dc->SelectObject(oldFont);
    ::EndBufferedPaint(pb,TRUE);
    *result = CDRF_SKIPDEFAULT;
  }
  break;
  }
}

void FindInFiles::OnResultsSelect(NMHDR*, LRESULT* result)
{
  int item = m_resultsList.GetNextItem(-1,LVNI_SELECTED);
  if ((item >= 0) && (item < (int)m_results.size()))
    ShowResult(m_results.at(item));
  *result = 0;
}

LRESULT FindInFiles::OnResultsResize(WPARAM, LPARAM)
{
  // Set up a device context
  CDC* dc = m_resultsList.GetDC();
  CFont* oldFont = dc->SelectObject(m_resultsList.GetFont());

  // Create bold and italic fonts
  LOGFONT logFont;
  m_resultsList.GetFont()->GetLogFont(&logFont);
  logFont.lfWeight = FW_BOLD;
  CFont boldFont;
  boldFont.CreateFontIndirect(&logFont);
  logFont.lfWeight = FW_NORMAL;
  logFont.lfItalic = TRUE;
  CFont italicFont;
  italicFont.CreateFontIndirect(&logFont);

  int colWidth = 0;
  for (int i = 0; i < (int)m_results.size(); i++)
  {
    // Get the text
    const CStringW& prefix = m_results[i].prefix;
    const CStringW& text = m_results[i].context;
    int high1 = m_results[i].inContext.cpMin;
    int high2 = m_results[i].inContext.cpMax;

    // Measure the text
    int width = 0;
    if (!prefix.IsEmpty())
    {
      dc->SelectObject(&italicFont);
      width += MeasureText(dc,prefix.GetString(),prefix.GetLength());
    }
    dc->SelectObject(m_resultsList.GetFont());
    width += MeasureText(dc,text.GetString(),high1);
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

void FindInFiles::Find(const CString& text, const char* doc, const char* docSort,
  const char* path, const char* prefix, FoundIn type)
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
      leading.Replace(L'\t',L' ');
      leading.TrimLeft();
      CStringW match = GetMatchRange(text,matchStart,matchEnd);
      CStringW trailing = GetMatchRange(text,matchEnd,lineEnd);
      trailing.Replace(L'\t',L' ');
      trailing.TrimRight();
      CStringW context = leading + match + trailing;
      context.Replace(L'\n',L' ');
      context.Replace(L'\r',L' ');
      context.Replace(L'\t',L' ');

      // Store the found result
      FindResult result;
      result.prefix = TextFormat::UTF8ToUnicode(prefix);
      result.context = context;
      result.inContext.cpMin = leading.GetLength();
      result.inContext.cpMax = leading.GetLength() + match.GetLength();
      result.type = type;
      result.doc = doc;
      result.docSort = docSort;
      result.path = path;
      result.loc.cpMin = matchStart;
      result.loc.cpMax = matchEnd;
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

void FindInFiles::FindInExtensions(void)
{
  // If an extension census is running, wait for it to finish
  while (true)
  {
    bool findNow = true;
    CArray<CFrameWnd*> frames;
    theApp.GetWindowFrames(frames);
    for (int i = 0; i < frames.GetSize(); i++)
    {
      if (frames[i]->IsKindOf(RUNTIME_CLASS(ProjectFrame)))
      {
        if (((ProjectFrame*)frames[i])->IsProcessRunning("ni (census)"))
          findNow = false;
      }
    }
    if (findNow)
      break;
    ::Sleep(100);
    theApp.RunMessagePump();
  }

  for (const auto& extension : theApp.GetExtensions())
  {
    CFile extFile;
    if (extFile.Open(extension.path.c_str(),CFile::modeRead))
    {
      int utfLen = (int)extFile.GetLength();

      // Read in the file as UTF-8
      CString utfText;
      LPSTR utfPtr = utfText.GetBufferSetLength(utfLen);
      extFile.Read(utfPtr,utfLen);
      utfText.ReleaseBuffer(utfLen);

      // Check for a UTF-8 BOM
      if (utfText.GetLength() >= 3)
      {
        if (utfText.Left(3) == "\xEF\xBB\xBF")
          utfText = utfText.Mid(3);
      }

      Find(utfText,extension.title.c_str(),"",extension.path.c_str(),"",FoundInExtension);
      theApp.RunMessagePump();
    }
  }
}

void FindInFiles::FindInDocumentation(void)
{
  // Make sure that the background thread has decoded the documentation
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

  // Clean up after the background thread
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
      // Find any matches in this documentation page
      DocText* docText = m_data->texts[i];
      CString title;
      title.Format("%s: %s",docText->section,docText->title);
      size_t resultsIndex = m_results.size();
      Find(docText->body,title,docText->sort,docText->link,docText->prefix,docText->type);

      // Check that any matches are in appropriate sections
      while (resultsIndex < m_results.size())
      {
        int start = m_results[resultsIndex].loc.cpMin;
        CString phraseId = GetSectionId(start,docText->phraseSections);
        CString codeId = GetSectionId(start,docText->codeSections);

        bool keep = false;
        CString linkId;
        if (m_lookDocMain && phraseId.IsEmpty() && codeId.IsEmpty())
          keep = true;
        else if (m_lookDocPhrases && !phraseId.IsEmpty())
        {
          keep = true;
          linkId = phraseId;
        }
        else if (m_lookDocCode && !codeId.IsEmpty())
        {
          keep = true;
          linkId = codeId;
        }

        if (keep)
        {
          if (!linkId.IsEmpty())
          {
            CString baseLink = m_results[resultsIndex].path;
            int endPos = baseLink.ReverseFind('#');
            if (endPos > 0)
              baseLink = baseLink.Left(endPos);
            m_results[resultsIndex].path.Format("%s#%s",baseLink,linkId);
          }
          resultsIndex++;
        }
        else
          m_results.erase(m_results.begin() + resultsIndex);
      }

      theApp.RunMessagePump();
    }
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
    ::DrawTextW(dc->GetSafeHdc(),text,length,rect,format|DT_SINGLELINE);

    CRect measure(rect);
    ::DrawTextW(dc->GetSafeHdc(),text,length,measure,format|DT_SINGLELINE|DT_CALCRECT);
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
  type = FoundInUnknown;
  inContext.cpMin = 0;
  inContext.cpMax = 0;
  loc.cpMin = 0;
  loc.cpMax = 0;
}

bool FindInFiles::FindResult::operator<(const FindInFiles::FindResult& fr) const
{
  if (type != fr.type)
    return type < fr.type;
  if (docSort != fr.docSort)
    return docSort < fr.docSort;
  if (doc != fr.doc)
    return doc < fr.doc;
  if (loc.cpMin != fr.loc.cpMin)
    return loc.cpMin < fr.loc.cpMin;
  return loc.cpMax < fr.loc.cpMax;
}

CString FindInFiles::FindResult::TypeName(void)
{
  switch (type)
  {
  case FoundInSource:
    return "Source";
  case FoundInExtension:
    return "Extensions";
  case FoundInWritingWithInform:
    return "Writing with Inform";
  case FoundInRecipeBook:
    return "The Inform Recipe Book";
  }
  return "";
}

COLORREF FindInFiles::FindResult::Colour(void)
{
  if (type == FoundInRecipeBook)
    return theApp.GetColour(InformApp::ColourContents);
  return theApp.GetColour(InformApp::ColourBack);
}

void FindInFiles::ShowResult(const FindResult& result)
{
  switch (result.type)
  {
  case FoundInSource:
    m_project->SelectInSource(result.loc);
    break;
  case FoundInExtension:
    ExtensionFrame::StartSelect(result.path,result.loc,m_project->GetSettings());
    break;
  case FoundInWritingWithInform:
  case FoundInRecipeBook:
    m_project->SelectInDocumentation(result.path,
      result.context.Mid(result.inContext.cpMin,result.inContext.cpMax-result.inContext.cpMin));
    break;
  }
}

// Caching of text extracted from the documentation

FindInFiles::DocData* FindInFiles::m_data = NULL;
CWinThread* FindInFiles::m_pThread = NULL;

struct Tag
{
  const char* name;
  int len;
  bool remove;
  bool cr;
};

static struct Tag tags[] =
{
  "a",          1,false,false,
  "B>",         2,false,false,
  "b>",         2,false,false,
  "blockquote",10,false,false,
  "br",         2,false,true,
  "font",       4,false,false,
  "h",          1,false,false,
  "i>",         2,false,false,
  "img",        3,false,false,
  "p>",         2,false,true,
  "p ",         2,false,true,
  "script",     6,true, false,
  "table",      5,false,false,
  "TABLE",      5,false,false,
  "td",         2,false,false,
  "TD",         2,false,false,
  "tr",         2,false,false,
  "TR",         2,false,false,
  "div",        3,false,false,
  "pre",        3,false,false,
  "span",       4,true, false,
};

struct Literal
{
  const char* name;
  int len;
  char replace;
};

static struct Literal literals[] =
{
  "quot;",5,'\"',
  "nbsp;",5,' ',
  "lt;",3,'<',
  "gt;",3,'>',
  "amp;",4,'&',
  "#160;",5,' ',
};

void FindInFiles::DecodeHTML(const char* filename, FoundIn docType)
{
  // Open the file
  CFile htmlFile;
  if (htmlFile.Open(filename,CFile::modeRead) == 0)
    return;

  // Read it into memory
  CString html;
  int len = (int)htmlFile.GetLength();
  htmlFile.Read(html.GetBuffer(len),len);
  html.ReleaseBuffer(len);

  // Get the body text
  int body1 = html.Find("<body");
  if (body1 == -1)
    return;
  body1 = html.Find(">",body1);
  if (body1 == -1)
    return;
  int body2 = html.Find("</body>");
  if (body2 <= body1)
    return;
  CString bodyHtml = html.Mid(body1+1,body2-body1-1);

  // Create a DocText instance for this file
  DocText* mainDocText = new DocText(docType);
  mainDocText->link = filename;
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
  int codeStart = 0;
  int phraseStart = 0;
  CString codeAnchor;
  CString phraseAnchor;
  const char* p1 = bodyHtml;
  const char* p2 = p1+len;
  while (p1 < p2)
  {
    // Look for a markup element
    if ((*p1 == '<') && (isalpha((unsigned char)(*(p1+1))) || (*(p1+1) == '/')))
    {
      // Check for a closing markup element
      bool closing = false;
      if (*(p1+1) == '/')
      {
        closing = true;
        p1++;
      }

      // Scan for a known markup element
      bool found = false;
      int i = 0;
      while (!found && (i < sizeof tags / sizeof tags[0]))
      {
        if (strncmp(p1+1,tags[i].name,tags[i].len) == 0)
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
        CString search;
        search.Format("</%s>",tags[i].name);
        p1 = strstr(p1,search);
        if (p1 != NULL)
          p1 += search.GetLength()-1;
        else
          p1 = p2;
      }
      else
      {
        // Remove just the element
        while ((p1 < p2) && (*p1 != '>'))
          p1++;
      }
      ASSERT(*p1 == '>');

      // Add a carriage return for appropriate markup
      if (found && !closing && tags[i].cr && !ignore)
        docText->body.AppendChar('\n');
      white = false;
    }
    else if ((*p1 == '<') && (*(p1+1) == '!'))
    {
      // Extract metadata from comments
      char meta1[256], meta2[256];
      if (sscanf(p1,"<!-- SEARCH TITLE \"%[^\"]",meta1) == 1)
        docText->title = meta1;
      else if (sscanf(p1,"<!-- SEARCH SECTION \"%[^\"]",meta1) == 1)
        docText->section = meta1;
      else if (sscanf(p1,"<!-- SEARCH SORT \"%[^\"]",meta1) == 1)
        docText->sort = meta1;
      else if (sscanf(p1,"<!-- START EXAMPLE \"%[^\"]\" \"%[^\"]",meta1,meta2) == 2)
      {
        docText = new DocText(mainDocText->type);
        docText->link.Format("%s#%s",mainDocText->link,meta2);
        docText->prefix.Format("(Example %s) ",meta1);
        docText->title = mainDocText->title;
        docText->section = mainDocText->section;
        docText->sort = mainDocText->sort;
        docText->body.Preallocate(len/2);
        {
          CSingleLock lock(&(m_data->lock),TRUE);
          m_data->texts.Add(docText);
        }
      }
      else if (strncmp(p1,"<!-- END EXAMPLE -->",20) == 0)
        docText = mainDocText;
      else if (strncmp(p1,"<!-- START IGNORE ",18) == 0)
        ignore = true;
      else if (strncmp(p1,"<!-- END IGNORE -->",19) == 0)
        ignore = false;
      else if (sscanf(p1,"<!-- START CODE \"%[^\"]",meta1) == 1)
      {
        codeStart = docText->body.GetLength();
        codeAnchor = meta1;
      }
      else if (strncmp(p1,"<!-- END CODE -->",17) == 0)
      {
        int codeEnd = docText->body.GetLength();
        docText->codeSections.push_back(DocSection(codeStart,codeEnd,codeAnchor));
      }
      else if (sscanf(p1,"<!-- START PHRASE \"%[^\"]",meta1) == 1)
      {
        phraseStart = docText->body.GetLength();
        phraseAnchor = meta1;
      }
      else if (strncmp(p1,"<!-- END PHRASE -->",19) == 0)
      {
        int phraseEnd = docText->body.GetLength();
        docText->phraseSections.push_back(DocSection(phraseStart,phraseEnd,phraseAnchor));
      }

      p1 = strstr(p1,"-->");
      if (p1 != NULL)
        p1 += 2;
      else
        p1 = p2;
    }
    else if (*p1 == '&')
    {
      // Scan for a known literal
      bool found = false;
      int i = 0;
      while (!found && (i < sizeof literals / sizeof literals[0]))
      {
        if (strncmp(p1+1,literals[i].name,literals[i].len) == 0)
          found = true;
        if (!found)
          i++;
      }

      // Replace the literal
      if (found)
      {
        if (!ignore)
          docText->body.AppendChar(literals[i].replace);
        p1 += literals[i].len;
      }
      else
      {
        ASSERT(FALSE);
        if (!ignore)
          docText->body.AppendChar(*p1);
      }
      white = false;
    }
    else if (isspace((unsigned char)(*p1)))
    {
      if (!white && !ignore)
        docText->body.AppendChar(' ');
      white = true;
    }
    else
    {
      if (!ignore)
        docText->body.AppendChar(*p1);
      white = false;
    }
    p1++;
  }
}

UINT FindInFiles::BackgroundDecodeThread(LPVOID)
{
  CFileFind findDoc;
  for (int i = 0; i < 2; i++)
  {
    CString findPath;
    FoundIn docType = FoundInUnknown;
    switch (i)
    {
    case 0:
      findPath.Format("%s\\Documentation\\doc*.html",theApp.GetAppDir());
      docType = FoundInWritingWithInform;
      break;
    case 1:
      findPath.Format("%s\\Documentation\\rdoc*.html",theApp.GetAppDir());
      docType = FoundInRecipeBook;
      break;
    }

    BOOL found = findDoc.FindFile(findPath);
    while (found)
    {
      // Get the filename of a documentation file
      found = findDoc.FindNextFile();

      // Extract the title and text
      DecodeHTML(findDoc.GetFilePath(),docType);
    }
  }

  {
    CSingleLock lock(&(m_data->lock),TRUE);
    m_data->done = true;
  }
  TRACE("Background thread finished processing HTML for searching\n");
  return 0;
}

CString FindInFiles::GetSectionId(int pos, const std::vector<DocSection>& sections)
{
  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    if (pos < it->start)
      break;
    if ((pos >= it->start) && (pos < it->end))
      return it->id;
  }
  return "";
}

FindInFiles::DocSection::DocSection(int start_, int end_, const char* id_)
  : start(start_), end(end_), id(id_)
{
}

FindInFiles::DocText::DocText(FoundIn docType) : type(docType)
{
}
