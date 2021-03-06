
#include "qdbmp.h"
#include <stdlib.h>
#include <string.h>




/* Holds the last error code */
static BMP_STATUS BMP_LAST_ERROR_CODE = (BMP_STATUS)0;


/* Error description strings */
static const char* BMP_ERROR_STRING[] =
{
	"",
	"General error",
	"Could not allocate enough memory to complete the operation",
	"File input/output error",
	"File not found",
	"File is not a supported BMP variant (must be uncompressed 8, 24 or 32 BPP)",
	"File is not a valid BMP image",
	"An argument is invalid or out of range",
	"The requested action is not compatible with the BMP's type"
};


/* Size of the palette data for 8 BPP bitmaps */
#define BMP_PALETTE_SIZE	( 256 * 4 )


static size_t POS = 0;

/*********************************** Forward declarations **********************************/
int		ReadHeader	( BMP* bmp, unsigned char* f );
int		WriteHeader	( BMP* bmp, unsigned char* f );

int		ReadUINT	( uint32_t* x, unsigned char* f );
int		ReadUSHORT	( uint16_t *x, unsigned char* f );

int		WriteUINT	( uint32_t x, unsigned char* f );
int		WriteUSHORT	( uint16_t x, unsigned char* f );




// void getTemp(char* fn) {

//     char temppath[MAX_PATH - 13];
//     if (0 == GetTempPath(sizeof(temppath), temppath))
//         return;

//     char filename[MAX_PATH + 1];
//     if (0 == GetTempFileName(temppath, "SC", 0, filename))
//         return;

// 	memcpy(fn, filename, MAX_PATH + 1);
// }

static int myCopy(unsigned char *src, unsigned char* dest, size_t size, bool posRight = true) {

    size_t count = 0;
    // size_t available = size - POS;
    int is_eof = 0;

    // if (size < 0) return - 1;

    // if ((size_t)size > available) {
    //     size = available;
    // } else {
    //     is_eof = 1;
    // }


	if(posRight) {

		while (count < size)
			dest[count++] = src[POS++];

	} else {


		while (count < size)
			dest[POS++] = src[count++];
	}


    if (is_eof == 1)
        return 0;

    return count;
}

/*********************************** Public methods **********************************/


/**************************************************************
	Creates a blank BMP image with the specified dimensions
	and bit depth.
**************************************************************/
BMP* BMP_Create( uint32_t width, uint32_t height, uint16_t depth )
{
	BMP*	bmp;
	int		bytes_per_pixel = depth >> 3;
	uint32_t	bytes_per_row;

	if ( height <= 0 || width <= 0 )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return NULL;
	}

	if ( depth != 8 && depth != 24 && depth != 32 )
	{
		BMP_LAST_ERROR_CODE = BMP_FILE_NOT_SUPPORTED;
		return NULL;
	}


	/* Allocate the bitmap data structure */
	bmp = (BMP*)calloc( 1, sizeof( BMP ) );
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
		return NULL;
	}


	/* Set header' default values */
	bmp->Header.Magic				= 0x4D42;
	bmp->Header.Reserved1			= 0;
	bmp->Header.Reserved2			= 0;
	bmp->Header.HeaderSize			= 40;
	bmp->Header.Planes				= 1;
	bmp->Header.CompressionType		= 0;
	bmp->Header.HPixelsPerMeter		= 0;
	bmp->Header.VPixelsPerMeter		= 0;
	bmp->Header.ColorsUsed			= 0;
	bmp->Header.ColorsRequired		= 0;


	/* Calculate the number of bytes used to store a single image row. This is always
	rounded up to the next multiple of 4. */
	bytes_per_row = width * bytes_per_pixel;
	bytes_per_row += ( bytes_per_row % 4 ? 4 - bytes_per_row % 4 : 0 );


	/* Set header's image specific values */
	bmp->Header.Width				= width;
	bmp->Header.Height				= height;
	bmp->Header.BitsPerPixel		= depth;
	bmp->Header.ImageDataSize		= bytes_per_row * height;
	bmp->Header.FileSize			= bmp->Header.ImageDataSize + 54 + ( depth == 8 ? BMP_PALETTE_SIZE : 0 );
	bmp->Header.DataOffset			= 54 + ( depth == 8 ? BMP_PALETTE_SIZE : 0 );


	/* Allocate palette */
	if ( bmp->Header.BitsPerPixel == 8 )
	{
		bmp->Palette = (uint8_t*) calloc( BMP_PALETTE_SIZE, sizeof( uint8_t ) );
		if ( bmp->Palette == NULL )
		{
			BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
			free( bmp );
			return NULL;
		}
	}
	else
	{
		bmp->Palette = NULL;
	}

	/* Allocate pixels */
	bmp->Data = (uint8_t*) calloc( bmp->Header.ImageDataSize, sizeof( uint8_t ) );
	if ( bmp->Data == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
		free( bmp->Palette );
		free( bmp );
		return NULL;
	}


	BMP_LAST_ERROR_CODE = BMP_OK;

	return bmp;
}


