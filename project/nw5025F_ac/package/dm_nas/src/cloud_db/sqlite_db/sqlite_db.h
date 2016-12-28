/*
 * =============================================================================
 *
 *       Filename:  sqlite_db.h
 *
 *    Description:  create database file and create database table
 *
 *        Version:  1.0
 *        Created:  2016/10/19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oliver <henry.jin@dmsys.com>
 *   Organization:  longsys
 *
 * =============================================================================
 */


#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#ifdef __cplusplus
extern "C"{
#endif
#include "db_base.h"
#include "db_table.h"

#define SQLITE_CREATE_AUDIO_TABLE \
	"CREATE TABLE IF NOT EXISTS audio_table ( "\
    "ID       INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "DURATION INT,"\
    "NAME     CHAR NOT NULL,"\
    "ALBUM    CHAR,"\
    "ARTIST   CHAR,"\
    "ENCODE   CHAR,"\
    "BITRATE  INT );"

#define SQLITE_CREATE_AUTHORITY_TABLE \
	"CREATE TABLE IF NOT EXISTS authority_table ( "\
    "ID        INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "USER_ID   INT NOT NULL,"\
    "BUCKET_ID INT NOT NULL,"\
    "AUTHORITY INT NOT NULL DEFAULT ( 0 ));"

#define SQLITE_CREATE_BUCKET_TABLE \
	"CREATE TABLE IF NOT EXISTS bucket_table (" \
    "BUCKET_ID         INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "BUCKET_TABLE_NAME CHAR NOT NULL UNIQUE,"\
    "CREATE_USER_ID    INT  NOT NULL );"

#define SQLITE_CREATE_DISK_TABLE \
	"CREATE TABLE IF NOT EXISTS disk_table ( "\
    "DISK_ID       INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "UUID          CHAR NOT NULL UNIQUE,"\
    "CAPACITY      INT  NOT NULL,"\
    "FREE_CAPACITY INT  NOT NULL );"

#define SQLITE_CREATE_FILE_TABLE \
	"CREATE TABLE IF NOT EXISTS file_table ( "\
    "FILE_ID     INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "NAME        CHAR NOT NULL,"\
    "PATH        CHAR NOT NULL,"\
    "TYPE        INT  NOT NULL,"\
    "SIZE        INT  NOT NULL,"\
    "CREATE_TIME INT  NOT NULL,"\
    "MODIFY_TIME INT  NOT NULL,"\
    "ACCESS_TIME INT  NOT NULL,"\
    "UUID        CHAR NOT NULL,"\
    "LINK 		 INT  NOT NULL,"\
    "MEDIA_INDEX INT  NOT NULL,"\
    "THUM_INDEX  INT  NOT NULL);"

#define SQLITE_CREATE_THUMBNAIL_TABLE \
	"CREATE TABLE IF NOT EXISTS thumbnail_table ( "\
    "ID     	 INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "SMALL_PATH        CHAR NOT NULL,"\
    "MEDIAN_PATH       CHAR NOT NULL);"

#define SQLITE_CREATE_IMAGE_TABLE \
	"CREATE TABLE IF NOT EXISTS image_table ( "\
    "ID                INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "WIDTH             INT,"\
    "HEIGHT            INT,"\
    "TIME              INT,"\
    "VENDOR_NAME       CHAR NOT NULL,"\
    "CAMERA_TYPE       CHAR NOT NULL,"\
    "APERTURE_VALUE    CHAR NOT NULL,"\
    "EXPOSURE_TIME     CHAR NOT NULL,"\
    "ISO_VAL           CHAR NOT NULL,"\
    "EXPOSURE_BIAS_VAL CHAR NOT NULL,"\
    "FOCAL_LENGTH      CHAR NOT NULL,"\
    "MAX_APETURE_VAL   CHAR NOT NULL,"\
    "METER_MODE        CHAR NOT NULL,"\
    "FLASH             CHAR NOT NULL );"

#define SQLITE_CREATE_USER_TABLE \
	"CREATE TABLE IF NOT EXISTS user_table ("\
    "USER_ID   INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "USER_NAME VARCHAR NOT NULL UNIQUE,"\
    "PASSWORD  VARCHAR NOT NULL,"\
    "NICK_NAME VARCHAR );"

#define SQLITE_CREATE_V_FILE_TABLE \
	"CREATE TABLE IF NOT EXISTS v_file_table ( "\
    "FILE_ID      INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "FILE_NAME    CHAR NOT NULL,"\
    "FILE_PATH	  CHAR NULL,"\
    "REAL_FILE_ID INT  NOT NULL,"\
    "PARENT_ID    INT  NOT NULL,"\
    "IS_DIR       INT  NOT NULL,"\
    "TYPE        INT  NOT NULL,"\
    "CREATE_TIME INT  NOT NULL,"\
    "MODIFY_TIME INT  NOT NULL,"\
    "ACCESS_TIME INT  NOT NULL,"\
    "SIZE 		INT  NOT NULL);"

#define SQLITE_CREATE_VIDEO_TABLE \
	"CREATE TABLE IF NOT EXISTS video_table ( "\
    "ID       INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,"\
    "WIDTH    INT,"\
    "HEIGHT   INT,"\
    "BITRATE  INT,"\
    "ENCODE   CHAR,"\
    "DURATION INT );"

#define SQLITE_CREATE_VERSION_TABLE \
	"CREATE TABLE IF NOT EXISTS version_table ( "\
    "VERSION CHAR NOT NULL,"\
	"STATUS  INT  NOT NULL);"


void register_db_table_ops(void);

error_t db_module_init();

error_t v_file_table_create(char *buncket_name);





#ifdef __cplusplus
}
#endif



#endif
