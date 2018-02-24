/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_Q3MDL_HPP
#define PRIMPORT_Q3MDL_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::String;
	using prcore::Array;


	// ====================================
	// structs
	// ====================================

	struct TagQ3MDL
	{
		String name;
		Array<matrix4x4f> matrices;
	};
	
	struct VertexQ3MDL
	{
		point3f	point;
		vec3f	normal;
	};

	struct FrameQ3MDL
	{
		Array<VertexQ3MDL> vertices;
	};

	struct FaceQ3MDL
	{
		uint16 index[3];
	};

	struct MeshQ3MDL
	{
		String				name;
		Array<String>		skins;
		Array<FrameQ3MDL>	frames;
		Array<point2f>		texcoords;
		Array<FaceQ3MDL>	faces;
	};

	struct BBoxQ3MDL
	{
		box3f	box;
		vec3f	pos;
		float	radius;
	};


	// ====================================
	// importer
	// ====================================

	class ImportQ3MDL : public prcore::RefCount
	{
		public:

		Array<TagQ3MDL>		tags;
		Array<MeshQ3MDL>	meshes;
		Array<BBoxQ3MDL>	frameboxes;

		ImportQ3MDL(prcore::Stream& stream);
		ImportQ3MDL(const String& filename);
		~ImportQ3MDL();
	};

} // namespace primport


#endif