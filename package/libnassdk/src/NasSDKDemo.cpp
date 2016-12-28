#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "TXNasSDK.h"
#include "string.h"
#include <dirent.h>
#include <sys/stat.h>
#include <json-c/json.h>
#include <fcntl.h>
#define c_Print_Ctrl_Off        "\033[0m"
#define c_CharColor_Red         "\033[1;31m"



//////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//示例代码，实现了一个文件列表的处理
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

void create_failed_response(int ret, const char* err_msg, tx_data_point* req_data_point, tx_data_point* rsp_data_point)
{
	if(rsp_data_point == NULL) { //如果为空，那么就不做任何处理
		return;
	}
	rsp_data_point->id = req_data_point->id;
	rsp_data_point->seq = req_data_point->seq;
	rsp_data_point->ret_code = ret;

	//value
	char buf[256] = {0};
	sprintf(buf, "{\"ret\":%d,\"msg\":\"%s\"}", ret, err_msg); //返回Json格式的数据

	int buf_len = strlen(buf);
	rsp_data_point->value = new char[buf_len+1];     
	memset(rsp_data_point->value,0,buf_len+1);
	memcpy(rsp_data_point->value, buf, buf_len);
}

void create_success_response(const char* value, tx_data_point* req_data_point, tx_data_point* rsp_data_point)
{
	//对于来自httpserver的请求，生成响应
	if(rsp_data_point == NULL) { //如果为空，那么就不做任何处理
		return;
	}
	rsp_data_point->id = req_data_point->id;
	rsp_data_point->seq = req_data_point->seq;
	rsp_data_point->ret_code = 0;

	//value
	int len = strlen(value);
	rsp_data_point->value = new char[len+1];
	memset(rsp_data_point->value,0,len+1);
	memcpy(rsp_data_point->value, value, len);
}
// 示例代码,结束
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char g_root[] = "/tmp/mnt/USB-disk-1";
int  g_isLogin = 1;    //0 表示已登录，1表示未登录

int filekey2abspath(unsigned long long from_client, const char* filekey, char* abspath, int len)
{
	if(filekey == NULL ) return -1;
	
	snprintf(abspath,len,"%s%s",g_root,filekey);
	//printf("abspath is %s",abspath);
	return 0;
}

