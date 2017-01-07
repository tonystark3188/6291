#ifndef __LINKLIST_TOKEN__
#define __LINKLIST_TOKEN__

#define STR_LEN 128

#define TEST_MAIN 0
#define LINKLIST_LOCK_DM 1
#define L_LOG 1

#ifndef _int64_t
typedef long long _int64_t;
#endif

typedef _int64_t token_t;

enum{
	LINKLIST_UNINIT=0,
	LINKLIST_INIT_OK=1
};

typedef struct __myattr_type 
{  
	char info[STR_LEN]; 
	token_t token;
	void* value; //一个地址
}myattr_type;

typedef struct link  
{  
	myattr_type myvalue;  
	struct link *next;  
}linklist;  

typedef struct __token_list
{   
	linklist head; 
#if LINKLIST_LOCK_DM
	pthread_mutex_t mutex;	
#endif
	int init_flag;
}token_list; 


#define _DEBUG_LOG(args...) do{\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                fprintf(fp,"[TOKEN_LIST][%s][-%d] ", __FUNCTION__, __LINE__);\
                fprintf(fp,##args);\
                fprintf(fp,"\r\n");\
                fclose(fp);\
        }\
}while(0)


#if L_LOG
#define l_log _DEBUG_LOG
#else
#define l_log(args...)
#endif

#define l_log_info _DEBUG_LOG

#define l_log_err(args...) do{\
        FILE *fp = fopen("/dev/console","w");\
        if(fp){\
                fprintf(fp,"[TK_ERROR][%s][-%d] ", __FUNCTION__, __LINE__);\
                fprintf(fp,##args);\
                fprintf(fp,"\r\n");\
                fclose(fp);\
        }\
}while(0)
	
//#define  my_dbg(args...)   printf(args)
//#define  my_dbg(...)   printf(__VA_ARGS__)
//#define l_log(_fmt_, args...) printf("[%s-%d]"_fmt_"\r\n", __FUNCTION__, __LINE__, ##args)

void CreateListFront();		//测试创建单链表；
int InitLinklist(); 				//初始化
void ShowLinklist();			//打印链表；
void DeleteLaseNode();		//出栈， 删除头结点；
void ReleaseLinklist();		//删除链表
int AddLinkNode(myattr_type *data);		//入栈，头插入新节点；
int FindTokenByValue(void *value, token_t *token); // value 为一个结构体地址
int DeleteNodeByValue(void *value); // value 为一个结构体地址

#endif
