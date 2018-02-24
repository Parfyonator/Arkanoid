/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.


    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


	source: 
		BitmapCodec "gif"

	revision history:
		Mar/10/2003 - Jukka Liimatta - initial revision

	notes:
		- uses "Gif-Lib" original library as foundation
		- decoding process ripped from the corona image library written by Chad Austin
		- corona homepage, http://corona.sf.net/
*/
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// Gif-Lib original code - adapted to prcore
// =================================================

/******************************************************************************
* In order to make life a little bit easier when using the GIF file format,   *
* this library was written, and which does all the dirty work...              *
*                                                                             *
*                                        Written by Gershon Elber,  Jun. 1989 *
*                                        Hacks by Eric S. Raymond,  Sep. 1992 *
*******************************************************************************
* History:                                                                    *
* 14 Jun 89 - Version 1.0 by Gershon Elber.                                   *
*  3 Sep 90 - Version 1.1 by Gershon Elber (Support for Gif89, Unique names)  *
* 15 Sep 90 - Version 2.0 by Eric S. Raymond (Changes to suoport GIF slurp)   *
* 26 Jun 96 - Version 3.0 by Eric S. Raymond (Full GIF89 support)             * 
* 17 Dec 98 - Version 4.0 by Toshio Kuratomi (Fix extension writing code)     *
******************************************************************************/

#define	GIF_ERROR	0
#define GIF_OK		1

typedef	unsigned char	GifPixelType;
typedef unsigned char	GifByteType;


#ifdef SYSV
#define VoidPtr char *
#else
#define VoidPtr void *
#endif // SYSV

typedef struct GifColorType {
    GifByteType Red, Green, Blue;
} GifColorType;

typedef struct ColorMapObject
{
    int	ColorCount;
    int BitsPerPixel;
    GifColorType *Colors;		/* on malloc(3) heap */
}
ColorMapObject;

typedef struct GifImageDesc {
    int Left, Top, Width, Height,	/* Current image dimensions. */
	Interlace;			/* Sequential/Interlaced lines. */
    ColorMapObject *ColorMap;		/* The local color map */
} GifImageDesc;

typedef struct GifFileType {
    int SWidth, SHeight,		/* Screen dimensions. */
	SColorResolution, 		/* How many colors can we generate? */
	SBackGroundColor;		/* I hope you understand this one... */
    ColorMapObject *SColorMap;		/* NULL if not exists. */
    int ImageCount;			/* Number of current image */
    GifImageDesc Image;			/* Block describing current image */
    struct SavedImage *SavedImages;	/* Use this to accumulate file state */
	Stream* stream;
    VoidPtr Private;	  		/* Don't mess with this! */
} GifFileType;

typedef enum {
    UNDEFINED_RECORD_TYPE,
    SCREEN_DESC_RECORD_TYPE,
    IMAGE_DESC_RECORD_TYPE,		/* Begin with ',' */
    EXTENSION_RECORD_TYPE,		/* Begin with '!' */
    TERMINATE_RECORD_TYPE		/* Begin with ';' */
} GifRecordType;

/******************************************************************************
*  GIF89 extension function codes                                             *
******************************************************************************/

#define COMMENT_EXT_FUNC_CODE		0xfe	/* comment */
#define GRAPHICS_EXT_FUNC_CODE		0xf9	/* graphics control */
#define PLAINTEXT_EXT_FUNC_CODE		0x01	/* plaintext */
#define APPLICATION_EXT_FUNC_CODE	0xff	/* application block */

#define	E_GIF_ERR_OPEN_FAILED	1
#define	E_GIF_ERR_WRITE_FAILED	2
#define E_GIF_ERR_HAS_SCRN_DSCR	3
#define E_GIF_ERR_HAS_IMAG_DSCR	4
#define E_GIF_ERR_NO_COLOR_MAP	5
#define E_GIF_ERR_DATA_TOO_BIG	6
#define E_GIF_ERR_NOT_ENOUGH_MEM 7
#define E_GIF_ERR_DISK_IS_FULL	8
#define E_GIF_ERR_CLOSE_FAILED	9
#define E_GIF_ERR_NOT_WRITEABLE	10

/******************************************************************************
* O.K., here are the routines one can access in order to decode GIF file:     *
* (GIF_LIB file DGIF_LIB.C).						      *
******************************************************************************/

static GifFileType* DGifOpen(Stream* stream);
static int DGifSlurp(GifFileType *GifFile);
static int DGifGetScreenDesc(GifFileType *GifFile);
static int DGifGetRecordType(GifFileType *GifFile, GifRecordType *GifType);
static int DGifGetImageDesc(GifFileType *GifFile);
static int DGifGetLine(GifFileType *GifFile, GifPixelType *GifLine, int GifLineLen);
static int DGifGetPixel(GifFileType *GifFile, GifPixelType GifPixel);
static int DGifGetExtension(GifFileType *GifFile, int *GifExtCode,	GifByteType **GifExtension);
static int DGifGetExtensionNext(GifFileType *GifFile, GifByteType **GifExtension);
static int DGifGetCode(GifFileType *GifFile, int *GifCodeSize,	GifByteType **GifCodeBlock);
static int DGifGetCodeNext(GifFileType *GifFile, GifByteType **GifCodeBlock);
static int DGifGetLZCodes(GifFileType *GifFile, int *GifCode);
static int DGifCloseFile(GifFileType *GifFile);

#define	D_GIF_ERR_OPEN_FAILED	101
#define	D_GIF_ERR_READ_FAILED	102
#define	D_GIF_ERR_NOT_GIF_FILE	103
#define D_GIF_ERR_NO_SCRN_DSCR	104
#define D_GIF_ERR_NO_IMAG_DSCR	105
#define D_GIF_ERR_NO_COLOR_MAP	106
#define D_GIF_ERR_WRONG_RECORD	107
#define D_GIF_ERR_DATA_TOO_BIG	108
#define D_GIF_ERR_NOT_ENOUGH_MEM 109
#define D_GIF_ERR_CLOSE_FAILED	110
#define D_GIF_ERR_NOT_READABLE	111
#define D_GIF_ERR_IMAGE_DEFECT	112
#define D_GIF_ERR_EOF_TOO_SOON	113


/*****************************************************************************
 *
 * Everything below this point is new after version 1.2, supporting `slurp
 * mode' for doing I/O in two big belts with all the image-bashing in core.
 *
 *****************************************************************************/

/******************************************************************************
* Color Map handling from ALLOCGIF.C					      *
******************************************************************************/

static ColorMapObject*	MakeMapObject(int ColorCount, GifColorType *ColorMap);
static void				FreeMapObject(ColorMapObject *Object);
static ColorMapObject*	UnionColorMap(ColorMapObject *ColorIn1, ColorMapObject *ColorIn2,GifPixelType ColorTransIn2[]);
static int				BitSize(int n);

