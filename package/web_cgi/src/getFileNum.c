#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "my_debug.h"
#include "msg.h"
int get_file_ext(char * filename,char* format){
	char *ptr;
	char ext[16]="\0";
	int i=1;
	p_debug("filename=%s\n",filename);
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
	p_debug(".ext=%s\n",format);
}
typedef enum enum_file_type_def_internal {
	file_type_rich_lite = -2, // 图片，音频，视频,(常见)
	file_type_rich = -1, // 图片，音频，视频
	file_type_doc = 1, //文档
	file_type_pic = 2, //图片
	file_type_audio = 3, //音频  (语音)
	file_type_video = 4, //视频
	file_type_file = 5, //文件
	file_type_dir = 6, //目录
	file_type_music = 7, //音乐
	file_type_other = 0 //其他文件
}nas_file_type;

int filter_dot(const struct dirent *ent)
{
	//if(ent->d_type != DT_REG)
		//return 0;
	return (ent->d_name[0] != '.');
}	
int get_file_type(char* format)
{
	p_debug("format=%s",format);
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

	for ( i = 0; i < sizeof(pic_type)/sizeof(char*); ++i)
	{
		if(strcmp(pic_type[i], lower_format) == 0)
		{
			return file_type_pic;
		}
	}
	for ( i = 0; i < sizeof(doc_type)/sizeof(char*); ++i)
	{
		if(strcmp(doc_type[i], lower_format) == 0)
		{
			return file_type_doc;
		}
	}
	for ( i = 0; i < sizeof(video_type)/sizeof(char*); ++i)
	{
		if(strcmp(video_type[i], lower_format) == 0)
		{
			return file_type_video;
		}
	}	
	for ( i = 0; i < sizeof(audio_type)/sizeof(char*); ++i)
	{
		if(strcmp(audio_type[i], lower_format) == 0)
		{
			return file_type_audio;
		}
	}
	return file_type_file;
}
#define MAX_FILE_NAME_LEN 256
#define letv_path "/tmp/mnt/USB-disk-1/hack"
#define public_path "/tmp/mnt/USB-disk-1/public"
#define video_path "/tmp/mnt/USB-disk-1/public/视频"
#define pic_path "/tmp/mnt/USB-disk-1/public/图片"
#define getPublicNum 1
#define getLetvNum 2

typedef struct fileNum{
	unsigned int fileTotal;
	unsigned int videoTotal;
	unsigned int picTotal;
}FileNum;

unsigned long letvVideoNum=0;
int scanDir(char *path, FileNum *filesNum,int get_type){
	DIR *dp;

	if((dp = opendir(path)) == NULL)
	{
		//printf("Can't open %s\n", path);	
		return -1;
	}
	closedir(dp);
	
	struct dirent **namelist;
	int i=0;
	int k=0;
	int total = scandir(path, &namelist, filter_dot, alphasort);
	if(total < 0)
	{	
		//perror("scandir");
		return -2;
	}
    else
    {
     //   for(i = 0; i < total; i++)
     //   printf("dir------%s\n", namelist[i]->d_name);
     //   printf("total = %d\n", total);
    }	
	if(get_type==getPublicNum) filesNum->fileTotal+=total;
	char value[4096]="\0";
	//char value2[20480]="\0";
	char format[16]="\0";
	//sprintf(value,"{\"r\": 0,\"p\": \"%s\",\"fs\":[",file_path);//size need to reconfirm.
	int j=0;
	int dir_num=0;
	int file_num=0;
	//printf("total = %d\n", total);
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
				p_debug("tmpfilelist[%d]=%s",k,tmpfilelist[k]);
				get_file_ext(tmpfilelist[k],format);
				int file_type=get_file_type(format);
				p_debug("file_type=%d",file_type);
				if(file_type==file_type_video){
					if(get_type==getPublicNum)
						filesNum->videoTotal++;
					else letvVideoNum++;
				}else if(file_type==file_type_pic){
					if(get_type==getPublicNum) 
						filesNum->picTotal++;			
				}
				k++;
			}
	}
	dir_num=j;
	file_num=k;
	//printf("dir_num=%d,file_num=%d",dir_num,file_num);

	for(i = 0; i < total; i++)
	{
		free(namelist[i]);
		free(filelist[i]);
		free(tmpfilelist[i]);
	}
	free(namelist);
	free(filelist);
	free(tmpfilelist);
	return 0;
}

int main(){
	system("countFiles.sh /tmp/mnt/USB-disk-1/public &");
	char v_n[16]={0};
	char p_n[16]={0};
	char o_n[16]={0};	
	char lv_n[16]={0};
    sleep(3);
	get_conf_str(v_n,"v_n");
	long videoNum=atoi(v_n);
	get_conf_str(p_n,"p_n");
	long picNum=atoi(p_n);	
	get_conf_str(o_n,"o_n");	
	long otherNum=atoi(o_n);

	long long total=videoNum+picNum+otherNum;
	
	get_conf_str(lv_n,"lv_n");

	long letvVideoNum=atoi(lv_n);

	char ret_buf[RET_BUF_LEN]={0};
	printf("Content-type:text/plain\r\n\r\n");

	p_debug("total=%lld,video=%d,pic=%d,letv_video=%d",total,videoNum,picNum,letvVideoNum);
	sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"letvVideoNum\":%ld,\"publicVideoNum\":%ld,\"publicImageNum\":%ld,\"publicTotalNum\":%lld}}",letvVideoNum, \
		videoNum,picNum,total);
	printf("%s",ret_buf);
		fflush(stdout);
		return;

	
	#if 0
	
	char ret_buf[RET_BUF_LEN]={0};
	int ret=0;
	FileNum *filesNum=(FileNum*)malloc(sizeof(struct fileNum));
	filesNum->fileTotal=0;
	filesNum->videoTotal=0;
	filesNum->picTotal=0;
	ret=scanDir(public_path,filesNum,getPublicNum);
	ret=scanDir(video_path,filesNum,getPublicNum);
	ret=scanDir(pic_path,filesNum,getPublicNum);
	ret=scanDir(letv_path,filesNum,getLetvNum);
	printf("Content-type:text/plain\r\n\r\n");
	
	if(ret<0){
		sprintf(ret_buf,"%s","{\"status\":1,\"errorCode\":5,\"errorMessage\":\"get file num error.\",\"data\":{}}");
	}else{ 
		p_debug("total=%d,video=%d,pic=%d,letv_video=%d",filesNum->fileTotal,filesNum->videoTotal,filesNum->picTotal,letvVideoNum);
		sprintf(ret_buf,"{\"status\":1,\"errorCode\":0,\"errorMessage\":\"success\",\"data\":{\"letvVideoNum\":%ld,\"publicVideoNum\":%ld,\"publicImageNum\":%ld,\"publicTotalNum\":%ld}}",letvVideoNum, \
		filesNum->videoTotal,filesNum->picTotal,filesNum->fileTotal);
	}
	printf("%s",ret_buf);
	fflush(stdout);
	return;
	#endif
}
