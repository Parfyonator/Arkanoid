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
		Autodesk 3D Studio R4 (.3ds) reader

	revision history:
		Jul/14/1999 - Mikael Kalms - initial revision
		May/04/2000 - Jukka Liimatta - revised data structures
		Feb/10/2001 - Jukka Liimatta - renaissance build

	TODO:
		- smoothing groups in vertex normal calculation
*/
#include <primport/3ds.hpp>

using namespace prcore;
using namespace primport;


// =================================================
// reader declaration
// =================================================

class Reader3DS
{
	public:

	Reader3DS(Import3DS& import, Stream& stream);
	~Reader3DS();

	private:

	struct State
	{
		float		percent;
		Color32		color;
		float		inner;
		float		outer;
		String		name;
		uint16		index;
		Texture3DS	texture;
		Material3DS	material;
		TriMesh3DS	trimesh;
		Camera3DS	camera;
		Light3DS	light;
		Node3DS*	node;
	};

	Import3DS&		import;
	Stream&			stream;
	State			state;
	int				level;

	void			RemapVector3(vec3f& v);
	void			RemapMatrix(matrix4x4f& m);
	void			RemapQuaternion(quat4f& q);
	vec3f			ReadVector3();
	void			ReadTexture(Texture3DS& texture, int32 ChunkLen);
	void			ReadKeyHeader(KeyHeader3DS& header);
	char*			ReadAscii();
	int				GetMaterialIndex(const char* name);
	void			InitializeMaterial(Material3DS& material);
	void			InitializeTexture(Texture3DS& texture);
	void			InitializeTriMesh(TriMesh3DS& trimesh);
	void			InitializeCamera(Camera3DS& camera);
	void			InitializeLight(Light3DS& light);
	void			InitializeNode(Node3DS& node);
	void			InitializeKeyHeader(KeyHeader3DS& header);
	void			CreateTriMeshNormals(TriMesh3DS& mesh);
	
	void			RecurseChunk(int32 ChunkLen);
	void			SwitchChunk();

	void			ChunkMain3DS(int32 ChunkLen);
	void			ChunkVersion(int32 ChunkLen);
	void			ChunkEditor(int32 ChunkLen);
	void			ChunkMeshVersion(int32 ChunkLen);
	void			ChunkColorFloat(int32 ChunkLen);
	void			ChunkColor24(int32 ChunkLen);
	void			ChunkIntPercent(int32 ChunkLen);
	void			ChunkFloatPercent(int32 ChunkLen);
	void			ChunkMasterScale(int32 ChunkLen);
	void			ChunkInnerRange(int32 ChunkLen);
	void			ChunkOuterRange(int32 ChunkLen);
	void			ChunkViewport(int32 ChunkLen);
	void			ChunkMaterial(int32 ChunkLen);
	void			ChunkMatName(int32 ChunkLen);
	void			ChunkMatAmbientColor(int32 ChunkLen);
	void			ChunkMatDiffuseColor(int32 ChunkLen);
	void			ChunkMatShininess(int32 ChunkLen);
	void			ChunkMatShin2(int32 ChunkLen);
	void			ChunkMatSpecularColor(int32 ChunkLen);
	void			ChunkMatTransparency(int32 ChunkLen);
	void			ChunkMatXPFall(int32 ChunkLen);
	void			ChunkMatRefBlur(int32 ChunkLen);
	void			ChunkMatTwosided(int32 ChunkLen);
	void			ChunkMatAdditive(int32 ChunkLen);
	void			ChunkMatSelfIllumPCT(int32 ChunkLen);
	void			ChunkMatWire(int32 ChunkLen);
	void			ChunkMatWireSize(int32 ChunkLen);
	void			ChunkMatShade(int32 ChunkLen);
	void			ChunkMatTextureMap1(int32 ChunkLen);
	void			ChunkMatTextureMap2(int32 ChunkLen);
	void			ChunkMatOpacityMap(int32 ChunkLen);
	void			ChunkMatBumpMap(int32 ChunkLen);
	void			ChunkMatSpecularMap(int32 ChunkLen);
	void			ChunkMatShininessMap(int32 ChunkLen);
	void			ChunkMatSelfIllumMap(int32 ChunkLen);
	void			ChunkMatReflectionMap(int32 ChunkLen);
	void			ChunkMatMapName(int32 ChunkLen);
	void			ChunkMatMapTiling(int32 ChunkLen);
	void			ChunkMatMapBlur(int32 ChunkLen);
	void			ChunkMatMapUScale(int32 ChunkLen);
	void			ChunkMatMapVScale(int32 ChunkLen);
	void			ChunkMatMapUOffset(int32 ChunkLen);
	void			ChunkMatMapVOffset(int32 ChunkLen);
	void			ChunkMatMapAngle(int32 ChunkLen);
	void			ChunkNamedObject(int32 ChunkLen);
	void			ChunkMesh(int32 ChunkLen);
	void			ChunkMeshVertexList(int32 ChunkLen);
	void			ChunkMeshFlagList(int32 ChunkLen);
	void			ChunkMeshFaceList(int32 ChunkLen);
	void			ChunkMeshMaterialList(int32 ChunkLen);
	void			ChunkMeshMappingList(int32 ChunkLen);
	void			ChunkMeshSmoothingList(int32 ChunkLen);
	void			ChunkMeshLocalAxis(int32 ChunkLen);
	void			ChunkMeshColor(int32 ChunkLen);
	void			ChunkCamera(int32 ChunkLen);
	void			ChunkCameraRange(int32 ChunkLen);
	void			ChunkLight(int32 ChunkLen);
	void			ChunkLightSpot(int32 ChunkLen);
	void			ChunkLightOff(int32 ChunkLen);
	void			ChunkLightSpotRoll(int32 ChunkLen);
	void			ChunkLightBrightness(int32 ChunkLen);
	void			ChunkKeyframer(int32 ChunkLen);
	void			ChunkKeyAmbient(int32 ChunkLen);
	void			ChunkKeyMesh(int32 ChunkLen);
	void			ChunkKeyCamera(int32 ChunkLen);
	void			ChunkKeyTarget(int32 ChunkLen);
	void			ChunkKeyLight(int32 ChunkLen);
	void			ChunkKeyLightTarget(int32 ChunkLen);
	void			ChunkKeySpotLight(int32 ChunkLen);
	void			ChunkKeyHeader(int32 ChunkLen);
	void			ChunkKeyCurrentTime(int32 ChunkLen);
	void			ChunkKeySegment(int32 ChunkLen);
	void			ChunkKeyNodeHeader(int32 ChunkLen);
	void			ChunkKeyDummyObject(int32 ChunkLen);
	void			ChunkKeyPivot(int32 ChunkLen);
	void			ChunkKeyBoundBox(int32 ChunkLen);
	void			ChunkKeyMorphSmooth(int32 ChunkLen);
	void			ChunkKeyPosTrack(int32 ChunkLen);
	void			ChunkKeyRotTrack(int32 ChunkLen);
	void			ChunkKeyScaleTrack(int32 ChunkLen);
	void			ChunkKeyFovTrack(int32 ChunkLen);
	void			ChunkKeyRollTrack(int32 ChunkLen);
	void			ChunkKeyColorTrack(int32 ChunkLen);
	void			ChunkKeyMorphTrack(int32 ChunkLen);
	void			ChunkKeyNodeID(int32 ChunkLen);
};


