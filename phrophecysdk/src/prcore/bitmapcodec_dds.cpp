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
		BitmapCodec "dds"

	revision history:
		Sep/07/2000 - Jukka Liimatta - initial revision
		Jan/24/2001 - Jukka Liimatta - renaissance build
		Feb/21/2002 - Jukka Liimatta - rewrote codec interface
		
	todo:
	- endianess aware "read surface description"
	- find if there is a flag which tells when image must be mirrored around y-axis
	- is the nvidia implementation endianess aware? If not, fix.

	note:
	- the "dxtc" codec is a simple wrapper around the nVidia S3TC/DXTC decompressor.
	- the original implementation can be downloaded from http://www.nvidia.com
*/
#include <cassert>
#include <prcore/endian.hpp>
#include <prcore/bitmap.hpp>

using namespace prcore;


// =================================================
// nvidia s3tc code
// =================================================

struct DXTColBlock
{
	uint16 col0;
	uint16 col1;
	char row[4];
};

struct DXTAlphaBlockExplicit
{
	uint16 row[4];
};

struct DXTAlphaBlock3BitLinear
{
	char alpha0;
	char alpha1;
	char stuff[6];
};

struct Color565
{
	unsigned nBlue:5;
	unsigned nGreen:6;
	unsigned nRed:5;
};


static inline void GetColorBlockColors(
	DXTColBlock* pBlock, 
	Color32* col_0, 
	Color32* col_1,
	Color32* col_2, 
	Color32* col_3,
	uint16& wrd  
	)
{
	// There are 4 methods to use - see the Time_ functions.
	// 1st = shift = does normal approach per byte for color comps
	// 2nd = use freak variable bit field color565 for component extraction
	// 3rd = use super-freak DWORD adds BEFORE shifting the color components
	//  This lets you do only 1 add per color instead of 3 BYTE adds and
	//  might be faster
	// Call RunTimingSession() to run each of them & output result to txt file

 
	// freak variable bit structure method
	// normal math
	// This method is fastest

	Color565* pCol;
	pCol = (Color565*) & (pBlock->col0);

	col_0->a = 0xff;
	col_0->r = pCol->nRed;
	col_0->r <<= 3;				// shift to full precision
	col_0->g = pCol->nGreen;
	col_0->g <<= 2;
	col_0->b = pCol->nBlue;
	col_0->b <<= 3;

	pCol = (Color565*) & (pBlock->col1 );
	col_1->a = 0xff;
	col_1->r = pCol->nRed;
	col_1->r <<= 3;				// shift to full precision
	col_1->g = pCol->nGreen;
	col_1->g <<= 2;
	col_1->b = pCol->nBlue;
	col_1->b <<= 3;


	if( pBlock->col0 > pBlock->col1 )
	{
		// Four-color block: derive the other two colors.    
		// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block.

		wrd = ((uint16)col_0->r * 2 + (uint16)col_1->r )/3;
											// no +1 for rounding
											// as bits have been shifted to 888
		col_2->r = (uint8)wrd;

		wrd = ((uint16)col_0->g * 2 + (uint16)col_1->g )/3;
		col_2->g = (uint8)wrd;

		wrd = ((uint16)col_0->b * 2 + (uint16)col_1->b )/3;
		col_2->b = (uint8)wrd;
		col_2->a = 0xff;

		wrd = ((uint16)col_0->r + (uint16)col_1->r *2 )/3;
		col_3->r = (uint8)wrd;

		wrd = ((uint16)col_0->g + (uint16)col_1->g *2 )/3;
		col_3->g = (uint8)wrd;

		wrd = ((uint16)col_0->b + (uint16)col_1->b *2 )/3;
		col_3->b = (uint8)wrd;
		col_3->a = 0xff;

	}
	else
	{
		// Three-color block: derive the other color.
		// 00 = color_0,  01 = color_1,  10 = color_2,  
		// 11 = transparent.
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block. 

		// explicit for each component, unlike some refrasts...

		wrd = ((uint16)col_0->r + (uint16)col_1->r )/2;		col_2->r = (uint8)wrd;
		wrd = ((uint16)col_0->g + (uint16)col_1->g )/2;		col_2->g = (uint8)wrd;
		wrd = ((uint16)col_0->b + (uint16)col_1->b )/2;		col_2->b = (uint8)wrd;
		col_2->a = 0xff;

		col_3->r = 0x00;		// random color to indicate alpha
		col_3->g = 0xff;
		col_3->b = 0xff;
		col_3->a = 0x00;
	}
}


