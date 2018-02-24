/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_Q2MDL_HPP
#define PRIMPORT_Q2MDL_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::Array;
	using prcore::String;

	typedef point2f	TexCoordsQ2MDL;

	// ====================================
	// structs
	// ====================================

	enum TypeQ2MDL
	{
		Q2NONE,
		Q2TRIANGLE_STRIP,
		Q2TRIANGLE_FAN
	};

	struct VertexQ2MDL
	{
		point3f	point;
		vec3f	normal;
	};

	struct TriangleQ2MDL
	{
		int	vertindex[3];
		int	texindex[3];
	};

	struct FrameQ2MDL
	{
		String				name;
		Array<VertexQ2MDL>	vertexarray;
	};

	struct VertexInfoQ2MDL
	{
		TexCoordsQ2MDL	texcoords;
		int				index;
	};

	struct CommandsQ2MDL
	{
		TypeQ2MDL				type;
		Array<VertexInfoQ2MDL>	vertinfo;
	};

	// ====================================
	// importer
	// ====================================

	class ImportQ2MDL : public prcore::RefCount
	{
		public:

		Array<prcore::String>	skins;
		Array<TriangleQ2MDL>	triangles;
		Array<TexCoordsQ2MDL>	texcoords;
		Array<FrameQ2MDL>		frames;
		Array<CommandsQ2MDL>	commands;

		ImportQ2MDL(prcore::Stream& stream);
		ImportQ2MDL(const String& filename);
		~ImportQ2MDL();
	};

} // namespace primport


#endif