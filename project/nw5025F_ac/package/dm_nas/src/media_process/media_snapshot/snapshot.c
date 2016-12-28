#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <jpeglib.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>

#include "internel.h"
#include "snapshot.h"

#define JPEG_QUALITY 60

//ffmpeg
////////////////////////////////////////////////////////////////////////////
static const char* ffmpeg_get_metadata(AVDictionary *m, char* tagname)
{
	if (!tagname) {
		LOG_E("tagname is null\n");
		return NULL;
	}
	if (!m) {
		LOG_E("metadata is null\n");
		return NULL;
	}
	AVDictionaryEntry *tag = NULL;		
	tag = av_dict_get(m, tagname, NULL, 0);
	if(tag)
	    return tag->value;
	else
		return NULL;
}
static void ffmpeg_print_error(int ret)
{
	char errbuf[128];
	av_strerror(ret, errbuf, sizeof(errbuf));
	fprintf(stderr, "Error while filtering: %s\n", errbuf);
	return ;
}

static int ffmepg_seek_frame(AVFormatContext *pFormatCtx, int videoStream, int64_t time_s)
{
	int64_t seek_time = 0;
	if (videoStream >=0) {
		AVStream* stream = pFormatCtx->streams[videoStream];
		seek_time = (time_s*(stream->time_base.den))/(stream->time_base.num);
	}
	else seek_time = time_s * AV_TIME_BASE;
	LOG_I("seek_time:%"PRId64" do_seek_time:%"PRId64"\n", time_s, seek_time);
	int	ret = av_seek_frame(pFormatCtx, videoStream, seek_time, AVSEEK_FLAG_BACKWARD); 
	if (ret < 0) {
		LOG_E("av_seek_frame error\n");
		ffmpeg_print_error(ret);
		return -1;
	}
	return 0;
}
static void ffmpeg_release_info(AVFormatContext **ppFormatCtx, AVCodecContext **ppCodecCtx)
{
	if (*ppCodecCtx) {
		avcodec_close(*ppCodecCtx);
		*ppCodecCtx = NULL;
	}
	if (*ppFormatCtx) {
		avformat_close_input(ppFormatCtx);	
		*ppFormatCtx = NULL;
	}
	return ;
}


//obtain the first stream of 'type' from AVFormatContext* 
static int open_stream_codec(AVFormatContext *pFormatCtx, AVCodecContext **ppCodecCtxO,
									AVCodec **ppCodecO, int* pStreamO, int type)
{	
	int ret = 0;
	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	int stream = -1;
	int i = 0;
	for( i = 0; i < pFormatCtx->nb_streams; i++ )
		if( pFormatCtx->streams[i]->codec->codec_type == type) {
			stream = i;
			break;
		}
	if( stream == -1 ) {
		LOG_E("no stream found\n");
		ret = -1;
		goto err_no;
	} 
	pCodecCtx = pFormatCtx->streams[stream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		LOG_E("avcodec_find_decoder fail\n");
		ret = -1;
		goto err_no;
	}
	if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 ){
		ret = -1;
		goto err_find_codec;
	}
	if (ppCodecO) {
		*ppCodecO = pCodec;
	}
	if (pStreamO)
		*pStreamO = stream;
	*ppCodecCtxO = pCodecCtx;
	return 0;
	
err_find_codec:
	avcodec_close(pCodecCtx);			
err_no:
	return ret;
}


static int ffmpeg_obtain_info(const char* path, AVFormatContext **ppFormatCtxO, AVCodecContext **ppCodecCtxO,
									AVCodec **ppCodecO, int* pStreamO, int type)
{
	int ret = 0;
	AVFormatContext* pFormatCtx = NULL;
	av_register_all();
	if( (ret = avformat_open_input(&pFormatCtx, path, NULL, NULL)) != 0 ) {
		LOG_E("avformat_open_input failed\n");
		ffmpeg_print_error(ret);
		ret = -1;
		goto err_no;
	}
	if( (ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0 ){
		LOG_E("avformat_find_stream_info failed\n");
		ffmpeg_print_error(ret);
		ret = -1;
		goto err_open_input;
	}
	//av_dump_format(*ppFormatCtx, -1, path, 0);
	if (open_stream_codec(pFormatCtx, ppCodecCtxO, ppCodecO, pStreamO, type) < 0) {
		ret = -1;
		goto err_open_input;
	}
	*ppFormatCtxO = pFormatCtx;
	return 0;
	
err_open_input:
	avformat_close_input(&pFormatCtx);
err_no:
	return ret;
}

//ffmpeg end
////////////////////////////////////////////////////////////////////////////

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}


