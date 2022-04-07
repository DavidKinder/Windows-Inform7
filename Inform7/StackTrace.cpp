#include "stdafx.h"
#include "Inform.h"

#include <fstream>
#include <sstream>

#include <dbghelp.h>

namespace {

void PrintStackTrace(HANDLE process, HANDLE thread,
  DWORD machine, DWORD64 pc, DWORD64 framePtr, DWORD64 stackPtr, PVOID context, std::ostream& log)
{
  // Set symbol options
  SymSetOptions(SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);

  // Load any symbols
  CString symPath = theApp.GetAppDir()+"\\Symbols";
  if (SymInitialize(process,symPath,TRUE) == 0)
    return;

  // Set up the initial stack frame
  STACKFRAME64 frame = { 0 };
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Mode = AddrModeFlat;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrPC.Offset = pc;
  frame.AddrFrame.Offset = framePtr;
  frame.AddrStack.Offset = stackPtr;

  while (true)
  {
    // Get the next stack frame
    if (StackWalk64(machine,process,thread,&frame,context,NULL,
                    SymFunctionTableAccess64,SymGetModuleBase64,NULL) == 0)
      break;

    // Check the stack frame
    if (frame.AddrFrame.Offset == 0)
      break;

    // Get information on the module containing the address
    IMAGEHLP_MODULE64 info;
    info.SizeOfStruct = sizeof (IMAGEHLP_MODULE64);
    BOOL gotModuleInfo = SymGetModuleInfo64(process,frame.AddrPC.Offset,&info);

    // Set up a symbol buffer
    BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL64)+512];
    PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)symbolBuffer;
    symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    symbol->MaxNameLength = 512;

    // Get the symbol
    DWORD64 displacement1 = 0;
    BOOL gotSymbol = SymGetSymFromAddr64(process,frame.AddrPC.Offset,&displacement1,symbol);

    // Get the file and line number
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof line;
    DWORD displacement2 = 0;
    BOOL gotLine = SymGetLineFromAddr64(process,frame.AddrPC.Offset,&displacement2,&line);

    // Print out the stack trace
    if (gotModuleInfo && gotSymbol)
    {
      log << info.ModuleName << '!' << symbol->Name << "()";
      if (gotLine)
        log << " at " << line.FileName << " line " << std::dec << line.LineNumber;
      log << std::endl;
    }
  }

  SymCleanup(process);
}

void LogStackTrace(void)
{
  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();

  // Create a stream to write to the log file
  char logPath[MAX_PATH];
  SHGetFolderPath(0,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,logPath);
  strcat(logPath,LOG_FILE);
  std::ofstream log(logPath);

  // Get the thread's context and then print its stack trace
  CONTEXT context = { 0 };
  context.ContextFlags = CONTEXT_FULL;
#ifdef _WIN64
  RtlCaptureContext(&context);
  PrintStackTrace(process,thread,IMAGE_FILE_MACHINE_AMD64,context.Rip,context.Rbp,context.Rsp,&context,log);
#else
  {
    __asm    call x
    __asm x: pop eax
    __asm    mov context.Eip, eax
    __asm    mov context.Ebp, ebp
    __asm    mov context.Esp, esp
  }
  PrintStackTrace(process,thread,IMAGE_FILE_MACHINE_I386,context.Eip,context.Ebp,context.Esp,&context,log);
#endif
  log.close();
}

} // unnamed namespace

CString GetStackTrace(HANDLE process, HANDLE thread, DWORD exCode)
{
  std::ostringstream log;
  log << "Exception code 0x" << std::hex << exCode << std::endl;

#ifdef _WIN64
  // Is the process a 64-bit one, or a 32-bit one running under WOW (Windows On Windows)?
  BOOL isWow64 = FALSE;
  if (IsWow64Process(process,&isWow64))
  {
    if (isWow64)
    {
      WOW64_CONTEXT context = { 0 };
      context.ContextFlags = CONTEXT_FULL;
      if (Wow64GetThreadContext(thread,&context))
      {
        PrintStackTrace(process,thread,
          IMAGE_FILE_MACHINE_I386,context.Eip,context.Ebp,context.Esp,&context,log);
      }
    }
    else
    {
      CONTEXT context = { 0 };
      context.ContextFlags = CONTEXT_FULL;
      if (GetThreadContext(thread,&context))
      {
        PrintStackTrace(process,thread,
          IMAGE_FILE_MACHINE_AMD64,context.Rip,context.Rbp,context.Rsp,&context,log);
      }
    }
  }
#else
  CONTEXT context = { 0 };
  context.ContextFlags = CONTEXT_FULL;
  if (GetThreadContext(thread,&context))
  {
    PrintStackTrace(process,thread,
      IMAGE_FILE_MACHINE_I386,context.Eip,context.Ebp,context.Esp,&context,log);
  }
#endif

  return CString(log.str().c_str());
}

extern "C" void FatalError(void)
{
  LogStackTrace();
  MessageBox(0,"A fatal error has occurred, see \"i7log.txt\" for details.",
    INFORM_TITLE,MB_ICONERROR|MB_OK);
  ExitProcess(1);
}
