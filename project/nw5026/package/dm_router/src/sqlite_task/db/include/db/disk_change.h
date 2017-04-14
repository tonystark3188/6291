//  task_base.h
//  HidiskClientLib
//
//  Created by apple on 15/9/15.
//  Copyright (c) 2015年 apple. All rights reserved.
//

#ifndef __DISK_CHANGE_H__
#define __DISK_CHANGE_H__

enum{
	EVENT_PC_MOUNT,
	EVENT_UDISK_EXTARCT,
};

void set_on_device_list_changed(void *self);

#endif