/**************************************************************
	Frees all the memory used by the specified BMP image.
**************************************************************/
void BMP_Free( BMP* bmp )
{
	if ( bmp == NULL )
	{
		return;
	}

	if ( bmp->Palette != NULL )
	{
		free( bmp->Palette );
	}

	if ( bmp->Data != NULL )
	{
		free( bmp->Data );
	}

	free( bmp );

	BMP_LAST_ERROR_CODE = BMP_OK;
}




/**************************************************************
	Reads the specified BMP image file.
**************************************************************/
BMP* BMP_ReadBuff(unsigned char* src, size_t size )
{
	BMP*	bmp;
	FILE* bla;
	/* Allocate */
	bmp = (BMP*)calloc( 1, sizeof( BMP ) );
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;

		POS = 0;
		return NULL;
	}    

	/* Read header */
	if ( ReadHeader( bmp, src ) != BMP_OK || bmp->Header.Magic != 0x4D42 )
	{

		BMP_LAST_ERROR_CODE = BMP_FILE_INVALID;
		free( bmp );

		POS = 0;
		return NULL;
	}

	/* Verify that the bitmap variant is supported */
	if ( ( bmp->Header.BitsPerPixel != 32 && bmp->Header.BitsPerPixel != 24 && bmp->Header.BitsPerPixel != 8 )
		|| bmp->Header.CompressionType != 0 || bmp->Header.HeaderSize != 40 )
	{

		BMP_LAST_ERROR_CODE = BMP_FILE_NOT_SUPPORTED;
		free( bmp );

		POS = 0;
		return NULL;
	}

	/* Allocate and read palette */
	if ( bmp->Header.BitsPerPixel == 8 )
	{
		bmp->Palette = (uint8_t*) malloc( BMP_PALETTE_SIZE * sizeof( uint8_t ) );
		if ( bmp->Palette == NULL )
		{
			BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
			free( bmp );

			POS = 0;
			return NULL;
		}

		if ( myCopy(src, bmp->Palette, BMP_PALETTE_SIZE) != BMP_PALETTE_SIZE )
		{
			BMP_LAST_ERROR_CODE = BMP_FILE_INVALID;
			free( bmp->Palette );
			free( bmp );

			POS = 0;
			return NULL;
		}
	}
	else	/* Not an indexed image */
	{
		bmp->Palette = NULL;
	}

	uint32_t bytes_per_row = bmp->Header.Width * bmp->Header.BitsPerPixel >> 3;
	bytes_per_row += ( bytes_per_row % 4 ? 4 - bytes_per_row % 4 : 0 );
	bmp->Header.ImageDataSize		= bytes_per_row * bmp->Header.Height;

	/* Allocate memory for image data */
	bmp->Data = (uint8_t*) malloc( bmp->Header.ImageDataSize );
	if ( bmp->Data == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
		free( bmp->Palette );
		free( bmp );

		POS = 0;
		return NULL;
	}

	/* Read image data */
	if ( myCopy(src, bmp->Data, bmp->Header.ImageDataSize) != bmp->Header.ImageDataSize )
	{
		BMP_LAST_ERROR_CODE = BMP_FILE_INVALID;
		free( bmp->Data );
		free( bmp->Palette );
		free( bmp );

		POS = 0;
		return NULL;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	POS = 0;

	return bmp;
}

size_t BMP_GetSizeInBytes(BMP* bmp) {

	size_t size = 0;

	size += BMP_PALETTE_SIZE;
	size += sizeof(BMP);
	size += bmp->Header.ImageDataSize;

	return size;
}


