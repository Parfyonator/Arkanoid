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
		Biovision motion capture (.bvh) reader

	revision history:
		Jun/01/2002 - Jukka Liimatta - initial version
*/
#include <primport/bvh.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// .bvh parser
// =================================================

// bvh context

enum bvhtype
{
	bvh_root,
	bvh_joint,
	bvh_endsite
};

enum bvhchan
{
	bvh_xposition,
	bvh_yposition,
	bvh_zposition,
	bvh_xrotation,
	bvh_yrotation,
	bvh_zrotation
};

struct bvhchannel
{
	bvhchan type;
	prcore::Array<float> keys;
};

struct bvhnode
{
	int depth;
	bvhtype type;
	uint16 parent;
	point3f offset;
	prcore::String name;
	prcore::Array<bvhchannel> channelv;

	prcore::Array<matrix4x4f> animation;
	matrix4x4f tm;

	matrix4x4f gettm(int index)
	{
		matrix4x4f tm(1);
		for ( int i=channelv.GetSize()-1; i>=0; --i )
		{
			bvhchannel& channel = channelv[i];

			float v = channel.keys[index];
			float a = v * prmath::pi / 180; // degrees to radians

			switch ( channel.type )
			{
				case bvh_xposition: tm.MultTranslate(v,0,0); break;
				case bvh_yposition: tm.MultTranslate(0,v,0); break;
				case bvh_zposition: tm.MultTranslate(0,0,v); break;
				case bvh_xrotation: tm.MultRotateX(a); break;
				case bvh_yrotation: tm.MultRotateY(a); break;
				case bvh_zrotation: tm.MultRotateZ(a); break;
			}
		}
		tm.MultTranslate(offset);
		return tm;
	}
};

struct bvhcontext
{
	int depth;
	int numframe;
	float fps;
	prcore::Array<bvhnode> nodev;

	bvhnode& get_curnode()
	{
		int size = nodev.GetSize() - 1;
		return nodev[size];
	}

	uint16 get_parent()
	{
		int size = nodev.GetSize() - 1;
		for ( int i=size; i>=0; --i )
		{
			if ( (nodev[i].depth + 1) == depth )
				return (uint16)i;
		}
		return 0xffff;
	}
};

// functions

static inline char* bvhtok(char* buffer = NULL)
{
	const char v[] = { ' ', '\n', 9, 13, 0 };
	return strtok(buffer,v);
}

static void func_open(bvhcontext& ctx, char*& buffer)
{
	++ctx.depth;
}

static void func_close(bvhcontext& ctx, char*& buffer)
{
	--ctx.depth;
}

static void func_endsite(bvhcontext& ctx, char*& buffer)
{
	char* name = bvhtok();

	bvhnode& node = ctx.nodev.PushBack();

	node.depth  = ctx.depth;
	node.type   = bvh_endsite;
	node.parent = ctx.get_parent();
	node.name   = name;
}

static void func_joint(bvhcontext& ctx, char*& buffer)
{
	func_endsite(ctx,buffer);

	bvhnode& node = ctx.get_curnode();
	node.type = bvh_joint;
}

static void func_root(bvhcontext& ctx, char*& buffer)
{
	func_joint(ctx,buffer);

	bvhnode& node = ctx.get_curnode();
	node.type = bvh_root;
}

static void func_offset(bvhcontext& ctx, char*& buffer)
{
	float x = static_cast<float>(atof(bvhtok()));
	float y = static_cast<float>(atof(bvhtok()));
	float z = static_cast<float>(atof(bvhtok()));

	bvhnode& node = ctx.get_curnode();
	node.offset.x = x;
	node.offset.y = y;
	node.offset.z = z;
}

static void func_channels(bvhcontext& ctx, char*& buffer)
{
	bvhnode& node = ctx.get_curnode();

	int numchan = atoi(bvhtok());
	for ( int i=0; i<numchan; ++i )
	{
		char* token = bvhtok();

		bvhchannel& channel = node.channelv.PushBack();

			 if ( !strcmp(token,"Xrotation") )	channel.type = bvh_xrotation;
		else if ( !strcmp(token,"Yrotation") )	channel.type = bvh_yrotation;
		else if ( !strcmp(token,"Zrotation") )	channel.type = bvh_zrotation;
		else if ( !strcmp(token,"Xposition") )	channel.type = bvh_xposition;
		else if ( !strcmp(token,"Yposition") )	channel.type = bvh_yposition;
		else if ( !strcmp(token,"Zposition") )	channel.type = bvh_zposition;
		else PRCORE_EXCEPTION("unknown channel type.");
	}
}

