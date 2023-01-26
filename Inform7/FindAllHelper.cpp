#include "stdafx.h"
#include "FindAllHelper.h"
#include "Messages.h"
#include "TextFormat.h"

#include "DarkMode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static COLORREF Darken(COLORREF colour)
{
  BYTE r = GetRValue(colour);
  BYTE g = GetGValue(colour);
  BYTE b = GetBValue(colour);
  r = (BYTE)(r * 0.9333);
  g = (BYTE)(g * 0.9333);
  b = (BYTE)(b * 0.9333);
  return RGB(r,g,b);
}

FindResult::FindResult()
{
  type = FoundIn_Unknown;
  inContext.cpMin = 0;
  inContext.cpMax = 0;
  loc.cpMin = 0;
  loc.cpMax = 0;
}

bool FindResult::operator<(const FindResult& fr) const
{
  if (type != fr.type)
    return type < fr.type;
  if (docSort != fr.docSort)
    return docSort < fr.docSort;
  if (doc != fr.doc)
    return doc < fr.doc;
  if (loc.cpMin != fr.loc.cpMin)
    return loc.cpMin < fr.loc.cpMin;
  return loc.cpMax < fr.loc.cpMax;
}

CString FindResult::TypeName(void)
{
  switch (type)
  {
  case FoundIn_Source:
    return "Source";
  case FoundIn_Extension:
    return "Extensions";
  case FoundIn_WritingWithInform:
    return "Writing with Inform";
  case FoundIn_RecipeBook:
    return "The Inform Recipe Book";
  }
  return "";
}

COLORREF FindResult::Colour(DarkMode* dark, bool darker)
{
  if (dark)
  {
    COLORREF colour = dark->GetColour(darker ? DarkMode::Back : DarkMode::Dark3);
    if (type == FoundIn_RecipeBook)
      colour &= 0xFF00FFFF; // Make yellow-ish by setting blue component to zero
    return colour;
  }
  else
  {
    COLORREF colour = theApp.GetColour(
      (type == FoundIn_RecipeBook) ? InformApp::ColourContents : InformApp::ColourBack);
    return darker ? Darken(colour) : colour;
  }
}

FindResultsCtrl::FindResultsCtrl()
{
}

void FindResultsCtrl::SetDarkMode(DarkMode* dark)
{
  if (GetSafeHwnd() != 0)
  {
    if (dark)
      SetBkColor(dark->GetColour(DarkMode::Back));
    else
      SetBkColor(theApp.GetColour(InformApp::ColourBack));
  }
}

BEGIN_MESSAGE_MAP(FindResultsCtrl, CListCtrl)
  ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHeaderDividerDblClick)
  ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHeaderDividerDblClick)
END_MESSAGE_MAP()

BOOL FindResultsCtrl::PreTranslateMessage(MSG* pMsg)
{
  if ((pMsg->hwnd == GetSafeHwnd()) || IsChild(CWnd::FromHandle(pMsg->hwnd)))
  {
    // Ctrl+(NumPad+) resizes the columns of a listview to fit, so send a message to get the first column width right
    if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ADD) && ((::GetKeyState(VK_CONTROL) & 0x8000) != 0))
      GetParent()->PostMessage(WM_RESIZERESULTS);
  }
  return CListCtrl::PreTranslateMessage(pMsg);
}

void FindResultsCtrl::OnHeaderDividerDblClick(NMHDR* pNotifyStruct, LRESULT* result)
{
  NMHEADER* hdr = (NMHEADER*)pNotifyStruct;
  if (hdr->iItem == 0)
    GetParent()->SendMessage(WM_RESIZERESULTS);
  else
    Default();
}

void FindAllHelper::InitResultsCtrl(FindResultsCtrl* ctrl, bool details)
{
  ctrl->SetFont(theApp.GetFont(ctrl,InformApp::FontSmall));
  ctrl->SetExtendedStyle(LVS_EX_FULLROWSELECT);
  ctrl->InsertColumn(0,"Result");
  if (details)
  {
    ctrl->InsertColumn(1,"Document");
    ctrl->InsertColumn(2,"Type");
  }
}

