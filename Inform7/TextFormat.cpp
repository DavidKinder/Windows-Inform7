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

CString TextFormat::AnsiToUTF8(const CString& in)
{
  return UnicodeToUTF8(CStringW(in));
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

CString TextFormat::FormatNumber(int value)
{
  CStringW unformatted;
  unformatted.Format(L"%d",value);

  NUMBERFMTW fmt = { 0 };
  fmt.lpDecimalSep = L"."; // Not used

  WCHAR thousands[32] = L"";
  if (::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT,LOCALE_STHOUSAND,thousands,32) == 0)
    return CString(unformatted);
  fmt.lpThousandSep = thousands;

  WCHAR grouping[32] = L"";
  if (::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT,LOCALE_SGROUPING,grouping,32) == 0)
    return CString(unformatted);
  CString groupInfo(grouping);
  if (groupInfo == "3;0")
    fmt.Grouping = 3;
  else if (groupInfo == "3;2;0")
    fmt.Grouping = 32;

  int len = ::GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT,0,unformatted,&fmt,NULL,0);
  if (len == 0)
    return CString(unformatted);

  CStringW formatted;
  LPWSTR buffer = formatted.GetBufferSetLength(len+1);
  ::GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT,0,unformatted,&fmt,buffer,len);
  formatted.ReleaseBuffer();
  return CString(formatted);
}

bool TextFormat::StartsWith(const CString& test, LPCSTR start)
{
  return (test.Left((int)strlen(start)) == start);
}

bool TextFormat::StartsWith(const CStringW& test, LPCWSTR start)
{
  return (test.Left((int)wcslen(start)) == start);
}

bool TextFormat::EndsWith(const CString& test, LPCSTR end)
{
  return (test.Right((int)strlen(end)) == end);
}

bool TextFormat::EndsWith(const CStringW& test, LPCWSTR end)
{
  return (test.Right((int)wcslen(end)) == end);
}

CString TextFormat::Unescape(const char* input)
{
  int len = (int)strlen(input);
  CString output;
  output.Preallocate(len);

  static const char* hex = "0123456789ABCDEF";

  int i = 0;
  while (i < len)
  {
    if ((input[i] == L'%') && (i < len-2))
    {
      const char* hex1 = strchr(hex,toupper(input[i+1]));
      const char* hex2 = strchr(hex,toupper(input[i+2]));
      if ((hex1 != NULL) && (hex2 != NULL))
      {
        char ch = (char)(((hex1-hex)<<4)+(hex2-hex));
        output.AppendChar(ch);
        i += 3;
        continue;
      }
    }
    output.AppendChar(input[i++]);
  }
  return output;
}
