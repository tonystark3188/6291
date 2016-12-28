#ifndef _GP_BUFFER_H
#define _GP_BUFFER_H

struct my_buffer {
	unsigned char* buf;//the reason respond stream to be here is that: make respsond stream in handle function, faster
	int buffer_size;
	int stream_len;
	int offset;
};
enum BUFFER_SEEK {
	BUFSEEK_CUR = 0,
	BUFSEEK_START = 1,
	BUFSEEK_END = 2,
};
int get_buffer_8(struct my_buffer* buffer, unsigned char *d);
int get_buffer_16(struct my_buffer* buffer, unsigned short *d);
int get_buffer_32(struct my_buffer* buffer, unsigned int *d);
int get_buffer_64(struct my_buffer* buffer, unsigned long long *d);
int get_buffer_stream(struct my_buffer* buffer, unsigned char* bytes, int size);
int put_buffer_64(struct my_buffer* buffer, unsigned long long p);
int put_buffer_16(struct my_buffer* buffer, unsigned short p);
int put_buffer_8(struct my_buffer* buffer, unsigned char p);
int put_buffer_32(struct my_buffer* buffer, unsigned int p);
int put_buffer_stream(struct my_buffer* buffer, unsigned char* minor_buf, int size);
int free_buffer(struct my_buffer *buffer);
int get_buffer_stream_8(struct my_buffer *buffer, unsigned char** pp_str, int* p_size);
int get_buffer_stream_4(struct my_buffer *buffer, unsigned char** pp_str, int* p_size);
int get_buffer_stream_2(struct my_buffer *buffer, unsigned char** pp_str, int* p_size);
int get_buffer_stream_1(struct my_buffer *buffer, unsigned char** pp_str, int* p_size);
int get_buffer_stream_withbuf_1(struct my_buffer* buffer, char* dest, int dest_size, int *size_read);
int get_buffer_stream_withbuf_2(struct my_buffer* buffer, char* dest, int dest_size, int *size_read);
int get_buffer_stream_withbuf_4(struct my_buffer* buffer, char* dest, int dest_size, int *read_size);
int get_buffer_stream_withbuf_8(struct my_buffer* buffer, char* dest, unsigned long long dest_size, unsigned long long *read_size);
int buffer_seek(struct my_buffer* buffer, enum BUFFER_SEEK pos, int offset1);
int buffer_expand_size(struct my_buffer* buffer, int size);

#endif

