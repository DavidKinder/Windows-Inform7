#include "stdafx.h"
#include "SemanticVersion.h"

// Based on Graham Nelson's inbuild code, from the Semantic Versioning chapter

static int VersionFloor(int n)
{
  if (n < 0)
    return 0;
  return n;
}

static int NumberFromText(const CStringW& t)
{
  for (int pos = 0; pos < t.GetLength(); pos++)
  {
    if (!iswdigit(t.GetAt(pos)))
      return -1;
  }
  if (t.GetLength() > 1)
  {
    if (t.GetAt(0) == '0')
      return -1;
  }
  return _wtoi(t);
}

bool SemanticVersion::IsNull(void) const
{
  bool allow = true;
  for (int i = 0; i < SEMVER_NUMBER_DEPTH; i++)
  {
    if (versionNumbers[i] < -1)
      return true;
    if (versionNumbers[i] == -1)
      allow = false;
    else if (!allow)
      return true;
  }
  if (versionNumbers[0] < 0)
    return true;
  return false;
}

CStringW SemanticVersion::ToText(void) const
{
  if (IsNull())
    return L"null";

  CStringW t;
  for (int i = 0; (i < SEMVER_NUMBER_DEPTH) && (versionNumbers[i] >= 0); i++)
  {
    if (i > 0)
      t.AppendChar(L'.');
    t.AppendFormat(L"%d",versionNumbers[i]);
  }
  if (!prereleaseSegments.empty())
  {
    int c = 0;
    for (const auto& segment : prereleaseSegments)
    {
      if (c++ == 0)
        t.AppendChar(L'-');
      else
        t.AppendChar(L'.');
      t.AppendFormat(L"%s",segment);
    }
  }
  if (!buildMetadata.IsEmpty())
    t.AppendFormat(L"+%s",buildMetadata);
  return t;
}

int SemanticVersion::Compare(const SemanticVersion& v) const
{
  for (int i = 0; i < SEMVER_NUMBER_DEPTH; i++)
  {
    int n1 = VersionFloor(versionNumbers[i]);
    int n2 = VersionFloor(v.versionNumbers[i]);
    if (n1 > n2)
      return +1;
    if (n1 < n2)
      return -1;
  }

  auto it1 = prereleaseSegments.begin();
  auto it1e = prereleaseSegments.end();
  auto it2 = v.prereleaseSegments.begin();
  auto it2e = v.prereleaseSegments.end();
  if ((it1 == it1e) && (it2 != it2e))
    return +1;
  if ((it1 != it1e) && (it2 == it2e))
    return -1;
  while ((it1 != it1e) && (it2 != it2e))
  {
    int n1 = NumberFromText(*it1);
    int n2 = NumberFromText(*it2);
    if ((n1 >= 0) && (n2 >= 0))
    {
      if (n1 < n2)
        return -1;
      if (n1 > n2)
        return +1;
    }
    else
    {
      int c = it1->Compare(*it2);
      if (c < 0)
        return -1;
      if (c > 0)
        return +1;
    }
    it1++;
    it2++;
  }
  if ((it1 == it1e) && (it2 != it2e))
    return -1;
  if ((it1 != it1e) && (it2 == it2e))
    return +1;
  return 0;
}

SemanticVersion SemanticVersion::Null(void)
{
  SemanticVersion v;
  for (int i = 0; i < SEMVER_NUMBER_DEPTH; i++)
    v.versionNumbers[i] = -1;
  return v;
}

enum Part
{
  MMP_SEMVERPART,
  PRE_SEMVERPART,
  BM_SEMVERPART
};

