#include "ghttp.h"
#include "http_hdrs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <io.h>
#include <unistd.h>

#define GET_ARRAY_LEN(array,len) {len = (sizeof(array) / sizeof(array[0]));}

int isFileExist(char * savePath) {
    if (!access(savePath, F_OK)) {
        return 1;
    } else {
        return 0;
    }
}

int download(char *uri, char *savePath) {
    ghttp_request *request = NULL;
    ghttp_status status;
    FILE * pFile;
    char *buf;
    int bytes_read;
//    int size;
//    if(!isFileExist(savePath))
//    {
//        printf("savePath not exist ");
//        return -1;
//    }
    pFile = fopen(savePath, "wb");
    request = ghttp_request_new();
    if (ghttp_set_uri(request, uri) == -1)
    {
        ghttp_request_destroy(request);
        return -1;
    }
    if (ghttp_set_type(request, ghttp_type_get) == -1)//get
    {
        ghttp_request_destroy(request);
        return -1;
    }
    ghttp_prepare(request);
    status = ghttp_process(request);
    if (status == ghttp_error)
    {
        ghttp_request_destroy(request);
        return -1;
    }
    printf("Status code -> %d\n", ghttp_status_code(request));
    buf = ghttp_get_body(request);
    bytes_read = ghttp_get_body_len(request);
//    size = (int)strlen(buf); //size == bytes_read
    fwrite(buf, bytes_read, 1, pFile);
    fclose(pFile);
    
    ghttp_request_destroy(request);
    return 0;
}
//result地址参数传递
/*
 return 0:succ
        -1:error
 */
int netGet(char* url, char* params, int timeout, char *result, int *result_len) {
    int ret = 0;
    ghttp_request *request = NULL;
    ghttp_status status;
    char *tempBuf;
    request = ghttp_request_new();
    
    if(params!=NULL&&strlen(params)>0)
    {
        char tmp[1024];
        strcpy(tmp,url);
        if(strchr(tmp, '?') == NULL)//url不存在
        {     strcat(tmp,"?")  ;
        }
        strcat(tmp,params) ;
        printf("%s\n",tmp);
        ghttp_set_uri(request, tmp);
    }else{
        ghttp_set_uri(request, url);
    }
    ghttp_set_type(request, ghttp_type_get); //get方法
    ghttp_set_header(request, http_hdr_Connection, "close");
    char timeout_str[10];
    sprintf(timeout_str, "%d", timeout);
    ghttp_set_header(request, http_hdr_Timeout, timeout_str);
    
    ghttp_prepare(request);
    status = ghttp_process(request);
    
    if(status == ghttp_error)
    {
        printf("ghttp_process error\n");
        ret = -1;
        goto EXIT;
    }
    printf("Status code -> %d\n",ghttp_status_code(request));
    tempBuf = ghttp_get_body(request);
    *result_len = ghttp_get_body_len(request);
    if(tempBuf)
    {
        strcpy(result, tempBuf);
    }
EXIT:
    ghttp_request_destroy(request);
    return ret;
}

int netPost(char* uri, char* params, int timeout, char *result, int *result_len) {
 //   char szVal[1024];
    ghttp_request *request = NULL;
    ghttp_status status;
    char *tempBuf;
    int len;
    printf("%s\n", params); //test
    request = ghttp_request_new();
    if (ghttp_set_uri(request, uri) == -1)
        return -1;
    if (ghttp_set_type(request, ghttp_type_post) == -1) //post
        return -1;
    ghttp_set_header(request, http_hdr_Content_Type,"application/x-www-form-urlencoded");
    
    char timeout_str[10];
    sprintf(timeout_str, "%d", timeout);
    ghttp_set_header(request, http_hdr_Timeout, timeout_str);
    //ghttp_set_sync(request, ghttp_sync); //set sync
    len = (int)strlen(params);
    ghttp_set_body(request, params, len); //
    ghttp_prepare(request);
    status = ghttp_process(request);
    if (status == ghttp_error)
        return -1;
    tempBuf = ghttp_get_body(request); //test
    *result_len=ghttp_get_body_len(request);
    if(tempBuf)
    {
        strcpy(result, tempBuf);
    }
    ghttp_clean(request);
    return 0;
}

int download_http_file(char* url , char* save_file_path  , int (*progress_chanage)(long progress,long total))
{
    ghttp_request*		request = NULL;
    ghttp_status		status;
    ghttp_current_status download_progress;
    char*				buf = 0;
    int					bytes_read=0 ;
//    int                 download_total_len = 0;
    FILE*				fp=NULL;
    
    fp = fopen( save_file_path , "wb");
    
    request = ghttp_request_new();
    if(ghttp_set_uri(request, url) == -1) {
        if (fp != NULL) {
            fclose(fp);
        }
        ghttp_request_destroy(request);
        return -1;
    }
    if(ghttp_set_type(request, ghttp_type_get) == -1) {
        if (fp != NULL) {
            fclose(fp);
        }
        ghttp_request_destroy(request); 
        return -1;
    }
    
    ghttp_set_sync(request, ghttp_async);
    ghttp_prepare(request);
    
    do{
        status = ghttp_process(request);
        if(status == ghttp_error)  {
            printf("download file failed\n");
            fclose(fp);
            ghttp_close(request);
            ghttp_request_destroy(request);
            return -1;
        }
        // A solution
        download_progress = ghttp_get_status(request);
        if ((progress_chanage != NULL) && (status == ghttp_not_done) && (download_progress.bytes_total > 0)) {
            if(progress_chanage(download_progress.bytes_read,download_progress.bytes_total))
            {
                fclose(fp);
                ghttp_close(request);
                ghttp_request_destroy(request);
                return -2;
            }
        }
//        if( download_percentage != 0 && file_length > 0 ) {
//            *download_percentage = (float)(download_progress.bytes_read * 100.0 / file_length);
//        }
    } while( status == ghttp_not_done );
    
    bytes_read = ghttp_get_body_len(request);
    buf = ghttp_get_body(request);
    
    fwrite( buf ,bytes_read , 1 , fp);
    fclose(fp);

    if (progress_chanage != NULL) {
        progress_chanage(bytes_read,bytes_read);
    }
//    *download_percentage = (float)(bytes_read * 100.0 / file_length);
    
    ghttp_close(request);
    ghttp_request_destroy(request);
    
    return 0;
}




int testGhttp()
{
//    char *uri = "http://www.hao123.com";
    char *uri = "http://x.dmsys.com/GetXml?customCode=A999(M.03)&versionCode=1.0.00.1&mac=&language=zh&time=120&versionflag=1";
    ghttp_request *request = NULL;
    ghttp_status status;
    char *buf;
    int bytes_read;
    
    request = ghttp_request_new();
    if(ghttp_set_uri(request, uri) == -1)
        exit(-1);
    if(ghttp_set_type(request, ghttp_type_get) == -1)
        exit(-1);
    ghttp_prepare(request);
    status = ghttp_process(request);
    if(status == ghttp_error)
        exit(-1);
    /* OK, done */
    printf("Status code -> %d\n", ghttp_status_code(request));
    buf = ghttp_get_body(request);
    bytes_read = ghttp_get_body_len(request);
    return 0;
}