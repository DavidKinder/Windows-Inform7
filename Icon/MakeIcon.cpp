#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <io.h>
#include <windows.h>

#include "png.h"

FILE* out = 0;

struct BitmapInfo
{
  const char* name;
  int mode;
  unsigned char* data;
  int headerPos;
  int len;
};

struct BitmapInfo bitmaps[] =
{
  "bitmaps/48x48x4.bmp",0,0,0,0,
  "bitmaps/32x32x4.bmp",0,0,0,0,
  "bitmaps/16x16x4.bmp",0,0,0,0,

  "bitmaps/48x48x8.bmp",1,0,0,0,
  "bitmaps/32x32x8.bmp",1,0,0,0,
  "bitmaps/16x16x8.bmp",1,0,0,0,

  "bitmaps/48x48.png",3,0,0,0,
  "bitmaps/32x32.png",3,0,0,0,
  "bitmaps/16x16.png",3,0,0,0,

  "bitmaps/256x256.png",4,0,0,0,
};

void fatal(void)
{
  exit(1);
}

void writeShort(unsigned short v)
{
  fwrite(&v,2,1,out);
}

void writeLong(unsigned long v)
{
  fwrite(&v,4,1,out);
}

struct PNGData
{
  BYTE* gfxData;
  ULONG offset;
};

void readPNGData(png_structp png_ptr, png_bytep data, png_size_t length)
{
  PNGData* pngData = (PNGData*)png_get_io_ptr(png_ptr);
  memcpy(data,pngData->gfxData+pngData->offset,length);
  pngData->offset += length;
}

void loadPNG(struct BitmapInfo* info)
{
  png_bytep* rowPointers = NULL;

  if (!png_check_sig(info->data,8))
    fatal();
  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING,(png_voidp)NULL,NULL,NULL);
  if (!png_ptr)
    fatal();
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    fatal();
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
    fatal();
  if (setjmp(png_jmpbuf(png_ptr)))
    fatal();

  PNGData pngData;
  pngData.gfxData = info->data;
  pngData.offset = 8;
  png_set_read_fn(png_ptr,&pngData,readPNGData);

  png_set_sig_bytes(png_ptr,8);
  png_read_info(png_ptr,info_ptr);

  png_uint_32 width = png_get_image_width(png_ptr,info_ptr);
  png_uint_32 height = png_get_image_height(png_ptr,info_ptr);
  int bit_depth = png_get_bit_depth(png_ptr,info_ptr);
  int color_type = png_get_color_type(png_ptr,info_ptr);

  if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
    png_set_palette_to_rgb(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);

  if (bit_depth == 16)
    png_set_strip_16(png_ptr);
  if (bit_depth < 8)
    png_set_packing(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

  png_set_bgr(png_ptr);
  png_set_filler(png_ptr,0xFF,PNG_FILLER_AFTER);

  unsigned char* newData = (unsigned char*)malloc(1024*1024);
  BITMAPINFOHEADER* head = (BITMAPINFOHEADER*)newData;
  memset(head,0,sizeof *head);
  head->biSize = sizeof *head;
  head->biWidth = width;
  head->biHeight = height;
  head->biPlanes = 1;
  head->biBitCount = 32;

  unsigned char* pixels = newData+(sizeof *head);
  rowPointers = new png_bytep[height];
  for (int i = 0; i < (int)height; i++)
    rowPointers[height-i-1] = pixels+(width*i*4);
  png_read_image(png_ptr,rowPointers);

  png_read_end(png_ptr,end_info);
  png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
  if (rowPointers)
    delete[] rowPointers;

  free(info->data);
  info->data = newData;
}

