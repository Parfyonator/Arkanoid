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
		ID Software Quake III Arena: bsp reader (.bsp)

	revision history:
		Aug/10/2000 - Rune Vendler - initial revision
		Feb/10/2001 - Jukka Liimatta - renaissance build
		Mar/19/2001 - Arnaud Linz - fixed a bug in lightmap size
*/
#include <primport/q3bsp.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// reader declaration
// =================================================

class ReaderQ3BSP
{
	public:

	ReaderQ3BSP(ImportQ3BSP& import, Stream& stream);
	~ReaderQ3BSP();

	private:

	struct DirectoryEntry
	{
		int offset;
		int length;
	};

	struct Header
	{
		// lump index
		#define Q3BSP_LUMP_ENTITIES			(0)
		#define Q3BSP_LUMP_TEXTURES			(1)
		#define	Q3BSP_LUMP_PLANES			(2)
		#define	Q3BSP_LUMP_NODES			(3)
		#define	Q3BSP_LUMP_LEAFS			(4)
		#define	Q3BSP_LUMP_LEAFFACES		(5)
		#define	Q3BSP_LUMP_LEAFBRUSHES		(6)
		#define	Q3BSP_LUMP_MODELS			(7)
		#define	Q3BSP_LUMP_BRUSHES			(8)
		#define	Q3BSP_LUMP_BRUSHSIDES		(9)
		#define	Q3BSP_LUMP_VERTICES			(10)
		#define	Q3BSP_LUMP_MESHVERTICES		(11)
		#define	Q3BSP_LUMP_EFFECTS			(12)
		#define	Q3BSP_LUMP_FACES			(13)
		#define	Q3BSP_LUMP_LIGHTMAPS		(14)
		#define	Q3BSP_LUMP_LIGHTVOLUMES		(15)
		#define	Q3BSP_LUMP_VISDATA			(16)
		#define	Q3BSP_NUM_LUMPS				(17)

		// we need the original struct-sizes to figure out the number of each type
		#define	Q3BSP_TEXTURE_SIZE			(72)
		#define Q3BSP_PLANE_SIZE			(16)
		#define Q3BSP_NODE_SIZE				(36)
		#define Q3BSP_LEAF_SIZE				(48)
		#define Q3BSP_LEAFFACE_SIZE			(4)
		#define Q3BSP_LEAFBRUSH_SIZE		(4)
		#define Q3BSP_MODEL_SIZE			(40)
		#define Q3BSP_BRUSH_SIZE			(12)
		#define Q3BSP_BRUSHSIDE_SIZE		(8)
		#define Q3BSP_VERTEX_SIZE			(44)
		#define Q3BSP_MESHVERTEX_SIZE		(4)
		#define Q3BSP_EFFECT_SIZE			(72)
		#define Q3BSP_FACE_SIZE				(104)
		#define Q3BSP_LIGHTMAP_SIZE			(49152)
		#define Q3BSP_LIGHTVOLUME_SIZE		(8)

		// data
		unsigned int	magicnumber;
		int				version;
		DirectoryEntry	directory[Q3BSP_NUM_LUMPS];
	};
		
	Header			header;
	ImportQ3BSP&	import;
	Stream&			stream;

	Color32		ReadColor3();
	Color32		ReadColor4();
	vec2f		ReadVec2();
	vec3f		ReadVec3();
	vec3f		ReadIntVec3();
	box3f		ReadBox();

	void		ReadHeader();
	void		ReadEntities();
	void		ReadTextures();
	void		ReadPlanes();
	void		ReadNodes();
	void		ReadLeafs();
	void		ReadLeafFaces();
	void		ReadLeafBrushes();
	void		ReadModels();
	void		ReadBrushes();
	void		ReadBrushSides();
	void		ReadVertices();
	void		ReadMeshVertices();
	void		ReadEffects();
	void		ReadFaces();
	void		ReadLightmaps();
	void		ReadLightVolumes();
	void		ReadVisData();
};


// =================================================
// reader implementation
// =================================================

