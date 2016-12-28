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

#ifndef _UPNP_RENDERER_H
#define _UPNP_RENDERER_H

void upnp_renderer_dump_connmgr_scpd(void);
void upnp_renderer_dump_control_scpd(void);
void upnp_renderer_dump_transport_scpd(void);

struct device *upnp_renderer_new(const char *friendly_name,
                                 const char *uuid);

#endif /* _UPNP_RENDERER_H */
