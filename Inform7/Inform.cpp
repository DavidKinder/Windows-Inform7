#include "stdafx.h"
#include "Inform.h"
#include "OSLayer.h"
#include "ReportHtml.h"
#include "AboutDialog.h"
#include "PrefsDialog.h"
#include "SplashScreen.h"
#include "ProjectFrame.h"
#include "ExtensionFrame.h"
#include "SpellCheck.h"
#include "TabDoc.h"

#include "png.h"
#include "jpeglib.h"

extern "C" __declspec(dllimport) void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(InformApp, CWinApp)
  ON_COMMAND(ID_APP_EXIT, OnAppExit)
  ON_COMMAND(ID_APP_PREFS, OnAppPrefs)
  ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
  ON_COMMAND(ID_APP_WEBPAGE, OnAppWebPage)
END_MESSAGE_MAP()

// The one and only InformApp object
InformApp theApp;

InformApp::InformApp() : m_job(0)
{
  for (int i = 0; i < sizeof m_fontSizes / sizeof m_fontSizes[0]; i++)
    m_fontSizes[i] = 0;
}

BOOL InformApp::InitInstance()
{
  InitCommonControls();
  CWinApp::InitInstance();
  theOS.Init();
  theOS.BufferedPaintInit();

  if (!AfxOleInit())
    return FALSE;
  if (!Scintilla_RegisterClasses(AfxGetInstanceHandle()))
    return FALSE;

  CheckInstalledVersions();

  SetRegistryKey("David Kinder");
  SetFonts();
  ReportHtml::SetIEPreferences(REGISTRY_PATH_BROWSER);

  // Set the HOME environment variable to the My Documents folder,
  // used by the Natural Inform compiler, and make sure directories
  // under My Documents exist.
  SetMyDocuments();

  // If possible, create a job to assign child processes to. Since the
  // job will be closed when this process exits, this ensures that any
  // child processes still running will also exit.
  m_job = theOS.CreateJobObject(NULL,NULL);
  if (m_job)
  {
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    VERIFY(theOS.SetInformationJobObject(m_job,
      JobObjectExtendedLimitInformation,&jeli,sizeof(jeli)));
  }

  // Discard any log file from a previous run
  /*::DeleteFile(m_home+LOG_FILE);*/

  // Install the protocol for inform: URLs
  m_protocol.Install(L"inform");
  CString dir = GetAppDir();
  m_protocol.AddDirectory(dir+"\\Documentation");
  m_protocol.AddDirectory(dir+"\\Documentation\\doc_images");
  m_protocol.AddDirectory(dir+"\\Documentation\\sections");
  m_protocol.AddDirectory(L"//Extensions",m_home+"\\Inform\\Documentation");
  m_protocol.AddDirectory(L"//Extensions",dir+"\\Documentation");

  // Find and create documentation for extensions
  FindExtensions();
  HANDLE ni = RunCensus();

  // Show the splash screen
  SplashScreen splash;
  splash.ShowSplash();

  // Only continue if a project has been opened
  CWnd* mainWnd = AfxGetMainWnd();
  if (mainWnd == NULL)
    return FALSE;

  // Make sure that any census failure is reported
  if (mainWnd->IsKindOf(RUNTIME_CLASS(ProjectFrame)))
    ((ProjectFrame*)mainWnd)->MonitorProcess(ni,ProjectFrame::ProcessNoAction,"ni (census)");
  return TRUE;
}

int InformApp::ExitInstance()
{
  // Clear the bitmap cache
  std::map<std::string,CDibSection*>::iterator it;
  for (it = m_bitmaps.begin(); it != m_bitmaps.end(); ++it)
    delete it->second;

  GameWindow::ExitInstance();
  TabDoc::ExitInstance();
  SpellCheck::Finalize();
  Scintilla_ReleaseResources();

  return CWinApp::ExitInstance();
}

BOOL InformApp::PreTranslateMessage(MSG* pMsg)
{
  if ((pMsg->hwnd == NULL) && DispatchThreadMessageEx(pMsg))
    return TRUE;

  CArray<CFrameWnd*> frames;
  GetWindowFrames(frames);

  CWnd* wnd = CWnd::FromHandle(pMsg->hwnd);
  for (int i = 0; i < frames.GetSize(); i++)
  {
    CFrameWnd* frame = frames[i];
    if (wnd->GetTopLevelParent() == frame)
      return CWnd::WalkPreTranslateTree(frame->GetSafeHwnd(),pMsg);
  }

  CWnd* mainWnd = AfxGetMainWnd();
  if (mainWnd != NULL)
    return mainWnd->PreTranslateMessage(pMsg);
  return FALSE;
}

void InformApp::DoWaitCursor(int nCode)
{
  CWinApp::DoWaitCursor(nCode);

  POINT current;
  ::GetCursorPos(&current);
  CWnd* underWnd = CWnd::WindowFromPoint(current);
  if ((underWnd != NULL) && underWnd->IsKindOf(RUNTIME_CLASS(StopButton)))
    underWnd->SendMessage(WM_SETCURSOR,0,0);
}

