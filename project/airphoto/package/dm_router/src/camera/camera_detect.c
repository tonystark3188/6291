#include <stdio.h>

#include "camera.h"
#include "debuglog.h"
#include "base.h"

static PTHREAD_MUTEX_T g_camdete_mutex;
#define CAMERADET_LOCK	PTHREAD_MUTEX_LOCK(&g_camdete_mutex)
#define CAMERADET_UNLOCK	PTHREAD_MUTEX_UNLOCK(&g_camdete_mutex)
#define CAMERADET_LINIT	PTHREAD_MUTEX_INIT(&g_camdete_mutex, NULL)

#define CAMERA_HOTPLUG_THERSHOLD_1  1
#define CAMERA_HOTPLUG_THERSHOLD_2  6000000//i hope it never happend

volatile int g_hotplug_counter = 0;
volatile int g_hotplug_thershold = CAMERA_HOTPLUG_THERSHOLD_1;
volatile int g_camera_count = 0;

static int cam_hotplug_event(int cam_count)
{
	if (cam_count != g_camera_count) {
		DMCLOG_D("camera count change,old = %d, new = %d", g_camera_count, cam_count);
		g_camera_count = cam_count;
		send_camera_detect(cam_count);
	}	
	return 0;
}
static inline int is_need_to_check_hotplug()
{
	if (++g_hotplug_counter >= g_hotplug_thershold) {
		g_hotplug_counter = 0;
		return 1;
	}
	return 0;
}
int CamD_check_hotplug()
{
	if (is_need_to_check_hotplug()) {
		int cam_count = cam_detect_camera();
		cam_hotplug_event(cam_count);
		CAMERADET_LOCK;
		g_hotplug_thershold = cam_count==0?CAMERA_HOTPLUG_THERSHOLD_1:CAMERA_HOTPLUG_THERSHOLD_2;
		CAMERADET_UNLOCK;
	}
	return 0;
}


int CamD_usb_hotplug_event(int arg)
{
	DMCLOG_D("usb hotplug event");
	CAMERADET_LOCK;
	g_hotplug_thershold = 1;
	CAMERADET_UNLOCK;
	return 0;
}

int CamD_get_camera_count()
{
	return g_camera_count;
}

int init_cam_detect_mutex()
{
	CAMERADET_LINIT;
	return 0;
}


