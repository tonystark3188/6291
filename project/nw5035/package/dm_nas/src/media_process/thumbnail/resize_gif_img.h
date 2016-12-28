#ifndef _RESIZE_GIF_H_
#define _RESIZE_GIF_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "resize_base.h"

int reduce_gif_image(const char *src_img_file, ImgReduceRequest **p_request);

#ifdef __cplusplus
}
#endif

#endif
