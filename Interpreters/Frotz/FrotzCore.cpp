extern "C"
{
typedef int f_bool;
#define bool f_bool
#include "frotz.h"
#undef bool

zword get_current_window(void);
int colour_in_use(zword colour);

#include "blorb.h"
#include "blorblow.h"
}

#include <malloc.h>
#include <windows.h>
#include "../../Inform7/InterpreterCommands.h"

bb_map_t* blorbMap;

int display_width = 0;
int display_height = 0;
int char_width = 0;
int char_height = 0;

void sendCommand(int command, int dataLength, const void* data)
{
  DWORD written = 0;
  HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);

  ::WriteFile(out,&command,sizeof command,&written,NULL);
  ::WriteFile(out,&dataLength,sizeof dataLength,&written,NULL);
  if (dataLength > 0)
    ::WriteFile(out,data,dataLength,&written,NULL);
}

void readReturnData(void* data, int length)
{
  HANDLE in = ::GetStdHandle(STD_INPUT_HANDLE);
  DWORD read = 0;

  ::ReadFile(in,data,length,&read,NULL);
  if (read != length)
    exit(1);
}

int readReturnCommand(int command)
{
  HANDLE in = ::GetStdHandle(STD_INPUT_HANDLE);

  // Wait for returned data
  while (true)
  {
    DWORD available = 0;
    ::PeekNamedPipe(in,NULL,0,NULL,&available,NULL);

    if (available > 0)
    {
      // Get the returned command
      int readCommand, readLength;
      readReturnData(&readCommand,sizeof readCommand);
      readReturnData(&readLength,sizeof readLength);

      // If the expected command, return the data length
      if (readCommand == command)
        return readLength;

      if (readCommand == Return_Size)
      {
        // Get the current screen size
        int new_size[2];
        readReturnData(new_size,sizeof new_size);

        h_screen_width = new_size[0];
        h_screen_height = new_size[1];
        h_screen_cols = h_screen_width / h_font_width;
        h_screen_rows = h_screen_height / h_font_height;
      }
      else
      {
        // Discard the data
        void* data = malloc(readLength);
        readReturnData(data,readLength);
        free(data);
      }
    }

    ::Sleep(100);
  }
}

zword outputBuffer[256];
int outputPos = 0;

void flushOutput(void)
{
  if (outputPos > 0)
  {
    sendCommand(Command_PrintOutput,
      outputPos * sizeof outputBuffer[0],outputBuffer);
    outputPos = 0;
  }
}

bool isValidChar(unsigned short c)
{
  if (c >= ZC_ASCII_MIN && c <= ZC_ASCII_MAX)
    return true;
  if (c >= ZC_LATIN1_MIN && c <= ZC_LATIN1_MAX)
    return true;
  if (c >= 0x100)
    return true;
  if (c == L'\n')
    return true;
  return false;
}

unsigned short zcolours[11] =
{
  0x0000,
  0x001D,
  0x0340,
  0x03BD,
  0x59A0,
  0x7C1F,
  0x77A0,
  0x7FFF,
  0x5AD6,
  0x4631,
  0x2D6B
};

#define NON_STD_COLS (256-18)
unsigned short nonStdColours[NON_STD_COLS];
int nonStdIndex = 0;

unsigned short getColour(int index)
{
  if ((index >= BLACK_COLOUR) && (index <= DARKGREY_COLOUR))
    return zcolours[index-BLACK_COLOUR];
  if ((index >= 18) && (index < 256))
  {
    if (nonStdColours[index-18] != 0xFFFF)
      return nonStdColours[index-18];
  }
  return 0;
}

int getColourIndex(unsigned short colour)
{
  for (int i = 0; i < 11; i++)
  {
    if (zcolours[i] == colour)
      return i+BLACK_COLOUR;
  }
  for (i = 0; i < NON_STD_COLS; i++)
  {
    if (nonStdColours[i] == colour)
      return i+18;
  }

  int index = -1;
  while (index == -1)
  {
    if (colour_in_use(nonStdIndex+18) == 0)
    {
      nonStdColours[nonStdIndex] = colour;
      index = nonStdIndex+18;
    }

    nonStdIndex++;
    if (nonStdIndex >= NON_STD_COLS)
      nonStdIndex = 0;
  }
  return index;
}