BOOL InformApp::OnIdle(LONG lCount)
{
  if (lCount == 0)
  {
    // Check for debugging events from any sub-processes, and terminate any that have failed
    DEBUG_EVENT debug;
    while (::WaitForDebugEvent(&debug,0))
    {
      int kill = 0;
      DWORD status = DBG_CONTINUE;

      if (debug.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
      {
        switch (debug.u.Exception.ExceptionRecord.ExceptionCode)
        {
        case EXCEPTION_ACCESS_VIOLATION:
          if (debug.u.Exception.dwFirstChance)
            status = DBG_EXCEPTION_NOT_HANDLED;
          else
            kill = 10;
          break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
        case EXCEPTION_ILLEGAL_INSTRUCTION:
          kill = 10;
          break;
        case EXCEPTION_STACK_OVERFLOW:
          kill = 11;
          break;
        }
      }

      if (kill != 0)
      {
        HANDLE process = ::OpenProcess(PROCESS_TERMINATE,FALSE,debug.dwProcessId);
        if (process != 0)
          ::TerminateProcess(process,kill);
      }

      ::ContinueDebugEvent(debug.dwProcessId,debug.dwThreadId,status);
    }
  }
  return CWinApp::OnIdle(lCount);
}

void InformApp::OnAppExit()
{
  // Close all secondary window frames first. The close message
  // will cause the window to be removed from the frames array.
  while (m_frames.GetSize() > 0)
  {
    int count = m_frames.GetSize();
    m_frames[0]->SendMessage(WM_CLOSE);

    // If the window does not close, stop
    if (count == m_frames.GetSize())
      return;
  }

  // Close the main window frame. This is done last to prevent
  // any of the secondary frames being promoted to the main
  // window frame.
  ASSERT(m_pMainWnd != NULL);
  m_pMainWnd->SendMessage(WM_CLOSE);
}

void InformApp::OnAppPrefs()
{
  PrefsDialog prefs;
  prefs.DoModal();
}

void InformApp::OnAppAbout()
{
  AboutDialog about;
  about.DoModal();
}

void InformApp::OnAppWebPage()
{
  ::ShellExecute(0,NULL,"http://inform7.com/",NULL,NULL,SW_SHOWNORMAL);
}

CFont* InformApp::GetFont(Fonts font)
{
  CFont& theFont = m_fonts[font];
  if ((HFONT)theFont == 0)
    theFont.CreatePointFont(10*GetFontSize(font),GetFontName(font));
  return &theFont;
}

const char* InformApp::GetFontName(Fonts font)
{
  return m_fontNames[font];
}

int InformApp::GetFontSize(Fonts font)
{
  return m_fontSizes[font];
}

CSize InformApp::MeasureFont(CFont* font)
{
  // Get the with and height of the font
  CDC* dc = AfxGetMainWnd()->GetDC();
  CFont* oldFont = dc->SelectObject(font);
  TEXTMETRIC metrics;
  dc->GetTextMetrics(&metrics);
  dc->SelectObject(oldFont);
  AfxGetMainWnd()->ReleaseDC(dc);

  // Cope with invalid font information in bad fonts
  if (metrics.tmAveCharWidth < 1)
    metrics.tmAveCharWidth = 8;

  return CSize(metrics.tmAveCharWidth,metrics.tmHeight);
}

CSize InformApp::MeasureText(CWnd* button)
{
  CString text;
  button->GetWindowText(text);
  CDC* dc = button->GetDC();
  CFont* oldFont = dc->SelectObject(button->GetFont());
  CSize size = dc->GetTextExtent(text);
  dc->SelectObject(oldFont);
  button->ReleaseDC(dc);
  return size;
}

COLORREF InformApp::GetColour(Colours colour)
{
  switch (colour)
  {
  case ColourBack:
    return RGB(0xff,0xff,0xff);
  case ColourText:
    return RGB(0x00,0x00,0x00);
  case ColourQuote:
    return RGB(0x0f,0x61,0xaa);
  case ColourQuoteBracket:
    return RGB(0x5f,0x66,0xaf);
  case ColourComment:
    return RGB(0x1b,0x7f,0x3f);
  case ColourError:
    return RGB(0xff,0x00,0x00);
  case ColourHighlight:
    return RGB(0x24,0xd4,0xd4);
  case ColourLocked:
    return RGB(0x00,0x00,0x00);
  case ColourUnlocked:
    return RGB(0xa0,0xa0,0xa0);
  case ColourSkeinInput:
    return RGB(0x68,0x65,0xff);
  case ColourTabBack:
    return RGB(0xf7,0xf3,0xe9);
  case ColourInform6Code:
    return RGB(0x60,0x60,0x60);
  case ColourBorder:
    return RGB(0xb0,0xb0,0xb0);
  case ColourTransDiffers:
    return RGB(0xff,0xf0,0xf0);
  case ColourTransSame:
    return RGB(0xf0,0xff,0xf0);
  case ColourTransNearlySame:
    return RGB(0xff,0xff,0xf0);
  case ColourTransInput:
    return RGB(0xe0,0xf0,0xff);
  case ColourTransUnset:
    return RGB(0xf0,0xf0,0xf0);
  case ColourTransPlayed:
    return RGB(0xff,0xff,0xb3);
  case ColourTransSelect:
    return RGB(0x66,0x66,0xff);
  case ColourHyperlink:
    return RGB(0x00,0x00,0xff);
  case ColourContents:
    return RGB(0xff,0xff,0xe8);
  case ColourContentsSelect:
    return RGB(0xc1,0xdb,0xfb);
  case ColourContentsBelow:
    return RGB(0xf0,0xf7,0xeb);
  case ColourI7XP:
    return RGB(0xff,0xff,0xe0);
  default:
    return RGB(0x00,0x00,0x00);
  }
}

COLORREF InformApp::BlendedColour(COLORREF col1, int rel1, COLORREF col2, int rel2)
{
  int r = ((rel1*GetRValue(col1))+(rel2*GetRValue(col2)))/(rel1+rel2);
  int g = ((rel1*GetGValue(col1))+(rel2*GetGValue(col2)))/(rel1+rel2);
  int b = ((rel1*GetBValue(col1))+(rel2*GetBValue(col2)))/(rel1+rel2);
  return RGB(min(r,255),min(g,255),min(b,255));
}

void InformApp::SetIcon(CWnd* wnd)
{
  wnd->SetIcon(LoadIcon(IDR_BIG_LOGO),TRUE);
  wnd->SetIcon(LoadIcon(IDR_SMALL_LOGO),FALSE);
}

CString InformApp::GetAppDir(void)
{
#ifdef DEBUG
  return "Z:\\Programs\\Adv\\Inform7\\Build";
#else
  // Get the path to this executable
  char path[_MAX_PATH];
  if (::GetModuleFileName(NULL,path,sizeof path) == 0)
    return "";

  // Strip off the executable name
  char* p = strrchr(path,'\\');
  if (p != NULL)
    *p = 0;
  return path;
#endif
}

CString InformApp::GetLastProjectDir(void)
{
  CRegKey registryKey;
  if (registryKey.Create(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW) == ERROR_SUCCESS)
  {
    char dir[MAX_PATH];
    ULONG len = sizeof dir;
    if (registryKey.QueryStringValue("Last Project",dir,&len) == ERROR_SUCCESS)
      return dir;
  }
  return m_home+"\\Inform\\Projects\\FirstProject";
}

CString InformApp::GetHomeDir(void)
{
  return m_home;
}

FileProtocol& InformApp::GetUrlProtocol(void)
{
  return m_protocol;
}

void InformApp::NewFrame(CFrameWnd* frame)
{
  if (m_pMainWnd == NULL)
    m_pMainWnd = frame;
  else
    m_frames.Add(frame);
}

void InformApp::FrameClosing(CFrameWnd* frame)
{
  // Is this a secondary frame?
  for (int i = 0; i < m_frames.GetSize(); i++)
  {
    if (m_frames[i] == frame)
    {
      m_frames.RemoveAt(i);
      return;
    }
  }

  // This must be the main frame
  ASSERT(m_pMainWnd == frame);
  if (m_pMainWnd == frame)
  {
    // If there are secondary frames, make one the main window
    if (!m_frames.IsEmpty())
    {
      m_pMainWnd = m_frames[0];
      m_frames.RemoveAt(0);
    }
  }
}

void InformApp::GetWindowFrames(CArray<CFrameWnd*>& frames)
{
  if (m_pMainWnd)
  {
    if (m_pMainWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
      frames.Add((CFrameWnd*)m_pMainWnd);
  }
  frames.Append(m_frames);
}

void InformApp::SendAllFrames(Changed changed, int value)
{
  if (changed == Preferences)
  {
    SetFonts();
    ClearScaledImages();
    ReportHtml::SetIEPreferences(NULL);
  }

  CArray<CFrameWnd*> frames;
  GetWindowFrames(frames);

  for (int i = 0; i < frames.GetSize(); i++)
  {
    CFrameWnd* frame = frames[i];
    if (frame->IsKindOf(RUNTIME_CLASS(ProjectFrame)))
      ((ProjectFrame*)frame)->SendChanged(changed,value);
    else if (frame->IsKindOf(RUNTIME_CLASS(ExtensionFrame)))
      ((ExtensionFrame*)frame)->SendChanged(changed,value);
  }
}

namespace {

// JPEG decoding error handling

struct JPEGErrorInfo
{
  struct jpeg_error_mgr base;
  jmp_buf errorJump;
};

void errorJPEGExit(j_common_ptr cinfo)
{
  (*cinfo->err->output_message)(cinfo);
  struct JPEGErrorInfo* error = (struct JPEGErrorInfo*)cinfo->err;
  longjmp(error->errorJump,1);
}

void outputJPEGMessage(j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message)(cinfo,buffer);
  TRACE("JPEG: %s\n",buffer);
}

} // unnamed namespace

CDibSection* InformApp::GetImage(const char* path)
{
  // Check if it's a PNG file
  CStdioFile imageFile;
  if (!imageFile.Open(path,CFile::modeRead|CFile::typeBinary))
    return 0;
  png_byte fileHeader[8];
  imageFile.Read(fileHeader,8);
  bool isPNG = (png_sig_cmp(fileHeader,0,8) == 0);

  if (isPNG)
  {
    // Prepare to read the PNG file
    png_structp png_ptr = png_create_read_struct
      (PNG_LIBPNG_VER_STRING,(png_voidp)NULL,NULL,NULL);
    if (png_ptr == NULL)
      return 0;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
      png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
      return 0;
    }
    png_infop end_info = png_create_info_struct(png_ptr);
    if (end_info == NULL)
    {
      png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
      return 0;
    }

    // Set up the point to return to in case of error
    png_bytep* rowPointers = NULL;
    if (setjmp(png_jmpbuf(png_ptr)))
    {
      if (rowPointers)
        delete[] rowPointers;
      png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
      return 0;
    }

    png_init_io(png_ptr,imageFile.m_pStream);
    png_set_sig_bytes(png_ptr,8);
    png_read_info(png_ptr,info_ptr);

    // Get details of the image
    png_uint_32 width = png_get_image_width(png_ptr,info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr,info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr,info_ptr);
    int color_type = png_get_color_type(png_ptr,info_ptr);

    // Set up transforms to the required format
    if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
      png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(png_ptr);
    double gamma;
    if (png_get_gAMA(png_ptr,info_ptr,&gamma))
      png_set_gamma(png_ptr,2.2,gamma);
    if (bit_depth == 16)
      png_set_strip_16(png_ptr);
    if (bit_depth < 8)
      png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);
    png_set_bgr(png_ptr);
    png_set_filler(png_ptr,0xFF,PNG_FILLER_AFTER);

    // Create a bitmap
    HDC dc = ::GetDC(NULL);
    CDibSection* dib = new CDibSection();
    BOOL created = dib->CreateBitmap(dc,width,height);
    ::ReleaseDC(NULL,dc);
    if (!created)
    {
      png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
      return NULL;
    }

    // Read in the image
    rowPointers = new png_bytep[height];
    for (int i = 0; i < (int)height; i++)
      rowPointers[i] = ((png_bytep)dib->GetBits())+(width*i*4);
    png_read_image(png_ptr,rowPointers);
    png_read_end(png_ptr,end_info);
    delete[] rowPointers;

    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    return dib;
  }
  else
  {
    imageFile.SeekToBegin();

    struct jpeg_decompress_struct info;
    struct JPEGErrorInfo error;

    // Initialize the error handling
    info.err = jpeg_std_error(&(error.base));
    error.base.error_exit = errorJPEGExit;
    error.base.output_message = outputJPEGMessage;
    if (setjmp(error.errorJump))
    {
      jpeg_destroy_decompress(&info);
      return NULL;
    }

    // Set up the decompression
    jpeg_create_decompress(&info);
    jpeg_stdio_src(&info,imageFile.m_pStream);

    // Read the image attributes
    jpeg_read_header(&info,TRUE);
    jpeg_calc_output_dimensions(&info);
    int width = info.output_width;
    int height = info.output_height;

    // Force RGB output
    info.out_color_space = JCS_RGB;

    // Create a bitmap
    HDC dc = ::GetDC(NULL);
    CDibSection* dib = new CDibSection();
    BOOL created = dib->CreateBitmap(dc,width,height);
    ::ReleaseDC(NULL,dc);
    if (!created)
    {
      jpeg_destroy_decompress(&info);
      return NULL;
    }

    // Get an output buffer
    JSAMPARRAY buffer = (*info.mem->alloc_sarray)
      ((j_common_ptr)&info,JPOOL_IMAGE,width*3,1);

    // Read in the image
    jpeg_start_decompress(&info);
    while ((int)info.output_scanline < height)
    {
      jpeg_read_scanlines(&info,buffer,1);

      BYTE* pixelRow = ((BYTE*)dib->GetBits())+(width*(info.output_scanline-1)*4);
      for (int i = 0; i < width; i++)
      {
        pixelRow[(i*4)+0] = (*buffer)[(i*3)+2];
        pixelRow[(i*4)+1] = (*buffer)[(i*3)+1];
        pixelRow[(i*4)+2] = (*buffer)[(i*3)+0];
        pixelRow[(i*4)+3] = 0xFF;
      }
    }
    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);
    return dib;
  }
}

