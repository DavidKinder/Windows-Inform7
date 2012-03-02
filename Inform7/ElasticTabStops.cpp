// Conversion of Nick Gravgaard's elastic tabstop GEdit plugin to Scintilla, modified to follow
// Andrew Hunder's Inform 7 OS X implementation in terms of where to apply elastic tabstops
// (that is, only on lines that do not start with tabstops, and are not purely whitespace).

#include "stdafx.h"

#include <malloc.h>
#include <vector>

#include "Platform.h"
#include "Scintilla.h"

extern "C" sptr_t __stdcall Scintilla_DirectFunction(sptr_t, UINT, uptr_t, sptr_t);

LONG_PTR call_edit(sptr_t edit, UINT msg, DWORD wp = 0, LONG_PTR lp = 0)
{
  return Scintilla_DirectFunction(edit,msg,wp,lp);
}

int get_line_start(sptr_t edit, int pos)
{
  int line = call_edit(edit,SCI_LINEFROMPOSITION,pos);
  return call_edit(edit,SCI_POSITIONFROMLINE,line);
}

int get_line_end(sptr_t edit, int pos)
{
  int line = call_edit(edit,SCI_LINEFROMPOSITION,pos);
  return call_edit(edit,SCI_GETLINEENDPOSITION,line);
}

bool is_line_end(sptr_t edit, int pos)
{
  int line = call_edit(edit,SCI_LINEFROMPOSITION,pos);
  int end_pos = call_edit(edit,SCI_GETLINEENDPOSITION,line);
  return (pos == end_pos);
}

struct et_tabstop
{
  int text_width_pix;
  int *widest_width_pix;
  bool ends_in_tab;
};

struct et_line
{
  int num_tabs;
};

enum direction
{
  BACKWARDS,
  FORWARDS
};

et_tabstop* grid_buffer = NULL;
int grid_buffer_size = 0;

// by default, tabstops are at least 40 pixels plus 16 pixels of padding
#define MINIMUM_WIDTH_DEFAULT 40
#define PADDING_WIDTH_DEFAULT 16

int tab_width_minimum = MINIMUM_WIDTH_DEFAULT;
int tab_width_padding = PADDING_WIDTH_DEFAULT;

int get_text_width(sptr_t edit, int start, int end)
{
  TextRange range;
  range.chrg.cpMin = start;
  range.chrg.cpMax = end;
  range.lpstrText = (char*)_alloca(end-start+1);
  call_edit(edit,SCI_GETTEXTRANGE,0,(sptr_t)&range);

  return call_edit(edit,SCI_TEXTWIDTH,0,(LONG_PTR)range.lpstrText);
}

int calc_tab_width(int text_width_in_tab)
{
  if (text_width_in_tab < tab_width_minimum)
  {
    text_width_in_tab = tab_width_minimum;
  }
  return text_width_in_tab + tab_width_padding;
}

bool change_line(sptr_t edit, int& location, direction which_dir)
{
  int line = call_edit(edit,SCI_LINEFROMPOSITION,location);
  if (which_dir == FORWARDS)
  {
    location = call_edit(edit,SCI_POSITIONFROMLINE,line+1);
  }
  else
  {
    if (line <= 0)
      return false;
    location = call_edit(edit,SCI_POSITIONFROMLINE,line-1);
  }
  return (location >= 0);
}

int get_block_boundary(sptr_t edit, int& location, direction which_dir)
{
  int current_pos;
  int max_tabs = 0;
  bool orig_line = true;

  location = get_line_start(edit,location);
  do
  {
    int tabs_on_line = 0;

    current_pos = location;
    unsigned char current_char = (unsigned char)call_edit(edit,SCI_GETCHARAT,current_pos);
    bool current_char_ends_line = is_line_end(edit,current_pos);

    while (current_char != '\0' && !current_char_ends_line)
    {
      if (current_char == '\t')
      {
        tabs_on_line++;
        if (tabs_on_line > max_tabs)
        {
          max_tabs = tabs_on_line;
        }
      }
      current_pos = call_edit(edit,SCI_POSITIONAFTER,current_pos);
      current_char = (unsigned char)call_edit(edit,SCI_GETCHARAT,current_pos);
      current_char_ends_line = is_line_end(edit,current_pos);
    }
    if (tabs_on_line == 0 && !orig_line)
    {
      return max_tabs;
    }
    orig_line = false;
  }
  while (change_line(edit,location,which_dir));
  return max_tabs;
}

int get_nof_tabs_between(sptr_t edit, int start, int end)
{
  int current_pos = get_line_start(edit,start);
  int max_tabs = 0;

  do
  {
    unsigned char current_char = (unsigned char)call_edit(edit,SCI_GETCHARAT,current_pos);
    bool current_char_ends_line = is_line_end(edit,current_pos);

    int tabs_on_line = 0;
    while (current_char != '\0' && !current_char_ends_line)
    {
      if (current_char == '\t')
      {
        tabs_on_line++;
        if (tabs_on_line > max_tabs)
        {
          max_tabs = tabs_on_line;
        }
      }
      current_pos = call_edit(edit,SCI_POSITIONAFTER,current_pos);
      current_char = (unsigned char)call_edit(edit,SCI_GETCHARAT,current_pos);
      current_char_ends_line = is_line_end(edit,current_pos);
    }
  }
  while (change_line(edit,current_pos,FORWARDS) && current_pos < end);
  return max_tabs;
}