int get_path(char* buf,char *path)
{
	char tmp_path[256];
	int i=0;
	char *path2=strstr(buf,"path");
	if(path != NULL)
	{
		path2=path2+7;
		while(*(path2+i) != '"')
		{
			tmp_path[i]=*(path2+i);
			
			printf("tmp_path[i]=%c\n",tmp_path[i]);
			i++;
		}
		tmp_path[i]='\0';
		sprintf(path,"%s",tmp_path);
		//path=&tmp_path[0];
		return 0;
	}
	else {
		
		return -1;
	}


}
int get_file_ext(char * filename,char* format){
	char *ptr;
	char ext[16]="\0";
	int i=1;
	ptr=strrchr(filename,'.');
	if(ptr != NULL ){
		//printf("ptr=%c\n",*ptr);
		while((*(ptr+i) != '\0')&&(i<16)){
			ext[i-1]=*(ptr+i);
			//printf("ext[%d]=%c\n",i-1,ext[i-1]);
			i++;
		}
	}
	ext[i]='\0';
	//printf("ext[]=%s\n",ext);
	sprintf(format,"%s",ext);
	//printf(".ext=%s\n",format);
}
int get_file_type(char* format)
{
	char lower_format[32]="\0";
	int i;
	for(i=0;i<strlen(format);i++){
		lower_format[i]=tolower(format[i]);
	}
	static const char* pic_type[] = {
		"ico","tif","tiff","ufo","raw","arw","srf",
		"sr2","dcr","k25","kdc","cr2","crw","nef",
		"mrw","ptx","pef","raf","3fr","erf","mef",
		"mos","orf","rw2","dng","x3f","jpg","jpeg",
		"png","gif","bmp",
	};
	static const char* doc_type[] = {
		"docx","wri","rtf","xla","xlb","xlc","xld",
		"xlk","xll","xlm","xlt","xlv","xlw","xlsx",
		"xlsm","xlsb","xltm","xlam","pptx","pps",
		"ppsx","pdf","txt","doc","xls","ppt",
	};
	static const char* video_type[] = {
		"3gp","3g2","asf","dat","divx","dvr-ms","m2t",
		"m2ts","m4v","mkv","mp4","mts","mov","qt","tp",
		"trp","ts","vob","wmv","xvid","ac3","rm",
		"rmvb","ifo","mpeg","mpg","mpe","m1v","m2v",
		"mpeg1","mpeg2","mpeg4","ogv","webm","flv","avi",
		"swf","f4v",
	};
	static const char* audio_type[] = {
		"aac","flac","m4a","m4b","aif","ogg","pcm",
		"wav","cda","mid","mp2","mka","mpc","ape",
		"ra","ac3","dts","wma","mp3","amr",
	};

	for (size_t i = 0; i < sizeof(pic_type)/sizeof(char*); ++i)
	{
		if(strcmp(pic_type[i], lower_format) == 0)
		{
			return file_type_pic;
		}
	}
	for (size_t i = 0; i < sizeof(doc_type)/sizeof(char*); ++i)
	{
		if(strcmp(doc_type[i], lower_format) == 0)
		{
			return file_type_doc;
		}
	}
	for (size_t i = 0; i < sizeof(video_type)/sizeof(char*); ++i)
	{
		if(strcmp(video_type[i], lower_format) == 0)
		{
			return file_type_video;
		}
	}	
	for (size_t i = 0; i < sizeof(audio_type)/sizeof(char*); ++i)
	{
		if(strcmp(audio_type[i], lower_format) == 0)
		{
			return file_type_audio;
		}
	}
	return file_type_file;
}
int filter_dot(const struct dirent *ent)
{
	//if(ent->d_type != DT_REG)
		//return 0;
	return (ent->d_name[0] != '.');
}
#define MAX_FILE_NAME_LEN 256
//返回文件列表
int process_datapoint_file_list(unsigned long long from_client, tx_data_point *req_data_point, tx_data_point* rsp_data_point)
{
	printf("[process_datapoint_file_list]: 返回文件列表\n");
	int len=strlen(req_data_point->value);
	int page=0;
	int count=20;
	char *file_path= new char[MAX_FILE_NAME_LEN];
	memset(file_path,0,MAX_FILE_NAME_LEN);
	//memcpy(file_path,req_data_point->value, len);
	json_object *new_obj,*medi_array_obj_name;
	new_obj = json_tokener_parse(req_data_point->value);
	medi_array_obj_name = json_object_object_get(new_obj, "path");
	strcpy(file_path,json_object_get_string(medi_array_obj_name));
	
	medi_array_obj_name = json_object_object_get(new_obj, "page");
	page = json_object_get_int(medi_array_obj_name);
	
	medi_array_obj_name = json_object_object_get(new_obj, "count");
	count = json_object_get_int(medi_array_obj_name);
	
	
	//get_path(req_data_point->value,file_path);
	printf("filepath,page====%s,%d\n",file_path,page);	
	printf("req_data_point.value====%s",req_data_point->value);

	DIR *dp;
	struct dirent *dirp;
	struct stat statbuf;
	char abspath[512]="\0";
	char real_file_path[512]="\0";
	char tmpvalue[512]="\0";
	int size=0;
	
	filekey2abspath(from_client,file_path,abspath,512);
	printf("abspath=%s\n",abspath);
	if((dp = opendir(abspath)) == NULL)
	{
		printf("Can't open %s\n", abspath);	
		create_failed_response(1,"cant open abspath",req_data_point,rsp_data_point);
		return 0;
	}
	closedir(dp);
	//sort dir
	struct dirent **namelist;
	int i=0;
	int k=0;
	int total = scandir(abspath, &namelist, filter_dot, alphasort);
	if(total < 0)
        perror("scandir");
    else
    {
     //   for(i = 0; i < total; i++)
     //   printf("dir------%s\n", namelist[i]->d_name);
     //   printf("total = %d\n", total);
    }	
	char value[4096]="\0";
	//char value2[20480]="\0";
	char format[16]="\0";
	sprintf(value,"{\"r\": 0,\"p\": \"%s\",\"fs\":[",file_path);//size need to reconfirm.
	int j=0;
	int dir_num=0;
	int file_num=0;
	printf("total = %d\n", total);
	char **filelist;
	char **tmpfilelist;
	filelist =(char**) malloc(sizeof(char *)*total);
	tmpfilelist =(char**) malloc(sizeof(char *)*total);
	for(i = 0; i < total; i++)
	{
		filelist[i]=(char*)malloc(sizeof(char)*MAX_FILE_NAME_LEN);
		tmpfilelist[i]=(char*)malloc(sizeof(char)*MAX_FILE_NAME_LEN);
		
	}
	for(i = 0; i < total; i++){
			//sprintf(real_file_path,"%s%s",abspath,namelist[i]->d_name);
			//stat(real_file_path,&statbuf);
			//printf("i=%d,real=%s\n",i,real_file_path);
			if(namelist[i]->d_type == 4)//is a dir
			{
				strcpy(filelist[j],namelist[i]->d_name);
				j++;
			}else{//is a file
				strcpy(tmpfilelist[k],namelist[i]->d_name);
				k++;
			}
	}
	dir_num=j;
	file_num=k;
	for(i=0;i<k;i++){
		strcpy(filelist[j],tmpfilelist[i]);
		j++;
	}
	//printf(filelist[i]);
	for(i = (page*count),j=0,k=0; i < total; i++){
			sprintf(real_file_path,"%s/%s",abspath,filelist[i]);
			stat(real_file_path,&statbuf);
			printf("i=%d,real=%s\n",i,real_file_path);
			if((i) < (dir_num))//is a dir
			{
				if(j == count) break;//
				else{
					if(j==0)sprintf(tmpvalue,"{\"n\":\"%s\",\"t\":\"%d\",\"mt\":\"%ld\",\"tl\":3}",filelist[i],6,statbuf.st_mtime);
					else sprintf(tmpvalue,",{\"n\":\"%s\",\"t\":\"%d\",\"mt\":\"%ld\",\"tl\":3}",filelist[i],6,statbuf.st_mtime);
					
					strcat(value,tmpvalue);
					j++;//number of directory
				}
			}else //is a file
			{
				if((j+k) == count){printf("-------------break---------------\n");break;} 			
				get_file_ext(filelist[i],format);
				int file_type=get_file_type(format);
				
				if((j==0)&&(k==0))sprintf(tmpvalue,"{\"n\":\"%s\",\"t\":\"%d\",\"s\":\"%d\",\"mt\":\"%ld\",\"tl\":3}",filelist[i],file_type,statbuf.st_size,statbuf.st_mtime);
				else
				sprintf(tmpvalue,",{\"n\":\"%s\",\"t\":\"%d\",\"s\":\"%d\",\"mt\":\"%ld\",\"tl\":3}",filelist[i],file_type,statbuf.st_size,statbuf.st_mtime);
				
				strcat(value,tmpvalue);
				k++;//number of file	
			}
			size=size+strlen(tmpvalue);
			{printf("-------------k=%d---------------\n",k);} 
	}
	printf("size=%ld\n",size);
	strcat(value,"],\"f\":1}");
	printf("value====%s,len+++[[[[[[%ld]]]]]\n",value,strlen(value));
	/*
	//while((dirp = readdir(dp)) != NULL)
	for(i = 0; i < total; i++)
	{
		//printf("%s\n", dirp->d_name);
		//if((dirp = readdir(dp)) != NULL)
		//if(namelist[i]->d_name[0]=='.')continue;
		sprintf(real_file_path,"%s%s",abspath,namelist[i]->d_name);
		stat(real_file_path,&statbuf);
		printf("i=%d,real=%s\n",i,real_file_path);
		//if((i/10)==page)
		{
		if(namelist[i+(page*count)]->d_type == 8)  //is a file 
			{
			
			if((j+k) == count) break;			
			get_file_ext(namelist[i+(page*count)]->d_name,format);
			int file_type=get_file_type(format);
		
			sprintf(tmpvalue,",{\"n\":\"%s\",\"t\":\"%d\",\"s\":\"%d\",\"mt\":\"%ld\",\"tl\":3}",namelist[i+(page*count)]->d_name,file_type,statbuf.st_size,statbuf.st_mtime);
			strcat(value2,tmpvalue);
			k++;//number of file	
		}
		else if(namelist[i+(page*count)]->d_type == 4)//is a directory
		{
			if(j == count) break;//
			else{
				if(j==0)sprintf(tmpvalue,"{\"n\":\"%s\",\"t\":\"%d\",\"mt\":\"%ld\",\"tl\":3}",namelist[i+(page*count)]->d_name,6,statbuf.st_mtime);
				else sprintf(tmpvalue,",{\"n\":\"%s\",\"t\":\"%d\",\"mt\":\"%ld\",\"tl\":3}",namelist[i+(page*count)]->d_name,6,statbuf.st_mtime);
				strcat(value,tmpvalue);
				j++;//number of directory
			}
		}
		size=size+strlen(tmpvalue);
		}
			
		
	}
	*/
	
	//strcat(value,value2);//dirs/files
	//strcat(value,"],\"f\":1}");
	//printf("value====%s,len+++[[[[[[%ld]]]]]\n",value,strlen(value));
	for(i = 0; i < total; i++)
	{
		free(namelist[i]);
		free(filelist[i]);
		free(tmpfilelist[i]);
	}
	free(namelist);
	free(filelist);
	free(tmpfilelist);
	//按照格式将文件列表返回（该格式可以自定义，只要返回给H5页面后页面能解析就ok）
	//这是一个返回示例，列表包含一个文件夹“视频”("t": 6) 和一个文件 "test.txt"("t":5)
	//char value[] = "{\"r\": 0,\"p\": \"/\",\"fs\":[{\"n\": \"视频\",\"t\": 6,\"mt\":1438845878,\"tl\": 3},{\"n\": \"test.txt\",\"t\": 5,\"s\": 6893883,\"mt\": 1438911842,\"tl\": 3},{\"n\": \"Honor.mp3\",\"t\": 7,\"s\": 11290456,\"mt\": 11290456,\"tl\": 3}],\"f\":1}";
	create_success_response(value, req_data_point, rsp_data_point);
	return 0;
}
int translate(const char *file_src, char* file_dst)
{
	int i,j;
	for(i=0,j=0;i<strlen(file_src);i++)
	{
		/*switch(file_src[i]){
			case ' ':
			case '\'':
			case '$':
			//case 0x60：//compile error
			case '#':
			case '%':
			case '&':
			case '(':
			case ')':
			case '{':
			case '}':
			case ';':
				file_dst[j++]='\\';
				file_dst[j++]=file_src[i];
				break;
			default:
				file_dst[j++]=file_src[i];
				break;
		}*/
		if(file_src[i] == ' '||file_src[i] == '\''||file_src[i] == '$'||file_src[i] == '`'||file_src[i] == '#'||file_src[i] == '%'||file_src[i] == '&'||file_src[i] == '('||file_src[i] == ')'||file_src[i] == '{'||file_src[i] == '}'||file_src[i] == ';')
		{
			file_dst[j++]='\\';
			file_dst[j++]=file_src[i];
		}
		else{
			file_dst[j++]=file_src[i];
		}
	}
}

