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
		NewTek Lightwave object2 v6 (.lwo) reader

	revision history:
		Apr/12/2001 - Jukka Liimatta - Initial revision
		Nov/16/2001 - Mats Byggmastar - suggested that the IFF chunk padding should be supported
*/
#include <primport/lwo2.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// reader internal types
// =================================================

typedef Array<VertexLWO2>     VertexArray;
typedef Array<uint32>         IndexArray;
typedef Array<struct MeshLW6> MeshArray;
typedef Array<struct ClipLW6> ClipArray;


struct ClipLW6
{
	String	filename;
	uint32	index;
};

struct PolyLW6
{
	uint16	count;
	uint32	index;
	uint16	surface;
	uint16	smoothgroup;
};

struct MeshLW6
{
	// layer
	String		name;
	uint16		number;
	uint16		parent;
	point3f		pivot;
	bool		hidden;

	// mesh
	VertexArray		vertices;
	IndexArray		indices;
	Array<PolyLW6>	polys;
};


// =================================================
// reader declaration
// =================================================

class ReaderLWO2
{
	public:

	ReaderLWO2(ImportLWO2& import, Stream& stream);
	~ReaderLWO2();

	private:

	int				depth;
	ImportLWO2&		import;
	Stream&			stream;
	MeshArray		meshes;
	ClipArray		clips;
	int				texchannel;

	vec3f			ReadVector3();
	char*			ReadAscii(int& length);
	uint32			ReadIndex(int& length);

	int				ReadChunk();
	int				ReadSubChunk();
	void			ChunkUnknown(int length);

	void			ChunkLAYR(int length);
	void			ChunkPNTS(int length);
	void			ChunkVMAP(int length);
	void			ChunkPOLS(int length);
	void			ChunkTAGS(int length);
	void			ChunkPTAG(int length);
	void			ChunkVMAD(int length);
	void			ChunkENVL(int length);
	void			ChunkCLIP(int length);
	void			ChunkSURF(int length);

	void			ChunkSTIL(int length);
	void			ChunkCOLR(int length);
	void			ChunkDIFF(int length);
	void			ChunkLUMI(int length);
	void			ChunkSPEC(int length);
	void			ChunkREFL(int length);
	void			ChunkTRAN(int length);
	void			ChunkTRNL(int length);
	void			ChunkGLOS(int length);
	void			ChunkSHRP(int length);
	void			ChunkBUMP(int length);
	void			ChunkSIDE(int length);
	void			ChunkBLOK(int length);
	void			ChunkIMAP(int length);
	void			ChunkTMAP(int length);
	void			ChunkIMAG(int length);
	void			ChunkCHAN(int length);

	void			InitSurface(SurfaceLWO2& surface);
	SurfaceLWO2*	FindSurface(const String& name);
	SurfaceLWO2&	GetCurrentSurface();
	ClipLW6&		GetCurrentClip();
	ClipLW6*		FindClip(uint32 index);
	MeshLW6&		GetCurrentMesh();

	void			GenerateVertexNormals(MeshLW6& mesh);
	void			ProcessGeometry();
};


// =================================================
// reader implementation
// =================================================

ReaderLWO2::ReaderLWO2(ImportLWO2& ip, Stream& str)
: depth(0),import(ip),stream(str)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read iff85 header
	uint32 id = ReadBigEndian<uint32>(stream);
	if ( id != PRCORE_BIGCODE32('F','O','R','M') )
		PRCORE_EXCEPTION("Not a valif iff85 stream.");

	// chunkdata size
	int size = ReadBigEndian<uint32>(stream) - 4;

	// read lwo2 magic
	uint32 magic = ReadBigEndian<uint32>(stream);
	if ( magic != PRCORE_CODE32('L','W','O','2') )
		PRCORE_EXCEPTION("Not a valid lwo2 stream.");

	// read chunks
	while ( size > 0 )
	{
		size -= ReadChunk();
	}

	// process
	ProcessGeometry();
}


ReaderLWO2::~ReaderLWO2()
{
}


