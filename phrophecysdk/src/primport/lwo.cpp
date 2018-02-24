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
		NewTek Lightwave object v5 (.lwo) reader

	revision history:
		Apr/14/1999 - Jukka Liimatta - initial revision
		Feb/10/2001 - Jukka Liimatta - renaissance build
		Nov/16/2001 - Mats Byggmastar - suggested that the IFF chunk padding should be supported
*/
#include <primport/lwo.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// reader declaration
// =================================================

class ReaderLWO
{
	public:

	ReaderLWO(ImportLWO& import, Stream& stream);
	~ReaderLWO();

	private:

	ImportLWO&		import;
	Stream&			stream;
	SurfaceLWO*		cursurface;
	TextureLWO*		curtexture;

	void	Switch(bool subchunk);
	void	ChunkPNTS(int length);
	void	ChunkPOLS(int length);
	void	ChunkSRFS(int length);
	void	ChunkSURF(int length);
	void	ChunkCOLR(int length);
	void	ChunkFLAG(int length);
	void	ChunkCTEX(int length);
	void	ChunkDTEX(int length);
	void	ChunkSTEX(int length);
	void	ChunkRTEX(int length);
	void	ChunkTTEX(int length);
	void	ChunkBTEX(int length);
	void	ChunkTIMG(int length);
	void	ChunkTFLG(int length);
	void	ChunkTSIZ(int length);
	void	ChunkTCTR(int length);
	void	ChunkTFAL(int length);
	void	ChunkTVEL(int length);
	void	ChunkTWRP(int length);
	void	ChunkTFP0(int length);
	void	ChunkTFP1(int length);
	void	ChunkLUMI(int length);
	void	ChunkDIFF(int length);
	void	ChunkSPEC(int length);
	void	ChunkREFL(int length);
	void	ChunkTRAN(int length);

	vec3f	ReadVector3();
	char*	ReadAscii();
	int		ReadMapStyle();
	void	SetupTexture(TextureLWO& texture);
	void	SetupSurface(SurfaceLWO& surface, const char* name);
};


// =================================================
// reader implementation
// =================================================

ReaderLWO::ReaderLWO(ImportLWO& i, Stream& s)
: import(i), stream(s)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// check header
	uint32 id = ReadBigEndian<uint32>(stream);
	if ( id != PRCORE_BIGCODE32('F','O','R','M') )
		PRCORE_EXCEPTION("not a valif iff/lwo stream.");

	//int size = 
		ReadBigEndian<uint32>(stream);
	stream.Seek(4,Stream::CURRENT);

	while ( !stream.IsEOS() )
	{
		Switch(false);
	}
}


ReaderLWO::~ReaderLWO()
{
}


