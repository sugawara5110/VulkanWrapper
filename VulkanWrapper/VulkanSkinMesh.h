//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanSkinMesh.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanSkinMesh_Header
#define VulkanSkinMesh_Header

#include "VulkanInstance.h"
#include "VulkanBasicPolygon.h"
#include "../FbxLoader/FbxLoader.h"

struct VertexSkin {
	float pos[3];//頂点
	float normal[3];//法線
	float uv[2];//UV座標
	int32_t bBoneIndex[4] = { 0,0,0,0 };//ボーン番号
	float bBoneWeight[4] = { 0.0f,0.0f,0.0f,0.0f };//ボーン重み
};

class VulkanSkinMesh {

private:
	Device* device = nullptr;
	uint32_t comIndex = 0;
	FbxLoader fbx;
	typedef VulkanBasicPolygon* BasicPolygon;
	std::unique_ptr <BasicPolygon[]> bp = nullptr;
	uint32_t numMesh = 0;
	uint32_t numBone = 0;
	MATRIX bindPose;
	MATRIX newPose;
	MATRIX bone;
	float currentframe = 0.0f;
	float endframe = 0.0f;

public:
	VulkanSkinMesh(Device* device, uint32_t comIndex = 0);
	~VulkanSkinMesh();
	void create(char* pass, float endframe);


};

#endif