void stretch_tabstops(sptr_t edit, int block_start_linenum, int block_nof_lines, int max_tabs)
{
  int l, t;
  et_line* lines = (et_line*)_alloca(sizeof (et_line) * block_nof_lines);
  memset(lines,0,sizeof (et_line) * block_nof_lines);

  int new_buffer_size = __max(8,sizeof (et_tabstop) * block_nof_lines * max_tabs);
  if (new_buffer_size > grid_buffer_size)
  {
    et_tabstop* new_buffer = (et_tabstop*)realloc(grid_buffer,new_buffer_size);
    if (new_buffer == NULL)
    {
      return;
    }
    grid_buffer = new_buffer;
    grid_buffer_size = new_buffer_size;
  }
  memset(grid_buffer,0,new_buffer_size);

  et_tabstop** grid = (et_tabstop**)_alloca(sizeof (et_tabstop*) * block_nof_lines);
  for (l = 0; l < block_nof_lines; l++)
  {
    grid[l] = grid_buffer + (l * max_tabs);
  }

  // get width of text in cells
  for (l = 0; l < block_nof_lines; l++) // for each line
  {
    int text_width_in_tab = 0;
    int current_line_num = block_start_linenum + l;
    int current_tab_num = 0;
    bool cell_empty = true;
    bool line_blank = true;
    bool line_uses_elastic_tabstops = true;

    int current_pos = call_edit(edit,SCI_POSITIONFROMLINE,current_line_num);
    int cell_start = current_pos;
    unsigned char current_char = (unsigned char)call_edit(edit,SCI_GETCHARAT,current_pos);
    bool current_char_ends_line = is_line_end(edit,current_pos);
    // maybe change this to search forwards for tabs/newlines

    while (current_char != '\0')
    {
      if (current_char_ends_line)
      {
        grid[l][current_tab_num].ends_in_tab = false;
        text_width_in_tab = 0;
        break;
      }
      else if (current_char == '\t')
      {
        if (line_blank)
        {
          line_uses_elastic_tabstops = false;
        }

        if (line_uses_elastic_tabstops)
        {
          if (!cell_empty)
          {
            text_width_in_tab = get_text_width(edit,cell_start,current_pos);
          }
          grid[l][current_tab_num].ends_in_tab = true;
          grid[l][current_tab_num].text_width_pix = calc_tab_width(text_width_in_tab);
          current_tab_num++;
          lines[l].num_tabs++;
          text_width_in_tab = 0;
          cell_empty = true;
        }
      }
      else
      {
        if (isspace(current_char) == 0)
        {
          line_blank = false;
        }

        if (cell_empty)
        {
          cell_start = current_pos;
          cell_empty = false;
        }
      }
      current_pos = call_edit(edit,SCI_POSITIONAFTER,current_pos);
      current_char = (unsigned char)call_edit(edit,SCI_GETCHARAT,current_pos);
      current_char_ends_line = is_line_end(edit,current_pos);
    }
  }

  // find columns blocks and stretch to fit the widest cell
  for (t = 0; t < max_tabs; t++) // for each column
  {
    bool starting_new_block = true;
    int first_line_in_block = 0;
    int max_width = 0;
    for (l = 0; l < block_nof_lines; l++) // for each line
    {
      if (starting_new_block)
      {
        starting_new_block = false;
        first_line_in_block = l;
        max_width = 0;
      }
      if (grid[l][t].ends_in_tab)
      {
        grid[l][t].widest_width_pix = &(grid[first_line_in_block][t].text_width_pix); // point widestWidthPix at first 
        if (grid[l][t].text_width_pix > max_width)
        {
          max_width = grid[l][t].text_width_pix;
          grid[first_line_in_block][t].text_width_pix = max_width;
        }
      }
      else // end column block
      {
        starting_new_block = true;
      }
    }
  }

  std::vector<int> tab_array;

  // set tabstops
  for (l = 0; l < block_nof_lines; l++) // for each line
  {
    int current_line_num = block_start_linenum + l;
    int acc_tabstop = 0;

    tab_array.resize(lines[l].num_tabs + 1);
    tab_array[lines[l].num_tabs] = 0;

    for (t = 0; t < lines[l].num_tabs; t++)
    {
      if (grid[l][t].widest_width_pix != NULL)
      {
        acc_tabstop += *(grid[l][t].widest_width_pix);
        tab_array[t] = acc_tabstop;
      }
      else
        tab_array[t] = 0;
    }

    call_edit(edit,SCIX_SETTABSTOPS,current_line_num,(LONG_PTR)&(tab_array.at(0)));
  }
}

void ElasticTabStops_OnModify(sptr_t edit, int start, int end)
{
  int tab_width = call_edit(edit,SCI_GETTABWIDTH) * call_edit(edit,SCI_TEXTWIDTH,0,(LONG_PTR)" ");
  tab_width_minimum = tab_width - tab_width_padding;
  if (tab_width_minimum < 8)
    tab_width_minimum = MINIMUM_WIDTH_DEFAULT;

  int max_tabs_between = get_nof_tabs_between(edit,start,end);
  int max_tabs_backwards = get_block_boundary(edit,start,BACKWARDS);
  int max_tabs_forwards = get_block_boundary(edit,end,FORWARDS);
  int max_tabs = __max(__max(max_tabs_between,max_tabs_backwards),max_tabs_forwards);

  int block_start_linenum = call_edit(edit,SCI_LINEFROMPOSITION,start);
  int block_end_linenum = call_edit(edit,SCI_LINEFROMPOSITION,end);
  int block_nof_lines = (block_end_linenum - block_start_linenum) + 1;

  stretch_tabstops(edit,block_start_linenum,block_nof_lines,max_tabs);
}

void ElasticTabStops_OnClear(sptr_t edit)
{
  int count = call_edit(edit,SCI_GETLINECOUNT);
  int null_stop = 0;
  for (int i = 0; i < count; i++)
    call_edit(edit,SCIX_SETTABSTOPS,i,(LONG_PTR)&null_stop);
}
