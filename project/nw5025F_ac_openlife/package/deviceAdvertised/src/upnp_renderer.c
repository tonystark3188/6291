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

#include "config.h"

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <upnp/upnp.h>
#include <upnp/ithread.h>
#include <upnp/upnptools.h>

#include "logging.h"
#include "webserver.h"
#include "upnp.h"
#include "upnp_device.h"

#include "upnp_renderer.h"

/*static struct service *upnp_services[] = {
	&transport_service,
	&connmgr_service,
	&control_service,
	NULL
};*/

static struct icon icon1 = {
        .width =        64,
        .height =       64,
        .depth =        24,
        .url =          "/upnp/grender-64x64.png",
        .mimetype =     "image/png"
};
static struct icon icon2 = {
        .width =        128,
        .height =       128,
        .depth =        24,
        .url =          "/upnp/grender-128x128.png",
        .mimetype =     "image/png"
};

static struct icon *renderer_icon[] = {
        &icon1,
        &icon2,
        NULL
};

static int upnp_renderer_init(void);

static struct device render_device = {
	.init_function          = upnp_renderer_init,
        .device_type            = "urn:schemas-upnp-org:device:airdisk",
        .friendly_name          = "airdisk",
        .manufacturer           = "DAMAI",
        .manufacturer_url       = "http://www.dmsys.com",
        .model_description      = PACKAGE_STRING,
        .model_name             = PACKAGE_NAME,
        .model_number           = PACKAGE_VERSION,
        .model_url              = "http://www.dmsys.com",
        .serial_number          = "1",
        .udn                    = "uuid:airdisk-1_0-000-000-002",
        .upc                    = "",
        .presentation_url       = "/renderpres.html",
        .icons                  = renderer_icon,
        .services               = NULL //upnp_services
};
/*
void upnp_renderer_dump_connmgr_scpd(void)
{
	fputs(upnp_get_scpd(&connmgr_service), stdout);
}
void upnp_renderer_dump_control_scpd(void)
{
	fputs(upnp_get_scpd(&control_service), stdout);
}
void upnp_renderer_dump_transport_scpd(void)
{
	fputs(upnp_get_scpd(&transport_service), stdout);
}
*/
static int upnp_renderer_init(void)
{
    return 0;//return connmgr_init();
}

struct device *upnp_renderer_new(const char *friendly_name,
                                 const char *uuid)
{
	ENTER();
	char *udn;

	render_device.friendly_name = friendly_name;
	
	asprintf(&udn, "uuid:%s", uuid);
	render_device.udn = udn;
	return &render_device;
}