SemanticVersion SemanticVersion::FromText(const CStringW& t)
{
  SemanticVersion v = Null();
  int component = 0, val = -1, dots_used = 0, slashes_used = 0, count = 0;
  Part part = MMP_SEMVERPART;
  CStringW prerelease;

  for (int pos = 0; pos < t.GetLength(); pos++)
  {
    WCHAR c = t.GetAt(pos);
    switch (part)
    {
    case MMP_SEMVERPART:
      if (c == '.')
        dots_used++;
      if (c == '/')
        slashes_used++;
      if ((c == '.') || (c == '/') || (c == '-') || (c == '+'))
      {
        if (val == -1)
          return Null();
        if (component >= SEMVER_NUMBER_DEPTH)
          return Null();
        v.versionNumbers[component] = val;
        component++;
        val = -1;
        count = 0;
        if (c == '-')
          part = PRE_SEMVERPART;
        if (c == '+')
          part = BM_SEMVERPART;
      }
      else if (iswdigit(c))
      {
        int digit = c - '0';
        if ((val == 0) && (slashes_used == 0))
          return Null();
        if (val < 0)
          val = digit;
        else
          val = (10*val) + digit;
        count++;
      }
      else
        return Null();
      break;
    case PRE_SEMVERPART:
      if (c == '.')
      {
        if (prerelease.IsEmpty())
          return Null();
        v.prereleaseSegments.push_back(prerelease);
        prerelease.Empty();
      }
      else if (c == '+')
      {
        if (prerelease.IsEmpty())
          return Null();
        v.prereleaseSegments.push_back(prerelease);
        prerelease.Empty();
        part = BM_SEMVERPART;
      }
      else
        prerelease.AppendChar(c);
      break;
    case BM_SEMVERPART:
      v.buildMetadata.AppendChar(c);
      break;
    }
  }
  if ((part == PRE_SEMVERPART) && !prerelease.IsEmpty())
    v.prereleaseSegments.push_back(prerelease);
  if ((dots_used > 0) && (slashes_used > 0))
    return Null();
  if (slashes_used > 0)
  {
    if (component > 1)
      return Null();
    if (count != 6)
      return Null();
    v.versionNumbers[1] = 0;
    component = 2;
  }
  if (part == MMP_SEMVERPART)
  {
    if (val == -1)
      return Null();
    if (component >= SEMVER_NUMBER_DEPTH)
      return Null();
    v.versionNumbers[component] = val;
  }
  return v;
}

#ifdef _DEBUG
void SemanticVersion::UnitTest(void)
{
  CStringW printInputs[] =
  {
    L"1+lobster",
    L"1",
    L"1.2",
    L"1.2.3",
    L"71.0.45672",
    L"1.2.3.4",
    L"9/861022",
    L"9/86102",
    L"9/8610223",
    L"9/861022.2",
    L"9/861022/2",
    L"1.2.3-alpha.0.x45.1789",
    L"1.2+lobster",
    L"1.2.3+lobster",
    L"1.2.3-beta.2+shellfish"
  };
  CStringW printOutputs[] =
  {
    L"1+lobster",
    L"1",
    L"1.2",
    L"1.2.3",
    L"71.0.45672",
    L"null",
    L"9.0.861022",
    L"null",
    L"null",
    L"null",
    L"null",
    L"1.2.3-alpha.0.x45.1789",
    L"1.2+lobster",
    L"1.2.3+lobster",
    L"1.2.3-beta.2+shellfish",
  };
  for (int i = 0; i < sizeof printInputs / sizeof printInputs[0]; i++)
  {
    SemanticVersion v = SemanticVersion::FromText(printInputs[i]);
    ASSERT(v.ToText() == printOutputs[i]);
  }

  CStringW compareInputs[] =
  {
    L"3", L"5",
    L"3", L"3",
    L"3", L"3.0",
    L"3", L"3.0.0",
    L"3.1.41", L"3.1.5",
    L"3.1.41", L"3.2.5",
    L"3.1.41", L"3.1.41+arm64",
    L"3.1.41", L"3.1.41-pre.0.1",
    L"3.1.41-alpha.72", L"3.1.41-alpha.8",
    L"3.1.41-alpha.72a", L"3.1.41-alpha.8a",
    L"3.1.41-alpha.72", L"3.1.41-beta.72",
    L"3.1.41-alpha.72", L"3.1.41-alpha.72.zeta",
    L"1.2.3+lobster.54", L"1.2.3+lobster.100"
  };
  int compareOutputs[]
  {
    -1,
     0,
     0,
     0,
     1,
    -1,
     0,
     1,
     1,
    -1,
    -1,
    -1,
     0
  };
  for (int i = 0; i < sizeof compareOutputs / sizeof compareOutputs[0]; i++)
  {
    SemanticVersion v1 = SemanticVersion::FromText(compareInputs[(2*i)]);
    SemanticVersion v2 = SemanticVersion::FromText(compareInputs[(2*i)+1]);
    ASSERT(v1.Compare(v2) == compareOutputs[i]);
  }
}
#endif
