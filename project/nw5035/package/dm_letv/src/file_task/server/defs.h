/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#ifndef DEFS_HEADER_DEFINED
#define	DEFS_HEADER_DEFINED

#include "std_includes.h"
#include "llist.h"
#include "io.h"
#include "shttpd.h"
#include "my_json.h"
#include "router_task.h"




#define EXPIRE_PER_SEC (5)
#define	DFLT_IO_SIZ	16384		/* Default max request size	*/
#define	DELIM_CHARS	" ,"		/* Separators for lists		*/
#define	URI_MAX		32768		/* Maximum URI size		*/

/*
 * Darwin prior to 7.0 and Win32 do not have socklen_t
 */
#ifdef NO_SOCKLEN_T
typedef int socklen_t;
#endif /* NO_SOCKLEN_T */

/*
 * For parsing. This guy represents a substring.
 */
struct vec {
	const char	*ptr;
	int		len;
};
enum {METHOD_GET, METHOD_POST, METHOD_PUT, METHOD_DELETE, METHOD_HEAD};
enum {HDR_DATE, HDR_INT, HDR_STRING};	/* HTTP header types		*/
enum {E_FATAL = 1, E_LOG = 2};		/* Flags for elog() function	*/
typedef unsigned long big_int_t;	/* Type for Content-Length	*/
	
/*
 * Unified socket address
 */
struct usa {
	socklen_t len;
	union {
		struct sockaddr	sa;
		struct sockaddr_in sin;
	} u;
};

/*
 * This thing is aimed to hold values of any type.
 * Used to store parsed headers' values.
 */
union variant {
	char		*v_str;
	int		v_int;
	big_int_t	v_big_int;
	time_t		v_time;
	void		(*v_func)(void);
	void		*v_void;
	struct vec	v_vec;
};


struct mime_type {
	const char	*ext;
	int		ext_len;
	const char	*mime;
	int file_type;
};
struct http_header {
	int		len;		/* Header name length		*/
	int		type;		/* Header type			*/
	size_t		offset;		/* Value placeholder		*/
	const char	*name;		/* Header name			*/
};

/*
 * This guy holds parsed HTTP headers
 */
struct headers {
	union variant	cl;		/* Content-Length:		*/
	union variant	ct;		/* Content-Type:		*/
	union variant	connection;	/* Connection:			*/
	union variant	ims;		/* If-Modified-Since:		*/
	union variant	user;		/* Remote user name		*/
	union variant	auth;		/* Authorization		*/
	union variant	useragent;	/* User-Agent:			*/
	union variant	referer;	/* Referer:			*/
	union variant	cookie;		/* Cookie:			*/
	union variant	location;	/* Location:			*/
	union variant	range;		/* Range:			*/
	union variant	status;		/* Status:			*/
	union variant	transenc;	/* Transfer-Encoding:		*/
};

/* Must go after union variant definition */
//#include "ssl.h"

/*
 * The communication channel
 */
union channel {
	int		fd;		/* Regular static file		*/
	int		sock;		/* Connected socket		*/		/* SSL-ed socket		*/
	struct {
		DIR	*dirp;
		char	*path;
	} dir;				/* Opened directory		*/
	struct {
		void		*state;	/* For keeping state		*/
		union variant	func;	/* User callback function	*/
		void		*data;	/* User defined parameters	*/
	} emb;				/* Embedded, user callback	*/
};

struct stream;

/*
 * IO class descriptor (file, directory, socket, SSL, CGI, etc)
 * These classes are defined in io_*.c files.
 */
struct io_class {
	const char *name;
	int (*read)(struct stream *, void *buf, size_t len);
	int (*write)(struct stream *, const void *buf, size_t len);
	void (*close)(struct stream *);
};

/*
 * Data exchange stream. It is backed by some communication channel:
 * opened file, socket, etc. The 'read' and 'write' methods are
 * determined by a communication channel.
 */
struct stream {
	struct conn		*conn;
	union channel		chan;		/* Descriptor		*/
	struct io		io;		/* IO buffer		*/
	const struct io_class	*io_class;	/* IO class		*/
	int			nread_last;	/* Bytes last read	*/
	int			headers_len;
	big_int_t		content_len;
	unsigned int		flags;
#define	FLAG_HEADERS_PARSED	1
#define	FLAG_SSL_ACCEPTED	2
#define	FLAG_R			4		/* Can read in general	*/
#define	FLAG_W			8		/* Can write in general	*/
#define	FLAG_CLOSED		16
#define	FLAG_DONT_CLOSE		32
#define	FLAG_ALWAYS_READY	64		/* File, dir, user_func	*/
};