static inline void DecodeColorBlock(
	uint32* pImPos, DXTColBlock* pColorBlock, int width,
	uint32* col_0, uint32* col_1, uint32* col_2, uint32* col_3 
	)
{
	// width is width of image in pixels
	uint32 bits;
	int r,n;

	// bit masks = 00000011, 00001100, 00110000, 11000000
	const uint32 masks[] = { 3, 12, 3 << 4, 3 << 6 };
	const int shift[] = { 0, 2, 4, 6 };

	// r steps through lines in y
	for( r=0; r < 4; r++, pImPos += width-4 )	// no width*4 as DWORD ptr inc will *4
	{

		// width * 4 bytes per pixel per line
		// each j dxtc row is 4 lines of pixels

		// pImPos = (DWORD*)((DWORD)pBase + i*16 + (r+j*4) * m_nWidth * 4 );

		// n steps through pixels
		for( n=0; n < 4; n++ )
		{
			bits =		pColorBlock->row[r] & masks[n];
			bits >>=	shift[n];

			switch( bits )
			{
			case 0 :
				*pImPos = *col_0;
				pImPos++; // increment to next DWORD
				break;
			case 1 :
				*pImPos = *col_1;
				pImPos++;
				break;
			case 2 :
				*pImPos = *col_2;
				pImPos++;
				break;
			case 3 :
				*pImPos = *col_3;
				pImPos++;
				break;
			default:
				pImPos++;
				break;
			}
		}
	}
}


static inline void DecodeAlphaExplicit(
	uint32* pImPos, DXTAlphaBlockExplicit* pAlphaBlock,
	int width, uint32 alphazero )
{
	// alphazero is a bit mask that when & with the image color
	//  will zero the alpha bits, so if the image DWORDs  are
	//  ARGB then alphazero will be 0x00ffffff or if
	//  RGBA then alphazero will be 0xffffff00
	//  alphazero constructed automaticaly from field order of Color structure

	// decodes to 32 bit format only

	int row, pix;
	uint16 wrd;

	Color32 col;
	col.r = col.g = col.b = 0;

	for( row=0; row < 4; row++, pImPos += width-4 )
	{
		// pImPow += pImPos += width-4 moves to next row down

		wrd = pAlphaBlock->row[ row ];

		for( pix = 0; pix < 4; pix++ )
		{
			// zero the alpha bits of image pixel
			*pImPos &= alphazero;

			col.a = wrd & 0x000f;		// get only low 4 bits
//			col.a <<= 4;				// shift to full byte precision
										// NOTE:  with just a << 4 you'll never have alpha
										// of 0xff,  0xf0 is max so pure shift doesn't quite
										// cover full alpha range.
										// It's much cheaper than divide & scale though.
										// To correct for this, and get 0xff for max alpha,
										//  or the low bits back in after left shifting
			col.a = col.a | (col.a << 4 );	// This allows max 4 bit alpha to be 0xff alpha
											//  in final image, and is crude approach to full 
											//  range scale

			*pImPos |= *((uint32*)&col);	// or the bits into the prev. nulled alpha

			wrd >>= 4;		// move next bits to lowest 4
			pImPos++;		// move to next pixel in the row

		}
	}
}


static char		gBits[4][4];
static uint16	gAlphas[8];
static Color32	gACol[4][4];


