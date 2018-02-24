/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_3DS_HPP
#define PRIMPORT_3DS_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::Array;
	using prcore::String;
	using prcore::Color32;


	// ====================================
	// keyframer structs
	// ====================================

	struct KeyHeader3DS
	{
		enum
		{
			TENSION		= 0x010000,
			CONT		= 0x020000,
			BIAS		= 0x040000,
			EASETO		= 0x080000,
			EASEFROM	= 0x100000
		};
		uint16	flags;
		uint32	time;
		float	tension;
		float	continuity;
		float	bias;
		float	easeto;
		float	easefrom;
	};

	struct PosKey3DS : public KeyHeader3DS
	{
		point3f pos;
	};

	struct RotKey3DS : public KeyHeader3DS
	{
		quat4f rot;
	};

	struct ScaleKey3DS : public KeyHeader3DS
	{
		vec3f scale;
	};

	struct RollKey3DS : public KeyHeader3DS
	{
		float roll;
	};

	struct FovKey3DS : public KeyHeader3DS
	{
		float fov;
	};

	struct MorphKey3DS : public KeyHeader3DS
	{
		String name;
	};

	struct ColorKey3DS : public KeyHeader3DS
	{
		Color32 color;
	};


	// ====================================
	// editor structs
	// ====================================

	struct Texture3DS
	{
		enum Tiling 
		{ 
			TILE	= 1, 
			DECAL	= 2, 
			BOTH	= 3 
		};
		Tiling	tiling;
		String	filename;
		float	strength;
		vec2f	scale;
		vec2f	offset;
		float	angle;
		float	blur;
	};

	struct Material3DS
	{
		enum Shade
		{
			WIRE	= 0,
			FLAT	= 1,
			GOURAUD	= 2,
			PHONG	= 3,
			METAL	= 4
		};
		Shade		shade;
		String		name;
		Color32		ambient; 
		Color32		diffuse;
		Color32		specular;
		float		shininess;
		float		transparency;
		bool		twosided;
		bool		additive;
		bool		wireframe;
		Texture3DS	texturemap1;
		Texture3DS	texturemap2;
		Texture3DS	opacitymap;
		Texture3DS	bumpmap;
		Texture3DS	specularitymap;
		Texture3DS	shininessmap;
		Texture3DS	selfilluminationmap;
		Texture3DS	reflectionmap;
	};

	struct Vertex3DS
	{
		point3f		point;
		vec3f		normal;
		point2f		texcoord;
	};

	struct Face3DS
	{
		enum Flags
		{
			EDGE_CA		= 0x0001,
			EDGE_BC		= 0x0002,
			EDGE_AB		= 0x0004,
			WRAP_U		= 0x0008,
			WRAP_V		= 0x0010
		};
		uint16	flags;
		uint32	smoothgroup;
		uint16	vertex[3];
		uint32	material;
	};

	struct Object3DS
	{
		String	name;
		uint16	node;
	};

	struct TriMesh3DS : public Object3DS
	{
		matrix4x4f			xform;
		Array<Vertex3DS>	vertices;
		Array<Face3DS>		faces;
	};

	struct Camera3DS : public Object3DS
	{
		point3f		position;
		point3f		target;
		float		bank;
		float		lense;
		float		inner_range;
		float		outer_range;
	};

	struct Light3DS : public Object3DS
	{
		point3f		position;
		Color32		color;
		bool		enable;
		float		brightness;
		float		falloff;
		bool		spotlight;
		point3f		target;
		float		hotspot;
		float		inner_range;
		float		outer_range;
		float		roll;
	};

	struct Node3DS
	{
		enum Type
		{
			NODE,
			TRIMESH,
			CAMERA,
			LIGHT,
			SPOTLIGHT,
			TARGETCAMERA,
			TARGETLIGHT,
			AMBIENT,
		};
		Type		type;
		String		name;
		uint16		nodeid;
		uint16		parent;
		uint16		target;
		point3f		pivot;

		Array<PosKey3DS>	postrack;		// node
		Array<RotKey3DS>	rottrack;		// trimesh
		Array<ScaleKey3DS>	scaletrack;		// trimesh
		Array<MorphKey3DS>	morphtrack;		// trimesh
		Array<RollKey3DS>	rolltrack;		// camera
		Array<FovKey3DS>	fovtrack;		// camera
		Array<ColorKey3DS>	colortrack;		// light
	};


	// ====================================
	// importer
	// ====================================

	class Import3DS : public prcore::RefCount
	{
		public:

		Array<Material3DS>	materials;
		Array<TriMesh3DS>	trimeshes;
		Array<Camera3DS>	cameras;
		Array<Light3DS>		lights;
		Array<Node3DS>		nodes;

		Import3DS(prcore::Stream& stream);
		Import3DS(const String& filename);
		~Import3DS();

		uint16		FindNodeID(const Object3DS& object) const;
		Node3DS*	FindNode(const Object3DS& object);
	};

} // namespace primport


#endif