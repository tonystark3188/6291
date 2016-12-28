#ifndef _UPLOAD_RECORDS_H_
#define _UPLOAD_RECORDS_H_

#include "my_debug.h"
#include "bfavfs.h"
#ifdef __cplusplus
extern "C"{
#endif

struct record_dnode
{
	int index;
	off_t start;
	off_t end;
	struct record_dnode *dn_next;
};
/*
*	add upload record  info to  list
*/
int add_record_to_list(struct record_dnode **dn,int index,off_t start,off_t end);
/*
*	 delete record info from list accorrding to client logout cmd 
*/
int del_record_from_list_for_index(struct record_dnode **head,off_t start);

/*
* destroy the upload record list
*/
int destory_record_list(struct record_dnode *dn);
 
void display_record_dnode(struct record_dnode *dn);

int write_list_to_stream(VFILE *record_fd,struct record_dnode *dn);//Ð´³öÊý¾Ý

/*
* read the upload record frm the path
*/
int read_list_from_file(const char *path,struct record_dnode **dn,void *token);
int update_record_for_index(struct record_dnode **head,off_t end,off_t cur);



#ifdef __cplusplus
}
#endif


#endif