void rgb5ToTrue(unsigned short five, int& r, int& g, int& b)
{
  r = five&0x001F;
  r = (r<<3)|(r>>2);
  g = (five&0x03E0)>>5;
  g = (g<<3)|(g>>2);
  b = (five&0x7C00)>>10;
  b = (b<<3)|(b>>2);
}

int sent_height = 0;
int real_height = 0;

void updateHeight(bool clear)
{
  if (real_height != sent_height)
  {
    int data[5];
    data[0] = 2;
    data[1] = Method_Above|Method_Fixed;
    data[2] = (int)(real_height/char_height);
    data[3] = 1;
    data[4] = 0;
    sendCommand(Command_ArrangeWindow,sizeof data,data);
    sent_height = real_height;
  }
  if (clear)
    sent_height = -1;
}

int win0_style = 0;
int win1_style = 0;
int win0_font = 0;

void sendStyle(int win)
{
  int data[3];
  data[0] = win;
  data[1] = StyleNormal;
  data[2] = 0;

  switch (win)
  {
  case 0:
    data[1] = win0_style;
    if (win0_font == FIXED_WIDTH_FONT)
      data[1] |= StyleFixed;
    break;
  case 1:
    data[1] = win1_style;
    break;
  }

  sendCommand(Command_SetStyle,sizeof data,data);
}

/*
 * os_beep
 *
 * Play a beep sound. Ideally, the sound should be high- (number == 1)
 * or low-pitched (number == 2).
 *
 */
extern "C" void os_beep(int number)
{
}

/*
 * os_display_char
 *
 * Display a character of the current font using the current colours and
 * text style. The cursor moves to the next position. Printable codes are
 * all ASCII values from 32 to 126, ISO Latin-1 characters from 160 to
 * 255, ZC_GAP (gap between two sentences) and ZC_INDENT (paragraph
 * indentation), and Unicode characters above 255. The screen should not
 * be scrolled after printing to the bottom right corner.
 *
 */
extern "C" void os_display_char(zword c)
{
  if (c == ZC_INDENT)
  {
    os_display_char(' ');
    os_display_char(' ');
    os_display_char(' ');
  }
  else if (c == ZC_GAP)
  {
    os_display_char(' ');
    os_display_char(' ');
  }
  else if (isValidChar(c))
  {
    // Is the buffer full?
    if (outputPos == (sizeof outputBuffer / sizeof outputBuffer[0]))
      flushOutput();

    // If this is the first character, send the output window first
    if (outputPos == 0)
      outputBuffer[outputPos++] = get_current_window();
    outputBuffer[outputPos++] = c;
  }
}

/*
 * os_display_string
 *
 * Pass a string of characters to os_display_char.
 *
 */
extern "C" void os_display_string(const zword *s)
{
  zword c;
  while ((c = *s++) != 0)
  {
    if ((c == ZC_NEW_FONT) || (c == ZC_NEW_STYLE))
    {
      int arg = *s++;
      if (c == ZC_NEW_FONT)
        os_set_font(arg);
      if (c == ZC_NEW_STYLE)
        os_set_text_style(arg);
    }
    else
      os_display_char(c);
  }
}

/*
 * os_erase_area
 *
 * Fill a rectangular area of the screen with the current background
 * colour. Top left coordinates are (1,1). The cursor does not move.
 *
 * The final argument gives the window being changed, -1 if only a
 * portion of a window is being erased, or -2 if the whole screen is
 * being erased.
 *
 */
extern "C" void os_erase_area(int top, int left, int bottom, int right, int win)
{
  flushOutput();
  updateHeight(true);

  switch (win)
  {
  case -2:
    {
      int data[2];
      data[0] = 0;
      data[1] = 0;
      sendCommand(Command_Clear,sizeof data,data);
      data[0] = 1;
      sendCommand(Command_Clear,sizeof data,data);
    }
    break;
  case -1:
    // Do nothing
    break;
  case 0:
  case 1:
    {
      int data[2];
      data[0] = win;
      data[1] = 0;
      sendCommand(Command_Clear,sizeof data,data);
    }
    break;
  }
}

