#pragma once

#include <vector>

// Based on Graham Nelson's inbuild code, from the Semantic Versioning chapter

#define SEMVER_NUMBER_DEPTH 3

struct SemanticVersion
{
  int versionNumbers[SEMVER_NUMBER_DEPTH];
  std::vector<CStringW> prereleaseSegments;
  CStringW buildMetadata;

  bool IsNull(void) const;
  CStringW ToText(void) const;
  int Compare(const SemanticVersion& v) const;

  static SemanticVersion Null(void);
  static SemanticVersion FromText(const CStringW& t);

#ifdef _DEBUG
  static void UnitTest(void);
#endif
};
