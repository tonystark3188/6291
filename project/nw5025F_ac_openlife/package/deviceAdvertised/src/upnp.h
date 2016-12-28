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

#ifndef _UPNP_H
#define _UPNP_H

struct action;
struct service;
struct action_event;

struct action {
	const char *action_name;
	int (*callback) (struct action_event *);
};

typedef enum {
        PARAM_DIR_IN,
        PARAM_DIR_OUT,
} param_dir;

struct argument {
        const char *name;
        param_dir direction;
        int statevar;
};

typedef enum {
        DATATYPE_STRING,
        DATATYPE_BOOLEAN,
        DATATYPE_I2,
        DATATYPE_I4,
        DATATYPE_UI2,
        DATATYPE_UI4,
        DATATYPE_UNKNOWN,
        DATATYPE_COUNT
} param_datatype;

typedef enum {
        SENDEVENT_NO,
        SENDEVENT_YES
} param_event;

struct param_range {
        long long min;
        long long max;
        long long step;
};

struct var_meta {
        param_event     sendevents;
        param_datatype  datatype;
        const char      **allowed_values;
        struct param_range      *allowed_range;
	const char      *default_value;
};


struct icon {
        int width;
        int height;
        int depth;
        const char *url;
        const char *mimetype;
};

struct device {
	ithread_mutex_t device_mutex;
	int (*init_function) (void);
        const char *device_type;
        const char *friendly_name;
        const char *manufacturer;
        const char *manufacturer_url;
        const char *model_description;
        const char *model_name;
        const char *model_number;
        const char *model_url;
        const char *serial_number;
        const char *udn;
        const char *upc;
        const char *presentation_url;
	struct icon **icons;
	struct service **services;
};

struct service {
	ithread_mutex_t *service_mutex;
	const char *service_name;
	char *type;
	const char *scpd_url;
	const char *control_url;
	const char *event_url;
	struct action *actions;
	struct argument ***action_arguments;
	const char **variable_names;
	char **variable_values;
	struct var_meta *variable_meta;
	int variable_count;
	int command_count;
	const char **eventvar_names;
	const char **eventvar_values;
};

struct action_event {
	struct Upnp_Action_Request *request;
	int status;
	struct service *service;
};

struct play_info{
    char filename[512];
    char total_duration_string[16];
    char current_time_string[16];
    unsigned int total_duration;
    unsigned int current_time;
};

struct service *find_service(struct device *device_def,
                             char *service_name);
struct action *find_action(struct service *event_service,
                                  char *action_name);

char *upnp_get_scpd(struct service *srv);
char *upnp_get_device_desc(struct device *device_def,char *ip_address, char *socketport);

#endif /* _UPNP_H */