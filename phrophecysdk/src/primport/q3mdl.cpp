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
		ID Software Quake III Arena: model reader (.md3)

	revision history:
		Dec/26/1999 - Jukka Liimatta - initial revision
		Feb/10/2001 - Jukka Liimatta - renaissance build
		Aug/17/2001 - Jukka Liimatta - added support for vertex normals
*/
#include <primport/q3mdl.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// reader declaration
// =================================================

class ReaderQ3MDL
{
	public:

	ReaderQ3MDL(ImportQ3MDL& import, Stream& stream);
	~ReaderQ3MDL();

	private:

	ImportQ3MDL&	import;
	Stream&			stream;

	vec2f			ReadVector2D();
	vec3f			ReadVector3D();
	vec3f			Remap(const vec3f& v) const;
	matrix4x4f		Remap(const matrix4x4f& m) const;
	vec3f			GetVertexNormal(uint8 longtitude, uint8 latitude) const;
};


// =================================================
// reader implementation
// =================================================

ReaderQ3MDL::ReaderQ3MDL(ImportQ3MDL& im, Stream& s)
: import(im), stream(s)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// id
	uint32 id = ReadLittleEndian<uint32>(stream);
	if ( id != PRCORE_CODE32('I','D','P','3') )
		PRCORE_EXCEPTION("Incorrect header id.");

	// header
	uint32 version = ReadLittleEndian<uint32>(stream);
	if ( version != 15 )
		PRCORE_EXCEPTION("Incorrect version.");

	char filename[68]; 
	stream.Read(filename,68);

	int numframe     = ReadLittleEndian<int32>(stream);
	int numtag       = ReadLittleEndian<int32>(stream);
	int nummesh      = ReadLittleEndian<int32>(stream);
	                   ReadLittleEndian<int32>(stream); // numskin
	int offsetframes = ReadLittleEndian<int32>(stream);
	int offsettags   = ReadLittleEndian<int32>(stream);
	int offsetmeshes = ReadLittleEndian<int32>(stream);
	                   ReadLittleEndian<int32>(stream); // filesize

	// frames
	stream.Seek(offsetframes,Stream::START);
	import.frameboxes.SetSize(numframe);
	int i = 0;
	for ( i=0; i<numframe; ++i )
	{
		vec3f bmin = ReadVector3D();
		vec3f bmax = ReadVector3D();
		vec3f pos  = ReadVector3D();
		import.frameboxes[i].radius = ReadLittleEndian<float>(stream);

		char creator[16];
		stream.Read(creator,16);

		import.frameboxes[i].box.vmin = Remap(bmin);
		import.frameboxes[i].box.vmax = Remap(bmax);
		import.frameboxes[i].pos      = Remap(pos);
	}

	// tags
	stream.Seek(offsettags,Stream::START);
	import.tags.SetSize( numtag );
	for ( i=0; i<numframe; ++i )
	{
		for ( int j=0; j<numtag; ++j )
		{
			char name[64]; 
			stream.Read(name,64);

			if ( import.tags[j].name.IsEmpty() )
				import.tags[j].name = name;

			matrix4x4f m(1);
			m.SetT(ReadVector3D());
			m.SetX(ReadVector3D());
			m.SetY(ReadVector3D());
			m.SetZ(ReadVector3D());

			import.tags[j].matrices.PushBack(Remap(m));
		}
	}

	// meshes
	for ( i=0; i<nummesh; ++i )
	{
		stream.Seek(offsetmeshes,Stream::START);
		MeshQ3MDL mesh;

		// id
		uint32 id = ReadLittleEndian<uint32>(stream);
		if ( id != PRCORE_CODE32('I','D','P','3') )
			PRCORE_EXCEPTION("Incorrect header id.");

		// header
		char meshname[68]; 
		stream.Read(meshname,68);

		int numframemesh     = ReadLittleEndian<int32>(stream);
		int numskinmesh      = ReadLittleEndian<int32>(stream);
		int numvertexmesh    = ReadLittleEndian<int32>(stream);
		int numtrimesh       = ReadLittleEndian<int32>(stream);
		int offsettrimesh    = ReadLittleEndian<int32>(stream);
		int offsetskinmesh   = ReadLittleEndian<int32>(stream);
		int offsettexmesh    = ReadLittleEndian<int32>(stream);
		int offsetvertexmesh = ReadLittleEndian<int32>(stream);
		int offsetendmesh    = ReadLittleEndian<int32>(stream);

		mesh.name = meshname;

		// skins
		stream.Seek(offsetmeshes + offsetskinmesh,Stream::START);
		int j = 0;
		for ( j=0; j<numskinmesh; ++j )
		{
			char skinname[68]; 
			stream.Read(skinname,68);
			if ( strlen(skinname) > 0 )
				mesh.skins.PushBack(skinname);
		}

		// setup arrays
		mesh.faces.SetSize(numtrimesh);
		mesh.texcoords.SetSize(numvertexmesh);

		// faces
		stream.Seek(offsetmeshes + offsettrimesh,Stream::START);
		for ( j=0; j<numtrimesh; ++j )
		{
			FaceQ3MDL& face = mesh.faces[j];
			face.index[0] = ReadLittleEndian<int32>(stream);
			face.index[1] = ReadLittleEndian<int32>(stream);
			face.index[2] = ReadLittleEndian<int32>(stream);
		}

		// texcoords
		stream.Seek(offsetmeshes + offsettexmesh,Stream::START);
		for ( j=0; j<numvertexmesh; ++j )
		{
			mesh.texcoords[j] = ReadVector2D();
		}
	
		// points
		stream.Seek(offsetmeshes + offsetvertexmesh,Stream::START);
		for ( j=0; j<numframemesh; ++j )
		{
			FrameQ3MDL frame;

			const float scale = 1.0f / 64.0f;
			for ( int k=0; k<numvertexmesh; ++k )
			{
				vec3f point;
				point.x = static_cast<float>(ReadLittleEndian<int16>(stream)) * scale;
				point.y = static_cast<float>(ReadLittleEndian<int16>(stream)) * scale;
				point.z = static_cast<float>(ReadLittleEndian<int16>(stream)) * scale;
				
				uint8 longtitude = ReadLittleEndian<int8>(stream);
				uint8 latitude   = ReadLittleEndian<int8>(stream);
				
				VertexQ3MDL vertex;
				vertex.point  = Remap(point);
				vertex.normal = Remap(GetVertexNormal(longtitude,latitude));
				
				frame.vertices.PushBack(vertex);
			}

			mesh.frames.PushBack(frame);
		}

		import.meshes.PushBack(mesh);
		offsetmeshes += offsetendmesh;
	}
}


