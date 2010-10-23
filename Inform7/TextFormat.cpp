#include "stdafx.h"
#include "TextFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

int UTF8ToUnicode(LPCSTR src, int srcLen, LPWSTR dest, int destLen)
{
  int ntb = 0;
  int nwc = 0;
  LPCSTR utfPtr = src;
  char utfChar;

  while ((srcLen--) && ((dest == NULL) || (nwc < destLen)))
  {
    if (((*utfPtr)&0x80) == 0)
    {
      if (destLen)
        dest[nwc] = (WCHAR)*utfPtr;
      nwc++;
    }
    else if (((*utfPtr)&0x40) == 0)
    {
      if (ntb != 0)
      {
        ntb--;
        if (destLen)
        {
          dest[nwc] <<= 6;
          dest[nwc] |= ((*utfPtr)&0x003f);
        }
        if (ntb == 0)
          nwc++;
      }
    }
    else
    {
      if (ntb > 0)
      {
        ntb = 0;
        nwc++;
      }
      else
      {
        utfChar = *utfPtr;
        while (((utfChar)&0x80) != 0)
        {
          utfChar <<= 1;
          ntb++;
        }
        if (destLen)
          dest[nwc] = utfChar >> ntb;
        ntb--;
      }
    }
    utfPtr++;
  }
  return nwc;
}

} // unnamed namespace

CStringW TextFormat::UTF8ToUnicode(const CString& in)
{
  int len = ::UTF8ToUnicode(in,in.GetLength(),NULL,0);
  CStringW out;
  LPWSTR ptr = out.GetBufferSetLength(len);
  ::UTF8ToUnicode(in,in.GetLength(),ptr,len);
  out.ReleaseBuffer(len);
  return out;
}

CString TextFormat::UnicodeToUTF8(const CStringW& in)
{
  CString utfText;
  int utfLen = ::AtlUnicodeToUTF8(in,in.GetLength(),NULL,0);
  LPSTR utfPtr = utfText.GetBufferSetLength(utfLen);
  ::AtlUnicodeToUTF8(in,in.GetLength(),utfPtr,utfLen);
  utfText.ReleaseBuffer(utfLen);
  return utfText;
}

CString TextFormat::ToXML_UTF8(const CStringW& in)
{
  // First escape any characters that will cause problems in the XML
  CStringW escText;
  int escLen = ::EscapeXML(in,in.GetLength(),NULL,0);
  LPWSTR escPtr = escText.GetBufferSetLength(escLen);
  ::EscapeXML(in,in.GetLength(),escPtr,escLen);
  escText.ReleaseBuffer(escLen);

  // Convert the escaped text to UTF-8
  CString utfText;
  int utfLen = ::AtlUnicodeToUTF8(escText,escText.GetLength(),NULL,0);
  LPSTR utfPtr = utfText.GetBufferSetLength(utfLen);
  ::AtlUnicodeToUTF8(escText,escText.GetLength(),utfPtr,utfLen);
  utfText.ReleaseBuffer(utfLen);
  return utfText;
}

CString TextFormat::UnicodeToLatin1(const CStringW& in)
{
  CString text;
  int len = in.GetLength();
  LPSTR textPtr = text.GetBufferSetLength(len);
  for (int i = 0; i < len; i++)
  {
    wchar_t c = in[i];
    textPtr[i] = (c < 256) ? (char)c : '?';
  }
  text.ReleaseBuffer(len);
  return text;
}

CStringW TextFormat::Latin1ToUnicode(const CString& in)
{
  CStringW text;
  int len = in.GetLength();
  LPWSTR textPtr = text.GetBufferSetLength(len);
  for (int i = 0; i < len; i++)
    textPtr[i] = (wchar_t)(unsigned char)in[i];
  text.ReleaseBuffer(len);
  return text;
}