void FindAllHelper::UpdateResultsCtrl(FindResultsCtrl* ctrl, bool details)
{
  ctrl->DeleteAllItems();
  ctrl->SetRedraw(FALSE);
  ctrl->SetItemCount((int)results.size());
  for (int i = 0; i < (int)results.size(); i++)
  {
    ctrl->InsertItem(i,"");
    if (details)
    {
      ctrl->SetItemText(i,1,results[i].doc);
      ctrl->SetItemText(i,2,results[i].TypeName());
    }
  }
  ctrl->SetRedraw(TRUE);

  CRect resultsRect;
  ctrl->GetWindowRect(resultsRect);

  int remain = resultsRect.Width() - (::GetSystemMetrics(SM_CXVSCROLL)+4);
  if (details)
  {
    // Resize all but the first column to be as wide as the least of their contents, or 25%
    int max = (int)(0.25 * resultsRect.Width());
    for (int i = 1; i <= 2; i++)
    {
      ctrl->SetColumnWidth(i,LVSCW_AUTOSIZE);
      if (ctrl->GetColumnWidth(i) > max)
        ctrl->SetColumnWidth(i,max);
      remain -= ctrl->GetColumnWidth(i);
    }
  }

  // Resize the first column to take up the remaining space
  ctrl->SetColumnWidth(0,remain);
}

