#ifndef _RESIZE_JPEG_IMAGE_H_
#define _RESIZE_JPEG_IMAGE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <jpeglib.h>
//#include <setjmp.h>
#include "resize_base.h"

typedef struct
{
	uint8_t *img_data;
	uint32_t width;
	uint32_t height;
	uint8_t  color_space;
	uint8_t  component;
}JpegImageData;

typedef struct
{
	int i;
	int j;
	uint8_t data[4]; // rgba or yuv
}Pixel;
#if 0
struct my_error_mgr {
  struct jpeg_error_mgr pub;  /* "public" fields */
  jmp_buf setjmp_buffer;    /* for return to caller */
};
#endif

typedef struct
{
	//FILE *dst_file;
	struct jpeg_compress_struct c_info;
	struct jpeg_error_mgr c_jerr;
}DstJpegImgInfo;


typedef struct
{
	//FILE *src_file;
	struct jpeg_compress_struct c_info;
	struct jpeg_decompress_struct dc_info;
	struct jpeg_error_mgr c_jerr;
	struct jpeg_error_mgr dc_jerr;
	jmp_buf setjmp_buffer;    /* for return to caller */
}SrcJpegImgInfo;


int reduce_jpeg_image(const char *src_img_file, ImgReduceRequest **p_request);

#ifdef __cplusplus
}
#endif

#endif