/*
 * os_fatal
 *
 * Display error message and stop interpreter.
 *
 */
extern "C" void os_fatal(const char *s)
{
  flushOutput();
  sendCommand(Command_FatalError,strlen(s) * sizeof s[0],s);

  exit(0);
}

/*
 * os_font_data
 *
 * Return true if the given font is available. The font can be
 *
 *    TEXT_FONT
 *    PICTURE_FONT
 *    GRAPHICS_FONT
 *    FIXED_WIDTH_FONT
 *
 * The font size should be stored in "height" and "width". If
 * the given font is unavailable then these values must _not_
 * be changed.
 *
 */
extern "C" int os_font_data(int font, int *height, int *width)
{
  switch (font)
  {
  case TEXT_FONT:
  case FIXED_WIDTH_FONT:
    *height = (zbyte)char_height;
    *width = (zbyte)char_width;
    return 1;
  }
  return 0;
}

/*
 * os_read_file_name
 *
 * Return the name of a file. Flag can be one of:
 *
 *    FILE_SAVE     - Save game file
 *    FILE_RESTORE  - Restore game file
 *    FILE_SCRIPT   - Transcript file
 *    FILE_RECORD   - Command file for recording
 *    FILE_PLAYBACK - Command file for playback
 *    FILE_SAVE_AUX - Save auxiliary ("preferred settings") file
 *    FILE_LOAD_AUX - Load auxiliary ("preferred settings") file
 *
 * The length of the file name is limited by MAX_FILE_NAME. Ideally
 * an interpreter should open a file requester to ask for the file
 * name. If it is unable to do that then this function should call
 * print_string and read_string to ask for a file name.
 *
 */
extern "C" int os_read_file_name(char *file_name, const char *default_name, int flag)
{
  flushOutput();

  // Always fail
  return 0;
}

/*
 * os_init_screen
 *
 * Initialise the IO interface. Prepare screen and other devices
 * (mouse, sound card). Set various OS depending story file header
 * entries:
 *
 *     h_config (aka flags 1)
 *     h_flags (aka flags 2)
 *     h_screen_cols (aka screen width in characters)
 *     h_screen_rows (aka screen height in lines)
 *     h_screen_width
 *     h_screen_height
 *     h_font_height (defaults to 1)
 *     h_font_width (defaults to 1)
 *     h_default_foreground
 *     h_default_background
 *     h_interpreter_number
 *     h_interpreter_version
 *     h_user_name (optional; not used by any game)
 *
 * Finally, set reserve_mem to the amount of memory (in bytes) that
 * should not be used for multiple undo and reserved for later use.
 *
 */
extern "C" void os_init_screen(void)
{
  int data[6];
  data[0] = 0;
  data[1] = -1;
  data[2] = -1;
  data[3] = 0;
  data[4] = 0;
  data[5] = Window_Text;
  sendCommand(Command_CreateWindow,sizeof data,data);
  data[0] = 1;
  data[1] = 0;
  data[2] = 2;
  data[3] = Method_Above|Method_Fixed;
  data[4] = 0;
  data[5] = Window_Grid;
  sendCommand(Command_CreateWindow,sizeof data,data);

  if ((h_version != V5) && (h_version != V8))
  {
    os_display_string(L"Only Z-code versions 5 and 8 are supported.\n");
    flushOutput();
    exit(0);
  }

  for (int i = 0; i < NON_STD_COLS; i++)
    nonStdColours[i] = 0xFFFF;

  h_config |= CONFIG_BOLDFACE|CONFIG_EMPHASIS|CONFIG_FIXED|CONFIG_COLOUR|CONFIG_SOUND;
  h_config &= ~CONFIG_TIMEDINPUT;
  h_flags &= ~MOUSE_FLAG;

  h_interpreter_version = 'F';
  h_interpreter_number = INTERP_MSDOS;
  h_default_foreground = BLACK_COLOUR;
  h_default_background = WHITE_COLOUR;
  h_font_width = char_width;
  h_font_height = char_height;
  h_screen_width = display_width;
  h_screen_height = display_height;
  h_screen_cols = h_screen_width / h_font_width;
  h_screen_rows = h_screen_height / h_font_height;

  h_standard_high = 1;
  h_standard_low = 1;

  hx_flags = 0;
  hx_fore_colour = 0x0000;
  hx_back_colour = 0x7FFF;

  err_report_mode = ERR_REPORT_ALWAYS;

  win0_style = 0;
  win1_style = 0;
  win0_font = TEXT_FONT;
}