void writeIconHeader(struct BitmapInfo* info)
{
  FILE* in = fopen(info->name,"rb");
  if (in == 0)
    return;
    
  info->data = (unsigned char*)malloc(256*1024);
  info->len = _filelength(_fileno(in));
  fread(info->data,1,info->len,in);
  fclose(in);

  if (info->mode != 4)
  {
    if ((info->data[0] == 'B') && (info->data[1] == 'M'))
      info->data += 14;
    else if ((info->data[1] == 'P') && (info->data[2] == 'N') && (info->data[3] == 'G'))
      loadPNG(info);
    else
      fatal();

    BITMAPINFOHEADER* head = (BITMAPINFOHEADER*)(info->data);
    if ((head->biSize != sizeof(BITMAPINFOHEADER)) || (head->biCompression != 0))
      fatal();

    fputc(head->biWidth,out);
    fputc(head->biHeight,out);
  }
  else
  {
    fputc(0,out);
    fputc(0,out);
  }

  fputc(0,out); // color count
  fputc(0,out);
  writeShort(0);

  // Bits per pixel
  switch (info->mode)
  {
  case 0:
    writeShort(4);
    break;
  case 1:
    writeShort(8);
    break;
  case 2:
  case 3:
  case 4:
    writeShort(32);
    break;
  default:
    fatal();
    break;
  }

  info->headerPos = ftell(out);
  writeLong(0); // size
  writeLong(0); // offset
}

int match(RGBQUAD* colours, int index, int max)
{
  int best = 0;
  double bestDiff = 0.0;
  RGBQUAD find = colours[index];
  for (int i = 0; i < max; i++)
  {
    RGBQUAD test = colours[i];
    double diff = pow(abs(find.rgbBlue-test.rgbBlue),2.0);
    diff += pow(abs(find.rgbGreen-test.rgbGreen),2.0);
    diff += pow(abs(find.rgbRed-test.rgbRed),2.0);

    if ((i == 0) || (diff < bestDiff))
    {
      best = i;
      bestDiff = diff;
    }
  }
  return best;
}

void writeIconData4(struct BitmapInfo* info, bool opaque)
{
  BITMAPINFOHEADER* head = (BITMAPINFOHEADER*)(info->data);
  if (head->biBitCount != 8)
    fatal();

  BITMAPINFOHEADER outHead;
  memset(&outHead,0,sizeof outHead);
  outHead.biSize = sizeof outHead;
  outHead.biWidth = head->biWidth;
  outHead.biHeight = 2*head->biHeight; // 2 bitmaps
  outHead.biPlanes = 1;
  outHead.biBitCount = 4;
  outHead.biSizeImage = (outHead.biWidth*head->biHeight)/2;
  outHead.biClrUsed = 16;
  outHead.biClrImportant = 16;
  fwrite(&outHead,sizeof outHead,1,out);

  unsigned char* colours = info->data+sizeof(BITMAPINFOHEADER);
  unsigned char* data = colours+(256*4);

  int backcol = data[0];
  if (backcol >= 16)
    fatal();
  if (!opaque)
  {
    for (int i = 0; i < 4; i++)
      colours[(backcol*4)+i] = 0;
  }

  for (int i = 0; i < 16*4; i++)
    fputc(colours[i],out);

  int datasz = (outHead.biWidth*head->biHeight)/2;
  for (int i = 0; i < datasz; i++)
  {
    int d0 = data[(i*2)+0];
    int d1 = data[(i*2)+1];
    if (d0 >= 16)
      d0 = match((RGBQUAD*)colours,d0,16);
    if (d1 >= 16)
      d1 = match((RGBQUAD*)colours,d1,16);
    fputc((d0<<4)|d1,out);
  }

  for (int h = 0; h < head->biHeight; h++)
  {
    int len = outHead.biWidth/8;
    for (int w = 0; w < len; w++)
    {
      int mask = 0;
      for (int x = 0; x < 8; x++)
      {
        mask <<= 1;
        if (!opaque && (data[(h*outHead.biWidth)+(w*8)+x] == backcol))
          mask |= 1;
      }
      fputc(mask,out);
    }
    if ((len % 4) > 0)
    {
      for (int x = 0; x < 4-(len % 4); x++)
        fputc(0,out);
    }
  }
}