typedef struct conn {
	struct llhead	link;		/* Connections chain		*/
	struct shttpd_ctx *ctx;		/* Context this conn belongs to */
	struct usa	sa;		/* Remote socket address	*/

	int		loc_port;	/* Local port			*/
	int		status;		/* Reply status code		*/
	char		*uri;		/* Decoded URI			*/
	char		*request;	/* Request line			*/
	char		*query;		/* QUERY_STRING part of the URI	*/

	struct headers	ch;		/* Parsed client headers	*/
	struct stream	loc;		/* Local stream			*/
	struct stream	rem;		/* Remote stream		*/
	unsigned long offset;
	unsigned long length;
	unsigned long fileSize;
	unsigned long modifyTime;
	char *path;
	char *rename_path;
	char* cfg_path;
	char* tmp_path;
	int count;
	JObj *r_json; 
	struct timeval tstart;
	//int cmd;
	char cmd[32];
	int error;
	char client_ip[32];
	int client_port;
	unsigned long cur_time;
	
	char pid[33];    /*剧集ID*/
    char vid[33];    /*单个视频ID*/
	char ext[256];    /*剧集ID*/
	char tag[512];    /*剧集ID*/

    char *img_url;    /*视频封面图片的下载链接*/
    char *vid_url;    /*视频的下载链接*/
    char *info;    /*2K 加密字段，从服务器得到后提供给APP自己解析*/
    unsigned long total_size;    /*文件总大小*/
    unsigned long downloaded_size;    /*已下载的大小*/
    unsigned longlast_update_time;    /*最后更新时间*/
    int download_status;    /*下载状态，参考交互文档中定义*/
    char *vid_path;    /*本地提供给APP的下载链接*/
    char *img_path;    /*本地提供给APP的下载链接*/
	char error_msg[128];
	int errorCode;

	struct task_dnode *dn;
};

#define THREAD_COUNT 1
#define DOCUMENT_ROOT "/tmp/mnt/"
#define TMP_PATH_NAME ".dmt"
#define CFG_PATH_NAME ".dmt.cfg"

#define CFG_PATH "/tmp/mnt/USB-disk-1/letv/.tasklist"


#define RECV_TIMEOUT        3000


#define INIT_PORT 13112



/*
 * SHTTPD context
 */
struct shttpd_ctx {
	int		nactive;	/* # of connections now		*/
	uint64_t	in, out;	/* IN/OUT traffic counters	*/
	struct llhead	connections;	/* List of connections		*/
	char	*document_root;		/* Document root		*/
	char	*ports;			/* Listening ports		*/
	int	io_buf_size;		/* IO buffer size		*/
	pthread_mutex_t mutex;
};

/*
 * In SHTTPD, list of values are represented as comma or space separated
 * string. For example, list of CGI extensions can be represented as
 * ".cgi,.php,.pl", or ".cgi .php .pl". The macro that follows allows to
 * loop through the individual values in that list.
 * A "const char *" pointer and size_t variable must be passed to the macro.
 * Spaces or commas can be used as delimiters (macro DELIM_CHARS)
 */
#define	FOR_EACH_WORD_IN_LIST(s,len)	\
	for (; s != NULL && (len = strcspn(s, DELIM_CHARS)) != 0; s += len + 1)
extern int db_get_mime_type(const char *uri, int len);
extern void	stop_stream(struct stream *stream);
extern void	open_listening_ports(struct shttpd_ctx *ctx);

extern struct shttpd_ctx *init_from_argc_argv();
extern int	my_strncasecmp(register const char *,
		register const char *, size_t);
extern int	my_snprintf(char *buf, size_t buflen, const char *fmt, ...);

extern void	set_close_on_exec(int fd);
extern int	set_non_blocking_mode(int fd);
extern int	my_stat(const char *, struct stat *stp);
extern int	my_open(const char *, int flags, int mode);

extern const struct io_class	io_file;
extern const struct io_class	io_socket;

extern int file_json_to_string(struct conn *c,JObj* response_json);



#endif /* DEFS_HEADER_DEFINED */
