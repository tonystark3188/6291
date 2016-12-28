#include "resize_png_img.h"

static void _do_png_size_adjust(PngImageData *p_png_image_data, ImgReduceRequest *p_request)
{
    //for original file, width < height, we should exchange the width and height of output image 
    if(p_png_image_data->width < p_png_image_data->height)
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

	if(p_png_image_data->width >= p_request->width 
		 && p_png_image_data->height >= p_request->height)
	{
		float width_scale, height_scale;
		width_scale = (float)p_png_image_data->width / (float)p_request->width;
		height_scale = (float)p_png_image_data->height / (float)p_request->height;

		//printf("width_scale:%f, height_scale: %f\n", width_scale, height_scale);
		//printf("p_request->height: %d, p_request->width: %d\n", p_request->height, p_request->width);
		if(width_scale > height_scale){
			p_request->height = p_png_image_data->height * p_request->width / p_png_image_data->width;
		}
		else{
			p_request->width = p_png_image_data->width * p_request->height / p_png_image_data->height;
		}
		//printf("p_request->height: %d, p_request->width: %d\n", p_request->height, p_request->width);
	}
	else if(p_png_image_data->width >= p_request->width 
		&& p_png_image_data->height <= p_request->height)
	{
		p_request->height = p_png_image_data->height * p_request->width / p_png_image_data->width;
	}
	else if(p_png_image_data->width <= p_request->width
		&& p_png_image_data->height >= p_request->height)
	{
		p_request->width = p_png_image_data->width * p_request->height / p_png_image_data->height;
	}

	#if 0
	if(p_png_image_data->width >= p_request->width 
		 && p_png_image_data->height <= p_request->height)
	{
		p_request->width = p_request->width * p_png_image_data->height / p_request->height;
		p_request->height = p_png_image_data->height;
	}
	else if(p_png_image_data->width <= p_request->width
		&& p_png_image_data->height >= p_request->height)
	{
		p_request->height = p_request->height * p_png_image_data->width / p_request->width;
		p_request->width = p_png_image_data->width;
	}
	#endif
}


static void adjust_png_output_size(PngImageData *p_png_image_data, ImgReduceRequest **p_request)
{
	ImgReduceRequest **p = p_request;

	while(*p != NULL)
	{
		_do_png_size_adjust(p_png_image_data, *p);
		p++;
	}
}

static int read_png_file(const char *src_img_file, SrcPngImgInfo *p_src_png_info, PngImageData *p_png_image_data)
{
	int x, y;
	int ret = 0;
	FILE *src_file = NULL;
	char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
  	src_file = fopen(src_img_file, "rb");
    if (!src_file){
		printf("open src image file failed(%s)!\r\n", src_img_file);
		ret = -1;
		goto EXIT;
	}
    fread(header, 1, 8, src_file);
    if (png_sig_cmp(header, 0, 8)){
		printf("read src image file failed(%s)!\r\n", src_img_file);
		ret = -1;
		goto EXIT;
	}	

    /* initialize stuff */
    p_src_png_info->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!p_src_png_info->png_ptr){
		printf("png_create_read_struct failed! \r\n");
		ret = -1;
		goto EXIT;
	}

    p_src_png_info->info_ptr = png_create_info_struct(p_src_png_info->png_ptr);
    if (!p_src_png_info->info_ptr){
		printf("png_create_info_struct failed! \n\r");
		ret = -1;
		goto EXIT;
	}

    if (setjmp(png_jmpbuf(p_src_png_info->png_ptr))){
		printf("Error during init_io! \n\r");
		ret = -1;
		goto EXIT;
	}
    png_init_io(p_src_png_info->png_ptr, src_file);
    png_set_sig_bytes(p_src_png_info->png_ptr, 8);

    png_read_info(p_src_png_info->png_ptr, p_src_png_info->info_ptr);

    p_png_image_data->width = png_get_image_width(p_src_png_info->png_ptr, p_src_png_info->info_ptr);
    p_png_image_data->height = png_get_image_height(p_src_png_info->png_ptr, p_src_png_info->info_ptr);
    p_png_image_data->color_type = png_get_color_type(p_src_png_info->png_ptr, p_src_png_info->info_ptr);
    p_png_image_data->bit_depth = png_get_bit_depth(p_src_png_info->png_ptr, p_src_png_info->info_ptr);
	p_png_image_data->channels = png_get_channels(p_src_png_info->png_ptr, p_src_png_info->info_ptr); /*获取通道数*/
		
    p_png_image_data->number_of_passes = png_set_interlace_handling(p_src_png_info->png_ptr);
    png_read_update_info(p_src_png_info->png_ptr, p_src_png_info->info_ptr);


    /* read file */
    if (setjmp(png_jmpbuf(p_src_png_info->png_ptr))){
		printf("[read_png_file] Error during read_image \r\n");
		ret = -1;
		goto EXIT;
	}

	p_png_image_data->row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * p_png_image_data->height);
    for (y=0; y<p_png_image_data->height; y++)
            p_png_image_data->row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(p_src_png_info->png_ptr,p_src_png_info->info_ptr));

    png_read_image(p_src_png_info->png_ptr, p_png_image_data->row_pointers);

