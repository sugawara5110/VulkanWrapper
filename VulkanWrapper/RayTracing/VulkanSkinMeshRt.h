//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanSkinMeshRt.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanSkinMeshRt_Header
#define VulkanSkinMeshRt_Header

#include "VulkanBasicPolygonRt.h"
#include "../../SkinMeshHelper/SkinMeshHelper.h"

class VulkanSkinMeshRt :public SkinMeshHelper {

public:
	struct MY_VERTEX_S {
		CoordTf::VECTOR3 vPos = {};//頂点
		CoordTf::VECTOR3 vNorm = {};//法線
		CoordTf::VECTOR3 vTangent = {};//接ベクトル
		CoordTf::VECTOR2 vTex0 = {};//UV座標0
		CoordTf::VECTOR2 vTex1 = {};//UV座標1
		uint32_t bBoneIndex[4] = {};//ボーン　番号
		float bBoneWeight[4] = {};//ボーン　重み
	};

protected:
	VulkanDevice::Uniform<SHADER_GLOBAL_BONES>* mObject_BONES = nullptr;

	MY_VERTEX_S** pvVB = nullptr;//使用後保持するか破棄するかフラグで決める,通常は破棄
	VulkanBasicPolygonRt::Vertex3D_t** pvVBM = nullptr;
	uint32_t*** newIndex = nullptr;
	uint32_t** NumNewIndex = nullptr;
	VulkanDevice::textureIdSetInput** textureId = nullptr;
	bool pvVB_delete_f = true;

	VulkanBasicPolygonRt* mObj = nullptr;

	void createMaterial(int meshInd, uint32_t numMaterial, FbxMeshNode* mesh, char* uv0Name, char* uv1Name, int* uvSw);
	void swapTex(void* vb, uint32_t Stride, FbxMeshNode* mesh, int* uvSw);

	class SkinningCom {
	private:
		VkPipelineLayout pipelineLayoutSkinned = VK_NULL_HANDLE;
		VkPipeline computeSkiningPipeline = VK_NULL_HANDLE;
		VkDescriptorSetLayout dsLayoutSkinned = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSetCompute = VK_NULL_HANDLE;
		BufferSetRt Buf;
		int numVer = 0;

	public:
		~SkinningCom();
		void createVertexBuffer(uint32_t QueueIndex, uint32_t comIndex, MY_VERTEX_S* ver, uint32_t num);
		void CreateLayouts();
		void CreateComputePipeline();
		void CreateDescriptorSets(VkDescriptorBufferInfo* output, VkDescriptorBufferInfo* bone);
		void Skinned(uint32_t QueueIndex, uint32_t comIndex);
	};
	SkinningCom* sk[VulkanBasicPolygonRt::numSwap] = {};

public:
	VulkanSkinMeshRt();
	~VulkanSkinMeshRt();

	void Vertex_hold();
	void CreateBuffer(int num_end_frame, float* end_frame, bool singleMesh = false, bool deformer = true);
	void CreateBuffer(float end_frame, bool singleMesh = false, bool deformer = true);

	void SetVertex(bool lclOn = false, bool axisOn = false);

	void SetDiffuseTextureName(char* textureName, int materialIndex, int meshIndex);
	void SetNormalTextureName(char* textureName, int materialIndex, int meshIndex);
	void SetSpeculerTextureName(char* textureName, int materialIndex, int meshIndex);

	void setMaterialType(vkMaterialType type, uint32_t meshIndex, uint32_t matIndex);

	void LightOn(bool on, uint32_t meshIndex, uint32_t InstanceIndex, uint32_t matIndex,
		float range = 100.0f, float att1 = 0.1f, float att2 = 0.001f, float att3 = 0.001f);

	bool CreateFromFBX(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, uint32_t numInstance);

	void Instancing(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor);

	bool InstancingUpdate(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex, int AnimationIndex, float time, int InternalAnimationIndex = 0);

	bool Update(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex,
		int AnimationIndex, float time,
		CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor,
		int InternalAnimationIndex = 0);

	CoordTf::VECTOR3 GetVertexPosition(int meshIndex, int verNum, float adjustZ, float adjustY, float adjustX,
		float thetaZ, float thetaY, float thetaX, float scale);

	void setMaterialColor(CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient,
		uint32_t meshIndex,
		uint32_t materialIndex = 0);

	void setMaterialShininess(float shininess,
		uint32_t meshIndex,
		uint32_t materialIndex = 0);

	void setMaterialRefractiveIndex(float RefractiveIndex,
		uint32_t meshIndex,
		uint32_t materialIndex = 0);

	void setMaterialRoughness(float Roughness,
		uint32_t meshIndex,
		uint32_t materialIndex = 0);

	std::vector<VulkanBasicPolygonRt::RtData>& getRtData(uint32_t meshIndex) {
		return mObj[meshIndex].Rdata;
	}
};

#endif