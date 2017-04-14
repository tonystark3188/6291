#include <stdio.h>
#include <stdlib.h>
#include "random-string.h"


static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static int mod_table[] = {0, 2, 1};
/*
void reverse_string(char *ch)
{	
	int i=0;int k=0;
	while(*(ch+i)!='\0')//计算字符串长度
		++i;
	for(k=0;k<i/2;k++)
	{
	
		ch[k]=ch[k]^ch[i-k-1];//按位异或
		ch[i-k-1]=ch[k]^ch[i-k-1];
		ch[k]=ch[k]^ch[i-k-1];
	}
}*/
char *reverse_string(char *ch)   
{   
	if( !ch )   
	{   
		return NULL;
	}  
    int len = strlen(ch);//计算指定的字符串的长度, 不包括结束字符\0   
    if( len > 1 )   
    {   
        char ctemp =ch[0];   
        ch[0] = ch[len-1];      
        ch[len-1] = '\0';// 最后一个字符在下次递归时不再处理   
        reverse_string(ch+1); // 递归调用，每调用一次，要反转的字符串分别从头和末尾各减少一个   
        ch[len-1] = ctemp;   
    }   
    return ch;   
}

char *base64_encode(const unsigned char *data,
                    size_t input_length){
                    
	int i;
	int j;	
    int output_length = 4 * ((input_length + 2) / 3);
	printf("data1=%s\n",data);
	printf("output_length=%d\n",output_length);

    char *encoded_data = (char *) malloc(output_length);
	memset(encoded_data,0,output_length);
	
    if (encoded_data == NULL) return NULL;

    for (i = 0,j = 0; i < input_length;) {

        unsigned int octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        unsigned int octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        unsigned int octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = tolower(encoding_table[(triple >> 3 * 6) & 0x3F]);
        encoded_data[j++] = tolower(encoding_table[(triple >> 2 * 6) & 0x3F]);
        encoded_data[j++] = tolower(encoding_table[(triple >> 1 * 6) & 0x3F]);
        encoded_data[j++] = tolower(encoding_table[(triple >> 0 * 6) & 0x3F]);
    }

    for (i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = 'n';
	encoded_data[output_length-1]='\0';
	printf("encoded_data=%s\n",encoded_data);
	//reverse_string(encoded_data);
	printf("encoded_data2=%s\n",encoded_data);
    return encoded_data;
}


	

int randomString(char *result,int length) {
  int numOfChars = 62;
  char * characters = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int i = 0;
  //char* result = (char*)malloc(length);

  for (i = 0; i < length; i++) {
  	
    int j = rand() % numOfChars;
    result[i] = characters[j];
	printf("result=%c\n",result[i]);
  }

  return 0;
}

