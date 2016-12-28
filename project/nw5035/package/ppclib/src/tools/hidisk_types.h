//
//  hidisk_types.h
//  HidiskClientLib
//
//  Created by apple on 15/8/13.
//  Copyright (c) 2015年 apple. All rights reserved.
//

#ifndef HidiskClientLib_hidisk_types_h
#define HidiskClientLib_hidisk_types_h

#include <sys/stat.h>
#include <unistd.h>

#ifdef Mac
typedef struct stat64 _stat64;
#define _lstat64 lstat64
#define _lseek64 lseek64
#else
typedef struct stat _stat64;
#define _lstat64 lstat
#define _lseek64 lseek
#endif

//typedef unsigned long long uint64_t;
typedef long long _int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
// typedef char int8_t;
typedef unsigned char uint8_t;

#endif