/******************************************************************************
* Support for the in-core structures allocation (slurp mode).		      *
******************************************************************************/

// This is the in-core version of an extension record
typedef struct {
    int		ByteCount;
    char	*Bytes;		// on malloc(3) heap
    int Function;       // Holds the type of the Extension block.
} ExtensionBlock;

// This holds an image header, its unpacked raster bits, and extensions
typedef struct SavedImage {
    GifImageDesc	ImageDesc;

    char		*RasterBits;		// on malloc(3) heap

    int			Function; // DEPRECATED: Use ExtensionBlocks[x].Function instead
    int			ExtensionBlockCount;
    ExtensionBlock	*ExtensionBlocks;	// on malloc(3) heap
} SavedImage;

static void			ApplyTranslation(SavedImage *Image, GifPixelType Translation[]);
static void			MakeExtension(SavedImage *New, int Function);
static int			AddExtensionBlock(SavedImage *New, int Len, char ExtData[]);
static void			FreeExtension(SavedImage *Image);
static SavedImage*	MakeSavedImage(GifFileType *GifFile, SavedImage *CopyFrom);
static void			FreeSavedImages(GifFileType *GifFile);

// ======================================================

#define LZ_MAX_CODE	4095		/* Biggest code possible in 12 bits. */
#define LZ_BITS		12

#define FLUSH_OUTPUT		4096    /* Impossible code, to signal flush. */
#define FIRST_CODE		4097    /* Impossible code, to signal first. */
#define NO_SUCH_CODE		4098    /* Impossible code, to signal empty. */

#define FILE_STATE_WRITE    0x01
#define FILE_STATE_SCREEN   0x02
#define FILE_STATE_IMAGE    0x04
#define FILE_STATE_READ     0x08

#define IS_READABLE(Private)    (Private->FileState & FILE_STATE_READ)
#define IS_WRITEABLE(Private)   (Private->FileState & FILE_STATE_WRITE)


typedef struct GifFilePrivateType {
    int FileState,
	BitsPerPixel,	    /* Bits per pixel (Codes uses at least this + 1). */
	ClearCode,				       /* The CLEAR LZ code. */
	EOFCode,				         /* The EOF LZ code. */
	RunningCode,		    /* The next code algorithm can generate. */
	RunningBits,/* The number of bits required to represent RunningCode. */
	MaxCode1,  /* 1 bigger than max. possible code, in RunningBits bits. */
	LastCode,		        /* The code before the current code. */
	CrntCode,				  /* Current algorithm code. */
	StackPtr,		         /* For character stack (see below). */
	CrntShiftState;		        /* Number of bits in CrntShiftDWord. */
    unsigned long CrntShiftDWord;     /* For bytes decomposition into codes. */
    unsigned long PixelCount;		       /* Number of pixels in image. */
	Stream* stream;
    GifByteType Buf[256];	       /* Compressed input is buffered here. */
    GifByteType Stack[LZ_MAX_CODE];	 /* Decoded pixels are stacked here. */
    GifByteType Suffix[LZ_MAX_CODE+1];	       /* So we can trace the codes. */
    unsigned int Prefix[LZ_MAX_CODE+1];
} GifFilePrivateType;


// ======================================================

/******************************************************************************
*   "Gif-Lib" - Yet another gif library.				      *
*									      *
* Written by:  Gershon Elber			IBM PC Ver 1.1,	Aug. 1990     *
*******************************************************************************
* The kernel of the GIF Decoding process can be found here.		      *
*******************************************************************************
* History:								      *
* 16 Jun 89 - Version 1.0 by Gershon Elber.				      *
*  3 Sep 90 - Version 1.1 by Gershon Elber (Support for Gif89, Unique names). *
******************************************************************************/

static int _GifError = 0;



#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define COMMENT_EXT_FUNC_CODE	0xfe /* Extension function code for comment. */
#define GIF_STAMP	"GIFVER"	 /* First chars in file - GIF stamp. */
#define GIF_STAMP_LEN	sizeof(GIF_STAMP) - 1
#define GIF_VERSION_POS	3		/* Version first character in stamp. */

// avoid extra function call in case we use fread (TVT)
#define READ(_gif,_buf,_len) ((GifFilePrivateType*)_gif->Private)->stream->Read(_buf,_len)


static int DGifGetWord(GifFileType *GifFile, int *Word);
static int DGifSetupDecompress(GifFileType *GifFile);
static int DGifDecompressLine(GifFileType *GifFile, GifPixelType *Line,	int LineLen);
static int DGifGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode);
static int DGifDecompressInput(GifFileType *GifFile, int *Code);
static int DGifBufferedInput(GifFileType *GifFile, GifByteType *Buf, GifByteType *NextByte);

/******************************************************************************
* GifFileType constructor with user supplied input function (TVT)             *
*                                                                             *
******************************************************************************/
static GifFileType* DGifOpen(Stream* stream)
{
    char Buf[GIF_STAMP_LEN+1];
    GifFileType* GifFile;
    GifFilePrivateType* Private;


    if ((GifFile = (GifFileType *) malloc(sizeof(GifFileType))) == NULL)
	{
	  _GifError = D_GIF_ERR_NOT_ENOUGH_MEM;
	  return NULL;
    }

    memset(GifFile, '\0', sizeof(GifFileType));

    if (!(Private = (GifFilePrivateType*) malloc(sizeof(GifFilePrivateType))))
	{
	  _GifError = D_GIF_ERR_NOT_ENOUGH_MEM;
	  free((char *) GifFile);
	  return NULL;
    }

    GifFile->Private = (VoidPtr) Private;
    Private->FileState = FILE_STATE_READ;
	Private->stream = stream;
	GifFile->stream = stream;    /* TVT */

    /* Lets see if this is a GIF file: */
    if ( READ( GifFile, (GifByteType*)Buf, GIF_STAMP_LEN) != GIF_STAMP_LEN) 
	{
	  _GifError = D_GIF_ERR_READ_FAILED;
	  free((char *) Private);
	  free((char *) GifFile);
	  return NULL;
    }

    /* The GIF Version number is ignored at this time. Maybe we should do    */
    /* something more useful with it.					     */
    Buf[GIF_STAMP_LEN] = 0;
    if (strncmp(GIF_STAMP, Buf, GIF_VERSION_POS) != 0) 
	{
	  _GifError = D_GIF_ERR_NOT_GIF_FILE;
	  free((char *) Private);
	  free((char *) GifFile);
	  return NULL;
    }

    if (DGifGetScreenDesc(GifFile) == GIF_ERROR) {
	  free((char *) Private);
	  free((char *) GifFile);
	  return NULL;
    }

    _GifError = 0;

    return GifFile;
}