int ReaderLWO2::ReadChunk()
{
	// chunk header
	uint32 id     = ReadBigEndian<uint32>(stream);
	uint32 length = ReadBigEndian<uint32>(stream);

	++depth;

	switch ( id )
	{
		case PRCORE_BIGCODE32('L','A','Y','R'): ChunkLAYR(length); break;
		case PRCORE_BIGCODE32('P','N','T','S'): ChunkPNTS(length); break;
		case PRCORE_BIGCODE32('V','M','A','P'): ChunkVMAP(length); break;
		case PRCORE_BIGCODE32('P','O','L','S'): ChunkPOLS(length); break;
		case PRCORE_BIGCODE32('T','A','G','S'): ChunkTAGS(length); break;
		case PRCORE_BIGCODE32('P','T','A','G'): ChunkPTAG(length); break;
		case PRCORE_BIGCODE32('V','M','A','D'): ChunkVMAD(length); break;
		case PRCORE_BIGCODE32('E','N','V','L'): ChunkENVL(length); break;
		case PRCORE_BIGCODE32('C','L','I','P'): ChunkCLIP(length); break;
		case PRCORE_BIGCODE32('S','U','R','F'): ChunkSURF(length); break;
		default: ChunkUnknown(length); break;
	};

	--depth;
	
	// chunk alignment
	if ( length & 1 )
	{
		stream.Seek(1,Stream::CURRENT);
		++length;
	}

	return length + 8;
}


int ReaderLWO2::ReadSubChunk()
{
	// chunk header
	uint32 id     = ReadBigEndian<uint32>(stream);
	uint16 length = ReadBigEndian<uint16>(stream);

	++depth;

	switch ( id )
	{
		case PRCORE_BIGCODE32('S','T','I','L'): ChunkSTIL(length); break;
		case PRCORE_BIGCODE32('C','O','L','R'): ChunkCOLR(length); break;
		case PRCORE_BIGCODE32('D','I','F','F'): ChunkDIFF(length); break;
		case PRCORE_BIGCODE32('L','U','M','I'): ChunkLUMI(length); break;
		case PRCORE_BIGCODE32('S','P','E','C'): ChunkSPEC(length); break;
		case PRCORE_BIGCODE32('R','E','F','L'): ChunkREFL(length); break;
		case PRCORE_BIGCODE32('T','R','A','N'): ChunkTRAN(length); break;
		case PRCORE_BIGCODE32('T','R','N','L'): ChunkTRNL(length); break;
		case PRCORE_BIGCODE32('G','L','O','S'): ChunkGLOS(length); break;
		case PRCORE_BIGCODE32('S','H','R','P'): ChunkSHRP(length); break;
		case PRCORE_BIGCODE32('B','U','M','P'): ChunkBUMP(length); break;
		case PRCORE_BIGCODE32('S','I','D','E'): ChunkSIDE(length); break;
		case PRCORE_BIGCODE32('B','L','O','K'): ChunkBLOK(length); break;
		case PRCORE_BIGCODE32('I','M','A','P'): ChunkIMAP(length); break;
		case PRCORE_BIGCODE32('T','M','A','P'): ChunkTMAP(length); break;
		case PRCORE_BIGCODE32('I','M','A','G'): ChunkIMAG(length); break;
		case PRCORE_BIGCODE32('C','H','A','N'): ChunkCHAN(length); break;
		default: ChunkUnknown(length); break;
	};

	--depth;

	// chunk alignment
	if ( length & 1 )
	{
		stream.Seek(1,Stream::CURRENT);
		++length;
	}

	return length + 6;
}


void ReaderLWO2::ChunkUnknown(int length)
{
	stream.Seek(length,Stream::CURRENT);
}


// =================================================
// chunks
// =================================================

void ReaderLWO2::ChunkLAYR(int length)
{
	MeshLW6& mesh = meshes.PushBack();

	mesh.number  = ReadBigEndian<uint16>(stream);
	uint16 flags = ReadBigEndian<uint16>(stream);
	mesh.hidden  = (flags & 1) != 0;
	mesh.pivot   = ReadVector3();
	mesh.name    = ReadAscii(length);
	mesh.parent  = 0xffff;

	if ( length > 16 )
	{
		mesh.parent = ReadBigEndian<uint16>(stream);
	}
}


