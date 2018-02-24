/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_Q3BSP_HPP
#define PRIMPORT_Q3BSP_HPP


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

	struct TextureQ3
	{
		String	name;
		uint32	flags;
		int		contents;
	};

	struct NodeQ3
	{
		int		planeindex;
		int		children[2];
		box3f	boundingbox;
	};

	struct LeafQ3
	{
		int		cluster;
		int		area;
		box3f	boundingbox;
		int		leaffaceindex;
		int		numleaffaces;
		int		leafbrushindex;
		int		numleafbrushes;
	};

	struct LeafFaceQ3
	{
		int faceindex;
	};

	struct LeafBrushQ3
	{
		int brushindex;
	};

	struct ModelQ3
	{
		box3f	boundingbox;
		int		faceindex;
		int		numfaces;
		int		brushindex;
		int		numbrushes;
	};

	struct BrushQ3
	{
		int	brushsideindex;
		int	numbrushsides;
		int	textureindex;
	};

	struct BrushSideQ3
	{
		int	planeindex;
		int	textureindex;
	};

	struct VertexQ3
	{
		point3f		position;
		point2f		texcoords;
		point2f		lightmapcoords;
		vec3f		normal;
		Color32		color;
	};

	struct MeshVertexQ3
	{
		int offset;
	};

	struct EffectQ3
	{
		String	name;
		int		brushindex;
		int		unknown;
	};

	enum
	{
		Q3FACETYPE_POLYGON		= 1,
		Q3FACETYPE_PATCH		= 2,
		Q3FACETYPE_MESH			= 3,
		Q3FACETYPE_BILLBOARD	= 4
	};

	struct FaceQ3
	{
		int			textureindex;
		int			effectindex;
		int			type;
		int			vertexindex;
		int			numvertices;
		int			meshvertexindex;
		int			nummeshvertices;
		int			lightmapindex;
		int			lightmapcorner[2];
		int			lightmapsize[2];
		point3f		lightmapworldspaceorigin;
		vec3f		lightmapworldspaces;
		vec3f		lightmapworldspacet;
		vec3f		normal;
		int			patchsize[2];
	};

	struct LightmapQ3
	{
		uint8 map[128*128*3];
	};

	struct LightvolQ3
	{
		Color32	ambientcolor;
		Color32	directionalcolor;
		uint8	phi;
		uint8	theta;
	};

	struct VisDataQ3
	{
		int		numvectors;
		int		vectorsize;
		uint8*	data;
	};


	// ====================================
	// importer
	// ====================================

	class ImportQ3BSP : public prcore::RefCount
	{
		public:
		
		String				entitydesc;
		Array<TextureQ3>	textures;
		Array<plane3f>		planes;
		Array<NodeQ3>		nodes;
		Array<LeafQ3>		leafs;
		Array<LeafFaceQ3>	leaffaces;
		Array<LeafBrushQ3>	leafbrushes;
		Array<ModelQ3>		models;
		Array<BrushQ3>		brushes;
		Array<BrushSideQ3>	brushsides;
		Array<VertexQ3>		vertices;
		Array<MeshVertexQ3>	meshvertices;
		Array<EffectQ3>		effects;
		Array<FaceQ3>		faces;
		Array<LightmapQ3>	lightmaps;
		Array<LightvolQ3>	lightvolumes;
		VisDataQ3			visdata;

		ImportQ3BSP();
		ImportQ3BSP(prcore::Stream& stream);
		ImportQ3BSP(const String& filename);
		~ImportQ3BSP();
	};

} // namespace primport


#endif