EXIT:
	if(src_file != NULL)
		fclose(src_file);

	return ret;
}

#if 0
void process_file_display(int w_Dest,int h_Dest)
{
	int x, y;
	#if 0
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
            abort_("[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
                   "(lacks the alpha channel)");

    if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
            abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
                   PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));
	#endif

    for (y=0; y<h_Dest; y++) {
            png_byte* row = row_pointers_out[y];
            for (x=0; x<w_Dest; x++) {
                    png_byte* ptr = &(row[x*3]);
                    printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d\n",
                           x, y, ptr[0], ptr[1], ptr[2]);
            }
    }
}
#endif


int write_png_file(ImgReduceRequest *p_request, PngImageData *p_png_img_data)
{
	int x, y;
	SrcPngImgInfo src_png_info;
    /* create file */
	memset(&src_png_info, 0, sizeof(SrcPngImgInfo));
    FILE *fp = fopen(p_request->img_name, "wb");
    if (!fp){
        printf("[write_png_file] File %s could not be opened for writing\r\n", p_request->img_name);
		return -1;
    }
    /* initialize stuff */
    src_png_info.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!src_png_info.png_ptr){
        printf("[write_png_file] png_create_write_struct failed\r\n");
		return -1;
	}

    src_png_info.info_ptr = png_create_info_struct(src_png_info.png_ptr);
    if (!src_png_info.info_ptr){
		printf("[write_png_file] png_create_info_struct failed\r\n");
		return -1;
	}

    if (setjmp(png_jmpbuf(src_png_info.png_ptr))){
        printf("[write_png_file] Error during init_io\r\n");
		return -1;
	}

	png_init_io(src_png_info.png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(src_png_info.png_ptr))){
        printf("[write_png_file] Error during writing header\r\n");
		return -1;
	}

    //png_set_IHDR(png_ptr, info_ptr, width, height,
    png_set_IHDR(src_png_info.png_ptr, src_png_info.info_ptr, p_request->width, p_request->height,
                 p_png_img_data->bit_depth, p_png_img_data->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(src_png_info.png_ptr, src_png_info.info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(src_png_info.png_ptr))){
        printf("[write_png_file] Error during writing bytes\r\n");
		return -1;
	}
	
	if(p_png_img_data->row_pointers_out == NULL){
		printf("row_pointers_out is null");
		return -1;
	}
		
	png_write_image(src_png_info.png_ptr, p_png_img_data->row_pointers_out);

    /* end write */
    if (setjmp(png_jmpbuf(src_png_info.png_ptr))){
    	printf("[write_png_file] Error during end of write\r\n");
		return -1;
    }
	
    png_write_end(src_png_info.png_ptr, NULL);
	
    fclose(fp);
}

#if 0
static int do_Stretch_Linear_Png(ImgReduceRequest *p_request, PngImageData *p_png_img_data)
{
	uint32_t x = 0, y = 0;
	uint32_t i = 0, j = 0;
	uint32_t scale = 0, w_scale = 0, h_scale = 0;

	w_scale = p_png_img_data->width/p_request->width;
	h_scale = p_png_img_data->height/p_request->height;

	scale = (w_scale > h_scale)?h_scale:w_scale;
	printf("p_png_img_data->width: %d, p_png_img_data->height: %d, p_request->width: %d, p_request->height: %d\r\n", \
		p_png_img_data->width, p_png_img_data->height, p_request->width, p_request->height);
	printf("w_scale: %d, h_scale: %d, scale: %d\r\n", w_scale, h_scale, scale);

	p_png_img_data->row_pointers_out = (png_bytep*) malloc(sizeof(png_bytep)*p_request->height);
	for (i=0,y=0; y<p_png_img_data->height&&i<p_request->height; y+=h_scale, i++) {
			p_png_img_data->row_pointers_out[i] = (png_byte*) malloc(sizeof(png_byte)*p_request->width*3);
            png_byte* row = p_png_img_data->row_pointers[y];
			png_byte* row_out = p_png_img_data->row_pointers_out[i];
            for (x=0, j=0; x<p_png_img_data->width&&j<p_request->width; x+=w_scale, j++) {
                    png_byte* ptr = &(row[x*3]);
					png_byte* ptr_out = &(row_out[j*3]);
					
					ptr_out[0] = ptr[0];
					ptr_out[1] = ptr[1];
					ptr_out[2] = ptr[2];
					//printf("i:%d, y:%d, j:%d, x:%d\r\n", i, y, j, x);
            }
    }
	return 0;
}
#endif

