/*
	Copyright (C) 2002-2003 Franceso Montecuccoli.


	source:
		ID Software Quake II model reader

	revision history:
		Feb/06/2002 - Francesco Montecuccoli - initial revision
*/
#include <primport/q2mdl.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// Quake II constants
// =================================================

namespace
{

	float gva_normal[] = \
	{
		-0.525731f, 0.000000f, 0.850651f, 
		-0.442863f, 0.238856f, 0.864188f, 
		-0.295242f, 0.000000f, 0.955423f, 
		-0.309017f, 0.500000f, 0.809017f, 
		-0.162460f, 0.262866f, 0.951056f, 
		0.000000f, 0.000000f, 1.000000f, 
		0.000000f, 0.850651f, 0.525731f, 
		-0.147621f, 0.716567f, 0.681718f, 
		0.147621f, 0.716567f, 0.681718f, 
		0.000000f, 0.525731f, 0.850651f, 
		0.309017f, 0.500000f, 0.809017f, 
		0.525731f, 0.000000f, 0.850651f, 
		0.295242f, 0.000000f, 0.955423f, 
		0.442863f, 0.238856f, 0.864188f, 
		0.162460f, 0.262866f, 0.951056f, 
		-0.681718f, 0.147621f, 0.716567f, 
		-0.809017f, 0.309017f, 0.500000f, 
		-0.587785f, 0.425325f, 0.688191f, 
		-0.850651f, 0.525731f, 0.000000f, 
		-0.864188f, 0.442863f, 0.238856f, 
		-0.716567f, 0.681718f, 0.147621f, 
		-0.688191f, 0.587785f, 0.425325f, 
		-0.500000f, 0.809017f, 0.309017f, 
		-0.238856f, 0.864188f, 0.442863f, 
		-0.425325f, 0.688191f, 0.587785f, 
		-0.716567f, 0.681718f, -0.147621f, 
		-0.500000f, 0.809017f, -0.309017f, 
		-0.525731f, 0.850651f, 0.000000f, 
		0.000000f, 0.850651f, -0.525731f, 
		-0.238856f, 0.864188f, -0.442863f, 
		0.000000f, 0.955423f, -0.295242f, 
		-0.262866f, 0.951056f, -0.162460f, 
		0.000000f, 1.000000f, 0.000000f, 
		0.000000f, 0.955423f, 0.295242f, 
		-0.262866f, 0.951056f, 0.162460f, 
		0.238856f, 0.864188f, 0.442863f, 
		0.262866f, 0.951056f, 0.162460f, 
		0.500000f, 0.809017f, 0.309017f, 
		0.238856f, 0.864188f, -0.442863f, 
		0.262866f, 0.951056f, -0.162460f, 
		0.500000f, 0.809017f, -0.309017f, 
		0.850651f, 0.525731f, 0.000000f, 
		0.716567f, 0.681718f, 0.147621f, 
		0.716567f, 0.681718f, -0.147621f, 
		0.525731f, 0.850651f, 0.000000f, 
		0.425325f, 0.688191f, 0.587785f, 
		0.864188f, 0.442863f, 0.238856f, 
		0.688191f, 0.587785f, 0.425325f, 
		0.809017f, 0.309017f, 0.500000f, 
		0.681718f, 0.147621f, 0.716567f, 
		0.587785f, 0.425325f, 0.688191f, 
		0.955423f, 0.295242f, 0.000000f, 
		1.000000f, 0.000000f, 0.000000f, 
		0.951056f, 0.162460f, 0.262866f, 
		0.850651f, -0.525731f, 0.000000f, 
		0.955423f, -0.295242f, 0.000000f, 
		0.864188f, -0.442863f, 0.238856f, 
		0.951056f, -0.162460f, 0.262866f, 
		0.809017f, -0.309017f, 0.500000f, 
		0.681718f, -0.147621f, 0.716567f, 
		0.850651f, 0.000000f, 0.525731f, 
		0.864188f, 0.442863f, -0.238856f, 
		0.809017f, 0.309017f, -0.500000f, 
		0.951056f, 0.162460f, -0.262866f, 
		0.525731f, 0.000000f, -0.850651f, 
		0.681718f, 0.147621f, -0.716567f, 
		0.681718f, -0.147621f, -0.716567f, 
		0.850651f, 0.000000f, -0.525731f, 
		0.809017f, -0.309017f, -0.500000f, 
		0.864188f, -0.442863f, -0.238856f, 
		0.951056f, -0.162460f, -0.262866f, 
		0.147621f, 0.716567f, -0.681718f, 
		0.309017f, 0.500000f, -0.809017f, 
		0.425325f, 0.688191f, -0.587785f, 
		0.442863f, 0.238856f, -0.864188f, 
		0.587785f, 0.425325f, -0.688191f, 
		0.688191f, 0.587785f, -0.425325f, 
		-0.147621f, 0.716567f, -0.681718f, 
		-0.309017f, 0.500000f, -0.809017f, 
		0.000000f, 0.525731f, -0.850651f, 
		-0.525731f, 0.000000f, -0.850651f, 
		-0.442863f, 0.238856f, -0.864188f, 
		-0.295242f, 0.000000f, -0.955423f, 
		-0.162460f, 0.262866f, -0.951056f, 
		0.000000f, 0.000000f, -1.000000f, 
		0.295242f, 0.000000f, -0.955423f, 
		0.162460f, 0.262866f, -0.951056f, 
		-0.442863f, -0.238856f, -0.864188f, 
		-0.309017f, -0.500000f, -0.809017f, 
		-0.162460f, -0.262866f, -0.951056f, 
		0.000000f, -0.850651f, -0.525731f, 
		-0.147621f, -0.716567f, -0.681718f, 
		0.147621f, -0.716567f, -0.681718f, 
		0.000000f, -0.525731f, -0.850651f, 
		0.309017f, -0.500000f, -0.809017f, 
		0.442863f, -0.238856f, -0.864188f, 
		0.162460f, -0.262866f, -0.951056f, 
		0.238856f, -0.864188f, -0.442863f, 
		0.500000f, -0.809017f, -0.309017f, 
		0.425325f, -0.688191f, -0.587785f, 
		0.716567f, -0.681718f, -0.147621f, 
		0.688191f, -0.587785f, -0.425325f, 
		0.587785f, -0.425325f, -0.688191f, 
		0.000000f, -0.955423f, -0.295242f, 
		0.000000f, -1.000000f, 0.000000f, 
		0.262866f, -0.951056f, -0.162460f, 
		0.000000f, -0.850651f, 0.525731f, 
		0.000000f, -0.955423f, 0.295242f, 
		0.238856f, -0.864188f, 0.442863f, 
		0.262866f, -0.951056f, 0.162460f, 
		0.500000f, -0.809017f, 0.309017f, 
		0.716567f, -0.681718f, 0.147621f, 
		0.525731f, -0.850651f, 0.000000f, 
		-0.238856f, -0.864188f, -0.442863f, 
		-0.500000f, -0.809017f, -0.309017f, 
		-0.262866f, -0.951056f, -0.162460f, 
		-0.850651f, -0.525731f, 0.000000f, 
		-0.716567f, -0.681718f, -0.147621f, 
		-0.716567f, -0.681718f, 0.147621f, 
		-0.525731f, -0.850651f, 0.000000f, 
		-0.500000f, -0.809017f, 0.309017f, 
		-0.238856f, -0.864188f, 0.442863f, 
		-0.262866f, -0.951056f, 0.162460f, 
		-0.864188f, -0.442863f, 0.238856f, 
		-0.809017f, -0.309017f, 0.500000f, 
		-0.688191f, -0.587785f, 0.425325f, 
		-0.681718f, -0.147621f, 0.716567f, 
		-0.442863f, -0.238856f, 0.864188f, 
		-0.587785f, -0.425325f, 0.688191f, 
		-0.309017f, -0.500000f, 0.809017f, 
		-0.147621f, -0.716567f, 0.681718f, 
		-0.425325f, -0.688191f, 0.587785f, 
		-0.162460f, -0.262866f, 0.951056f, 
		0.442863f, -0.238856f, 0.864188f, 
		0.162460f, -0.262866f, 0.951056f, 
		0.309017f, -0.500000f, 0.809017f, 
		0.147621f, -0.716567f, 0.681718f, 
		0.000000f, -0.525731f, 0.850651f, 
		0.425325f, -0.688191f, 0.587785f, 
		0.587785f, -0.425325f, 0.688191f, 
		0.688191f, -0.587785f, 0.425325f, 
		-0.955423f, 0.295242f, 0.000000f, 
		-0.951056f, 0.162460f, 0.262866f, 
		-1.000000f, 0.000000f, 0.000000f, 
		-0.850651f, 0.000000f, 0.525731f, 
		-0.955423f, -0.295242f, 0.000000f, 
		-0.951056f, -0.162460f, 0.262866f, 
		-0.864188f, 0.442863f, -0.238856f, 
		-0.951056f, 0.162460f, -0.262866f, 
		-0.809017f, 0.309017f, -0.500000f, 
		-0.864188f, -0.442863f, -0.238856f, 
		-0.951056f, -0.162460f, -0.262866f, 
		-0.809017f, -0.309017f, -0.500000f, 
		-0.681718f, 0.147621f, -0.716567f, 
		-0.681718f, -0.147621f, -0.716567f, 
		-0.850651f, 0.000000f, -0.525731f, 
		-0.688191f, 0.587785f, -0.425325f, 
		-0.587785f, 0.425325f, -0.688191f, 
		-0.425325f, 0.688191f, -0.587785f, 
		-0.425325f, -0.688191f, -0.587785f, 
		-0.587785f, -0.425325f, -0.688191f, 
		-0.688191f, -0.587785f, -0.425325f, 
	};

} // namespace