ReaderQ3BSP::ReaderQ3BSP(ImportQ3BSP& i, Stream& s)
: import(i), stream(s)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// read header
	ReadHeader();
	
	// read lumps
	if ( header.directory[Q3BSP_LUMP_ENTITIES].length > 0 )		ReadEntities();
	if ( header.directory[Q3BSP_LUMP_TEXTURES].length > 0 )		ReadTextures();
	if ( header.directory[Q3BSP_LUMP_PLANES].length > 0 )		ReadPlanes();
	if ( header.directory[Q3BSP_LUMP_NODES].length > 0 )		ReadNodes();
	if ( header.directory[Q3BSP_LUMP_LEAFS].length > 0 )		ReadLeafs();
	if ( header.directory[Q3BSP_LUMP_LEAFFACES].length > 0 )	ReadLeafFaces();
	if ( header.directory[Q3BSP_LUMP_LEAFBRUSHES].length > 0 )	ReadLeafBrushes();
	if ( header.directory[Q3BSP_LUMP_MODELS].length > 0 )		ReadModels();
	if ( header.directory[Q3BSP_LUMP_BRUSHES].length > 0 )		ReadBrushes();
	if ( header.directory[Q3BSP_LUMP_BRUSHSIDES].length > 0 )	ReadBrushSides();
	if ( header.directory[Q3BSP_LUMP_VERTICES].length > 0 )		ReadVertices();
	if ( header.directory[Q3BSP_LUMP_MESHVERTICES].length > 0 )	ReadMeshVertices();
	if ( header.directory[Q3BSP_LUMP_EFFECTS].length > 0 )		ReadEffects();
	if ( header.directory[Q3BSP_LUMP_FACES].length > 0 )		ReadFaces();
	if ( header.directory[Q3BSP_LUMP_LIGHTMAPS].length > 0 )	ReadLightmaps();
	if ( header.directory[Q3BSP_LUMP_LIGHTVOLUMES].length > 0 )	ReadLightVolumes();
	if ( header.directory[Q3BSP_LUMP_VISDATA].length > 0 )		ReadVisData();
}


ReaderQ3BSP::~ReaderQ3BSP()
{
}


Color32 ReaderQ3BSP::ReadColor3()
{
	Color32 color;
	color.r = ReadLittleEndian<uint8>(stream);
	color.g = ReadLittleEndian<uint8>(stream);
	color.b = ReadLittleEndian<uint8>(stream);
	color.a = 0xff;
	return color;
}


Color32	ReaderQ3BSP::ReadColor4()
{
	Color32 color;
	color.r = ReadLittleEndian<uint8>(stream);
	color.g = ReadLittleEndian<uint8>(stream);
	color.b = ReadLittleEndian<uint8>(stream);
	color.a = ReadLittleEndian<uint8>(stream);
	return color;
}


vec2f ReaderQ3BSP::ReadVec2()
{
	float x = ReadLittleEndian<float>(stream);
	float y = ReadLittleEndian<float>(stream);
	return vec2f(x,y);
}


vec3f ReaderQ3BSP::ReadVec3()
{
	float x = ReadLittleEndian<float>(stream);
	float y = ReadLittleEndian<float>(stream);
	float z = ReadLittleEndian<float>(stream);
	return vec3f(-y,z,x);
}


vec3f ReaderQ3BSP::ReadIntVec3()
{
	float x = (float)ReadLittleEndian<int32>(stream);
	float y = (float)ReadLittleEndian<int32>(stream);
	float z = (float)ReadLittleEndian<int32>(stream);
	return vec3f(-y,z,x);
}


box3f ReaderQ3BSP::ReadBox()
{
	box3f box;
	box.vmin = ReadIntVec3();
	box.vmax = ReadIntVec3();

	for ( int i=0; i<3; ++i )
	{
		if ( box.vmin[i] > box.vmax[i] )
		{
			float v = box.vmin[i];
			box.vmin[i] = box.vmax[i];
			box.vmax[i] = v;
		}
	}

	return box;
}


void ReaderQ3BSP::ReadHeader()
{
	stream.Seek(0,Stream::START);
	
	// id
	header.magicnumber = ReadLittleEndian<uint32>(stream);
	if( header.magicnumber != PRCORE_CODE32('I','B','S','P') )
		PRCORE_EXCEPTION("Incorrect header id.");
		
	// version
	header.version = ReadLittleEndian<uint32>(stream);
	
	// directory
	for( int i=0; i<Q3BSP_NUM_LUMPS; ++i )
	{
		header.directory[i].offset = ReadLittleEndian<uint32>(stream);
		header.directory[i].length = ReadLittleEndian<uint32>(stream);
	}
}
	
	
void ReaderQ3BSP::ReadEntities()
{
	stream.Seek(header.directory[Q3BSP_LUMP_ENTITIES].offset,Stream::START);
	
	int size = header.directory[Q3BSP_LUMP_ENTITIES].length;
	char* buffer = new char[size];
	stream.Read(buffer,size);
	
	import.entitydesc = buffer;
	delete[] buffer;
}