// =================================================
// reader implementation
// =================================================

Reader3DS::Reader3DS(Import3DS& i, Stream& s)
: import(i),stream(s),level(-1)
{
	// reset stream
	stream.Seek(0,Stream::START);

	// initialize state
	state.index = 0;
	state.node = NULL;

	// chunk reader
	SwitchChunk();
}


Reader3DS::~Reader3DS()
{
}


// =================================================
// chunk reader
// =================================================

void Reader3DS::SwitchChunk()
{
	++level;

	// read chunk header
	uint16 ChunkID = ReadLittleEndian<uint16>(stream);
	int32 ChunkLen = ReadLittleEndian<int32>(stream) - 6;

//	for ( int space=0; space<level; space++ ) cout << "  ";
//	cout << (hex) << "chunk: 0x" << ChunkID << (dec) << ", length: " << ChunkLen << endl;

	// invoke chunk reader
	int endoff = stream.GetOffset() + ChunkLen;
	switch ( ChunkID )
	{
		case 0x4d4d:	ChunkMain3DS(ChunkLen); break;
		case 0x0002:	ChunkVersion(ChunkLen); break;
		case 0x3d3d:	ChunkEditor(ChunkLen); break;
		case 0x3d3e:	ChunkMeshVersion(ChunkLen); break;
		case 0x0010:	ChunkColorFloat(ChunkLen); break;
		case 0x0011:	ChunkColor24(ChunkLen); break;
		case 0x0012:	ChunkColor24(ChunkLen); break;
		case 0x0013:	ChunkColorFloat(ChunkLen); break;
		case 0x0030:	ChunkIntPercent(ChunkLen); break;
		case 0x0031:	ChunkFloatPercent(ChunkLen); break;
		case 0x0100:	ChunkMasterScale(ChunkLen); break;
		case 0x4659:	ChunkInnerRange(ChunkLen); break;
		case 0x465a:	ChunkOuterRange(ChunkLen); break;
		case 0x7001:	ChunkViewport(ChunkLen); break;
		case 0xafff:	ChunkMaterial(ChunkLen); break;
		case 0xa000:	ChunkMatName(ChunkLen); break;
		case 0xa010:	ChunkMatAmbientColor(ChunkLen); break;
		case 0xa020:	ChunkMatDiffuseColor(ChunkLen); break;
		case 0xa030:	ChunkMatSpecularColor(ChunkLen); break;
		case 0xa040:	ChunkMatShininess(ChunkLen); break;
		case 0xa041:	ChunkMatShin2(ChunkLen); break;
		case 0xa050:	ChunkMatTransparency(ChunkLen); break;
		case 0xa052:	ChunkMatXPFall(ChunkLen); break;
		case 0xa053:	ChunkMatRefBlur(ChunkLen); break;
		case 0xa081:	ChunkMatTwosided(ChunkLen); break;
		case 0xa083:	ChunkMatAdditive(ChunkLen); break;
		case 0xa084:	ChunkMatSelfIllumPCT(ChunkLen); break;
		case 0xa085:	ChunkMatWire(ChunkLen); break;
		case 0xa087:	ChunkMatWireSize(ChunkLen); break;
		case 0xa100:	ChunkMatShade(ChunkLen); break;
		case 0xa200:	ChunkMatTextureMap1(ChunkLen); break;
		case 0xa33a:	ChunkMatTextureMap2(ChunkLen); break;
		case 0xa210:	ChunkMatOpacityMap(ChunkLen); break;
		case 0xa230:	ChunkMatBumpMap(ChunkLen); break;
		case 0xa204:	ChunkMatSpecularMap(ChunkLen); break;
		case 0xa33c:	ChunkMatShininessMap(ChunkLen); break;
		case 0xa33d:	ChunkMatSelfIllumMap(ChunkLen); break;
		case 0xa220:	ChunkMatReflectionMap(ChunkLen); break;
		case 0xa300:	ChunkMatMapName(ChunkLen); break;
		case 0xa351:	ChunkMatMapTiling(ChunkLen); break;
		case 0xa353:	ChunkMatMapBlur(ChunkLen); break;
		case 0xa354:	ChunkMatMapUScale(ChunkLen); break;
		case 0xa356:	ChunkMatMapVScale(ChunkLen); break;
		case 0xa358:	ChunkMatMapUOffset(ChunkLen); break;
		case 0xa35a:	ChunkMatMapVOffset(ChunkLen); break;
		case 0xa35c:	ChunkMatMapAngle(ChunkLen); break;
		case 0x4000:	ChunkNamedObject(ChunkLen); break;
		case 0x4100:	ChunkMesh(ChunkLen); break;
		case 0x4110:	ChunkMeshVertexList(ChunkLen); break;
		case 0x4111:	ChunkMeshFlagList(ChunkLen); break;
		case 0x4120:	ChunkMeshFaceList(ChunkLen); break;
		case 0x4130:	ChunkMeshMaterialList(ChunkLen); break;
		case 0x4140:	ChunkMeshMappingList(ChunkLen); break;
		case 0x4150:	ChunkMeshSmoothingList(ChunkLen); break;
		case 0x4160:	ChunkMeshLocalAxis(ChunkLen); break;
		case 0x4165:	ChunkMeshColor(ChunkLen); break;
		case 0x4700:	ChunkCamera(ChunkLen); break;
		case 0x4720:	ChunkCameraRange(ChunkLen); break;
		case 0x4600:	ChunkLight(ChunkLen); break;
		case 0x4610:	ChunkLightSpot(ChunkLen); break;
		case 0x4620:	ChunkLightOff(ChunkLen); break;
		case 0x4656:	ChunkLightSpotRoll(ChunkLen); break;
		case 0x465b:	ChunkLightBrightness(ChunkLen); break;
		case 0xb000:	ChunkKeyframer(ChunkLen); break;
		case 0xb001:	ChunkKeyAmbient(ChunkLen); break;
		case 0xb002:	ChunkKeyMesh(ChunkLen); break;
		case 0xb003:	ChunkKeyCamera(ChunkLen); break;
		case 0xb004:	ChunkKeyTarget(ChunkLen); break;
		case 0xb005:	ChunkKeyLight(ChunkLen); break;
		case 0xb006:	ChunkKeyLightTarget(ChunkLen); break;
		case 0xb007:	ChunkKeySpotLight(ChunkLen); break;
		case 0xb008:	ChunkKeySegment(ChunkLen); break;
		case 0xb009:	ChunkKeyCurrentTime(ChunkLen); break;
		case 0xb00a:	ChunkKeyHeader(ChunkLen); break;
		case 0xb010:	ChunkKeyNodeHeader(ChunkLen); break;
		case 0xb011:	ChunkKeyDummyObject(ChunkLen); break;
		case 0xb013:	ChunkKeyPivot(ChunkLen); break;
		case 0xb014:	ChunkKeyBoundBox(ChunkLen); break;
		case 0xb015:	ChunkKeyMorphSmooth(ChunkLen); break;
		case 0xb020:	ChunkKeyPosTrack(ChunkLen); break;
		case 0xb021:	ChunkKeyRotTrack(ChunkLen); break;
		case 0xb022:	ChunkKeyScaleTrack(ChunkLen); break;
		case 0xb023:	ChunkKeyFovTrack(ChunkLen); break;
		case 0xb024:	ChunkKeyRollTrack(ChunkLen); break;
		case 0xb025:	ChunkKeyColorTrack(ChunkLen); break;
		case 0xb026:	ChunkKeyMorphTrack(ChunkLen); break;
		case 0xb030:	ChunkKeyNodeID(ChunkLen); break;
		default:		stream.Seek(ChunkLen,Stream::CURRENT); break;
	}

	// recurse subchunks
	while ( stream.GetOffset() < endoff )
		SwitchChunk();

	stream.Seek(endoff,Stream::START);
	--level;
}