void ReaderLWO::Switch(bool subchunk)
{
	uint32 id = ReadBigEndian<uint32>(stream);
	int32 length = subchunk ? 
		ReadBigEndian<uint16>(stream) :
		ReadBigEndian<uint32>(stream);

	switch ( id )
	{
		case PRCORE_BIGCODE32('P','N','T','S'): ChunkPNTS(length); break;	// points
		case PRCORE_BIGCODE32('P','O','L','S'): ChunkPOLS(length); break;	// polygons
		case PRCORE_BIGCODE32('S','R','F','S'): ChunkSRFS(length); break;	// surfaces
		case PRCORE_BIGCODE32('S','U','R','F'): ChunkSURF(length); break;	// surface index
		case PRCORE_BIGCODE32('C','O','L','R'): ChunkCOLR(length); break;	// color
		case PRCORE_BIGCODE32('F','L','A','G'): ChunkFLAG(length); break;	// flags
		case PRCORE_BIGCODE32('C','T','E','X'): ChunkCTEX(length); break;	// tmap: color
		case PRCORE_BIGCODE32('D','T','E','X'): ChunkDTEX(length); break;	// tmap: diffuse
		case PRCORE_BIGCODE32('S','T','E','X'): ChunkSTEX(length); break;	// tmap: specular
		case PRCORE_BIGCODE32('R','T','E','X'): ChunkRTEX(length); break;	// tmap: reflection
		case PRCORE_BIGCODE32('T','T','E','X'): ChunkTTEX(length); break;	// tmap: transparency
		case PRCORE_BIGCODE32('B','T','E','X'): ChunkBTEX(length); break;	// tmap: bump
		case PRCORE_BIGCODE32('T','I','M','G'): ChunkTIMG(length); break;	// texture filename
		case PRCORE_BIGCODE32('T','F','L','G'): ChunkTFLG(length); break;	// texture flags
		case PRCORE_BIGCODE32('T','S','I','Z'): ChunkTSIZ(length); break;	// texture size
		case PRCORE_BIGCODE32('T','C','T','R'): ChunkTCTR(length); break;	// texture center
		case PRCORE_BIGCODE32('T','F','A','L'): ChunkTFAL(length); break;	// texture center
		case PRCORE_BIGCODE32('T','V','E','L'): ChunkTVEL(length); break;	// texture center
		case PRCORE_BIGCODE32('T','W','R','P'): ChunkTWRP(length); break;	// texture wrap
		case PRCORE_BIGCODE32('T','F','P','0'): ChunkTFP0(length); break;	// texture xtile
		case PRCORE_BIGCODE32('T','F','P','1'): ChunkTFP1(length); break;	// texture ytile
		case PRCORE_BIGCODE32('L','U','M','I'): ChunkLUMI(length); break;	// luminosity
		case PRCORE_BIGCODE32('D','I','F','F'): ChunkDIFF(length); break;	// diffuse
		case PRCORE_BIGCODE32('S','P','E','C'): ChunkSPEC(length); break;	// specular
		case PRCORE_BIGCODE32('R','E','F','L'): ChunkREFL(length); break;	// reflection
		case PRCORE_BIGCODE32('T','R','A','N'): ChunkTRAN(length); break;	// transparency
		default: stream.Seek(length,Stream::CURRENT); break; // unknown
	};
	
	// chunk alignment
	if ( length & 1 )
		stream.Seek(1,Stream::CURRENT);
}


void ReaderLWO::ChunkPNTS(int length)
{
	int count = length / sizeof(point3f);
	import.points.SetSize(count);

	for ( int i=0; i<count; ++i )
	{
		import.points[i] = ReadVector3();
	}
}


void ReaderLWO::ChunkPOLS(int length)
{
	int max = stream.GetOffset() + length;

	while ( stream.GetOffset() < max )
	{
		uint16 numvert = ReadBigEndian<uint16>(stream);

		PolygonLWO poly;
		poly.vertices.SetSize(numvert);

		for ( int i=0; i<numvert; ++i )
		{
			poly.vertices[i] = ReadBigEndian<uint16>(stream);
		}
		int16 sid = ReadBigEndian<int16>(stream);

		if ( sid < 0 )
		{
			// detail polygons
			for ( ; sid<0; ++sid )
			{
				uint16 numvert = ReadBigEndian<uint16>(stream);
				stream.Seek(numvert * sizeof(uint16),Stream::CURRENT);
			}
		}
		else
		{
			// surface id
			poly.surface = sid - 1;
			import.polygons.PushBack(poly);
		}
	}
}


void ReaderLWO::ChunkSRFS(int length)
{
	int end = stream.GetOffset() + length;
	while ( stream.GetOffset() < end )
	{
		char* name = ReadAscii();

		SurfaceLWO surface;
		SetupSurface(surface,name);
		import.surfaces.PushBack(surface);
	}
}


void ReaderLWO::ChunkSURF(int length)
{
	int end = stream.GetOffset() + length;
	char* name = ReadAscii();

	cursurface = NULL;
	for ( int i=0; i<import.surfaces.GetSize(); ++i )
	{
		if ( strcmp(import.surfaces[i].name,name) == 0 )
		{
			cursurface = &import.surfaces[i];
			break;
		}
	}

	while ( stream.GetOffset() < end )
	{
		Switch(true);
	}
}


void ReaderLWO::ChunkCOLR(int)
{
	uint8 r = ReadBigEndian<uint8>(stream);
	uint8 g = ReadBigEndian<uint8>(stream);
	uint8 b = ReadBigEndian<uint8>(stream);
	ReadBigEndian<uint8>(stream); // ignore

	cursurface->color = Color32(r,g,b,0xff);
}


void ReaderLWO::ChunkFLAG(int)
{
	cursurface->flags = ReadBigEndian<uint16>(stream);
}


void ReaderLWO::ChunkCTEX(int)
{
	curtexture = &cursurface->ctex;
	curtexture->style = ReadMapStyle();
}