/******************************************************************************
*   This routine should be called before any other DGif calls. Note that      *
* this routine is called automatically from DGif file open routines.	      *
******************************************************************************/
static int DGifGetScreenDesc(GifFileType *GifFile)
{
    int i, BitsPerPixel;
    GifByteType Buf[3];
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) 
	{
		/* This file was NOT open for reading: */
		_GifError = D_GIF_ERR_NOT_READABLE;
		return GIF_ERROR;
    }

    /* Put the screen descriptor into the file: */
    if (DGifGetWord(GifFile, &GifFile->SWidth) == GIF_ERROR ||
	DGifGetWord(GifFile, &GifFile->SHeight) == GIF_ERROR)
	return GIF_ERROR;

    if (READ( GifFile, Buf, 3) != 3) 
	{
		_GifError = D_GIF_ERR_READ_FAILED;
		return GIF_ERROR;
    }
    GifFile->SColorResolution = (((Buf[0] & 0x70) + 1) >> 4) + 1;
    BitsPerPixel = (Buf[0] & 0x07) + 1;
    GifFile->SBackGroundColor = Buf[1];
    if (Buf[0] & 0x80) {		     /* Do we have global color map? */

	GifFile->SColorMap = MakeMapObject(1 << BitsPerPixel, NULL);

	/* Get the global color map: */
	for (i = 0; i < GifFile->SColorMap->ColorCount; i++) {
	    if (READ(GifFile, Buf, 3) != 3) 
		{
			_GifError = D_GIF_ERR_READ_FAILED;
			return GIF_ERROR;
	    }
	    GifFile->SColorMap->Colors[i].Red   = Buf[0];
	    GifFile->SColorMap->Colors[i].Green = Buf[1];
	    GifFile->SColorMap->Colors[i].Blue  = Buf[2];
	}
    }

    return GIF_OK;
}

/******************************************************************************
*   This routine should be called before any attemp to read an image.         *
******************************************************************************/
static int DGifGetRecordType(GifFileType *GifFile, GifRecordType *Type)
{
    GifByteType Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

    if (READ( GifFile, &Buf, 1) != 1) {
	_GifError = D_GIF_ERR_READ_FAILED;
	return GIF_ERROR;
    }

    switch (Buf) {
	case ',':
	    *Type = IMAGE_DESC_RECORD_TYPE;
	    break;
	case '!':
	    *Type = EXTENSION_RECORD_TYPE;
	    break;
	case ';':
	    *Type = TERMINATE_RECORD_TYPE;
	    break;
	default:
	    *Type = UNDEFINED_RECORD_TYPE;
	    _GifError = D_GIF_ERR_WRONG_RECORD;
	    return GIF_ERROR;
    }

    return GIF_OK;
}

/******************************************************************************
*   This routine should be called before any attemp to read an image.         *
*   Note it is assumed the Image desc. header (',') has been read.	      *
******************************************************************************/
static int DGifGetImageDesc(GifFileType *GifFile)
{
    int i, BitsPerPixel;
    GifByteType Buf[3];
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;
	SavedImage	*sp;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

    if (DGifGetWord(GifFile, &GifFile->Image.Left) == GIF_ERROR ||
	DGifGetWord(GifFile, &GifFile->Image.Top) == GIF_ERROR ||
	DGifGetWord(GifFile, &GifFile->Image.Width) == GIF_ERROR ||
	DGifGetWord(GifFile, &GifFile->Image.Height) == GIF_ERROR)
	return GIF_ERROR;
    if (READ(GifFile,Buf, 1) != 1) {
	_GifError = D_GIF_ERR_READ_FAILED;
	return GIF_ERROR;
    }
    BitsPerPixel = (Buf[0] & 0x07) + 1;
    GifFile->Image.Interlace = (Buf[0] & 0x40);
    if (Buf[0] & 0x80) {	    /* Does this image have local color map? */

	if (GifFile->Image.ColorMap && GifFile->SavedImages == NULL)
	    FreeMapObject(GifFile->Image.ColorMap);

	GifFile->Image.ColorMap = MakeMapObject(1 << BitsPerPixel, NULL);
    
	/* Get the image local color map: */
	for (i = 0; i < GifFile->Image.ColorMap->ColorCount; i++) {
	    if (READ(GifFile,Buf, 3) != 3) {
		_GifError = D_GIF_ERR_READ_FAILED;
		return GIF_ERROR;
	    }
	    GifFile->Image.ColorMap->Colors[i].Red = Buf[0];
	    GifFile->Image.ColorMap->Colors[i].Green = Buf[1];
	    GifFile->Image.ColorMap->Colors[i].Blue = Buf[2];
	}
    }

    if (GifFile->SavedImages) {
	    if ((GifFile->SavedImages = (SavedImage *)realloc(GifFile->SavedImages,
		     sizeof(SavedImage) * (GifFile->ImageCount + 1))) == NULL) {
	        _GifError = D_GIF_ERR_NOT_ENOUGH_MEM;
	        return GIF_ERROR;
	    }
    } else {
        if ((GifFile->SavedImages =
             (SavedImage *)malloc(sizeof(SavedImage))) == NULL) {
            _GifError = D_GIF_ERR_NOT_ENOUGH_MEM;
            return GIF_ERROR;
        }
    }

	sp = &GifFile->SavedImages[GifFile->ImageCount];
	memcpy(&sp->ImageDesc, &GifFile->Image, sizeof(GifImageDesc));
    if (GifFile->Image.ColorMap != NULL) {
	    sp->ImageDesc.ColorMap =
               (ColorMapObject *)malloc(sizeof (ColorMapObject));
	    memcpy(&sp->ImageDesc.ColorMap, &GifFile->Image.ColorMap,
               sizeof(ColorMapObject));
	    sp->ImageDesc.ColorMap->Colors =
               (GifColorType *)malloc(sizeof (GifColorType));
	    memcpy(&sp->ImageDesc.ColorMap->Colors,
               &GifFile->Image.ColorMap->Colors, sizeof(GifColorType));
    }
	sp->RasterBits = (char *)NULL;
	sp->ExtensionBlockCount = 0;
	sp->ExtensionBlocks = (ExtensionBlock *)NULL;

    GifFile->ImageCount++;

    Private->PixelCount = (long) GifFile->Image.Width *
			    (long) GifFile->Image.Height;

    DGifSetupDecompress(GifFile);  /* Reset decompress algorithm parameters. */

    return GIF_OK;
}

