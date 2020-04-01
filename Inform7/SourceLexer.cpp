#include "stdafx.h"
#include "SourceLexer.h"
#include "SourceEdit.h"
#include "TextFormat.h"

#include "Platform.h"
#include "Scintilla.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
const char* headings[] = { "volume", "book", "part", "chapter", "section", "example" };
const char* docHeading = "---- documentation ----";
const char* beforeQuote = " \t\r\n.,:;?!(){}[]";
}

const int SourceLexer::StyleMask = 0x1F;

SourceLexer::SourceLexer(SourceEdit* edit, LexAction action)
{
  m_edit = edit;
  m_action = action;
}

void SourceLexer::Process(int startPos, int endPos, bool includeExt)
{
  // Find where to start applying styles
  int line = (int)CallEdit(SCI_LINEFROMPOSITION,startPos);
  startPos = (int)CallEdit(SCI_POSITIONFROMLINE,line);

  // Find where to stop applying styles
  int len = (int)CallEdit(SCI_GETLENGTH);
  if (endPos < 0)
    endPos = len;

  // Get the initial style to start with
  int style = STYLE_TEXT;
  if (startPos > 0)
    style = (int)CallEdit(SCI_GETSTYLEAT,startPos-1) & StyleMask;

  // Set the character to '\n' to indicate that we always start at the start of a line
  unsigned char c = '\n';

  // Apply styles as far as is required
  int pos = startPos;
  while (pos < endPos)
  {
    // Get the next character, and note if it is the start of a new line
    bool newLine = ((c == '\r') || (c == '\n'));
    c = (unsigned char)CallEdit(SCI_GETCHARAT,pos);

    if (style == STYLE_QUOTE)
    {
      if (IsQuote(c,pos,true))
        ApplyStyle(startPos,pos+1,style,STYLE_TEXT,StyleMask);
      else if (c == '[')
        ApplyStyle(startPos,pos,style,STYLE_SUBSTITUTION,StyleMask);
    }
    else if (style == STYLE_SUBSTITUTION)
    {
      if (IsQuote(c,pos,true))
        ApplyStyle(startPos,pos+1,style,STYLE_TEXT,StyleMask);
      else if (c == ']')
        ApplyStyle(startPos,pos+1,style,STYLE_QUOTE,StyleMask);
    }
    else if (style == STYLE_INFORM6)
    {
      if (c == ')')
      {
        if ((char)CallEdit(SCI_GETCHARAT,pos-1) == '-')
          ApplyStyle(startPos,pos+1,style,STYLE_TEXT,StyleMask);
      }
    }
    else if (style == STYLE_HEADING)
    {
      if ((c == '\r') || (c == '\n'))
        ApplyStyle(startPos,pos,style,STYLE_TEXT,StyleMask);
    }
    else if ((style >= STYLE_COMMENT) && (style < STYLE_COMMENT+NEST_COMMENTS))
    {
      int nested = style-STYLE_COMMENT;

      if (c == '[')
      {
        // For a nested comment, increase the nesting value
        if (nested < NEST_COMMENTS-1)
          ApplyStyle(startPos,pos,style,style+1,StyleMask);
      }
      else if (c == ']')
      {
        // For the end of a comment, either decrease the nesting value, or end the comment
        if (nested < 1)
          ApplyStyle(startPos,pos+1,style,STYLE_TEXT,StyleMask);
        else
          ApplyStyle(startPos,pos+1,style,style-1,StyleMask);
      }
    }
    else
    {
      // Check for a title?
      if ((pos == 0) && IsQuote(c,pos,false))
      {
        // Get the first line
        CString line;
        int endPos = (int)CallEdit(SCI_GETLINEENDPOSITION,0);
        TextRange range;
        range.chrg.cpMin = 0;
        range.chrg.cpMax = endPos;
        range.lpstrText = line.GetBuffer(endPos+1);
        CallEdit(SCI_GETTEXTRANGE,0,(sptr_t)&range);
        line.ReleaseBuffer();

        // Replace any Unicode quotes with normal ones (see the comments in IsQuote())
        line.Replace("\xe2\x80\x9c","\"");
        line.Replace("\xe2\x80\x9d","\"");

        // Look for an opening quote
        if (line.GetAt(0) == '\"')
        {
          bool title = false;
          if (line.Find("\" by ") > 0)
          {
            // Found a title followed by the author name
            title = true;
          }
          else if (line.GetAt(line.GetLength()-1) == '\"')
          {
            // Found a title on its own
            title = true;
          }

          if (title)
          {
            ApplyStyle(startPos,pos,style,STYLE_HEADING,StyleMask);
            pos = endPos;
            c = (unsigned char)CallEdit(SCI_GETCHARAT,pos);
            AddHeading(Title,range.lpstrText,startPos);
            ApplyStyle(startPos,pos,style,STYLE_TEXT,StyleMask);
            pos++;
            continue;
          }
        }
      }
      if (pos == 0)
        AddHeading(Title,"The Whole Source Text",0);

      // Check for a heading?
      if (newLine && (isalpha(c) || (c == '-')))
      {
        // Get the start of the line
        int len = (int)CallEdit(SCI_GETLENGTH);
        TextRange range;
        range.chrg.cpMin = pos;
        range.chrg.cpMax = min(pos+24,len);
        char text[26];
        range.lpstrText = text;
        CallEdit(SCI_GETTEXTRANGE,0,(sptr_t)&range);

        // Check for a heading match
        HeadingLevel hl = IsHeading(text,includeExt);
        if (hl != No_Heading)
        {
          ApplyStyle(startPos,pos,style,STYLE_HEADING,StyleMask);

          // Style the whole line
          int line = (int)CallEdit(SCI_LINEFROMPOSITION,pos);
          pos = (int)CallEdit(SCI_GETLINEENDPOSITION,line);
          c = (unsigned char)CallEdit(SCI_GETCHARAT,pos);
          AddHeading(hl,NULL,startPos);
          ApplyStyle(startPos,pos,style,STYLE_TEXT,StyleMask);
          pos++;
        }
      }

      // Plain text
      if (c == '[')
        ApplyStyle(startPos,pos,style,STYLE_COMMENT,StyleMask);
      else if (c == '(')
      {
        if ((char)CallEdit(SCI_GETCHARAT,pos+1) == '-')
          ApplyStyle(startPos,pos,style,STYLE_INFORM6,StyleMask);
      }
      else if (IsQuote(c,pos,false))
      {
        // A double quote indicates the start of a string only if it is at the
        // start of a line or preceeded by white space or punctuation
        if (strchr(beforeQuote,(char)CallEdit(SCI_GETCHARAT,pos-1)) != NULL)
          ApplyStyle(startPos,pos,style,STYLE_QUOTE,StyleMask);
      }
    }
    pos++;
  }

  // Apply the final style
  if (pos > len)
    pos = len;
  ApplyStyle(startPos,pos,style,style,StyleMask);

  if (includeExt && (m_action == LexHeadings))
  {
    // Find the documentation part, if present
    int doc = -1;
    CStringW dh(docHeading);
    for (int i = 0; i < m_headings.GetSize(); i++)
    {
      const Heading& heading = m_headings.GetAt(i);
      if (heading.level == ExtensionPart)
      {
        if (heading.name.Left(dh.GetLength()).CompareNoCase(dh) == 0)
        {
          doc = i;
          break;
        }
      }
    }

    // If the documentation part is present, rename it and add an "extension" part
    if (doc > 0)
    {
      m_headings.GetAt(doc).name = "Documentation";
      m_headings.InsertAt(1,Heading(ExtensionPart,"Extension",0));
    }
  }
}

