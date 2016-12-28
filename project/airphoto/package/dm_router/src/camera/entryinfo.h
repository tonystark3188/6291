#ifndef __ENTRYINFO_H
#define __ENTRYINFO_H

#include "llist.h"

struct EntryInfoList {
	struct llhead link; //file chain	
};

struct EntryInfo {
	char type[32];
	char path[512];
	long long time;
	long long size;
	int total_num;	//valid only when type is folder, number of photos
	
	struct llhead link; //file chain

	struct EntryInfoList son_list;//valid only when type is folder, list of first 4 pictures 
	
};

#define eil_safe_free(p) do{\
	if((p) != NULL)\
	{\
		eil_free_list((p)); \
		free((p));\
		(p) = NULL;\
	}\
	}while(0)

int eil_init(struct EntryInfoList* list);
int eil_get_count(struct EntryInfoList* list);
struct EntryInfo* eil_get_element(struct EntryInfoList* list, int index);
struct EntryInfoList* eil_get_son_list(struct EntryInfo* entryinfo);
int eil_free_list(struct EntryInfoList* list);
int eil_addto_list(struct EntryInfoList* list, struct EntryInfo* ei);
struct EntryInfo* ei_new(char* path);
struct EntryInfoList* eil_new();

#endif