void writeIconData8(struct BitmapInfo* info, bool opaque)
{
  BITMAPINFOHEADER* head = (BITMAPINFOHEADER*)(info->data);
  if (head->biBitCount != 8)
    fatal();

  BITMAPINFOHEADER outHead;
  memset(&outHead,0,sizeof outHead);
  outHead.biSize = sizeof outHead;
  outHead.biWidth = head->biWidth;
  outHead.biHeight = 2*head->biHeight; // 2 bitmaps
  outHead.biPlanes = 1;
  outHead.biBitCount = head->biBitCount;
  outHead.biSizeImage = (outHead.biWidth*head->biHeight);
  outHead.biClrUsed = 256;
  outHead.biClrImportant = 256;
  fwrite(&outHead,sizeof outHead,1,out);

  unsigned char* colours = info->data+sizeof(BITMAPINFOHEADER);
  unsigned char* data = colours+(256*4);

  int backcol = data[0];
  if (!opaque)
  {
    for (int i = 0; i < 4; i++)
      colours[(backcol*4)+i] = 0;
  }

  for (int i = 0; i < 256*4; i++)
    fputc(colours[i],out);

  int datasz = outHead.biWidth*head->biHeight;
  for (int i = 0; i < datasz; i++)
    fputc(data[i],out);

  for (int h = 0; h < head->biHeight; h++)
  {
    int len = outHead.biWidth/8;
    for (int w = 0; w < len; w++)
    {
      int mask = 0;
      for (int x = 0; x < 8; x++)
      {
        mask <<= 1;
        if (!opaque && (data[(h*outHead.biWidth)+(w*8)+x] == backcol))
          mask |= 1;
      }
      fputc(mask,out);
    }
    if ((len % 4) > 0)
    {
      for (int x = 0; x < 4-(len % 4); x++)
        fputc(0,out);
    }
  }
}

void writeIconData24(struct BitmapInfo* info, bool opaque)
{
  BITMAPINFOHEADER* head = (BITMAPINFOHEADER*)(info->data);
  if (head->biBitCount != 8)
    fatal();

  BITMAPINFOHEADER outHead;
  memset(&outHead,0,sizeof outHead);
  outHead.biSize = sizeof outHead;
  outHead.biWidth = head->biWidth;
  outHead.biHeight = 2*head->biHeight; // 2 bitmaps
  outHead.biPlanes = 1;
  outHead.biBitCount = 32;
  outHead.biSizeImage = (outHead.biWidth*head->biHeight);
  fwrite(&outHead,sizeof outHead,1,out);

  unsigned char* colours = info->data+sizeof(BITMAPINFOHEADER);
  unsigned char* data = colours+(256*4);

  int backcol = data[0];
  if (!opaque)
  {
    for (int i = 0; i < 4; i++)
      colours[(backcol*4)+i] = 0;
  }

  int datasz = outHead.biWidth*head->biHeight;
  for (int i = 0; i < datasz; i++)
  {
    unsigned char* colour = colours+(data[i]*4);
    fputc(colour[0],out);
    fputc(colour[1],out);
    fputc(colour[2],out);
    if (!opaque && (data[i] == backcol))
      fputc(0,out);
    else
      fputc(255,out);
  }

  for (int h = 0; h < head->biHeight; h++)
  {
    int len = outHead.biWidth/8;
    for (int w = 0; w < len; w++)
    {
      int mask = 0;
      for (int x = 0; x < 8; x++)
      {
        mask <<= 1;
        if (!opaque && (data[(h*outHead.biWidth)+(w*8)+x] == backcol))
          mask |= 1;
      }
      fputc(mask,out);
    }
    if ((len % 4) > 0)
    {
      for (int x = 0; x < 4-(len % 4); x++)
        fputc(0,out);
    }
  }
}

