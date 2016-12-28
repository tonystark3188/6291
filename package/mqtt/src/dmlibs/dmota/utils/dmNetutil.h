/*
 * File:   netutil.h
 * Author: Administrator
 *
 * Created on 2014年9月2日, 下午3:51
 */

#ifndef DMNETUTIL_H
#define	DMNETUTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
extern    int isFileExist(char * savePath);
extern    int download(char *uri, char *savePath) ;
    //result地址参数传递
extern    int netGet(char* url, char* params, int timeout, char *result, int *result_len) ;
extern    int netPost(char* uri, char* params, int timeout, char *result, int *result_len) ;
    /**
     *  http下载到文件，数据会先缓存到内存，下载完成后一次性写入到文件，建议小文件才使用该方法，如果是大文件不见已采用
     *
     *  @param url              下载的文件的url
     *  @param save_file_path   本地保存的绝对路径
     *  @param progress_chanage 当前进度的毁掉，返回非0时下载程序中断。
     *
     *  @return 0为成功，非0异常
     */
extern    int download_http_file(char* url , char* save_file_path  , int (*progress_chanage)(long progress,long total));
#ifdef	__cplusplus
}
#endif

#endif	/* NETUTIL_H */
