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

#endif
