#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <jpeglib.h>
//#include <setjmp.h>

#include "resize_base.h"
#include "resize_jpeg_img.h"
#include "transupp.h"
#include "exif.h"

struct my_error_mgr {
  struct jpeg_error_mgr pub;  /* "public" fields */
  jmp_buf setjmp_buffer;    /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

// #define PIXEL_VAL(p_img, i, j, offset) (*(p_img->img_data + i * p_img->width * p_img->component + j * p_img->component + offset))

/*
 *Desc: get pixel value by coordiantes.
 *p_img: input, point to the image data
 *p_pixel: input/output, point to the pixel, it include coordiantes.
 */
static void _get_pixel_val(JpegImageData *p_img, Pixel *p_pixel)
{
	int tmp1 = p_pixel->i * p_img->width * p_img->component;
	int tmp2 = p_pixel->j * p_img->component;

	//printf("tmp1: %d, tmp2: %d\n", tmp1, tmp2);
	p_pixel->data[0] = *(p_img->img_data + tmp1 + tmp2 + 0);
	if(p_img->component > 1)
	{
		p_pixel->data[1] = *(p_img->img_data + tmp1 + tmp2 + 1);
		p_pixel->data[2] = *(p_img->img_data + tmp1 + tmp2 + 2);
	}
}

/*
 *Desc: get average value by coordiantes.
 *p_img: input, point to the image data
 *p_pixel: input/output, point to the pixel, it includes specific coordiantes.
 *scale: input, the nums we want to be averaged! For now, we only support 4 and 8!
 *
 */
static void _get_pixel_avg_val(JpegImageData *p_img, Pixel *p_pixel, uint8_t scale)
{
	int tmp_x, tmp_y;
	uint8_t nums = 0;
	uint8_t bit = 0;
	Pixel p[8];

	tmp_x = p_pixel->i;
	tmp_y = p_pixel->j;

	if(scale <= 4)
	{
		nums = 4;
		bit = 2;
	}
	else
	{
		nums = 8;
		bit = 3;
	}
	
	//printf("nums: %d, bit: %d\n", nums, bit);
	//printf("tmp_x: %d, tmp_y: %d\n", tmp_x, tmp_y);
	switch(nums)
	{
	case 8:
		if(tmp_x < 1)
		{
			p[4].i = 0;
			p[5].i = 0;
			p[6].i = 1;
			p[7].i = 1;
		}
		else if(tmp_x >= (p_img->height-1))
		{
			p[4].i = p_img->height - 2;
			p[5].i = p_img->height - 2;
			p[6].i = p_img->height - 1;
			p[7].i = p_img->height - 1;
		}
		else
		{
			p[4].i = tmp_x - 1;
			p[5].i = tmp_x - 1;
			p[6].i = tmp_x + 1;
			p[7].i = tmp_x + 1;
		}

		if(tmp_y < 1)
		{
			p[4].j = 0;
			p[5].j = 1;
			p[6].j = 1;
			p[7].j = 0;
		}
		else if(tmp_y >= (p_img->width-1))
		{
			p[4].j = p_img->width - 2;
			p[5].j = p_img->width - 1;
			p[6].j = p_img->width - 1;
			p[7].j = p_img->width - 2;
		}
		else 
		{
			p[4].j = tmp_y - 1;
			p[5].j = tmp_y + 1;
			p[6].j = tmp_y + 1;
			p[7].j = tmp_y - 1;
		}

	case 4:
		if(tmp_x < 1)
		{
			p[0].i = 0;
			p[1].i = 1;
			p[2].i = 0;
			p[3].i = 0;	
		}
		else if(tmp_x >= (p_img->height-1))
		{
			p[0].i = p_img->height - 2;
			p[1].i = p_img->height - 1;
			p[2].i = p_img->height - 1;
			p[3].i = p_img->height - 1;	
		}
		else 
		{
			p[0].i = tmp_x - 1;
			p[1].i = tmp_x + 1;
			p[2].i = tmp_x;
			p[3].i = tmp_x;
		}

		if(tmp_y < 1)
		{
			p[0].j = 0;
			p[1].j = 0;
			p[2].j = 0;
			p[3].j = 1;	
		}
		else if(tmp_y >= (p_img->width-1))
		{
			p[0].j = p_img->width - 1;
			p[1].j = p_img->width - 1;
			p[2].j = p_img->width - 2;
			p[3].j = p_img->width - 1;	
		}
		else 
		{
			p[0].j = tmp_y;
			p[1].j = tmp_y;
			p[2].j = tmp_y - 1;
			p[3].j = tmp_y + 1;
		}
	}

	uint8_t i;
	for(i = 0; i < nums; ++i)
	{
		//printf("%dth: (i,j)=(%d,%d), img_size=(%d,%d)\n", i, p[i].i, p[i].j, p_img->width, p_img->height);
		_get_pixel_val(p_img, &p[i]);
	}	

	int tmp_r = 0;
    int	tmp_g = 0;
	int	tmp_b = 0;
	for(i = 0; i < nums; ++i)
	{
		tmp_r += p[i].data[0];
		if(p_img->component > 1)
		{
			tmp_g += p[i].data[1];
			tmp_b += p[i].data[2];
		}
	}

	p_pixel->data[0] = (tmp_r >> bit);
	if(p_img->component > 1)
	{
		p_pixel->data[1] = (tmp_g >> bit);
		p_pixel->data[2] = (tmp_b >> bit);
	}

}

/*
 *Desc: compress image by accute output width and height.
 *p_dc_info: input, 
 *p_request: input, point to image reduce request object.
 *p_img_data: input, point to the image data.
 *
 *Return: success on 0.
 */
static int _handle_accute_compress_jpeg(SrcJpegImgInfo *p_src_jpeg_info,
                            ImgReduceRequest *p_request, JpegImageData *p_img_data)
{
#if 0
	if(p_request->scale > 0)
	{
		p_request->width = p_dc_info->image_width / p_request->scale;
		p_request->height = p_dc_info->image_height / p_request->scale;
	}
#endif
	p_src_jpeg_info->c_info.image_width = p_request->width;
	p_src_jpeg_info->c_info.image_height = p_request->height;

	p_src_jpeg_info->c_info.input_components = p_src_jpeg_info->dc_info.output_components;
	p_src_jpeg_info->c_info.in_color_space = p_src_jpeg_info->dc_info.out_color_space;
	jpeg_set_defaults(&p_src_jpeg_info->c_info);
	jpeg_set_quality(&p_src_jpeg_info->c_info, p_request->quality, TRUE);
	jpeg_start_compress(&p_src_jpeg_info->c_info, TRUE);

	float width_scale = p_src_jpeg_info->dc_info.output_width * 1.0f / p_request->width;
	float height_scale = p_src_jpeg_info->dc_info.output_height * 1.0f / p_request->height;

	int row_stride = 0;
	row_stride = p_src_jpeg_info->dc_info.output_width * p_src_jpeg_info->dc_info.output_components;
	JSAMPROW row_pointer[1];

#if 0
	if(p_img_data->img_data == NULL)
	{
		uint8_t *img_buf = (uint8_t *)malloc(p_dc_info->output_width * p_dc_info->output_height * p_dc_info->output_components);
		if(img_buf == NULL)
		{
			printf("malloc failed!");
			return -1;	
		}
		p_img_data->img_data = img_buf;
		p_img_data->width = p_dc_info->output_width;
		p_img_data->height = p_dc_info->output_height;
		p_img_data->color_space = p_dc_info->out_color_space;
		p_img_data->component = p_dc_info->output_components;

		p_dc_info->output_scanline = 0;
		while(p_dc_info->output_scanline < p_dc_info->output_height)
		{
			row_pointer[0] = img_buf + (row_stride * p_dc_info->output_scanline);
			(void)jpeg_read_scanlines(p_dc_info, row_pointer, 1);	
		}
	}
#endif

	uint8_t *line = (uint8_t *)malloc(p_request->width * p_src_jpeg_info->dc_info.output_components);
	if(line == NULL)
	{
		printf("malloc 2 failed!");
		return -2;
	}

	int i;
	int j;
	row_pointer[0] = line;
	Pixel p_tmp;
	uint8_t scale = (int)width_scale + 1;
	//printf("p_request height:%d,width:%d,scale:%d\n", p_request->height, p_request->width, p_request->scale);
	//printf("height_scale:%f, width_scale:%f\n", height_scale, width_scale);
	for(i = 0; i < p_request->height; ++i)
	{
		for(j = 0; j < p_request->width; ++j)
		{
			p_tmp.i = (int)(i * height_scale);
			p_tmp.j = (int)(j * width_scale);

#if 0
			//if(p_tmp.i >= p_dc_info->output_width)
			//	p_tmp.i = p_dc_info->output_width - 1;

			//if(p_tmp.j >= p_dc_info->output_height)
			//	p_tmp.j = p_dc_info->output_height - 1;

			_get_pixel_val(p_img_data, &p_tmp);
#else
			 _get_pixel_avg_val(p_img_data, &p_tmp, scale); 
#endif
			
			if(p_src_jpeg_info->dc_info.output_components > 1)
			{
                *(line + ((j << 1) + j) + 0) = p_tmp.data[0];
				*(line + ((j << 1) + j) + 1) = p_tmp.data[1];
				*(line + ((j << 1) + j) + 2) = p_tmp.data[2];	
			}
            else
            {
                *(line + j) = p_tmp.data[0];
            }
		}
		(void)jpeg_write_scanlines(&p_src_jpeg_info->c_info, row_pointer, 1);
	}

	safe_free(line);
	return 0;
}

/*
 *Desc: compress image with specific image data.
 *p_dc_info: input, point to jpeg_decompress_struct object.
 *p_request: input, point to image reduct request object.
 *p_img_data: input, point to image data.
 *
 *Return: success on 0.
 */
 #if 0
static int _handle_normal_compress_jpeg(struct jpeg_decompress_struct *p_dc_info, 
                            ImgReduceRequest *p_request, JpegImageData *p_img_data)
{
	p_request->c_info.image_width = p_dc_info->output_width;
	p_request->c_info.image_height = p_dc_info->output_height;
	p_request->c_info.input_components = p_dc_info->output_components;
	p_request->c_info.in_color_space = p_dc_info->out_color_space;
	jpeg_set_defaults(&p_request->c_info);
	jpeg_set_quality(&p_request->c_info, p_request->quality, TRUE);
	jpeg_start_compress(&p_request->c_info, TRUE);

	int row_stride = 0;
	row_stride = p_dc_info->output_width * p_dc_info->output_components;
	JSAMPROW row_pointer[1];

	uint32_t i = 0;
	for(i = 0; i < p_img_data->height; ++i)
	{
		row_pointer[0] = p_img_data->img_data + row_stride * i;
		(void)jpeg_write_scanlines(&p_request->c_info, row_pointer, 1);
	}

	return 0;
}
#endif

static int _handle_and_compress_jpeg(SrcJpegImgInfo *p_src_jpeg_info, 
                            ImgReduceRequest *p_request, JpegImageData *p_img_data)
{
	DstJpegImgInfo dst_jpeg_info;
	memset(&dst_jpeg_info, 0, sizeof(DstJpegImgInfo));
	//p_request->img_info = &dst_jpeg_info;
	
	p_request->dst_file = fopen(p_request->img_name, "wb");
	if(p_request->dst_file == NULL)
	{
		printf("open dst image file failed(%s)!\r\n", p_request->img_name);
		return -1;
	}

	p_src_jpeg_info->c_info.err = jpeg_std_error(&p_src_jpeg_info->c_jerr);
	jpeg_create_compress(&p_src_jpeg_info->c_info);
	jpeg_stdio_dest(&p_src_jpeg_info->c_info, p_request->dst_file);

	if(p_request->scale > 0)
	{
		p_request->width = p_src_jpeg_info->dc_info.image_width / p_request->scale;
		p_request->height = p_src_jpeg_info->dc_info.image_height / p_request->scale;
	}

	if(p_request->is_accurate)
	{
		_handle_accute_compress_jpeg(p_src_jpeg_info, p_request, p_img_data);
	}
	else
	{
		#if 0
		uint8_t tmp = p_dc_info->output_width / p_request->width;

		if(tmp < 2)
		{
			_handle_normal_compress_jpeg(p_dc_info, p_request, p_img_data);
		}
		else
		{
			tmp = _find_bit(tmp);
			p_request->width = (p_dc_info->output_width >> tmp);
			p_request->height = (p_dc_info->output_height >> tmp);
			p_request->is_accurate = 1;
			p_request->scale = 0;
			_handle_accute_compress_jpeg(p_dc_info, p_request, p_img_data);
		}
		#endif
	}

	jpeg_finish_compress(&p_src_jpeg_info->c_info);
	fclose(p_request->dst_file);
	jpeg_destroy_compress(&p_src_jpeg_info->c_info);

	return 0;
}



static void _do_size_adjust(struct jpeg_decompress_struct *dc_info, ImgReduceRequest *p_request)
{
    //for original file, width < height, we should exchange the width and height of output image 
    if(dc_info->image_width < dc_info->image_height)
    {
		if(p_request->width > p_request->height)
		{
		    uint32_t tmp;

		    tmp = p_request->width;
		    p_request->width = p_request->height;
		    p_request->height = tmp;
		}
	}
	else
	{
	    if(p_request->width < p_request->height)
		{
		    uint32_t tmp;

		    tmp = p_request->width;
		    p_request->width = p_request->height;
		    p_request->height = tmp;
		}
	}

	if(dc_info->image_width >= p_request->width 
		 && dc_info->image_height >= p_request->height)
	{
		float width_scale, height_scale;
		width_scale = (float)dc_info->image_width / (float)p_request->width;
		height_scale = (float)dc_info->image_height / (float)p_request->height;

		//printf("width_scale:%f, height_scale: %f\n", width_scale, height_scale);
		//printf("p_request->height: %d, p_request->width: %d\n", p_request->height, p_request->width);
		if(width_scale > height_scale){
			p_request->height = dc_info->image_height * p_request->width / dc_info->image_width;
		}
		else{
			p_request->width = dc_info->image_width * p_request->height / dc_info->image_height;
		}
		//printf("p_request->height: %d, p_request->width: %d\n", p_request->height, p_request->width);
	}
	else if(dc_info->image_width >= p_request->width 
		&& dc_info->image_height <= p_request->height)
	{
		p_request->height = dc_info->image_height * p_request->width / dc_info->image_width;
	}
	else if(dc_info->image_width <= p_request->width
		&& dc_info->image_height >= p_request->height)
	{
		p_request->width = dc_info->image_width * p_request->height / dc_info->image_height;
	}
		
}


void adjust_jpeg_output_size(struct jpeg_decompress_struct *dc_info, 
									 ImgReduceRequest **p_request)
{
	ImgReduceRequest **p = p_request;

	while(*p != NULL)
	{
		_do_size_adjust(dc_info, *p);
		p++;
	}
}


/*
 *Desc: reduce image by request.
 *src_img_file: input, input image file name
 *p_request: input, point to the first request object, end with NULL pointer.
 *
 *Return: 1 : means reduce image success.
          0 : means not handle it.
         <0 : means error occure. 
 */
int reduce_jpeg_image(const char *src_img_file, ImgReduceRequest **p_request)
{
	uint16_t scale = 1;
	SrcJpegImgInfo src_jpeg_info;
	int ret = 0;
	JpegImageData img_data;
	int row_stride = 0;
	FILE *src_file = NULL;
	JSAMPROW row_pointer[1];
	int rotate_type = 0; 
	int jxform_value = 0;
    
	memset(&img_data, 0, sizeof(JpegImageData));
	memset(&src_jpeg_info, 0, sizeof(SrcJpegImgInfo));
	if(src_img_file == NULL || p_request == NULL){
		printf("invalid argument!\r\n");
		ret = -1;
		goto EXIT;
	}
	
	src_file = fopen(src_img_file, "rb");
	if(src_file == NULL){
		printf("open src image file failed(%s)!\r\n", src_img_file);
		ret = -1;
		goto EXIT;
	}
	//printf("start decompress!\r\n");
	src_jpeg_info.dc_info.err = jpeg_std_error(&src_jpeg_info.dc_jerr);
    /* We set up the normal JPEG error routines, then override error_exit. */
    src_jpeg_info.dc_jerr.error_exit = my_error_exit;
     /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(src_jpeg_info.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
        jpeg_destroy_decompress(&src_jpeg_info.dc_info);
		ret = -1;
		goto EXIT;
    }
    
	jpeg_create_decompress(&src_jpeg_info.dc_info);
	jpeg_stdio_src(&src_jpeg_info.dc_info, src_file);
	jpeg_read_header(&src_jpeg_info.dc_info, TRUE);

	// adjust dest width and height.
	adjust_jpeg_output_size(&src_jpeg_info.dc_info, p_request);

    // if src file is too small, we will do nothing to it. 
    // add by wenhao at 2014-10-31
    if(src_jpeg_info.dc_info.image_width < p_request[0]->width &&
        src_jpeg_info.dc_info.image_height < p_request[0]->height){
        printf("src(%d x %d), big dest(%d x %d)", src_jpeg_info.dc_info.image_width, 
                    src_jpeg_info.dc_info.image_height, p_request[0]->width, p_request[0]->height);
        // do not call jpeg_finish_decompress, because we do not decompress it now!
	    //jpeg_finish_decompress(&dc_info);
	    jpeg_destroy_decompress(&src_jpeg_info.dc_info);
        ret = 0;
		goto EXIT;
		//!!!!!!!!!!!!!!!cp the original or get the original file
    }

	scale = _find_scale_factor(src_jpeg_info.dc_info.image_width, src_jpeg_info.dc_info.image_height, p_request[0]);
	if(scale > 8){
		printf("For now, We do not support over 1/8 reduce!\n");
		scale = 8;
	}

	if(scale > 1){
		src_jpeg_info.dc_info.scale_num = 1;
		src_jpeg_info.dc_info.scale_denom = scale;
	}
	//printf("scale = %d\r\n", scale);

	jpeg_start_decompress(&src_jpeg_info.dc_info);
	printf("src image width(%d), height(%d), out_color_space(%d), color_space(%d), num_components(%d), output_components(%d)\r\n",
			src_jpeg_info.dc_info.image_width, src_jpeg_info.dc_info.image_height, src_jpeg_info.dc_info.out_color_space, src_jpeg_info.dc_info.jpeg_color_space, 
			src_jpeg_info.dc_info.num_components, src_jpeg_info.dc_info.output_components);
	//printf("finish decompress!\r\n");

	row_stride = src_jpeg_info.dc_info.output_width * src_jpeg_info.dc_info.output_components;
	uint8_t *img_buf = (uint8_t *)malloc(row_stride * src_jpeg_info.dc_info.output_height);
	if(img_buf == NULL){
		printf("malloc failed!");
        jpeg_finish_decompress(&src_jpeg_info.dc_info);
	    jpeg_destroy_decompress(&src_jpeg_info.dc_info);
		ret = -1;
		goto EXIT;	
	}
	img_data.img_data = img_buf;
	img_data.width = src_jpeg_info.dc_info.output_width;
	img_data.height = src_jpeg_info.dc_info.output_height;
	img_data.color_space = src_jpeg_info.dc_info.out_color_space;
	img_data.component = src_jpeg_info.dc_info.output_components;

	while(src_jpeg_info.dc_info.output_scanline < src_jpeg_info.dc_info.output_height){
		row_pointer[0] = img_buf + (row_stride * src_jpeg_info.dc_info.output_scanline);
		(void)jpeg_read_scanlines(&src_jpeg_info.dc_info, row_pointer, 1);	
	}
	
	if(src_file != NULL)
		fclose(src_file);

	ret = get_exif_rotate_type(src_img_file, IMAGE_EXIF_IFD_0);
	if(ret > 0){
		rotate_type = ret;
	}
	else{
		rotate_type = 0;
	}
	//printf("rotate_type: %d\n", rotate_type);

	switch(rotate_type){
		case Rotate_Bottom_Right:
			jxform_value = JXFORM_ROT_180;
			break;
		case Rotate_Right_Top:
			jxform_value = JXFORM_ROT_90;
			break;
		case Rotate_Left_Bottom:
			jxform_value = JXFORM_ROT_270;
			break;
		default:
			jxform_value = 0;
			break;
	}
	
	ImgReduceRequest **p_img_request = p_request;
	while((*p_img_request) != NULL){
		ret = _handle_and_compress_jpeg(&src_jpeg_info, (*p_img_request), &img_data);		
		//printf("ret : %d, jxform_value: %d\n", ret, jxform_value);
		if(!ret && jxform_value){
			//printf("start rotate image\n");
			ret = rotate_jpeg_image((*p_img_request)->img_name, jxform_value);
			if(ret == 0){
				printf("rotate image success\n");
			} 
			else{
				printf("rotate image fail\n");
			}
		}
		p_img_request += 1;
	}
#if 0
	uint8_t i = 0;
	for(i = 0; i < nums; ++i)
	{
		_handle_and_compress_jpeg(&dc_info, p_request[i], &img_data);		
	}
#endif
	
	jpeg_finish_decompress(&src_jpeg_info.dc_info);
	jpeg_destroy_decompress(&src_jpeg_info.dc_info);
	safe_free(img_data.img_data);

	//creat_exif_thumbnail(src_img_file, "bb.jpeg");
	return ret;
EXIT:
	if(src_file != NULL)
		fclose(src_file);
	return ret;
}

#if 0
int reduce_jpeg_image(const char *src_img_file, ImgReduceRequest **p_request)
{
	return get_exif_rotate(src_img_file, IMAGE_EXIF_IFD_0);
}
#endif