static inline void DecodeAlpha3BitLinear(
	uint32* pImPos, DXTAlphaBlock3BitLinear* pAlphaBlock, 
	int width, uint32 alphazero
	)
{
	gAlphas[0] = pAlphaBlock->alpha0;
	gAlphas[1] = pAlphaBlock->alpha1;

	
	// 8-alpha or 6-alpha block?    
	if( gAlphas[0] > gAlphas[1] )
	{
		// 8-alpha block:  derive the other 6 alphas.    
		// 000 = alpha_0, 001 = alpha_1, others are interpolated

		gAlphas[2] = ( 6 * gAlphas[0] +     gAlphas[1]) / 7;	// bit code 010
		gAlphas[3] = ( 5 * gAlphas[0] + 2 * gAlphas[1]) / 7;	// Bit code 011    
		gAlphas[4] = ( 4 * gAlphas[0] + 3 * gAlphas[1]) / 7;	// Bit code 100    
		gAlphas[5] = ( 3 * gAlphas[0] + 4 * gAlphas[1]) / 7;	// Bit code 101
		gAlphas[6] = ( 2 * gAlphas[0] + 5 * gAlphas[1]) / 7;	// Bit code 110    
		gAlphas[7] = (     gAlphas[0] + 6 * gAlphas[1]) / 7;	// Bit code 111
	}    
	else
	{
		// 6-alpha block:  derive the other alphas.    
		// 000 = alpha_0, 001 = alpha_1, others are interpolated

		gAlphas[2] = (4 * gAlphas[0] +     gAlphas[1]) / 5;	// Bit code 010
		gAlphas[3] = (3 * gAlphas[0] + 2 * gAlphas[1]) / 5;	// Bit code 011    
		gAlphas[4] = (2 * gAlphas[0] + 3 * gAlphas[1]) / 5;	// Bit code 100    
		gAlphas[5] = (    gAlphas[0] + 4 * gAlphas[1]) / 5;	// Bit code 101
		gAlphas[6] = 0;										// Bit code 110
		gAlphas[7] = 255;									// Bit code 111
	}


	// Decode 3-bit fields into array of 16 BYTES with same value

	// first two rows of 4 pixels each:
	// pRows = (Alpha3BitRows*) & ( pAlphaBlock->stuff[0] );
	const uint32 mask = 0x00000007;		// bits = 00 00 01 11

	uint32 bits = *( (uint32*) & ( pAlphaBlock->stuff[0] ));

	gBits[0][0] = (uint8)( bits & mask );	bits >>= 3;
	gBits[0][1] = (uint8)( bits & mask );	bits >>= 3;
	gBits[0][2] = (uint8)( bits & mask );	bits >>= 3;
	gBits[0][3] = (uint8)( bits & mask );	bits >>= 3;
	gBits[1][0] = (uint8)( bits & mask );	bits >>= 3;
	gBits[1][1] = (uint8)( bits & mask );	bits >>= 3;
	gBits[1][2] = (uint8)( bits & mask );	bits >>= 3;
	gBits[1][3] = (uint8)( bits & mask );

	// now for last two rows:
	bits = *( (uint32*) & ( pAlphaBlock->stuff[3] ));		// last 3 bytes

	gBits[2][0] = (uint8)( bits & mask );	bits >>= 3;
	gBits[2][1] = (uint8)( bits & mask );	bits >>= 3;
	gBits[2][2] = (uint8)( bits & mask );	bits >>= 3;
	gBits[2][3] = (uint8)( bits & mask );	bits >>= 3;
	gBits[3][0] = (uint8)( bits & mask );	bits >>= 3;
	gBits[3][1] = (uint8)( bits & mask );	bits >>= 3;
	gBits[3][2] = (uint8)( bits & mask );	bits >>= 3;
	gBits[3][3] = (uint8)( bits & mask );

	// decode the codes into alpha values
	int row, pix;

	for( row = 0; row < 4; row++ )
	{
		for( pix=0; pix < 4; pix++ )
		{
			int index = gBits[row][pix];
			gACol[row][pix].a = static_cast<uint8>(gAlphas[index]);
		}
	}

	// Write out alpha values to the image bits
	for( row=0; row < 4; row++, pImPos += width-4 )
	{
		// pImPow += pImPos += width-4 moves to next row down

		for( pix = 0; pix < 4; pix++ )
		{
			// zero the alpha bits of image pixel
			*pImPos &=  alphazero;

			*pImPos |=  *((uint32*) &(gACol[row][pix]));	// or the bits into the prev. nulled alpha
			pImPos++;
		}
	}
}