void ReaderLWO2::ChunkPNTS(int length)
{
	int count = length / sizeof(point3f);

	MeshLW6& mesh = GetCurrentMesh();
	mesh.vertices.SetSize( count );

	for ( int i=0; i<count; ++i )
	{
		VertexLWO2& vertex = mesh.vertices[i];

		vertex.point   = ReadVector3();
		vertex.normal  = vec3f(0,0,0);
		vertex.texture = vec2f(0,0);
	}
}


void ReaderLWO2::ChunkVMAP(int length)
{
	uint32 type      = ReadBigEndian<uint32>(stream);
	uint16 dimension = ReadBigEndian<uint16>(stream);
	String name      = ReadAscii(length);
	length -= 6;

	switch ( type )
	{
		case PRCORE_BIGCODE32('T','X','U','V'): 
			if ( dimension != 2 )
			{
				ChunkUnknown(length);
				return;
			}
			break;

		case PRCORE_BIGCODE32('P','I','C','K'): 
		case PRCORE_BIGCODE32('W','G','H','T'): 
		default:
			ChunkUnknown(length);
			return;
	}

	MeshLW6& mesh = GetCurrentMesh();

	while ( length > 0 )
	{
		uint32 index = ReadIndex(length);
		VertexLWO2& vertex = mesh.vertices[index];

		// read vertex data
		for ( int i=0; i<dimension; ++i )
		{
			vertex.texture[i] = ReadBigEndian<float>(stream);
		}
		length -= dimension * sizeof(float);
	}
}


void ReaderLWO2::ChunkPOLS(int length)
{
	uint32 type = ReadBigEndian<uint32>(stream);
	length -= sizeof(uint32);

	switch ( type )
	{
		case PRCORE_BIGCODE32('F','A','C','E'): 
			break;

		case PRCORE_BIGCODE32('C','U','R','V'):
		case PRCORE_BIGCODE32('P','T','C','H'):
		case PRCORE_BIGCODE32('M','B','A','L'):
		case PRCORE_BIGCODE32('B','O','N','E'):
		default:
			ChunkUnknown(length);
			return;
	};

	MeshLW6& mesh = GetCurrentMesh();
	mesh.polys.SetSize(0);
	int cindex = 0;

	while ( length > 0 )
	{
		uint16 count = ReadBigEndian<uint16>(stream) & 0x03ff;
		length -= sizeof(uint16);

		PolyLW6& poly = mesh.polys.PushBack();

		poly.count       = count;
		poly.index       = cindex;
		poly.surface     = 0;
		poly.smoothgroup = 0;

		for ( int i=0; i<static_cast<int>(count); ++i )
		{
			uint32 index = ReadIndex(length);
			mesh.indices.PushBack(index);
		}
		cindex += count;
	}
}


void ReaderLWO2::ChunkTAGS(int length)
{
	while ( length > 0 )
	{
		String name = ReadAscii(length);
		//TODO: store name(tag)
		// current polygon lists's "tag" 
		// ( used to find which SURF is associated with the PTAG chunk below )
	}
}


void ReaderLWO2::ChunkPTAG(int length)
{
	uint32 type = ReadBigEndian<uint32>(stream);
	length -= sizeof(uint32);

	MeshLW6& mesh = GetCurrentMesh();

	while ( length > 0 )
	{
		uint32 index = ReadIndex(length);
		uint16 tag = ReadBigEndian<uint16>(stream);
		length -= sizeof(uint16);

		PolyLW6& poly = mesh.polys[index];

		switch ( type )
		{
			case PRCORE_BIGCODE32('S','U','R','F'): 
				poly.surface = tag;
				break;

			case PRCORE_BIGCODE32('P','A','R','T'):
				//TODO
				break;

			case PRCORE_BIGCODE32('S','M','G','P'):
				poly.smoothgroup = tag;
				break;
		};
	}
}


