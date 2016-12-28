#ifndef _DB_QUERY_CACHE_H_
#define _DB_QUERY_CACHE_H_

#include "db/SqlOperation.h"
#include "base.h"

#define DB_QUERY_CACHE_MEM_SIZE   1024*1024  //1M

#define DB_QUERY_CACHE_NUM   32
#define CACHE_ITEM_NUM       (DB_QUERY_CACHE_MEM_SIZE/DB_QUERY_CACHE_NUM)

#define MAX_ITEM_NUM_PER_QUERY 50


typedef enum
{
    CACHE_DIR_ITEMS,
    CACHE_FILE_NAME,
    CACHE_FILE_TYPE,
    CACHE_FILE_FORMAT,
    CACHE_FILE_ACCESS_TIME,
    CACHE_FILE_SIZE,
    CACHE_NONE_CONTENT
}QueryCacheContent;


typedef struct
{
    uint32_t key;  
}CachedItem;


typedef struct
{
    bool cache_in_use;   //indicate whether the cache is in use
	QueryCacheContent cache_content;  //which type of query cmd the cache content is from
	uint32_t start_index;      //cached item start index
	uint32_t items_in_cache;   //total items in cache
	uint32_t total_items;      //total matched items in database
	uint32_t lazy_level;       //it provide a relative value to determine which cache is coldest.
	CachedItem cache_mem[CACHE_ITEM_NUM];  //cache memory for cached item
	file_info_t match;               //match condition
	DB_OprObj *pobj;   //which DB_OprObj the cache is serving now
}DB_QueryCache;


typedef struct
{
    DB_QueryCache cache_set[DB_QUERY_CACHE_NUM];
	int candidate;
}DB_QueryCacheSet;


bool fetch_items_from_cache(QueryCacheContent content, DB_OprObj *pobj);
void free_cache(DB_QueryCache *pcache);
void clean_cacheset();



#endif