/**************************************************************
	Writes the BMP image to the specified file.
**************************************************************/
void BMP_WriteBuff(BMP* bmp, unsigned char* dest, size_t size)
{
	/* Write header */
	if ( WriteHeader( bmp, dest ) != BMP_OK )
	{

		POS = 0;

		BMP_LAST_ERROR_CODE = BMP_IO_ERROR;
		return;
	}

	/* Write palette */
	if ( bmp->Palette )
	{
		if ( myCopy(bmp->Palette, dest, BMP_PALETTE_SIZE, false) != BMP_PALETTE_SIZE )
		{
			POS = 0;

			BMP_LAST_ERROR_CODE = BMP_IO_ERROR;
			return;
		}
	}



	/* Write data */
	if (myCopy(bmp->Data, dest, bmp->Header.ImageDataSize, false) != bmp->Header.ImageDataSize )
	{
		POS = 0;

		BMP_LAST_ERROR_CODE = BMP_IO_ERROR;
		return;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	POS = 0;
}


/**************************************************************
	Returns the image's width.
**************************************************************/
uint32_t BMP_GetWidth( BMP* bmp )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return -1;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	return ( bmp->Header.Width );
}


/**************************************************************
	Returns the image's height.
**************************************************************/
uint32_t BMP_GetHeight( BMP* bmp )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return -1;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	return ( bmp->Header.Height );
}


/**************************************************************
	Returns the image's color depth (bits per pixel).
**************************************************************/
uint16_t BMP_GetDepth( BMP* bmp )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return -1;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	return ( bmp->Header.BitsPerPixel );
}


/**************************************************************
	Populates the arguments with the specified pixel's RGB
	values.
**************************************************************/
void BMP_GetPixelRGB( BMP* bmp, uint32_t x, uint32_t y, uint8_t* r, uint8_t* g, uint8_t* b )
{
	uint8_t*	pixel;
	uint32_t	bytes_per_row;
	uint8_t	bytes_per_pixel;

	if ( bmp == NULL || x < 0 || x >= bmp->Header.Width || y < 0 || y >= bmp->Header.Height )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
	}
	else
	{
		BMP_LAST_ERROR_CODE = BMP_OK;

		bytes_per_pixel = bmp->Header.BitsPerPixel >> 3;

		/* Row's size is rounded up to the next multiple of 4 bytes */
		bytes_per_row = bmp->Header.ImageDataSize / bmp->Header.Height;

		/* Calculate the location of the relevant pixel (rows are flipped) */
		pixel = bmp->Data + ( ( bmp->Header.Height - y - 1 ) * bytes_per_row + x * bytes_per_pixel );


		/* In indexed color mode the pixel's value is an index within the palette */
		if ( bmp->Header.BitsPerPixel == 8 )
		{
			pixel = bmp->Palette + *pixel * 4;
		}

		/* Note: colors are stored in BGR order */
		if ( r )	*r = *( pixel + 2 );
		if ( g )	*g = *( pixel + 1 );
		if ( b )	*b = *( pixel + 0 );
	}
}


/**************************************************************
	Sets the specified pixel's RGB values.
**************************************************************/
void BMP_SetPixelRGB( BMP* bmp, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b )
{
	uint8_t*	pixel;
	uint32_t	bytes_per_row;
	uint8_t	bytes_per_pixel;

	if ( bmp == NULL || x < 0 || x >= bmp->Header.Width || y < 0 || y >= bmp->Header.Height )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
	}

	else if ( bmp->Header.BitsPerPixel != 24 && bmp->Header.BitsPerPixel != 32 )
	{
		BMP_LAST_ERROR_CODE = BMP_TYPE_MISMATCH;
	}

	else
	{
		BMP_LAST_ERROR_CODE = BMP_OK;

		bytes_per_pixel = bmp->Header.BitsPerPixel >> 3;

		/* Row's size is rounded up to the next multiple of 4 bytes */
		bytes_per_row = bmp->Header.ImageDataSize / bmp->Header.Height;

		/* Calculate the location of the relevant pixel (rows are flipped) */
		pixel = bmp->Data + ( ( bmp->Header.Height - y - 1 ) * bytes_per_row + x * bytes_per_pixel );

		/* Note: colors are stored in BGR order */
		*( pixel + 2 ) = r;
		*( pixel + 1 ) = g;
		*( pixel + 0 ) = b;
	}
}


