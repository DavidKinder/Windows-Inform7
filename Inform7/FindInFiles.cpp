#include "stdafx.h"
#include "FindInFiles.h"
#include "ExtensionFrame.h"
#include "ProjectFrame.h"
#include "Resource.h"
#include "RichEdit.h"
#include "TextFormat.h"

#include "DpiFunctions.h"

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

BEGIN_MESSAGE_MAP(FindInFiles, I7BaseDialog)
  ON_WM_CLOSE()
  ON_WM_DESTROY()
  ON_WM_DRAWITEM()
  ON_WM_ERASEBKGND()
  ON_WM_GETMINMAXINFO()
  ON_WM_SIZE()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
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
  m_dpi = 96;
  
  m_lookSource = TRUE;
  m_lookExts = TRUE;
  m_lookDocPhrases = TRUE;
  m_lookDocMain = TRUE;
  m_lookDocCode = TRUE;

  m_ignoreCase = TRUE;
  m_findRule = FindRule_Contains;
  m_richText = NULL;

  m_total = 0;
  m_current = 0;
}

void FindInFiles::Show(void)
{
  if (GetSafeHwnd() == 0)
  {
    if (m_richText == NULL)
      m_richText = new RichDrawText(InformApp::FontSystem);
    Create(m_lpszTemplateName,m_project);
    theApp.SetIcon(this);
    m_dpi = DPI::getWindowDPI(this);

    bool placementSet = false;
    CRegKey registryKey;
    if (registryKey.Create(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW) == ERROR_SUCCESS)
    {
      // Restore the window state
      WINDOWPLACEMENT place;
      ULONG len = sizeof WINDOWPLACEMENT;
      if (registryKey.QueryBinaryValue("Find in Files Placement 96dpi",&place,&len) == ERROR_SUCCESS)
      {
        DPI::ContextUnaware dpiUnaware;
        SetWindowPlacement(&place);
        placementSet = true;
      }
      else if (registryKey.QueryBinaryValue("Find in Files Placement",&place,&len) == ERROR_SUCCESS)
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

    UpdateUIState();
    ShowWindow(SW_SHOW);
    m_findAllBtn.EnableWindow(!m_findText.IsEmpty());
    GetDlgItem(IDC_FIND)->SetFocus();
  }
}

void FindInFiles::Hide(void)
{
  if (GetSafeHwnd() != 0)
    ShowWindow(SW_HIDE);
}

void FindInFiles::SetDarkMode(DarkMode* dark)
{
  I7BaseDialog::SetDarkMode(dark);
  m_progress.SetDarkMode(dark);
  m_resultsList.SetDarkMode(dark);
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
  m_findRule = FindRule_Contains;
  m_findText = text;
  UpdateData(FALSE);
  m_findAllBtn.EnableWindow(!m_findText.IsEmpty());
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
  m_findRule = FindRule_Contains;
  m_findText = text;
  UpdateData(FALSE);
  m_findAllBtn.EnableWindow(!m_findText.IsEmpty());
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
}

void FindInFiles::ExitInstance(void)
{
  if (m_data)
  {
    if (m_pThread)
    {
      // If the background thread is still running, stop it
      {
        CSingleLock lock(&(m_data->lock),TRUE);
        m_data->stop = true;
      }

      if (::WaitForSingleObject(*m_pThread,10*1000) != WAIT_OBJECT_0)
        ::ExitProcess(1);

      delete m_pThread;
      m_pThread = NULL;
    }

    // Free the memory for the text extracted from the documentation
    {
      CSingleLock lock(&(m_data->lock),TRUE);
      if (m_data->done)
      {
        for (int i = 0; i < m_data->texts.GetSize(); i++)
          delete m_data->texts[i];
        m_data->texts.RemoveAll();
      }
    }
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
  DDX_Control(pDX, IDC_FOUND, m_found);
  DDX_Control(pDX, IDC_PROGRESS, m_progress);
  DDX_Control(pDX, IDC_REGEX_HELP, m_regexHelp);
  DDX_Control(pDX, IDC_FIND, m_findEdit);
  DDX_TextW(pDX, IDC_FIND, m_findText);
  DDX_Control(pDX, IDC_FIND_ALL, m_findAllBtn);
  DDX_Control(pDX, IDC_LOOK_GROUP, m_lookGroup);
  DDX_Check(pDX,IDC_LOOK_SOURCE,m_lookSource);
  DDX_Check(pDX,IDC_LOOK_EXTENSIONS,m_lookExts);
  DDX_Check(pDX,IDC_LOOK_DOC_PHRASES,m_lookDocPhrases);
  DDX_Check(pDX,IDC_LOOK_DOC_MAIN,m_lookDocMain);
  DDX_Check(pDX,IDC_LOOK_DOC_CODE,m_lookDocCode);
  DDX_Control(pDX, IDC_HOW_GROUP, m_howGroup);
  DDX_Check(pDX,IDC_IGNORE_CASE,m_ignoreCase);
  DDX_Control(pDX,IDC_FIND_RULE,m_findRuleCtrl);
  DDX_CBIndex(pDX,IDC_FIND_RULE,(int&)m_findRule);
  DDX_Control(pDX, IDC_RESULTS, m_resultsList);
}

BOOL FindInFiles::OnInitDialog()
{
  if (I7BaseDialog::OnInitDialog())
  {
    m_lookSourceCtrl.SubclassDlgItem(IDC_LOOK_SOURCE,this,IDR_DARK_CHECK);
    m_lookExtsCtrl.SubclassDlgItem(IDC_LOOK_EXTENSIONS,this,IDR_DARK_CHECK);
    m_lookDocPhrasesCtrl.SubclassDlgItem(IDC_LOOK_DOC_PHRASES,this,IDR_DARK_CHECK);
    m_lookDocMainCtrl.SubclassDlgItem(IDC_LOOK_DOC_MAIN,this,IDR_DARK_CHECK);
    m_lookDocCodeCtrl.SubclassDlgItem(IDC_LOOK_DOC_CODE,this,IDR_DARK_CHECK);
    m_ignoreCaseCtrl.SubclassDlgItem(IDC_IGNORE_CASE,this,IDR_DARK_CHECK);
    m_resultsList.SubclassHeader(&m_resultsHeader);

    // Initialize auto-completion for the find string
    if (FAILED(m_findAutoComplete.CoCreateInstance(CLSID_AutoComplete)))
      return FALSE;
    FindEnumString* fes = new FindEnumString();
    CComQIPtr<IEnumString> ies(fes->GetInterface(&IID_IEnumString));
    m_findAutoComplete->Init(GetDlgItem(IDC_FIND)->GetSafeHwnd(),ies,NULL,NULL);
    m_findAutoComplete->SetOptions(ACO_AUTOSUGGEST);
    fes->ExternalRelease();

    m_findHelper.InitResultsCtrl(&m_resultsList,true);

    CRect windowRect, resultsRect;
    GetWindowRect(windowRect);
    m_minSize = windowRect.Size();
    m_resultsList.GetWindowRect(resultsRect);
    m_gapBottomRight = windowRect.BottomRight() - resultsRect.BottomRight();

    m_found.ModifyStyle(0,WS_VISIBLE);
    m_progress.ModifyStyle(WS_VISIBLE,0);

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
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
  {
    WINDOWPLACEMENT place;
    place.length = sizeof place;
    {
      DPI::ContextUnaware dpiUnaware;
      GetWindowPlacement(&place);
    }
    registryKey.SetBinaryValue("Find in Files Placement 96dpi",&place,sizeof WINDOWPLACEMENT);
  }

  I7BaseDialog::OnDestroy();
}

void FindInFiles::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di)
{
  if (nIDCtl == IDC_REGEX_HELP)
  {
    DarkMode* dark = DarkMode::GetActive(this);

    CRect helpRect(di->rcItem);
    HDC hdc = 0;
    HANDLE pb = ::BeginBufferedPaint(di->hDC,helpRect,BPBF_COMPATIBLEBITMAP,NULL,&hdc);
    if (pb == 0)
      return;
    CDC* dc = CDC::FromHandle(hdc);

    if (dark)
    {
      dc->FillSolidRect(helpRect,dark->GetColour(DarkMode::Darkest));
      m_richText->TextColourChanged(dark->GetColour(DarkMode::Fore));
    }
    else
    {
      dc->FillSolidRect(helpRect,::GetSysColor(COLOR_BTNFACE));
      m_richText->TextColourChanged(theApp.GetColour(InformApp::ColourText));
    }

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
    SetRichTextRTF(text1);
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
    SetRichTextRTF(text2);
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
    SetRichTextRTF(text3);
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
    resultsRect.right = windowRect.right - m_gapBottomRight.cx;
    resultsRect.bottom = windowRect.bottom - m_gapBottomRight.cy;
    ScreenToClient(resultsRect);
    m_resultsList.MoveWindow(resultsRect);
  }

  // Make sure that the sizing gripper is redrawn
  Invalidate();
}

LRESULT FindInFiles::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  int newDpi = (int)HIWORD(wparam);

  // Reset the minimum size before default processing
  CSize minSize = m_minSize;
  m_minSize.SetSize(0,0);

  // Get the positon of the results control before default processing
  CRect resultsRectBefore;
  if (m_resultsList.GetSafeHwnd() != 0)
  {
    m_resultsList.GetWindowRect(resultsRectBefore);
    ScreenToClient(resultsRectBefore);
  }

  Default();

  if (newDpi != m_dpi)
  {
    if (m_resultsList.GetSafeHwnd() != 0)
    {
      CRect resultsRectAfter;
      m_resultsList.GetWindowRect(resultsRectAfter);
      ScreenToClient(resultsRectAfter);
      double scaleX = (double)resultsRectAfter.right / (double)resultsRectBefore.right;
      double scaleY = (double)resultsRectAfter.top / (double)resultsRectBefore.top;

      m_resultsList.SetFont(theApp.GetFont(this,InformApp::FontSmall));
      for (int i = 0; i < 3; i++)
        m_resultsList.SetColumnWidth(i,(int)(scaleX*m_resultsList.GetColumnWidth(i)));

      m_gapBottomRight.cx = (int)(scaleX*m_gapBottomRight.cx);
      m_gapBottomRight.cy = (int)(scaleY*m_gapBottomRight.cy);
      m_minSize.cx = (int)(scaleX*minSize.cx);
      m_minSize.cy = (int)(scaleY*minSize.cy);
    }
    m_dpi = newDpi;
  }
  return 0;
}

void FindInFiles::OnFindAll()
{
  // Handle the return key being pressed on the results list as a select action
  if ((GetFocus() == &m_resultsList) && (::GetKeyState(VK_RETURN) != 0))
  {
    int item = m_resultsList.GetNextItem(-1,LVNI_SELECTED);
    if ((item >= 0) && (item < (int)m_findHelper.results.size()))
      ShowResult(m_findHelper.results.at(item));
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

  CWaitCursor wc;

  // How many files are to be searched?
  m_total = 0;
  if (m_lookSource)
    m_total += CountSource();
  if (m_lookExts)
    m_total += CountExtensions();
  if (m_lookDocMain || m_lookDocPhrases || m_lookDocCode)
    m_total += CountDocumentation();
  m_current = 0;

  // Show the progress bar
  CRect progressRect, resultsRect;
  m_progress.GetWindowRect(progressRect);
  m_resultsList.GetWindowRect(resultsRect);
  progressRect.left = resultsRect.left;
  progressRect.right = resultsRect.right;
  ScreenToClient(progressRect);
  m_progress.MoveWindow(progressRect);
  m_progress.ModifyStyle(0,WS_VISIBLE);
  m_found.ModifyStyle(WS_VISIBLE,0);

  // Search for the text
  m_findHelper.results.clear();
  try
  {
    if (m_lookSource)
      FindInSource();
    if (m_lookExts)
      FindInExtensions();
    if (m_lookDocMain || m_lookDocPhrases || m_lookDocCode)
      FindInDocumentation();
  }
  catch (std::regex_error& ex)
  {
    MessageBox(FindAllHelper::RegexError(ex),INFORM_TITLE,MB_ICONERROR|MB_OK);
  }
  std::sort(m_findHelper.results.begin(),m_findHelper.results.end());
  m_findHelper.UpdateResultsCtrl(&m_resultsList,true);

  // Hide the progress bar
  m_found.ModifyStyle(0,WS_VISIBLE);
  m_progress.ModifyStyle(WS_VISIBLE,0);

  // Update the found status message and what is visible
  CString foundMsg;
  switch (m_findHelper.results.size())
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
    foundMsg.Format("Found %s results:",TextFormat::FormatNumber((int)m_findHelper.results.size()));
    m_resultsList.ModifyStyle(0,WS_VISIBLE);
    m_resultsList.SetFocus();
    break;
  }
  m_found.SetWindowText(foundMsg);
  m_regexHelp.ModifyStyle(WS_VISIBLE,0);
  Invalidate();
}

void FindInFiles::OnChangeFindRule()
{
  UpdateData(TRUE);
  if (m_findRule == FindRule_Regex)
  {
    m_resultsList.ModifyStyle(WS_VISIBLE,0);
    m_regexHelp.ModifyStyle(0,WS_VISIBLE);
    m_found.SetWindowText("");
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
  m_findAllBtn.EnableWindow(!m_findText.IsEmpty());
}

void FindInFiles::OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result)
{
  m_findHelper.OnResultsDraw(&m_resultsList,(NMLVCUSTOMDRAW*)pNotifyStruct,result);
}

void FindInFiles::OnResultsSelect(NMHDR*, LRESULT* result)
{
  int item = m_resultsList.GetNextItem(-1,LVNI_SELECTED);
  if ((item >= 0) && (item < (int)m_findHelper.results.size()))
    ShowResult(m_findHelper.results.at(item));
  *result = 0;
}

LRESULT FindInFiles::OnResultsResize(WPARAM, LPARAM)
{
  m_findHelper.OnResultsResize(&m_resultsList);
  return 0;
}

void FindInFiles::FindInSource(void)
{
  UpdateProgress();

  m_findHelper.Find(m_project->GetSource(),m_findText,m_ignoreCase,m_findRule,
    m_project->GetDisplayName(false),"","",FoundIn_Source);
  theApp.RunMessagePump();
  m_current++;

  std::vector<CString> paths;
  m_project->GetExternalSourceFiles(paths);
  for (const CString& path : paths)
  {
    UpdateProgress();

    CString sourceText = ReadUTF8File(path);
    if (!sourceText.IsEmpty())
    {
      CString title = path;
      int start = title.ReverseFind('\\');
      if (start >= 0)
        title = title.Mid(start+1);

      m_findHelper.Find(sourceText,m_findText,m_ignoreCase,m_findRule,
        title,"",path,FoundIn_Source);
      theApp.RunMessagePump();
    }
    m_current++;
  }
}

size_t FindInFiles::CountSource(void)
{
  std::vector<CString> paths;
  m_project->GetExternalSourceFiles(paths);
  return paths.size()+1;
}

void FindInFiles::FindInExtensions(void)
{
  Extension::Set extensions;
  Extension::GetAll(extensions,m_project->GetMaterialsFolder());

  for (const auto& extension : extensions)
  {
    UpdateProgress();

    CString extText = ReadUTF8File(extension.path.c_str());
    if (!extText.IsEmpty())
    {
      m_findHelper.Find(extText,m_findText,m_ignoreCase,m_findRule,
        extension.title.c_str(),"",extension.path.c_str(),FoundIn_Extension);
      theApp.RunMessagePump();
    }
    m_current++;
  }
}

size_t FindInFiles::CountExtensions(void)
{
  Extension::Set extensions;
  Extension::GetAll(extensions,m_project->GetMaterialsFolder());
  return extensions.size();
}

void FindInFiles::FindInDocumentation(void)
{
  WaitForDocThread();

  CSingleLock lock(&(m_data->lock),TRUE);
  for (int i = 0; i < m_data->texts.GetSize(); i++)
  {
    UpdateProgress();

    // Find any matches in this documentation page
    DocText* docText = m_data->texts[i];
    CString title;
    title.Format("%s: %s",docText->section,docText->title);
    size_t resultsIndex = m_findHelper.results.size();
    m_findHelper.Find(docText->body,m_findText,m_ignoreCase,m_findRule,
      title,docText->sort,docText->link,docText->type);

    // Check that any matches are in appropriate sections
    while (resultsIndex < m_findHelper.results.size())
    {
      int start = m_findHelper.results[resultsIndex].loc.cpMin;
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
          CString baseLink = m_findHelper.results[resultsIndex].path;
          int endPos = baseLink.ReverseFind('#');
          if (endPos > 0)
            baseLink = baseLink.Left(endPos);
          m_findHelper.results[resultsIndex].path.Format("%s#%s",baseLink,linkId);
        }
        resultsIndex++;
      }
      else
        m_findHelper.results.erase(m_findHelper.results.begin() + resultsIndex);
    }

    theApp.RunMessagePump();
    m_current++;
  }
}

