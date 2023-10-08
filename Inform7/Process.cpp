#include "stdafx.h"
#include "Process.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Process::Process()
{
  process = INVALID_HANDLE_VALUE;
  processId = -1;
}

void Process::set(PROCESS_INFORMATION pi)
{
  process = pi.hProcess;
  processId = pi.dwProcessId;
}

void Process::close()
{
  if (process != INVALID_HANDLE_VALUE)
  {
    ::CloseHandle(process);
    process = INVALID_HANDLE_VALUE;
    processId = -1;
  }
}