int save_rgb_as_jpeg(const char* out_file, int image_width, int image_height, uint8_t* image_buffer, int quality)
{
	int ret = 0;
	struct jpeg_compress_struct cinfo;
	FILE * fp_outfile;
	JSAMPROW row_pointer[1];
	int row_stride;
	int create_compress = 0;

	if ((fp_outfile = fopen(out_file, "wb")) == NULL) {
		LOG_E("can't open %s: %s\n", out_file, strerror(errno));
		ret = -1;
		goto err_no;
	}

	//setup error handler, quite awful.
	struct my_error_mgr jerr;
	bzero(&jerr, sizeof(struct my_error_mgr));
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		ret = -1;
		//本来是打算对start_compress也做一下finish处理的，但想到这个程序已经出错了，
		//这时再做这个不知会否引起崩溃，所以没加
		if (create_compress == 1)
			goto err_create_compress;
		goto err_file;
	}
	
	jpeg_create_compress(&cinfo);
	create_compress = 1;
	jpeg_stdio_dest(&cinfo, fp_outfile);
	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);
	
	row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(fp_outfile);
	return 0;
err_create_compress:
	jpeg_destroy_compress(&cinfo);
err_file:  
	fclose(fp_outfile);
err_no:
	return ret;
}


int save_rgb_as_jpeg_byindex(const char* out_file, int index, int image_width, int image_height, uint8_t* image_buffer, int quality)
{
	int ret = 0;
	char* szFilename = NULL;
	if (asprintf(&szFilename, "%s%d.jpg", out_file, index) < 0) {
		LOG_E("malloc error\n");
		ret = -1;
		goto err_no;
	}
	
	ret = save_rgb_as_jpeg(szFilename, image_width,
							image_height,image_buffer, quality);
	safe_free(szFilename);
err_no:
	return ret;
}

int seek_frame(AVFormatContext *pFormatCtx, int videoStream, OptionDmContext *dm_context)
{
	int64_t time_s = 0;
	if (strlen(dm_context->seek_proportion) != 0) 
		time_s = pFormatCtx->duration * atoll(dm_context->seek_proportion) / 100 /AV_TIME_BASE;
	else
		time_s = atoll(dm_context->seek_time);

	return ffmepg_seek_frame(pFormatCtx, videoStream, time_s);
}

#if 0
static void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
	FILE *pFile;
	char szFilename[32];
	int y;

	sprintf(szFilename, "frame%d.ppm", iFrame);
	pFile = fopen(szFilename, "wb");
	if( !pFile )
		return;
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	for( y = 0; y < height; y++ )
		fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);

	fclose(pFile);
}
#endif

#if 0
/**
 * 将AVFrame(YUV420格式)保存为JPEG格式的图片
 *
 * @param width YUV420的宽
 * @param height YUV42的高
 *
 */