void ReaderQ3BSP::ReadTextures()
{
	stream.Seek(header.directory[Q3BSP_LUMP_TEXTURES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_TEXTURES].length / Q3BSP_TEXTURE_SIZE;
	import.textures.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		char name[64];
		stream.Read(name,64);
		
		import.textures[i].name     = name;
		import.textures[i].flags    = ReadLittleEndian<uint32>(stream);
		import.textures[i].contents = ReadLittleEndian<uint32>(stream);
	}
}


void ReaderQ3BSP::ReadPlanes()
{
	stream.Seek(header.directory[Q3BSP_LUMP_PLANES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_PLANES].length / Q3BSP_PLANE_SIZE;
	import.planes.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.planes[i].normal = ReadVec3();
		import.planes[i].dist   = ReadLittleEndian<float>(stream);
	}
}


void ReaderQ3BSP::ReadNodes()
{
	stream.Seek(header.directory[Q3BSP_LUMP_NODES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_NODES].length / Q3BSP_NODE_SIZE;
	import.nodes.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.nodes[i].planeindex  = ReadLittleEndian<uint32>(stream);
		import.nodes[i].children[0] = ReadLittleEndian<uint32>(stream);
		import.nodes[i].children[1] = ReadLittleEndian<uint32>(stream);
		import.nodes[i].boundingbox = ReadBox();
	}
}


