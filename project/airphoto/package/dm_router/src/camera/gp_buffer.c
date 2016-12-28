#include<stdio.h>
#include<memory.h>
#include<string.h>
#include<errno.h>
#include <stdlib.h>

#include "debuglog.h"
#include "gp_buffer.h"

#define BUF_MALLOC_SIZE 100

int free_buffer(struct my_buffer* buffer)
{
	if (!buffer)
		return 0;
	if (buffer->buf) {
		free(buffer->buf);
		buffer->buf = 0;
	}
	buffer->buffer_size = 0;
	buffer->stream_len = 0;
	buffer->offset = 0;
	return 0;
		
}

static int get_buffer(struct my_buffer* buffer, void *d, int rsize)
{
	if (buffer->offset + rsize > buffer->stream_len) {
		//DEBG(0, "[read %d bytes exceed buffer!!!][len=%d offset=%d]\n", rsize, buffer->stream_len, buffer->offset);
		return -1;
	}
	memcpy(d, buffer->buf+buffer->offset, rsize);
	buffer->offset+=rsize;
	
	return 0;
}

static int get_buffer_interger(struct my_buffer* buffer, void* num, int num_size)
{		
	unsigned long long num_64 = 0;
	if (buffer->offset + num_size > buffer->stream_len) {
		//DEBG(0, "[read %d bytes exceed buffer!!!][len=%d offset=%d]\n", num_size, buffer->stream_len, buffer->offset);
		return -1;
	}
	int i = 0;
	num_64 = 0;
	for (i = num_size-1; i >= 0; --i) {
		num_64 += *(buffer->buf+buffer->offset)<<8*i;
		buffer->offset++;
	}
	switch (num_size) {
		case 1:
			*(unsigned char*)num = num_64;
			break;
		case 2:
			*(unsigned short*)num = num_64;
			break;
		case 4:
			*(unsigned int*)num = num_64;
			break;
		case 8:
			*(unsigned long long*)num = num_64;
			break;
	}
	return 0;
}

int get_buffer_8(struct my_buffer* buffer, unsigned char *d)
{
	return get_buffer_interger(buffer, (unsigned long long *)d, 1);
}

int get_buffer_16(struct my_buffer* buffer,  unsigned short *d)
{
	return get_buffer_interger(buffer, (unsigned long long *)d, 2);
}

int get_buffer_32(struct my_buffer* buffer, unsigned int *d)
{
	return get_buffer_interger(buffer, (unsigned long long *)d, 4);
}

int get_buffer_64(struct my_buffer* buffer, unsigned long long *d)
{
	return get_buffer_interger(buffer, (unsigned long long *)d, 8);
}


int get_buffer_stream(struct my_buffer* buffer, unsigned char* bytes, int rsize)
{
	return get_buffer(buffer, (void*)bytes, rsize);
}

static int ensure_buffer_size(unsigned char** maior_buf, int *len, int offset, int size)
{
	unsigned char* buf = *maior_buf;
	if (offset + size > *len) {
		int newlen = *len+size+BUF_MALLOC_SIZE;
		char* tmp;
		CK20 (tmp = realloc(buf, newlen), err_crealloc);
		buf = (unsigned char*)tmp;
		*len = newlen;
	}
	*maior_buf = buf;
	return 0;
	
err_crealloc:
	//DEBG(0, "[realloc failed][%s]\n", strerror(errno));
	return -1;
}
int buffer_expand_size(struct my_buffer* buffer, int size)
{
	return ensure_buffer_size(&buffer->buf, &buffer->buffer_size, buffer->buffer_size, size);
}

static int put_buffer(struct my_buffer* buffer, void* srcbuf, int wsize)
{
	CK02 (ensure_buffer_size(&buffer->buf, &buffer->buffer_size, buffer->offset, wsize), err_no);
	memcpy(buffer->buf+buffer->offset, srcbuf, wsize);
	buffer->offset += wsize;
	if (buffer->offset > buffer->stream_len)
		buffer->stream_len = buffer->offset;
	return 0;

err_no:
	return -1;
}


