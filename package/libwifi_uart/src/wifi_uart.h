#ifndef __WIFI_UART_H
#define __WIFI_UART_H 

#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <limits.h>      
#include <setjmp.h>
#include <sys/mount.h>
#include <fcntl.h> 
#include <sys/types.h>  
#include <sys/stat.h>   
#include <termios.h>    
#include <sys/ioctl.h> 

#define __DEBUG__ 

#ifdef __DEBUG__  
//#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#define debug(format,...) printf(format, ##__VA_ARGS__) 
#else  
#define debug(format,...)  
#endif 


#define READ_FLAG  1
#define WRITE_FLAG 0
#define BAUD_RATE (B19200)


#define ALARM_CANCEL 0xFF
#define SET_COMPLETE 0xF0
//#define SET_FUN 0x20
#define SET_YEAR 0x13
#define SET_MONTH 0x12
#define SET_WEEK 0x11
#define SET_DAY 0x10
#define SET_HOUR 0x08
#define SET_AMPM 0x04
#define SET_MINUTE 0x02
#define SET_SECORD 0x01


//UPLOAD CMD
#define UPLOAD_ADDR    				0x13
#define UPLOAD_DATA     			0x12
#define UPLOAD_END_FLAG 			0x14

//GET CMD
#define CMD_GET_POWER_PERCENT 		0x30
#define CMD_GET_BUTTON_STATUS		0X31
#define CMD_GET_CHARGE_STATUS		0x32
#define CMD_GET_STORAGE_POWER		0x33
#define CMD_GET_PC_DETECT_STATUS	0x34
#define CMD_GET_FIRMWARE_VERSION 	0x35
#define CMD_GET_KEY					0x36
#define CMD_GET_POWEROFF			0x37
#define CMD_GET_LINE_IN				0x38
#define CMD_GET_MEM_DIRECT 0x39
#define CMD_GET_POWER_STATUS 0x40

//SET CMD
#define CMD_SET_RESET				0x50
#define CMD_SET_POWEROFF			0x51
#define CMD_SET_STORAGE_POWER		0x52
#define CMD_SET_STORAGE_DIRECTION	0x53
#define CMD_SET_STORAGE_MODE		0x54
#define CMD_SET_VERIFY_KEY			0x55
#define CMD_SET_AMP					0x56
#define CMD_SET_MUTE				0x57
#define CMD_SET_SURE_POWEROFF		0x58
#define CMD_SET_RESET_LED			0x59
#define CMD_SET_SYSTEM_UP 0X60
#define CMD_SET_ALARM 0X61
#define CMD_SET_RTC 0x62

#endif

