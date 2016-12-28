#include "defs.h"


static const struct mime_type default_mime_types[] = {
    //{"html",	4,	"text/html"			,4},
	//{"wps",		3,	"text/wps"			,4},
    //{"htm",		3,	"text/html"			,4},
    {"txt",		3,	"text/plain"		,4	},
    //{"css",		3,	"text/css"			,4},
    //{"rar",		3,	"text/rar"			,4},
    //{"zip",		3,	"text/zip"			,4},
    //{"apk",		3,	"text/apk"			,4},
    {"doc",		3,	"application/msword"		,4},
    {"docx",	4,	"text/docx"			,4},
    {"pdf",		3,	"application/pdf"		,4},
    {"ppt",		3,	"text/ppt"			,4},
    {"pptx",	4,	"text/pptx"			,4},
    {"xls",		3,	"text/pptx"			,4},
    {"xlsx",	4,	"text/pptx"			,4},
    //{"rtf",		3,	"text/pptx"			,4},
    {"bmp",		3,	"image/bmp"			,3},
    {"pcx",		3,	"image/pcx"			,3},
    {"tga",		3,	"image/tga"			,3},
    {"ico",		3,	"image/x-icon"		,3	},
    {"gif",		3,	"image/gif"			,3},
    {"jpg",		3,	"image/jpeg"		,3	},
    {"jpeg",	4,	"image/jpeg"		,3	},
    {"png",		3,	"image/png"			,3},
    {"svg",		3,	"image/svg+xml"		,3	},
    {"tif",		3,	"image/tif"		,3	},
    //{"torrent",	7,	"application/x-bittorrent"	,4},
    {"wav",		3,	"audio/x-wav"			,2},
    {"mp3",		3,	"audio/x-mp3"			,2},
    {"mid",		3,	"audio/mid"			,2},
    {"m3u",		3,	"audio/x-mpegurl"	,2},
    {"ram",		3,	"audio/x-pn-realaudio"		,2},
    {"ra",		2,	"audio/x-pn-realaudio"		,2},
    {"ac3",		3,	"audio/ac3"			,2},
    {"wma",		3,	"audio/wma"			,2},
    {"flac",	4,	"audio/flac"			,2},
    {"aac",		3,	"audio/aac"			,2},
    {"m4a",		3,	"audio/mp3"			,2},
    {"ape",		3,	"audio/ape"			,2},
    {"dts",		3,	"audio/dts"			,2},
    {"m4r",		3,	"audio/m4r"			,2},
    {"mmf",		3,	"audio/mmf"			,2},
    {"mp2",		3,	"audio/mp2"			,2},
    {"ogg",		3,	"audio/ogg"			,2},
    
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
    {"mpg",		3,	"video/mpeg"			,1},
    {"mpeg",	4,	"video/mpeg"			,1},
    {"asf",		3,	"video/x-ms-asf"		,1},
    {"avi",		3,	"video/x-msvideo"		,1},
    {"mp4",		3,	"video/x-msvideo"		,1},
    {"3gp",		3,	"video/x-msvideo"		,1},
    {"mkv",		3,	"video/x-msvideo"		,1},
    {"MKV",		3,	"video/x-msvideo"		,1},
    {"flv",		3,	"video/x-msvideo"		,1},
    {"mov",		3,	"video/x-msvideo"		,1},
    {"rmvb",	4,	"video/x-msvideo"		,1},
    {"ts",		3,	"video/x-msvideo"		,1},
    {"wmv",		3,	"video/x-msvideo"		,1},
    {"vob",		3,	"video/x-msvideo"		,1},
    {"asf",		3,	"video/x-msvideo"		,1},
    {"swf",		3,	"video/x-msvideo"		,1},
    {"m2ts",	4,	"video/x-msvideo"		,1},
    {"dat",		3,	"video/dat"				,1},
    {"rm",		2,	"video/rm"				,1},
    {"tp",		2,	"video/tp"				,1},
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