/*
 * os_more_prompt
 *
 * Display a MORE prompt, wait for a keypress and remove the MORE
 * prompt from the screen.
 *
 */
extern "C" void os_more_prompt(void)
{
  flushOutput();
}

/*
 * os_process_arguments
 *
 * Handle command line switches. Some variables may be set to activate
 * special features of Frotz:
 *
 *     option_attribute_assignment
 *     option_attribute_testing
 *     option_context_lines
 *     option_object_locating
 *     option_object_movement
 *     option_left_margin
 *     option_right_margin
 *     option_ignore_errors
 *     option_piracy
 *     option_undo_slots
 *     option_expand_abbreviations
 *     option_script_cols
 *
 * The global pointer "story_name" is set to the story file name.
 *
 */
extern "C" void os_process_arguments(int argc, char *argv[])
{
  if (argc > 5)
  {
    story_name = argv[1];
    display_width = atoi(argv[2]);
    display_height = atoi(argv[3]);
    char_width = atoi(argv[4]);
    char_height = atoi(argv[5]);
  }
}

/*
 * os_read_line
 *
 * Read a line of input from the keyboard into a buffer. The buffer
 * may already be primed with some text. In this case, the "initial"
 * text is already displayed on the screen. After the input action
 * is complete, the function returns with the terminating key value.
 * The length of the input should not exceed "max" characters plus
 * an extra 0 terminator.
 *
 * Terminating keys are the return key (13) and all function keys
 * (see the Specification of the Z-machine) which are accepted by
 * the is_terminator function. Mouse clicks behave like function
 * keys except that the mouse position is stored in global variables
 * "mouse_x" and "mouse_y" (top left coordinates are (1,1)).
 *
 * Furthermore, Frotz introduces some special terminating keys:
 *
 *     ZC_HKEY_PLAYBACK (Alt-P)
 *     ZC_HKEY_RECORD (Alt-R)
 *     ZC_HKEY_SEED (Alt-S)
 *     ZC_HKEY_UNDO (Alt-U)
 *     ZC_HKEY_RESTART (Alt-N, "new game")
 *     ZC_HKEY_QUIT (Alt-X, "exit game")
 *     ZC_HKEY_DEBUG (Alt-D)
 *     ZC_HKEY_HELP (Alt-H)
 *
 * If the timeout argument is not zero, the input gets interrupted
 * after timeout/10 seconds (and the return value is 0).
 *
 * The complete input line including the cursor must fit in "width"
 * screen units.
 *
 * The function may be called once again to continue after timeouts,
 * misplaced mouse clicks or hot keys. In this case the "continued"
 * flag will be set. This information can be useful if the interface
 * implements input line history.
 *
 * The screen is not scrolled after the return key was pressed. The
 * cursor is at the end of the input line when the function returns.
 *
 * Since Frotz 2.2 the helper function "completion" can be called
 * to implement word completion (similar to tcsh under Unix).
 *
 */
extern "C" zword os_read_line(int max, zword *buf, int timeout, int width, int continued)
{
  flushOutput();

  // Line input is only supported in window 0
  if (get_current_window() != 0)
  {
    *buf = 0;
    return 13;
  }

  // Tell the UI to read in a line
  int data[2];
  data[0] = 0;
  data[1] = wcslen(buf);
  sendCommand(Command_ReadLine,sizeof data,data);

  // Get the length of the input line
  int length = readReturnCommand(Return_ReadLine);
  if (length >= sizeof (int))
  {
    int wndId;
    readReturnData(&wndId,sizeof (int));
    length -= sizeof (int);
  }
  else
    length = 0;

  // Is there any input line?
  if (length == 0)
  {
    *buf = 0;
    updateHeight(false);
    return 13;
  }

  // Get the input command line
  wchar_t* line = (wchar_t*)alloca(length);
  readReturnData(line,length);

  int i = 0;
  while ((i < max) && (i < length/sizeof wchar_t))
  {
    buf[i] = line[i];
    i++;
  }
  buf[i] = 0;

  updateHeight(false);
  return 13;
}

