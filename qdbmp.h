#ifndef _BMP_H_
#define _BMP_H_


/**************************************************************

	QDBMP - Quick n' Dirty BMP

	v1.0.0 - 2007-04-07
	http://qdbmp.sourceforge.net


	The library supports the following BMP variants:
	1. Uncompressed 32 BPP (alpha values are ignored)
	2. Uncompressed 24 BPP
	3. Uncompressed 8 BPP (indexed color)

	QDBMP is free and open source software, distributed
	under the MIT licence.

	Copyright (c) 2007 Chai Braudo (braudo@users.sourceforge.net)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

**************************************************************/

#include <stdio.h>

#include <iostream>

//#include "util-fmemopen.h"

#include <windows.h>

/* Type definitions */
//#ifndef uint32_t
	#define uint32_t	uint32_t
//#endif

//#ifndef uint16_t
	#define uint16_t	uint16_t
//#endif

//#ifndef uint8_t
	#define uint8_t	uint8_t
//#endif


/* Version */
#define QDBMP_VERSION_MAJOR		1
#define QDBMP_VERSION_MINOR		0
#define QDBMP_VERSION_PATCH		1


/* Error codes */
typedef enum
{
	BMP_OK = 0,				/* No error */
	BMP_ERROR,				/* General error */
	BMP_OUT_OF_MEMORY,		/* Could not allocate enough memory to complete the operation */
	BMP_IO_ERROR,			/* General input/output error */
	BMP_FILE_NOT_FOUND,		/* File not found */
	BMP_FILE_NOT_SUPPORTED,	/* File is not a supported BMP variant */
	BMP_FILE_INVALID,		/* File is not a BMP image or is an invalid BMP */
	BMP_INVALID_ARGUMENT,	/* An argument is invalid or out of range */
	BMP_TYPE_MISMATCH,		/* The requested action is not compatible with the BMP's type */
	BMP_ERROR_NUM
} BMP_STATUS;


/* Bitmap header */
struct BMP_Header
{
	uint16_t		Magic;				/* Magic identifier: "BM" */
	uint32_t		FileSize;			/* Size of the BMP file in bytes */
	uint16_t		Reserved1;			/* Reserved */
	uint16_t		Reserved2;			/* Reserved */
	uint32_t		DataOffset;			/* Offset of image data relative to the file's start */
	uint32_t		HeaderSize;			/* Size of the header in bytes */
	uint32_t		Width;				/* Bitmap's width */
	uint32_t		Height;				/* Bitmap's height */
	uint16_t		Planes;				/* Number of color planes in the bitmap */
	uint16_t		BitsPerPixel;		/* Number of bits per pixel */
	uint32_t		CompressionType;	/* Compression type */
	uint32_t		ImageDataSize;		/* Size of uncompressed image's data */
	uint32_t		HPixelsPerMeter;	/* Horizontal resolution (pixels per meter) */
	uint32_t		VPixelsPerMeter;	/* Vertical resolution (pixels per meter) */
	uint32_t		ColorsUsed;			/* Number of color indexes in the color table that are actually used by the bitmap */
	uint32_t		ColorsRequired;		/* Number of color indexes that are required for displaying the bitmap */
} __attribute__((packed));


/* Private data structure */
struct BMP
{
	BMP_Header	Header;
	uint8_t*		Palette;
	uint8_t*		Data;
} __attribute__((packed));


/* Bitmap image */
//typedef struct _BMP BMP;




/*********************************** Public methods **********************************/


/* Construction/destruction */
BMP*			BMP_Create					( uint32_t width, uint32_t height, uint16_t depth );
void			BMP_Free					( BMP* bmp );


/* I/O */
BMP*			BMP_ReadBuff				( unsigned char* buff, size_t size);
void			BMP_WriteBuff				( BMP* bmp, unsigned char* buff, size_t size );


size_t 			BMP_GetSizeInBytes			(BMP* bmp);

/* Meta info */
uint32_t			BMP_GetWidth				( BMP* bmp );
uint32_t			BMP_GetHeight				( BMP* bmp );
uint16_t			BMP_GetDepth				( BMP* bmp );


/* Pixel access */
void			BMP_GetPixelRGB				( BMP* bmp, uint32_t x, uint32_t y, uint8_t* r, uint8_t* g, uint8_t* b );
void			BMP_SetPixelRGB				( BMP* bmp, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b );
void			BMP_GetPixelIndex			( BMP* bmp, uint32_t x, uint32_t y, uint8_t* val );
void			BMP_SetPixelIndex			( BMP* bmp, uint32_t x, uint32_t y, uint8_t val );


/* Palette handling */
void			BMP_GetPaletteColor			( BMP* bmp, uint8_t index, uint8_t* r, uint8_t* g, uint8_t* b );
void			BMP_SetPaletteColor			( BMP* bmp, uint8_t index, uint8_t r, uint8_t g, uint8_t b );


/* Error handling */
BMP_STATUS		BMP_GetError				();
const char*		BMP_GetErrorDescription		();


/* Useful macro that may be used after each BMP operation to check for an error */
#define BMP_CHECK_ERROR( output_file, return_value ) \
	if ( BMP_GetError() != BMP_OK )													\
	{																				\
		fprintf( ( output_file ), "BMP error: %s\n", BMP_GetErrorDescription() );	\
		return( return_value );														\
	}																				\

#endif