CSize InformApp::GetImageSize(const char* path)
{
  CSize size(0,0);

  // Check if it's a PNG file
  CStdioFile imageFile;
  if (!imageFile.Open(path,CFile::modeRead|CFile::typeBinary))
    return size;
  png_byte fileHeader[8];
  imageFile.Read(fileHeader,8);
  bool isPNG = (png_sig_cmp(fileHeader,0,8) == 0);

  if (isPNG)
  {
    png_structp png_ptr = png_create_read_struct
      (PNG_LIBPNG_VER_STRING,(png_voidp)NULL,NULL,NULL);
    if (png_ptr == NULL)
      return size;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
      png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
      return size;
    }
    png_infop end_info = png_create_info_struct(png_ptr);
    if (end_info == NULL)
    {
      png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
      return size;
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
      png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
      return size;
    }

    png_init_io(png_ptr,imageFile.m_pStream);
    png_set_sig_bytes(png_ptr,8);
    png_read_info(png_ptr,info_ptr);

    size.cx = png_get_image_width(png_ptr,info_ptr);
    size.cy = png_get_image_height(png_ptr,info_ptr);

    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    return size;
  }
  else
  {
    imageFile.SeekToBegin();

    struct jpeg_decompress_struct info;
    struct JPEGErrorInfo error;

    info.err = jpeg_std_error(&(error.base));
    error.base.error_exit = errorJPEGExit;
    error.base.output_message = outputJPEGMessage;
    if (setjmp(error.errorJump))
    {
      jpeg_destroy_decompress(&info);
      return size;
    }

    jpeg_create_decompress(&info);
    jpeg_stdio_src(&info,imageFile.m_pStream);

    jpeg_read_header(&info,TRUE);
    jpeg_calc_output_dimensions(&info);

    size.cx = info.output_width;
    size.cy = info.output_height;

    jpeg_destroy_decompress(&info);
    return size;
  }
}

