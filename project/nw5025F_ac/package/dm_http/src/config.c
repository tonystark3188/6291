/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#include "defs.h"

/*
 * Configuration parameters setters
 */
static void
set_int(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	ctx = NULL;	/* Unused */
	* (int *) ptr = atoi(string);
}

static void
set_str(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	ctx = NULL;	/* Unused */
	* (char **) ptr = my_strdup(string);
}

static void
set_log_file(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	FILE	**fp = ptr;
	ctx = NULL;

	if ((*fp = fopen(string, "a")) == NULL)
		elog(E_FATAL, NULL, "cannot open log file %s: %s",
		    string, strerror(errno));
}

#if 0
/*
 * Dynamically load SSL library. Set up ctx->ssl_ctx pointer.
 */
static void
set_ssl(struct shttpd_ctx *ctx, void *arg, const char *pem)
{
	SSL_CTX		*CTX;
	void		*lib;
	struct ssl_func	*fp;

	arg = NULL;	/* Unused */

	/* Load SSL library dynamically */
	if ((lib = dlopen(SSL_LIB, RTLD_LAZY)) == NULL)
		elog(E_FATAL, NULL, "set_ssl: cannot load %s", SSL_LIB);

	for (fp = ssl_sw; fp->name != NULL; fp++)
		if ((fp->ptr.v_void = dlsym(lib, fp->name)) == NULL)
			elog(E_FATAL, NULL,"set_ssl: cannot find %s", fp->name);

	/* Initialize SSL crap */
	SSL_library_init();

	if ((CTX = SSL_CTX_new(SSLv23_server_method())) == NULL)
		elog(E_FATAL, NULL, "SSL_CTX_new error");
	else if (SSL_CTX_use_certificate_file(CTX, pem, SSL_FILETYPE_PEM) == 0)
		elog(E_FATAL, NULL, "cannot open %s", pem);
	else if (SSL_CTX_use_PrivateKey_file(CTX, pem, SSL_FILETYPE_PEM) == 0)
		elog(E_FATAL, NULL, "cannot open %s", pem);
	ctx->ssl_ctx = CTX;
}
#endif /* NO_SSL */

static void
set_mime(struct shttpd_ctx *ctx, void *arg, const char *string)
{
	arg = NULL;
	set_mime_types(ctx, string);
}


static void
initialize_context(struct shttpd_ctx *ctx)
{
	struct tm		*tm;

	current_time = time(NULL);
	tm = localtime(&current_time);
	tz_offset = 0;
#if 0
	tm->tm_gmtoff - 3600 * (tm->tm_isdst > 0 ? 1 : 0);
#endif

	(void) memset(ctx, 0, sizeof(*ctx));

	ctx->start_time = current_time;
	InitializeCriticalSection(&ctx->mutex);

	LL_INIT(&ctx->connections);
	LL_INIT(&ctx->mime_types);
	LL_INIT(&ctx->registered_uris);
	LL_INIT(&ctx->uri_auths);
	LL_INIT(&ctx->error_handlers);

#if !defined(NO_SSI)
	LL_INIT(&ctx->ssi_funcs);
#endif /* NO_SSI */

	/* Second pass: load config file  */
	
	/* If document_root is not set, set it to current directory */
	if (ctx->document_root == NULL) {
		//(void) my_getcwd(root, sizeof(root));
		ctx->document_root = my_strdup("/tmp/mnt");
		
	}
	ctx->io_buf_size = DFLT_IO_SIZ;
#ifdef _WIN32
	{WSADATA data;	WSAStartup(MAKEWORD(2,2), &data);}
#endif /* _WIN32 */

	DBG(("init_ctx: initialized context %p", (void *) ctx));
}

struct shttpd_ctx *
shttpd_init()
{
	struct shttpd_ctx	*ctx;
#ifdef __linux__
	if ((ctx = malloc(sizeof(*ctx))) != NULL) {
#else
	if ((ctx = os_mem_alloc(sizeof(*ctx))) != NULL) {
#endif
		initialize_context(ctx);
	}
	return (ctx);
}