int MyWriteJPEG(AVFrame* pFrame, int width, int height, int iIndex)
{
    // 输出文件路径
    char out_file[256] = {0};
    //sprintf_s(out_file, sizeof(out_file), "%s%d.jpg", "E:/temp/", iIndex);
	sprintf(out_file, "out_%d.jpg", iIndex);
	
    // 分配AVFormatContext对象
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    
    // 设置输出文件格式
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
    // 创建并初始化一个和该url相关的AVIOContext
    if( avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Couldn't open output file.");
        return -1;
    }
    
    // 构建一个新stream
    AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
    if( pAVStream == NULL ) {
        return -1;
    }
    
    // 设置该stream的信息
    AVCodecContext* pCodecCtx = pAVStream->codec;
    
    pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = PIX_FMT_YUVJ420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    
    // Begin Output some information
    av_dump_format(pFormatCtx, 0, out_file, 1);
    // End Output some information
    
    // 查找解码器
    AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if( !pCodec ) {
        printf("Codec not found.");
        return -1;
    }
    // 设置pCodecCtx的解码器为pCodec
    if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 ) {
        printf("Could not open codec.");
        return -1;
    }
    
    //Write Header
    avformat_write_header(pFormatCtx, NULL);
    
    int y_size = pCodecCtx->width * pCodecCtx->height;
    
    //Encode
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);
    
    // 
    int got_picture = 0;
    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
    if( ret < 0 ) {
        printf("Encode Error.\n");
        return -1;
    }
    if( got_picture == 1 ) {
        //pkt.stream_index = pAVStream->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    av_free_packet(&pkt);

    //Write Trailer
    av_write_trailer(pFormatCtx);

    //printf("Encode Successful.\n");

    if( pAVStream ) {
        avcodec_close(pAVStream->codec);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    
    return 0;
}
#endif


static int audiocover_stream(AVStream *st)
{
	const char* comment = ffmpeg_get_metadata(st->metadata, "comment");
	LOG_I("comment: %s\n", comment?comment:NULL);
	if (comment && strstr(comment, "Cover")) {
		return 1;
	}
	return 0;
}

static int do_media_snapshot(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, 
				int videoStream, OptionDmContext *dm_context)
{
	AVFrame         *pFrame;
	AVFrame         *pFrameRGB;
	AVPacket        packet;
	int             frameFinished = 0;
	int             numBytes;
	uint8_t         *buffer;
	int ret = 0;
	pFrame = avcodec_alloc_frame();
	if( pFrame == NULL ) {
		LOG_E("avcodec_alloc_frame failed\n");
		ret = -1;
		goto err_no;
	}

	pFrameRGB = avcodec_alloc_frame();
	if( pFrameRGB == NULL ) {
		LOG_E("avcodec_alloc_frame failed\n");
		ret = -1;
		goto err_Frame;
	}
	numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
			pCodecCtx->height);
	buffer = av_malloc(numBytes);
	if( buffer == NULL ) {
		LOG_E("av_malloc failed\n");
		ret = -1;
		goto err_FrameRGB;
	}
 
 	avpicture_fill( (AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
						pCodecCtx->width, pCodecCtx->height);
	if (audiocover_stream(pFormatCtx->streams[videoStream]) == 0) {
		LOG_I("not a audiocover stream, do the seek\n");
		if (seek_frame(pFormatCtx, videoStream, dm_context) < 0) {
			ret = -1;
			goto err_buffer;
		}
	}
	int i = 0;
	while(1){
		if ( av_read_frame(pFormatCtx, &packet) < 0 )  {
			LOG_E("av_read_frame error, maybe the file is end\n");
			ret = -1;
			goto err_buffer;
		}
		if (i++ > 2000) {
			LOG_E("can't find keyframe in 2000 packet, failed\n");
			av_free_packet(&packet);
			ret = -1;
			goto err_buffer;
		}
		if( packet.stream_index == videoStream ) {
			LOG_E("qqqq\n");
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			if( frameFinished && pFrame->key_frame==1 ) {
				struct SwsContext *img_convert_ctx = NULL;
				img_convert_ctx = 
					sws_getCachedContext(img_convert_ctx, pCodecCtx->width,
							pCodecCtx->height, pCodecCtx->pix_fmt,
							pCodecCtx->width, pCodecCtx->height,
							PIX_FMT_RGB24, SWS_BICUBIC,
							NULL, NULL, NULL);
				if( !img_convert_ctx ) {
					ret = -1;
					LOG_E("Cannot initialize sws conversion context\n");
					av_free_packet(&packet);
					goto err_buffer;
				}
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
						pFrameRGB->linesize);
				if (save_rgb_as_jpeg(dm_context->output_file_name, pCodecCtx->width,
						pCodecCtx->height,pFrameRGB->data[0], JPEG_QUALITY) < 0) {
					ret = -1;
					av_free_packet(&packet);
					sws_freeContext(img_convert_ctx);
					goto err_buffer;
				}
				sws_freeContext(img_convert_ctx);
				av_free_packet(&packet);
				break;
			}
		}
		av_free_packet(&packet);
	}
	av_free(buffer);
	av_free(pFrameRGB);
	av_free(pFrame);
	return 0;
err_buffer:
	av_free(buffer);
err_FrameRGB:
	av_free(pFrameRGB);
err_Frame:
	av_free(pFrame);
err_no:
	return ret;
}


int fix_bad_context_arg(OptionDmContext *dm_context)
{
	if (dm_context == NULL) {
		LOG_E("dm_context is null!\n");
		return -1;
	}
	if (strlen(dm_context->seek_proportion) != 0) {
		if (atoll(dm_context->seek_proportion) < 0 || atoll(dm_context->seek_proportion) > 100)
			snprintf(dm_context->seek_proportion, sizeof(dm_context->seek_proportion), "%d", 50);
	}
	if (strlen(dm_context->seek_time) != 0) {
		if (atoll(dm_context->seek_time) < 0)
			snprintf(dm_context->seek_time, sizeof(dm_context->seek_time), "%d", 0);
	}
	return 0;
}


int media_snapshot(OptionDmContext *dm_context)
{
	AVFormatContext *pFormatCtx = NULL;
	int             videoStream = 0;
	AVCodecContext  *pCodecCtx = NULL;
	int ret = 0;
	if (fix_bad_context_arg(dm_context) < 0) {
		ret = -1;
		goto err_no;
	}
	if (ffmpeg_obtain_info(dm_context->input_file_name, &pFormatCtx, &pCodecCtx, 
							NULL, &videoStream, AVMEDIA_TYPE_VIDEO) < 0) {
		ret = -1;
		goto err_no;
	}
	if (do_media_snapshot(pFormatCtx, pCodecCtx, videoStream, dm_context) < 0) {
		ret = -1;
		goto err_ffmpeg;
	}
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	return 0;
	
err_ffmpeg:
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
err_no:
	return ret;
}