CDibSection* InformApp::GetCachedImage(const char* name)
{
  // Is this image already loaded?
  std::map<std::string,CDibSection*>::iterator it = m_bitmaps.find(name);
  if (it != m_bitmaps.end())
    return it->second;

  // Get the path to the PNG image and load it
  CString path;
  path.Format("%s\\Images\\%s.png",(LPCSTR)GetAppDir(),name);
  CDibSection* dib = GetImage(path);
  if (dib == NULL)
  {
    path.Format("%s\\Documentation\\%s.png",(LPCSTR)GetAppDir(),name);
    dib = GetImage(path);
  }

  // Cache and return it
  if (dib != NULL)
    m_bitmaps[name] = dib;
  return dib;
}

void InformApp::CacheImage(const char* name, CDibSection* dib)
{
  ASSERT(m_bitmaps.count(name) == 0);
  m_bitmaps[name] = dib;
}

CDibSection* InformApp::CreateScaledImage(CDibSection* fromImage, double scaleX, double scaleY)
{
  // Work out the scaled image size
  CSize fromSize = fromImage->GetSize();
  CSize newSize((int)(fromSize.cx*scaleX),(int)(fromSize.cy*scaleY));

  // Create a scaled image
  CDibSection* newImage = new CDibSection();
  CDC* dc = AfxGetMainWnd()->GetDC();
  BOOL created = newImage->CreateBitmap(dc->GetSafeHdc(),newSize.cx,newSize.cy);
  ASSERT(created);
  AfxGetMainWnd()->ReleaseDC(dc);

  // Scale and stretch the image
  ScaleGfx(fromImage->GetBits(),fromSize.cx,fromSize.cy,
    newImage->GetBits(),newSize.cx,newSize.cy);
  return newImage;
}