/*
 * os_read_key
 *
 * Read a single character from the keyboard (or a mouse click) and
 * return it. Input aborts after timeout/10 seconds.
 *
 */
extern "C" zword os_read_key(int timeout, int cursor)
{
  flushOutput();

  // Tell the UI to read a key
  sendCommand(Command_ReadKey,0,NULL);

  // Get the returned key
  int key = 0;
  readReturnCommand(Return_ReadKey);
  readReturnData(&key,sizeof key);

  // Translate any special keys
  switch (key)
  {
  case Key_Left:
    key = ZC_ARROW_LEFT;
    break;
  case Key_Right:
    key = ZC_ARROW_RIGHT;
    break;
  case Key_Up:
    key = ZC_ARROW_UP;
    break;
  case Key_Down:
    key = ZC_ARROW_DOWN;
    break;
  }

  updateHeight(false);
  return (zword)key;
}

/*
 * os_read_mouse
 *
 * Store the mouse position in the global variables "mouse_x" and
 * "mouse_y", the code of the last clicked menu in "menu_selected"
 * and return the mouse buttons currently pressed.
 *
 */
zword os_read_mouse(void)
{
  return 0;
}

/*
 * os_menu
 *
 * Add to or remove a menu item. Action can be:
 *     MENU_NEW    - Add a new menu with the given title
 *     MENU_ADD    - Add a new menu item with the given text
 *     MENU_REMOVE - Remove the menu at the given index
 *
 */
void os_menu(int action, int menu, const zword * text)
{
}

/*
 * os_reset_screen
 *
 * Reset the screen before the program ends.
 *
 */
extern "C" void os_reset_screen(void)
{
  flushOutput();
}

/*
 * os_scroll_area
 *
 * Scroll a rectangular area of the screen up (units > 0) or down
 * (units < 0) and fill the empty space with the current background
 * colour. Top left coordinates are (1,1). The cursor stays put.
 *
 */
extern "C" void os_scroll_area(int top, int left, int bottom, int right, int units)
{
}

/*
 * os_set_colour
 *
 * Set the foreground and background colours which can be:
 *
 *     1
 *     BLACK_COLOUR
 *     RED_COLOUR
 *     GREEN_COLOUR
 *     YELLOW_COLOUR
 *     BLUE_COLOUR
 *     MAGENTA_COLOUR
 *     CYAN_COLOUR
 *     WHITE_COLOUR
 *     TRANSPARENT_COLOUR
 *
 *     Amiga only:
 *
 *     LIGHTGREY_COLOUR
 *     MEDIUMGREY_COLOUR
 *     DARKGREY_COLOUR
 *
 * There may be more colours in the range from 16 to 255; see the
 * remarks about os_peek_colour.
 *
 */
extern "C" void os_set_colour(int new_foreground, int new_background)
{
  flushOutput();

  if (new_foreground == 1)
    new_foreground = BLACK_COLOUR;
  if (new_background == 1)
    new_background = WHITE_COLOUR;

  int data[7];
  data[0] = get_current_window();

  int r,g,b;
  rgb5ToTrue(getColour(new_foreground),r,g,b);
  data[1] = r;
  data[2] = g;
  data[3] = b;

  rgb5ToTrue(getColour(new_background),r,g,b);
  data[4] = r;
  data[5] = g;
  data[6] = b;

  sendCommand(Command_SetColour,sizeof data,data);
}

/*
 * os_from_true_cursor
 *
 * Given a true colour, return an appropriate colour index.
 *
 */
extern "C" int os_from_true_colour(zword colour)
{
  return getColourIndex(colour);
}

/*
 * os_to_true_cursor
 *
 * Given a colour index, return the appropriate true colour.
 *
 */