void FindAllHelper::OnResultsDraw(FindResultsCtrl* ctrl, NMLVCUSTOMDRAW* custom, LRESULT* result)
{
  // Default to letting Windows draw the control
  *result = CDRF_DODEFAULT;

  // Work out where we are in the drawing process
  switch (custom->nmcd.dwDrawStage)
  {
  case CDDS_PREPAINT:
    // Tell us when an item is drawn
    *result = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    // Tell us when a sub-item is drawn
    *result = CDRF_NOTIFYSUBITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
    {
      // Make sure that we have a result for the given item
      int item = (int)custom->nmcd.dwItemSpec;
      if (item >= results.size())
        return;

      // Work out the background colour
      DarkMode* dark = DarkMode::GetActive(ctrl);
      COLORREF backColour = results[item].Colour(dark,(item % 2) != 0);

      // Get if the item is selected
      bool selected = false;
      if (CWnd::GetFocus() == ctrl)
      {
        if (item == ctrl->GetNextItem(-1,LVNI_SELECTED))
          selected = true;
      }

      // Get the bounding rectangle for drawing the text
      CRect rect;
      ctrl->GetSubItemRect(item,custom->iSubItem,LVIR_LABEL,rect);
      CRect textRect = rect;
      textRect.DeflateRect(2,0);

      // Set up the device context
      HDC hdc = 0;
      HANDLE pb = ::BeginBufferedPaint(custom->nmcd.hdc,rect,BPBF_COMPATIBLEBITMAP,NULL,&hdc);
      if (pb == 0)
        return;
      CDC* dc = CDC::FromHandle(hdc);
      if (dark)
        dc->SetTextColor(dark->GetColour(selected ? DarkMode::Back : DarkMode::Fore));
      else
        dc->SetTextColor(::GetSysColor(selected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
      dc->SetBkMode(TRANSPARENT);
      CFont* oldFont = dc->SelectObject(ctrl->GetFont());

      // Draw the background
      if (dark)
        dc->FillSolidRect(rect,selected ? dark->GetColour(DarkMode::Dark1) : backColour);
      else
        dc->FillSolidRect(rect,selected ? ::GetSysColor(COLOR_HIGHLIGHT) : backColour);

      // Special case painting of the first column
      if (custom->iSubItem == 0)
      {
        // Get the text
        const CStringW& prefix = results[item].prefix;
        const CStringW& text = results[item].context;
        int high1 = results[item].inContext.cpMin;
        int high2 = results[item].inContext.cpMax;

        // Draw the text
        if (!prefix.IsEmpty())
        {
          LOGFONT logFont;
          ctrl->GetFont()->GetLogFont(&logFont);
          logFont.lfItalic = TRUE;
          CFont italicFont;
          italicFont.CreateFontIndirect(&logFont);
          CFont* previousFont = dc->SelectObject(&italicFont);
          DrawText(dc,prefix,prefix.GetLength(),textRect,DT_VCENTER|DT_NOPREFIX);
          dc->SelectObject(previousFont);
        }
        DrawText(dc,text.GetString(),high1,textRect,DT_VCENTER|DT_NOPREFIX);
        if (textRect.left < textRect.right)
        {
          LOGFONT logFont;
          ctrl->GetFont()->GetLogFont(&logFont);
          logFont.lfWeight = FW_BOLD;
          CFont boldFont;
          boldFont.CreateFontIndirect(&logFont);
          CFont* previousFont = dc->SelectObject(&boldFont);
          DrawText(dc,text.GetString()+high1,high2-high1,textRect,DT_VCENTER|DT_NOPREFIX);
          dc->SelectObject(previousFont);
        }
        if (textRect.left < textRect.right)
        {
          DrawText(dc,text.GetString()+high2,text.GetLength()-high2,textRect,
            DT_VCENTER|DT_WORD_ELLIPSIS|DT_NOPREFIX);
        }
      }
      else
      {
        CString text = ctrl->GetItemText(item,custom->iSubItem);
        dc->DrawText(text,textRect,DT_VCENTER|DT_SINGLELINE|DT_WORD_ELLIPSIS|DT_NOPREFIX);
      }

      dc->SelectObject(oldFont);
      ::EndBufferedPaint(pb,TRUE);
      *result = CDRF_SKIPDEFAULT;
    }
    break;
  }
}

void FindAllHelper::OnResultsResize(FindResultsCtrl* ctrl)
{
  // Set up a device context
  CDC* dc = ctrl->GetDC();
  CFont* oldFont = dc->SelectObject(ctrl->GetFont());

  // Create bold and italic fonts
  LOGFONT logFont;
  ctrl->GetFont()->GetLogFont(&logFont);
  logFont.lfWeight = FW_BOLD;
  CFont boldFont;
  boldFont.CreateFontIndirect(&logFont);
  logFont.lfWeight = FW_NORMAL;
  logFont.lfItalic = TRUE;
  CFont italicFont;
  italicFont.CreateFontIndirect(&logFont);

  int colWidth = 0;
  for (int i = 0; i < (int)results.size(); i++)
  {
    // Get the text
    const CStringW& prefix = results[i].prefix;
    const CStringW& text = results[i].context;
    int high1 = results[i].inContext.cpMin;
    int high2 = results[i].inContext.cpMax;

    // Measure the text
    int width = 0;
    if (!prefix.IsEmpty())
    {
      dc->SelectObject(&italicFont);
      width += MeasureText(dc,prefix.GetString(),prefix.GetLength());
    }
    dc->SelectObject(ctrl->GetFont());
    width += MeasureText(dc,text.GetString(),high1);
    dc->SelectObject(&boldFont);
    width += MeasureText(dc,text.GetString()+high1,high2-high1);
    dc->SelectObject(ctrl->GetFont());
    width += MeasureText(dc,text.GetString()+high2,text.GetLength()-high2);

    if (width > colWidth)
      colWidth = width;
  }

  // Free the device context
  dc->SelectObject(oldFont);
  ctrl->ReleaseDC(dc);

  ctrl->SetColumnWidth(0,colWidth+8);
}

void FindAllHelper::Find(const CString& textUtf8, const CStringW& findText, bool ignoreCase, FindRule findRule,
  const char* doc, const char* docSort, const char* path, const char* prefix, FoundIn type)
{
  // Set up a regular expression
  CString findTextUtf8 = TextFormat::UnicodeToUTF8(findText);
  std::regex::flag_type flags = std::regex::ECMAScript;
  if (ignoreCase)
    flags |= std::regex::icase;
  if (findRule != FindRule_Regex)
  {
    // Escape any characters with a special meaning in regular expressions
    for (int i = 0; i < findTextUtf8.GetLength(); i++)
    {
      if (strchr(".^$|()[]{}*+?\\",findTextUtf8.GetAt(i)))
      {
        findTextUtf8.Insert(i,'\\');
        i++;
      }
    }      
  }
  switch (findRule)
  {
  case FindRule_StartsWith:
    findTextUtf8.Insert(0,"\\b");
    break;
  case FindRule_FullWord:
    findTextUtf8.Insert(0,"\\b");
    findTextUtf8.Append("\\b");
    break;
  }
  std::regex regexp;
  regexp.assign(findTextUtf8,flags);

  // Search for the text
  std::cregex_iterator regexIt(textUtf8.GetString(),textUtf8.GetString()+textUtf8.GetLength(),regexp);
  for (; regexIt != std::cregex_iterator(); ++regexIt)
  {
    if (regexIt->length() <= 0)
      return;

    int matchStart = (int)regexIt->position();
    int matchEnd = (int)(regexIt->position() + regexIt->length());

    // Get the surrounding text as context. Note that we get the leading, matching and
    // trailing texts separately so that we can count the number of Unicode characters in each.
    int lineStart = FindLineStart(textUtf8,matchStart);
    int lineEnd = FindLineEnd(textUtf8,matchEnd);
    CStringW leading = GetMatchRange(textUtf8,lineStart,matchStart);
    leading.Replace(L'\t',L' ');
    leading.TrimLeft();
    CStringW match = GetMatchRange(textUtf8,matchStart,matchEnd);
    CStringW trailing = GetMatchRange(textUtf8,matchEnd,lineEnd);
    trailing.Replace(L'\t',L' ');
    trailing.TrimRight();
    CStringW context = leading + match + trailing;
    context.Replace(L'\n',L' ');
    context.Replace(L'\r',L' ');
    context.Replace(L'\t',L' ');

    // Store the found result
    FindResult result;
    result.prefix = TextFormat::UTF8ToUnicode(prefix);
    result.context = context;
    result.inContext.cpMin = leading.GetLength();
    result.inContext.cpMax = leading.GetLength() + match.GetLength();
    result.type = type;
    result.doc = doc;
    result.docSort = docSort;
    result.path = path;
    result.loc.cpMin = matchStart;
    result.loc.cpMax = matchEnd;
    results.push_back(result);
  }
}

const char* FindAllHelper::RegexError(const std::regex_error& ex)
{
  switch (ex.code())
  {
  case std::regex_constants::error_collate:
    return "The find expression contained an invalid collating element name.";
  case std::regex_constants::error_ctype:
    return "The find expression contained an invalid character class name.";
  case std::regex_constants::error_escape:
    return "The find expression contained an invalid escaped character, or a trailing escape.";
  case std::regex_constants::error_backref:
    return "The find expression contained an invalid back reference.";
  case std::regex_constants::error_brack:
    return "The find expression contained mismatched [ and ].";
  case std::regex_constants::error_paren:
    return "The find expression contained mismatched ( and ).";
  case std::regex_constants::error_brace:
    return "The find expression contained mismatched { and }.";
  case std::regex_constants::error_badbrace:
    return "The find expression contained an invalid range in a { expression }.";
  case std::regex_constants::error_range:
    return "The find expression contained an invalid character range, such as [b-a].";
  case std::regex_constants::error_space:
    return "Insufficient memory to process the find expression.";
  case std::regex_constants::error_badrepeat:
    return "One of *?+{ was not preceded by a valid regular expression.";
  case std::regex_constants::error_complexity:
    return "The complexity of an attempted match against a regular expression was too much.";
  case std::regex_constants::error_stack:
    return "There was insufficient memory to determine whether the regular expression could match the specified character sequence.";
  case std::regex_constants::error_parse:
    return "Failed to parse find expression.";
  case std::regex_constants::error_syntax:
    return "Syntax error in find expression.";
  default:
    return "Error in find expression.";
  }
}

int FindAllHelper::FindLineStart(const CString& text, int pos)
{
  while (pos > 0)
  {
    char c = text.GetAt(pos-1);
    if ((c == '\n') || (c == '\r'))
      return pos;
    pos--;
  }
  return pos;
}

int FindAllHelper::FindLineEnd(const CString& text, int pos)
{
  int len = text.GetLength();
  while (pos < len)
  {
    char c = text.GetAt(pos);
    if ((c == '\n') || (c == '\r'))
      return pos;
    pos++;
  }
  return pos;
}

CStringW FindAllHelper::GetMatchRange(const CString& text, int start, int end)
{
  return TextFormat::UTF8ToUnicode(text.Mid(start,end-start));
}

void FindAllHelper::DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format)
{
  if (length > 0)
  {
    ::DrawTextW(dc->GetSafeHdc(),text,length,rect,format|DT_SINGLELINE);

    CRect measure(rect);
    ::DrawTextW(dc->GetSafeHdc(),text,length,measure,format|DT_SINGLELINE|DT_CALCRECT);
    rect.left += measure.Width();
  }
}

int FindAllHelper::MeasureText(CDC* dc, LPCWSTR text, int length)
{
  if (length > 0)
  {
    SIZE textSize;
    ::GetTextExtentPoint32W(dc->GetSafeHdc(),text,length,&textSize);
    return textSize.cx;
  }
  return 0;
}
