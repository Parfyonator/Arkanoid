/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_LWO_HPP
#define PRIMPORT_LWO_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::Array;
	using prcore::String;
	using prcore::Color32;


	// ====================================
	// structs
	// ====================================

	struct TextureLWO
	{
		enum Flags
		{
			X_AXIS		= 0x01,
			Y_AXIS		= 0x02,
			Z_AXIS		= 0x04,
			WORLDCOORD	= 0x08,
			NEGIMAGE	= 0x10,
			PIXELBLEND	= 0x20,
			ANTIALIAS	= 0x40
		};
		enum Style
		{
			PLANAR		= 0,
			CYLINDRICAL	= 1,
			SPHERICAL	= 2,
			CUBIC		= 3
		};

		point3f	center;
		vec3f	velocity;
		vec3f	falloff;
		vec3f	size;
		vec3f	tile;
		int32	flags;
		int32	style;
		String	filename;

		point2f GetTexCoord(const point3f& point, float time) const;
	};

	struct SurfaceLWO
	{
		enum Flags
		{
			LUMINOUS			= 0x001,
			OUTLINE				= 0x002,
			SMOOTHING			= 0x004,
			COLORHIGHLIGHT		= 0x008,
			COLORFILTER			= 0x010,
			OPAQUEEDGE			= 0x020,
			TRANSPARENTEDGE		= 0x040,
			DOUBLESIDED			= 0x080,
			ADDITIVE			= 0x100
		};

		uint16		flags;
		String		name;
		Color32		color;
		TextureLWO	ctex;
		TextureLWO	dtex;
		TextureLWO	stex;
		TextureLWO	rtex;
		TextureLWO	ttex;
		TextureLWO	btex;
		float   	luminosity;
		float   	diffuse;
		float   	specular;
		float   	reflection;
		float   	transparency;
	};

	struct PolygonLWO
	{
		Array<uint16>	vertices;
		uint16			surface;
	};


	// ====================================
	// importer
	// ====================================

	class ImportLWO : public prcore::RefCount
	{
		public:

		Array<point3f>		points;
		Array<PolygonLWO>	polygons;
		Array<SurfaceLWO>	surfaces;

		ImportLWO(prcore::Stream& stream);
		ImportLWO(const String& filename);
		~ImportLWO();
	};

} // namespace primport


#endif