// =================================================
// Quake II model reader
// =================================================

class ReaderQ2MDL
{
	public:

	ReaderQ2MDL(ImportQ2MDL& ip, Stream& str)
	: import(ip),stream(str)
	{
		// reset stream
		stream.Seek(0,Stream::START);

		// id
		uint32 id = ReadLittleEndian<uint32>(stream);
		if ( id != PRCORE_CODE32('I','D','P','2') )
			PRCORE_EXCEPTION("Incorrect header id.");

		// version
		uint32 version = ReadLittleEndian<uint32>(stream);
		if ( version != 8 )
			PRCORE_EXCEPTION("Incorrect version.");

		int skinwidth        = ReadLittleEndian<int32>(stream);
		int skinheight       = ReadLittleEndian<int32>(stream);
		                       ReadLittleEndian<int32>(stream); // framesize
		int numskin          = ReadLittleEndian<int32>(stream);
		int numvertex        = ReadLittleEndian<int32>(stream);
		int numtexcoord      = ReadLittleEndian<int32>(stream);
		int numtriangle      = ReadLittleEndian<int32>(stream);
		int numglcommand     = ReadLittleEndian<int32>(stream);
		int numframe         = ReadLittleEndian<int32>(stream);
		int offsetskins      = ReadLittleEndian<int32>(stream);
		int offsettexcoords  = ReadLittleEndian<int32>(stream);
		int offsettriangles  = ReadLittleEndian<int32>(stream);
		int offsetframes     = ReadLittleEndian<int32>(stream);
		int offsetglcommands = ReadLittleEndian<int32>(stream);
		                       ReadLittleEndian<int32>(stream); // offsetend

		// skins
		stream.Seek(offsetskins,Stream::START);
		import.skins.SetSize(numskin);

		int i;
		for ( i=0; i<numskin; ++i )
		{
			char name[64];
			stream.Read(name,64);
			import.skins[i] = name;
		}

		// texture coordinates
		stream.Seek(offsettexcoords,Stream::START);
		import.texcoords.SetSize(numtexcoord);

		for ( i=0; i<numtexcoord; ++i )
		{
			import.texcoords[i].x =
				static_cast<float>(ReadLittleEndian<int16>(stream)) /
				static_cast<float>(skinwidth);

			import.texcoords[i].y =
				static_cast<float>(ReadLittleEndian<int16>(stream)) /
				static_cast<float>(skinheight);
		}

		// triangles
		stream.Seek(offsettriangles,Stream::START);
		import.triangles.SetSize(numtriangle);

		for ( i=0; i<numtriangle; ++i )
		{
			int j;
			for ( j=0; j<3; ++j )
			{
				import.triangles[i].vertindex[j] = ReadLittleEndian<int16>(stream);
			}

			for ( j=0; j<3; ++j )
			{
				import.triangles[i].texindex[j] = ReadLittleEndian<int16>(stream);
			}
		}

		// frames
		stream.Seek(offsetframes,Stream::START);
		import.frames.SetSize(numframe);

		for ( i=0; i<numframe; ++i )
		{
			FrameQ2MDL& frame = import.frames[i];
			vec3f scale     = ReadVector3D();
			vec3f translate = ReadVector3D();

			char name[16];
			stream.Read(name,16);
			frame.name = name;
		
			frame.vertexarray.SetSize(numvertex);

			for ( int j=0; j<numvertex; ++j )
			{
				VertexQ2MDL& vertex = frame.vertexarray[j];
				vertex.point = ReadVertex(scale, translate, vertex.normal);
			}
		}

		// glcommands
		stream.Seek(offsetglcommands,Stream::START);

		while ( numglcommand > 0 )
		{
			CommandsQ2MDL command;

			int numvert = ReadLittleEndian<int32>(stream);
			numglcommand--;

			if ( numvert > 0 )
			{
				command.type = Q2TRIANGLE_STRIP;
			}
			else if ( numvert < 0 )
			{
				command.type = Q2TRIANGLE_FAN;
				numvert = -numvert;
			}
			else
			{
				command.type = Q2NONE;
				continue;
			}

			command.vertinfo.SetSize(numvert);
			for ( int j=0; j<numvert; ++j )
			{
				VertexInfoQ2MDL& vertinfo = command.vertinfo[j];
				vertinfo.texcoords = ReadVector2D();
				vertinfo.index     = ReadLittleEndian<int32>(stream);
			}
			numglcommand -= numvert*3;

			import.commands.PushBack(command);
		}
	}

