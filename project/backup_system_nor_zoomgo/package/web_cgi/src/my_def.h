

#ifndef __MY_DEF_H__
#define __MY_DEF_H__

#if __cplusplus
extern "C" {
#endif

//#define DEBUG


typedef int 		        SINT32;
typedef unsigned int        UINT32;
typedef short int 	        SINT16;
typedef unsigned short int  UINT16;
typedef char 		        SINT8;
typedef unsigned char       UINT8;
typedef unsigned char       UBOOL8;

typedef long long 			int64_tt;
typedef unsigned long long 	uint64_tt;
typedef unsigned int 		uint32_tt;
typedef int 				int32_tt;
typedef unsigned short 		uint16_tt;
typedef short 				int16_tt;
// typedef char int8_t;
typedef unsigned char 		uint8_tt;

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#endif

#ifndef BIT
#define BIT(val, i) ((val) & (0x1 << (i)))
#endif


#define safe_free(p) do{\
	if((p) != NULL)\
	{\
		free((p));\
		(p) = NULL;\
	}\
	}while(0)

#define safe_fclose(f) do{\
	if((f) != NULL)\
	{\
		fclose((f));\
		(f) = NULL;\
	}\
	}while(0)

#define safe_close(fd) do{\
	if((fd) > 0)\
	{\
		close((fd));\
		(fd) = -1;\
	}\
	}while(0)

#ifdef DEBUG
#define d_printf(format, ...) \
	do { \
		printf("[%s@%s:%d] " format "\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
		} while (0)
#else
#define d_printf(format, ...) 
#endif

#define printf_info(format, ...) \
	do { \
		printf(format "\n",##__VA_ARGS__ ); \
		} while (0)



// #define d_printf(format, ...) 
// #define printf_info(format, ...)

#if __cplusplus
}
#endif

#endif /* __MY_DEF_H__ */