size_t FindInFiles::CountDocumentation(void)
{
  WaitForDocThread();
  CSingleLock lock(&(m_data->lock),TRUE);
  return m_data->texts.GetSize();
}

CString FindInFiles::ReadUTF8File(const char* path)
{
  CFile textFile;
  if (textFile.Open(path,CFile::modeRead))
  {
    int utfLen = (int)textFile.GetLength();

    // Read in the file as UTF-8
    CString utfText;
    LPSTR utfPtr = utfText.GetBufferSetLength(utfLen);
    textFile.Read(utfPtr,utfLen);
    utfText.ReleaseBuffer(utfLen);

    // Check for a UTF-8 BOM
    if (utfText.GetLength() >= 3)
    {
      if (utfText.Left(3) == "\xEF\xBB\xBF")
        utfText = utfText.Mid(3);
    }

    return utfText;
  }
  return "";
}

void FindInFiles::WaitForDocThread(void)
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
}

void FindInFiles::UpdateProgress(void)
{
  m_progress.SetPos((int)((100 * (m_current+1)) / (m_total+1)));
}

void FindInFiles::SetRichTextRTF(const char* fragment)
{
  int sysDpi = DPI::getSystemDPI();
  int wndDpi = DPI::getWindowDPI(this);

  CString rtfText;
  rtfText.Format("{\\rtf1\\ansi{\\fs%d%s}}",
    (2*theApp.GetFontSize(InformApp::FontSystem)*wndDpi)/sysDpi,fragment);
  m_richText->SetTextRTF(rtfText);
}

