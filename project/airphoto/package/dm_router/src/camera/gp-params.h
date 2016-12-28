#ifndef __GP_PARAMS_H
#define __GP_PARAMS_H


#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-context.h>


typedef enum {
	FLAGS_RECURSE		= 1 << 0,
	FLAGS_REVERSE		= 1 << 1,
	FLAGS_QUIET		= 1 << 2,
	FLAGS_FORCE_OVERWRITE	= 1 << 3,
	FLAGS_STDOUT		= 1 << 4,
	FLAGS_STDOUT_SIZE	= 1 << 5,
	FLAGS_NEW		= 1 << 6,
	FLAGS_RESET_CAPTURE_INTERVAL = 1 << 7,
	FLAGS_KEEP 		= 1 << 8,
	FLAGS_KEEP_RAW 		= 1 << 9,
	FLAGS_SKIP_EXISTING	= 1 << 10
} Flags;

typedef enum {
	MULTI_UPLOAD,
	MULTI_UPLOAD_META,
	MULTI_DOWNLOAD,
	MULTI_DELETE
} MultiType;

typedef struct _GPParams GPParams;
struct _GPParams {
	Camera *camera;
	GPContext *context;
	char *folder;
	char *filename;

	unsigned int cols;

	Flags flags;

	/** This field is supposed to be private. Usually, you use the
	 * gp_camera_abilities_list() function to access it.
	 */ 
	CameraAbilitiesList *_abilities_list;

	GPPortInfoList *portinfo_list;
	int debug_func_id;

	MultiType	multi_type;
	CameraFileType	download_type; /* for multi download */
       
	char *hook_script; /* If non-NULL, hook script to run */
	char **envp;  /* envp from the main() function */
};

void gp_params_init (GPParams *params, char **envp);
//void gp_params_exit (GPParams *params);


#endif
