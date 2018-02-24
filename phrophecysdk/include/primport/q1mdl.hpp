/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_Q1MDL_HPP
#define PRIMPORT_Q1MDL_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::Array;
	using prcore::String;
	using prcore::Bitmap;


	// ====================================
	// structs
	// ====================================

	struct VertexMDL
	{
		point3f	point;
		vec3f	normal;
	};

	struct FaceMDL
	{
		uint32	index[3];
		point2f	texcoord[3];
	};

	struct FrameMDL
	{
		String	name;
		box3f	box;
		Array<VertexMDL> vertexarray;
	};

	
	// ====================================
	// importer
	// ====================================

	class ImportQ1MDL : public prcore::RefCount
	{
		public:

		Bitmap			skin;
		Array<FaceMDL>	faces;
		Array<FrameMDL>	frames;

		ImportQ1MDL(prcore::Stream& stream);
		ImportQ1MDL(const String& filename);
		~ImportQ1MDL();
	};

} // namespace primport


#endif