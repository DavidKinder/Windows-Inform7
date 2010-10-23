#include <cstdio>
#include <atlbase.h>
#include "png.h"

//#define VERBOSE
void msg(const char* text)
{
#ifdef VERBOSE
  ::MessageBox(0,text,"Install I7",MB_OK);
#endif
}

bool checkIEVersion(double required)
{
  CRegKey ieKey;
  if (ieKey.Open(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Internet Explorer",KEY_READ) == ERROR_SUCCESS)
  {
    char version[256];
    ULONG len = sizeof version;
    if (ieKey.QueryStringValue("Version",version,&len) == ERROR_SUCCESS)
    {
      if (atof(version) >= required)
        return true;
    }
  }
  return false;
}

void removeAlpha(const char* dir, const char* file, int back = 255)
{
  char path[_MAX_PATH];
  strcpy(path,dir);
  strcat(path,file);
  strcat(path,".png");

  FILE* in = fopen(path,"rb");
  if (in == NULL)
  {
    msg("Cannot open input file");
    return;
  }
  png_byte fileHeader[8];
  fread(fileHeader,1,8,in);
  if (png_sig_cmp(fileHeader,0,8))
  {
    msg("No valid PNG header");
    return;
  }

  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING,(png_voidp)NULL,NULL,NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_infop end_info = png_create_info_struct(png_ptr);

  png_bytep* rowPointers = NULL;
  if (setjmp(png_ptr->jmpbuf))
    return;

  png_init_io(png_ptr,in);
  png_set_sig_bytes(png_ptr,8);
  png_read_info(png_ptr,info_ptr);

  png_uint_32 width = png_get_image_width(png_ptr,info_ptr);
  png_uint_32 height = png_get_image_height(png_ptr,info_ptr);
  int bit_depth = png_get_bit_depth(png_ptr,info_ptr);
  int color_type = png_get_color_type(png_ptr,info_ptr);
  int interlace_type, compression_type, filter_type;
  png_get_IHDR(png_ptr,info_ptr,&width,&height,&bit_depth,&color_type,
    &interlace_type,&compression_type,&filter_type);

  switch (color_type)
  {
  case PNG_COLOR_TYPE_RGB:
  case PNG_COLOR_TYPE_RGB_ALPHA:
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    break;
  default:
    msg("Unknown colour type");
    return;
  }

  if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);
  if (bit_depth == 16)
    png_set_strip_16(png_ptr);
  if (bit_depth < 8)
    png_set_packing(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);
  png_set_filler(png_ptr,0xFF,PNG_FILLER_AFTER);

  png_bytep bitmap = new png_byte[width*height*4];
  png_bytep* rows = new png_bytep[height];
  for (int i = 0; i < (int)height; i++)
    rows[i] = bitmap+(width*i*4);

  png_read_image(png_ptr,rows);
  png_read_end(png_ptr,end_info);
  png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
  fclose(in);

  int r = back;
  int g = back;
  int b = back;
  for (int y = 0; y < (int)height; y++)
  {
    for (int x = 0; x < (int)width; x++)
    {
      png_bytep pixel = bitmap+(width*y*4)+(x*4);
      int sr = pixel[0];
      int sg = pixel[1];
      int sb = pixel[2];
      int a =  pixel[3];

      int dr = r;
      int dg = g;
      int db = b;
      if (a == 0)
      {
      }
      else if (a == 255)
      {
        dr = sr;
        dg = sg;
        db = sb;
      }
      else
      {
        a += a>>7;
        dr += (a * (sr - dr) >> 8);
        dg += (a * (sg - dg) >> 8);
        db += (a * (sb - db) >> 8);
      }
      pixel[0] = dr;
      pixel[1] = dg;
      pixel[2] = db;
      pixel[3] = 255;
    }
  }

  FILE* out = fopen(path,"wb");
  if (out == NULL)
  {
    msg("Cannot open output file");
    return;
  }

  png_structp wpng_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING,(png_voidp)NULL,NULL,NULL);
  png_infop winfo_ptr = png_create_info_struct(wpng_ptr);
  png_init_io(wpng_ptr,out);

  png_set_IHDR(wpng_ptr,winfo_ptr,width,height,8,
    PNG_COLOR_TYPE_RGB_ALPHA,PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);

  png_set_rows(wpng_ptr,winfo_ptr,rows);
  png_write_png(wpng_ptr,winfo_ptr,PNG_TRANSFORM_IDENTITY,NULL);

  png_destroy_write_struct(&wpng_ptr,&winfo_ptr);
  fclose(out);
}