void ReaderQ3BSP::ReadLeafs()
{
	stream.Seek(header.directory[Q3BSP_LUMP_LEAFS].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_LEAFS].length / Q3BSP_LEAF_SIZE;
	import.leafs.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.leafs[i].cluster        = ReadLittleEndian<int32>(stream);
		import.leafs[i].area           = ReadLittleEndian<int32>(stream);
		import.leafs[i].boundingbox    = ReadBox();
		import.leafs[i].leaffaceindex  = ReadLittleEndian<int32>(stream);
		import.leafs[i].numleaffaces   = ReadLittleEndian<int32>(stream);
		import.leafs[i].leafbrushindex = ReadLittleEndian<int32>(stream);
		import.leafs[i].numleafbrushes = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadLeafFaces()
{
	stream.Seek(header.directory[Q3BSP_LUMP_LEAFFACES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_LEAFFACES].length / Q3BSP_LEAFFACE_SIZE;
	import.leaffaces.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.leaffaces[i].faceindex = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadLeafBrushes()
{
	stream.Seek(header.directory[Q3BSP_LUMP_LEAFBRUSHES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_LEAFBRUSHES].length / Q3BSP_LEAFBRUSH_SIZE;
	import.leafbrushes.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.leafbrushes[i].brushindex = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadModels()
{
	stream.Seek(header.directory[Q3BSP_LUMP_MODELS].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_MODELS].length / Q3BSP_MODEL_SIZE;
	import.models.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.models[i].boundingbox = ReadBox();
		import.models[i].faceindex   = ReadLittleEndian<int32>(stream);
		import.models[i].numfaces    = ReadLittleEndian<int32>(stream);
		import.models[i].brushindex  = ReadLittleEndian<int32>(stream);
		import.models[i].numbrushes  = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadBrushes()
{
	stream.Seek(header.directory[Q3BSP_LUMP_BRUSHES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_BRUSHES].length / Q3BSP_BRUSH_SIZE;
	import.brushes.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.brushes[i].brushsideindex = ReadLittleEndian<int32>(stream);
		import.brushes[i].numbrushsides  = ReadLittleEndian<int32>(stream);
		import.brushes[i].textureindex   = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadBrushSides()
{
	stream.Seek(header.directory[Q3BSP_LUMP_BRUSHSIDES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_BRUSHSIDES].length / Q3BSP_BRUSHSIDE_SIZE;
	import.brushsides.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.brushsides[i].planeindex   = ReadLittleEndian<int32>(stream);
		import.brushsides[i].textureindex = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadVertices()
{
	stream.Seek(header.directory[Q3BSP_LUMP_VERTICES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_VERTICES].length / Q3BSP_VERTEX_SIZE;
	import.vertices.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.vertices[i].position       = ReadVec3();
		import.vertices[i].texcoords      = ReadVec2();
		import.vertices[i].lightmapcoords = ReadVec2();
		import.vertices[i].normal         = ReadVec3();
		import.vertices[i].color          = ReadColor4();
	}
}


void ReaderQ3BSP::ReadMeshVertices()
{
	stream.Seek(header.directory[Q3BSP_LUMP_MESHVERTICES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_MESHVERTICES].length / Q3BSP_MESHVERTEX_SIZE;
	import.meshvertices.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.meshvertices[i].offset = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadEffects()
{
	stream.Seek(header.directory[Q3BSP_LUMP_EFFECTS].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_EFFECTS].length / Q3BSP_EFFECT_SIZE;
	import.effects.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		char name[64];
		stream.Read(name,64);

		import.effects[i].name       = name;
		import.effects[i].brushindex = ReadLittleEndian<int32>(stream);
		import.effects[i].unknown    = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadFaces()
{
	stream.Seek(header.directory[Q3BSP_LUMP_FACES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_FACES].length / Q3BSP_FACE_SIZE;
	import.faces.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.faces[i].textureindex       = ReadLittleEndian<int32>(stream);
		import.faces[i].effectindex        = ReadLittleEndian<int32>(stream);
		import.faces[i].type               = ReadLittleEndian<int32>(stream);
		import.faces[i].vertexindex        = ReadLittleEndian<int32>(stream);
		import.faces[i].numvertices        = ReadLittleEndian<int32>(stream);
		import.faces[i].meshvertexindex    = ReadLittleEndian<int32>(stream);
		import.faces[i].nummeshvertices    = ReadLittleEndian<int32>(stream);
		import.faces[i].lightmapindex      = ReadLittleEndian<int32>(stream);
		import.faces[i].lightmapcorner[0]  = ReadLittleEndian<int32>(stream);
		import.faces[i].lightmapcorner[1]  = ReadLittleEndian<int32>(stream);
		import.faces[i].lightmapsize[0]    = ReadLittleEndian<int32>(stream);
		import.faces[i].lightmapsize[1]    = ReadLittleEndian<int32>(stream);
		import.faces[i].lightmapworldspaceorigin = ReadVec3();
		import.faces[i].lightmapworldspaces      = ReadVec3();
		import.faces[i].lightmapworldspacet      = ReadVec3();
		import.faces[i].normal                   = ReadVec3();
		import.faces[i].patchsize[0]             = ReadLittleEndian<int32>(stream);
		import.faces[i].patchsize[1]             = ReadLittleEndian<int32>(stream);
	}
}


void ReaderQ3BSP::ReadLightmaps()
{
	stream.Seek(header.directory[Q3BSP_LUMP_LIGHTMAPS].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_LIGHTMAPS].length / Q3BSP_LIGHTMAP_SIZE;
	import.lightmaps.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		stream.Read((char*)import.lightmaps[i].map,128*128*3);
	}
}


void ReaderQ3BSP::ReadLightVolumes()
{
	stream.Seek(header.directory[Q3BSP_LUMP_LIGHTVOLUMES].offset,Stream::START);
	
	int	count = header.directory[Q3BSP_LUMP_LIGHTVOLUMES].length / Q3BSP_LIGHTVOLUME_SIZE;
	import.lightvolumes.SetSize(count);
	
	for( int i=0; i<count; ++i )
	{
		import.lightvolumes[i].ambientcolor     = ReadColor3();
		import.lightvolumes[i].directionalcolor = ReadColor3();
		import.lightvolumes[i].phi   = ReadLittleEndian<uint8>(stream);
		import.lightvolumes[i].theta = ReadLittleEndian<uint8>(stream);
	}
}


void ReaderQ3BSP::ReadVisData()
{
	stream.Seek(header.directory[Q3BSP_LUMP_VISDATA].offset,Stream::START);
	
	import.visdata.numvectors = ReadLittleEndian<int32>(stream);
	import.visdata.vectorsize = ReadLittleEndian<int32>(stream);
	
	int size = import.visdata.numvectors * import.visdata.vectorsize;
	import.visdata.data = new uint8[size];
	stream.Read(import.visdata.data,size);
}


// =================================================
// importer
// =================================================

ImportQ3BSP::ImportQ3BSP()
{
}


ImportQ3BSP::ImportQ3BSP(Stream& stream)
{
	ReaderQ3BSP reader(*this,stream);
}


ImportQ3BSP::ImportQ3BSP(const String& filename)
{
	FileStream stream(filename);
	ReaderQ3BSP reader(*this,stream);
}


ImportQ3BSP::~ImportQ3BSP()
{
	delete[] visdata.data;
}