void ReaderLWO::ChunkDTEX(int)
{
	curtexture = &cursurface->dtex;
	curtexture->style = ReadMapStyle();
}


void ReaderLWO::ChunkSTEX(int)
{
	curtexture = &cursurface->stex;
	curtexture->style = ReadMapStyle();
}


void ReaderLWO::ChunkRTEX(int)
{
	curtexture = &cursurface->rtex;
	curtexture->style = ReadMapStyle();
}


void ReaderLWO::ChunkTTEX(int)
{
	curtexture = &cursurface->ttex;
	curtexture->style = ReadMapStyle();
}


void ReaderLWO::ChunkBTEX(int)
{
	curtexture = &cursurface->btex;
	curtexture->style = ReadMapStyle();
}


void ReaderLWO::ChunkTIMG(int)
{
	char* name = ReadAscii();
	if ( strcmp(name,"(none)") )
	{
		curtexture->filename = name;
	}
}


void ReaderLWO::ChunkTFLG(int)
{
	curtexture->flags = ReadBigEndian<uint16>(stream);
}


void ReaderLWO::ChunkTSIZ(int)
{
	curtexture->size = ReadVector3();
}


void ReaderLWO::ChunkTCTR(int)
{
	curtexture->center = ReadVector3();
}


void ReaderLWO::ChunkTFAL(int)
{
	curtexture->falloff = ReadVector3();
}


void ReaderLWO::ChunkTVEL(int)
{
	curtexture->velocity = ReadVector3();
}


void ReaderLWO::ChunkTWRP(int)
{
	ReadBigEndian<uint32>(stream); // wrap
}


void ReaderLWO::ChunkTFP0(int)
{
	curtexture->tile.x = ReadBigEndian<float>(stream);
}


void ReaderLWO::ChunkTFP1(int)
{
	curtexture->tile.y = ReadBigEndian<float>(stream);
}


void ReaderLWO::ChunkLUMI(int)
{
	cursurface->luminosity = ReadBigEndian<uint16>(stream) / 256.0f;
}


void ReaderLWO::ChunkDIFF(int)
{
	cursurface->diffuse = ReadBigEndian<uint16>(stream) / 256.0f;
}


void ReaderLWO::ChunkSPEC(int)
{
	cursurface->specular = ReadBigEndian<uint16>(stream) / 256.0f;
}


void ReaderLWO::ChunkREFL(int)
{
	cursurface->reflection = ReadBigEndian<uint16>(stream) / 256.0f;
}


void ReaderLWO::ChunkTRAN(int)
{
	cursurface->transparency = ReadBigEndian<uint16>(stream) / 256.0f;
}


vec3f ReaderLWO::ReadVector3()
{
	vec3f v;
	v.x = ReadBigEndian<float>(stream);
	v.y = ReadBigEndian<float>(stream);
	v.z = ReadBigEndian<float>(stream);
	return v;
}


char* ReaderLWO::ReadAscii()
{
	static char buffer[256];

	char* ptr = buffer;
	for ( ;; )
	{
		char c;
		stream.Read(&c,1);
		*ptr++ = c;

		if ( !c ) break;
	}

	int count = static_cast<int>(strlen(buffer)) + 1;
	if ( count % 2 )
		stream.Seek(1,Stream::CURRENT);

	return buffer;
}


int ReaderLWO::ReadMapStyle()
{
	static const char* type[] = 
	{
		"Planar Image Map",
		"Cylindrical Image Map",
		"Spherical Image Map",
		"Cubic Image Map"
	};
	static const int numtype = sizeof(type) / sizeof(char*);

	char* name = ReadAscii();
	for ( int i=0; i<numtype; ++i )
	{
		if ( !strcmp(name,type[i]) )
			return i;
	}
	return 0;
}


// =================================================
// setup
// =================================================

void ReaderLWO::SetupTexture(TextureLWO& texture)
{
	texture.flags    = TextureLWO::X_AXIS;
	texture.style    = 0;
	texture.center   = point3f(0,0,0);
	texture.velocity = vec3f(0,0,0);
	texture.falloff  = vec3f(0,0,0);
	texture.size     = vec3f(1,1,1);
	texture.tile     = vec3f(1,1,1);
}