static void func_motion(bvhcontext& ctx, char*& buffer)
{
	typedef prcore::Array<float> FloatArray;
	prcore::Array<FloatArray*> channelv;

	int i,j;
	for ( i=0; i<ctx.nodev.GetSize(); ++i )
	{
		bvhnode& node = ctx.nodev[i];
		for ( j=0; j<node.channelv.GetSize(); ++j )
		{
			bvhchannel& ncv = node.channelv[j];
			FloatArray& fv = ncv.keys;
			channelv.PushBack(&fv);
		}
	}

	bvhtok();
	ctx.numframe = atoi(bvhtok());

	bvhtok();
	bvhtok();
	ctx.fps = 1 / static_cast<float>(atof(bvhtok()));

	int numchannel = channelv.GetSize();

	for ( i=0; i<ctx.numframe; ++i )
	{
		for ( j=0; j<numchannel; ++j )
		{
			char* token = bvhtok();
			if ( token == NULL )
				PRCORE_EXCEPTION("Unexpected end of bvh stream.");

			float v = static_cast<float>(atof(token));
			channelv[j]->PushBack(v);
		}
	}
}

// function table

struct bvhchunk
{
	const char*	name;
	void (*func)(bvhcontext& ctx, char*& buffer);
};

static bvhchunk chunkv[] =
{
	{	"{",			func_open		},
	{	"}",			func_close		},
	{	"End",			func_endsite	},
	{	"JOINT",		func_joint		},
	{	"ROOT",			func_root		},
	{	"OFFSET",		func_offset		},
	{	"CHANNELS",		func_channels	},
	{	"MOTION",		func_motion		},
	{	NULL,			NULL			}
};

// main parser

static void bvhparser(ImportBVH& import, MemoryStream& stream)
{
	bvhcontext context;
	int i,j;

	char* buffer = reinterpret_cast<char*>(stream.GetData());
	int size = stream.GetSize();
	if ( !buffer || size < 1 )
		return;

	// prepare buffer for tokenizing
	buffer[size-1] = 0;

	// establish connection
	char* token = bvhtok(buffer);
	if ( strcmp(token,"HIERARCHY") )
		PRCORE_EXCEPTION("Incompatible bvh stream.");

	context.depth = 0;
	for ( ;; )
	{
		// read next token
		token = bvhtok();
		if ( token == NULL )
			break;

		// find token function
		for ( bvhchunk* node=chunkv; node->func; ++node )
		{
			if ( !strcmp(token,node->name) )
			{
				// call token function
				node->func(context,buffer);
				break;
			}
		}
	}

	for ( i=0; i<context.nodev.GetSize(); ++i )
	{
		bvhnode& node = context.nodev[i];

		node.animation.SetSize(context.numframe);
		for ( j=0; j<context.numframe; ++j )
		{
			node.animation[j] = node.gettm(j);
		}

		for ( j=0; j<context.numframe; ++j )
		{
			if ( node.parent != 0xffff )
			{
				bvhnode& parent = context.nodev[node.parent];
				node.animation[j] *= parent.animation[j];
			}
		}
	}

	// write results into import object
	import.fps = context.fps;
	import.numframe = context.numframe;

	int numbone = context.nodev.GetSize();
	import.bones.SetSize(numbone);

	for ( i=0; i<numbone; ++i )
	{
		bvhnode& node = context.nodev[i];
		BoneBVH& bone = import.bones[i];

		bone.name      = node.name;
		bone.type      = (TypeBVH)node.type;
		bone.parent    = node.parent;
		bone.animation = node.animation;
	}
}


// =================================================
// importer
// =================================================

ImportBVH::ImportBVH(Stream& stream)
{
	int size = stream.GetSize();
	MemoryStream memstr(size);
	stream.Read(memstr.GetData(),size);

	bvhparser(*this,memstr);
}


ImportBVH::ImportBVH(const String& filename)
{
	MemoryStream memstr(filename);

	bvhparser(*this,memstr);
}


ImportBVH::~ImportBVH()
{
}