SourceLexer::HeadingLevel SourceLexer::IsHeading(const char* line, bool includeExt)
{
  // Check for an extension documentation heading
  if (includeExt)
  {
    size_t hlen = strlen(docHeading);
    if (strnicmp(line,docHeading,hlen) == 0)
      return ExtensionPart;
  }

  // Check for a heading match
  for (int i = 0; i < sizeof headings / sizeof headings[0]; i++)
  {
    size_t hlen = strlen(headings[i]);
    if (strnicmp(line,headings[i],hlen) == 0)
    {
      // Got a possible match, so check it is followed by whitespace, or a colon (for extensions)
      wchar_t follow = line[hlen];
      if ((follow == L' ') || (follow == L'\t'))
        return (HeadingLevel)(Volume+i);
      if (includeExt && (follow == L':'))
        return (HeadingLevel)(Volume+i);
    }
  }
  return No_Heading;
}

const CArray<SourceLexer::Heading>& SourceLexer::GetHeadings(void) const
{
  return m_headings;
}

void SourceLexer::ApplyStyle(int& start, int pos, int& style, int newStyle, int mask)
{
  if (m_action == LexApply)
  {
    CallEdit(SCI_STARTSTYLING,start,mask);
    CallEdit(SCI_SETSTYLING,pos-start,style);
  }
  start = pos;
  style = newStyle;
}

void SourceLexer::AddHeading(HeadingLevel level, LPCSTR name, int pos)
{
  if (m_action == LexHeadings)
  {
    int line = (int)CallEdit(SCI_LINEFROMPOSITION,pos);
    if (name == NULL)
    {
      int endPos = (int)CallEdit(SCI_GETLINEENDPOSITION,line);
      name = (char*)alloca(endPos-pos+2);
      TextRange range;
      range.chrg.cpMin = pos;
      range.chrg.cpMax = endPos;
      range.lpstrText = (char*)name;
      CallEdit(SCI_GETTEXTRANGE,0,(sptr_t)&range);
    }
    m_headings.Add(Heading(level,name,line));
  }
}

bool SourceLexer::IsQuote(unsigned char c, int& pos, bool advance)
{
  // Check for the usual double quote character
  if (c == '\"')
    return true;

  // Check for Unicode left- and right-double quotes (0x201C and 0x201D)
  // In UTF-8, 0x201C is 0xE2 0x80 0x9C
  if (c == 0xE2)
  {
    c = (unsigned char)CallEdit(SCI_GETCHARAT,pos+1);
    if (c == 0x80)
    {
      c = (unsigned char)CallEdit(SCI_GETCHARAT,pos+2);
      if ((c == 0x9C) || (c == 0x9D))
      {
        if (advance)
          pos += 2;
        return true;
      }
    }
  }
  return false;
}

LONG_PTR SourceLexer::CallEdit(UINT msg, DWORD wp, LONG_PTR lp)
{
  return m_edit->CallEdit(msg,wp,lp);
}

SourceLexer::Heading::Heading()
{
  level = Section;
  line = 0;
}

SourceLexer::Heading::Heading(HeadingLevel lv, LPCSTR n, int ln)
{
  level = lv;
  line = ln;
  name = TextFormat::UTF8ToUnicode(n).Trim();
  for (int i = 0; i < name.GetLength(); i++)
  {
    if (iswspace(name.GetAt(i)))
      name.SetAt(i,L' ');
  }
}
