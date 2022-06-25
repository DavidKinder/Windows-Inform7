#include "stdafx.h"
#include "TranscriptDiff.h"

#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/* Adapted from IFDiffer.m in the MacOS front-end, originally written in Objective C by Toby Nelson,
   based on an algorithm supplied by Graham Nelson.

   Purpose: To provide text matching in the style of the Unix tool diff.

   Our task is to take two strings, "ideal" and "actual", and return a fairly minimal, fairly legible
   sequence of edits which would turn ideal into actual. We won't use Myers's algorithm because it's
   overkill for the text sizes we have here, and because we want to pay more attention to word
   boundaries so as to produce human-readable results; the running time below is worst-case quadratic
   in the number of words scanned, but plenty fast enough for IF transcript use in practice.
*/

#define MINIMUM_SPLICE_WORTH_BOTHERING_WITH 5

const CStringW& TranscriptDiff::GetIdeal(void)
{
  return m_ideal;
}

bool TranscriptDiff::SetIdeal(LPCWSTR ideal)
{
  CStringW prev = m_ideal;
  m_ideal = ideal;
  m_ideal.Replace('\r','\n');
  return m_ideal != prev;
}

const CStringW& TranscriptDiff::GetActual(void)
{
  return m_actual;
}

bool TranscriptDiff::SetActual(LPCWSTR actual)
{
  CStringW prev = m_actual;
  m_actual = actual;
  m_actual.Replace('\r','\n');
  return m_actual != prev;
}

/* The diff algorithm.
   We do this in the simplest way possible. This outer routine, which is not
   recursively called, sets up the returned structure and sends it back.

   Note that a sequence of edits with no insertions or deletions means the
   match was in fact perfect, and is converted to an empty sequence.
*/
void TranscriptDiff::Diff(void)
{
  m_diffs.clear();

  DiffOuterRange(Range(0,m_ideal.GetLength()),Range(0,m_actual.GetLength()));

  for (auto& diff : m_diffs)
  {
    if ((diff.formOfEdit != PRESERVE_EDIT) && (diff.formOfEdit != PRESERVE_ACTUAL_EDIT))
      return;
  }
  m_diffs.clear();
}

bool TranscriptDiff::HasDiff(void)
{
  return (m_diffs.size() > 0);
}

static TranscriptDiff::Range MatchRegEx(std::wregex& regexp, LPCWSTR str, TranscriptDiff::Range inRange)
{
  std::wcregex_iterator it(str+inRange.location,str+inRange.location+inRange.length,regexp);
  if (it != std::wcregex_iterator())
    return TranscriptDiff::Range(inRange.location+it->position(),it->length());
  return TranscriptDiff::Range();
}

/* The first level down is also non-recursive and simply looks for a typical
   I7 banner line. Any correctly formed I7 banner matches any other; this
   ensures that transcripts of the same interaction, taken from builds on
   different days or with different compiler versions, continue to match.

   If text A (as we call the ideal version) and text B (the actual) both
   contain I7 banners, we split them into before, then the banner, then
   after. The result then consists of a diff of the before-texts, followed
   by preserving the actual banner, followed by a diff of the after-texts.
*/
void TranscriptDiff::DiffOuterRange(Range rangeA, Range rangeB)
{
  // Performance optimisation: Don't bother checking for the complex RegEx expression below
  // unless both strings are a good candidate, i.e. each string contains the words "Serial number".
  if ((m_ideal.Find(L"Serial number") > 0) && (m_actual.Find(L"Serial number") > 0))
  {
    std::wregex regexp;
    regexp.assign(L"(.*?)(Release \\d+ / Serial number \\d+ / Inform 7 build .... .I6.+?lib .+?SD).*");//XXXXDK 10.1?
    Range matchRangeA2 = MatchRegEx(regexp,m_ideal,rangeA);
    if (matchRangeA2.location != -1)
    {
      ASSERT (matchRangeA2.location >= rangeA.location);
      Range matchRangeA1(rangeA.location,matchRangeA2.location - rangeA.location);

      int A_pre_len  = (int) matchRangeA1.length;
      int A_ver_len  = (int) matchRangeA2.length;
      int A_post_len = (int) rangeA.length - A_pre_len - A_ver_len;

      Range matchRangeB2 = MatchRegEx(regexp,m_actual,rangeB);
      if (matchRangeB2.location != -1)
      {
        ASSERT (matchRangeB2.location >= rangeB.location);
        Range matchRangeB1(rangeB.location,matchRangeB2.location - rangeB.location);

        int B_pre_len  = (int) matchRangeB1.length;
        int B_ver_len  = (int) matchRangeB2.length;
        int B_post_len = (int) rangeB.length - B_pre_len - B_ver_len;

        DiffInnerRange(matchRangeA1,matchRangeB1);
        m_diffs.emplace_back(matchRangeB2,PRESERVE_ACTUAL_EDIT);
        DiffInnerRange(
          Range(rangeA.location + A_pre_len + A_ver_len,A_post_len),
          Range(rangeB.location + B_pre_len + B_ver_len,B_post_len));
        return;
      }
    }
  }
  DiffInnerRange(rangeA,rangeB);
}