/******************************************************************************
*  Get one full scanned line (Line) of length LineLen from GIF file.	      *
******************************************************************************/
static int DGifGetLine(GifFileType *GifFile, GifPixelType *Line, int LineLen)
{
    GifByteType *Dummy;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

    if (!LineLen) LineLen = GifFile->Image.Width;

#if defined(__MSDOS__) || defined(__GNUC__)
    if ((Private->PixelCount -= LineLen) > 0xffff0000UL) {
#else
    if ((Private->PixelCount -= LineLen) > 0xffff0000) {
#endif /* __MSDOS__ */
	_GifError = D_GIF_ERR_DATA_TOO_BIG;
	return GIF_ERROR;
    }

    if (DGifDecompressLine(GifFile, Line, LineLen) == GIF_OK) {
	if (Private->PixelCount == 0) {
	    /* We probably would not be called any more, so lets clean 	     */
	    /* everything before we return: need to flush out all rest of    */
	    /* image until empty block (size 0) detected. We use GetCodeNext.*/
	    do if (DGifGetCodeNext(GifFile, &Dummy) == GIF_ERROR)
		return GIF_ERROR;
	    while (Dummy != NULL);
	}
	return GIF_OK;
    }
    else
	return GIF_ERROR;
}

/******************************************************************************
* Put one pixel (Pixel) into GIF file.					      *
******************************************************************************/
static int DGifGetPixel(GifFileType *GifFile, GifPixelType Pixel)
{
    GifByteType *Dummy;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

#if defined(__MSDOS__) || defined(__GNUC__)
    if (--Private->PixelCount > 0xffff0000UL)
#else
    if (--Private->PixelCount > 0xffff0000)
#endif /* __MSDOS__ */
    {
	_GifError = D_GIF_ERR_DATA_TOO_BIG;
	return GIF_ERROR;
    }

    if (DGifDecompressLine(GifFile, &Pixel, 1) == GIF_OK) {
	if (Private->PixelCount == 0) {
	    /* We probably would not be called any more, so lets clean 	     */
	    /* everything before we return: need to flush out all rest of    */
	    /* image until empty block (size 0) detected. We use GetCodeNext.*/
	    do if (DGifGetCodeNext(GifFile, &Dummy) == GIF_ERROR)
		return GIF_ERROR;
	    while (Dummy != NULL);
	}
	return GIF_OK;
    }
    else
	return GIF_ERROR;
}

/******************************************************************************
*   Get an extension block (see GIF manual) from gif file. This routine only  *
* returns the first data block, and DGifGetExtensionNext shouldbe called      *
* after this one until NULL extension is returned.			      *
*   The Extension should NOT be freed by the user (not dynamically allocated).*
*   Note it is assumed the Extension desc. header ('!') has been read.	      *
******************************************************************************/
static int DGifGetExtension(GifFileType *GifFile, int *ExtCode,
						    GifByteType **Extension)
{
    GifByteType Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

    if (READ(GifFile,&Buf, 1) != 1) {
	_GifError = D_GIF_ERR_READ_FAILED;
	return GIF_ERROR;
    }
    *ExtCode = Buf;

    return DGifGetExtensionNext(GifFile, Extension);
}

/******************************************************************************
*   Get a following extension block (see GIF manual) from gif file. This      *
* routine sould be called until NULL Extension is returned.		      *
*   The Extension should NOT be freed by the user (not dynamically allocated).*
******************************************************************************/
static int DGifGetExtensionNext(GifFileType *GifFile, GifByteType **Extension)
{
    GifByteType Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (READ(GifFile,&Buf, 1) != 1) {
	_GifError = D_GIF_ERR_READ_FAILED;
	return GIF_ERROR;
    }
    if (Buf > 0) {
	*Extension = Private->Buf;           /* Use private unused buffer. */
	(*Extension)[0] = Buf;  /* Pascal strings notation (pos. 0 is len.). */
	if (READ(GifFile,&((*Extension)[1]), Buf) != Buf) {
	    _GifError = D_GIF_ERR_READ_FAILED;
	    return GIF_ERROR;
	}
    }
    else
	*Extension = NULL;

    return GIF_OK;
}

/******************************************************************************
*   This routine should be called last, to close the GIF file.		      *
******************************************************************************/
static int DGifCloseFile(GifFileType *GifFile)
{
    GifFilePrivateType *Private;

    if (GifFile == NULL) return GIF_ERROR;

    Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

    if (GifFile->Image.ColorMap)
	FreeMapObject(GifFile->Image.ColorMap);
    if (GifFile->SColorMap)
	FreeMapObject(GifFile->SColorMap);
    if (Private)
	free((char *) Private);
    if (GifFile->SavedImages)
	FreeSavedImages(GifFile);
    free(GifFile);

    return GIF_OK;
}

/******************************************************************************
*   Get 2 bytes (word) from the given file:				      *
******************************************************************************/
static int DGifGetWord(GifFileType *GifFile, int *Word)
{
    unsigned char c[2];

    if (READ(GifFile,c, 2) != 2) {
	_GifError = D_GIF_ERR_READ_FAILED;
	return GIF_ERROR;
    }

    *Word = (((unsigned int) c[1]) << 8) + c[0];
    return GIF_OK;
}

/******************************************************************************
*   Get the image code in compressed form.  his routine can be called if the  *
* information needed to be piped out as is. Obviously this is much faster     *
* than decoding and encoding again. This routine should be followed by calls  *
* to DGifGetCodeNext, until NULL block is returned.			      *
*   The block should NOT be freed by the user (not dynamically allocated).    *
******************************************************************************/
static int DGifGetCode(GifFileType *GifFile, int *CodeSize, GifByteType **CodeBlock)
{
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

    *CodeSize = Private->BitsPerPixel;

    return DGifGetCodeNext(GifFile, CodeBlock);
}

/******************************************************************************
*   Continue to get the image code in compressed form. This routine should be *
* called until NULL block is returned.					      *
*   The block should NOT be freed by the user (not dynamically allocated).    *
******************************************************************************/
static int DGifGetCodeNext(GifFileType *GifFile, GifByteType **CodeBlock)
{
    GifByteType Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (READ(GifFile,&Buf, 1) != 1) {
	_GifError = D_GIF_ERR_READ_FAILED;
	return GIF_ERROR;
    }

    if (Buf > 0) {
	*CodeBlock = Private->Buf;	       /* Use private unused buffer. */
	(*CodeBlock)[0] = Buf;  /* Pascal strings notation (pos. 0 is len.). */
	if (READ(GifFile,&((*CodeBlock)[1]), Buf) != Buf) {
	    _GifError = D_GIF_ERR_READ_FAILED;
	    return GIF_ERROR;
	}
    }
    else {
	*CodeBlock = NULL;
	Private->Buf[0] = 0;		   /* Make sure the buffer is empty! */
	Private->PixelCount = 0;   /* And local info. indicate image read. */
    }

    return GIF_OK;
}

/******************************************************************************
*   Setup the LZ decompression for this image:				      *
******************************************************************************/
static int DGifSetupDecompress(GifFileType *GifFile)
{
    int i, BitsPerPixel;
    GifByteType CodeSize;
    unsigned int *Prefix;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    READ(GifFile,&CodeSize, 1);    /* Read Code size from file. */
    BitsPerPixel = CodeSize;

    Private->Buf[0] = 0;			      /* Input Buffer empty. */
    Private->BitsPerPixel = BitsPerPixel;
    Private->ClearCode = (1 << BitsPerPixel);
    Private->EOFCode = Private->ClearCode + 1;
    Private->RunningCode = Private->EOFCode + 1;
    Private->RunningBits = BitsPerPixel + 1;	 /* Number of bits per code. */
    Private->MaxCode1 = 1 << Private->RunningBits;     /* Max. code + 1. */
    Private->StackPtr = 0;		    /* No pixels on the pixel stack. */
    Private->LastCode = NO_SUCH_CODE;
    Private->CrntShiftState = 0;	/* No information in CrntShiftDWord. */
    Private->CrntShiftDWord = 0;

    Prefix = Private->Prefix;
    for (i = 0; i <= LZ_MAX_CODE; i++) Prefix[i] = NO_SUCH_CODE;

    return GIF_OK;
}

/******************************************************************************
*   The LZ decompression routine:					      *
*   This version decompress the given gif file into Line of length LineLen.   *
*   This routine can be called few times (one per scan line, for example), in *
* order the complete the whole image.					      *
******************************************************************************/
static int DGifDecompressLine(GifFileType *GifFile, GifPixelType *Line,
								int LineLen)
{
    int i = 0, j, CrntCode, EOFCode, ClearCode, CrntPrefix, LastCode, StackPtr;
    GifByteType *Stack, *Suffix;
    unsigned int *Prefix;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    StackPtr = Private->StackPtr;
    Prefix = Private->Prefix;
    Suffix = Private->Suffix;
    Stack = Private->Stack;
    EOFCode = Private->EOFCode;
    ClearCode = Private->ClearCode;
    LastCode = Private->LastCode;

    if (StackPtr != 0) {
	/* Let pop the stack off before continueing to read the gif file: */
	while (StackPtr != 0 && i < LineLen) Line[i++] = Stack[--StackPtr];
    }

    while (i < LineLen) {			    /* Decode LineLen items. */
	if (DGifDecompressInput(GifFile, &CrntCode) == GIF_ERROR)
    	    return GIF_ERROR;

	if (CrntCode == EOFCode) {
	    /* Note however that usually we will not be here as we will stop */
	    /* decoding as soon as we got all the pixel, or EOF code will    */
	    /* not be read at all, and DGifGetLine/Pixel clean everything.   */
	    if (i != LineLen - 1 || Private->PixelCount != 0) {
		_GifError = D_GIF_ERR_EOF_TOO_SOON;
		return GIF_ERROR;
	    }
	    i++;
	}
	else if (CrntCode == ClearCode) {
	    /* We need to start over again: */
	    for (j = 0; j <= LZ_MAX_CODE; j++) Prefix[j] = NO_SUCH_CODE;
	    Private->RunningCode = Private->EOFCode + 1;
	    Private->RunningBits = Private->BitsPerPixel + 1;
	    Private->MaxCode1 = 1 << Private->RunningBits;
	    LastCode = Private->LastCode = NO_SUCH_CODE;
	}
	else {
	    /* Its regular code - if in pixel range simply add it to output  */
	    /* stream, otherwise trace to codes linked list until the prefix */
	    /* is in pixel range:					     */
	    if (CrntCode < ClearCode) {
		/* This is simple - its pixel scalar, so add it to output:   */
		Line[i++] = CrntCode;
	    }
	    else {
		/* Its a code to needed to be traced: trace the linked list  */
		/* until the prefix is a pixel, while pushing the suffix     */
		/* pixels on our stack. If we done, pop the stack in reverse */
		/* (thats what stack is good for!) order to output.	     */
		if (Prefix[CrntCode] == NO_SUCH_CODE) {
		    /* Only allowed if CrntCode is exactly the running code: */
		    /* In that case CrntCode = XXXCode, CrntCode or the	     */
		    /* prefix code is last code and the suffix char is	     */
		    /* exactly the prefix of last code!			     */
		    if (CrntCode == Private->RunningCode - 2) {
			CrntPrefix = LastCode;
			Suffix[Private->RunningCode - 2] =
			Stack[StackPtr++] = DGifGetPrefixChar(Prefix,
							LastCode, ClearCode);
		    }
		    else {
			_GifError = D_GIF_ERR_IMAGE_DEFECT;
			return GIF_ERROR;
		    }
		}
		else
		    CrntPrefix = CrntCode;

		/* Now (if image is O.K.) we should not get an NO_SUCH_CODE  */
		/* During the trace. As we might loop forever, in case of    */
		/* defective image, we count the number of loops we trace    */
		/* and stop if we got LZ_MAX_CODE. obviously we can not      */
		/* loop more than that.					     */
		j = 0;
		while (j++ <= LZ_MAX_CODE &&
		       CrntPrefix > ClearCode &&
		       CrntPrefix <= LZ_MAX_CODE) {
		    Stack[StackPtr++] = Suffix[CrntPrefix];
		    CrntPrefix = Prefix[CrntPrefix];
		}
		if (j >= LZ_MAX_CODE || CrntPrefix > LZ_MAX_CODE) {
		    _GifError = D_GIF_ERR_IMAGE_DEFECT;
		    return GIF_ERROR;
		}
		/* Push the last character on stack: */
		Stack[StackPtr++] = CrntPrefix;

		/* Now lets pop all the stack into output: */
		while (StackPtr != 0 && i < LineLen)
		    Line[i++] = Stack[--StackPtr];
	    }
	    if (LastCode != NO_SUCH_CODE) {
		Prefix[Private->RunningCode - 2] = LastCode;

		if (CrntCode == Private->RunningCode - 2) {
		    /* Only allowed if CrntCode is exactly the running code: */
		    /* In that case CrntCode = XXXCode, CrntCode or the	     */
		    /* prefix code is last code and the suffix char is	     */
		    /* exactly the prefix of last code!			     */
		    Suffix[Private->RunningCode - 2] =
			DGifGetPrefixChar(Prefix, LastCode, ClearCode);
		}
		else {
		    Suffix[Private->RunningCode - 2] =
			DGifGetPrefixChar(Prefix, CrntCode, ClearCode);
		}
	    }
	    LastCode = CrntCode;
	}
    }

    Private->LastCode = LastCode;
    Private->StackPtr = StackPtr;

    return GIF_OK;
}

/******************************************************************************
* Routine to trace the Prefixes linked list until we get a prefix which is    *
* not code, but a pixel value (less than ClearCode). Returns that pixel value.*
* If image is defective, we might loop here forever, so we limit the loops to *
* the maximum possible if image O.k. - LZ_MAX_CODE times.		      *
******************************************************************************/
static int DGifGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode)
{
    int i = 0;

    while (Code > ClearCode && i++ <= LZ_MAX_CODE) Code = Prefix[Code];
    return Code;
}

