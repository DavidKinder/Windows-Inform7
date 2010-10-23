// Helper function to return the difference between two arrays,
// based on Andrew Hunter's Objective-C implementation from the
// OS X Inform application.

#include "stdafx.h"
#include "Diff.h"

#include <malloc.h>
#include <set>

struct str_elem {
  const wchar_t* str;
  int len;
};

static bool operator<(const str_elem& a, const str_elem& b) {
  if (a.len != b.len) return (a.len < b.len);
  return (wcsncmp(a.str,b.str,a.len) < 0);
}

static DWORD_PTR hash(const str_elem& el) {
  if (el.len == 0)
    return 0;
  return (DWORD_PTR)el.str;
}

static int white(const wchar_t* str, bool iswhite) {
  int n = 0;
  while (true) {
    switch (*(str+n)) {
    case 0:
      return n;
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      if (!iswhite)
        return n;
      break;
    default:
      if (iswhite)
        return n;
      break;
    }
    n++;
  }
  return n;
}

static void makeWords(const wchar_t* str, std::vector<str_elem>& words, std::set<str_elem>& elements) {
  while (*str != 0) {
    int len = white(str,false);
    if (len > 0) {
      str_elem el;
      el.str = str;
      el.len = len;
      std::set<str_elem>::iterator it = elements.find(el);
      if (it != elements.end())
        words.push_back(*it);
      else
      {
        elements.insert(el);
        words.push_back(el);
      }
    }
    str += len;
    str += white(str,true);
  }
}

// Performing the comparison

struct diff_hash {
  int serial;
  DWORD_PTR hash;
};

struct diff_equivalence {
  int serial;
  bool last;
};

struct diff_candidate {
  int srcItem;
  int destItem;
  struct diff_candidate* previous;
};

static int hashCompare(const void* a, const void* b) {
  const struct diff_hash* aHash = (struct diff_hash*)a;
  const struct diff_hash* bHash = (struct diff_hash*)b;

  if (aHash->hash > bHash->hash) {
    return 1;
  } else if (aHash->hash < bHash->hash) {
    return -1;
  } else {
    if (aHash->serial > bHash->serial) {
      return 1;
    } else if (aHash->serial < bHash->serial) {
      return -1;
    } else {
      return 0;
    }
  }
}

static int hashCompare2(const void* a, const void* b) {
  const struct diff_hash* aHash = (struct diff_hash*)a;
  const struct diff_hash* bHash = (struct diff_hash*)b;

  if (aHash->hash > bHash->hash) {
    return 1;
  } else if (aHash->hash < bHash->hash) {
    return -1;
  } else {
    return 0;
  }
}

