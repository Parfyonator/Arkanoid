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
		KLIMAX (.klx) reader

	revision history:
		Mar/20/2001 - Jukka Liimatta - initial revision
		Apr/18/2001 - Mikko Nurmi    - new features (animation etc.)
		May/02/2001 - Mikko Nurmi    - Vertex/TextureCoordinate flags
		Oct/27/2001 - Jukka Liimatta - ReadVertexBuffer() fix (KLX_VERTEX_COLOR fvf flag)
		Nov/02/2001 - Mikko Nurmi    - improved vertex animation
*/
#include <primport/klx.hpp>

using namespace prcore;
using namespace primport;

// =================================================
// reader declaration
// =================================================

class ReaderKLX
{
	public:

	ReaderKLX(ImportKLX& import, Stream& stream);
	~ReaderKLX();

	private:

	ImportKLX&		import;
	Stream&			stream;
	int				level;
	MaterialKLX*	curmaterial;
	ObjectKLX*		curobject;
	MeshKLX*		curmesh;
	LightKLX*		curlight;

	char*			ReadString();
	vec2f			ReadVector2();
	vec3f			ReadVector3();
	quat4f			ReadQuat();
	matrix4x4f		ReadMatrix();
	void			ReadTrackInfo(BaseTrackKLX& track);

	void			SwitchChunk(bool mainchunk);
	void			ChunkUnknown(uint32 length);
	void			ChunkMain(uint32 length);
	void			ChunkWorldInfo(uint32 length);
	void			ChunkFogInfo(uint32 length);
	void			ChunkMaterial(uint32 length);
	void			ChunkBitmap(uint32 length);
	void			ChunkDynamics(uint32 length);
	void			ChunkObject(uint32 length);
	void			ChunkPositionTrack(uint32 length);
	void			ChunkRotationTrack(uint32 length);
	void			ChunkScaleTrack(uint32 length);
	void			ChunkMesh(uint32 length);
	void			ChunkVertexBuffer(uint32 length);
	void			ChunkIndexBuffer(uint32 length);
	void			ChunkBoneWeights(uint32 length);
	void			ChunkVertexAnimation(uint32 length);
	void			ChunkBoundingVolumes(uint32 length);
	void			ChunkTrimeshExtraInfo(uint32 length);
	void			ChunkLightmapJPG(uint32 length);
	void			ChunkLightmapTGA(uint32 length);
	void			ChunkCamera(uint32 length);
	void			ChunkLight(uint32 length);
	void			ChunkLightExclude(uint32 length);
	void			ChunkHelper(uint32 length);
};


// =================================================
// reader implementation
// =================================================

ReaderKLX::ReaderKLX(ImportKLX& i, Stream& s)
: import(i), stream(s), level(0)
{
	// set default anim lengths and metric scale
	import.info.start  = 0;
	import.info.end    = 10;
	import.info.mscale = 1;

	// reset stream
	stream.Seek(0,Stream::START);

	// read stream
	SwitchChunk(true);
}


ReaderKLX::~ReaderKLX()
{
}


char* ReaderKLX::ReadString()
{
	static char buffer[256];

	char* ptr = buffer;
	for ( ;; )
	{
		char c;
		stream.Read(&c,1);
		*ptr++ = c;

		if ( !c )
			break;
	}

	return buffer;
}


vec2f ReaderKLX::ReadVector2()
{
	vec2f v;
	v.x = ReadLittleEndian<float>(stream);
	v.y = ReadLittleEndian<float>(stream);
	return v;
}


vec3f ReaderKLX::ReadVector3()
{
	vec3f v;
	v.x = ReadLittleEndian<float>(stream);
	v.y = ReadLittleEndian<float>(stream);
	v.z = ReadLittleEndian<float>(stream);
	return v;
}


quat4f ReaderKLX::ReadQuat()
{
	quat4f q;
	q.x = ReadLittleEndian<float>(stream);
	q.y = ReadLittleEndian<float>(stream);
	q.z = ReadLittleEndian<float>(stream);
	q.w = ReadLittleEndian<float>(stream);
	return q;
}