/******************************************************************************
*   Interface for accessing the LZ codes directly. Set Code to the real code  *
* (12bits), or to -1 if EOF code is returned.				      *
******************************************************************************/
static int DGifGetLZCodes(GifFileType *GifFile, int *Code)
{
    GifByteType *CodeBlock;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile->Private;

    if (!IS_READABLE(Private)) {
	/* This file was NOT open for reading: */
	_GifError = D_GIF_ERR_NOT_READABLE;
	return GIF_ERROR;
    }

    if (DGifDecompressInput(GifFile, Code) == GIF_ERROR)
	return GIF_ERROR;

    if (*Code == Private->EOFCode) {
	/* Skip rest of codes (hopefully only NULL terminating block): */
	do if (DGifGetCodeNext(GifFile, &CodeBlock) == GIF_ERROR)
    	    return GIF_ERROR;
	while (CodeBlock != NULL);

	*Code = -1;
    }
    else if (*Code == Private->ClearCode) {
	/* We need to start over again: */
	Private->RunningCode = Private->EOFCode + 1;
	Private->RunningBits = Private->BitsPerPixel + 1;
	Private->MaxCode1 = 1 << Private->RunningBits;
    }

    return GIF_OK;
}

/******************************************************************************
*   The LZ decompression input routine:					      *
*   This routine is responsable for the decompression of the bit stream from  *
* 8 bits (bytes) packets, into the real codes.				      *
*   Returns GIF_OK if read succesfully.					      *
******************************************************************************/
static int DGifDecompressInput(GifFileType *GifFile, int *Code)
{
    GifFilePrivateType *Private = (GifFilePrivateType *)GifFile->Private;

    GifByteType NextByte;

    while (Private->CrntShiftState < Private->RunningBits) {
	/* Needs to get more bytes from input stream for next code: */
	if (DGifBufferedInput(GifFile, Private->Buf, &NextByte)
	    == GIF_ERROR) {
	    return GIF_ERROR;
	}
	Private->CrntShiftDWord |=
		((unsigned long) NextByte) << Private->CrntShiftState;
	Private->CrntShiftState += 8;
    }
    *Code = Private->CrntShiftDWord & ((1 << Private->RunningBits) - 1);

    Private->CrntShiftDWord >>= Private->RunningBits;
    Private->CrntShiftState -= Private->RunningBits;

    /* If code cannt fit into RunningBits bits, must raise its size. Note */
    /* however that codes above 4095 are used for special signaling.      */
    if ( ++Private->RunningCode > Private->MaxCode1 && Private->RunningBits < LZ_BITS )
	{
		Private->MaxCode1 <<= 1;
		Private->RunningBits++;
    }
    return GIF_OK;
}