static void DecodeDXT1(int width, int height, uint32* out, uint16* in)
{
	int xblocks = width / 4;
	int yblocks = height / 4;

	for ( int j=0; j<yblocks; ++j )
	{
		// 8 bytes per block
		DXTColBlock* pBlock = (DXTColBlock*) ( (uint32)in + j * xblocks * 8 );

		for ( int i=0; i<xblocks; i++, pBlock++ )
		{
			Color32 col_0, col_1, col_2, col_3;
			uint16 wrd;

			GetColorBlockColors( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );

			uint32* pImPos = (uint32*)((uint32)out + i*16 + (j*4) * width * 4 );

			DecodeColorBlock( pImPos, pBlock, width, 
				(uint32*)&col_0, (uint32*)&col_1,	(uint32*)&col_2, (uint32*)&col_3 );
		}
	}
}


static void DecodeDXT2(int width, int height, uint32* out, uint16* in)
{
	// Can do color & alpha same as dxt3, but color is pre-multiplied 
	//   so the result will be wrong unless corrected. 
	// DecompressDXT3();
}


static void DecodeDXT3(int width, int height, uint32* out, uint16* in)
{
	int xblocks = width / 4;
	int yblocks = height / 4;

	Color32 col_0, col_1, col_2, col_3;

	// fill alphazero with appropriate value to zero out alpha when
	//  alphazero is ANDed with the image color 32 bit DWORD:
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;
	uint32 alphazero = *((uint32*) &col_0);

	for ( int j=0; j<yblocks; j++ )
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color
		DXTColBlock* pBlock = (DXTColBlock*) ( (uint32)in + j * xblocks * 16 );

		for ( int i=0; i<xblocks; i++, pBlock++ )
		{
			// Get alpha block
			DXTAlphaBlockExplicit* pAlphaBlock = (DXTAlphaBlockExplicit*) pBlock;

			// Get color block & colors
			pBlock++;
			uint16 wrd;
			GetColorBlockColors( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );

			// Decode the color block into the bitmap bits
			uint32* pImPos = (uint32*)((uint32)out + i*16 + (j*4) * width * 4 );

			DecodeColorBlock( pImPos, pBlock, width, 
				(uint32*)&col_0, (uint32*)&col_1,	(uint32*)&col_2, (uint32*)&col_3 );

			// Overwrite the previous alpha bits with the alpha block
			DecodeAlphaExplicit( pImPos, pAlphaBlock, width, alphazero );
		}
	}
}


static void DecodeDXT4(int width, int height, uint32* out, uint16* in)
{
	// Can do color & alpha same as dxt5, but color is pre-multiplied 
	//   so the result will be wrong unless corrected. 
	// DecompressDXT5();
}


static void DecodeDXT5(int width, int height, uint32* out, uint16* in)
{
	int xblocks = width / 4;
	int yblocks = height / 4;

	uint32* pBase = out;
	//uint16* pPos = in;

	Color32 col_0, col_1, col_2, col_3;
	uint16 wrd;

	// fill alphazero with appropriate value to zero out alpha when
	//  alphazero is ANDed with the image color 32 bit uint32:
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;
	uint32 alphazero = *((uint32*) &col_0);

	for ( int j=0; j<yblocks; j++ )
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color
		DXTColBlock* pBlock = (DXTColBlock*) ( (uint32)in + j * xblocks * 16 );

		for ( int i=0; i<xblocks; i++, pBlock++ )
		{
			// Get alpha block
			DXTAlphaBlock3BitLinear* pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;

			// Get color block & colors
			pBlock++;
			GetColorBlockColors( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );

			// Decode the color block into the bitmap bits
			uint32* pImPos = (uint32*)((uint32)pBase + i*16 + (j*4) * width * 4 );

			DecodeColorBlock( pImPos, pBlock, width, 
				(uint32*)&col_0, (uint32*)&col_1, (uint32*)&col_2, (uint32*)&col_3 );

			// Overwrite the previous alpha bits with the alpha block
			DecodeAlpha3BitLinear( pImPos, pAlphaBlock, width, alphazero );
		}
	}
}


