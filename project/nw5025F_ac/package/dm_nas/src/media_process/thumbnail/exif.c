#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <libexif/exif-loader.h>
#include "libexif/exif-data.h"
#include "transupp.h"
#include "exif.h"

int creat_exif_thumbnail(const char *src_file, char *thumbnail_file)
{
    int rc = 0;
    ExifLoader *l;

    if (src_file == NULL || thumbnail_file == NULL) {
        printf("Extracts a thumbnail from the given EXIF image.\n");
        return -1;
    }

    /* Create an ExifLoader object to manage the EXIF loading process */
    l = exif_loader_new();
    if (l) {
        ExifData *ed;

        /* Load the EXIF data from the image file */
        exif_loader_write_file(l, src_file);

        /* Get a pointer to the EXIF data */
        ed = exif_loader_get_data(l);

		/* The loader is no longer needed--free it */
        exif_loader_unref(l);
		l = NULL;
        if (ed) {
	    /* Make sure the image had a thumbnail before trying to write it */
            if (ed->data && ed->size) {
                FILE *thumb;

                thumb = fopen(thumbnail_file, "wb");
                if (thumb) {
		    		/* Write the thumbnail image to the file */
                    fwrite(ed->data, 1, ed->size, thumb);
                    fclose(thumb);
                    printf("Wrote thumbnail to %s\n", thumbnail_file);
                    rc = 0;
                } else {
                    printf("Could not create file %s\n", thumbnail_file);
                    rc = -1;
                }
            } else {
                printf("No EXIF thumbnail in file %s\n", thumbnail_file);
                rc = 1;
            }
	    	/* Free the EXIF data */
            exif_data_unref(ed);
        }
    }
    return rc;
}

image_rotate_info all_image_rotate_info[]=
{
	{Rotate_Multiple, ""},
	{Rotate_Top_Left, "Top-left"},
	{Rotate_Top_Right, "Top-right"},
	{Rotate_Bottom_Right, "Bottom-right"},
	{Rotate_Bottom_Left, "Bottom-left"},
	{Rotate_Left_Top, "Left-top"},
	{Rotate_Right_Top, "Right-top"},
	{Rotate_Right_Bottom, "Right-bottom"},
	{Rotate_Left_Bottom, "Left-bottom"},
	{RoTate_None, "none"},
};

#define IMAGE_ROTATE_INFO_NUM (sizeof(all_image_rotate_info)/sizeof(all_image_rotate_info[0]))

int get_exif_rotate_type(const char *src_file, int image_ifd_type)
{
	ExifData *d = NULL;
	ExifContent *ifd = NULL;
	ExifEntry *entry;
	int i = 0, j = 0;
	int rotate_type = 0;  
	char buf[2000];

	if(image_ifd_type > EXIF_IFD_COUNT){
		printf("error idf type: %d\n", image_ifd_type);
		return -1;
	}
	
	d = exif_data_new_from_file(src_file);
	if(d == NULL){
		printf("get exif error\n");
		return -1;
	}
	ifd = d->ifd[image_ifd_type];
	//printf("count: %d\n", ifd->count);

	for(i = 0; i < ifd->count && !rotate_type; i++){
		entry = ifd->entries[i];	
		if(entry->tag == EXIF_TAG_ORIENTATION){
			memset(buf, 0, sizeof(buf));
			printf("%s: %s\n", exif_tag_get_name_in_ifd(entry->tag, image_ifd_type), exif_entry_get_value(entry, buf, sizeof(buf)));
			for(j = 0; j < IMAGE_ROTATE_INFO_NUM; j++){
				memset(buf, 0, sizeof(buf));
				if(!strcmp(all_image_rotate_info[j].name, exif_entry_get_value(entry, buf, sizeof(buf)))){
					rotate_type = all_image_rotate_info[j].type;
					break;
				}
			}
		}

		#if 0
		memset(buf, 0, sizeof(buf));
		entry = ifd->entries[i];		  
		exif_entry_get_value(entry, buf, sizeof(buf));
		printf("    Entry %p: %s (%s)\n"
			"      Size, Comps: %d, %d\n"
			"      Value: %s\n", 
			entry,
			exif_tag_get_name(entry->tag),
			exif_format_get_name(entry->format),
			entry->size,
			(int)(entry->components),
			exif_entry_get_value(entry, buf, sizeof(buf)));
		#endif
	}
	exif_data_unref(d);
	return rotate_type;
}