ReaderQ3MDL::~ReaderQ3MDL()
{
}


vec2f ReaderQ3MDL::ReadVector2D()
{
	vec2f v;
	v.x = ReadLittleEndian<float>(stream);
	v.y = ReadLittleEndian<float>(stream);
	return v;
}


vec3f ReaderQ3MDL::ReadVector3D()
{
	vec3f v;
	v.x = ReadLittleEndian<float>(stream);
	v.y = ReadLittleEndian<float>(stream);
	v.z = ReadLittleEndian<float>(stream);
	return v;
}


vec3f ReaderQ3MDL::Remap(const vec3f& v) const
{
	return vec3f(v.y,v.z,v.x);
}


matrix4x4f ReaderQ3MDL::Remap(const matrix4x4f& m) const
{
	matrix4x4f e(1);
	e.SetX(Remap(m.GetY()));
	e.SetY(Remap(m.GetZ()));
	e.SetZ(Remap(m.GetX()));
	e.SetT(Remap(m.GetT()));
	return e;
}


vec3f ReaderQ3MDL::GetVertexNormal(uint8 longtitude8bit, uint8 latitude8bit) const
{
	// based on Nate Miller's MD3 reader
	const float sp = prmath::pi * 2 / 255.0f;

	float longtitude = static_cast<float>(longtitude8bit) * sp;
	float latitude   = static_cast<float>(latitude8bit) * sp;
	float sinlong    = sinf(longtitude);
	
	return vec3f(
		cosf(latitude) * sinlong,
		sinf(latitude) * sinlong,
		cosf(longtitude));
}


// =================================================
// importer
// =================================================

ImportQ3MDL::ImportQ3MDL(Stream& stream)
{
	ReaderQ3MDL reader(*this,stream);
}


ImportQ3MDL::ImportQ3MDL(const String& filename)
{
	FileStream stream(filename);
	ReaderQ3MDL reader(*this,stream);
}


ImportQ3MDL::~ImportQ3MDL()
{
}
