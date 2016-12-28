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

int native_cnt = 0;

/*
 * Initialize shttpd context
 */
static void
initialize_context(struct shttpd_ctx *ctx)
{
	(void) memset(ctx, 0, sizeof(*ctx));
	InitializeCriticalSection(&ctx->mutex);
	LL_INIT(&ctx->connections);
	ctx->io_buf_size = DFLT_IO_SIZ;
	ctx->nactive_fd_cnt = 0;
	native_cnt = &ctx->nactive_fd_cnt;
	/* First pass: set the defaults */
}


struct shttpd_ctx *
shttpd_init()
{
	struct shttpd_ctx	*ctx;
	if ((ctx = malloc(sizeof(*ctx))) != NULL) {
		initialize_context(ctx);
	}
#ifdef TOKEN_MANAGE
	dm_token_init(ctx);
#endif
	xxInitPinyinDB();
	media_list_init();
	media_prc_thpool_init();
	rfsvfs_init(RFSVFS_DIR_PATH_DEFAULT);

	return (ctx);
}
