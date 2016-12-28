#ifndef _RESIZE_PNG_H_
#define _RESIZE_PNG_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <png.h>
#include "resize_base.h"


typedef struct
{
	uint32_t width, height;
	png_byte color_type;
	png_byte bit_depth;
	png_byte channels;
	int number_of_passes;
	png_bytep *row_pointers;
	png_bytep *row_pointers_out;
}PngImageData;


typedef struct
{
	uint32_t width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep * row_pointers;
}DstPngImgInfo;


typedef struct
{
	png_structp png_ptr;
	png_infop info_ptr;
}SrcPngImgInfo;

int reduce_png_image(const char *src_img_file, ImgReduceRequest **p_request);


#ifdef __cplusplus
}
#endif

#endif