void ReaderLWO2::ChunkVMAD(int length)
{
	uint32 type      = ReadBigEndian<uint32>(stream);
	uint16 dimension = ReadBigEndian<uint16>(stream);
	String name      = ReadAscii(length);
	length -= 6;

	switch ( type )
	{
		case PRCORE_BIGCODE32('T','X','U','V'): 
			if ( dimension != 2 )
			{
				ChunkUnknown(length);
				return;
			}
			break;

		case PRCORE_BIGCODE32('P','I','C','K'): 
		case PRCORE_BIGCODE32('W','G','H','T'): 
		default:
			ChunkUnknown(length);
			return;
	}

	MeshLW6& mesh = GetCurrentMesh();

	while ( length > 0 )
	{
		uint32 v = ReadIndex(length);
		uint32 p = ReadIndex(length);

		PolyLW6& poly = mesh.polys[p];

		// find vertex with correct index
		uint32* pindex = &mesh.indices[poly.index];
		for ( int c=0; c<poly.count; ++c )
		{
			if ( *pindex == v )
				break;

			++pindex;
		}

		// new vertex
		VertexLWO2& vertex = mesh.vertices.PushBack();
		vertex = mesh.vertices[v];
		*pindex = mesh.vertices.GetSize() - 1;

		// read vertex data
		for ( int i=0; i<dimension; ++i )
		{
			vertex.texture[i] =	ReadBigEndian<float>(stream);
		}
		length -= dimension * sizeof(float);
	}
}


void ReaderLWO2::ChunkENVL(int length)
{
	//TODO: if anyone has urgent need for envelopes, this is the place to begin ;-)
	ChunkUnknown(length);
}


void ReaderLWO2::ChunkCLIP(int length)
{
	uint32 index = ReadBigEndian<uint32>(stream);
	length -= sizeof(uint32);

	ClipLW6& clip = clips.PushBack();
	clip.index = index;

	// subchunks
	while ( length > 0 )
	{
		length -= ReadSubChunk();
	}
}


void ReaderLWO2::ChunkSURF(int length)
{
	String name       = ReadAscii(length);
	String sourcename = ReadAscii(length);

	SurfaceLWO2* source  = FindSurface(sourcename);
	SurfaceLWO2& surface = import.surfaces.PushBack();
	InitSurface(surface);

	if ( source )
		surface = *source;

	surface.name = name;

	// subchunks
	while ( length > 0 )
	{
		length -= ReadSubChunk();
	}
}


// =================================================
// subchunks
// =================================================

void ReaderLWO2::ChunkSTIL(int length)
{
	ClipLW6& clip = GetCurrentClip();

	String filename = ReadAscii(length);
	clip.filename   = GetFilenameNOPATH(filename);
}


void ReaderLWO2::ChunkCOLR(int length)
{
	float r = ReadBigEndian<float>(stream);
	float g = ReadBigEndian<float>(stream);
	float b = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope

	SurfaceLWO2& surface = GetCurrentSurface();

	surface.color.b = static_cast<uint8>(b * 255);
	surface.color.g = static_cast<uint8>(g * 255);
	surface.color.r = static_cast<uint8>(r * 255);
	surface.color.a = 0xff;
}