void FindInFiles::ShowResult(const FindResult& result)
{
  switch (result.type)
  {
  case FoundIn_Source:
    if (result.path.IsEmpty())
      m_project->SelectInSource(result.loc);
    else
      ExtensionFrame::StartSelect(result.path,result.loc,m_project->GetSettings());
    break;
  case FoundIn_Extension:
    ExtensionFrame::StartSelect(result.path,result.loc,m_project->GetSettings());
    break;
  case FoundIn_WritingWithInform:
  case FoundIn_RecipeBook:
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
  "br",    2,false,true,
  "p>",    2,false,true,
  "p ",    2,false,true,
  "script",6,true,false,
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
  "bull;",5,' ',
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

  // Get the content
  int body1 = html.Find("<!-- CONTENT BEGINS -->");
  if (body1 == -1)
    return;
  body1 = html.Find(">",body1);
  if (body1 == -1)
    return;
  int body2 = html.Find("<!-- CONTENT ENDS -->",body1);
  if (body2 <= body1)
    return;
  CString bodyHtml = html.Mid(body1+1,body2-body1-1);

  // Create a DocText instance for this file
  DocText* docText = new DocText(docType);
  docText->link = filename;
  {
    CSingleLock lock(&(m_data->lock),TRUE);
    m_data->texts.Add(docText);
  }

  // Reserve space for the main text
  len = bodyHtml.GetLength();
  docText->body.Preallocate(len);

  // Scan the text, removing markup
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

      // Remove the markup
      if (found && !closing && tags[i].remove)
      {
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
      char meta[256];
      if (sscanf(p1,"<!-- SEARCH TITLE \"%[^\"]",meta) == 1)
        docText->title = meta;
      else if (sscanf(p1,"<!-- SEARCH SECTION \"%[^\"]",meta) == 1)
        docText->section = meta;
      else if (sscanf(p1,"<!-- SEARCH SORT \"%[^\"]",meta) == 1)
        docText->sort = meta;
      else if (strncmp(p1,"<!-- START IGNORE ",18) == 0)
        ignore = true;
      else if (strncmp(p1,"<!-- END IGNORE -->",19) == 0)
        ignore = false;
      else if (sscanf(p1,"<!-- START CODE \"%[^\"]",meta) == 1)
      {
        codeStart = docText->body.GetLength();
        codeAnchor = meta;
      }
      else if (strncmp(p1,"<!-- END CODE -->",17) == 0)
      {
        int codeEnd = docText->body.GetLength();
        docText->codeSections.push_back(DocSection(codeStart,codeEnd,codeAnchor));
      }
      else if (sscanf(p1,"<!-- START PHRASE \"%[^\"]",meta) == 1)
      {
        phraseStart = docText->body.GetLength();
        phraseAnchor = meta;
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
  for (int i = 0; i < 3; i++)
  {
    CString findPath;
    FoundIn docType = FoundIn_Unknown;
    switch (i)
    {
    case 0:
      findPath.Format("%s\\Documentation\\WI_*_*.html",theApp.GetAppDir());
      docType = FoundIn_WritingWithInform;
      break;
    case 1:
      findPath.Format("%s\\Documentation\\RB_*_*.html",theApp.GetAppDir());
      docType = FoundIn_RecipeBook;
      break;
    case 2:
      findPath.Format("%s\\Documentation\\eg_*.html",theApp.GetAppDir());
      docType = FoundIn_RecipeBook;
      break;
    }

    BOOL found = findDoc.FindFile(findPath);
    while (found)
    {
      {
        CSingleLock lock(&(m_data->lock),TRUE);
        if (m_data->stop)
        {
          m_data->done = true;
          return 0;
        }
      }

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
  TRACE("Background thread finished processing HTML for find in files\n");
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