//删除文件
int process_datapoint_file_del(unsigned long long from_client, tx_data_point *req_data_point, tx_data_point* rsp_data_point)
{
	json_object *new_obj, *files_array, *medi_array_obj, *medi_array_obj_name; 
	int i,j=0;
	char real_file_path[1024]="\0";
	char translate_file_path[1024]="\0";
	char file_path[512]="\0";
	new_obj = json_tokener_parse(req_data_point->value);
	files_array = json_object_object_get(new_obj, "files");

	int arraylen = json_object_array_length(files_array);
	char value[10240]="{\"ret\":-9998,\"failed_files\":[";
	char tmpvalue[512]="\0";
	int fd=0;
	for (i = 0; i < arraylen; i++) {
		// get the i-th object in files_array
		medi_array_obj = json_object_array_get_idx(files_array, i);
		// get the name attribute in the i-th object
		medi_array_obj_name = json_object_object_get(medi_array_obj, "path");
		// print out the name attribute
		strcpy(file_path,json_object_get_string(medi_array_obj_name));
		printf("path=%s\n", file_path);
		translate(file_path,translate_file_path);
		printf("ttttpath=%s\n", translate_file_path);
		sprintf(real_file_path,"rm -rf %s%s",g_root,translate_file_path);
		printf("real_file_path=%s\n",real_file_path);
		system(real_file_path);
		fd=access(real_file_path,0);
		
		//if( remove(real_file_path) == 0 )
		if(fd<0){	
			j++;
			printf("Remove file %s OK \n",real_file_path);
		}
		else{
			printf("Remove file %s fail \n",real_file_path);
			//sprintf(value,"{\"ret\":-9998,failed_files[
			if(i==j){
				sprintf(tmpvalue,"{\"path\":\"%s\"}",file_path);
				strcat(value,tmpvalue);
			}
			else{
				sprintf(tmpvalue,",{\"path\":\"%s\"}",file_path);
				strcat(value,tmpvalue);				
			}
			perror("remove");
			
		}
	}
	if(j == arraylen){
		sprintf(value,"%s","{\"ret\":\"0\"}");
	}
	else {
		strcat(value,"]}");
	}
	printf("value=%s\n",value);
	create_success_response(value, req_data_point, rsp_data_point);
}
void send_dp_callback(unsigned int cookie, unsigned long long from_client, int err_code)
{
	printf("send_dp_callback cookie[%u] from_client [%llu]err_code[%d] \n",cookie,from_client, err_code);
}