void InformApp::ClearScaledImages(void)
{
  static const char* names[] = 
  {
    // From ContentsPane
    "Contents-circle-scaled-0",
    "Contents-circle-scaled-1",
    "Contents-circle-scaled-2",
    // From SkeinWindow
    "Skein-played-scaled",
    "Skein-played-scaled-dark",
    "Skein-unplayed-scaled",
    "Skein-unplayed-scaled-dark",
    "Skein-annotation-scaled",
    "SkeinDiffersBadge-scaled",
    // From StopButton
    "Stop-scaled",
    "Stop-scaled-dark",
  };

  for (int i = 0; i < sizeof names / sizeof names[0]; i++)
  {
    std::map<std::string,CDibSection*>::iterator it = m_bitmaps.find(names[i]);
    if (it != m_bitmaps.end())
    {
      delete it->second;
      m_bitmaps.erase(it);
    }
  }
}

double InformApp::GetIEVersion(void)
{
  CRegKey ieKey;
  if (ieKey.Open(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Internet Explorer",KEY_READ) == ERROR_SUCCESS)
  {
    // Return the major and minor components of the version number
    char version[256];
    ULONG len = sizeof version;
    if (ieKey.QueryStringValue("svcVersion",version,&len) == ERROR_SUCCESS)
    {
      if (len > 0)
        return atof(version);
    }
    len = sizeof version;
    if (ieKey.QueryStringValue("Version",version,&len) == ERROR_SUCCESS)
    {
      if (len > 0)
        return atof(version);
    }
  }
  return 0.0;
}

void InformApp::CheckInstalledVersions(void)
{
  if (GetIEVersion() < 5.5)
  {
    CString msg;
    msg.Format("Internet Explorer version 5.5 or higher must be installed.");
    AfxMessageBox(msg,MB_ICONSTOP|MB_OK);
    exit(0);
  }

  if (theOS.GetDllVersion("comctl32.dll") < DLLVERSION(5,81))
  {
    AfxMessageBox("comctrl32 5.81 or higher is required.\n",MB_ICONSTOP|MB_OK);
    exit(0);
  }

  CComPtr<IXMLDOMDocument> doc;
  if (FAILED(doc.CoCreateInstance(CLSID_DOMDocument)))
  {
    AfxMessageBox(
      "Microsoft Data Access Components (MDAC) 2.6 or higher is required.\n"
      "MDAC can be downloaded from http://www.microsoft.com/downloads/",MB_ICONSTOP|MB_OK);
    exit(0);
  }
}

int InformApp::GetColourDepth(void)
{
  HDC dc = ::GetDC(NULL);
  int bits = ::GetDeviceCaps(dc,BITSPIXEL);
  ::ReleaseDC(NULL,dc);
  return bits;
}

void InformApp::RunMessagePump(void)
{
  LONG count = 0;
  while (OnIdle(count++));

  MSG msg;
  while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
  {
    if (msg.message == WM_COMMAND)
      ::PeekMessage(&msg,NULL,0,0,PM_REMOVE);
    else if (PumpMessage() == FALSE)
      exit(ExitInstance());
  }

  if (IsWaitCursor())
    RestoreWaitCursor();
}

HANDLE InformApp::CreateProcess(const char* dir, CString& command, STARTUPINFO& start, bool debug)
{
  BOOL flags = CREATE_NO_WINDOW;
  if (debug)
    flags |= DEBUG_PROCESS;
  if (m_job)
    flags |= CREATE_BREAKAWAY_FROM_JOB;

  // Actually create the process
  PROCESS_INFORMATION process;
  char* cmdLine = command.GetBuffer();
  BOOL created = ::CreateProcess(NULL,cmdLine,NULL,NULL,TRUE,flags,NULL,dir,&start,&process);
  command.ReleaseBuffer();

  if (created)
  {
    // Close the thread handle that is never used
    ::CloseHandle(process.hThread);

    // If there is a job, assign the process to it
    if (m_job)
      VERIFY(theOS.AssignProcessToJobObject(m_job,process.hProcess));

    return process.hProcess;
  }
  return INVALID_HANDLE_VALUE;
}

HANDLE InformApp::RunCensus(void)
{
  CString command, dir = GetAppDir();
  command.Format("\"%s\\Compilers\\ni\" -internal \"%s\\Internal\" -census",
    (LPCSTR)dir,(LPCSTR)dir);

  STARTUPINFO start;
  ::ZeroMemory(&start,sizeof start);
  start.cb = sizeof start;
  start.wShowWindow = SW_HIDE;
  start.dwFlags = STARTF_USESHOWWINDOW;
  return CreateProcess(NULL,command,start,true);
}

int InformApp::RunCommand(const char* dir, CString& command, OutputSink& output)
{
  CWaitCursor wc;

  // Create a pipe to read the command's output
  SECURITY_ATTRIBUTES security;
  ::ZeroMemory(&security,sizeof security);
  security.nLength = sizeof security;
  security.bInheritHandle = TRUE;
  HANDLE pipeRead, pipeWrite;
  ::CreatePipe(&pipeRead,&pipeWrite,&security,0);

  // Use the pipe for standard I/O and, for Windows 9X, make sure that the
  // console window is hidden
  STARTUPINFO start;
  ::ZeroMemory(&start,sizeof start);
  start.cb = sizeof start;
  start.hStdOutput = pipeWrite;
  start.hStdError = pipeWrite;
  start.wShowWindow = SW_HIDE;
  start.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;

  // Create the process for the command
  HANDLE process = CreateProcess(dir,command,start,true);

  DWORD result = 512;
  if (process != INVALID_HANDLE_VALUE)
  {
    // Wait for the process to complete
    result = STILL_ACTIVE;
    while (result == STILL_ACTIVE)
    {
      // Check if the process should be stopped
      if (output.WantStop())
        ::TerminateProcess(process,20);

      // Wait for a window message or 100ms to elapse
      ::MsgWaitForMultipleObjects(0,NULL,FALSE,100,QS_ALLINPUT);
      RunMessagePump();

      // Is there any more output?
      DWORD available = 0;
      ::PeekNamedPipe(pipeRead,NULL,0,NULL,&available,NULL);
      if (available > 0)
      {
        // Read the output from the pipe
        CString msg;
        char* buffer = msg.GetBuffer(available);
        DWORD read = 0;
        ::ReadFile(pipeRead,buffer,available,&read,NULL);
        msg.ReleaseBuffer(read);
        output.Output(msg);
      }

      // Get the exit code
      ::GetExitCodeProcess(process,&result);
    }

    // Wait for the process to exit
    ::WaitForSingleObject(process,1000);
    ::CloseHandle(process);

    // Read any final output from the pipe
    DWORD available = 0;
    ::PeekNamedPipe(pipeRead,NULL,0,NULL,&available,NULL);
    if (available > 0)
    {
      CString msg;
      char* buffer = msg.GetBuffer(available);
      DWORD read = 0;
      ::ReadFile(pipeRead,buffer,available,&read,NULL);
      msg.ReleaseBuffer(read);
      output.Output(msg);
    }
  }

  ::CloseHandle(pipeRead);
  ::CloseHandle(pipeWrite);
  return result;
}

void InformApp::WriteLog(const char* msg)
{
  FILE* f = fopen(m_home+LOG_FILE,"at");
  if (f != NULL)
  {
    fprintf(f,"%s\n",msg);
    fclose(f);
  }
}

bool InformApp::IsWaitCursor(void)
{
  return (m_nWaitCursorCount > 0);
}

CStringW InformApp::GetProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR defaultValue)
{
  if (theOS.IsWindows9X())
  {
    CString entryA(entry), defaultValueA(defaultValue);
    return CStringW(CWinApp::GetProfileString(section,entryA,defaultValueA));
  }

  if (m_pszRegistryKey == NULL)
    return defaultValue;
  HKEY secKey = GetSectionKey(section);
  if (secKey == NULL)
    return defaultValue;

  CStringW value;
  DWORD type, count;
  LONG result = ::RegQueryValueExW(secKey,entry,NULL,&type,NULL,&count);
  if (result == ERROR_SUCCESS)
  {
    result = ::RegQueryValueExW(secKey,entry,NULL,&type,
      (LPBYTE)value.GetBuffer(count/sizeof(WCHAR)),&count);
    value.ReleaseBuffer();
  }
  ::RegCloseKey(secKey);
  if (result == ERROR_SUCCESS)
    return value;
  return defaultValue;
}

BOOL InformApp::WriteProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR value)
{
  if (theOS.IsWindows9X())
  {
    CString entryA(entry), valueA(value);
    return CWinApp::WriteProfileString(section,entryA,valueA);
  }

  if (m_pszRegistryKey == NULL)
    return FALSE;
  HKEY secKey = GetSectionKey(section);
  if (secKey == NULL)
    return FALSE;

  LONG result = ::RegSetValueExW(secKey,entry,NULL,REG_SZ,
    (LPBYTE)value,(lstrlenW(value)+1)*sizeof(WCHAR));
  ::RegCloseKey(secKey);
  return (result == ERROR_SUCCESS);
}