/******************************************************************************
*   This routines read one gif data block at a time and buffers it internally *
* so that the decompression routine could access it.			      *
*   The routine returns the next byte from its internal buffer (or read next  *
* block in if buffer empty) and returns GIF_OK if succesful.		      *
******************************************************************************/
static int DGifBufferedInput(GifFileType *GifFile, GifByteType *Buf, GifByteType *NextByte)
{
    if (Buf[0] == 0) {
	/* Needs to read the next buffer - this one is empty: */
	if (READ(GifFile, Buf, 1) != 1)
	{
	    _GifError = D_GIF_ERR_READ_FAILED;
	    return GIF_ERROR;
	}
	if (READ(GifFile,&Buf[1], Buf[0]) != Buf[0])
	{
	    _GifError = D_GIF_ERR_READ_FAILED;
	    return GIF_ERROR;
	}
	*NextByte = Buf[1];
	Buf[1] = 2;	   /* We use now the second place as last char read! */
	Buf[0]--;
    }
    else {
	*NextByte = Buf[Buf[1]++];
	Buf[0]--;
    }

    return GIF_OK;
}

/******************************************************************************
* This routine reads an entire GIF into core, hanging all its state info off  *
* the GifFileType pointer.  Call DGifOpenFileName() or DGifOpenFileHandle()   *
* first to initialize I/O.  Its inverse is EGifSpew().			      *
* 
 ******************************************************************************/
static int DGifSlurp(GifFileType *GifFile)
{
    int ImageSize;
    GifRecordType RecordType;
    SavedImage *sp;
    GifByteType *ExtData;
    SavedImage temp_save;

    temp_save.ExtensionBlocks=NULL;
    temp_save.ExtensionBlockCount=0;
  
    do {
	if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR)
	    return(GIF_ERROR);

	switch (RecordType) {
	    case IMAGE_DESC_RECORD_TYPE:
		if (DGifGetImageDesc(GifFile) == GIF_ERROR)
		    return(GIF_ERROR);

		sp = &GifFile->SavedImages[GifFile->ImageCount-1];
		ImageSize = sp->ImageDesc.Width * sp->ImageDesc.Height;

		sp->RasterBits
		    = (char*) malloc(ImageSize * sizeof(GifPixelType));

		if (DGifGetLine(GifFile, (GifPixelType*)sp->RasterBits, ImageSize)
		    == GIF_ERROR)
		    return(GIF_ERROR);

        if (temp_save.ExtensionBlocks) {
            sp->ExtensionBlocks = temp_save.ExtensionBlocks;
            sp->ExtensionBlockCount = temp_save.ExtensionBlockCount;

            temp_save.ExtensionBlocks = NULL;
            temp_save.ExtensionBlockCount=0;

        /* FIXME: The following is wrong.  It is left in only for backwards
         * compatibility.  Someday it should go away.  Use the
         * sp->ExtensionBlocks->Function variable instead.
         */
            sp->Function = sp->ExtensionBlocks[0].Function;

        }

		break;

	    case EXTENSION_RECORD_TYPE:
		if (DGifGetExtension(GifFile,&temp_save.Function,&ExtData)==GIF_ERROR)
		    return(GIF_ERROR);
		while (ExtData != NULL) {
            
            /* Create an extension block with our data */
            if (AddExtensionBlock(&temp_save, ExtData[0], (char*)&ExtData[1])
               == GIF_ERROR)
                return (GIF_ERROR); 
            
		    if (DGifGetExtensionNext(GifFile, &ExtData) == GIF_ERROR)
			    return(GIF_ERROR);
            temp_save.Function = 0;
		}
		break;

	    case TERMINATE_RECORD_TYPE:
		break;

	    default:	/* Should be trapped by DGifGetRecordType */
		break;
	}
    } while (RecordType != TERMINATE_RECORD_TYPE);

    /* Just in case the Gif has an extension block without an associated
     * image... (Should we save this into a savefile structure with no image
     * instead?  Have to check if the present writing code can handle that as
     * well....
     */
    if (temp_save.ExtensionBlocks)
        FreeExtension(&temp_save);
    
    return(GIF_OK);
}