/**
 * 收到data_point数据回调
 */
void on_receive_data_point(unsigned long long from_client, tx_data_point * data_points, int data_points_count)
{
	printf("receive data point from client=%lld count=%d\n", from_client,data_points_count);
	for (int i=0; i<data_points_count; ++i)
	{
		//根据id区分并处理不同的datapoint数据包
		printf("id====%d\n",data_points[i].id);
		printf("value====%s\n",data_points[i].value);
		switch(data_points[i].id)
		{
		case 100000110:	//返回文件列表
			{  
				unsigned int cookie = 0;
				tx_data_point dp_to_send = {0};
				process_datapoint_file_list(from_client,&data_points[i], &dp_to_send);            //处理datapoint
				tx_ack_data_points(from_client,&dp_to_send,1,&cookie, send_dp_callback);      //注意该函数的调用频率限制，每秒最多1次
				if (dp_to_send.value) delete dp_to_send.value;                                     //释放内存
			}
			break;
		case 100000112:	//删除文件
			{
				unsigned int cookie = 0;
				tx_data_point dp_to_send = {0};
				process_datapoint_file_del(from_client,&data_points[i], &dp_to_send);            //处理datapoint
				tx_ack_data_points(from_client,&dp_to_send,1,&cookie, send_dp_callback);      //注意该函数的调用频率限制，每秒最多1次
				if (dp_to_send.value) delete dp_to_send.value;                                     //释放内存
			}
			break;
		default:               //未识别的datapoint数据包
			{		
				int err_code = -1;                                                                                              //错误码
				unsigned int cookie = 0;
				tx_data_point dp_to_send = {0};
				create_failed_response(err_code,"非法的消息", &data_points[i], &dp_to_send);   //生成一个"非法的消息"的响应datapoint
				tx_ack_data_points(from_client,&dp_to_send,1,&cookie, send_dp_callback);         //注意该函数的调用频率限制，每秒最多1次
				if (dp_to_send.value) delete dp_to_send.value;                                        //释放内存
			}
		}
	}
}



