#ifndef _RESIZE_BASE_H_
#define _RESIZE_BASE_H_

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

#define TRUE 	1
#define FALSE	0

#define safe_free(p) do{\
	if((p) != NULL)\
	{\
		free((p));\
		(p) = NULL;\
	}\
	}while(0)

#ifndef BIT
#define BIT(val, i) ((val) & (0x1 << (i)))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#endif


//typedef long long int64_t;
//typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
// typedef char int8_t;
typedef unsigned char uint8_t;


typedef struct
{
	//struct jpeg_compress_struct c_info;
	//struct jpeg_error_mgr c_jerr;
	FILE *dst_file;
	uint32_t width;
	uint32_t height;
	uint16_t scale;
	uint8_t  quality;
	uint8_t	 is_accurate;
	char img_name[256];
	//void *img_info;
}ImgReduceRequest;

extern uint16_t _find_scale_factor(uint32_t src_width, uint32_t src_height, 
                                    ImgReduceRequest *p_request);
#ifdef __cplusplus
}
#endif

#endif