static int put_buffer_interget(struct my_buffer* buffer, unsigned long long num, int num_size)
{
	CK02 (ensure_buffer_size(&buffer->buf, &buffer->buffer_size, buffer->offset, num_size), err_no);
	int i = 0;
	for (i = num_size-1; i >= 0; --i) {
		*(buffer->buf+buffer->offset) = (num >> 8*i)&0xff;
		buffer->offset++;
	}
	if (buffer->offset > buffer->stream_len)
		buffer->stream_len = buffer->offset;
	return 0;
	
err_no:
	return -1;
}

int put_buffer_8(struct my_buffer* buffer, unsigned char p)
{
	return put_buffer_interget(buffer, p, 1);
}

int put_buffer_16(struct my_buffer* buffer, unsigned short p)
{
	return put_buffer_interget(buffer, p, 2);
}

int put_buffer_32(struct my_buffer* buffer, unsigned int p)
{
	return put_buffer_interget(buffer, p, 4);
}

int put_buffer_64(struct my_buffer* buffer, unsigned long long p)
{
	return put_buffer_interget(buffer, p, 8);
}

int put_buffer_stream(struct my_buffer* buffer, unsigned char* srcbuf, int wsize)
{
	return put_buffer(buffer, srcbuf, wsize);
}

static int get_buffer_stream_r(struct my_buffer *buffer, unsigned char** pp_str, int* p_size, int len_space)
{
	int size = 0;
	int ret = 0;
	switch (len_space) {
		case 1:
			CK00 (ret = get_buffer_8(buffer,(unsigned char*)&size), err_no);
			break;
		case 2:		
			CK00 (ret = get_buffer_16(buffer,(unsigned short*)&size), err_no);
			break;
		case 4:		
			CK00 (ret = get_buffer_32(buffer,(unsigned int*)&size), err_no);
			break;
		default:
			ret = -2;
			return ret;
	}
	CK20 (*pp_str = (unsigned char*) malloc(size+1), err_cmalloc_no);
	memset(*pp_str, 0, size+1);
	CK00 (ret = get_buffer_stream(buffer, *pp_str, size), err_ppstr);
	if (p_size)
		*p_size = size;
	return 0;

err_cmalloc_no:
	//DEBG (0, "[error_msg][malloc error][%s]\n", strerror(errno) );
	ret = -1;
	return ret;
err_ppstr:
	free(*pp_str);
	*pp_str = NULL;
err_no:
	return ret;
}

//get a entire buffer stream which size field  takes 8 bytes
int get_buffer_stream_8(struct my_buffer *buffer, unsigned char** pp_str, int* p_size)
{
	int ret = 0;
	ret = get_buffer_stream_r(buffer, pp_str, p_size, 8);
	if (ret < 0)
		return -1;
	return 0;
}
//get a entire buffer stream which size field  takes 4 bytes
int get_buffer_stream_4(struct my_buffer *buffer, unsigned char** pp_str, int* p_size)
{
	int ret = 0;
	ret = get_buffer_stream_r(buffer, pp_str, p_size, 4);
	if (ret < 0)
		return -1;
	return 0;
}

//get a entire buffer stream which size field  takes 2 bytes
int get_buffer_stream_2(struct my_buffer *buffer, unsigned char** pp_str, int* p_size)
{
	int ret = 0;
	ret = get_buffer_stream_r(buffer, pp_str, p_size, 2);
	if (ret < 0)
		return -1;
	return 0;
}

//get a entire buffer stream which size field  takes 1 bytes
int get_buffer_stream_1(struct my_buffer *buffer, unsigned char** pp_str, int* p_size)
{
	int ret = 0;
	ret = get_buffer_stream_r(buffer, pp_str, p_size, 1);
	if (ret < 0)
		return -1;
	return 0;
}


static int buffer_seek2(struct my_buffer * buffer, int offset1)
{
	if (offset1 < 0) {
		buffer->offset = 0;
		return 0;
	}
	if (offset1 >= buffer->stream_len) {
		buffer->offset = buffer->stream_len;
		return 0;
	}
	buffer->offset = offset1;
	return 0;
}

