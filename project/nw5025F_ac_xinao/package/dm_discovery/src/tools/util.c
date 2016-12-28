#include<unistd.h>
#include<stdio.h>
#include<dirent.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

void* xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if(ptr == NULL && size != 0)
        printf("bb_msg_memory_exhausted\n");
    return ptr;
}

void* xzalloc(size_t size)
{
    void *ptr = xmalloc(size);
    memset(ptr,0,size);
    return ptr;
}





	