void Reader3DS::RecurseChunk(int32 ChunkLen)
{
	// recurse subchunks
	int endoff = stream.GetOffset() + ChunkLen;
	while ( stream.GetOffset() < endoff )
		SwitchChunk();
}


// =================================================
// Main3DS
// =================================================

void Reader3DS::ChunkMain3DS(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
}


void Reader3DS::ChunkVersion(int32)
{
	ReadLittleEndian<uint32>(stream); // version
}


// =================================================
// Editor3DS
// =================================================

void Reader3DS::ChunkEditor(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
}


void Reader3DS::ChunkMeshVersion(int32)
{
	ReadLittleEndian<uint32>(stream); // version
}


// =================================================
// Misc3DS
// =================================================

void Reader3DS::ChunkColorFloat(int32)
{
	state.color.r = static_cast<uint8>(ReadLittleEndian<float>(stream));
	state.color.g = static_cast<uint8>(ReadLittleEndian<float>(stream));
	state.color.b = static_cast<uint8>(ReadLittleEndian<float>(stream));
}


void Reader3DS::ChunkColor24(int32)
{
	state.color.r = ReadLittleEndian<uint8>(stream);
	state.color.g = ReadLittleEndian<uint8>(stream);
	state.color.b = ReadLittleEndian<uint8>(stream);
}


