/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_LWO2_HPP
#define PRIMPORT_LWO2_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::String;
	using prcore::Color32;
	using prcore::Array;


	// ====================================
	// structs
	// ====================================

	struct TextureLWO2
	{
		String	filename;
		bool	enable;
	};

	enum
	{
		LWO2_TEXTURE_COLOR,
		LWO2_TEXTURE_DIFFUSE,
		LWO2_TEXTURE_LUMINOSITY,
		LWO2_TEXTURE_SPECULAR,
		LWO2_TEXTURE_REFLECTION,
		LWO2_TEXTURE_TRANSPARENCY,
		LWO2_TEXTURE_TRANSLUCENCY,
		LWO2_TEXTURE_GLOSSINESS,
		LWO2_TEXTURE_SHARPNESS,
		LWO2_TEXTURE_BUMP,
		LWO2_TEXTURE_COUNT // not a texture channel
	};

	struct SurfaceLWO2
	{
		String		name;
		Color32		color;
		float		diffuse;
		float		luminosity;
		float		specular;
		float		reflection;
		float		transparency;
		float		translucency;
		float		glossiness;
		float		sharpness;
		float		bump;
		bool		twosided;
		TextureLWO2	texture[LWO2_TEXTURE_COUNT];
	};
	
	struct VertexLWO2
	{
		point3f	point;
		vec3f	normal;
		vec2f	texture;
	};

	struct IndexBufferLWO2
	{
		uint32			surface;
		Array<uint32>	indexbuffer;
	};

	struct MeshLWO2
	{
		Array<VertexLWO2>		vertexbuffer;
		Array<IndexBufferLWO2>	indexbuffers;
	};


	// ====================================
	// importer
	// ====================================

	class ImportLWO2 : public prcore::RefCount
	{
		public:

		Array<SurfaceLWO2>	surfaces;
		Array<MeshLWO2>		meshes;

		ImportLWO2(prcore::Stream& stream);
		ImportLWO2(const String& filename);
		~ImportLWO2();
	};

} // namespace primport


#endif