int copyfile(const char* src, const char* dst)
{
	if(src == NULL || dst == NULL) return -1;

	FILE *fpSrc, *fpDst;

	fpSrc = fopen(src,"rb");
	if(fpSrc == NULL){
		return -1;
	}

	fpDst = fopen(dst,"wb");
	if(fpDst == NULL){
		fclose(fpSrc);
		return -1;
	}

	int b;
	while(1){
		b = fgetc(fpSrc);
		if(!feof(fpSrc)){
			fputc(b,fpDst);
		}else{
			break;
		}
	}

	fclose(fpSrc);
	fclose(fpDst);

	return 0;
}

int notify_file_download_complete(unsigned long long from_client, const char* tmp_file_path, int file_type, const char* specified_dst_dir)
{
	if(tmp_file_path == NULL) return -1;

	int ret = 0;

	char dst_dir[512] = {0};

	//如果spcified_dst_dir 为NULL，用户可以通过file_type来自定义文件保存位置，这里为演示，统一放到当前目录
	if(specified_dst_dir == NULL){
		snprintf(dst_dir,512,"/tmp/mnt/USB-disk-1");
	}else{
		snprintf(dst_dir,512,"%s/%s",g_root,specified_dst_dir);           //存放到specified_dst_dir 目录下                                
	}
	int fd;
	fd=access(dst_dir,0);
	if(fd<0){		
		mkdir(dst_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
	}
	
	const char *filename = strrchr(tmp_file_path,'/');

	char dst_file[1024] = {0};
	snprintf(dst_file,1024,"%s%s",dst_dir,filename);

	char src_file[1024] = {0};
	snprintf(src_file,1024,"%s",tmp_file_path);

	printf("[notify_file_download_complete] src_file=%s  dst_file =%s\n",src_file, dst_file);	

	//复制或移动文件
	ret = copyfile(src_file,dst_file);
	if(ret){
		printf("copy file failed! code[%d]\n",ret);
		return ret;
	}
	//无论成功或失败，都删除下载的备份
	if( remove(src_file) == 0 )
	{	
		printf("Remove tmp file %s OK \n",src_file);
	}
	else{
		printf("Remove tmp file %s Failed \n",src_file);
	}
	//添加到最近上传文件列表中
}


//登录
int notify_user_login(unsigned long long tinyid,const char* user, const char* pwd, const char* extra_data)
{
	if(user == NULL || pwd == NULL) return -1;

	//使用用户名和密码登陆
	printf("[notify_user_login] user:%s, pwd:%s\n",user,pwd);

	if(strcmp(user,"admin") == 0 &&
	   strcmp(pwd,"123456") == 0){

		//登陆成功，保存用户tinyid和user
		g_isLogin = 0;

		return 0;
	}

	return -1;
}


//登出
int notify_user_logout(unsigned long long tinyid, const char* extra_data)
{
	printf("logout success!");

	//退出，删除tinyid和user的绑定

	g_isLogin = 1;

	return 0;
}


//检查用户登录状态
int notify_user_check_login(unsigned long long tinyid,const char* extra_data, char* user,int user_len)
{
	if(g_isLogin == 0){
		snprintf(user,user_len,"admin");
		return 0;
	}
	return -1;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//收到文件(收到C2C transfer通知)
void on_file_in_come(unsigned long long transfer_cookie, const tx_ccmsg_inst_info * inst_info, const tx_file_transfer_info * tran_info)
{
    printf("[on_file_in_come]: cookie[%llu] from uin[%llu]\n", transfer_cookie, inst_info->target_id);
}

//文件传输进度
void on_transfer_progress(unsigned long long transfer_cookie, unsigned long long transfer_progress, unsigned long long max_transfer_progress)
{
    printf("========> on file progress %f%% cookie[%llu]\n", transfer_progress * 100.0 / max_transfer_progress, transfer_cookie);
}


//文件传输结果
void on_transfer_complete(unsigned long long transfer_cookie, int err_code, tx_file_transfer_info* tran_info)
{
    printf("==========================ontransfercomplete==============================\n");
    printf("errcode %d, cookie[%llu] bussiness_name [%s], file path[%s], file key[%s], file_type[%d], transtype[%d], channeltype[%d] \n", 
        err_code, transfer_cookie, tran_info->bussiness_name,  tran_info->file_path, tran_info->file_key, tran_info->file_type, tran_info->transfer_type, tran_info->channel_type);
    printf("===============================================================================\n");
}


/**
 * 登录完成的通知，errcode为0表示登录成功，其余请参考全局的错误码表
 */
void on_login_complete(int errcode) 
{
    if(errcode != 0) {
        printf("Login failure, error = %d\n", errcode);
        sleep(1);//等待一秒，然后重新登陆
        //tx_device_relogin();
    } else {
        printf("Login success\n");
    }
}

/**
 * 在线状态变化通知， 状态（status）取值为 11 表示 在线， 取值为 21 表示  离线
 * old是前一个状态，new是变化后的状态（当前）
 */
void on_online_status(int old_status, int new_status) 
{
    if(new_status == 11) {
        printf("device online, new status = %d, old status = %d, din = %llu\n", new_status, old_status, tx_get_self_din());
        //int count = 0;
        //tx_get_binder_list(NULL, &count, on_get_binder_list_result_callback);
    } else if (new_status == 21 ) {   
        printf("device offline, new status = %d, old status = %d\n", new_status, old_status);
    } else {
        printf("device status unknown, new status = %d, old status = %d\n", new_status, old_status);
    }
}


// 绑定列表变化通知
void on_binder_list_change(int error_code, tx_binder_info * pBinderList, int nCount)
{
	int i = 0;
	for (i = 0; i < nCount; ++i )
	{
		printf(">>   binder uin[%llu], nick_name[%s]\n", pBinderList[i].uin, pBinderList[i].nick_name);
	}
}

int readBufferFromFile(char *pPath, char *pBuffer, int nInSize, int *pSizeUsed)
{
	if (!pPath || !pBuffer)
	{
		return -1;
	}

	int uLen = 0;
	FILE * file = fopen(pPath, "rb");
	if (!file)
	{
	    return -1;
	}

	do
	{
	    fseek(file, 0L, SEEK_END);
	    uLen = ftell(file);
	    fseek(file, 0L, SEEK_SET);

	    if (0 == uLen || uLen > nInSize)
	    {
	    	printf("invalide file or buffer size is too small...\n");
	        break;
	    }

	    *pSizeUsed = fread(pBuffer, 1, uLen, file);

	    fclose(file);
	    return 0;

	}while(0);

    fclose(file);
	return -1;
}


void log_func(int level, const char* module, int line, const char* message)
{
	printf("%s\t%d\t%s\n", module, line, message);
	//return;
	if (level == 1)
	{
	    FILE * file = fopen("/tmp/devicelog", "aw+");
	    if (file)
	    {
	    	fprintf(file, "%s\t%d\t%s\n", module, line, message);
	        fclose(file);
	    }
	}
}

void* thread_func_initdevice(void * arg)
{
	char license[256] = {0};
    int nLicenseSize = 0;
    if (0 != readBufferFromFile("/etc/qq/licence.sign.file.txt", license, sizeof(license), &nLicenseSize))
    {
        printf(c_CharColor_Red"[error]get license from file failed..."c_Print_Ctrl_Off"\n");
        return NULL;
    }
	printf("license=%s,strlen=%d\n",license,nLicenseSize);
	if(license[nLicenseSize-1]==0x0a)
	{
		license[nLicenseSize-1]='\0';
	}
    char guid[32] = {0};
    int nGUIDSize = 0;
    if(0 != readBufferFromFile("/etc/qq/Guid_file.txt", guid, sizeof(guid), &nGUIDSize))
    {
        printf(c_CharColor_Red"[error]get guid from file failed..."c_Print_Ctrl_Off"\n");
        return NULL;
    }
	guid[16]='\0';
	
	printf("guid=%s,strlen=%d\n",guid,nGUIDSize);
    char svrPubkey[256] = {0};
    int nPubkeySize = 0;
    if (0 != readBufferFromFile("/etc/qq/1000000705.pem", svrPubkey, sizeof(svrPubkey), &nPubkeySize))
    {
        printf(c_CharColor_Red"[error]get svrPubkey from file failed..."c_Print_Ctrl_Off"\n");
        return NULL;
    }


    //设备信息
    tx_device_info info = {0};
    info.os_platform            = "Linux";
    info.device_name            = "DM.NAS";
    info.device_serial_number   = guid;
    info.device_license         = license;
    info.product_version        = 1;
    info.product_id             = 1000000705;
    info.server_pub_key         = svrPubkey;
    info.test_mode              = 0;
    info.network_type           = network_type_wifi;

    //设备登录、在线状态、消息等相关的事件通知
    //   注意事项：
	//   如下的这些notify回调函数，都是来自硬件SDK内部的一个线程，所以在这些回调函数内部的代码一定要注意线程安全问题
	//   比如在on_login_complete操作某个全局变量时，一定要考虑是不是您自己的线程也有可能操作这个变量
    tx_device_notify notify = {0};
    notify.on_login_complete       = on_login_complete;
    notify.on_online_status        = on_online_status;
    notify.on_binder_list_change = on_binder_list_change;

    // SDK初始化目录，写入配置、Log输出等信息
    //   为了了解设备的运行状况，存在上传异常错误日志 到 服务器的必要
    //   system_path：SDK会在该目录下写入保证正常运行必需的配置信息
    //   system_path_capicity：是允许SDK在该目录下最多写入多少字节的数据（最小大小：10K，建议大小：100K）
    //   app_path：用于保存运行中产生的log或者crash堆栈
    //   app_path_capicity：同上，（最小大小：300K，建议大小：1M）
    //   temp_path：可能会在该目录下写入临时文件
    //   temp_path_capicity：这个参数实际没有用的，可以忽略
    tx_init_path init_path = {0};
    init_path.system_path = "/etc/qq/";
    init_path.system_path_capicity  = 10240;
    init_path.app_path = "/tmp/";
    init_path.app_path_capicity  = 1024000;
    init_path.temp_path = "/tmp";
    init_path.temp_path_capicity  = 102400;

    // 设置log输出函数，如果不想打印log，则无需设置。
    // 建议开发在开发调试阶段开启log，在产品发布的时候禁用log。
	//tx_set_log_func(log_func); //在函数中使用宏控制了，所以一定设置logfunc


	//[1/4]. 初始化设备SDK
	//       若初始化成功，则内部会启动一个线程去执行相关逻辑，该线程会持续运行，直到收到 exit 调用
	//       函数的错误返回值同tx_init_device
	int ret = tx_nas_init_device(&info, &notify, &init_path);
	if (err_null == ret) {
		printf("tx_nas_init_device success\n");
	}else {
		printf("tx_nas_init_device failed [%d]\n",ret);
		return NULL;
	}

	//[2/4]. 初始化文件传输
	tx_file_transfer_notify file_transfer_nofity = {0};
	file_transfer_nofity.on_transfer_complete 	= on_transfer_complete;
	file_transfer_nofity.on_file_in_come 		= on_file_in_come;
	file_transfer_nofity.on_transfer_progress 	= on_transfer_progress;

	ret = tx_nas_init_file_transfer(&file_transfer_nofity, "/tmp/mnt/USB-disk-1/.tmp/");
	if (0 == ret) {
		printf("tx_nas_init_file_transfer success\n");
	}else {
		printf("tx_nas_init_file_transfer failed [%d]\n", ret);
		return NULL;
	}


	//[3/4]. 初始化datapoint
	tx_data_point_notify data_point_notify = {0};
	data_point_notify.on_receive_data_point = on_receive_data_point;

	ret = tx_nas_init_data_point(&data_point_notify);
	if (0 == ret) {
		printf("tx_nas_init_data_point success\n");
	}else {
		printf("tx_nas_init_data_point failed [%d]\n", ret);
		return NULL;
	}


	//[4/4]. 初始化 Nas
	//       如果没有账号管理，则user相关的三个回调可以不注册
	tx_nas_device_mgr_wrapper device_mgr_wrapper = {0};
	//device_mgr_wrapper.filekey2abspath                        = filekey2abspath;
	device_mgr_wrapper.notify_file_download_complete = notify_file_download_complete;
	//device_mgr_wrapper.notify_user_check_login            = notify_user_check_login;
	//device_mgr_wrapper.notify_user_login                       = notify_user_login;
	//device_mgr_wrapper.notify_user_logout                     = notify_user_logout;

	ret = tx_nas_init(&device_mgr_wrapper);
	if (0 == ret) {
		printf("tx_nas_init success\n");
	}
	else {
		printf("tx_nas_init failed [%d]\n", ret);
		return NULL;
	}
	
	return NULL;
}

bool initdevice()
{
	pthread_t ntid = 0;
	int err;
	err = pthread_create(&ntid, NULL, thread_func_initdevice, NULL);
	if(err == 0 && ntid != 0)
	{
        pthread_join(ntid,NULL);
        ntid = 0;
    }

    return true;
}


int main(int argc, char* argv[])
{
	int fd;
	fd=access(g_root,0);
	if(fd<0){		
		mkdir(g_root, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
	}

    if(!initdevice()){
    	return -1;
    }
	


    // 你可以在做其他相关的事情
	// ...

	//char input[100];
	//while (scanf("%s", input)) {
	while (1) {
		/*
		if ( !strcmp(input, "quit") ) {
			tx_nas_uninit();
			break;
		}
		*/
		sleep(1);
	}
	
}