matrix4x4f ReaderKLX::ReadMatrix()
{
	matrix4x4f m;

	for ( int i=0; i<16; ++i )
		m.m16[i] = ReadLittleEndian<float>(stream);

	return m;
}


void ReaderKLX::SwitchChunk(bool mainchunk)
{
	++level;

	// read chunk header
	uint32 ChunkID  = ReadLittleEndian<uint32>(stream);
	uint32 ChunkLen = ReadLittleEndian<int32>(stream);

	if ( mainchunk && (ChunkID != KLX_CHUNK_MAIN) )
		PRCORE_EXCEPTION("Stream is not valid .klx");

	// invoke chunk reader
	int endoff = stream.GetOffset() + ChunkLen;
	switch ( ChunkID )
	{
		case KLX_CHUNK_MAIN:				ChunkMain(ChunkLen); break;
		case KLX_CHUNK_WORLDINFO:			ChunkWorldInfo(ChunkLen); break;
		case KLX_CHUNK_FOGINFO:				ChunkFogInfo(ChunkLen); break;
		case KLX_CHUNK_MATERIAL:			ChunkMaterial(ChunkLen); break;
		case KLX_CHUNK_BITMAP:				ChunkBitmap(ChunkLen); break;
		case KLX_CHUNK_DYNAMICS:			ChunkDynamics(ChunkLen); break;
		case KLX_CHUNK_OBJECT:				ChunkObject(ChunkLen); break;
		case KLX_CHUNK_POSITION_TRACK:		ChunkPositionTrack(ChunkLen); break;
		case KLX_CHUNK_ROTATION_TRACK:		ChunkRotationTrack(ChunkLen); break;
		case KLX_CHUNK_SCALE_TRACK:			ChunkScaleTrack(ChunkLen); break;
		case KLX_CHUNK_TRIMESH:				ChunkMesh(ChunkLen); break;
		case KLX_CHUNK_VERTEXBUFFER:		ChunkVertexBuffer(ChunkLen); break;
		case KLX_CHUNK_INDEXBUFFER:			ChunkIndexBuffer(ChunkLen); break;
		case KLX_CHUNK_BONEWEIGHTS:			ChunkBoneWeights(ChunkLen); break;
		case KLX_CHUNK_VERTEXANIMATION:		ChunkVertexAnimation(ChunkLen); break;
		case KLX_CHUNK_BOUNDINGVOLUMES:		ChunkBoundingVolumes(ChunkLen); break;
		case KLX_CHUNK_TRIMESH_EXTRAINFO:	ChunkTrimeshExtraInfo(ChunkLen); break;
		case KLX_CHUNK_LIGHTMAP_JPG:		ChunkLightmapJPG(ChunkLen); break;
		case KLX_CHUNK_LIGHTMAP_TGA:		ChunkLightmapTGA(ChunkLen); break;
		case KLX_CHUNK_CAMERA:				ChunkCamera(ChunkLen); break;
		case KLX_CHUNK_LIGHT:				ChunkLight(ChunkLen); break;
		case KLX_CHUNK_LIGHT_EXCLUDE:		ChunkLightExclude(ChunkLen); break;
		case KLX_CHUNK_HELPER:				ChunkHelper(ChunkLen); break;
		default:							ChunkUnknown(ChunkLen); break;
	}

	// recurse subchunks
	while ( stream.GetOffset() < endoff )
		SwitchChunk(false);

	stream.Seek(endoff,Stream::START);
	--level;
}


void ReaderKLX::ChunkUnknown(uint32 length)
{
	stream.Seek(length,Stream::CURRENT);
}


void ReaderKLX::ChunkMain(uint32 length)
{
}


void ReaderKLX::ChunkWorldInfo(uint32 length)
{
	import.info.start  = ReadLittleEndian<float>(stream);
	import.info.end    = ReadLittleEndian<float>(stream);
	import.info.mscale = ReadLittleEndian<float>(stream);
}