	~ReaderQ2MDL()
	{
	}

	private:

	ImportQ2MDL&	import;
	Stream&			stream;

	vec2f ReadVector2D()
	{
		vec2f v;
		v.x = ReadLittleEndian<float>(stream);
		v.y = ReadLittleEndian<float>(stream);
		return v;
	}
	
	vec3f ReadVector3D()
	{
		vec3f v;
		v.x = ReadLittleEndian<float>(stream);
		v.y = ReadLittleEndian<float>(stream);
		v.z = ReadLittleEndian<float>(stream);
		return v;
	}

	vec3f ReadVertex(const vec3f& scale, const vec3f& origin, vec3f& normal)
	{
		vec3f v;
		v.x = static_cast<float>(ReadLittleEndian<uint8>(stream));
		v.y = static_cast<float>(ReadLittleEndian<uint8>(stream));
		v.z = static_cast<float>(ReadLittleEndian<uint8>(stream));

		int index = static_cast<int>(ReadLittleEndian<uint8>(stream));

		vec3f& n = *reinterpret_cast<vec3f*>(gva_normal + index * 3);
		normal = Remap(n);

		return Remap(v * scale + origin);
	}

	vec3f Remap(const vec3f& v) const
	{
		return vec3f(v.y,v.z,v.x);
	}
};


// =================================================
// importer
// =================================================

ImportQ2MDL::ImportQ2MDL(Stream& stream)
{
	ReaderQ2MDL reader(*this,stream);
}


ImportQ2MDL::ImportQ2MDL(const String& filename)
{
	FileStream stream(filename);
	ReaderQ2MDL reader(*this,stream);
}


ImportQ2MDL::~ImportQ2MDL()
{
}
