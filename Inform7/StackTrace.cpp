#include "stdafx.h"
#include "Inform.h"
#include "OSLayer.h"

#include <fstream>
#include <dbghelp.h>

namespace {

typedef BOOL(__stdcall *STACKWALK64)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID,
  PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64,
  PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
typedef BOOL(__stdcall *SYMGETLINEFROMADDR64)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
typedef BOOL(__stdcall *SYMGETMODULEINFO64)(HANDLE, DWORD64, PIMAGEHLP_MODULE64);
typedef BOOL(__stdcall *SYMGETSYMFROMADDR64)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
typedef BOOL(__stdcall *SYMINITIALIZE)(HANDLE, PSTR, BOOL);
typedef DWORD(__stdcall *SYMSETOPTIONS)(DWORD);

HMODULE debugDll = 0;
STACKWALK64 stackWalk64 = NULL;
PFUNCTION_TABLE_ACCESS_ROUTINE64 symFunctionTableAccess64 = NULL;
SYMGETLINEFROMADDR64 symGetLineFromAddr64 = NULL;
PGET_MODULE_BASE_ROUTINE64 symGetModuleBase64 = NULL;
SYMGETMODULEINFO64 symGetModuleInfo64 = NULL;
SYMGETSYMFROMADDR64 symGetSymFromAddr64 = NULL;
SYMINITIALIZE symInitialize = NULL;
SYMSETOPTIONS symSetOptions = NULL;

void PrintStackTrace(void)
{
  HANDLE process = ::GetCurrentProcess();
  HANDLE thread = ::GetCurrentThread();

  // Create a stream to write to the log file
  CString logPath = theOS.SHGetFolderPath(0,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT);
  logPath = logPath + LOG_FILE;
  std::ofstream log(logPath);

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
    symSetOptions = (SYMSETOPTIONS)::GetProcAddress(debugDll,"SymSetOptions");
  }
  if (!stackWalk64 && !symFunctionTableAccess64 && !symGetLineFromAddr64 && !symGetModuleBase64 &&
      !symGetModuleInfo64 && !symGetSymFromAddr64 && !symInitialize && !symSetOptions)
  {
    log << "Failed to get debughlp entry points" << std::endl;
    log.close();
    return;
  }

  // Check that this is an X86 processor
  SYSTEM_INFO si;
  ::GetSystemInfo(&si);
  if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL)
  {
    log << "Processor architecture is not X86" << std::endl;
    log.close();
    return;
  }

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

  // Set symbol options
  (*symSetOptions)(SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);

  // Load any symbols
  if ((*symInitialize)(process,NULL,TRUE) == 0)
  {
    log << "Failed to get load debugging symbols" << std::endl;
    log.close();
    return;
  }

  // Set up the initial stack frame
  STACKFRAME64 frame;
  ::ZeroMemory(&frame,sizeof frame);
  frame.AddrPC.Offset = context.Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.Esp;
  frame.AddrStack.Mode = AddrModeFlat;

  log << "Address  Frame    Function" << std::endl;
  while (true)
  {
    // Get the next stack frame
    if ((*stackWalk64)(IMAGE_FILE_MACHINE_I386,process,thread,&frame,&context,
                       NULL,symFunctionTableAccess64,symGetModuleBase64,NULL) == 0)
      break;

    // Check the stack frame
    if (frame.AddrFrame.Offset == 0)
      break;

    // Output the program counter and the stack frame
    log.width(8);
    log.fill('0');
    log << std::hex << frame.AddrPC.Offset << " ";
    log.width(8);
    log.fill('0');
    log << std::hex << frame.AddrFrame.Offset << " ";

    // Get information on the module containing the address
    IMAGEHLP_MODULE64 info;
    info.SizeOfStruct = sizeof info;
    BOOL gotModuleInfo = (symGetModuleInfo64)(process,frame.AddrPC.Offset,&info);

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
    if (gotModuleInfo && !gotLine)
      log << info.ModuleName << "!";
    log << (gotSymbol ? symbol->Name : "<unknown>");
    if (gotLine)
    {
      log << " line " << std::dec << line.LineNumber;
      log << ", " << line.FileName;
    }
    log << std::endl;
  }
}

} // unnamed namespace

extern "C" void FatalError(void)
{
  PrintStackTrace();
  ::MessageBox(0,"A fatal error has occurred, see \"i7log.txt\" for details.",
    INFORM_TITLE,MB_ICONERROR|MB_OK);
  ::ExitProcess(1);
}
