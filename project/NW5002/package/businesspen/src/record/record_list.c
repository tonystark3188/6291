#include "headers.h"
#include "record_list.h"

static RECORD_LIST *g_record_list = NULL;
static int g_record_index = 0;
static int g_encord_index = 0;


RECORD_LIST *getRecordNode(void)
{
    RECORD_LIST *temp = NULL;

	temp = g_record_list+g_record_index;
	printf("%s:g_record_index=%d,g_encord_index=%d\n",__FUNCTION__,g_record_index,g_encord_index);
    if(temp->datalen)
    {
        PRINTFERROR("no space node to record data,wait encode\n");
		return NULL;
	}/**/
	printf("111 temp->datalen=%d\n\n",temp->datalen);
	g_record_index = (g_record_index+1)%(MAXRECORDNODE);
    return temp;
}


RECORD_LIST *getEncordNode(void)
{
    RECORD_LIST *temp = NULL;

	temp = g_record_list+g_encord_index;
	printf("%s:g_record_index=%d,g_encord_index=%d\n",__FUNCTION__,g_record_index,g_encord_index);
    if(temp->datalen == 0)
    {
        PRINTFERROR("no space node to encord data,wait recode\n");
		return NULL;
	}/**/
	printf("222 temp->datalen=%d\n\n",temp->datalen);
	g_encord_index = (g_encord_index+1)%(MAXRECORDNODE);
    return temp;
}


int createRecordList(int nodenum)
{
	RECORD_LIST *listtemp = NULL;	  
	RECORD_LIST *header = NULL;	 
	int i = 0;	  
	
	header = listtemp = (RECORD_LIST *)malloc(sizeof(RECORD_LIST));	
	if(NULL == listtemp)	
	{
	    printf("NULL == listtemp in %s\n",__FUNCTION__);
		return RETERROR;
	}
	else
	{
	    for(i = 0; i < nodenum; i++)		
		{
		    //printf(">>>>>>>>>>>>>>>>>>>>>>%d\n",i);
		    //listtemp->index = i;
//			listtemp->datalen = 0;
		    listtemp->next = (RECORD_LIST *)malloc(sizeof(RECORD_LIST));		  
		    listtemp = listtemp->next;

		    if(NULL == listtemp)		   
			{
			    printf("malloc space failed in %s\n",__FUNCTION__);   
		        return RETERROR;			  
			} 
		    listtemp->next = NULL;
		}
	}  
	freeRecordList();	  
	g_record_list = header;
	while(header)
	{//printf("----\n");
	    header->datalen = 0;
		memset(header->buff,0x0,DEFAULT_BUFF_SIZE);
		header = header->next;
	}
	printf("leave createRecordList\n");
	return RETSUCCESS;
}


void freeRecordList(void)
{
	RECORD_LIST *listtempheader = g_record_list;	
	RECORD_LIST *listtemp = g_record_list;    

	while(listtemp)	  
	{ 		 
	    listtempheader = listtemp->next;		
		free(listtemp); 	 
		listtemp = listtempheader;	
	}	 
	g_record_list = NULL;	  
}

