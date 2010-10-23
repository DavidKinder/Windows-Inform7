#pragma once

enum TerpCommand
{
  Command_FatalError,
  Command_CreateWindow,
  Command_DestroyWindow,
  Command_ArrangeWindow,
  Command_PrintOutput,
  Command_SetStyle,
  Command_SetColour,
  Command_SetCursor,
  Command_ReadLine,
  Command_ReadKey,
  Command_Clear,
  Command_Draw,
  Command_PlaySound,
  Command_StopSound,
  Command_SetVolume,
  Command_FillRect,
  Command_BackColour,
  Command_SetLink,
  Command_SetParagraph,
  Command_CancelLine,
  Command_CancelKey,
};

enum TerpReturn
{
  Return_ReadLine,
  Return_ReadKey,
  Return_Size,
  Return_SoundOver,
  Return_Mouse,
  Return_Link
};

enum TerpWindow
{
  Window_Pair = 1,
  Window_Blank = 2,
  Window_Text = 3,
  Window_Grid = 4,
  Window_Graphics = 5,
};

enum TerpTextStyle
{
  StyleNormal = 0,
  StyleReverse = 1,
  StyleBold = 2,
  StyleItalic = 4,
  StyleFixed = 8
};

enum TerpJustify
{
  Just_LeftFlush = 0,
  Just_LeftRight = 1,
  Just_Centred = 2,
  Just_RightFlush = 3,
};

#define Method_Left   0
#define Method_Right  1
#define Method_Above  2
#define Method_Below  3
#define Method_Fixed 16
#define Method_Prop  32
#define Method_DivMask 0xF0
#define Method_DirMask 0x0F

#define Key_Left  (0xfffe)
#define Key_Right (0xfffd)
#define Key_Up    (0xfffc)
#define Key_Down  (0xfffb)