/**************************************************************
	Gets the specified pixel's color index.
**************************************************************/
void BMP_GetPixelIndex( BMP* bmp, uint32_t x, uint32_t y, uint8_t* val )
{
	uint8_t*	pixel;
	uint32_t	bytes_per_row;

	if ( bmp == NULL || x < 0 || x >= bmp->Header.Width || y < 0 || y >= bmp->Header.Height )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
	}

	else if ( bmp->Header.BitsPerPixel != 8 )
	{
		BMP_LAST_ERROR_CODE = BMP_TYPE_MISMATCH;
	}

	else
	{
		BMP_LAST_ERROR_CODE = BMP_OK;

		/* Row's size is rounded up to the next multiple of 4 bytes */
		bytes_per_row = bmp->Header.ImageDataSize / bmp->Header.Height;

		/* Calculate the location of the relevant pixel */
		pixel = bmp->Data + ( ( bmp->Header.Height - y - 1 ) * bytes_per_row + x );


		if ( val )	*val = *pixel;
	}
}


/**************************************************************
	Sets the specified pixel's color index.
**************************************************************/
void BMP_SetPixelIndex( BMP* bmp, uint32_t x, uint32_t y, uint8_t val )
{
	uint8_t*	pixel;
	uint32_t	bytes_per_row;

	if ( bmp == NULL || x < 0 || x >= bmp->Header.Width || y < 0 || y >= bmp->Header.Height )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
	}

	else if ( bmp->Header.BitsPerPixel != 8 )
	{
		BMP_LAST_ERROR_CODE = BMP_TYPE_MISMATCH;
	}

	else
	{
		BMP_LAST_ERROR_CODE = BMP_OK;

		/* Row's size is rounded up to the next multiple of 4 bytes */
		bytes_per_row = bmp->Header.ImageDataSize / bmp->Header.Height;

		/* Calculate the location of the relevant pixel */
		pixel = bmp->Data + ( ( bmp->Header.Height - y - 1 ) * bytes_per_row + x );

		*pixel = val;
	}
}


/**************************************************************
	Gets the color value for the specified palette index.
**************************************************************/
void BMP_GetPaletteColor( BMP* bmp, uint8_t index, uint8_t* r, uint8_t* g, uint8_t* b )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
	}

	else if ( bmp->Header.BitsPerPixel != 8 )
	{
		BMP_LAST_ERROR_CODE = BMP_TYPE_MISMATCH;
	}

	else
	{
		if ( r )	*r = *( bmp->Palette + index * 4 + 2 );
		if ( g )	*g = *( bmp->Palette + index * 4 + 1 );
		if ( b )	*b = *( bmp->Palette + index * 4 + 0 );

		BMP_LAST_ERROR_CODE = BMP_OK;
	}
}


/**************************************************************
	Sets the color value for the specified palette index.
**************************************************************/
void BMP_SetPaletteColor( BMP* bmp, uint8_t index, uint8_t r, uint8_t g, uint8_t b )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
	}

	else if ( bmp->Header.BitsPerPixel != 8 )
	{
		BMP_LAST_ERROR_CODE = BMP_TYPE_MISMATCH;
	}

	else
	{
		*( bmp->Palette + index * 4 + 2 ) = r;
		*( bmp->Palette + index * 4 + 1 ) = g;
		*( bmp->Palette + index * 4 + 0 ) = b;

		BMP_LAST_ERROR_CODE = BMP_OK;
	}
}


/**************************************************************
	Returns the last error code.
**************************************************************/
BMP_STATUS BMP_GetError()
{
	return BMP_LAST_ERROR_CODE;
}


/**************************************************************
	Returns a description of the last error code.
**************************************************************/
const char* BMP_GetErrorDescription()
{
	if ( BMP_LAST_ERROR_CODE > 0 && BMP_LAST_ERROR_CODE < BMP_ERROR_NUM )
	{
		return BMP_ERROR_STRING[ BMP_LAST_ERROR_CODE ];
	}
	else
	{
		return NULL;
	}
}





/*********************************** Private methods **********************************/