void Diff::DiffStrings(
  const wchar_t* destStr, const wchar_t* sourceStr,
  DiffResults& destDiffs, DiffResults& sourceDiffs) {
  int i,j;

  std::set<str_elem> elements;
  std::vector<str_elem> destArray, sourceArray;
  makeWords(destStr, destArray, elements);
  makeWords(sourceStr, sourceArray, elements);

  // Array 'V' described in the diff algorithm
  struct diff_hash* hashArray = (struct diff_hash*)
    alloca(sizeof (struct diff_hash) * (destArray.size()+1));

  // Hash all the destination elements
  for (j=1; j<=(int)destArray.size(); j++) {
    hashArray[j].serial = j;
    hashArray[j].hash = hash(destArray[j-1]);
  }

  // Sort the array
  qsort(hashArray+1, destArray.size(), sizeof(struct diff_hash), hashCompare);

  // Array 'E' described in the diff algorithm
  struct diff_equivalence* equiv = (struct diff_equivalence*)
    alloca(sizeof (struct diff_equivalence) * (destArray.size()+1));

  equiv[0].serial = 0;
  equiv[0].last   = true;

  // Work out the equivalence classes
  for (j=1; j<(int)destArray.size(); j++) {
    equiv[j].serial = hashArray[j].serial;
    equiv[j].last   = hashArray[j].hash != hashArray[j+1].hash;
  }

  equiv[j].serial = hashArray[j].serial;
  equiv[j].last   = true;

  // Array 'P' described in the diff algorithm. This points to the beginning
  // of the class of lines in the destination equivalent to lines in the source
  int* srcEquiv = (int*)alloca(sizeof (int) * (sourceArray.size()+1));

  for (i=1; i<=(int)sourceArray.size(); i++) {
    struct diff_hash searchHash;
  
    searchHash.serial = i;
    searchHash.hash = hash(sourceArray[i-1]);
  
    // Search for an item with the same hash
    struct diff_hash* diffItem = (struct diff_hash*)bsearch(&searchHash,
      hashArray+1, destArray.size(), sizeof(struct diff_hash), hashCompare2);
  
    if (diffItem == NULL) {
      srcEquiv[i] = 0;
    } else {
      j = (int)(diffItem - hashArray);
      while (!equiv[j-1].last) j--;
      srcEquiv[i] = j;
    }
  }

  // Array 'K' described in the diff algorithm
  int candidateSize = (int)
    (sourceArray.size()<destArray.size()?sourceArray.size():destArray.size());
  struct diff_candidate* candidates = (struct diff_candidate*)
    alloca(sizeof (struct diff_candidate) * (candidateSize+2));

  candidates[0].srcItem = 0;
  candidates[0].destItem = 0;
  candidates[0].previous = NULL;

  candidates[1].srcItem = (int)sourceArray.size() + 2;
  candidates[1].destItem = (int)destArray.size() + 2;
  candidates[1].previous = NULL;

  int lastCandidate = 0;

  // Find the longest common subsequences
  for (i=1; i<=(int)sourceArray.size(); i++) {
    if (srcEquiv[i] != 0) {
      int p = srcEquiv[i];
    
      // 'Merge step': algorithm A.3 from the diff paper
      int candidateNum = 0;
      struct diff_candidate candidate = candidates[0];
    
      while (1) {
        int serial = equiv[p].serial;
      
        int s;
        for (s=candidateNum; s<=lastCandidate; s++) {
          if (candidates[s].destItem < serial && candidates[s+1].destItem > serial) break;
        }
      
        if (s <= lastCandidate) {
          // (Step 4)
          if (candidates[s+1].destItem > serial) {
            candidates[candidateNum] = candidate;
            candidateNum = s+1;

            candidate.srcItem = i;
            candidate.destItem = serial;
            candidate.previous = candidates + s;
          }
        
          // (Step 5)
          if (s == lastCandidate) {
            candidates[lastCandidate+2] = candidates[lastCandidate+1];
            lastCandidate++;
            break;
          }
        }
      
        if (equiv[p].last) break;
        p++;
      }
    
      candidates[candidateNum] = candidate;
    }
  }

  // Array of Longest Common Subsequences
  int* subsequence = (int*)alloca(sizeof (int) * (sourceArray.size()+1));

  for (i=0; i<=(int)sourceArray.size(); i++) subsequence[i] = 0;

  struct diff_candidate* candidate = candidates + lastCandidate;
  while (candidate) {
    subsequence[candidate->srcItem] = candidate->destItem;
    candidate = candidate->previous;
  }

  int* subsequence2 = (int*)alloca(sizeof (int) * (destArray.size()+1));
  for (i=1; i<=(int)destArray.size(); i++) subsequence2[i] = 0;

  // Finally: produce a result
  for (i=1; i<=(int)sourceArray.size(); i++) {
    if (subsequence[i] == 0) {
      DiffResult result;
      result.start = (int)(sourceArray[i-1].str - sourceStr);
      result.length = sourceArray[i-1].len;
      sourceDiffs.push_back(result);
    } else {
      subsequence2[subsequence[i]] = i;
    }
  }
  for (i=1; i<=(int)destArray.size(); i++) {
    if (subsequence2[i] == 0) {
      DiffResult result;
      result.start = (int)(destArray[i-1].str - destStr);
      result.length = destArray[i-1].len;
      destDiffs.push_back(result);
    }
  }
}
