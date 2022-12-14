#pragma once

#include <vector>

class TranscriptDiff
{
public:
  void Diff(LPCWSTR ideal, LPCWSTR actual);
  bool HasDiff(void) const;

  struct Range
  {
    Range();
    Range(size_t loc, size_t len);

    size_t location;
    size_t length;
  };

  enum EFormOfEdit
  {
    DELETE_EDIT,
    PRESERVE_EDIT,
    PRESERVE_ACTUAL_EDIT,
    INSERT_EDIT 
  };

  struct DiffEdit
  {
    DiffEdit();
    DiffEdit(Range frag, EFormOfEdit edit);

    CStringW SubString(const CStringW& str) const;

    Range fragment;
    EFormOfEdit formOfEdit;
  };

  typedef std::vector<DiffEdit> DiffEdits;
  const DiffEdits& GetDifferences(void) const;
  CStringW SubString(const DiffEdit& diff) const;
  LPCWSTR GetIdeal(void) const;

private:
  void DiffOuterRange(Range rangeA, Range rangeB);
  void DiffInnerRange(Range rangeA, Range rangeB);
  bool IsWordBoundary(LPCWSTR str, size_t index);

  CStringW m_ideal;
  CStringW m_actual;
  DiffEdits m_diffs;
};
