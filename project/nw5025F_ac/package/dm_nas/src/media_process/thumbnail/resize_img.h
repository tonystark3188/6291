#ifndef _RESIZE_IMAGE_H_
#define _RESIZE_IMAGE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "resize_base.h"

#define IS_UNKONW_TYPE	0
#define IS_JPEG_TYPE	1
#define IS_PNG_TYPE		2
#define IS_GIF_TYPE		3

#define PKG_NAME 		"lib_thumbnail"
#define PKG_VERSION		"1.0.01"

int reduce_image(const char *src_img_file, ImgReduceRequest **p_request);

#ifdef __cplusplus
}
#endif

#endif
