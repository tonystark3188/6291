#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "gif_lib.h"
//#include "getarg.h"

#include "resize_gif_img.h"
#include "resize_base.h"


/******************************************************************************
* Close both input and output file (if open), and exit.			      *
******************************************************************************/
static int QuitGifError(GifFileType *GifFileIn, GifFileType *GifFileOut)
{
	int ErrorCode;
//    PrintGifError(ErrorCode);
    if (GifFileIn != NULL) DGifCloseFile(GifFileIn, &ErrorCode);
    if (GifFileOut != NULL) EGifCloseFile(GifFileOut, &ErrorCode);
    //exit(1);
	return -1;
}

/******************************************************************************
* Perform the disassembly operation - take one input files into few output.   *
******************************************************************************/
static int DoDisassemblyNum(const char *InFileName, char *OutFileName, int FileNum)
{
    int	ExtCode, CodeSize, FileEmpty;
    GifRecordType RecordType;
    char CrntFileName[80], *p;
    GifByteType *Extension, *CodeBlock;
    GifFileType *GifFileIn = NULL, *GifFileOut = NULL;
	int ErrorCode;

    /* If name has type postfix, strip it out, and make sure name is less    */
    /* or equal to 6 chars, so we will have 2 chars in name for numbers.     */
    //strupr(OutFileName);		      /* Make sure all is upper case */
	#if 0
	printf("OutFileName : %s\n", OutFileName);
	if ((p = strrchr(OutFileName, '.')) != NULL && strlen(p) <= 4) p[0] = 0;
    if ((p = strrchr(OutFileName, '/')) != NULL ||
	(p = strrchr(OutFileName, '\\')) != NULL ||
	(p = strrchr(OutFileName, ':')) != NULL) {
	if (strlen(p) > 7) p[7] = 0;   /* p includes the '/', '\\', ':' char */
    }
    else {
	/* Only name is given for current directory: */
	//if (strlen(OutFileName) > 6) OutFileName[6] = 0;
    }
	printf("OutFileName : %s\n", OutFileName);
	#endif

    /* Open input file: */
    if (InFileName != NULL) {
	if ((GifFileIn = DGifOpenFileName(InFileName, &ErrorCode)) == NULL)
	    return QuitGifError(GifFileIn, GifFileOut);
    }
    else {
	/* Use the stdin instead: */
	if ((GifFileIn = DGifOpenFileHandle(0, &ErrorCode)) == NULL)
	    return QuitGifError(GifFileIn, GifFileOut);
    }

    /* Scan the content of GIF file and dump image(s) to seperate file(s): */
	//sprintf(CrntFileName, "%s_%02d.gif", OutFileName, FileNum);
	sprintf(CrntFileName, "%s", OutFileName);
	if ((GifFileOut = EGifOpenFileName(CrntFileName, TRUE, &ErrorCode)) == NULL)
	    return QuitGifError(GifFileIn, GifFileOut);
	FileEmpty = TRUE;

	/* And dump out its exactly same screen information: */
	if (EGifPutScreenDesc(GifFileOut,
	    GifFileIn->SWidth, GifFileIn->SHeight,
	    GifFileIn->SColorResolution, GifFileIn->SBackGroundColor,
	    GifFileIn->SColorMap) == GIF_ERROR)
	    return QuitGifError(GifFileIn, GifFileOut);

	do {
	    if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
			return QuitGifError(GifFileIn, GifFileOut);

	    switch (RecordType) {
		case IMAGE_DESC_RECORD_TYPE:
		    FileEmpty = FALSE;
		    if (DGifGetImageDesc(GifFileIn) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    /* Put same image descriptor to out file: */
		    if (EGifPutImageDesc(GifFileOut,
			GifFileIn->Image.Left, GifFileIn->Image.Top,
			GifFileIn->Image.Width, GifFileIn->Image.Height,
			GifFileIn->Image.Interlace,
			GifFileIn->Image.ColorMap) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);

		    /* Now read image itself in decoded form as we dont      */
		    /* really care what is there, and this is much faster.   */
		    if (DGifGetCode(GifFileIn, &CodeSize, &CodeBlock) == GIF_ERROR
		     || EGifPutCode(GifFileOut, CodeSize, CodeBlock) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    while (CodeBlock != NULL)
			if (DGifGetCodeNext(GifFileIn, &CodeBlock) == GIF_ERROR ||
			    EGifPutCodeNext(GifFileOut, CodeBlock) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    break;
		case EXTENSION_RECORD_TYPE:
		    FileEmpty = FALSE;
		    /* Skip any extension blocks in file: */
		    if (DGifGetExtension(GifFileIn, &ExtCode, &Extension)
			== GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    if (EGifPutExtension(GifFileOut, ExtCode, Extension[0],
							Extension) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);

		    /* No support to more than one extension blocks, discard */
		    while (Extension != NULL)
			if (DGifGetExtensionNext(GifFileIn, &Extension)
			    == GIF_ERROR)
			    return QuitGifError(GifFileIn, GifFileOut);
		    break;
		case TERMINATE_RECORD_TYPE:
		    break;
		default:	     /* Should be traps by DGifGetRecordType */
		    break;
	    }
	}
	while (RecordType != IMAGE_DESC_RECORD_TYPE &&
	       RecordType != TERMINATE_RECORD_TYPE);

	if (EGifCloseFile(GifFileOut, &ErrorCode) == GIF_ERROR)
	    return QuitGifError(GifFileIn, GifFileOut);
	if (FileEmpty) {
	    /* Might happen on last file - delete it if so: */
	    unlink(CrntFileName);
	}

    if (DGifCloseFile(GifFileIn, &ErrorCode) == GIF_ERROR)
		return QuitGifError(GifFileIn, GifFileOut);
}


/******************************************************************************
* Perform the disassembly operation - take one input files into few output.   *
******************************************************************************/
static int DoDisassemblyAll(const char *InFileName, char *OutFileName)
{
    int	ExtCode, CodeSize, FileNum = 0, FileEmpty;
    GifRecordType RecordType;
    char CrntFileName[80], *p;
    GifByteType *Extension, *CodeBlock;
    GifFileType *GifFileIn = NULL, *GifFileOut = NULL;
	int ErrorCode;

    /* If name has type postfix, strip it out, and make sure name is less    */
    /* or equal to 6 chars, so we will have 2 chars in name for numbers.     */
    //strupr(OutFileName);		      /* Make sure all is upper case */
    if ((p = strrchr(OutFileName, '.')) != NULL && strlen(p) <= 4) p[0] = 0;
    if ((p = strrchr(OutFileName, '/')) != NULL ||
	(p = strrchr(OutFileName, '\\')) != NULL ||
	(p = strrchr(OutFileName, ':')) != NULL) {
	if (strlen(p) > 7) p[7] = 0;   /* p includes the '/', '\\', ':' char */
    }
    else {
	/* Only name is given for current directory: */
	if (strlen(OutFileName) > 6) OutFileName[6] = 0;
    }

    /* Open input file: */
    if (InFileName != NULL) {
	if ((GifFileIn = DGifOpenFileName(InFileName, &ErrorCode)) == NULL)
	    return QuitGifError(GifFileIn, GifFileOut);
    }
    else {
	/* Use the stdin instead: */
	if ((GifFileIn = DGifOpenFileHandle(0, &ErrorCode)) == NULL)
	    return QuitGifError(GifFileIn, GifFileOut);
    }

    /* Scan the content of GIF file and dump image(s) to seperate file(s): */
    do {
	sprintf(CrntFileName, "%s%02d.gif", OutFileName, FileNum++);
	if ((GifFileOut = EGifOpenFileName(CrntFileName, TRUE, &ErrorCode)) == NULL)
	    return QuitGifError(GifFileIn, GifFileOut);
	FileEmpty = TRUE;

	/* And dump out its exactly same screen information: */
	if (EGifPutScreenDesc(GifFileOut,
	    GifFileIn->SWidth, GifFileIn->SHeight,
	    GifFileIn->SColorResolution, GifFileIn->SBackGroundColor,
	    GifFileIn->SColorMap) == GIF_ERROR)
	    return QuitGifError(GifFileIn, GifFileOut);

	do {
	    if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
			return QuitGifError(GifFileIn, GifFileOut);

	    switch (RecordType) {
		case IMAGE_DESC_RECORD_TYPE:
		    FileEmpty = FALSE;
		    if (DGifGetImageDesc(GifFileIn) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    /* Put same image descriptor to out file: */
		    if (EGifPutImageDesc(GifFileOut,
			GifFileIn->Image.Left, GifFileIn->Image.Top,
			GifFileIn->Image.Width, GifFileIn->Image.Height,
			GifFileIn->Image.Interlace,
			GifFileIn->Image.ColorMap) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);

		    /* Now read image itself in decoded form as we dont      */
		    /* really care what is there, and this is much faster.   */
		    if (DGifGetCode(GifFileIn, &CodeSize, &CodeBlock) == GIF_ERROR
		     || EGifPutCode(GifFileOut, CodeSize, CodeBlock) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    while (CodeBlock != NULL)
			if (DGifGetCodeNext(GifFileIn, &CodeBlock) == GIF_ERROR ||
			    EGifPutCodeNext(GifFileOut, CodeBlock) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    break;
		case EXTENSION_RECORD_TYPE:
		    FileEmpty = FALSE;
		    /* Skip any extension blocks in file: */
		    if (DGifGetExtension(GifFileIn, &ExtCode, &Extension)
			== GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);
		    if (EGifPutExtension(GifFileOut, ExtCode, Extension[0],
							Extension) == GIF_ERROR)
				return QuitGifError(GifFileIn, GifFileOut);

		    /* No support to more than one extension blocks, discard */
		    while (Extension != NULL)
			if (DGifGetExtensionNext(GifFileIn, &Extension)
			    == GIF_ERROR)
			    return QuitGifError(GifFileIn, GifFileOut);
		    break;
		case TERMINATE_RECORD_TYPE:
		    break;
		default:	     /* Should be traps by DGifGetRecordType */
		    break;
	    }
	}
	while (RecordType != IMAGE_DESC_RECORD_TYPE &&
	       RecordType != TERMINATE_RECORD_TYPE);

	if (EGifCloseFile(GifFileOut, &ErrorCode) == GIF_ERROR)
	    return QuitGifError(GifFileIn, GifFileOut);
	if (FileEmpty) {
	    /* Might happen on last file - delete it if so: */
	    unlink(CrntFileName);
	}
   	}
    while (RecordType != TERMINATE_RECORD_TYPE);

    if (DGifCloseFile(GifFileIn, &ErrorCode) == GIF_ERROR)
		return QuitGifError(GifFileIn, GifFileOut);
}


/*
 *Desc: reduce image by request.
 *src_img_file: input, input image file name
 *p_request: input, point to the first request object, end with NULL pointer.
 *
 *Return: 1 : means reduce image success.
          0 : means not handle it.
         <0 : means error occure. 
 */
int reduce_gif_image(const char *src_img_file, ImgReduceRequest **p_request)
{
	int ret = 0;
	ImgReduceRequest **p_img_request = p_request;
	while((*p_img_request) != NULL){
		ret = DoDisassemblyNum(src_img_file, (*p_img_request)->img_name, 1);
		//if(ret)
			//break;
		p_img_request += 1;
	}
	return 0;
}



