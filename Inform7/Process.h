#pragma once

struct Process
{
  HANDLE process;
  DWORD processId;

  Process();
  void set(PROCESS_INFORMATION pi);
  void close();
};