void Reader3DS::ChunkIntPercent(int32)
{
	state.percent = static_cast<float>(ReadLittleEndian<uint16>(stream) * 0.01f);
}


void Reader3DS::ChunkFloatPercent(int32)
{
	state.percent = ReadLittleEndian<float>(stream) * 0.01f;
}


void Reader3DS::ChunkMasterScale(int32)
{
	ReadLittleEndian<float>(stream); // scale
}


void Reader3DS::ChunkInnerRange(int32)
{
	state.inner = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkOuterRange(int32)
{
	state.outer = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkViewport(int32 ChunkLen)
{
	stream.Seek(ChunkLen,Stream::CURRENT);
}


// =================================================
// Material3DS
// =================================================

void Reader3DS::ChunkMaterial(int32 ChunkLen)
{
	InitializeMaterial(state.material);

	RecurseChunk(ChunkLen);

	import.materials.PushBack(state.material);
}


void Reader3DS::ChunkMatName(int32)
{
	state.material.name = ReadAscii();
}


void Reader3DS::ChunkMatAmbientColor(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);

	state.material.ambient = state.color;
}


void Reader3DS::ChunkMatDiffuseColor(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);

	state.material.diffuse = state.color;
}


void Reader3DS::ChunkMatSpecularColor(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);

	state.material.specular = state.color;
}


void Reader3DS::ChunkMatShininess(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);

	state.material.shininess = state.percent;
}


void Reader3DS::ChunkMatShin2(int32 ChunkLen)
{
	RecurseChunk(ChunkLen); // float v = state.percent;
}


void Reader3DS::ChunkMatTransparency(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
	state.material.transparency = state.percent;
}


void Reader3DS::ChunkMatXPFall(int32 ChunkLen)
{
	RecurseChunk(ChunkLen); // float v = state.percent;
}


void Reader3DS::ChunkMatRefBlur(int32 ChunkLen)
{
	RecurseChunk(ChunkLen); // float v = state.percent;
}


void Reader3DS::ChunkMatTwosided(int32)
{
	state.material.twosided = true;
}


void Reader3DS::ChunkMatAdditive(int32)
{
	state.material.additive = true;
}


void Reader3DS::ChunkMatSelfIllumPCT(int32 ChunkLen)
{
	RecurseChunk(ChunkLen); // float v = state.percent;
}


void Reader3DS::ChunkMatWire(int32)
{
	state.material.wireframe = true;
}


void Reader3DS::ChunkMatWireSize(int32)
{
	ReadLittleEndian<float>(stream); // wiresize
}


void Reader3DS::ChunkMatShade(int32)
{
	uint8 shade = ReadLittleEndian<uint8>(stream);
	ReadLittleEndian<uint8>(stream); // unassigned

	state.material.shade = (Material3DS::Shade)shade;
}


// =================================================
// Texture3DS
// =================================================

void Reader3DS::ReadTexture(Texture3DS& texture, int32 ChunkLen)
{
	InitializeTexture(state.texture);

	RecurseChunk(ChunkLen);

	texture = state.texture;
}


void Reader3DS::ChunkMatTextureMap1(int32 ChunkLen)
{
	ReadTexture(state.material.texturemap1,ChunkLen);
}


void Reader3DS::ChunkMatTextureMap2(int32 ChunkLen)
{
	ReadTexture(state.material.texturemap2,ChunkLen);
}


void Reader3DS::ChunkMatOpacityMap(int32 ChunkLen)
{
	ReadTexture(state.material.opacitymap,ChunkLen);
}


void Reader3DS::ChunkMatBumpMap(int32 ChunkLen)
{
	ReadTexture(state.material.bumpmap,ChunkLen);
}


void Reader3DS::ChunkMatSpecularMap(int32 ChunkLen)
{
	ReadTexture(state.material.specularitymap,ChunkLen);
}


void Reader3DS::ChunkMatShininessMap(int32 ChunkLen)
{
	ReadTexture(state.material.shininessmap,ChunkLen);
}


void Reader3DS::ChunkMatSelfIllumMap(int32 ChunkLen)
{
	ReadTexture(state.material.selfilluminationmap,ChunkLen);
}


void Reader3DS::ChunkMatReflectionMap(int32 ChunkLen)
{
	ReadTexture(state.material.reflectionmap,ChunkLen);
}


void Reader3DS::ChunkMatMapName(int32)
{
	char* name = ReadAscii();
	state.texture.filename = name;
}


void Reader3DS::ChunkMatMapTiling(int32)
{
	state.texture.tiling = static_cast<Texture3DS::Tiling>(ReadLittleEndian<uint16>(stream));
}


