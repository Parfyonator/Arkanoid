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
		ID Software Quake model reader (.mdl)

	revision history:
		Jul/30/2001 - Jukka Liimatta - initial revision
		Feb/06/2002 - Francesco Montecuccoli - added bounding box support
*/
#include <primport/q1mdl.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// quake constants
// =================================================

namespace
{

	#pragma pack(1)
	uint8 quake_palette[768] =\
	{
		0, 0, 0, 15, 15, 15, 31, 31, 31, 47, 47, 47, 63, 63, 63, 75,
		75, 75, 91, 91, 91, 107, 107, 107, 123, 123, 123, 139, 139, 139, 155, 155,
		155, 171, 171, 171, 187, 187, 187, 203, 203, 203, 219, 219, 219, 235, 235, 235,
		15, 11, 7, 23, 15, 11, 31, 23, 11, 39, 27, 15, 47, 35, 19, 55,
		43, 23, 63, 47, 23, 75, 55, 27, 83, 59, 27, 91, 67, 31, 99, 75,
		31, 107, 83, 31, 115, 87, 31, 123, 95, 35, 131, 103, 35, 143, 111, 35,
		11, 11, 15, 19, 19, 27, 27, 27, 39, 39, 39, 51, 47, 47, 63, 55,
		55, 75, 63, 63, 87, 71, 71, 103, 79, 79, 115, 91, 91, 127, 99, 99,
		139, 107, 107, 151, 115, 115, 163, 123, 123, 175, 131, 131, 187, 139, 139, 203,
		0, 0, 0, 7, 7, 0, 11, 11, 0, 19, 19, 0, 27, 27, 0, 35,
		35, 0, 43, 43, 7, 47, 47, 7, 55, 55, 7, 63, 63, 7, 71, 71,
		7, 75, 75, 11, 83, 83, 11, 91, 91, 11, 99, 99, 11, 107, 107, 15,
		7, 0, 0, 15, 0, 0, 23, 0, 0, 31, 0, 0, 39, 0, 0, 47,
		0, 0, 55, 0, 0, 63, 0, 0, 71, 0, 0, 79, 0, 0, 87, 0,
		0, 95, 0, 0, 103, 0, 0, 111, 0, 0, 119, 0, 0, 127, 0, 0,
		19, 19, 0, 27, 27, 0, 35, 35, 0, 47, 43, 0, 55, 47, 0, 67,
		55, 0, 75, 59, 7, 87, 67, 7, 95, 71, 7, 107, 75, 11, 119, 83,
		15, 131, 87, 19, 139, 91, 19, 151, 95, 27, 163, 99, 31, 175, 103, 35,
		35, 19, 7, 47, 23, 11, 59, 31, 15, 75, 35, 19, 87, 43, 23, 99,
		47, 31, 115, 55, 35, 127, 59, 43, 143, 67, 51, 159, 79, 51, 175, 99,
		47, 191, 119, 47, 207, 143, 43, 223, 171, 39, 239, 203, 31, 255, 243, 27,
		11, 7, 0, 27, 19, 0, 43, 35, 15, 55, 43, 19, 71, 51, 27, 83,
		55, 35, 99, 63, 43, 111, 71, 51, 127, 83, 63, 139, 95, 71, 155, 107,
		83, 167, 123, 95, 183, 135, 107, 195, 147, 123, 211, 163, 139, 227, 179, 151,
		171, 139, 163, 159, 127, 151, 147, 115, 135, 139, 103, 123, 127, 91, 111, 119,
		83, 99, 107, 75, 87, 95, 63, 75, 87, 55, 67, 75, 47, 55, 67, 39,
		47, 55, 31, 35, 43, 23, 27, 35, 19, 19, 23, 11, 11, 15, 7, 7,
		187, 115, 159, 175, 107, 143, 163, 95, 131, 151, 87, 119, 139, 79, 107, 127,
		75, 95, 115, 67, 83, 107, 59, 75, 95, 51, 63, 83, 43, 55, 71, 35,
		43, 59, 31, 35, 47, 23, 27, 35, 19, 19, 23, 11, 11, 15, 7, 7,
		219, 195, 187, 203, 179, 167, 191, 163, 155, 175, 151, 139, 163, 135, 123, 151,
		123, 111, 135, 111, 95, 123, 99, 83, 107, 87, 71, 95, 75, 59, 83, 63,
		51, 67, 51, 39, 55, 43, 31, 39, 31, 23, 27, 19, 15, 15, 11, 7,
		111, 131, 123, 103, 123, 111, 95, 115, 103, 87, 107, 95, 79, 99, 87, 71,
		91, 79, 63, 83, 71, 55, 75, 63, 47, 67, 55, 43, 59, 47, 35, 51,
		39, 31, 43, 31, 23, 35, 23, 15, 27, 19, 11, 19, 11, 7, 11, 7,
		255, 243, 27, 239, 223, 23, 219, 203, 19, 203, 183, 15, 187, 167, 15, 171,
		151, 11, 155, 131, 7, 139, 115, 7, 123, 99, 7, 107, 83, 0, 91, 71,
		0, 75, 55, 0, 59, 43, 0, 43, 31, 0, 27, 15, 0, 11, 7, 0,
		0, 0, 255, 11, 11, 239, 19, 19, 223, 27, 27, 207, 35, 35, 191, 43,
		43, 175, 47, 47, 159, 47, 47, 143, 47, 47, 127, 47, 47, 111, 47, 47,
		95, 43, 43, 79, 35, 35, 63, 27, 27, 47, 19, 19, 31, 11, 11, 15,
		43, 0, 0, 59, 0, 0, 75, 7, 0, 95, 7, 0, 111, 15, 0, 127,
		23, 7, 147, 31, 7, 163, 39, 11, 183, 51, 15, 195, 75, 27, 207, 99,
		43, 219, 127, 59, 227, 151, 79, 231, 171, 95, 239, 191, 119, 247, 211, 139,
		167, 123, 59, 183, 155, 55, 199, 195, 55, 231, 227, 87, 127, 191, 255, 171,
		231, 255, 215, 255, 255, 103, 0, 0, 139, 0, 0, 179, 0, 0, 215, 0,
		0, 255, 0, 0, 255, 243, 147, 255, 247, 199, 255, 255, 255, 159, 91, 83
	};
	#pragma pack()