extern "C" zword os_to_true_colour(int index)
{
  return getColour(index);
}

/*
 * os_set_cursor
 *
 * Place the text cursor at the given coordinates. Top left is (1,1).
 *
 */
extern "C" void os_set_cursor(int row, int col)
{
  flushOutput();

  // Don't send cursor commands for the main window
  if (get_current_window() == 0)
    return;

  int data[3];
  data[0] = get_current_window();
  data[1] = (int)((col-1)/char_width);
  data[2] = (int)((row-1)/char_height);
  sendCommand(Command_SetCursor,sizeof data,data);
}

/*
 * os_set_font
 *
 * Set the font for text output. The interpreter takes care not to
 * choose fonts which aren't supported by the interface.
 *
 */
extern "C" void os_set_font(int new_font)
{
  flushOutput();

  if (get_current_window() == 0)
  {
    win0_font = new_font;
    sendStyle(0);
  }
}

/*
 * os_set_text_style
 *
 * Set the current text style. Following flags can be set:
 *
 *     REVERSE_STYLE
 *     BOLDFACE_STYLE
 *     EMPHASIS_STYLE (aka underline aka italics)
 *     FIXED_WIDTH_STYLE
 *
 */
extern "C" void os_set_text_style(int new_style)
{
  flushOutput();

  switch (get_current_window())
  {
  case 0:
    win0_style = new_style;
    sendStyle(0);
    break;
  case 1:
    win1_style = new_style;
    sendStyle(1);
    break;
  }
}

/*
 * os_string_width
 *
 * Calculate the length of a word in screen units. Apart from letters,
 * the word may contain special codes:
 *
 *    ZC_NEW_STYLE - next character is a new text style
 *    ZC_NEW_FONT  - next character is a new font
 *
 */
extern "C" int os_string_width(const zword *s)
{
  int width = 0;
  for (const zword* s1 = s; *s1 != 0; s1++)
  {
    switch (*s1)
    {
    case ZC_NEW_STYLE:
      s1++;
      break;
    case ZC_NEW_FONT:
      s1++;
      break;
    case ZC_INDENT:
      width += 3;
      break;
    case ZC_GAP:
      width += 2;
      break;
    default:
      width++;
      break;
    }
  }
  return width;
}

/*
 * os_char_width
 *
 * Return the length of the character in screen units.
 *
 */
extern "C" int os_char_width(zword c)
{
  return 1;
}

/*
 * os_check_unicode
 *
 * Return with bit 0 set if the Unicode character can be
 * displayed, and bit 1 if it can be input.
 * 
 *
 */
extern "C" int os_check_unicode(int font, zword c)
{
  return isValidChar(c) ? 3 : 0;
}

/*
 * os_peek_colour
 *
 * Return the colour of the screen unit below the cursor. (If the
 * interface uses a text mode, it may return the background colour
 * of the character at the cursor position instead.) This is used
 * when text is printed on top of pictures. Note that this coulor
 * need not be in the standard set of Z-machine colours. To handle
 * this situation, Frotz entends the colour scheme: Colours above
 * 15 (and below 256) may be used by the interface to refer to non
 * standard colours. Of course, os_set_colour must be able to deal
 * with these colours.
 *
 */
extern "C" int os_peek_colour(void)
{
  return 0;
}

/*
 * os_picture_data
 *
 * Return true if the given picture is available. If so, store the
 * picture width and height in the appropriate variables. Picture
 * number 0 is a special case: Write the highest legal picture number
 * and the picture file release number into the height and width
 * variables respectively when this picture number is asked for.
 *
 */
extern "C" int os_picture_data(int picture, int *height, int *width)
{
  *height = 0;
  *width = 0;
  return 0;
}

/*
 * os_draw_picture
 *
 * Display a picture at the given coordinates.
 *
 */
extern "C" void os_draw_picture(int picture, int y, int x)
{
}

/*
 * os_random_seed
 *
 * Return an appropriate random seed value in the range from 0 to
 * 32767, possibly by using the current system time.
 *
 */
extern "C" int os_random_seed(void)
{
  return ::GetTickCount() & 32767;
}