void ReaderKLX::ChunkFogInfo(uint32 length)
{
	import.fog.enable = true;

	import.fog.type          = ReadLittleEndian<uint8>(stream);
	import.fog.color         = ReadLittleEndian<uint32>(stream);
	import.fog.fogbackground = ReadLittleEndian<uint8>(stream) != 0;
	import.fog.fnear         = ReadLittleEndian<float>(stream);
	import.fog.ffar          = ReadLittleEndian<float>(stream);
	import.fog.top           = ReadLittleEndian<float>(stream);
	import.fog.bottom        = ReadLittleEndian<float>(stream);
	import.fog.density       = ReadLittleEndian<float>(stream);
	import.fog.fallofftype   = ReadLittleEndian<uint8>(stream);
	import.fog.noise         = ReadLittleEndian<uint8>(stream) != 0;
	import.fog.noisescale    = ReadLittleEndian<float>(stream);
	import.fog.noiseangle    = ReadLittleEndian<float>(stream);
	import.fog.noisephase    = ReadLittleEndian<float>(stream);
}


void ReaderKLX::ChunkMaterial(uint32 length)
{
	MaterialKLX material;

	material.name       = ReadString();
	material.materialID = ReadLittleEndian<uint32>(stream);
	material.wirecolor  = ReadLittleEndian<uint32>(stream);
	material.ambient    = ReadLittleEndian<uint32>(stream);
	material.diffuse    = ReadLittleEndian<uint32>(stream);
	material.specular   = ReadLittleEndian<uint32>(stream);
	material.emissive   = ReadLittleEndian<uint32>(stream);
	material.opacity    = ReadLittleEndian<float>(stream);
	material.flags      = ReadLittleEndian<uint32>(stream);

	for ( int i=0; i<KLX_MAP_MAX; ++i )
		material.maps[i].enable = false;

	material.bounce           = 1.0f;
	material.static_friction  = 0.5f;
	material.sliding_friction = 0.4f;

	curmaterial = &import.materials.PushBack(material);
}


void ReaderKLX::ChunkBitmap(uint32 length)
{
	uint8 type        = ReadLittleEndian<uint8>(stream);
	BitmapKLX& bitmap = curmaterial->maps[type];

	bitmap.enable     = true;
	bitmap.amount     = ReadLittleEndian<float>(stream);
	bitmap.name       = ReadString();
	bitmap.mapname    = ReadString();
	bitmap.maptype    = ReadLittleEndian<uint8>(stream);
	bitmap.mapchannel = ReadLittleEndian<uint8>(stream);
	bitmap.flags      = ReadLittleEndian<uint32>(stream);

	if ( !bitmap.mapname.GetLength() ) 
		bitmap.enable = false;
}


void ReaderKLX::ChunkDynamics(uint32 length)
{
	MaterialKLX& material = *curmaterial;

	material.bounce           = ReadLittleEndian<float>(stream);
	material.static_friction  = ReadLittleEndian<float>(stream);
	material.sliding_friction = ReadLittleEndian<float>(stream);
}


void ReaderKLX::ChunkObject(uint32 length)
{
	ObjectKLX object;

	object.name     = ReadString();
	object.nodeID   = ReadLittleEndian<uint32>(stream);
	object.parentID = ReadLittleEndian<uint32>(stream);
	object.targetID = ReadLittleEndian<uint32>(stream);
	object.world    = ReadMatrix();
	object.offset   = ReadMatrix();

	// initial transforms, used if no track found

	VectorTrackKLX::TrackInfo& poskey = object.position.info.PushBack();
	poskey.time = 0;
	poskey.data = ReadVector3();

	QuatTrackKLX::TrackInfo& rotkey = object.rotation.info.PushBack();
	rotkey.time = 0;
	rotkey.data = ReadQuat();

	ScaleTrackKLX::TrackInfo& scalekey = object.scale.info.PushBack();
	scalekey.time     = 0;
	scalekey.data     = ReadVector3();
	scalekey.dataaxis = ReadQuat();

	object.inheritflags = ReadLittleEndian<uint32>(stream);
	object.userdata     = ReadString();

	curobject = &import.objects.PushBack(object);
}