static int do_Stretch_Linear_Png(ImgReduceRequest *p_request, PngImageData *p_png_img_data)
{
	uint32_t x = 0, y = 0;
	int nx , ny;
	int pixel_len = 0;

	if(p_png_img_data->channels == 3 && p_png_img_data->color_type == PNG_COLOR_TYPE_RGB){
		pixel_len = 3;
	}
	else if(p_png_img_data->channels == 4 && p_png_img_data->color_type == PNG_COLOR_TYPE_RGB_ALPHA){
		pixel_len = 4;
	}
	else{
		printf("nonsupport format channel(%d), color_type(%d)\n", p_png_img_data->channels, p_png_img_data->color_type);
		return -1;
	}		
	
	p_png_img_data->row_pointers_out = (png_bytep*) malloc(sizeof(png_bytep)*p_request->height);
	for(y = 0; y < p_request->height; y++){
		p_png_img_data->row_pointers_out[y] = (png_byte*) malloc(sizeof(png_byte)*p_request->width*3);
	}

	for(y = 0; y < p_request->height; y++){
		for(x = 0; x < p_request->width; x++){
			nx = x * p_png_img_data->width / p_request->width;
			ny = y * p_png_img_data->height / p_request->height;
			p_png_img_data->row_pointers_out[y][x*pixel_len] = p_png_img_data->row_pointers[ny][nx*pixel_len];
			p_png_img_data->row_pointers_out[y][x*pixel_len+1] = p_png_img_data->row_pointers[ny][nx*pixel_len+1];
			p_png_img_data->row_pointers_out[y][x*pixel_len+2] = p_png_img_data->row_pointers[ny][nx*pixel_len+2];
			if(pixel_len == 4){
				p_png_img_data->row_pointers_out[y][x*pixel_len+3] = p_png_img_data->row_pointers[ny][nx*pixel_len+3];
			}
		}
	}

	return 0;
}


static int _handle_and_compress_png(SrcPngImgInfo *p_src_png_info, 
                            ImgReduceRequest *p_request, PngImageData *p_png_img_data)
{
	int ret = 0;
	int x = 0, y = 0;
	//DstPngImgInfo src_png_info;
	//memset(&src_png_info, 0, sizeof(SrcPngImgInfo));
	//p_request->img_info = &src_png_info;

	if(p_png_img_data->row_pointers == NULL){
		printf("row_pointers is null");
		ret = -1;
		goto EXIT1;
	}

	ret = do_Stretch_Linear_Png(p_request, p_png_img_data);
	if(ret < 0){
		printf("do_Stretch_Linear fail\r\n");
		ret = -1;
		goto EXIT1;
	}

	p_request->dst_file = fopen(p_request->img_name, "wb");
	if(p_request->dst_file == NULL){
		printf("open dst image file failed(%s)!\r\n", p_request->img_name);
		ret = -1;
		goto EXIT0;
	}

	ret = write_png_file(p_request, p_png_img_data);
	if(ret < 0){
		printf("write png file fail\r\n");
		ret = -1;
		goto EXIT0;
	}

	  /* cleanup heap allocation */
EXIT0:
    for (y=0; y < p_request->height; y++)
    	safe_free(p_png_img_data->row_pointers_out[y]);
    safe_free(p_png_img_data->row_pointers_out);	
EXIT1:
	return ret;
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
int reduce_png_image(const char *src_img_file, ImgReduceRequest **p_request)
{
	int ret = 0;
	int x = 0, y = 0;
	int scale = 0;
	SrcPngImgInfo src_png_info;
	PngImageData png_image_data;
	memset(&src_png_info, 0, sizeof(SrcPngImgInfo));
	memset(&png_image_data, 0, sizeof(PngImageData));

	if(src_img_file == NULL || p_request == NULL){
		printf("invalid argument!\r\n");
		return -1;
	}

	ret = read_png_file(src_img_file, &src_png_info, &png_image_data);
	if(ret){
		printf("read png file fail \r\n");
		ret = -1;
		goto EXIT;
	}

	adjust_png_output_size(&png_image_data, p_request);

	// if src file is too small, we will do nothing to it. 
    if(png_image_data.width < p_request[0]->width &&
        png_image_data.height < p_request[0]->height){
        printf("src(%d x %d), big dest(%d x %d)", png_image_data.width, 
                    png_image_data.height, p_request[0]->width, p_request[0]->height);
        return 0;
		//!!!!!!!!!!!!!!!cp the original or get the original file
    }

	scale = _find_scale_factor(png_image_data.width, png_image_data.height, p_request[0]);
	if(scale > 8){
		printf("For now, We do not support over 1/8 reduce!\n");
		scale = 8;
	}

	if(scale > 1)
	{
		//src_jpeg_info.c_info.scale_num = 1;
		//src_jpeg_info.c_info.scale_denom = scale;
	}
	//printf("scale = %d\r\n", scale);

	ImgReduceRequest **p_img_request = p_request;
	while((*p_img_request) != NULL)
	{
		_handle_and_compress_png(&src_png_info, (*p_img_request), &png_image_data);		
		p_img_request += 1;
	}

EXIT:
	for (y=0; y<png_image_data.height; y++)
    	safe_free(png_image_data.row_pointers[y]);
	safe_free(png_image_data.row_pointers);
	
	return ret;
}



