/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

//#include <execinfo.h>
//#include <unistd.h>
#include "defs.h"

static const char	*config_file = CONFIG;
static int		exit_flag;

static void
signal_handler(int sig_num)
{
	switch (sig_num) {
#ifndef _WIN32
	case SIGCHLD:
		while (waitpid(-1, &sig_num, WNOHANG) > 0) ;
		break;
#endif /* !_WIN32 */
	default:
		exit_flag = sig_num;
#if 0
		void *array[10];
		int size = 0;

		size = backtrace(array, 10);
		printf("Call Trace:\n");
		backtrace_symbols_fd(array, size,STDERR_FILENO);
		printf("end\n");
#endif
		break;
	}
}




int
main(int argc, char *argv[])
{
	struct shttpd_ctx	*ctx;

	current_time = time(NULL);
	if (argc > 1 && argv[argc - 2][0] != '-' && argv[argc - 1][0] != '-')
		config_file = argv[argc - 1];

#if !defined(NO_AUTH)
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'A') {
		if (argc != 6)
			usage(argv[0]);
		exit(edit_passwords(argv[2],argv[3],argv[4],argv[5]));
	}
#endif /* NO_AUTH */

	ctx = init_from_argc_argv(config_file, argc, argv);

#ifndef _WIN32
	/* Switch to alternate UID, it is safe now, after shttpd_listen() */
	if (ctx->uid != NULL) {
		struct passwd	*pw;

		if ((pw = getpwnam(ctx->uid)) == NULL)
			elog(E_FATAL, 0, "main: unknown user [%s]", ctx->uid);
		else if (setgid(pw->pw_gid) == -1)
			elog(E_FATAL, NULL, "main: setgid(%s): %s",
			    ctx->uid, strerror(errno));
		else if (setuid(pw->pw_uid) == -1)
			elog(E_FATAL, NULL, "main: setuid(%s): %s",
			    ctx->uid, strerror(errno));
	}
	(void) signal(SIGCHLD, signal_handler);
	(void) signal(SIGPIPE, SIG_IGN);
#endif /* _WIN32 */

	(void) signal(SIGTERM, signal_handler);
	(void) signal(SIGINT, signal_handler);

	if (ctx->inetd_mode) {
		(void) freopen("/dev/null", "a", stderr);
		shttpd_add_socket(ctx, fileno(stdin));
	} else {
		open_listening_ports(ctx);
	}

#ifdef _PPCLIB_H_
	char *ppc_username = "root";
	char *ppc_password = "123456";
	char *ppc_token = NULL;
	int ret= 0;
	ret = ppc_register(ppc_username, ppc_password);
	if(ret != SUCCESS){
		elog(E_LOG, NULL, "ppc register fail");
		shttpd_fini(ctx);
		return (-1);
	}
	
	ret = ppc_login(ppc_username,ppc_password,&ppc_token);
	if(ret != SUCCESS){
		elog(E_LOG, NULL, "ppc login fail");
		shttpd_fini(ctx);
		return (-1);
	}
	DMCLOG_D("get token success!!!, ppc_token: %d", (int)ppc_token);
	
#endif

	elog(E_LOG, NULL, "shttpd %s started on port(s) %s, serving %s",
	    VERSION, ctx->ports, ctx->document_root);

	while (exit_flag == 0)
		shttpd_poll(ctx, 5000);

	elog(E_LOG, NULL, "%d requests %.2lf Mb in %.2lf Mb out. "
	    "Exit on signal %d", ctx->nrequests, (double) (ctx->in / 1048576),
	    (double) ctx->out / 1048576, exit_flag);

	shttpd_fini(ctx);

	#ifdef _PPCLIB_H_
	ret = ppc_logout(ppc_token);	
	if(ret == SUCCESS){
		DMCLOG_D("logout success");
	}
	else{
		DMCLOG_D("logout fail");
	}		
	#endif

	return (EXIT_SUCCESS);
}
