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

#ifndef _UPNP_DEVICE_H
#define _UPNP_DEVICE_H

extern int upnp_device_init(struct device *device_def, char *ip_address,char *socketport);


int
upnp_add_response(struct action_event *event, char *key, const char *value);
void aNewOneIn(void);

extern void upnp_set_error(struct action_event *event, int error_code,
			   const char *format, ...);
extern char *upnp_get_string(struct action_event *event, const char *key);
int upnp_append_variable(struct action_event *event,
			 int varnum, char *paramname);

extern UpnpDevice_Handle device_handle;

#endif /* _UPNP_DEVICE_H */