void InformApp::FindExtensions(void)
{
  m_extensions.clear();
  for (int i = 0; i < 2; i++)
  {
    CString path;
    switch (i)
    {
    case 0:
      path.Format("%s\\Internal\\Extensions\\*.*",(LPCSTR)GetAppDir());
      break;
    case 1:
      path.Format("%s\\Inform\\Extensions\\*.*",(LPCSTR)GetHomeDir());
      break;
    default:
      ASSERT(FALSE);
      break;
    }

    CFileFind find;
    BOOL finding = find.FindFile(path);
    while (finding)
    {
      finding = find.FindNextFile();
      if (!find.IsDots() && find.IsDirectory())
      {
        CString author = find.GetFileName();
        if (author == "Reserved")
          continue;
        if ((author.GetLength() > 0) && (author.GetAt(0) == '.'))
          continue;

        path.Format("%s\\*.*",(LPCSTR)find.GetFilePath());
        CFileFind find;
        BOOL finding = find.FindFile(path);
        while (finding)
        {
          finding = find.FindNextFile();
          if (!find.IsDirectory())
          {
            CString ext = ::PathFindExtension(find.GetFilePath());
            if (ext.CompareNoCase(".i7x") == 0)
              m_extensions.push_back(ExtLocation(author,find.GetFileTitle(),(i == 0),find.GetFilePath()));
            else if (ext.IsEmpty() && (i == 1))
            {
              // Rename an old-style extension (with no file extension) to end with ".i7x"
              CString newPath = find.GetFilePath();
              newPath.Append(".i7x");
              if (::MoveFile(find.GetFilePath(),newPath))
                m_extensions.push_back(ExtLocation(author,find.GetFileTitle(),(i == 0),newPath));
            }
          }
        }
        find.Close();
      }
    }
    find.Close();
  }
  std::sort(m_extensions.begin(),m_extensions.end());
}