int rotate_jpeg_image(const char *src_file, int jxform_value)
{
	struct jpeg_decompress_struct srcinfo;
	struct jpeg_compress_struct dstinfo;
	struct jpeg_error_mgr jsrcerr, jdsterr;
	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;
	int file_index;
	static JCOPY_OPTION copyoption;	/* -copy switch */
	static jpeg_transform_info transformoption; /* image transformation options */
	/* We assume all-in-memory processing and can therefore use only a
	* single file pointer for sequential input and output operation. 
	*/
	FILE * fp;

	/* Initialize the JPEG decompression object with default error handling. */
	srcinfo.err = jpeg_std_error(&jsrcerr);
	jpeg_create_decompress(&srcinfo);
	/* Initialize the JPEG compression object with default error handling. */
	dstinfo.err = jpeg_std_error(&jdsterr);
	jpeg_create_compress(&dstinfo);

	//transformoption.transform = JXFORM_ROT_180;
	transformoption.transform = jxform_value;
	jsrcerr.trace_level = jdsterr.trace_level;
	srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;
	printf("111transformoption.transform: %d\n", transformoption.transform);

	if ((fp = fopen(src_file, "rb")) == NULL) {
	  fprintf(stderr, "%s: can't open %s for reading\n", src_file);
	  return -1;
	}

	/* Specify data source for decompression */
	jpeg_stdio_src(&srcinfo, fp);

	/* Enable saving of extra markers that we want to copy */
	jcopy_markers_setup(&srcinfo, copyoption);

	/* Read file header */
	(void) jpeg_read_header(&srcinfo, TRUE);

#if TRANSFORMS_SUPPORTED
	jtransform_request_workspace(&srcinfo, &transformoption);
#endif
	
	/* Read source file as DCT coefficients */
	src_coef_arrays = jpeg_read_coefficients(&srcinfo);
	
	/* Initialize destination compression parameters from source values */
	jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

	//dstinfo.image_height = srcinfo.image_height/16*16;
	//dstinfo.image_width = srcinfo.image_width/16*16;
	
	printf("dstinfo.image_height: %d, dstinfo.image_width: %d\n", dstinfo.image_height, dstinfo.image_width);
	/* Adjust destination parameters if required by transform options;
	* also find out which set of coefficient arrays will hold the output.
	*/
	
#if TRANSFORMS_SUPPORTED
	dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
						 src_coef_arrays,
						 &transformoption);
#else
	dst_coef_arrays = src_coef_arrays;
#endif

#if TRANSFORMS_SUPPORTED
	printf("image_width: %d, image_height: %d, max_h_samp_factor * DCTSIZE: %d, max_v_samp_factor * DCTSIZE: %d\n",
		srcinfo.image_width, srcinfo.image_height, srcinfo.max_h_samp_factor * DCTSIZE, srcinfo.max_v_samp_factor * DCTSIZE);
	if (!jtransform_perfect_transform(srcinfo.image_width, srcinfo.image_height,
	  srcinfo.max_h_samp_factor * DCTSIZE, srcinfo.max_v_samp_factor * DCTSIZE,
	  transformoption.transform)) {
		fprintf(stderr, "transformation is not perfect\n");
		switch (transformoption.transform) {
			case JXFORM_FLIP_H:
			case JXFORM_ROT_270:
				dstinfo.image_height = dstinfo.image_height/16*16;
				break;
			case JXFORM_FLIP_V:
			case JXFORM_ROT_90:
				dstinfo.image_width = dstinfo.image_height/16*16;
				break;
			case JXFORM_TRANSVERSE:
			case JXFORM_ROT_180:
				dstinfo.image_height = dstinfo.image_height/16*16;
				dstinfo.image_width = dstinfo.image_width/16*16;
				break;
		}
	}
	printf("dstinfo.image_height: %d, dstinfo.image_width: %d\n", dstinfo.image_height, dstinfo.image_width);
#endif
	
	if (fp != NULL)
		fclose(fp);

	/* Open the output file. */
	//if ((fp = fopen((*p_request)->img_name, "wb")) == NULL) {
	if ((fp = fopen(src_file, "wb")) == NULL) {
		fprintf(stderr, "can't open %s for writing\n", src_file);
	  	return -1;
	}

	/* Specify data destination for compression */
	jpeg_stdio_dest(&dstinfo, fp);
	
	/* Start compressor (note no image data is actually written here) */
	jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

	/* Copy to the output file any extra markers that we want to preserve */
	jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

	/* Execute image transformation, if any */	
#if TRANSFORMS_SUPPORTED
	jtransform_execute_transformation(&srcinfo, &dstinfo,
				    src_coef_arrays,
				    &transformoption);
#endif

	printf("dstinfo.image_height: %d, dstinfo.image_width: %d\n", dstinfo.image_height, dstinfo.image_width);
	/* Finish compression and release memory */
	jpeg_finish_compress(&dstinfo);
	jpeg_destroy_compress(&dstinfo);
	(void) jpeg_finish_decompress(&srcinfo);
	jpeg_destroy_decompress(&srcinfo);

	/* Close output file, if we opened it */
	if (fp != stdout)
	fclose(fp);

	return 0;			/* suppress no-return-value warnings */
}