// =================================================
// DirectX datatypes
// =================================================

struct DDPIXELFORMAT
{
    uint32       dwSize;                 // size of structure
    uint32       dwFlags;                // pixel format flags
    uint32       dwFourCC;               // (FOURCC code)
    union
    {
        uint32   dwRGBBitCount;          // how many bits per pixel
        uint32   dwYUVBitCount;          // how many bits per pixel
        uint32   dwZBufferBitDepth;      // how many total bits/pixel in z buffer (including any stencil bits)
        uint32   dwAlphaBitDepth;        // how many bits for alpha channels
        uint32   dwLuminanceBitCount;    // how many bits per pixel
        uint32   dwBumpBitCount;         // how many bits per "buxel", total
        uint32   dwPrivateFormatBitCount;// Bits per pixel of private driver formats. Only valid in texture
                                         // format list and if DDPF_D3DFORMAT is set
    };
    union
    {
        uint32   dwRBitMask;             // mask for red bit
        uint32   dwYBitMask;             // mask for Y bits
        uint32   dwStencilBitDepth;      // how many stencil bits (note: dwZBufferBitDepth-dwStencilBitDepth is total Z-only bits)
        uint32   dwLuminanceBitMask;     // mask for luminance bits
        uint32   dwBumpDuBitMask;        // mask for bump map U delta bits
        uint32   dwOperations;           // DDPF_D3DFORMAT Operations
    };
    union
    {
        uint32   dwGBitMask;             // mask for green bits
        uint32   dwUBitMask;             // mask for U bits
        uint32   dwZBitMask;             // mask for Z bits
        uint32   dwBumpDvBitMask;        // mask for bump map V delta bits
        struct
        {
            uint16    wFlipMSTypes;       // Multisample methods supported via flip for this D3DFORMAT
            uint16    wBltMSTypes;        // Multisample methods supported via blt for this D3DFORMAT
        } MultiSampleCaps;

    };
    union
    {
        uint32   dwBBitMask;             // mask for blue bits
        uint32   dwVBitMask;             // mask for V bits
        uint32   dwStencilBitMask;       // mask for stencil bits
        uint32   dwBumpLuminanceBitMask; // mask for luminance in bump map
    };
    union
    {
        uint32   dwRGBAlphaBitMask;      // mask for alpha channel
        uint32   dwYUVAlphaBitMask;      // mask for alpha channel
        uint32   dwLuminanceAlphaBitMask;// mask for alpha channel
        uint32   dwRGBZBitMask;          // mask for Z channel
        uint32   dwYUVZBitMask;          // mask for Z channel
    };
};

struct DDCOLORKEY
{
    uint32       dwColorSpaceLowValue;   // low boundary of color space that is to be treated as Color Key, inclusive
    uint32       dwColorSpaceHighValue;  // high boundary of color space that is to be treated as Color Key, inclusive
};

struct DDSCAPS2
{
    uint32 dwCaps;         // capabilities of surface wanted
    uint32 dwCaps2;
    uint32 dwCaps3;
    union
    {
        uint32 dwCaps4;
        uint32 dwVolumeDepth;
    };
};