void ReaderKLX::ReadTrackInfo(BaseTrackKLX& track)
{
	track.type  = ReadLittleEndian<uint8>(stream);
	track.start = ReadLittleEndian<float>(stream);
	track.end   = ReadLittleEndian<float>(stream);
	track.ort   = ReadLittleEndian<uint32>(stream);
}


void ReaderKLX::ChunkPositionTrack(uint32 length)
{
	ReadTrackInfo(curobject->position);
	
	int count = static_cast<int>(ReadLittleEndian<uint32>(stream));
	curobject->position.info.SetSize(count);

	for ( int i=0; i<count; ++i )
	{
		VectorTrackKLX::TrackInfo& info = curobject->position.info[i];

		info.time = ReadLittleEndian<float>(stream);
		info.data = ReadVector3();

		if ( curobject->position.type == KLX_TRACK_HERMITE )
		{
			info.intan   = ReadVector3();
			info.outtan  = ReadVector3();
			info.easein  = ReadLittleEndian<float>(stream);
			info.easeout = ReadLittleEndian<float>(stream);
		}
	}
}


void ReaderKLX::ChunkRotationTrack(uint32 length)
{
	ReadTrackInfo(curobject->rotation);

	int count = static_cast<int>(ReadLittleEndian<uint32>(stream));
	curobject->rotation.info.SetSize(count);

	for ( int i=0; i<count; ++i )
	{
		QuatTrackKLX::TrackInfo& info = curobject->rotation.info[i];

		info.time = ReadLittleEndian<float>(stream);
		info.data = ReadQuat();

		if ( curobject->rotation.type == KLX_TRACK_HERMITE )
		{
			info.intan   = ReadQuat();
			info.outtan  = ReadQuat();
			info.easein  = ReadLittleEndian<float>(stream);
			info.easeout = ReadLittleEndian<float>(stream);
		}
	}
}


void ReaderKLX::ChunkScaleTrack(uint32 length)
{
	ReadTrackInfo(curobject->scale);

	int count = static_cast<int>(ReadLittleEndian<uint32>(stream));
	curobject->scale.info.SetSize(count);

	for ( int i=0; i<count; ++i )
	{
		ScaleTrackKLX::TrackInfo& info = curobject->scale.info[i];

		info.time     = ReadLittleEndian<float>(stream);
		info.data     = ReadVector3();
		info.dataaxis = ReadQuat();

		if ( curobject->scale.type == KLX_TRACK_HERMITE )
		{
			info.intan      = ReadVector3();
			info.outtan     = ReadVector3();
			info.intanaxis  = ReadQuat();
			info.outtanaxis = ReadQuat();
			info.easein     = ReadLittleEndian<float>(stream);
			info.easeout    = ReadLittleEndian<float>(stream);
		}
	}
}


void ReaderKLX::ChunkMesh(uint32 length)
{
	// recursion
	curmesh = &import.meshes.PushBack();

	curmesh->nodeID					= ReadLittleEndian<uint32>(stream);
	curmesh->vertexanimationmode	= 0;
	curmesh->isboundingvolumes		= false;
	curmesh->boundingbox.vmin		= vec3f(0,0,0);
	curmesh->boundingbox.vmax		= vec3f(0,0,0);
	curmesh->boundingsphere.center	= vec3f(0,0,0);
	curmesh->boundingsphere.radius	= 0;
	curmesh->castshadows			= true;
	curmesh->receiveshadows			= true;
}