int buffer_seek(struct my_buffer* buffer, enum BUFFER_SEEK pos, int offset1)
{
	int offset_after = 0;
	switch (pos) {
		case BUFSEEK_CUR:
			offset_after = buffer->offset + offset1;
			return buffer_seek2(buffer, offset_after);
		case BUFSEEK_START:
			offset_after = offset1;
			return buffer_seek2(buffer, offset_after);
		case BUFSEEK_END:
			offset_after = buffer->stream_len-1 + offset1;
			return buffer_seek2(buffer, offset_after);
	}
	return -3;
}

static int get_buffer_stream_withbuf(struct my_buffer* buffer, char* dest, int size_to_reads)
{
	int ret = 0;
	
	CK02 (ret = get_buffer_stream(buffer, (unsigned char*)dest, size_to_reads), err_no);
	return 0;
err_no:
	return ret;
}

static int check_dest_size(struct my_buffer* buffer, unsigned long long dest_size, unsigned long long size_to_read)
{
	if (size_to_read >= dest_size) {
		//DEBG (0, "[error_msg][length of string in buffer is too large, skip this][src_string_length=%d dest_buffer_size=%d]\n", size_to_read, dest_size);
		buffer_seek(buffer, BUFSEEK_CUR, size_to_read);
		return -1;
	}
	return 0;
}

//argument read_size is the streamlen read
//string_len  takes 1 byte, dest has malloc memory
int get_buffer_stream_withbuf_1(struct my_buffer* buffer, char* dest, int dest_size, int *size_read)
{
	int ret = 0;
	int size_to_read = 0;
	CK00 (ret = get_buffer_interger(buffer, &size_to_read, 1), err_no);
	CK00 (ret = check_dest_size(buffer, dest_size, size_to_read), err_no);
	CK00 (get_buffer_stream_withbuf(buffer, dest, size_to_read), err_no);
	if (size_read)
		*size_read = size_to_read;
	return 0;
err_no:
	if (size_read)
		*size_read = 0;
	return ret;
}

//string_len  takes 2 byte, dest has malloc memory
int get_buffer_stream_withbuf_2(struct my_buffer* buffer, char* dest, int dest_size, int *size_read)
{
	int ret = 0;
	int size_to_read = 0;
	CK00 (ret = get_buffer_interger(buffer, &size_to_read, 2), err_no);
	CK00 (ret = check_dest_size(buffer, dest_size, size_to_read), err_no);
	CK00 (get_buffer_stream_withbuf(buffer, dest, size_to_read), err_no);
	if (size_read)
		*size_read = size_to_read;
	return 0;
err_no:
	if (size_read)
		*size_read = 0;
	return ret;
}
//string_len  takes 4 byte, dest has malloc memory
int get_buffer_stream_withbuf_4(struct my_buffer* buffer, char* dest, int dest_size, int *size_read)
{
	int ret = 0;
	int size_to_read = 0;
	CK00 (ret = get_buffer_interger(buffer, &size_to_read, 4), err_no);
	CK00 (ret = check_dest_size(buffer, dest_size, size_to_read), err_no);
	CK00 (get_buffer_stream_withbuf(buffer, dest, size_to_read), err_no);
	if (size_read)
		*size_read = size_to_read;
	return 0;
err_no:
	if (size_read)
		*size_read = 0;
	return ret;
}
//string_len  takes 8 byte, dest has malloc memory
int get_buffer_stream_withbuf_8(struct my_buffer* buffer, char* dest, unsigned long long dest_size, unsigned long long *size_read)
{
	int ret = 0;
	int size_to_read = 0;
	CK00 (ret = get_buffer_interger(buffer, &size_to_read, 8), err_no);
	CK00 (ret = check_dest_size(buffer, dest_size, size_to_read), err_no);
	CK00 (get_buffer_stream_withbuf(buffer, dest, size_to_read), err_no);
	if (size_read)
		*size_read = size_to_read;
	return 0;
err_no:
	if (size_read)
		*size_read = 0;
	return ret;
}