const std::vector<InformApp::ExtLocation>& InformApp::GetExtensions(void)
{
  return m_extensions;
}

const InformApp::ExtLocation* InformApp::GetExtension(const char* author, const char* title)
{
  for (std::vector<ExtLocation>::const_iterator it = m_extensions.begin(); it != m_extensions.end(); ++it)
  {
    if ((it->author == author) && (it->title == title))
      return &(*it);
  }
  return NULL;
}

static char hex(int digit)
{
  if (digit < 10)
    return '0'+digit;
  return 'a'+digit-10;
}

void InformApp::SetMyDocuments(void)
{
  CString homeName;
  homeName.Format("%s\\home.txt",(LPCSTR)GetAppDir());
  FILE* homeFile = fopen(homeName,"rt");
  if (homeFile != NULL)
  {
    char homeLine[256];
    fgets(homeLine,sizeof homeLine,homeFile);
    fclose(homeFile);
    m_home = homeLine;
    m_home.Trim();
    m_home.Trim('\"');
  }

  if (!m_home.IsEmpty())
  {
    DWORD attrs = ::GetFileAttributes(m_home);
    if ((attrs == INVALID_FILE_ATTRIBUTES) ||
       ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0) || ((attrs & FILE_ATTRIBUTE_READONLY) != 0) ||
       !CreateHomeDirs())
    {
      CString msg;
      msg.Format(
        "Found a \"home.txt\" file redirecting the home directory to\n\n    %s\n\n"
        "but this is not accessible, so the default will be used instead.",(LPCSTR)m_home);
      AfxMessageBox(msg,MB_ICONWARNING|MB_OK);
      m_home.Empty();
    }
  }
  if (m_home.IsEmpty())
    m_home = theOS.SHGetFolderPath(0,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT);

  int len = m_home.GetLength();
  if (len > 0)
  {
    if (m_home.GetAt(len-1) == '\\')
      m_home.Truncate(len-1);

    ::SetEnvironmentVariable("HOME",m_home);
    CreateHomeDirs();
  }

  CString desktop = theOS.SHGetFolderPath(0,CSIDL_DESKTOP,NULL,SHGFP_TYPE_CURRENT);
  if (desktop.IsEmpty() == FALSE)
    ::SetEnvironmentVariable("DESKTOP",desktop);
}

