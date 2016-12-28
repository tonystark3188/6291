#include <stdlib.h>
#include <string.h>
#include "md5.h"
#include "dm_encrypt.h"
#define	g_len_encrypt_src	32

//	加密用的干扰串
const unsigned char g_enc_noise[g_len_encrypt_src]	= {
	0x50,0x8B,0x4D,0x08,0x51,0xFF,0x15,0x8C,
	0x33,0x42,0x00,0x83,0xC4,0x0C,0x3B,0xF4,
	0xE8,0xB5,0x61,0xFF,0xFF,0x8D,0x45,0xDC,
	0x50,0xE8,0xF3,0x60,0xFF,0xFF,0x83,0xC4
};


char num_to_hex(short num)
{
	char	hex	= 0;
	if (0 <= num && 9 >= num)
	{
		hex	= num + '0';
	}
	else if (9 < num && 16 > num)
	{
		hex	= num + 'A' -10;
	}

	return	hex;
}

/* md5加密，长度32字节, 结果保存在out，out大小应当超过32字节 */
void dm_md5(_In_ const void *src, int src_len, _Out_ char *out)
{
	yasm_md5_context context;
	unsigned char checksum[16]	= {0};
	char buffer[32]	= {0};
	int	i	=0;

	yasm_md5_init (&context);
	yasm_md5_update (&context, (const unsigned char*)src, src_len);
	yasm_md5_final (checksum, &context);
	for (i = 0; i < 16; i++)
	{
		for (i = 0; i < 16; i++)
		{
			buffer[2*i]	= num_to_hex(checksum[i] >> 4);
			buffer[2*i +1]	= num_to_hex(checksum[i] & 0xf);
		}
	}

	memcpy(out, buffer, sizeof(buffer));
}

/* 生成utoken ，长度32字节
结果保存在utoken，大小应该超过32字节，注意没有0结尾
*/
void dm_gen_utoken(_Out_ char* utoken, _In_ const char *uin)
{
	char src[g_len_encrypt_src]	= {0};
	char buffer[32]		= {0};

	int	i;

	/* 1, uin 作为源 */
	strncpy(src, uin, g_len_encrypt_src);

	/* 2, 用干扰串异或 */
	for (i=0; i<g_len_encrypt_src; ++i)
	{
		src[i]	= src[i] ^ g_enc_noise[i];
	}

	/* 3，MD5 */
	dm_md5(src, g_len_encrypt_src, buffer);
	memcpy(utoken, buffer, g_len_encrypt_src);
}

/* 生成session ，长度16字节
结果保存在session，大小应该超过17字节，有0结尾
*/
void _dm_gen_session(_Out_ char* session_id, _In_ const char *uin, _In_ const char *ip, _In_ int time)
{
	char src[g_len_encrypt_src]	= {0};
	char buffer[36]		= {0};

	int	noise_start;
	int	i;

	/* 1, ip + uin 作为源 */
	strncpy(src, ip, g_len_encrypt_src);
	strncat(src, uin, g_len_encrypt_src);

	/* 2, 根据时间取干扰串起始位置 */
	noise_start	= time %g_len_encrypt_src;

	/* 3, 用干扰串异或 */
	//for (i=noise_start; i<g_len_encrypt_src +noise_start; ++i)
	for (i=0; i<g_len_encrypt_src; ++i)
	{
		if (g_len_encrypt_src > i +noise_start)
		{
			src[i]	= src[i] ^ g_enc_noise[i+noise_start];
		}
		else
		{
			src[i]	= src[i] ^ g_enc_noise[i +noise_start -g_len_encrypt_src];
		}
	}

	/*	4，MD5 */
	dm_md5(src, g_len_encrypt_src, buffer);

	/*  5, 换成 16位的MD5 */
	memcpy(session_id, buffer+8, 16);
	session_id[16]	= 0;
}

void _dm_gen_uuid(_Out_ char* uuid, _In_ const char *pid, _In_ const char *vid, _In_ unsigned long total_size,_In_ unsigned long free_size)
{
	char src[g_len_encrypt_src]	= {0};
	char buffer[36]		= {0};

	int	noise_start;
	int	i;

	/* 1, ip + uin 作为源 */
	strncpy(src, pid, g_len_encrypt_src);
	strncat(src, vid, g_len_encrypt_src);

	/* 2, 根据时间取干扰串起始位置 */
	noise_start	= total_size %g_len_encrypt_src;

	/* 3, 用干扰串异或 */
	//for (i=noise_start; i<g_len_encrypt_src +noise_start; ++i)
	for (i=0; i<g_len_encrypt_src; ++i)
	{
		if (g_len_encrypt_src > i +noise_start)
		{
			src[i]	= src[i] ^ g_enc_noise[i+noise_start];
		}
		else
		{
			src[i]	= src[i] ^ g_enc_noise[i +noise_start -g_len_encrypt_src];
		}
	}

	/*	4，MD5 */
	dm_md5(src, g_len_encrypt_src, buffer);

	/*  5, 换成 16位的MD5 */
	memcpy(uuid, buffer+8, 16);
	uuid[8]	= 0;
}

int _dm_put_session(_In_ char* session_id,_In_ const char* username)
{
	return 0;
}

int dm_check_session_cmd(_In_ char* sessionid, _In_ int cmd)
{
	return 0;
}
	

