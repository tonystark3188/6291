/*
 *  Shenzhen LongSys Electronics Co.,Ltd All rights reserved.
 *
 * The source code   are owned by  Shenzhen LongSys Electronics Co.,Ltd.
 * Corporation or its suppliers or licensors. Title LongSys or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of LongSys or its suppliers and
 * licensors. 
 *
 */

#ifndef _WEBSERVER_H
#define _WEBSERVER_H

extern struct UpnpVirtualDirCallbacks virtual_dir_callbacks;
extern int webserver_register_buf(const char *path, const char *contents,
                                  const char *content_type);
extern int webserver_register_file(const char *path,
                                   const char *content_type);

#endif /* _WEBSERVER_H */