void ReaderKLX::ChunkVertexBuffer(uint32 length)
{
	uint32 fvf = ReadLittleEndian<uint32>(stream);
	uint32 numvertex = ReadLittleEndian<uint32>(stream);

	curmesh->vertexformat = fvf;
	curmesh->vertexbuffer.SetSize(numvertex);

	uint8 texcount = (fvf & KLX_VERTEX_TEXTURE_MASK) >> KLX_VERTEX_TEXTURE_SHIFT;

	for ( uint32 j=0; j<numvertex; ++j )
	{
		VertexKLX& vertex = curmesh->vertexbuffer[j];

		vertex.point  = (fvf & KLX_VERTEX_POINT) ? ReadVector3() : vec3f(0,0,0);
		vertex.normal = (fvf & KLX_VERTEX_NORMAL) ? ReadVector3() : vec3f(0,0,0);
		vertex.color  = (fvf & KLX_VERTEX_COLOR) ? ReadLittleEndian<uint32>(stream) : 0xffffffff;

		if ( fvf & KLX_VERTEX_TEXTURE_MASK )	
		{
			for ( uint8 i=0; i<texcount; ++i )
			{
				vertex.texture[i] = ReadVector2();
			}
		}
	}
}


void ReaderKLX::ChunkIndexBuffer(uint32 length)
{
	IndexBufferKLX& buffer = curmesh->indexbuffers.PushBack();

	buffer.material = ReadLittleEndian<uint32>(stream);
	buffer.lightmap = ReadLittleEndian<uint32>(stream);
	buffer.type     = ReadLittleEndian<uint32>(stream);
	uint32 numindex = ReadLittleEndian<uint32>(stream);

	Array<uint16>& indices = buffer.indexbuffer;
	indices.SetSize(numindex);

	for ( uint32 j=0; j<numindex; ++j )
	{
		indices[j] = ReadLittleEndian<uint16>(stream);
	}
}


void ReaderKLX::ChunkBoneWeights(uint32 length)
{
	// read nodebones
	uint32 numbones = ReadLittleEndian<uint32>(stream);

	uint32 i = 0;
	for ( i=0; i<numbones; ++i )
	{
		uint32 nodeid = ReadLittleEndian<uint32>(stream);
		curmesh->bones.PushBack(nodeid);
	}

	uint32 numvertex = ReadLittleEndian<uint32>(stream);
	if ( curmesh->vertexbuffer.IsEmpty() )
	{
		curmesh->vertexbuffer.SetSize(numvertex);
	}

	for ( i=0; i<numvertex; ++i )
	{
		VertexKLX& vertex = curmesh->vertexbuffer[i];

		uint32 count = ReadLittleEndian<uint32>(stream);
		for ( uint32 j=0; j<count; ++j )
		{
			BoneKLX& bone = vertex.bones.PushBack();

			bone.index  = ReadLittleEndian<uint32>(stream);
			bone.weight = ReadLittleEndian<float>(stream);
		}
	}
}


void ReaderKLX::ChunkVertexAnimation(uint32 length)
{
	// which stuff we want to interpolate
	uint32 mode = ReadLittleEndian<uint32>(stream);
	curmesh->vertexanimationmode = mode;

	// number of vertices
	int numvertex = curmesh->vertexbuffer.GetSize();

	// number of animation frames
	int count = ReadLittleEndian<int>(stream);
	curmesh->vertexanimation.SetSize(count);
	for ( int j=0; j<count; ++j )
	{
		curmesh->vertexanimation[j].time = ReadLittleEndian<float>(stream);

		// vertices in this animation frame
		curmesh->vertexanimation[j].vertices.SetSize(numvertex);
		memset(curmesh->vertexanimation[j].vertices.GetArray(),0,sizeof(VADataKLX)*numvertex);
		for ( int i=0; i<numvertex; ++i )
		{
			// read desired data from fileformat
			VADataKLX& vadata = curmesh->vertexanimation[j].vertices[i];

			if ( mode & KLX_VERTEXANIMATION_POINT )
				vadata.point = ReadVector3();

			if ( mode & KLX_VERTEXANIMATION_NORMAL )
				vadata.normal = ReadVector3();
		}
	}
}


void ReaderKLX::ChunkBoundingVolumes(uint32 length)
{
	curmesh->isboundingvolumes     = true;
	curmesh->boundingbox.vmin      = ReadVector3();
	curmesh->boundingbox.vmax      = ReadVector3();
	curmesh->boundingsphere.center = ReadVector3();
	curmesh->boundingsphere.radius = ReadLittleEndian<float>(stream);
}


