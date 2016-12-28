#ifndef __RECORD_LIST_H
#define __RECORD_LIST_H
#include "common.h"
typedef struct RECORD_LIST_{
//    int index;
    int datalen;
	unsigned char buff[DEFAULT_BUFF_SIZE];
	struct RECORD_LIST_ *next;
}RECORD_LIST;

int createRecordList(int nodenum);
void freeRecordList(void);

RECORD_LIST *getRecordNode(void);
RECORD_LIST *getEncordNode(void);

#endif