// The second level is at last recursive.
void TranscriptDiff::DiffInnerRange(Range rangeA, Range rangeB)
{
  // If A is empty B must be inserted, if B is empty A must be deleted.
  if ((rangeA.length == 0) && (rangeB.length == 0))
    return;
  if (rangeA.length == 0)
  {
    m_diffs.emplace_back(rangeB,INSERT_EDIT);
    return;
  }
  if (rangeB.length == 0)
  {
    m_diffs.emplace_back(rangeA,DELETE_EDIT);
    return;
  }

  // We look for the longest common prefix consisting of a sequence of entire
  // words, or at any rate, ending at a word boundary (in both texts).
  int i;
  for (i = 0; (i < rangeA.length) && (i < rangeB.length); i++ )
  {
    if (m_ideal[(int)rangeA.location+i] != m_actual[(int)rangeB.location+i])
      break;
  }
  while ((i > 0) && (!IsWordBoundary(m_ideal,rangeA.location+i-1)))
    i--;
  if (i > 0)
  {
    m_diffs.emplace_back(Range(rangeA.location,i),PRESERVE_EDIT);
    DiffInnerRange(
      Range(rangeA.location + i,rangeA.length - i),
      Range(rangeB.location + i,rangeB.length - i));
    return;
  }

  // Similarly, we're only interested in a common suffix going back to the start
  // of a whole word.
  size_t rangeEndA = rangeA.location + rangeA.length;
  size_t rangeEndB = rangeB.location + rangeB.length;
  for (i = 0; (i < rangeA.length) && (i < rangeB.length); i++)
  {
    if (m_ideal[(int)rangeEndA-1-i] != m_actual[(int)rangeEndB-1-i])
      break;
  }
  while ((i > 0) && (!IsWordBoundary(m_ideal,rangeEndA-i-1)))
    i--;
  if (i > 0)
  {
    DiffInnerRange(
      Range(rangeA.location,rangeA.length - i),
      Range(rangeB.location,rangeB.length - i));
    m_diffs.emplace_back(Range(rangeEndA-i,i),PRESERVE_EDIT);
    return;
  }

  /* In the typical use case most of the strings will now be gone, and this is where the algorithm
     goes quadratic. We're going to look for the longest common substring between A and B, provided
     it occurs at word boundaries, and is not trivially short. If we find this, we'll recurse to
     diff the text before the substring, then preserve the substring, then recurse to diff the text
     afterwards.
  */
  int max_i = -1;
  int max_j = -1;
  int max_len = 0;

  for (i = 0; i < rangeA.length; i++)
  {
    if ((i == 0) || (IsWordBoundary(m_ideal,rangeA.location + i-1)))
    {
      for (int j = 0; j < rangeB.length; j++)
      {
        if ((j == 0) || (IsWordBoundary(m_actual,rangeB.location + j-1)))
        {
          int k;
          for (k = 0; (i+k < (int)rangeA.length) && (j+k < (int)rangeB.length) &&
            (m_ideal[(int)rangeA.location + i+k] == m_actual[(int)rangeB.location + j+k]); k++)
          {
          }
          while ((k > MINIMUM_SPLICE_WORTH_BOTHERING_WITH) &&
            (!(IsWordBoundary(m_ideal,rangeA.location + i+k-1))))
          {
            k--;
          }
          if (k > max_len)
          {
            max_len = k;
            max_i = i;
            max_j = j;
          }
        }
      }
    }
  }
  if (max_len >= MINIMUM_SPLICE_WORTH_BOTHERING_WITH)
  {
    for (int c = 0; c < max_len; c++)
      ASSERT (m_ideal[(int)rangeA.location + max_i+c] == m_actual[(int)rangeB.location + max_j+c]);

    DiffInnerRange(Range(rangeA.location,max_i),Range(rangeB.location,max_j));
    m_diffs.emplace_back(Range(rangeA.location + max_i,max_len),PRESERVE_EDIT);
    DiffInnerRange(
      Range(rangeA.location + max_i + max_len,rangeA.length - max_i - max_len),
      Range(rangeB.location + max_j + max_len,rangeB.length - max_j - max_len));
    return;
  }

  // If we can't find any good substring, all we can usefully do is say that
  // the text has entirely changed, and we display this as cleanly as possible:
  m_diffs.emplace_back(rangeA,DELETE_EDIT);
  m_diffs.emplace_back(rangeB,INSERT_EDIT);
}

bool TranscriptDiff::IsWordBoundary(LPCWSTR str, size_t index)
{
  if (::IsCharAlphaW(str[index]) && ::IsCharAlphaW(str[index+1]))
    return false;
  return true;
}

TranscriptDiff::Range::Range()
{
  location = -1;
  length = 0;
}

TranscriptDiff::Range::Range(size_t loc, size_t len)
{
  location = loc;
  length = len;
}

TranscriptDiff::DiffEdit::DiffEdit()
{
  formOfEdit = DELETE_EDIT;
};

TranscriptDiff::DiffEdit::DiffEdit(Range frag, EFormOfEdit edit)
{
  fragment = frag;
  formOfEdit = edit;
}