/*
 * os_restart_game
 *
 * This routine allows the interface to interfere with the process of
 * restarting a game at various stages:
 *
 *     RESTART_BEGIN - restart has just begun
 *     RESTART_WPROP_SET - window properties have been initialised
 *     RESTART_END - restart is complete
 *
 */
extern "C" void os_restart_game(int stage)
{
}

/*
 * os_path_open
 *
 * Open a file in the current directory.
 *
 */
extern "C" FILE *os_path_open(const char *name, const char *mode, long *size)
{
  FILE* file = fopen(name,mode);
  if (file != NULL)
  {
    if (bb_create_map(file,&blorbMap) == bb_err_None)
    {
      bb_result_t result;
      if (bb_load_resource(blorbMap,bb_method_FilePos,&result,bb_ID_Exec,0) == bb_err_None)
      {
        unsigned int id = blorbMap->chunks[result.chunknum].type;
        if (id == bb_make_id('Z','C','O','D'))
        {
          fseek(file,result.data.startpos,SEEK_SET);
          *size = result.length;
          return file;
        }
      }
    }
    fseek(file,0,SEEK_SET);
    *size = -1;
  }
  return file;
}

/*
 * os_finish_with_sample
 *
 * Remove the current sample from memory (if any).
 *
 */
extern "C" void os_finish_with_sample(int number)
{
}

/*
 * os_prepare_sample
 *
 * Load the given sample from the disk.
 *
 */
extern "C" void os_prepare_sample(int number)
{
}

/*
 * os_start_sample
 *
 * Play the given sample at the given volume (ranging from 1 to 8 and
 * 255 meaning a default volume). The sound is played once or several
 * times in the background (255 meaning forever). The end_of_sound
 * function is called as soon as the sound finishes, passing in the
 * eos argument.
 *
 */
extern "C" void os_start_sample(int number, int volume, int repeats, zword eos)
{
  if (volume == 255)
    volume = 0x10000;
  else
    volume = volume * 0x2000;

  if (repeats == 0)
    repeats = 1;
  else if (repeats == 255)
    repeats = -1;

  int data[4];
  data[0] = 0;
  data[1] = number;
  data[2] = repeats;
  data[3] = volume;
  sendCommand(Command_PlaySound,sizeof data,data);
}

/*
 * os_stop_sample
 *
 * Turn off the current sample.
 *
 */
extern "C" void os_stop_sample(int number)
{
  int data[1];
  data[0] = 0;
  sendCommand(Command_StopSound,sizeof data,data);
}

/*
 * os_scrollback_char
 *
 * Write a character to the scrollback buffer.
 *
 */
extern "C" void  os_scrollback_char(zword c)
{
}

/*
 * os_scrollback_erase
 *
 * Remove characters from the scrollback buffer.
 *
 */
extern "C" void os_scrollback_erase (int erase)
{
}

/*
 * os_tick
 *
 * Called after each opcode.
 *
 */
extern "C" void os_tick (void)
{
}

/*
 * os_buffer_screen
 *
 * Set the screen buffering mode, and return the previous mode.
 * Possible values for mode are:
 *
 *     0 - update the display to reflect changes when possible
 *     1 - do not update the display
 *    -1 - redraw the screen, do not change the mode
 *
 */
extern "C" int os_buffer_screen (int mode)
{
  return 0;
}

/*
 * os_wrap_window
 *
 * Return non-zero if the window should have text wrapped.
 *
 */
extern "C" int os_wrap_window (int win)
{
  return (win == 0) ? 0 : 1;
}

/*
 * os_window_height
 *
 * Called when the height of a window is changed.
 *
 */
extern "C" void os_window_height (int win, int height)
{
  flushOutput();

  if (win == 1)
  {
    if (height > sent_height)
    {
      int data[5];
      data[0] = 2;
      data[1] = Method_Above|Method_Fixed;
      data[2] = (int)(height/char_height);
      data[3] = 1;
      data[4] = 0;
      sendCommand(Command_ArrangeWindow,sizeof data,data);
      sent_height = height;
    }
    real_height = height;
  }
}