// ======================================================

/*****************************************************************************
*   "Gif-Lib" - Yet another gif library.				     *
*									     *
* Written by:  Gershon Elber				Ver 0.1, Jun. 1989   *
* Extensively hacked by: Eric S. Raymond		Ver 1.?, Sep 1992    *
******************************************************************************
* GIF construction tools						      *
******************************************************************************
* History:								     *
* 15 Sep 92 - Version 1.0 by Eric Raymond.				     *
*****************************************************************************/

#define MAX(x, y)	(((x) > (y)) ? (x) : (y))

/******************************************************************************
* Miscellaneous utility functions					      *
******************************************************************************/

// return smallest bitfield size n will fit in
static int BitSize(int n)
{
	int i;
	for ( i=1; i<=8; ++i )
	if ( (1 << i) >= n )
		break;

	return i;
}


/******************************************************************************
* Color map object functions						      *
******************************************************************************/

static ColorMapObject *MakeMapObject(int ColorCount, GifColorType *ColorMap)
/*
 * Allocate a color map of given size; initialize with contents of
 * ColorMap if that pointer is non-NULL.
 */
{
    ColorMapObject *Object;

    if (ColorCount != (1 << BitSize(ColorCount)))
	return((ColorMapObject *)NULL);

    Object = (ColorMapObject *)malloc(sizeof(ColorMapObject));
    if (Object == (ColorMapObject *)NULL)
	return((ColorMapObject *)NULL);

    Object->Colors = (GifColorType *)calloc(ColorCount, sizeof(GifColorType));
    if (Object->Colors == (GifColorType *)NULL)
	return((ColorMapObject *)NULL);

    Object->ColorCount = ColorCount;
    Object->BitsPerPixel = BitSize(ColorCount);

    if (ColorMap)
	memcpy((char *)Object->Colors,
	       (char *)ColorMap, ColorCount * sizeof(GifColorType));

    return(Object);
}

static void FreeMapObject(ColorMapObject *Object)
/*
 * Free a color map object
 */
{
    free(Object->Colors);
    free(Object);
}

static ColorMapObject *UnionColorMap(
			 ColorMapObject *ColorIn1,
			 ColorMapObject *ColorIn2,
			 GifPixelType ColorTransIn2[])
/*
 * Compute the union of two given color maps and return it.  If result can't 
 * fit into 256 colors, NULL is returned, the allocated union otherwise.
 * ColorIn1 is copied as is to ColorUnion, while colors from ColorIn2 are
 * copied iff they didn't exist before.  ColorTransIn2 maps the old
 * ColorIn2 into ColorUnion color map table.
 */
{
    int i, j, CrntSlot, RoundUpTo, NewBitSize;
    ColorMapObject *ColorUnion;

    /*
     * Allocate table which will hold the result for sure.
     */
    ColorUnion
	= MakeMapObject(MAX(ColorIn1->ColorCount,ColorIn2->ColorCount)*2,NULL);

    if (ColorUnion == NULL)
	return(NULL);

    /* Copy ColorIn1 to ColorUnionSize; */
    for (i = 0; i < ColorIn1->ColorCount; i++)
	ColorUnion->Colors[i] = ColorIn1->Colors[i];
    CrntSlot = ColorIn1->ColorCount;

    /*
     * Potentially obnoxious hack:
     *
     * Back CrntSlot down past all contiguous {0, 0, 0} slots at the end
     * of table 1.  This is very useful if your display is limited to
     * 16 colors.
     */
    while (ColorIn1->Colors[CrntSlot-1].Red == 0
	   && ColorIn1->Colors[CrntSlot-1].Green == 0
	   && ColorIn1->Colors[CrntSlot-1].Red == 0)
	CrntSlot--;

    /* Copy ColorIn2 to ColorUnionSize (use old colors if they exist): */
    for (i = 0; i < ColorIn2->ColorCount && CrntSlot<=256; i++)
    {
	/* Let's see if this color already exists: */
	for (j = 0; j < ColorIn1->ColorCount; j++)
	    if (memcmp(&ColorIn1->Colors[j], &ColorIn2->Colors[i], sizeof(GifColorType)) == 0)
		break;

	if (j < ColorIn1->ColorCount)
	    ColorTransIn2[i] = j;	/* color exists in Color1 */
	else
	{
	    /* Color is new - copy it to a new slot: */
	    ColorUnion->Colors[CrntSlot] = ColorIn2->Colors[i];
	    ColorTransIn2[i] = CrntSlot++;
	}
    }

    if (CrntSlot > 256)
    {
	FreeMapObject(ColorUnion);
	return((ColorMapObject *)NULL);
    }

    NewBitSize = BitSize(CrntSlot);
    RoundUpTo = (1 << NewBitSize);

    if (RoundUpTo != ColorUnion->ColorCount)
    {
	register GifColorType	*Map = ColorUnion->Colors;

	/*
	 * Zero out slots up to next power of 2.
	 * We know these slots exist because of the way ColorUnion's
	 * start dimension was computed.
	 */
	for (j = CrntSlot; j < RoundUpTo; j++)
	    Map[j].Red = Map[j].Green = Map[j].Blue = 0;

	/* perhaps we can shrink the map? */
	if (RoundUpTo < ColorUnion->ColorCount)
	    ColorUnion->Colors 
		= (GifColorType *)realloc(Map, sizeof(GifColorType)*RoundUpTo);
    }

    ColorUnion->ColorCount = RoundUpTo;
    ColorUnion->BitsPerPixel = NewBitSize;

    return(ColorUnion);
}

static void ApplyTranslation(SavedImage *Image, GifPixelType Translation[])
/*
 * Apply a given color translation to the raster bits of an image
 */
{
    register int i;
    register int RasterSize = Image->ImageDesc.Height * Image->ImageDesc.Width;

    for (i = 0; i < RasterSize; i++)
	Image->RasterBits[i] = Translation[Image->RasterBits[i]];
}

/******************************************************************************
* Extension record functions						      *
******************************************************************************/

static void MakeExtension(SavedImage *New, int Function)
{
    New->Function = Function;
    /*
     * Someday we might have to deal with multiple extensions.
     */
}

static int AddExtensionBlock(SavedImage *New, int Len, char ExtData[])
{
    ExtensionBlock	*ep;

    if (New->ExtensionBlocks == NULL)
	New->ExtensionBlocks = (ExtensionBlock *)malloc(sizeof(ExtensionBlock));
    else
	New->ExtensionBlocks =
	    (ExtensionBlock *)realloc(New->ExtensionBlocks,
		      sizeof(ExtensionBlock) * (New->ExtensionBlockCount + 1));

    if (New->ExtensionBlocks == NULL)
	return(GIF_ERROR);

    ep = &New->ExtensionBlocks[New->ExtensionBlockCount++];

    if ((ep->Bytes = (char *)malloc(ep->ByteCount = Len)) == NULL)
	return(GIF_ERROR);

    if (ExtData) {
	    memcpy(ep->Bytes, ExtData, Len);
        ep->Function = New->Function;
    }

    return(GIF_OK);
}