void Reader3DS::ChunkMatMapBlur(int32)
{
	state.texture.blur = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkMatMapUScale(int32)
{
	state.texture.scale.x = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkMatMapVScale(int32)
{
	state.texture.scale.y = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkMatMapUOffset(int32)
{
	state.texture.offset.x = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkMatMapVOffset(int32)
{
	state.texture.offset.y = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkMatMapAngle(int32)
{
	state.texture.angle = ReadLittleEndian<float>(stream);
}


// =================================================
// NamedObject3DS
// =================================================

void Reader3DS::ChunkNamedObject(int32 ChunkLen)
{
	int endoff = stream.GetOffset() + ChunkLen;

	state.name = ReadAscii();

	// recurse subchunks
	while ( stream.GetOffset() < endoff )
		SwitchChunk();

	++state.index;
}


// =================================================
// TriMesh3DS
// =================================================

void Reader3DS::ChunkMesh(int32 ChunkLen)
{
	TriMesh3DS& trimesh = state.trimesh;
	InitializeTriMesh(trimesh);

	RecurseChunk(ChunkLen);

	// inverse node xform
	matrix4x4f ixform = trimesh.xform;
	ixform.Inverse();

	// transform to model coordinates
	for ( int i=0; i<trimesh.vertices.GetSize(); ++i )
		trimesh.vertices[i].point *= ixform;

	// calculate normals
	CreateTriMeshNormals(trimesh);


	trimesh.name = state.name;
	trimesh.node = state.index;
	import.trimeshes.PushBack(trimesh);
}


void Reader3DS::CreateTriMeshNormals(TriMesh3DS& mesh)
{
	int i = 0;

	// zero the vertex normals
	for ( i=0; i<mesh.vertices.GetSize(); ++i )
	{
		mesh.vertices[i].normal = vec3f(0,0,0);
	}

	// calculate vertex normals
	for ( i=0; i<mesh.faces.GetSize(); ++i )
	{
		Face3DS& face = mesh.faces[i];

		Vertex3DS& v0 = mesh.vertices[face.vertex[0]];
		Vertex3DS& v1 = mesh.vertices[face.vertex[1]];
		Vertex3DS& v2 = mesh.vertices[face.vertex[2]];

		vec3f normal = prmath::CrossProduct(v0.point-v1.point,v0.point-v2.point);
		v0.normal += normal;
		v1.normal += normal;
		v2.normal += normal;
	}

	// normalize vertex normals
	for ( i=0; i<mesh.vertices.GetSize(); ++i )
	{
		mesh.vertices[i].normal.Normalize();
	}
}


void Reader3DS::ChunkMeshVertexList(int32)
{
	TriMesh3DS& trimesh = state.trimesh;

	uint16 numvertex = ReadLittleEndian<uint16>(stream);
	trimesh.vertices.SetSize(numvertex);

	for ( int i=0; i<numvertex; ++i )
		trimesh.vertices[i].point = ReadVector3();
}


void Reader3DS::ChunkMeshFlagList(int32)
{
	uint16 numflag = ReadLittleEndian<uint16>(stream);

	for ( int i=0; i<numflag; ++i )
		ReadLittleEndian<uint16>(stream);
}


void Reader3DS::ChunkMeshFaceList(int32 ChunkLen)
{
	TriMesh3DS& trimesh = state.trimesh;

	int endoff = stream.GetOffset() + ChunkLen;

	uint16 numface = ReadLittleEndian<uint16>(stream);
	trimesh.faces.SetSize(numface);

	for ( int i=0; i<numface; ++i )
	{
		Face3DS& face = trimesh.faces[i];

		face.vertex[2] = ReadLittleEndian<uint16>(stream);
		face.vertex[1] = ReadLittleEndian<uint16>(stream);
		face.vertex[0] = ReadLittleEndian<uint16>(stream);
		face.flags = ReadLittleEndian<uint16>(stream);
	}

	// recurse subchunks
	while ( stream.GetOffset() < endoff )
		SwitchChunk();
}


void Reader3DS::ChunkMeshMaterialList(int32)
{
	char* name = ReadAscii();
	uint16 numindex = ReadLittleEndian<uint16>(stream);

	uint32 matidx = GetMaterialIndex(name);
	Array<Face3DS>& faces = state.trimesh.faces;

	for ( int i=0; i<numindex; ++i )
	{
		uint16 faceindex = ReadLittleEndian<uint16>(stream);
		faces[faceindex].material = matidx;
	}
}


void Reader3DS::ChunkMeshMappingList(int32 ChunkLen)
{
	TriMesh3DS& trimesh = state.trimesh;

	uint16 numtexcoord = ReadLittleEndian<uint16>(stream);
	if ( numtexcoord != trimesh.vertices.GetSize() )
	{
		stream.Seek(ChunkLen-2,Stream::CURRENT);
		return;
	}

	for ( int i=0; i<numtexcoord; ++i )
	{
		Vertex3DS& vertex = trimesh.vertices[i];

		vertex.texcoord.x =  ReadLittleEndian<float>(stream);
		vertex.texcoord.y = -ReadLittleEndian<float>(stream);
	}
}


void Reader3DS::ChunkMeshSmoothingList(int32 ChunkLen)
{
	TriMesh3DS& trimesh = state.trimesh;

	int count = ChunkLen / 4;
	if ( count != trimesh.faces.GetSize() )
	{
		stream.Seek(ChunkLen,Stream::CURRENT);
		return;
	}

	for ( int i=0; i<count; ++i )
		trimesh.faces[i].smoothgroup = ReadLittleEndian<uint32>(stream);
}


void Reader3DS::ChunkMeshLocalAxis(int32)
{
	TriMesh3DS& trimesh = state.trimesh;
	matrix4x4f& xform = trimesh.xform;

	for ( int i=0; i<4; ++i )
	{
		for ( int j=0; j<3; ++j )
			xform[j][i] = ReadLittleEndian<float>(stream);
	}
	RemapMatrix(xform);
}


void Reader3DS::ChunkMeshColor(int32)
{
	ReadLittleEndian<int8>(stream); // color
}


// =================================================
// Camera3DS
// =================================================

void Reader3DS::ChunkCamera(int32 ChunkLen)
{
	Camera3DS& camera = state.camera;
	InitializeCamera(camera);

	int endoff = stream.GetOffset() + ChunkLen;

	camera.position = ReadVector3();
	camera.target   = ReadVector3();
	camera.bank     = ReadLittleEndian<float>(stream);
	camera.lense    = ReadLittleEndian<float>(stream);

	// recurse subchunks
	while ( stream.GetOffset() < endoff )
		SwitchChunk();

	camera.inner_range = state.inner;
	camera.outer_range = state.outer;

	camera.name = state.name;
	camera.node = state.index;
	import.cameras.PushBack(camera);
}


void Reader3DS::ChunkCameraRange(int32)
{
	Camera3DS& camera = state.camera;

	camera.inner_range = ReadLittleEndian<float>(stream);
	camera.outer_range = ReadLittleEndian<float>(stream);
}


// =================================================
// Light3DS
// =================================================

void Reader3DS::ChunkLight(int32 ChunkLen)
{
	Light3DS& light = state.light;
	InitializeLight(light);

	int endoff = stream.GetOffset() + ChunkLen;

	light.position = ReadVector3();

	// recurse subchunks
	while ( stream.GetOffset() < endoff )
		SwitchChunk();

	light.color = state.color;
	light.inner_range = state.inner;
	light.outer_range = state.outer;

	light.name = state.name;
	light.node = state.index;
	import.lights.PushBack(light);
}


void Reader3DS::ChunkLightSpot(int32)
{
	Light3DS& light = state.light;
	light.spotlight = true;
	light.target    = ReadVector3();
	light.hotspot   = ReadLittleEndian<float>(stream);
	light.falloff   = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkLightOff(int32)
{
	state.light.enable = false;
}


void Reader3DS::ChunkLightSpotRoll(int32)
{
	state.light.roll = ReadLittleEndian<float>(stream);
}


void Reader3DS::ChunkLightBrightness(int32)
{
	state.light.brightness = ReadLittleEndian<float>(stream);
}


// =================================================
// Keyframes3DS
// =================================================

void Reader3DS::ChunkKeyframer(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
}


void Reader3DS::ChunkKeyAmbient(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
	state.node->type = Node3DS::AMBIENT;
}


void Reader3DS::ChunkKeyMesh(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);

	// detect dummy chunk 
	// ( weird autodesk kludge, someone please explain this )
	if ( state.node->name != "$$$DUMMY" )
		state.node->type = Node3DS::TRIMESH;
}


void Reader3DS::ChunkKeyCamera(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
	state.node->type = Node3DS::CAMERA;
}


void Reader3DS::ChunkKeyTarget(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
	state.node->type = Node3DS::TARGETCAMERA;
}


void Reader3DS::ChunkKeyLight(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
	state.node->type = Node3DS::LIGHT;
}


void Reader3DS::ChunkKeyLightTarget(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
	state.node->type = Node3DS::TARGETLIGHT;
}


void Reader3DS::ChunkKeySpotLight(int32 ChunkLen)
{
	RecurseChunk(ChunkLen);
	state.node->type = Node3DS::SPOTLIGHT;
}


void Reader3DS::ChunkKeySegment(int32)
{
	ReadLittleEndian<int32>(stream); // minframe
	ReadLittleEndian<int32>(stream); // maxframe
}


void Reader3DS::ChunkKeyCurrentTime(int32)
{
	ReadLittleEndian<int32>(stream); // time
}


void Reader3DS::ChunkKeyHeader(int32 ChunkLen)
{
	stream.Seek(ChunkLen,Stream::CURRENT);
}


void Reader3DS::ChunkKeyNodeHeader(int32)
{
	String name = ReadAscii();
	ReadLittleEndian<uint16>(stream); // flag1
	ReadLittleEndian<uint16>(stream); // flag2
	uint16 parent = ReadLittleEndian<uint16>(stream);

	// new node
	Node3DS& node = import.nodes.PushBack();
	state.node = &node;
	
	node.type   = Node3DS::NODE;
	node.name   = name;
	node.nodeid = state.index;
	node.parent = parent;
	node.target = 0xffff;
	node.pivot  = vec3f(0,0,0);
}


void Reader3DS::ChunkKeyDummyObject(int32)
{
	state.node->name = ReadAscii();
}


void Reader3DS::ChunkKeyPivot(int32)
{
	state.node->pivot = ReadVector3();
}


void Reader3DS::ChunkKeyBoundBox(int32)
{
	ReadVector3(); // min
	ReadVector3(); // max
}


void Reader3DS::ChunkKeyMorphSmooth(int32)
{
	ReadLittleEndian<float>(stream); // smoothing
}


void Reader3DS::ChunkKeyNodeID(int32)
{
	state.index = ReadLittleEndian<uint16>(stream);
}


// =================================================
// KeyframesTrack3DS
// =================================================

void Reader3DS::ReadKeyHeader(KeyHeader3DS& header)
{
	header.tension    = 0.0f;
	header.continuity = 0.0f;
	header.bias       = 0.0f;
	header.easefrom   = 25.0f;
	header.easeto     = 25.0f;

	header.time = ReadLittleEndian<uint16>(stream);
	uint32 mask = ReadLittleEndian<uint32>(stream);

	for ( int bit=0; bit<32; ++bit )
	{
		int code = (1 << bit) & mask;
		if ( code )
		{
			float data = ReadLittleEndian<float>(stream);
			switch ( code )
			{
				case KeyHeader3DS::EASEFROM:	header.easefrom = data; break;
				case KeyHeader3DS::EASETO:		header.easeto = data; break;
				case KeyHeader3DS::TENSION:		header.tension = data; break;
				case KeyHeader3DS::CONT:		header.continuity = data; break;
				case KeyHeader3DS::BIAS:		header.bias = data; break;
			}
		}
	}
}


void Reader3DS::ChunkKeyPosTrack(int32)
{
	stream.Seek(10,Stream::CURRENT);
	int numkey = ReadLittleEndian<int32>(stream);

	while ( numkey-- )
	{
		PosKey3DS key;
		ReadKeyHeader(key);

		key.pos = ReadVector3();
		state.node->postrack.PushBack(key);
	}
}


void Reader3DS::ChunkKeyRotTrack(int32)
{
	stream.Seek(10,Stream::CURRENT);
	int numkey = ReadLittleEndian<int32>(stream);
	
	quat4f heading;
	heading.Identity();

	while ( numkey-- )
	{
		RotKey3DS key;
		ReadKeyHeader(key);

		float roll = ReadLittleEndian<float>(stream);
		vec3f axis;
		axis.x = ReadLittleEndian<float>(stream);
		axis.y = ReadLittleEndian<float>(stream);
		axis.z = ReadLittleEndian<float>(stream);

		quat4f delta(roll,axis);
		RemapQuaternion(delta);
		heading = delta * heading;

		key.rot = heading;
		state.node->rottrack.PushBack(key);
	}
}


void Reader3DS::ChunkKeyScaleTrack(int32)
{
	stream.Seek(10,Stream::CURRENT);
	int numkey = ReadLittleEndian<int32>(stream);

	while ( numkey-- )
	{
		ScaleKey3DS key;
		ReadKeyHeader(key);
	
		key.scale.x = ReadLittleEndian<float>(stream);
		key.scale.z = ReadLittleEndian<float>(stream);
		key.scale.y = ReadLittleEndian<float>(stream);
		state.node->scaletrack.PushBack(key);
	}
}


void Reader3DS::ChunkKeyFovTrack(int32)
{
	stream.Seek(10,Stream::CURRENT);
	int numkey = ReadLittleEndian<int32>(stream);

	while ( numkey-- )
	{
		FovKey3DS key;
		ReadKeyHeader(key);
	
		key.fov = ReadLittleEndian<float>(stream);
		state.node->fovtrack.PushBack(key);
	}
}


void Reader3DS::ChunkKeyRollTrack(int32)
{
	stream.Seek(10,Stream::CURRENT);
	int numkey = ReadLittleEndian<int32>(stream);

	while ( numkey-- )
	{
		RollKey3DS key;
		ReadKeyHeader(key);
	
		key.roll = ReadLittleEndian<float>(stream);
		state.node->rolltrack.PushBack(key);
	}
}


void Reader3DS::ChunkKeyColorTrack(int32)
{
	stream.Seek(10,Stream::CURRENT);
	int numkey = ReadLittleEndian<int32>(stream);

	while ( numkey-- )
	{
		ColorKey3DS key;
		ReadKeyHeader(key);

		float r = ReadLittleEndian<float>(stream) * 255.0f;
		float g = ReadLittleEndian<float>(stream) * 255.0f;
		float b = ReadLittleEndian<float>(stream) * 255.0f;

		key.color.r = static_cast<uint8>(r);
		key.color.g = static_cast<uint8>(g);
		key.color.b = static_cast<uint8>(b);
		key.color.a = 0xff;
		state.node->colortrack.PushBack(key);
	}
}


void Reader3DS::ChunkKeyMorphTrack(int32)
{
	stream.Seek(10,Stream::CURRENT);
	int numkey = ReadLittleEndian<int32>(stream);

	while ( numkey-- )
	{
		MorphKey3DS key;
		ReadKeyHeader(key);
	
		key.name = ReadAscii();
		state.node->morphtrack.PushBack(key);
	}
}


// =================================================
// utilities
// =================================================

void Reader3DS::RemapVector3(vec3f& v)
{
	v = vec3f(-v.x,v.z,-v.y);
}


void Reader3DS::RemapMatrix(matrix4x4f& m)
{
	float t[12];
	
	t[0] = m[0][0];
	t[1] = m[0][1];
	t[2] = m[0][2];
	t[3] = m[1][0];
	t[4] = m[1][1];
	t[5] = m[1][2];
	t[6] = m[2][0];
	t[7] = m[2][1];
	t[8] = m[2][2];
	t[9] = m[0][3];
	t[10] = m[1][3];
	t[11] = m[2][3];
	
	m[0][0] = t[0];
	m[0][1] =-t[2];
	m[0][2] = t[1];
	m[0][3] =-t[9];
	m[1][0] =-t[6];
	m[1][1] = t[8];
	m[1][2] =-t[7];
	m[1][3] = t[11];
	m[2][0] = t[3];
	m[2][1] =-t[5];
	m[2][2] = t[4];
	m[2][3] =-t[10];
	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;
}


void Reader3DS::RemapQuaternion(quat4f& q)
{
	q = quat4f(-q.x,q.z,-q.y,q.w);
}


vec3f Reader3DS::ReadVector3()
{
	vec3f v;
	v.x = ReadLittleEndian<float>(stream);
	v.y = ReadLittleEndian<float>(stream);
	v.z = ReadLittleEndian<float>(stream);

	RemapVector3(v);
	return v;
}


char* Reader3DS::ReadAscii()
{
	static char buffer[256];

	char* ptr = buffer;
	for ( ;; )
	{
		char c;
		stream.Read(&c,1);
		*ptr++ = c;

		if ( !c ) break;
	}

	return buffer;
}


int Reader3DS::GetMaterialIndex(const char* name)
{
	for ( int i=0; i<import.materials.GetSize(); ++i )
	{
		if ( import.materials[i].name == name )
			return i;
	}
	return 0;
}


void Reader3DS::InitializeMaterial(Material3DS& material)
{
	material.shade        = Material3DS::GOURAUD;
	material.ambient      = Color32(~0);
	material.diffuse      = Color32(~0);
	material.specular     = Color32(~0);
	material.shininess    = 0;
	material.transparency = 0;
	material.twosided     = false;
	material.additive     = false;
	material.wireframe    = false;
}


void Reader3DS::InitializeTexture(Texture3DS& texture)
{
	texture.tiling   = Texture3DS::TILE;
	texture.strength = 0.0f;
	texture.scale    = vec2f(1,1);
	texture.offset   = vec2f(0,0);
	texture.angle    = 0;
	texture.blur     = 0;
}


void Reader3DS::InitializeNode(Node3DS& node)
{
	node.type   = Node3DS::NODE;
	node.parent = 0xffff;
	node.target = 0xffff;
}


void Reader3DS::InitializeTriMesh(TriMesh3DS& trimesh)
{
	trimesh.xform.Identity();
	trimesh.vertices.Clear();
	trimesh.faces.Clear();
}


void Reader3DS::InitializeCamera(Camera3DS& camera)
{
	camera.position    = point3f(0,0,0);
	camera.target      = point3f(0,0,1);
	camera.bank        = 0.0f;
	camera.lense       = 1;
	camera.inner_range = 0;
	camera.outer_range = 1;
}


void Reader3DS::InitializeLight(Light3DS& light)
{
	light.spotlight = false;
	light.enable    = true;
}


void Reader3DS::InitializeKeyHeader(KeyHeader3DS& header)
{
	header.easeto     = 0;
	header.easefrom   = 0;
	header.tension    = 0;
	header.continuity = 0;
	header.bias       = 0;
}


// =================================================
// interpolation
// =================================================
/*
static float HermiteEase(float a, float b, float t)
{
	float s = a + b;
	if ( !s ) return t;
	if ( s > 1 )
	{
		a = a / s;
		b = b / s;
	}

	float k = 1 / (2 - a - b);
	if ( t < a ) 
	{
		return (k/a) * t * t;
	}
    else 
	{
		if ( t < 1-b )
		{
			return k * (2*t - a);
		}
        else 
		{
			t = 1 - t;
			return (1 - (k/b) * t * t);
		}
	}
}


static float HermiteInterpolate(
	const Import3DS::KeyHeader& header0,
	const Import3DS::KeyHeader& header1,
	float left,
	float src,
	float dest,
	float right,
	float t)
{
	// hermite co-efficients
	t = HermiteEase( header0.easefrom, header1.easeto, t );
	float t2 = t * t;
	float t3 = t2 * t;
	float h00 = 2*t3 - 3*t2 + 1;
	float h01 = t3 - 2*t2 + t;
	float h10 = -2*t3 + 3*t2;
	float h11 = t3 - t2;

	// source derivate
	float s0 = (1 - header0.tension) * (1 - header0.continuity) * (1 + header0.bias);
	float s1 = (1 - header0.tension) * (1 + header0.continuity) * (1 - header0.bias);
	float sd = (s0 * (src - left) + s1 * (dest - src)) * 0.5f;

	// destination derivate
	float d0 = (1 - header1.tension) * (1 + header1.continuity) * (1 + header1.bias);
	float d1 = (1 - header1.tension) * (1 - header1.continuity) * (1 - header1.bias);
	float dd = (d0 * (dest - src) + d1 * (right - dest)) * 0.5f;

	return h00*src + h10*dest + h01*sd + h11*dd;
}


static float HermiteLerp(float left, float right, float t)
{
	return Lerp(left, right, t);
}
*/


// =================================================
// importer
// =================================================

Import3DS::Import3DS(Stream& stream)
{
	Reader3DS reader(*this,stream);
}


Import3DS::Import3DS(const String& filename)
{
	FileStream stream(filename);
	Reader3DS reader(*this,stream);
}


Import3DS::~Import3DS()
{
}


uint16 Import3DS::FindNodeID(const Object3DS& object) const
{
	for ( int i=0; i<nodes.GetSize(); ++i )
	{
		if ( nodes[i].name == object.name )
			return nodes[i].nodeid;
	}
	return 0xffff;
}


Node3DS* Import3DS::FindNode(const Object3DS& object)
{
	for ( int i=0; i<nodes.GetSize(); ++i )
	{
		if ( nodes[i].name == object.name )
			return &nodes[i];
	}
	return NULL;
}
