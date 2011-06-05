#include "stdafx.h"

#include <dbghelp.h>

namespace {

typedef PVOID(__stdcall *IMAGEDIRECTORYENTRYTODATA)(PVOID, BOOLEAN, USHORT, PULONG);

HMODULE debugDll = 0;
IMAGEDIRECTORYENTRYTODATA imageDirectoryEntryToData = NULL;

bool GotFunctions(void)
{
  if (imageDirectoryEntryToData == NULL)
    return false;
  return true;
}

} // unnamed namespace

void HookApiFunction(
  HMODULE callingDll, HMODULE calledDll, const char* calledDllName, const char* functionName, PROC newFunction)
{
  if ((callingDll == 0) || (calledDll == 0))
    return;

  // Load the debug help DLL, if available, and find entry points
  if (debugDll == 0)
  {
    debugDll = ::LoadLibrary("dbghelp.dll");
    imageDirectoryEntryToData = (IMAGEDIRECTORYENTRYTODATA)::GetProcAddress(debugDll,"ImageDirectoryEntryToData");
  }
  if (!GotFunctions())
    return;

  // Get the pointer to the 'real' function
  PROC realFunction = ::GetProcAddress(calledDll,functionName);
  if (realFunction == 0)
    return;

  // Get the import section of the DLL
  ULONG sz;
  PIMAGE_IMPORT_DESCRIPTOR import = (PIMAGE_IMPORT_DESCRIPTOR)
    (*imageDirectoryEntryToData)(callingDll,TRUE,IMAGE_DIRECTORY_ENTRY_IMPORT,&sz);
  if (import == NULL)
    return;

  // Find the import section matching the named DLL
  while (import->Name)
  {
    PSTR dllName = (PSTR)((PBYTE)callingDll + import->Name);
    {
      if (stricmp(dllName,calledDllName) == 0)
        break;
    }
    import++;
  }
  if (import->Name == NULL)
    return;

  // Scan the IAT for this DLL
  PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((PBYTE)callingDll + import->FirstThunk);
  while (thunk->u1.Function)
  {
    PROC* function = (PROC*)&(thunk->u1.Function);
    if (*function == realFunction)
    {
      // Make the function pointer writable and hook the function
      MEMORY_BASIC_INFORMATION mbi;
      ::VirtualQuery(function,&mbi,sizeof mbi);
      if (::VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_READWRITE,&mbi.Protect))
      {
        *function = newFunction;
        DWORD protect;
        ::VirtualProtect(mbi.BaseAddress,mbi.RegionSize,mbi.Protect,&protect);
        return;
      }
    }
    thunk++;
  }
}