struct DDSURFACEDESC2
{
    uint32               dwSize;                 // size of the DDSURFACEDESC structure
    uint32               dwFlags;                // determines what fields are valid
    uint32               dwHeight;               // height of surface to be created
    uint32               dwWidth;                // width of input surface
    union
    {
        int32            lPitch;                 // distance to start of next line (return value only)
        uint32           dwLinearSize;           // Formless late-allocated optimized surface size
    };
    union
    {
        uint32           dwBackBufferCount;      // number of back buffers requested
        uint32           dwDepth;                // the depth if this is a volume texture 
    };
    union
    {
        uint32           dwMipMapCount;          // number of mip-map levels requestde
                                                 // dwZBufferBitDepth removed, use ddpfPixelFormat one instead
        uint32           dwRefreshRate;          // refresh rate (used when display mode is described)
        uint32           dwSrcVBHandle;          // The source used in VB::Optimize
    };
    uint32               dwAlphaBitDepth;        // depth of alpha buffer requested
    uint32               dwReserved;             // reserved
    void*              lpSurface;                // pointer to the associated surface memory
    union
    {
        DDCOLORKEY      ddckCKDestOverlay;       // color key for destination overlay use
        uint32           dwEmptyFaceColor;       // Physical color for empty cubemap faces
    };
    DDCOLORKEY          ddckCKDestBlt;           // color key for destination blt use
    DDCOLORKEY          ddckCKSrcOverlay;        // color key for source overlay use
    DDCOLORKEY          ddckCKSrcBlt;            // color key for source blt use
    union
    {
        DDPIXELFORMAT   ddpfPixelFormat;         // pixel format description of the surface
        uint32           dwFVF;                  // vertex format description of vertex buffers
    };
    DDSCAPS2            ddsCaps;                 // direct draw surface capabilities
    uint32               dwTextureStage;         // stage in multitexture cascade
};

#define DDSD_LINEARSIZE         0x00080000l


// =================================================
// select decoder
// =================================================

typedef void (*FuncDXT)(int width, int height, uint32* out, uint16* in);


static FuncDXT GetFuncDXT(DDPIXELFORMAT& ddpxf)
{
	switch ( ddpxf.dwFourCC )
	{
        case PRCORE_CODE32('D','X','T','1'): return DecodeDXT1;
        case PRCORE_CODE32('D','X','T','2'): return DecodeDXT2;
        case PRCORE_CODE32('D','X','T','3'): return DecodeDXT3;
        case PRCORE_CODE32('D','X','T','4'): return DecodeDXT4;
        case PRCORE_CODE32('D','X','T','5'): return DecodeDXT5;
		default: return NULL;
	}
}


// =================================================
// codec
// =================================================

static bool dds_decode(Bitmap& target, Stream& stream)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// check header
	uint32 magic = ReadLittleEndian<uint32>(stream);
	if ( magic != PRCORE_CODE32('D','D','S',' ') )
		return false;

	// read surface description
	// TODO: read in endianess aware format
	DDSURFACEDESC2 ddsd;
	stream.Read((char*)&ddsd,sizeof(ddsd));

	//TODO: note for future generations:
	//bool IsMip = (ddsd.dwMipMapCount > 0) ? true : false;
	FuncDXT func = GetFuncDXT(ddsd.ddpfPixelFormat);

	if ( func == NULL )
		return false;

	int width = ddsd.dwWidth;
	int height = ddsd.dwHeight;
	assert( (width % 4) == 0 );
	assert( (height % 4) == 0 );

	// read first miplevel
	char* bytes = NULL;

	if ( ddsd.dwFlags & DDSD_LINEARSIZE )
	{
		bytes = new char[ddsd.dwLinearSize];
		stream.Read(bytes,ddsd.dwLinearSize);
	}
	else
	{
		int BytesPerRow = width * ddsd.ddpfPixelFormat.dwRGBBitCount / 8;
		bytes = new char[ddsd.lPitch * height];

		char* dest = bytes;
		for ( int yp=0; yp<height; yp++ )
		{
			stream.Read(dest,BytesPerRow);
			dest += ddsd.lPitch;
		}
	}

	// set image
	char* buffer = new char[width*height*4];
	target.SetImage(width,height,PIXELFORMAT_ARGB8888,buffer);

	// decode
	func(width,height,reinterpret_cast<uint32*>(buffer),reinterpret_cast<uint16*>(bytes));

	// release
	delete[] bytes;

	return true;
}


// =================================================
// CreateCodecDDS()
// =================================================

BitmapCodec CreateCodecDDS()
{
	static const String ext[] = { "dds" };

	BitmapCodec codec;

	codec.count     = sizeof(ext) / sizeof(ext[0]);
	codec.extension = ext;
	codec.decode    = dds_decode;
	codec.encode    = NULL;

	return codec;
}
