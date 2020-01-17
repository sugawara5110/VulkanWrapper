//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanSkinMesh.h                                       **//
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
	MATRIX bindPose = {};
	MATRIX newPose = {};
	MATRIX connectFirstPose = {};
	MATRIX connectLastPose = {};
};

class VulkanSkinMesh {

private:
	Device* device = nullptr;

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
	uint32_t prevAnimationIndex = -1;
	bool connectionOn = false;
	float ConnectionRatio = 0.0f;
	float connectionPitch = 0.1f;

	void setNewPoseMatrix(uint32_t animationIndex, float time);
	void copyConnectionPoseMatrix(uint32_t nextAnimationIndex);
	void setNewPoseMatrixConnection(float connectionRatio);
	MATRIX getCurrentPoseMatrix(uint32_t index);
	void subUpdate(uint32_t swapIndex, VECTOR3 pos, VECTOR3 theta, VECTOR3 scale);
	void setfbx();
	void setAnimation();

public:
	~VulkanSkinMesh();
	void setFbx(Device* device, char* pass, float endframe);
	void setFbxInByteArray(Device* device, char* byteArray, unsigned int size, float endframe);
	void additionalAnimation(char* pass, float endframe);
	void additionalAnimationInByteArray(char* byteArray, unsigned int size, float endframe);
	void create(uint32_t comIndex, bool useAlpha);
	void setMaterialParameter(uint32_t swapIndex, uint32_t meshIndex, uint32_t materialIndex, VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient);
	void setChangeTexture(uint32_t meshIndex, uint32_t materialIndex, int diffuseTexId, int normalTexId, int specularTexId);
	void update(uint32_t swapIndex, uint32_t animationIndex, float time, VECTOR3 pos = { 0.0f,0.0f,0.0f },
		VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
	void setConnectionPitch(float pitch);
	bool autoUpdate(uint32_t swapIndex, uint32_t animationIndex, float pitchTime, VECTOR3 pos = { 0.0f,0.0f,0.0f },
		VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
	void draw(uint32_t swapIndex, uint32_t comIndex);
};

#endif
