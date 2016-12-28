/*
 * nl80211 userspace tool
 *
 * Copyright 2007, 2008	Johannes Berg <johannes@sipsolutions.net>
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"
//#include "scan.h"

/* libnl 1.x compatibility code */
#if !defined(CONFIG_LIBNL20) && !defined(CONFIG_LIBNL30)
static inline struct nl_handle *nl_socket_alloc(void)
{
	return nl_handle_alloc();
}

static inline void nl_socket_free(struct nl_sock *h)
{
	nl_handle_destroy(h);
}

static inline int nl_socket_set_buffer_size(struct nl_sock *sk,
					    int rxbuf, int txbuf)
{
	return nl_set_buffer_size(sk, rxbuf, txbuf);
}
#endif /* CONFIG_LIBNL20 && CONFIG_LIBNL30 */

int iw_debug = 0;

static int nl80211_init(struct nl80211_state *state)
{
	int err;

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	nl_socket_set_buffer_size(state->nl_sock, 8192, 8192);

	if (genl_connect(state->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	state->nl80211_id = genl_ctrl_resolve(state->nl_sock, "nl80211");
	if (state->nl80211_id < 0) {
		fprintf(stderr, "nl80211 not found.\n");
		err = -ENOENT;
		goto out_handle_destroy;
	}

	return 0;

 out_handle_destroy:
	nl_socket_free(state->nl_sock);
	return err;
}

static void nl80211_cleanup(struct nl80211_state *state)
{
	nl_socket_free(state->nl_sock);
}

static int cmd_size;

extern struct cmd __start___cmd;
extern struct cmd __stop___cmd;

#define for_each_cmd(_cmd)					\
	for (_cmd = &__start___cmd; _cmd < &__stop___cmd;		\
	     _cmd = (const struct cmd *)((char *)_cmd + cmd_size))

static const char *argv0;

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

static int __handle_cmd(struct nl80211_state *state, enum wifi_mode mode,
			int argc, char **argv, const struct cmd **cmdout)
{
	const struct cmd *cmd, *match = NULL, *sectcmd;
	struct nl_cb *cb;
	struct nl_cb *s_cb;
	struct nl_msg *msg;
	signed long long devidx = 0;
	int err, o_argc;
	const char *command, *section;
	char *tmp, **o_argv;
	enum command_identify_by command_idby = CIB_NONE;

	if (argc <= 1)
		return 1;
	
	o_argc = argc;
	o_argv = argv;

	command_idby = CIB_NETDEV;
	devidx = if_nametoindex(*argv);
	if (devidx == 0)
		devidx = -1;
	argc--;
	argv++;

	if (devidx < 0)
		return -errno;

	section = *argv;
	argc--;
	argv++;

	for_each_cmd(sectcmd) {
		if (sectcmd->parent)
			continue;
		/* ok ... bit of a hack for the dupe 'info' section */
		if (match && sectcmd->idby != command_idby)
			continue;
		if (strcmp(sectcmd->name, section) == 0)
			match = sectcmd;
	}

	sectcmd = match;
	match = NULL;
	if (!sectcmd)
		return 1;

	if (argc > 0) {
		command = *argv;
		//p_debug("command = %s\n", command);
		for_each_cmd(cmd) {
			if (!cmd->handler)
				continue;
			if (cmd->parent != sectcmd)
				continue;
			/*
			 * ignore mismatch id by, but allow WDEV
			 * in place of NETDEV
			 */
			if (cmd->idby != command_idby &&
			    !(cmd->idby == CIB_NETDEV &&
			      command_idby == CIB_WDEV))
				continue;
			if (strcmp(cmd->name, command))
				continue;
			if (argc > 1 && !cmd->args)
				continue;
			match = cmd;
			break;
		}

		if (match) {
			argc--;
			argv++;
		}
	}
	
	if (match)
		cmd = match;
	else {
		/* Use the section itself, if possible. */
		cmd = sectcmd;
		if (argc && !cmd->args)
			return 1;
		if (cmd->idby != command_idby &&
		    !(cmd->idby == CIB_NETDEV && command_idby == CIB_WDEV))
			return 1;
		if (!cmd->handler)
			return 1;
	}

	if (cmd->selector) {
		cmd = cmd->selector(argc, argv);
		if (!cmd)
			return 1;
	}

	if (cmdout)
		*cmdout = cmd;
#if 1
	if (!cmd->cmd) {
		argc = o_argc;
		argv = o_argv;
		return cmd->handler(state, NULL, NULL, argc, argv, mode);
	}
#endif
	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		return 2;
	}

	cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
	s_cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
	if (!cb || !s_cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
		goto out_free_msg;
	}

	genlmsg_put(msg, 0, 0, state->nl80211_id, 0,
		    cmd->nl_msg_flags, cmd->cmd, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);

	err = cmd->handler(state, cb, msg, argc, argv, mode);
	if (err){
		p_debug("goto out\n");
		goto out;
	}
	nl_socket_set_cb(state->nl_sock, s_cb);

	err = nl_send_auto_complete(state->nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(state->nl_sock, cb);
 out:
	nl_cb_put(cb);
 out_free_msg:
	nlmsg_free(msg);
	return err;
 nla_put_failure:
	fprintf(stderr, "building message failed\n");
	return 2;
}


int handle_cmd(struct nl80211_state *state, enum wifi_mode mode,
	       int argc, char **argv)
{
	return __handle_cmd(state, mode, argc, argv, NULL);
}

extern ap_list_info_t scan_ap_list_info;

int get_scan_list_nl80211(char *ifname, ap_list_info_t *p_ap_list){
	int i = 0;
	int wifi_mode;
	int err = 0;

	int scan_argc = 2;
	char **scan_argv;
	scan_argv = calloc(scan_argc, sizeof(*scan_argv));
	if(scan_argv == NULL){
		err = -1;
		goto EXIT1;
	}
	scan_argv[0] = ifname;
	scan_argv[1] = "scan";
	
	struct nl80211_state nlstate;
	const struct cmd *cmd = NULL;

	/* calculate command size including padding */
	cmd_size = abs((long)&__section_set - (long)&__section_get);

	err = nl80211_init(&nlstate);
	if (err){
		goto EXIT1;
	}

	err = __handle_cmd(&nlstate, wifi_mode, scan_argc, scan_argv, &cmd);
	if (err){
		goto EXIT2;
	}
#if 0
	for(i = 0; i < scan_ap_list_info.count; i++){
		printf("ssid: %s\t", scan_ap_list_info.ap_info[i].ssid);
		printf("mac: %s\t", scan_ap_list_info.ap_info[i].mac);
		printf("channel: %d\t", scan_ap_list_info.ap_info[i].channel);
		printf("signal: %d\t", scan_ap_list_info.ap_info[i].wifi_signal);
		printf("encrypt: %s\t", scan_ap_list_info.ap_info[i].encrypt);
		printf("tkip_aes: %s\t", scan_ap_list_info.ap_info[i].tkip_aes);
		printf("\n");
	}
#endif
	if(p_ap_list != NULL){
		memcpy(p_ap_list, &scan_ap_list_info, sizeof(ap_list_info_t));
	}
	else{
		err = -1;
		goto EXIT2;
	}
	
EXIT2:
	nl80211_cleanup(&nlstate);
EXIT1:
	if(scan_argv != NULL){
		free(scan_argv);
	}		
	return err;
}