	float quake_normal[] = \
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


	void GetQuakePalette(Color32 palette[])
	{
		uint8* p = quake_palette;
		for ( int i=0; i<256; ++i )
		{
			palette[i].r = *p++;
			palette[i].g = *p++;
			palette[i].b = *p++;
			palette[i].a = 255;
		}
	}

} // namespace


// =================================================
// reader
// =================================================

class ReaderMDL
{
	public:

	ReaderMDL(ImportQ1MDL& ip, Stream& s)
	: import(ip), stream(s)
	{
		// reset stream
		stream.Seek(0,Stream::START);

		// id
		uint32 id = ReadLittleEndian<uint32>(stream);
		if ( id != PRCORE_CODE32('I','D','P','O') )
			PRCORE_EXCEPTION("Incorrect header id.");

		// version
		uint32 version = ReadLittleEndian<uint32>(stream);
		if ( version != 6 )
			PRCORE_EXCEPTION("Incorrect version.");

		// header
		vec3f scale       = ReadVector3D();
		vec3f origin      = ReadVector3D();
		                    ReadLittleEndian<float>(stream);  // radius
		                    ReadVector3D();                   // eyepos
		uint32 numskin    = ReadLittleEndian<uint32>(stream);
		uint32 skinwidth  = ReadLittleEndian<uint32>(stream);
		uint32 skinheight = ReadLittleEndian<uint32>(stream);
		uint32 numvertex  = ReadLittleEndian<uint32>(stream);
		uint32 numface    = ReadLittleEndian<uint32>(stream);
		uint32 numframe   = ReadLittleEndian<uint32>(stream);
		                    ReadLittleEndian<uint32>(stream); // synctype
		                    ReadLittleEndian<uint32>(stream); // flags
		                    ReadLittleEndian<float>(stream);  // size

		// palette for skins
		Color32 palette[256];
		GetQuakePalette(palette);

		// support only one skin
		import.skin = Bitmap(skinwidth,skinheight,PIXELFORMAT_PALETTE8(palette));

		// skin(s)
		int i;
		for ( i=0; i<static_cast<int>(numskin); ++i )
		{
			uint8* image = import.skin.GetImage();

			ReadLittleEndian<uint32>(stream); // group
			stream.Read(image,skinwidth*skinheight);
		}

		// texture coordinates
		struct tex2f
		{
			uint32 onseam;
			uint32 s;
			uint32 t;
		};
		tex2f* texcoord = new tex2f[numvertex];

		for ( i=0; i<static_cast<int>(numvertex); ++i )
		{
			tex2f& tex = texcoord[i];

			tex.onseam = ReadLittleEndian<uint32>(stream);
			tex.s      = ReadLittleEndian<uint32>(stream);
			tex.t      = ReadLittleEndian<uint32>(stream);
		}

		// faces (triangles)
		import.faces.SetSize(numface);

		for ( i=0; i<static_cast<int>(numface); ++i )
		{
			FaceMDL& face = import.faces[i];

			uint32 frontface = ReadLittleEndian<uint32>(stream);
			face.index[0]    = ReadLittleEndian<uint32>(stream);
			face.index[1]    = ReadLittleEndian<uint32>(stream);
			face.index[2]    = ReadLittleEndian<uint32>(stream);

			for ( int j=0; j<3; ++j )
			{
				tex2f& tex = texcoord[face.index[j]];

				uint32 s = (!frontface && tex.onseam) ? tex.s + (skinwidth / 2) : tex.s;
				uint32 t = tex.t;

				face.texcoord[j].x = static_cast<float>(s) / static_cast<float>(skinwidth);
				face.texcoord[j].y = static_cast<float>(t) / static_cast<float>(skinheight);
			}
		}

		delete[] texcoord;

		// frames
		import.frames.SetSize(numframe);

		for ( i=0; i<static_cast<int>(numframe); ++i )
		{
			FrameMDL& frame = import.frames[i];

			ReadLittleEndian<uint32>(stream); // type
			frame.box.vmin = ReadVertex(scale,origin,NULL);
			frame.box.vmax = ReadVertex(scale,origin,NULL);

			char name[16];
			stream.Read(name,16);

			frame.name = name;
			frame.vertexarray.SetSize(numvertex);

			for ( int j=0; j<static_cast<int>(numvertex); ++j )
			{
				VertexMDL& vertex = frame.vertexarray[j];
				vertex.point = ReadVertex(scale,origin,&vertex.normal);
			}
		}
	}

