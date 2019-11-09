﻿//*****************************************************************************************//
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
	float difUv[2];//UV座標
	float speUv[2];//UV座標
	float bBoneIndex[4] = { 0.0f,0.0f,0.0f,0.0f };//ボーン番号
	float bBoneWeight[4] = { 0.0f,0.0f,0.0f,0.0f };//ボーン重み
};

struct Bone {
	MATRIX bindPose;
	MATRIX newPose;
};

class VulkanSkinMesh {

private:
	Device* device = nullptr;
	uint32_t comIndex = 0;

	struct FbxObj {
		FbxLoader fbx;
		Deformer** defo = nullptr;
		float endframe = 0.0f;
		float currentframe = 0.0f;

		~FbxObj() {
			ARR_DELETE(defo);
		}
	};
	FbxObj* fbxObj[32] = {};
	uint32_t numFbxObj = 1;

	std::unique_ptr <VulkanBasicPolygon * []> bp = nullptr;
	std::unique_ptr <textureIdSet * []> cTexId = nullptr;
	uint32_t numMesh = 0;
	uint32_t numBone = 0;
	std::unique_ptr<Bone[]> bone = nullptr;
	std::unique_ptr<MATRIX[]>outPose = nullptr;

	void setNewPoseMatrix(uint32_t animationIndex, float time);
	MATRIX getCurrentPoseMatrix(uint32_t index);

public:
	VulkanSkinMesh(Device* device, char* pass, float endframe, uint32_t comIndex = 0);
	~VulkanSkinMesh();
	void additionalAnimation(char* pass, float endframe);
	void create();
	void setMaterialParameter(uint32_t meshIndex, uint32_t materialIndex, VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient);
	void setChangeTexture(uint32_t meshIndex, uint32_t materialIndex, int diffuseTexId, int normalTexId, int specularTexId);
	void draw(uint32_t animationIndex, float time, VECTOR3 pos = { 0.0f,0.0f,0.0f },
		VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
	bool autoDraw(uint32_t animationIndex, float pitchTime, VECTOR3 pos = { 0.0f,0.0f,0.0f },
		VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
};

#endif
