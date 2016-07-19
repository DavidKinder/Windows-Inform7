#include "stdafx.h"
#include "Inform.h"
#include "OSLayer.h"

#include <fstream>
#include <sstream>
#include <dbghelp.h>

#pragma warning(disable : 4748)

namespace {

typedef BOOL(__stdcall *STACKWALK64)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID,
  PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64,
  PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
typedef BOOL(__stdcall *SYMGETLINEFROMADDR64)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
typedef BOOL(__stdcall *SYMGETMODULEINFO64)(HANDLE, DWORD64, PIMAGEHLP_MODULE64);
typedef BOOL(__stdcall *SYMGETSYMFROMADDR64)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
typedef BOOL(__stdcall *SYMINITIALIZE)(HANDLE, LPCSTR, BOOL);
typedef BOOL(__stdcall *SYMCLEANUP)(HANDLE);
typedef DWORD(__stdcall *SYMSETOPTIONS)(DWORD);

HMODULE debugDll = 0;
STACKWALK64 stackWalk64 = NULL;
PFUNCTION_TABLE_ACCESS_ROUTINE64 symFunctionTableAccess64 = NULL;
SYMGETLINEFROMADDR64 symGetLineFromAddr64 = NULL;
PGET_MODULE_BASE_ROUTINE64 symGetModuleBase64 = NULL;
SYMGETMODULEINFO64 symGetModuleInfo64 = NULL;
SYMGETSYMFROMADDR64 symGetSymFromAddr64 = NULL;
SYMINITIALIZE symInitialize = NULL;
SYMCLEANUP symCleanup = NULL;
SYMSETOPTIONS symSetOptions = NULL;

bool GotFunctions(void)
{
  if (stackWalk64 == NULL)
    return false;
  if (symFunctionTableAccess64 == NULL)
    return false;
  if (symGetLineFromAddr64 == NULL)
    return false;
  if (symGetModuleBase64 == NULL)
    return false;
  if (symGetModuleInfo64 == NULL)
    return false;
  if (symGetSymFromAddr64 == NULL)
    return false;
  if (symInitialize == NULL)
    return false;
  if (symCleanup == NULL)
    return false;
  if (symSetOptions == NULL)
    return false;
  return true;
}

void PrintStackTrace(HANDLE process, HANDLE thread, PCONTEXT context, std::ostream& log)
{
  // Load the debug help DLL, if available, and find entry points
  if (debugDll == 0)
  {
    debugDll = ::LoadLibrary("dbghelp.dll");
    stackWalk64 = (STACKWALK64)::GetProcAddress(debugDll,"StackWalk64");
    symFunctionTableAccess64 = (PFUNCTION_TABLE_ACCESS_ROUTINE64)::GetProcAddress(debugDll,"SymFunctionTableAccess64");
    symGetLineFromAddr64 = (SYMGETLINEFROMADDR64)::GetProcAddress(debugDll,"SymGetLineFromAddr64");
    symGetModuleBase64 = (PGET_MODULE_BASE_ROUTINE64)::GetProcAddress(debugDll,"SymGetModuleBase64");
    symGetModuleInfo64 = (SYMGETMODULEINFO64)::GetProcAddress(debugDll,"SymGetModuleInfo64");
    symGetSymFromAddr64 = (SYMGETSYMFROMADDR64)::GetProcAddress(debugDll,"SymGetSymFromAddr64");
    symInitialize = (SYMINITIALIZE)::GetProcAddress(debugDll,"SymInitialize");
    symCleanup = (SYMCLEANUP)::GetProcAddress(debugDll,"SymCleanup");
    symSetOptions = (SYMSETOPTIONS)::GetProcAddress(debugDll,"SymSetOptions");
  }
  if (!GotFunctions())
    return;

  // Check that this is an X86 processor
  SYSTEM_INFO si;
  ::GetSystemInfo(&si);
  if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL)
    return;

  // Set symbol options
  (*symSetOptions)(SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);

  // Load any symbols
  CString symPath = theApp.GetAppDir()+"\\Symbols";
  if ((*symInitialize)(process,symPath,TRUE) == 0)
    return;

  // Set up the initial stack frame
  STACKFRAME64 frame;
  ::ZeroMemory(&frame,sizeof frame);
  frame.AddrPC.Offset = context->Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context->Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context->Esp;
  frame.AddrStack.Mode = AddrModeFlat;

  while (true)
  {
    // Get the next stack frame
    if ((*stackWalk64)(IMAGE_FILE_MACHINE_I386,process,thread,&frame,context,
                       NULL,symFunctionTableAccess64,symGetModuleBase64,NULL) == 0)
      break;

    // Check the stack frame
    if (frame.AddrFrame.Offset == 0)
      break;

    // Get information on the module containing the address
    IMAGEHLP_MODULE64 info;
    info.SizeOfStruct = sizeof (IMAGEHLP_MODULE64);
    BOOL gotModuleInfo = (symGetModuleInfo64)(process,frame.AddrPC.Offset,&info);
    if (!gotModuleInfo)
    {
      info.SizeOfStruct = sizeof (IMAGEHLP_MODULE);
      gotModuleInfo = (symGetModuleInfo64)(process,frame.AddrPC.Offset,&info);
    }

    // Set up a symbol buffer
    BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL64)+512];
    PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)symbolBuffer;
    symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    symbol->MaxNameLength = 512;

    // Get the symbol
    DWORD64 displacement1 = 0;
    BOOL gotSymbol = (*symGetSymFromAddr64)(process,frame.AddrPC.Offset,&displacement1,symbol);

    // Get the file and line number
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof line;
    DWORD displacement2 = 0;
    BOOL gotLine = (*symGetLineFromAddr64)(process,frame.AddrPC.Offset,&displacement2,&line);

    // Print out the stack trace
    if (!gotModuleInfo && !gotSymbol)
      continue;
    if (gotModuleInfo)
      log << info.ModuleName << '!';
    log << (gotSymbol ? symbol->Name : "<unknown>") << "()";
    if (gotLine)
      log << " at " << line.FileName << " line " << std::dec << line.LineNumber;
    log << std::endl;
  }

  (*symCleanup)(process);
}

void LogStackTrace(void)
{
  HANDLE process = ::GetCurrentProcess();
  HANDLE thread = ::GetCurrentThread();

  // Get the thread's context
  CONTEXT context;
  ::ZeroMemory(&context,sizeof context);
  context.ContextFlags = CONTEXT_FULL;
  {
    __asm    call x
    __asm x: pop eax
    __asm    mov context.Eip, eax
    __asm    mov context.Ebp, ebp
    __asm    mov context.Esp, esp
  }

  // Create a stream to write to the log file
  CString logPath = theOS.SHGetFolderPath(0,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT);
  logPath = logPath + LOG_FILE;
  std::ofstream log(logPath);

  PrintStackTrace(process,thread,&context,log);
  log.close();
}

} // unnamed namespace

std::string GetStackTrace(HANDLE process, HANDLE thread)
{
  CONTEXT context;
  ::ZeroMemory(&context,sizeof context);
  context.ContextFlags = CONTEXT_FULL;
  if (::GetThreadContext(thread,&context))
  {
    std::ostringstream log;
    PrintStackTrace(process,thread,&context,log);
    return log.str();
  }
  return "";
}

extern "C" void FatalError(void)
{
  LogStackTrace();
  ::MessageBox(0,"A fatal error has occurred, see \"i7log.txt\" for details.",
    INFORM_TITLE,MB_ICONERROR|MB_OK);
  ::ExitProcess(1);
}