bool InformApp::CreateHomeDirs(void)
{
  if (::CreateDirectory(m_home+"\\Inform",NULL) == 0)
  {
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      return false;
  }
  if (::CreateDirectory(m_home+"\\Inform\\Extensions",NULL) == 0)
  {
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      return false;
  }
  if (::CreateDirectory(m_home+"\\Inform\\Projects",NULL) == 0)
  {
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      return false;
  }
  if (::CreateDirectory(m_home+"\\Inform\\Templates",NULL) == 0)
  {
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      return false;
  }
  return true;
}

static int CALLBACK EnumFontProc(ENUMLOGFONTEX*, NEWTEXTMETRICEX* ,DWORD, LPARAM found)
{
  *((bool*)found) = true;
  return 0;
}

void InformApp::SetFonts(void)
{
  // Clear existing fonts
  m_fonts[FontDisplay].DeleteObject();
  m_fonts[FontFixedWidth].DeleteObject();

  // Look for registry settings
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_READ) == ERROR_SUCCESS)
  {
    char fontName[MAX_PATH];
    ULONG len = sizeof fontName;
    if (registryKey.QueryStringValue("Font Name",fontName,&len) == ERROR_SUCCESS)
      m_fontNames[FontDisplay] = fontName;
    len = sizeof fontName;
    if (registryKey.QueryStringValue("Fixed Font Name",fontName,&len) == ERROR_SUCCESS)
      m_fontNames[FontFixedWidth] = fontName;
    DWORD fontSize = 0;
    if (registryKey.QueryDWORDValue("Font Size",fontSize) == ERROR_SUCCESS)
    {
      m_fontSizes[FontDisplay] = fontSize;
      m_fontSizes[FontFixedWidth] = fontSize;
    }
  }

  // Get a device context for the display
  CWnd* wnd = CWnd::GetDesktopWindow();
  CDC* dc = wnd->GetDC();

  if (m_fontNames[FontFixedWidth].IsEmpty())
  {
    const char* fixedFonts[] =
    {
      "Consolas",
      "Lucida Console",
      "Courier New",
      "Courier"
    };

    // Search the list of known fixed width fonts for a match
    LOGFONT fontInfo;
    ::ZeroMemory(&fontInfo,sizeof fontInfo);
    fontInfo.lfCharSet = DEFAULT_CHARSET;
    bool found = false;
    for (int i = 0; i < sizeof fixedFonts / sizeof fixedFonts[0]; i++)
    {
      strcpy(fontInfo.lfFaceName,fixedFonts[i]);
      ::EnumFontFamiliesEx(dc->GetSafeHdc(),&fontInfo,(FONTENUMPROC)EnumFontProc,(LPARAM)&found,0);
      if (found)
      {
        m_fontNames[FontFixedWidth] = fontInfo.lfFaceName;
        break;
      }
    }
  }

  // Get desktop settings, and use the message font as a default
  NONCLIENTMETRICS ncm;
  ::ZeroMemory(&ncm,sizeof ncm);
  ncm.cbSize = sizeof ncm;
  ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof ncm,&ncm,0);
  int fontSize = abs(MulDiv(ncm.lfMessageFont.lfHeight,72,dc->GetDeviceCaps(LOGPIXELSY)));
  fontSize = max(fontSize,(theOS.GetWindowsVersion() < 6) ? 8 : 9);
  for (int i = 0; i < 3; i++)
  {
    if (m_fontNames[i].IsEmpty())
      m_fontNames[i] = ncm.lfMessageFont.lfFaceName;
    if (m_fontSizes[i] == 0)
      m_fontSizes[i] = fontSize;
  }

  if ((HFONT)m_fonts[FontPanel] == 0)
  {
    LOGFONT fontInfo;
    ::ZeroMemory(&fontInfo,sizeof fontInfo);
    GetFont(InformApp::FontSystem)->GetLogFont(&fontInfo);
    fontInfo.lfHeight = (LONG)(Panel::GetFontScale(wnd,dc) * fontInfo.lfHeight);

    m_fontNames[FontPanel] = m_fontNames[FontSystem];
    m_fontSizes[FontPanel] = abs(::MulDiv(fontInfo.lfHeight,72,dc->GetDeviceCaps(LOGPIXELSY)));
    m_fonts[FontPanel].CreateFontIndirect(&fontInfo);
  }

  // Release the desktop device context
  wnd->ReleaseDC(dc);
}

InformApp::ExtLocation::ExtLocation(const char* a, const char* t, bool s, const char* p)
  : author(a), title(t), system(s), path(p)
{
}

bool InformApp::ExtLocation::operator<(const ExtLocation& el) const
{
  if (author != el.author)
    return author < el.author;
  if (system != el.system)
    return system > el.system;
  return title < el.title;
}
