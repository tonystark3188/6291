/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * $Id: shttpd.h,v 1.3 2007/04/11 13:01:53 drozd Exp $
 */

#ifndef SHTTPD_HEADER_INCLUDED
#define	SHTTPD_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */







struct shttpd_ctx;

struct shttpd_ctx *shttpd_init();
void shttpd_fini(struct shttpd_ctx *);
int shttpd_listen(struct shttpd_ctx *ctx, int port);
void shttpd_poll(struct shttpd_ctx *, int milliseconds);

/*
 * The following three functions are for applications that need to
 * load-balance the connections on their own. Many threads may be spawned
 * with one SHTTPD context per thread. Boss thread may only wait for
 * new connections by means of shttpd_accept(). Then it may scan thread
 * pool for the idle thread by means of shttpd_active(), and add new
 * connection to the context by means of shttpd_add().
 */
void shttpd_add_socket(struct shttpd_ctx *, int sock);
int shttpd_accept(int lsn_sock, int milliseconds);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SHTTPD_HEADER_INCLUDED */
