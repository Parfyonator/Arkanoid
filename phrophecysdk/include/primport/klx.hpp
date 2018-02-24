/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_KLX_HPP
#define PRIMPORT_KLX_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::Array;
	using prcore::String;


	// ====================================
	// constants
	// ====================================

	enum
	{
		KLX_FOG_STANDARD	= 0,
		KLX_FOG_LAYERED		= 1,
	};

	enum
	{
		KLX_FOG_LAYERED_FALLOFF_TOP		= 0,
		KLX_FOG_LAYERED_FALLOFF_BOTTOM	= 1,
		KLX_FOG_LAYERED_FALLOFF_NONE	= 2,
	};

	enum
	{
		KLX_MATERIAL_TWOSIDED	= 0x1,
		KLX_MATERIAL_WIREFRAME	= 0x2,
		KLX_MATERIAL_FACEMAP	= 0x4,
		KLX_MATERIAL_ADDITIVE	= 0x8,
		KLX_MATERIAL_FACETED    = 0x10,
	};

	enum
	{
		KLX_MAP_AMBIENT				= 0x0,
		KLX_MAP_DIFFUSE				= 0x1,
		KLX_MAP_SPECULAR			= 0x2,
		KLX_MAP_SHININESS			= 0x3,
		KLX_MAP_GLOSSINESS			= 0x4,
		KLX_MAP_SELFILLUMINATION	= 0x5,
		KLX_MAP_OPACITY				= 0x6,
		KLX_MAP_FILTERCOLOR			= 0x7,
		KLX_MAP_BUMP				= 0x8,
		KLX_MAP_REFLECTION			= 0x9,
		KLX_MAP_REFRACTION			= 0xa,
		KLX_MAP_DISPLACEMENT		= 0xb,
		KLX_MAP_MAX					= 0xc,
	};

	enum
	{
		KLX_MAP_EXPLICIT_UV		= 0x0,
		KLX_MAP_EXPLICIT_COLOR	= 0x1,
		KLX_MAP_OBJECT_XYZ		= 0x2,
		KLX_MAP_WORLD_XYZ		= 0x3,
		KLX_MAP_SPHERICAL_ENV	= 0x4,
		KLX_MAP_CYLINDRICAL_ENV	= 0x5,
		KLX_MAP_SHRINKWRAP_ENV	= 0x6,
		KLX_MAP_SCREEN_ENV		= 0x7,
	};

	enum
	{
		KLX_MAP_UWRAP		= 0x1,
		KLX_MAP_VWRAP		= 0x2,
		KLX_MAP_UMIRROR		= 0x4,
		KLX_MAP_VMIRROR 	= 0x8,
	};

	enum
	{
		KLX_INHERIT_POS_X	= 0x1,
		KLX_INHERIT_POS_Y	= 0x2,
		KLX_INHERIT_POS_Z	= 0x4,
		KLX_INHERIT_ROT_X	= 0x8,
		KLX_INHERIT_ROT_Y	= 0x10,
		KLX_INHERIT_ROT_Z	= 0x20,
		KLX_INHERIT_SCL_X	= 0x40,
		KLX_INHERIT_SCL_Y	= 0x80,
		KLX_INHERIT_SCL_Z	= 0x100,
		KLX_INHERIT_ALL		= 511,
	};

	enum
	{
		KLX_TRACK_LINEAR	= 0x1,
		KLX_TRACK_HERMITE	= 0x2,
	};

	enum
	{
		KLX_ORT_CONSTANT		= 0x1,
		KLX_ORT_CYCLE			= 0x2,
		KLX_ORT_LOOP			= 0x3,
		KLX_ORT_PINGPONG		= 0x4,
		KLX_ORT_LINEAR			= 0x5,
		KLX_ORT_RELATIVE_REPEAT	= 0x6,
	};

	enum
	{
		KLX_CAMERA_MANUALCLIP	= 0x1,
		KLX_CAMERA_ORTHOGONAL	= 0x2,
	};

	enum
	{
		KLX_LIGHT_OMNI				= 0x1,
		KLX_LIGHT_FREESPOT			= 0x2,
		KLX_LIGHT_TARGETSPOT		= 0x3,
		KLX_LIGHT_DIRECTIONAL		= 0x4,
		KLX_LIGHT_TARGETDIRECTIONAL = 0x5,
	};

	enum
	{
		KLX_LIGHT_ON				= 0x1,
		KLX_LIGHT_AFFECTDIFFUSE		= 0x2,
		KLX_LIGHT_AFFECTSPECULAR	= 0x4,
		KLX_LIGHT_AMBIENTONLY		= 0x8,
		KLX_LIGHT_SHADOW			= 0x10,
		KLX_LIGHT_NEARATTENUATE		= 0x20,
		KLX_LIGHT_ATTENUATE			= 0x40,
		KLX_LIGHT_OVERSHOOT			= 0x80,
		KLX_LIGHT_RECTANGULAR		= 0x100,
	};

	enum
	{
		KLX_LIGHT_EXCLUDE_INCLUDE		= 0x1,
		KLX_LIGHT_EXCLUDE_ILLUMINATION	= 0x2,
		KLX_LIGHT_EXCLUDE_SHADOWCAST	= 0x4,
	};

	enum
	{
		KLX_INDEXBUFFER_TRILIST		= 0,
		KLX_INDEXBUFFER_TRISTRIP	= 1,
	};

	enum
	{
		KLX_VERTEX_POINT			= 0x0001,
		KLX_VERTEX_NORMAL			= 0x0002,
		KLX_VERTEX_COLOR			= 0x0004,
		KLX_VERTEX_TEXTURE0			= 0x0000,
		KLX_VERTEX_TEXTURE1			= 0x0100,
		KLX_VERTEX_TEXTURE2			= 0x0200,
		KLX_VERTEX_TEXTURE3			= 0x0300,
		KLX_VERTEX_TEXTURE4			= 0x0400,
		KLX_VERTEX_TEXTURE5			= 0x0500,
		KLX_VERTEX_TEXTURE6			= 0x0600,
		KLX_VERTEX_TEXTURE7			= 0x0700,
		KLX_VERTEX_TEXTURE8			= 0x0800,
		KLX_VERTEX_TEXTURE_MASK		= 0x0f00,
		KLX_VERTEX_TEXTURE_SHIFT	= 8,
	};

	enum
	{
		KLX_VERTEXANIMATION_POINT	= 0x1,
		KLX_VERTEXANIMATION_NORMAL	= 0x2,
	};

	enum
	{
		KLX_CHUNK_MAIN						= 0x0717c0cc,
			KLX_CHUNK_WORLDINFO				= 0x00031337,
			KLX_CHUNK_FOGINFO				= 0x00000069,
			KLX_CHUNK_MATERIAL				= 0x00010000,
				KLX_CHUNK_BITMAP			= 0x00010001,
				KLX_CHUNK_DYNAMICS			= 0x00010002,
			KLX_CHUNK_OBJECT				= 0x00020000,
				KLX_CHUNK_POSITION_TRACK	= 0x00020011,
				KLX_CHUNK_ROTATION_TRACK	= 0x00020012,
				KLX_CHUNK_SCALE_TRACK		= 0x00020013,
			KLX_CHUNK_TRIMESH				= 0x00021000,
				KLX_CHUNK_VERTEXBUFFER		= 0x00021001,
				KLX_CHUNK_INDEXBUFFER		= 0x00021002,
				KLX_CHUNK_BONEWEIGHTS		= 0x00021003,
				KLX_CHUNK_VERTEXANIMATION	= 0x00021005,
				KLX_CHUNK_BOUNDINGVOLUMES	= 0x00021010,
				KLX_CHUNK_TRIMESH_EXTRAINFO = 0x00021020,
			KLX_CHUNK_CAMERA				= 0x00022000,
			KLX_CHUNK_LIGHT					= 0x00023000,
				KLX_CHUNK_LIGHT_EXCLUDE		= 0x00023001,
			KLX_CHUNK_HELPER				= 0x00024000,
			KLX_CHUNK_LIGHTMAP_JPG			= 0x00030000,
			KLX_CHUNK_LIGHTMAP_TGA			= 0x00030001,
	};


	// ====================================
	// structs
	// ====================================

	typedef Array<struct VertexKLX> VertexArrayKLX;
	typedef Array<struct IndexBufferKLX> IndexBufferListKLX;
	typedef prcore::Bitmap LightmapKLX;


	struct WorldInfoKLX
	{
		float	start;
		float	end;
		float	mscale;
	};


	struct BitmapKLX
	{
		bool	enable;
		float	amount;
		String	name;
		String	mapname;
		uint8	maptype;
		uint8	mapchannel;
		uint32	flags;
	};


	struct FogInfoKLX
	{
		FogInfoKLX() { enable = false; }

		bool	enable;
		uint8	type;
		uint32	color;
		bool	fogbackground;

		float	fnear;
		float	ffar;
		float	top;
		float	bottom;
		float	density;

		uint8	fallofftype;

		bool	noise;
		float	noisescale;
		float	noiseangle;
		float	noisephase;
	};


	struct MaterialKLX
	{
		String		name;
		uint32		materialID;
		uint32		wirecolor;
		uint32		ambient;
		uint32		diffuse;
		uint32		specular;
		uint32		emissive;
		float		opacity;
		uint32		flags;
						
		BitmapKLX	maps[KLX_MAP_MAX];
						
		float		bounce;
		float		static_friction;
		float		sliding_friction;
	};


	struct BoneKLX
	{
		uint32	index;
		float	weight;
	};


	struct VertexKLX
	{
		vec3f	point;
		vec3f	normal;
		uint32	color;
		vec2f	texture[8];
		Array<BoneKLX> bones;
	};


	struct IndexBufferKLX
	{
		uint32	material;
		uint32	lightmap;
		uint32	type;
		Array<uint16> indexbuffer;
	};


	struct BaseTrackKLX
	{
		uint32	type;
		float	start;
		float	end;
		uint32	ort;
	};


	struct VectorTrackKLX : BaseTrackKLX
	{
		struct TrackInfo
		{
			float	time;
			vec3f	data;
			vec3f	intan;
			vec3f	outtan;
			float	easein;
			float	easeout;
		};
		Array<TrackInfo> info;
	};
	
	struct QuatTrackKLX : BaseTrackKLX
	{
		struct TrackInfo
		{
			float	time;
			quat4f	data;
			quat4f	intan;
			quat4f	outtan;
			float	easein;
			float	easeout;
		};
		Array<TrackInfo> info;
	};

	struct ScaleTrackKLX : BaseTrackKLX
	{
		struct TrackInfo
		{
			float	time;
			vec3f	data;
			vec3f	intan;
			vec3f	outtan;
			quat4f	dataaxis;
			quat4f	intanaxis;
			quat4f	outtanaxis;
			float	easein;
			float	easeout;
		};

		Array<TrackInfo> info;
	};


	struct ObjectKLX
	{
		String			name;
		uint32			nodeID;
		uint32			parentID;
		uint32			targetID;
		matrix4x4f		world;
		matrix4x4f		offset;
		VectorTrackKLX	position;
		QuatTrackKLX	rotation;
		ScaleTrackKLX	scale;
		uint32			inheritflags;
		String			userdata;
	};


	struct VADataKLX
	{
		vec3f	point;
		vec3f	normal;
	};


	struct VertexAnimationKLX
	{
		float				time;
		Array<VADataKLX>	vertices;
	
	};


	struct MeshKLX
	{
		uint32						nodeID;
		uint32						vertexformat;
		VertexArrayKLX				vertexbuffer;
		IndexBufferListKLX			indexbuffers;

		bool						isboundingvolumes;
		box3f						boundingbox;
		sphere3f					boundingsphere;

		Array<uint32>				bones;
		uint32						vertexanimationmode; // point | normal etc
		Array<VertexAnimationKLX>	vertexanimation;

		bool						castshadows;
		bool						receiveshadows;
	};


	struct CameraKLX
	{
		uint32		nodeID;
		float		near_range;
		float		far_range;
		float		fov;
		float		hither;
		float		yon;
		float		target_distance;
		uint32		flags;
	};


	struct LExcludeKLX
	{
		uint32			flags;
		Array<uint32>	nodeID;
	};


	struct LightKLX
	{
		uint32		nodeID;
		uint8		type;
		uint32		color;
		float		intensity;
		float		aspect;
		float		hotspot;
		float		falloff;
		float		nearAttenStart;
		float		nearAttenEnd;
		float		attenStart;
		float		attenEnd;
		uint32		flags;
		LExcludeKLX	exclude;
	};


	// ====================================
	// importer
	// ====================================

	class ImportKLX : public prcore::RefCount
	{
		public:

		WorldInfoKLX		info;
		FogInfoKLX			fog;
		Array<MaterialKLX>	materials;
		Array<LightmapKLX>	lightmaps;
		Array<ObjectKLX>	objects;
		Array<MeshKLX>		meshes;
		Array<CameraKLX>	cameras;
		Array<LightKLX>		lights;

		ImportKLX(prcore::Stream& stream);
		ImportKLX(const String& filename);
		~ImportKLX();

		ObjectKLX* GetObject(uint32 node);
	};

} // namespace primport


#endif