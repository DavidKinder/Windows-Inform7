#pragma once

#include <vector>

class TranscriptDiff
{
public:
  const CStringW& GetIdeal(void);
  bool SetIdeal(LPCWSTR ideal);
  const CStringW& GetActual(void);
  bool SetActual(LPCWSTR actual);

  void Diff(void);
  bool HasDiff(void);

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

    Range fragment;
    EFormOfEdit formOfEdit;
  };

private:
  void DiffOuterRange(Range rangeA, Range rangeB);
  void DiffInnerRange(Range rangeA, Range rangeB);
  bool IsWordBoundary(LPCWSTR str, size_t index);

  typedef std::vector<DiffEdit> DiffEdits;

  CStringW m_ideal;
  CStringW m_actual;
  DiffEdits m_diffs;
};