void ReaderKLX::ChunkTrimeshExtraInfo(uint32 length)
{
	curmesh->castshadows    = ReadLittleEndian<uint8>(stream) ? 1 : 0;
	curmesh->receiveshadows = ReadLittleEndian<uint8>(stream) ? 1 : 0;
	ReadLittleEndian<uint32>(stream);
	ReadLittleEndian<uint32>(stream);
	ReadLittleEndian<uint32>(stream);
	ReadLittleEndian<uint32>(stream);
}


void ReaderKLX::ChunkLightmapJPG(uint32 length)
{
	const BitmapCodec* codec = BitmapCodecManager::manager.FindInterface("jpg");
	if ( codec )
	{
		Bitmap& map = import.lightmaps.PushBack();

		MemoryStream buffer(length);
		uint8* data = buffer.GetData();
		stream.Read(data,length);

		codec->decode(map,buffer);
	}
	else
	{
		ChunkUnknown(length);
	}
}


void ReaderKLX::ChunkLightmapTGA(uint32 length)
{
	const BitmapCodec* codec = BitmapCodecManager::manager.FindInterface("tga");
	if ( codec )
	{
		Bitmap& map = import.lightmaps.PushBack();

		MemoryStream buffer(length);
		uint8* data = buffer.GetData();
		stream.Read(data,length);

		codec->decode(map,buffer);
	}
	else
	{
		ChunkUnknown(length);
	}
}


void ReaderKLX::ChunkCamera(uint32 length)
{
	CameraKLX camera;

	camera.near_range      = ReadLittleEndian<float>(stream);
	camera.far_range       = ReadLittleEndian<float>(stream);
	camera.fov             = ReadLittleEndian<float>(stream);
	camera.hither          = ReadLittleEndian<float>(stream);
	camera.yon             = ReadLittleEndian<float>(stream);
	camera.target_distance = ReadLittleEndian<float>(stream);
	camera.flags           = ReadLittleEndian<uint32>(stream);

	camera.nodeID = curobject->nodeID;
	import.cameras.PushBack(camera);
}


void ReaderKLX::ChunkLight(uint32 length)
{
	LightKLX light;

	light.type           = ReadLittleEndian<uint8>(stream);
	light.color          = ReadLittleEndian<uint32>(stream);
	light.intensity      = ReadLittleEndian<float>(stream);
	light.aspect         = ReadLittleEndian<float>(stream);
	light.hotspot        = ReadLittleEndian<float>(stream);
	light.falloff        = ReadLittleEndian<float>(stream);
	light.nearAttenStart = ReadLittleEndian<float>(stream);
	light.nearAttenEnd   = ReadLittleEndian<float>(stream);
	light.attenStart     = ReadLittleEndian<float>(stream);
	light.attenEnd       = ReadLittleEndian<float>(stream);
	light.flags          = ReadLittleEndian<uint32>(stream);

	light.nodeID = curobject->nodeID;

	light.exclude.flags = 0;
	curlight = &import.lights.PushBack(light);
}


void ReaderKLX::ChunkLightExclude(uint32 length)
{
	uint32 count = ReadLittleEndian<uint32>(stream);
	curlight->exclude.flags = ReadLittleEndian<uint32>(stream);
	for ( uint32 i=0; i<count; ++i )
	{
		uint32 v = ReadLittleEndian<uint32>(stream);
		curlight->exclude.nodeID.PushBack(v);
	}
}

void ReaderKLX::ChunkHelper(uint32 length)
{
}


// =================================================
// importer
// =================================================

ImportKLX::ImportKLX(Stream& stream)
{
	ReaderKLX reader(*this,stream);
}


ImportKLX::ImportKLX(const String& filename)
{
	FileStream stream(filename);
	ReaderKLX reader(*this,stream);
}


ImportKLX::~ImportKLX()
{
}


ObjectKLX* ImportKLX::GetObject(uint32 node)
{
	for ( int i=0; i<objects.GetSize(); ++i )
	{
		ObjectKLX& object = objects[i];
		if ( object.nodeID == node )
			return &object;
	}
	return NULL;
}
