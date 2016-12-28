#ifndef _EXIF_H_
#define _EXIF_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef enum {
	IMAGE_EXIF_IFD_0 = 0,                /*!< */
	IMAGE_EXIF_IFD_1,                    /*!< */
	IMAGE_EXIF_IFD_EXIF,                 /*!< */
	IMAGE_EXIF_IFD_GPS,                  /*!< */
	IMAGE_EXIF_IFD_INTEROPERABILITY,     /*!< */
	IMAGE_EXIF_IFD_COUNT                 /*!< Not a real value, just (max_value + 1). */
} ImageExifIfd;

typedef enum {
	Rotate_Multiple = 0,
	Rotate_Top_Left = 1,
	Rotate_Top_Right = 2,
	Rotate_Bottom_Right = 3,
	Rotate_Bottom_Left = 4,
	Rotate_Left_Top = 5,
	Rotate_Right_Top = 6,
	Rotate_Right_Bottom = 7,
	Rotate_Left_Bottom = 8,
	RoTate_None = 0xffff
}ImageRotateType;

typedef struct rotate_info{
 int type;
 char name[32];
}image_rotate_info;

int creat_exif_thumbnail(const char *src_file, char *thumbnail_file);
int get_exif_rotate_type(const char *src_file, int image_ifd_type);

#ifdef __cplusplus
}
#endif

#endif

