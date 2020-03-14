#include "stdafx.h"
#include "FindInFiles.h"
#include "ProjectFrame.h"
#include "Inform.h"
#include "resource.h"

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

  ~FindEnumString()
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

// The one and only FindInFiles object
FindInFiles theFinder;

BEGIN_MESSAGE_MAP(FindInFiles, I7BaseDialog)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_FIND_ALL, OnFindAll)
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
    Create(m_lpszTemplateName,CWnd::GetDesktopWindow());
    CenterWindow(project);
    theApp.SetIcon(this);
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
            m_findComplete.AddTail(historyItem);
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
  if (GetSafeHwnd() != 0)
    DestroyWindow();
  m_project = NULL;

  try
  {
    if (m_findComplete.GetCount() > 0)
    {
      // Write the list of the auto complete history to a memory file
      CMemFile historyFile;
      CArchive historyArchive(&historyFile,CArchive::store);
      historyArchive << m_findComplete.GetCount();
      POSITION pos = m_findComplete.GetHeadPosition();
      for (INT_PTR i = 0; i < m_findComplete.GetCount(); i++)
        historyArchive << m_findComplete.GetNext(pos);
      historyArchive.Close();
 
      // Write the above buffer to the registry
      CRegKey registryKey;
      if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_WRITE) == ERROR_SUCCESS)
      {
        ULONG historyLen = historyFile.GetLength();
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
}

LPCWSTR FindInFiles::GetAutoComplete(int index)
{
  if (index < m_findComplete.GetSize())
  {
    POSITION pos = m_findComplete.FindIndex(index);
    if (pos != NULL)
      return m_findComplete.GetAt(pos).GetString();
  }
  return NULL;
}

void FindInFiles::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FIND, m_find);
  DDX_TextW(pDX, IDC_FIND, m_findText);
  DDX_Check(pDX,IDC_LOOK_SOURCE,m_lookSource);
  DDX_Check(pDX,IDC_LOOK_EXTENSIONS,m_lookExts);
  DDX_Check(pDX,IDC_LOOK_DOC_PHRASES,m_lookDocPhrases);
  DDX_Check(pDX,IDC_LOOK_DOC_MAIN,m_lookDocMain);
  DDX_Check(pDX,IDC_LOOK_DOC_EXAMPLES,m_lookDocExamples);
  DDX_Check(pDX,IDC_IGNORE_CASE,m_ignoreCase);
  DDX_CBIndex(pDX,IDC_FIND_RULE,m_findRule);
}

BOOL FindInFiles::OnInitDialog()
{
  if (I7BaseDialog::OnInitDialog())
  {
    // Initialize auto-completion for the find string
    if (FAILED(m_auto.CoCreateInstance(CLSID_AutoComplete)))
      return FALSE;
    FindEnumString* fes = new FindEnumString(this);
    CComQIPtr<IEnumString> ies(fes->GetInterface(&IID_IEnumString));
    m_auto->Init(m_find.GetSafeHwnd(),ies,NULL,NULL);
    m_auto->SetOptions(ACO_AUTOSUGGEST);
    fes->ExternalRelease();
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

void FindInFiles::OnFindAll()
{
  UpdateData(TRUE);

  POSITION pos = m_findComplete.Find(m_findText);
  if (pos == NULL)
    m_findComplete.AddTail(m_findText);
  else if (pos != m_findComplete.GetTailPosition())
  {
    m_findComplete.RemoveAt(pos);
    m_findComplete.AddTail(m_findText);
  }
  while (m_findComplete.GetSize() > FIND_HISTORY_LENGTH)
    m_findComplete.RemoveHead();

  CComQIPtr<IAutoCompleteDropDown> acdd(m_auto);
  if (acdd)
    acdd->ResetEnumerator(); 
}
