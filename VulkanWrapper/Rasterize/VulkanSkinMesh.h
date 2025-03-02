//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanSkinMesh.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanSkinMesh_Header
#define VulkanSkinMesh_Header

#include "VulkanBasicPolygon.h"
#include "../../FbxLoader/FbxLoader.h"

struct VertexSkin {
	float pos[3] = {};//頂点
	float normal[3] = {};//法線
	float difUv[2] = {};//UV座標
	float speUv[2] = {};//UV座標
	float bBoneIndex[4] = { 0.0f,0.0f,0.0f,0.0f };//ボーン番号
	float bBoneWeight[4] = { 0.0f,0.0f,0.0f,0.0f };//ボーン重み
};

struct Bone {
	CoordTf::MATRIX bindPose = {};
	CoordTf::MATRIX newPose = {};
	CoordTf::MATRIX connectFirstPose = {};
	CoordTf::MATRIX connectLastPose = {};
};

class VulkanSkinMesh {

private:
	struct FbxObj {
		FbxLoader fbx;
		Deformer** defo = nullptr;
		float endframe = 0.0f;
		float currentframe = 0.0f;

		~FbxObj() {
			vkUtil::ARR_DELETE(defo);
		}
	};
	FbxObj* fbxObj[32] = {};
	uint32_t numFbxObj = 1;

	std::unique_ptr <VulkanBasicPolygon* []> bp = nullptr;
	std::unique_ptr <VulkanDevice::textureIdSet* []> cTexId = nullptr;
	uint32_t numMesh = 0;
	uint32_t numBone = 0;
	std::unique_ptr<Bone[]> bone = nullptr;
	std::unique_ptr<CoordTf::MATRIX[]>outPose = nullptr;
	uint32_t prevAnimationIndex = -1;
	bool connectionOn = false;
	float ConnectionRatio = 0.0f;
	float connectionPitch = 0.1f;

	void setNewPoseMatrix(uint32_t animationIndex, float time);
	void copyConnectionPoseMatrix(uint32_t nextAnimationIndex);
	void setNewPoseMatrixConnection(float connectionRatio);
	CoordTf::MATRIX getCurrentPoseMatrix(uint32_t index);
	void subUpdate(uint32_t swapIndex);
	void setfbx();
	void setAnimation();

public:
	~VulkanSkinMesh();

	void setFbx(char* path, float endframe);
	void setFbxInByteArray(char* byteArray, unsigned int size, float endframe);
	void additionalAnimation(char* path, float endframe);
	void additionalAnimationInByteArray(char* byteArray, unsigned int size, float endframe);

	void setNumMaxInstancing(uint32_t num);

	void create(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, bool blending);

	void setMaterialParameter(uint32_t swapIndex, uint32_t meshIndex, uint32_t materialIndex,
		CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient);

	void setChangeTexture(uint32_t meshIndex, uint32_t materialIndex, int diffuseTexId, int normalTexId, int specularTexId);

	void Instancing(uint32_t swapIndex, CoordTf::VECTOR3 pos = { 0.0f,0.0f,0.0f },
		CoordTf::VECTOR3 theta = { 0.0f,0.0f,0.0f }, CoordTf::VECTOR3 scale = { 1.0f,1.0f,1.0f },
		CoordTf::VECTOR4 addCol = {});

	void Instancing_update(uint32_t swapIndex, uint32_t animationIndex, float time);

	void update(uint32_t swapIndex, uint32_t animationIndex, float time, CoordTf::VECTOR3 pos = { 0.0f,0.0f,0.0f },
		CoordTf::VECTOR3 theta = { 0.0f,0.0f,0.0f }, CoordTf::VECTOR3 scale = { 1.0f,1.0f,1.0f },
		CoordTf::VECTOR4 addCol = {});

	void setConnectionPitch(float pitch);

	bool auto_Instancing_update(uint32_t swapIndex, uint32_t animationIndex, float pitchTime);

	bool autoUpdate(uint32_t swapIndex, uint32_t animationIndex, float pitchTime, CoordTf::VECTOR3 pos = { 0.0f,0.0f,0.0f },
		CoordTf::VECTOR3 theta = { 0.0f,0.0f,0.0f }, CoordTf::VECTOR3 scale = { 1.0f,1.0f,1.0f },
		CoordTf::VECTOR4 addCol = {});

	void draw(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex);
};

#endif
