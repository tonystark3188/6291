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
#include "base.h"
/*
 * Initialize shttpd context
 */
static void
initialize_context(struct shttpd_ctx *ctx)
{
	ENTER_FUNC();
	(void) memset(ctx, 0, sizeof(*ctx));
	InitializeCriticalSection(&ctx->mutex);
	LL_INIT(&ctx->connections);
	ctx->io_buf_size = DFLT_IO_SIZ;
	/* First pass: set the defaults */
	EXIT_FUNC();
}

struct shttpd_ctx *
init_from_argc_argv()
{
	struct shttpd_ctx	*ctx;
	if ((ctx = malloc(sizeof(*ctx))) != NULL)
		initialize_context(ctx);
	return (ctx);
}

struct shttpd_ctx *
shttpd_init()
{
	struct shttpd_ctx	*ctx;
	if ((ctx = malloc(sizeof(*ctx))) != NULL) {
		initialize_context(ctx);
	}
	return (ctx);
}
