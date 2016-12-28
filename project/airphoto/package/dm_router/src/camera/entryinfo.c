#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "entryinfo.h"


int eil_init(struct EntryInfoList* list)
{
	memset(list, 0, sizeof(struct EntryInfoList) );
	LL_INIT(&list->link);
	return 0;
	
}

int eil_get_count(struct EntryInfoList* list)
{
	int count = 0;
	struct llhead* temp = NULL;
	LL_FOREACH(&list->link, temp) {
		count++;
	}
	return count;
}


struct EntryInfo* eil_get_element(struct EntryInfoList* list, int index)
{
	int count = 0;
	struct llhead* temp = NULL;
	LL_FOREACH(&list->link, temp) {
		if (count >=index)
			break;
		count++;
	}
	if (count < index)
		return NULL;
	struct EntryInfo* ei = LL_ENTRY(temp, struct EntryInfo, link);
	return ei;
}

struct EntryInfoList* eil_get_son_list(struct EntryInfo* entryinfo)
{
	return &entryinfo->son_list;
}

int eil_free_list(struct EntryInfoList* list)
{
	if (!list)
		return 0;
	struct EntryInfo* ei = NULL;
	struct llhead* tmp = NULL;
	struct llhead* p_link  = NULL;
	if (!list->link.next || !list->link.prev)
		return 0;
	LL_FOREACH_SAFE(&list->link, p_link, tmp) {
		ei = LL_ENTRY(p_link, struct EntryInfo, link);
		eil_free_list(&ei->son_list);
		LL_DEL(&ei->link);
		free(ei);
	}
	return 0;
}

int eil_addto_list(struct EntryInfoList* list, struct EntryInfo* ei)
{
	if (!list || !ei) {
		printf("argument is null\n");
		return -1;
	}
	LL_TAIL(&list->link, &ei->link);
	return 0;
}

struct EntryInfo* ei_new(char* path)
{
	struct EntryInfo* ei = NULL;
	ei = (struct EntryInfo*)malloc(sizeof(struct EntryInfo));
	if (!ei) {
		printf("ei_new failed, out of memory\n");
		return NULL;
	}
	memset(ei, 0, sizeof(struct EntryInfo));

	LL_INIT(&ei->link);
	strncpy(ei->path, path, sizeof(ei->path)-1);
	return ei;

}

struct EntryInfoList* eil_new()
{
	struct EntryInfoList* eil = NULL;
	eil = (struct EntryInfoList*)malloc(sizeof(struct EntryInfoList));
	if (!eil) {
		printf("eil_new failed, out of memory\n");
		return NULL;
	}
	memset(eil, 0, sizeof(struct EntryInfoList));
	
	eil_init(eil);
	return eil;

}