void ReaderLWO2::ChunkDIFF(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.diffuse = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkLUMI(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.luminosity = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkSPEC(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.specular = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkREFL(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.reflection = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkTRAN(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.transparency = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkTRNL(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.translucency = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkGLOS(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.glossiness = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkSHRP(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.sharpness = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkBUMP(int length)
{
	SurfaceLWO2& surface = GetCurrentSurface();
	surface.bump = ReadBigEndian<float>(stream);
	ReadIndex(length); // envelope
}


void ReaderLWO2::ChunkSIDE(int)
{
	uint16 side = ReadBigEndian<uint16>(stream);
	if ( side > 1 )
	{
		SurfaceLWO2& surface = GetCurrentSurface();
		surface.twosided = true;
	}
}


void ReaderLWO2::ChunkBLOK(int length)
{
	// subchunks
	while ( length > 0 )
	{
		length -= ReadSubChunk();
	}
}


void ReaderLWO2::ChunkIMAP(int length)
{
	String ordinal = ReadAscii(length);

	// subchunks
	while ( length > 0 )
	{
		length -= ReadSubChunk();
	}
}


void ReaderLWO2::ChunkTMAP(int length)
{
	// subchunks
	while ( length > 0 )
	{
		length -= ReadSubChunk();
	}
}


void ReaderLWO2::ChunkIMAG(int length)
{
	uint32 index = ReadIndex(length);
	ClipLW6* clip = FindClip(index);
	if ( clip )
	{
		SurfaceLWO2& surface = GetCurrentSurface();
		TextureLWO2& texture = surface.texture[texchannel];

		if ( clip->filename.GetLength() > 0 )
		{
			texture.filename = clip->filename;
			texture.enable = true;
		}
	}
}


void ReaderLWO2::ChunkCHAN(int)
{
	uint32 chan = ReadBigEndian<uint32>(stream);
	switch ( chan )
	{
		case PRCORE_BIGCODE32('C','O','L','R'): texchannel = LWO2_TEXTURE_COLOR; break;
		case PRCORE_BIGCODE32('D','I','F','F'): texchannel = LWO2_TEXTURE_DIFFUSE; break;
		case PRCORE_BIGCODE32('L','U','M','I'): texchannel = LWO2_TEXTURE_LUMINOSITY; break;
		case PRCORE_BIGCODE32('S','P','E','C'): texchannel = LWO2_TEXTURE_SPECULAR; break;
		case PRCORE_BIGCODE32('G','L','O','S'): texchannel = LWO2_TEXTURE_REFLECTION; break;
		case PRCORE_BIGCODE32('R','E','F','L'): texchannel = LWO2_TEXTURE_TRANSPARENCY; break;
		case PRCORE_BIGCODE32('T','R','A','N'): texchannel = LWO2_TEXTURE_TRANSLUCENCY; break;
		case PRCORE_BIGCODE32('R','I','N','D'): texchannel = LWO2_TEXTURE_GLOSSINESS; break;
		case PRCORE_BIGCODE32('T','R','N','L'): texchannel = LWO2_TEXTURE_SHARPNESS; break;
		case PRCORE_BIGCODE32('B','U','M','P'): texchannel = LWO2_TEXTURE_BUMP; break;
	}
}


// =================================================
// reader methods
// =================================================

inline vec3f ReaderLWO2::ReadVector3()
{
	vec3f v;
	v.x = ReadBigEndian<float>(stream);
	v.y = ReadBigEndian<float>(stream);
	v.z = ReadBigEndian<float>(stream);
	return v;
}


inline char* ReaderLWO2::ReadAscii(int& length)
{
	static char buffer[256];

	char* ptr = buffer;
	for ( ;; )
	{
		char c;
		stream.Read(&c,1);
		*ptr++ = c;

		if ( !c )
			break;
	}

	int count = static_cast<int>(ptr - buffer);
	int modulo = count % 2;
	if ( modulo )
		stream.Seek(1,Stream::CURRENT);

	length -= (count + modulo);
	return buffer;
}


inline uint32 ReaderLWO2::ReadIndex(int& length)
{
	uint32 v0 = ReadBigEndian<uint16>(stream);
	if ( v0 < 0xff00 )
	{
		length -= sizeof(uint16);
		return v0;
	}

	length -= sizeof(uint32);
	return (((v0 & 0x00ff) << 16) | ReadBigEndian<uint16>(stream)) - 0xff00;
}


void ReaderLWO2::InitSurface(SurfaceLWO2& surface)
{
	surface.color        = 0xffbbbbbb;
	surface.diffuse      = 0;
	surface.luminosity   = 0;
	surface.specular     = 0;
	surface.reflection   = 0;
	surface.transparency = 0;
	surface.translucency = 0;
	surface.glossiness   = 0.2f;
	surface.sharpness    = 0.5f;
	surface.bump         = 1;

	surface.twosided = false;

	for ( int i=0; i<LWO2_TEXTURE_COUNT; ++i )
	{
		surface.texture[i].enable = false;
	}
}


SurfaceLWO2* ReaderLWO2::FindSurface(const String& name)
{
	for ( int i=0; i<import.surfaces.GetSize(); ++i )
	{
		if ( import.surfaces[i].name == name )
			return &import.surfaces[i];
	}
	return NULL;
}


SurfaceLWO2& ReaderLWO2::GetCurrentSurface()
{
	int count = import.surfaces.GetSize();
	if ( count )
		return import.surfaces[count-1];

	// create arbitrary surface
	SurfaceLWO2& surface = import.surfaces.PushBack();
	InitSurface(surface);

	return surface;
}


ClipLW6& ReaderLWO2::GetCurrentClip()
{
	int count = clips.GetSize();
	return clips[count-1];
}


ClipLW6* ReaderLWO2::FindClip(uint32 index)
{
	for ( int i=0; i<clips.GetSize(); ++i )
	{
		if ( clips[i].index == index )
			return &clips[i];
	}
	return NULL;
}


MeshLW6& ReaderLWO2::GetCurrentMesh()
{
	int count = meshes.GetSize();
	if ( count )
		return meshes[count-1];

	// create arbitrary mesh
	MeshLW6& mesh = meshes.PushBack();

	mesh.name   = "arbitrary mesh";
	mesh.number = 0xfffe;
	mesh.parent = 0xffff;
	mesh.pivot  = point3f(0,0,0);
	mesh.hidden = false;

	return mesh;
}


void ReaderLWO2::GenerateVertexNormals(MeshLW6& mesh)
{
	for ( int i=0; i<mesh.polys.GetSize(); ++i )
	{
		PolyLW6& poly = mesh.polys[i];
		uint32* index = &mesh.indices[ poly.index ];

		point3f& a = mesh.vertices[index[0]].point;
		point3f& b = mesh.vertices[index[1]].point;
		point3f& c = mesh.vertices[index[2]].point;
		vec3f normal = prmath::CrossProduct(b-a,c-a);

		for ( int v=0; v<poly.count; ++v )
		{
			VertexLWO2& vertex = mesh.vertices[index[v]];
			vertex.normal += normal;
		}
	}

	for ( int v=0; v<mesh.vertices.GetSize(); ++v )
	{
		VertexLWO2& vertex = mesh.vertices[v];
		vertex.normal.Normalize();
	}
}


void ReaderLWO2::ProcessGeometry()
{
	for ( int i=0; i<meshes.GetSize(); ++i )
	{
		// source
		MeshLW6& mesh = meshes[i];

		// destination
		MeshLWO2& trimesh = import.meshes.PushBack();

		// generate vertex normals
		GenerateVertexNormals(mesh);

		// convert into trimesh
		int j = 0;

		for ( j=0; j<mesh.polys.GetSize(); ++j )
		{
			PolyLW6& poly = mesh.polys[j];
			uint32* index = &mesh.indices[poly.index];

			// find indexbuffer
			IndexBufferLWO2* cbuffer = NULL;
			for ( int c=0; c<trimesh.indexbuffers.GetSize(); ++c )
			{
				if ( trimesh.indexbuffers[c].surface == poly.surface )
				{
					cbuffer = &trimesh.indexbuffers[c];
					break;
				}
			}

			// create new indexbuffer
			if ( !cbuffer )
			{
				cbuffer = &trimesh.indexbuffers.PushBack();
				cbuffer->surface = poly.surface;
			}

			// write polygon vertex indices as triangle indices
			// into the vertexbuffer
			for ( int v=0; v<(poly.count-2); ++v )
			{
				cbuffer->indexbuffer.PushBack(index[0]);
				cbuffer->indexbuffer.PushBack(index[v+1]);
				cbuffer->indexbuffer.PushBack(index[v+2]);
			}
		}

		// copy vertexbuffer
		for ( j=0; j<mesh.vertices.GetSize(); ++j )
		{
			VertexLWO2& vertex = trimesh.vertexbuffer.PushBack();
			VertexLWO2& source = mesh.vertices[j];

			vertex.point     = source.point;
			vertex.normal    = source.normal;
			vertex.texture.x = source.texture.x;
			vertex.texture.y = -source.texture.y;
		}
	}
}


// =================================================
// importer
// =================================================

ImportLWO2::ImportLWO2(Stream& stream)
{
	ReaderLWO2 reader(*this,stream);
}


ImportLWO2::ImportLWO2(const String& filename)
{
	FileStream stream(filename);
	ReaderLWO2 reader(*this,stream);
}


ImportLWO2::~ImportLWO2()
{
}
