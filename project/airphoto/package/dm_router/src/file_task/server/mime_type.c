#include "defs.h"
//1£ºÊÓÆµ£¬2£ºÒôÆµ£¬3£ºÍ¼Æ¬£¬4£ºÎÄµµ
static const struct mime_type default_mime_types[] = {
	{"html",	4,	"text/html"			,4},
	{"htm",		3,	"text/html"			,4},
	{"txt",		3,	"text/plain"		,4	},
	{"css",		3,	"text/css"			,4},
	{"ico",		3,	"image/x-icon"		,3	},
	{"gif",		3,	"image/gif"			,3},
	{"jpg",		3,	"image/jpeg"		,3	},
	{"jpeg",	4,	"image/jpeg"		,3	},
	{"png",		3,	"image/png"			,3},
	{"svg",		3,	"image/svg+xml"		,3	},
	{"torrent",	7,	"application/x-bittorrent"	,4},
	{"wav",		3,	"audio/x-wav"			,2},
	{"mp3",		3,	"audio/x-mp3"			,2},
	{"mid",		3,	"audio/mid"			,2},
	{"m3u",		3,	"audio/x-mpegurl"	,2},
	{"ram",		3,	"audio/x-pn-realaudio"		,2},
	{"ra",		2,	"audio/x-pn-realaudio"		,2},
	{"doc",		3,	"application/msword"		,4},
	{"exe",		3,	"application/octet-stream"	,0},
	{"zip",		3,	"application/x-zip-compressed"	,0},
	{"xls",		3,	"application/excel"		,0},
	{"tgz",		3,	"application/x-tar-gz"		,0},
	{"tar.gz",	6,	"application/x-tar-gz"		,0},
	{"tar",		3,	"application/x-tar"		,0},
	{"gz",		2,	"application/x-gunzip"		,0},
	{"arj",		3,	"application/x-arj-compressed"	,0},
	{"rar",		3,	"application/x-arj-compressed"	,0},
	{"rtf",		3,	"application/rtf"		,0},
	{"pdf",		3,	"application/pdf"		,4},
	{"mpg",		3,	"video/mpeg"			,1},
	{"mpeg",	4,	"video/mpeg"			,1},
	{"asf",		3,	"video/x-ms-asf"		,1},
	{"avi",		3,	"video/x-msvideo"		,1},
	{"mp4",		3,	"video/x-msvideo"		,1},
	{"3gp",		3,	"video/x-msvideo"		,1},
	{"mkv",		3,	"video/x-msvideo"		,1},
	{"MKV",		3,	"video/x-msvideo"		,1},
	{"bmp",		3,	"image/bmp"			,3},
	{NULL,		0,	NULL				}
};

int db_get_mime_type(const char *uri, int len)
{
	struct llhead		*lp;
	const struct mime_type	*mt;
	const char		*s;
	/* If no luck, try built-in mime types */
	for (mt = default_mime_types; mt->ext != NULL; mt++) {
		s = uri + len - mt->ext_len;
		if (s > uri && s[-1] == '.' &&
		    !my_strncasecmp(mt->ext, s, mt->ext_len))
			return mt->file_type;
	}
	/* Oops. This extension is unknown to us. Fallback to text/plain */
	return 0;
}




