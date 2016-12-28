
/* $Id: json_util.h,v 1.4 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef __MCU_H_
#define __MCU_H_
#include "base.h"
#include "hd_route.h"
#ifdef __cplusplus
extern "C" {
#endif

#define get_power_level_num  1
#define get_Firmware_Edition 5
#define rtl_encryp_control "/proc/rtl_encryp_control"

int dm_mcu_get_power(power_info_t *p_power_info);
int _dm_mcu_get_storage_dir(int *storage_dir);

#ifdef __cplusplus
}
#endif

#endif