void ReaderLWO::SetupSurface(SurfaceLWO& surface, const char* name)
{
	surface.flags        = 0;
	surface.name         = name;
	surface.color        = Color32(0xffffffff);
	surface.luminosity   = 0;
	surface.diffuse      = 0;
	surface.specular     = 0;
	surface.reflection   = 0;
	surface.transparency = 0;

	SetupTexture(surface.ctex);
	SetupTexture(surface.dtex);
	SetupTexture(surface.stex);
	SetupTexture(surface.rtex);
	SetupTexture(surface.ttex);
	SetupTexture(surface.btex);
}


// =================================================
// texture coordinate generation
// =================================================

static const float PI     = prmath::pi;
static const float HALFPI = PI * 0.5f;
static const float TWOPI  = PI * 2.0f;


static void xyztoh(float x, float y, float z, float *h)
{
	if ( x == 0.0f && z == 0.0f )
	{
	   *h = 0.0f;
	}
	else
	{
		if ( z == 0.0f )     *h = x < 0.0f ? HALFPI : -HALFPI;
		else if ( z < 0.0f ) *h = -static_cast<float>(atan(x / z) + PI);
		else                 *h = -static_cast<float>(atan(x / z));
	}
}


static void xyztohp(float x, float y, float z, float* h, float* p)
{
	if ( x == 0.0f && z == 0.0f )
	{
		*h = 0.0f;
		if ( y != 0.0f ) *p = y < 0.0f ? -HALFPI : HALFPI;
		else             *p = 0.0f;
	}
	else
	{
		if ( z == 0.0f )     *h = x < 0.0f ? HALFPI : -HALFPI;
		else if ( z < 0.0f ) *h = -static_cast<float>(atan(x / z) + PI);
		else                 *h = -static_cast<float>(atan(x / z));

		x = static_cast<float>(sqrt(x*x + z*z));
		if ( x == 0.0f ) *p = y < 0.0f ? -HALFPI : HALFPI;
		else             *p = static_cast<float>(atan(y / x));
	}
}


static inline float fract(float v)
{
	return v - static_cast<float>(floor(v));
}


point2f TextureLWO::GetTexCoord(const point3f& point, float time) const
{
	point3f p = point - center + (velocity-falloff*time) * time;

	switch ( style )
	{
		case PLANAR:
		{
			float s = (flags & X_AXIS) ? p.z / size.z + 0.5f :  p.x / size.x + 0.5f;
			float t = (flags & Y_AXIS) ? -p.z / size.z + 0.5f : -p.y / size.y + 0.5f;
			return point2f(s * tile.x,t * tile.y);
		}
		case CYLINDRICAL:
		{
			float lon, t;
			if ( flags & X_AXIS )
			{
				xyztoh(p.z,p.x,-p.y,&lon);
				t = -p.x / size.x + 0.5f;
			}
			else if ( flags & Y_AXIS )
			{
				xyztoh(-p.x,p.y,p.z,&lon);
				t = -p.y / size.y + 0.5f;
			}
			else
			{
				xyztoh(-p.x,p.z,-p.y,&lon);
				t = -p.z / size.z + 0.5f;
			}

			lon = 1.0f - lon / TWOPI;
			if ( tile.x != 1.0f ) lon = fract(lon) * tile.x;
			float s = fract(lon) / 4.0f;
			t = fract(t);
			return point2f(s,t);
		}
		case SPHERICAL:
		{
			float lon, lat;

			if ( flags & X_AXIS )		xyztohp( p.z, p.x,-p.y,&lon,&lat);
			else if ( flags & Y_AXIS )	xyztohp(-p.x, p.y, p.z,&lon,&lat);
			else						xyztohp(-p.x, p.z,-p.y,&lon,&lat);

			lon = 1.0f - lon / TWOPI;
			lat = 0.5f - lat / PI;
			float s = fract(fract(lon) * tile.x) / 4.0f;
			float t = fract(fract(lat) * tile.y);
			return point2f(s,t);
		};
		case CUBIC:
		default:
			return point2f(0,0);
	}
}


// =================================================
// importer
// =================================================

ImportLWO::ImportLWO(Stream& stream)
{
	ReaderLWO reader(*this,stream);
}


ImportLWO::ImportLWO(const String& filename)
{
	FileStream stream(filename);
	ReaderLWO reader(*this,stream);
}


ImportLWO::~ImportLWO()
{
}
