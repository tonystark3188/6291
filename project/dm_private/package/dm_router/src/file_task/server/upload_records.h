#ifndef _UPLOAD_RECORDS_H_
#define _UPLOAD_RECORDS_H_

#include "my_debug.h"
#ifdef __cplusplus
extern "C"{
#endif

struct record_dnode
{
	int index;
	unsigned long start;
	unsigned long end;
	struct record_dnode *dn_next;
};
/*
*	add upload record  info to  list
*/
int add_record_to_list(struct record_dnode **dn,int index,unsigned long start,unsigned long end);
/*
*	 delete record info from list accorrding to client logout cmd 
*/
int del_record_from_list_for_index(struct record_dnode **head,unsigned long start);

/*
* destroy the upload record list
*/
int destory_record_list(struct record_dnode *dn);
 
void display_record_dnode(struct record_dnode *dn);
/*
* write the list record info to the path
*/
int write_list_to_file(const char *path,struct record_dnode *dn);
/*
* read the upload record frm the path
*/
int read_list_from_file(const char *path,struct record_dnode **dn);
int update_record_for_index(struct record_dnode **head,unsigned long end,unsigned long cur);



#ifdef __cplusplus
}
#endif


#endif