void writeIconData32(struct BitmapInfo* info, bool opaque)
{
  BITMAPINFOHEADER* head = (BITMAPINFOHEADER*)(info->data);
  if (head->biBitCount != 32)
    fatal();

  BITMAPINFOHEADER outHead;
  memset(&outHead,0,sizeof outHead);
  outHead.biSize = sizeof outHead;
  outHead.biWidth = head->biWidth;
  outHead.biHeight = 2*head->biHeight; // 2 bitmaps
  outHead.biPlanes = 1;
  outHead.biBitCount = 32;
  outHead.biSizeImage = (outHead.biWidth*head->biHeight);
  fwrite(&outHead,sizeof outHead,1,out);

  unsigned char* data = info->data+sizeof(BITMAPINFOHEADER);
  DWORD backcol = *((DWORD*)data);

  int datasz = outHead.biWidth*head->biHeight;
  for (int i = 0; i < datasz; i++)
  {
    if (!opaque && (*(DWORD*)(data+(i*4)) == backcol))
    {
      fputc(0,out);
      fputc(0,out);
      fputc(0,out);
      fputc(0,out);
    }
    else
    {
      fputc(data[(i*4)+0],out);
      fputc(data[(i*4)+1],out);
      fputc(data[(i*4)+2],out);
      fputc(data[(i*4)+3],out);
    }
  }

  for (int h = 0; h < head->biHeight; h++)
  {
    int len = outHead.biWidth/8;
    for (int w = 0; w < len; w++)
    {
      int mask = 0;
      for (int x = 0; x < 8; x++)
      {
        mask <<= 1;
        if (!opaque && (*(DWORD*)(data+4*((h*outHead.biWidth)+(w*8)+x)) == backcol))
          mask |= 1;
      }
      fputc(mask,out);
    }
    if ((len % 4) > 0)
    {
      for (int x = 0; x < 4-(len % 4); x++)
        fputc(0,out);
    }
  }
}

void writeIconDataPNG(struct BitmapInfo* info)
{
  for (int i = 0; i < info->len; i++)
    fputc(info->data[i],out);
}

void writeIconData(struct BitmapInfo* info, bool opaque)
{
  if (info->data == NULL)
    return;

  int pos1 = ftell(out);

  switch (info->mode)
  {
  case 0:
    writeIconData4(info,opaque);
    break;
  case 1:
    writeIconData8(info,opaque);
    break;
  case 2:
    writeIconData24(info,opaque);
    break;
  case 3:
    writeIconData32(info,opaque);
    break;
  case 4:
    writeIconDataPNG(info);
    break;
  default:
    fatal();
    break;
  }

  int pos2 = ftell(out);
  fseek(out,info->headerPos,SEEK_SET);
  writeLong((pos2-pos1)); // size
  writeLong(pos1); // offset
  fseek(out,0,SEEK_END);
}

int getIconCount(void)
{
  int len = 0;
  int maxLen = sizeof bitmaps / sizeof bitmaps[0];
  for (int i = 0; i < maxLen; i++)
  {
    FILE* in = fopen(bitmaps[i].name,"rb");
    if (in != 0)
    {
      fclose(in);
      len++;
    }
  }
  return len;
}

int main(int argc, char** argv)
{
  bool opaque = false;
  if (argc > 1)
  {
    if (strcmp(argv[1],"opaque") == 0)
      opaque = true;
  }

  out = fopen("output.ico","wb");
  if (out == 0)
    fatal();

  writeShort(0);
  writeShort(1);
  writeShort(getIconCount());

  int maxLen = sizeof bitmaps / sizeof bitmaps[0];
  for (int i = 0; i < maxLen; i++)
    writeIconHeader(bitmaps+i);
  for (int i = 0; i < maxLen; i++)
    writeIconData(bitmaps+i,opaque);

  printf("Success!");
  return 0;
}