static void FreeExtension(SavedImage *Image)
{
    ExtensionBlock	*ep;

    for (ep = Image->ExtensionBlocks;
	 ep < Image->ExtensionBlocks + Image->ExtensionBlockCount;
	 ep++)
	(void) free((char *)ep->Bytes);
    free((char *)Image->ExtensionBlocks);
    Image->ExtensionBlocks = NULL;
}

/******************************************************************************
* Image block allocation functions					      *
******************************************************************************/
static SavedImage *MakeSavedImage(GifFileType *GifFile, SavedImage *CopyFrom)
/*
 * Append an image block to the SavedImages array  
 */
{
    SavedImage	*sp;

    if (GifFile->SavedImages == NULL)
	GifFile->SavedImages = (SavedImage *)malloc(sizeof(SavedImage));
    else
	GifFile->SavedImages = (SavedImage *)realloc(GifFile->SavedImages,
				sizeof(SavedImage) * (GifFile->ImageCount+1));

    if (GifFile->SavedImages == NULL)
	return((SavedImage *)NULL);
    else
    {
	sp = &GifFile->SavedImages[GifFile->ImageCount++];
	memset((char *)sp, '\0', sizeof(SavedImage));

	if (CopyFrom)
	{
	    memcpy((char *)sp, CopyFrom, sizeof(SavedImage));

	    /*
	     * Make our own allocated copies of the heap fields in the
	     * copied record.  This guards against potential aliasing
	     * problems.
	     */

	    /* first, the local color map */
	    if (sp->ImageDesc.ColorMap)
		sp->ImageDesc.ColorMap =
		    MakeMapObject(CopyFrom->ImageDesc.ColorMap->ColorCount,
				  CopyFrom->ImageDesc.ColorMap->Colors);

	    /* next, the raster */
	    sp->RasterBits = (char *)malloc(sizeof(GifPixelType)
				* CopyFrom->ImageDesc.Height
				* CopyFrom->ImageDesc.Width);
	    memcpy(sp->RasterBits,
		   CopyFrom->RasterBits,
		   sizeof(GifPixelType)
			* CopyFrom->ImageDesc.Height
			* CopyFrom->ImageDesc.Width);

	    /* finally, the extension blocks */
	    if (sp->ExtensionBlocks)
	    {
		sp->ExtensionBlocks
		    = (ExtensionBlock*)malloc(sizeof(ExtensionBlock)
					      * CopyFrom->ExtensionBlockCount);
		memcpy(sp->ExtensionBlocks,
		   CopyFrom->ExtensionBlocks,
		   sizeof(ExtensionBlock)
		   	* CopyFrom->ExtensionBlockCount);

		/*
		 * For the moment, the actual blocks can take their
		 * chances with free().  We'll fix this later. 
		 */
	    }
	}

	return(sp);
    }
}

static void FreeSavedImages(GifFileType *GifFile)
{
    SavedImage	*sp;

    for ( sp = GifFile->SavedImages;
	 sp < GifFile->SavedImages + GifFile->ImageCount;
	 sp++ )
    {
		if (sp->ImageDesc.ColorMap)
			FreeMapObject(sp->ImageDesc.ColorMap);

		if (sp->RasterBits)
			free((char *)sp->RasterBits);

		if (sp->ExtensionBlocks)
			FreeExtension(sp);
    }
    free((char *) GifFile->SavedImages);
}


// =================================================
// codec
// =================================================

static bool gif_decode(Bitmap& target, Stream& stream)
{
	// open gif stream
	GifFileType* gif = DGifOpen(&stream);
	if ( !gif )
	{
		PRCORE_EXCEPTION("Cannot open .gif stream.");
	}

	// read gif image
	if ( DGifSlurp(gif) != GIF_OK )
	{
		DGifCloseFile(gif);
		PRCORE_EXCEPTION("Cannot read .gif stream.");
	}

	// colormap
	ColorMapObject* cmap = gif->SColorMap;

	// validate attributes
	if ( cmap == 0 || gif->ImageCount < 1 || cmap->ColorCount != (1 << cmap->BitsPerPixel) )
	{
		DGifCloseFile(gif);
		PRCORE_EXCEPTION("Incorrect gif stream.");
	}

	// use the first frame of the animation
	SavedImage* gif_image = gif->SavedImages + 0;
		
	// palette
	color32 palette[256];
	memset(palette,0,256*sizeof(color32));

	int i;
	for ( i=0; i<cmap->ColorCount; ++i )
	{
		palette[i].r = cmap->Colors[i].Red;
		palette[i].g = cmap->Colors[i].Green;
		palette[i].b = cmap->Colors[i].Blue;
		palette[i].a = 255;
	}

	// look for the transparent color extension
	for ( i=0; i<gif_image->ExtensionBlockCount; ++i )
	{
		ExtensionBlock* eb = gif_image->ExtensionBlocks + i;
		if ( eb->Function == 0xf9 && eb->ByteCount == 4 )
		{
			if ( eb->Bytes[0] & 1 )
			{
				int index = eb->Bytes[3];
				palette[index].a = 0;
			}
		}
	}

	int width  = gif->SWidth;
	int height = gif->SHeight;

	target = Bitmap(width,height,PIXELFORMAT_PALETTE8(palette));
	uint8* image = target.GetImage();

	uint8* src = reinterpret_cast<uint8*>(gif_image->RasterBits);

	if ( gif->Image.Interlace )
	{
		// deinterlace
		// group: 0 3 2 3 1 3 2 3
		int y;

		// group 0
		for ( y=0; y<height; y += 8 )
		{
			memcpy(image + y*width,src,width);
			src += width;
		}

		// group 1
		for ( y=4; y<height; y += 8 )
		{
			memcpy(image + y*width,src,width);
			src += width;
		}

		// group 2
		for ( y=2; y<height; y += 4 )
		{
			memcpy(image + y*width,src,width);
			src += width;
		}

		// group 3
		for ( y=1; y<height; y += 2 )
		{
			memcpy(image + y*width,src,width);
			src += width;
		}
	}
	else
	{
		memcpy(image,src,width*height);
	}

	DGifCloseFile(gif);

	return true;
}


// =================================================
// CreateCodecGIF()
// =================================================

BitmapCodec CreateCodecGIF()
{
	static const String ext[] = { "gif" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = gif_decode;
	codec.encode    = NULL;

	return codec;
}
