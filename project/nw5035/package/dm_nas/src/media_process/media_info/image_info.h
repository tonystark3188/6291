#ifndef _IMAGE_INFO_H_
#define _IMAGE_INFO_H_

#include <jpeglib.h>
#include <setjmp.h>
#include "media_info.h"

#define IS_UNKONW_TYPE	0
#define IS_JPEG_TYPE	1
#define IS_PNG_TYPE		2
#define IS_GIF_TYPE		3

typedef struct
{
	//FILE *src_file;
	struct jpeg_compress_struct c_info;
	struct jpeg_decompress_struct dc_info;
	struct jpeg_error_mgr c_jerr;
	struct jpeg_error_mgr dc_jerr;
	jmp_buf setjmp_buffer;    /* for return to caller */
}SrcJpegImgInfo;

int get_jpeg_media_info(const char *src_img_file,image_media_info_t* pImageMediaInfo);
int get_png_media_info(const char *src_img_file,image_media_info_t* pImageMediaInfo);


#endif
