#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "rtsp.h"
#include "airplay_interface.h"
//#include "sharememory_interface.h"

void *module_switch_func(void *args)
{
	/*module_status status;

	while (1) {
		usleep(100000);

		if(share_mem_get(AIRPLAY_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_STOPPING) {
			if (mozart_airplay_stop_playback()) {
				printf("stop airplay playback error in %s:%s:%d, return.\n",
				       __FILE__, __func__, __LINE__);
				continue;
			}
		}
	}*/
}



void status_handler_setup(void)
{
	pthread_t module_switch_thread;
	// switch render, airplay, localplayer, bt_audio, voice_recogition
	if (pthread_create(&module_switch_thread, NULL, module_switch_func, NULL) != 0) {
		printf("Can't create module_switch_thread in %s:%s:%d: %s\n",__FILE__, __func__, __LINE__, strerror(errno));
		exit(1);
	}
	pthread_detach(module_switch_thread);
}

int main(int argc, char **argv)
{
	mozart_airplay_init(argc, argv);

	//status_handler_setup();

	rtsp_listen_loop();

	// should not.
	shairport_shutdown(1);

	return 1;
}
