#pragma once

#include <vector>

namespace Diff
{
  struct DiffResult
  {
    int start;
    int length;
  };
  typedef std::vector<DiffResult> DiffResults;

  void DiffStrings(const wchar_t*, const wchar_t*, DiffResults&, DiffResults&);
}