extern "C" __declspec(dllexport)
void ImageAlpha(HWND, int stringSize, char* variables, void**, void*)
{
  if (checkIEVersion(7.0))
  {
    msg("IE7+");
    return;
  }

  char* instDir = variables+(21*stringSize);
  msg(instDir);

  char imagesDir[_MAX_PATH];
  strcpy(imagesDir,instDir);
  if (imagesDir[strlen(imagesDir)-1] != '\\')
    strcat(imagesDir,"\\");
  strcat(imagesDir,"Documentation\\");
  msg(imagesDir);

  removeAlpha(imagesDir,"Beneath");
  removeAlpha(imagesDir,"Revealext");

  strcat(imagesDir,"doc_images\\");

  removeAlpha(imagesDir,"Debian_Icon");
  removeAlpha(imagesDir,"Fedora_Icon");
  removeAlpha(imagesDir,"Linux_Icon");
  removeAlpha(imagesDir,"MacOSX_Icon");
  removeAlpha(imagesDir,"Solaris_Icon");
  removeAlpha(imagesDir,"Ubuntu_Icon");
  removeAlpha(imagesDir,"Windows_Icon");

  removeAlpha(imagesDir,"Hookclose",0);
  removeAlpha(imagesDir,"Hookindex",0);
  removeAlpha(imagesDir,"Hookleft",0);
  removeAlpha(imagesDir,"Hookrecipe",0);
  removeAlpha(imagesDir,"Hookright",0);
  removeAlpha(imagesDir,"Hookup",0);

  removeAlpha(imagesDir,"FlipBlank");
  removeAlpha(imagesDir,"FlipBlankHalf");
  removeAlpha(imagesDir,"FlipClosed");
  removeAlpha(imagesDir,"FlipOpen");

  removeAlpha(imagesDir,"ruleequal");
  removeAlpha(imagesDir,"ruleless");
  removeAlpha(imagesDir,"rulemore");
  removeAlpha(imagesDir,"rulenone");

  removeAlpha(imagesDir,"vm_glulx");
  removeAlpha(imagesDir,"vm_z");
  removeAlpha(imagesDir,"vm_z5");
  removeAlpha(imagesDir,"vm_z6");
  removeAlpha(imagesDir,"vm_z8");

  removeAlpha(imagesDir,"asterisk");
  removeAlpha(imagesDir,"Below");
  removeAlpha(imagesDir,"beta");
  removeAlpha(imagesDir,"beta2");
  removeAlpha(imagesDir,"census_problem");
  removeAlpha(imagesDir,"cross");
  removeAlpha(imagesDir,"diamond");
  removeAlpha(imagesDir,"greycross");
  removeAlpha(imagesDir,"help");
  removeAlpha(imagesDir,"image_problem");
  removeAlpha(imagesDir,"linux");
  removeAlpha(imagesDir,"override_ext");
  removeAlpha(imagesDir,"ovoid");
  removeAlpha(imagesDir,"ovoid2");
  removeAlpha(imagesDir,"rbarrow");
  removeAlpha(imagesDir,"Reveal");
  removeAlpha(imagesDir,"Revealext");
  removeAlpha(imagesDir,"shelp");
  removeAlpha(imagesDir,"sound_okay");
  removeAlpha(imagesDir,"testing");
  removeAlpha(imagesDir,"wiarrow");
}