/**************************************************************
	Reads the BMP file's header into the data structure.
	Returns BMP_OK on success.
**************************************************************/
int	ReadHeader( BMP* bmp, unsigned char* f )
{
	if ( bmp == NULL || f == NULL )
	{
		return BMP_INVALID_ARGUMENT;
	}

	/* The header's fields are read one by one, and converted from the format's
	little endian to the system's native representation. */
	if ( !ReadUSHORT( &( bmp->Header.Magic ), f ) )			return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.FileSize ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.Reserved1 ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.Reserved2 ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.DataOffset ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.HeaderSize ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.Width ), f ) )			return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.Height ), f ) )			return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.Planes ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.BitsPerPixel ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.CompressionType ), f ) )	return BMP_IO_ERROR;
	
	if ( !ReadUINT( &( bmp->Header.ImageDataSize ), f ) ) {



		return BMP_IO_ERROR;
	}


	if ( !ReadUINT( &( bmp->Header.HPixelsPerMeter ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.VPixelsPerMeter ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.ColorsUsed ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.ColorsRequired ), f ) )	return BMP_IO_ERROR;

	return BMP_OK;
}


/**************************************************************
	Writes the BMP file's header into the data structure.
	Returns BMP_OK on success.
**************************************************************/
int	WriteHeader( BMP* bmp, unsigned char* f )
{
	/* The header's fields are written one by one, and converted to the format's
	little endian representation. */
	if ( !WriteUSHORT( bmp->Header.Magic, f ) )			return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.FileSize, f ) )		return BMP_IO_ERROR;
	if ( !WriteUSHORT( bmp->Header.Reserved1, f ) )		return BMP_IO_ERROR;
	if ( !WriteUSHORT( bmp->Header.Reserved2, f ) )		return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.DataOffset, f ) )		return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.HeaderSize, f ) )		return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.Width, f ) )			return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.Height, f ) )			return BMP_IO_ERROR;
	if ( !WriteUSHORT( bmp->Header.Planes, f ) )		return BMP_IO_ERROR;
	if ( !WriteUSHORT( bmp->Header.BitsPerPixel, f ) )	return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.CompressionType, f ) )	return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.ImageDataSize, f ) )	return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.HPixelsPerMeter, f ) )	return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.VPixelsPerMeter, f ) )	return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.ColorsUsed, f ) )		return BMP_IO_ERROR;
	if ( !WriteUINT( bmp->Header.ColorsRequired, f ) )	return BMP_IO_ERROR;

	return BMP_OK;
}


/**************************************************************
	Reads a little-endian unsigned int from the file.
	Returns non-zero on success.
**************************************************************/
int	ReadUINT( uint32_t* x, unsigned char* f )
{
	uint8_t little[ 4 ];	/* BMPs use 32 bit ints */

	myCopy(f, little, 4);

	*(unsigned long int*)x = ( little[ 3 ] << 24 | little[ 2 ] << 16 | little[ 1 ] << 8 | little[ 0 ] );

	return 1;
}


/**************************************************************
	Reads a little-endian unsigned short int from the file.
	Returns non-zero on success.
**************************************************************/
int	ReadUSHORT( uint16_t *x, unsigned char* f )
{
	uint8_t little[ 2 ];	/* BMPs use 16 bit shorts */

	myCopy(f, little, 2 );

	*(uint16_t*)x = ( little[ 1 ] << 8 | little[ 0 ] );

	return 1;
}


/**************************************************************
	Writes a little-endian unsigned int to the file.
	Returns non-zero on success.
**************************************************************/
int	WriteUINT( uint32_t x, unsigned char* f )
{
	uint8_t little[ 4 ];	/* BMPs use 32 bit ints */

	little[ 3 ] = (uint8_t)( ( x & 0xff000000 ) >> 24 );
	little[ 2 ] = (uint8_t)( ( x & 0x00ff0000 ) >> 16 );
	little[ 1 ] = (uint8_t)( ( x & 0x0000ff00 ) >> 8 );
	little[ 0 ] = (uint8_t)( ( x & 0x000000ff ) >> 0 );

	myCopy(little, f, 4, false);

	return 1;
}


/**************************************************************
	Writes a little-endian unsigned short int to the file.
	Returns non-zero on success.
**************************************************************/
int	WriteUSHORT( uint16_t x, unsigned char* f )
{
	uint8_t little[ 2 ];	/* BMPs use 16 bit shorts */

	little[ 1 ] = (uint8_t)( ( x & 0xff00 ) >> 8 );
	little[ 0 ] = (uint8_t)( ( x & 0x00ff ) >> 0 );

	myCopy(little, f, 2, false);

	return 1;
}

