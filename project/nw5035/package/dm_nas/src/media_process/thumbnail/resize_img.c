/*
 * =============================================================================
 *
 *	Filename:  resize_jpeg_img.c
 *
 *	Description:  resize jpeg image
 *
 *	Version:  1.0.00
 *	Created:  2016/11/1 12:00
 *	Revision:  none
 *	Compiler:  gcc
 *
 *	Author:  Neil
 *	Organization:  China
 *
 * =============================================================================
 */
#include "resize_jpeg_img.h"
#include "resize_png_img.h"
#include "resize_gif_img.h"
#include "resize_img.h"

static int check_image_type(const char *src_img_file)
{
	FILE *src_file = NULL;
	char src_buf[9];
	int len = 0;
	int ret = 0;
	memset(src_buf, 0, sizeof(src_buf));

	src_file = fopen(src_img_file, "rb");
	if(src_file == NULL){
		printf("open src image file failed(%s)!\r\n", src_img_file);
		ret = -1;
		goto EXIT;
	}

	fseek(src_file,0L,SEEK_SET);
	len = fread(src_buf, sizeof(char), sizeof(src_buf)-1, src_file);
	if(len != sizeof(src_buf)-1){
		printf("read buf fail !\r\n");
		ret = -1;
		goto EXIT;
	}

	if(memcmp(src_buf, "\377\330\377", 3) == 0){
		printf("is jpeg type\r\n");
		ret = IS_JPEG_TYPE;
		goto EXIT;
	}
	else if(memcmp(src_buf, "GIF8", 4) == 0){
		printf("is gif type\r\n");
		ret = IS_GIF_TYPE;
		goto EXIT;
	}
	else if (memcmp(src_buf, "\211PNG\r\n\032\n", 8) == 0){
		printf("is png type\r\n");
		ret = IS_PNG_TYPE;
		goto EXIT;
	}
	else{
		printf("is unknow tpye: %s\r\n", src_buf);
		ret = IS_UNKONW_TYPE;
		goto EXIT;
	}
EXIT:
	if(src_file != NULL)
		fclose(src_file);
	return ret;
}


int reduce_image(const char *src_img_file, ImgReduceRequest **p_request)
{
	int image_type = 0;
	int ret = 0;	

	image_type = check_image_type(src_img_file);
	if(image_type < 0){
		printf("check image type failed(%s)!\r\n", src_img_file);
		ret = -1;
		goto EXIT;
	}

	switch(image_type){
		case IS_JPEG_TYPE:
			ret = reduce_jpeg_image(src_img_file, p_request);
			if(ret < 0){
				printf("reduce jpeg image fail !\r\n");	
				ret = -1;
				goto EXIT;
			}
			break;
		case IS_PNG_TYPE:
			ret = reduce_png_image(src_img_file, p_request);
			if(ret < 0){
				printf("reduce png image fail !\r\n");	
				ret = -1;
				goto EXIT;
			}
			break;
		case IS_GIF_TYPE:
			ret = reduce_gif_image(src_img_file, p_request);
			if(ret < 0){
				printf("reduce jpeg image fail !\r\n");	
				ret = -1;
				goto EXIT;
			}
			break;
		default:
			printf("unknown image type: (%d)!\r\n", image_type);
			ret = -1;
			goto EXIT;
			break;
	}
	
EXIT:
	return ret;
}