	~ReaderMDL()
	{
	}

	private:

	ImportQ1MDL&	import;
	Stream&			stream;

	vec3f ReadVector3D()
	{
		vec3f v;
		v.x = ReadLittleEndian<float>(stream);
		v.y = ReadLittleEndian<float>(stream);
		v.z = ReadLittleEndian<float>(stream);
		return v;
	}

	vec3f ReadVertex(const vec3f& scale, const vec3f& origin, vec3f* normal)
	{
		vec3f v;
		v.x = static_cast<float>(ReadLittleEndian<uint8>(stream));
		v.y = static_cast<float>(ReadLittleEndian<uint8>(stream));
		v.z = static_cast<float>(ReadLittleEndian<uint8>(stream));

		int index = static_cast<int>(ReadLittleEndian<uint8>(stream));
		if ( normal )
		{
			vec3f& n = *reinterpret_cast<vec3f*>(quake_normal + index * 3);
			*normal = Remap(n);
		}

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

ImportQ1MDL::ImportQ1MDL(Stream& stream)
{
	ReaderMDL reader(*this,stream);
}


ImportQ1MDL::ImportQ1MDL(const String& filename)
{
	FileStream stream(filename);
	ReaderMDL reader(*this,stream);
}


ImportQ1MDL::~ImportQ1MDL()
{
}
