/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRIMPORT_BVH_HPP
#define PRIMPORT_BVH_HPP


#include <prmath/prmath.hpp>
#include <prcore/prcore.hpp>


namespace primport
{

	using prcore::Array;
	using prcore::String;

	// ====================================
	// structs
	// ====================================

	enum TypeBVH
	{
		BVH_ROOT,		// root bone, should only be one per bvh file
		BVH_JOINT,		// joint bone, all normal bones are of this type
		BVH_ENDSITE		// special "endsite" bone, terminates hierarchy
	};

	struct BoneBVH
	{
		String				name;		// name
		TypeBVH				type;		// type
		uint16				parent;		// parent index, 0xffff - no parent
		Array<matrix4x4f>	animation;	// animation frames
	};

	// ====================================
	// importer
	// ====================================

	class ImportBVH : public prcore::RefCount
	{
		public:

		float			fps;		// sampling rate of animation
		int				numframe;	// number of animation frames
		Array<BoneBVH>	bones;		// bone hierarchy (see parent field in bone)

		ImportBVH(prcore::Stream& stream);
		ImportBVH(const String& filename);
		~ImportBVH();
	};

} // namespace primport


#endif