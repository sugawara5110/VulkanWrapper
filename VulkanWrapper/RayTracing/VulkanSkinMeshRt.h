//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanSkinMeshRt.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanSkinMeshRt_Header
#define VulkanSkinMeshRt_Header

#include "VulkanBasicPolygonRt.h"
#include "../../FbxLoader/FbxLoader.h"

class VulkanSkinMeshRt {

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
	class SkinMesh_sub {
	public:
		FbxLoader* fbxL = nullptr;
		std::unique_ptr<float[]> end_frame = nullptr;
		float current_frame = 0.0f;
		bool centering = false;
		bool offset = false;
		float cx = 0.0f;
		float cy = 0.0f;
		float cz = 0.0f;
		float connect_step;
		CoordTf::MATRIX rotZYX = {};
		int InternalLastAnimationIndex = 0;

		SkinMesh_sub();
		~SkinMesh_sub();
		bool Create(char* szFileName);
	};

	struct BONE {
		CoordTf::MATRIX mBindPose;//初期ポーズ
		CoordTf::MATRIX mNewPose;//現在のポーズ

		BONE()
		{
			ZeroMemory(this, sizeof(BONE));
		}
	};

	struct SHADER_GLOBAL_BONES {
		CoordTf::MATRIX mBone[VulkanDevice::numBoneMax];
		SHADER_GLOBAL_BONES()
		{
			for (int i = 0; i < VulkanDevice::numBoneMax; i++)
			{
				CoordTf::MatrixIdentity(&mBone[i]);
			}
		}
	};

	//コンスタントバッファOBJ
	VulkanDevice::Uniform<SHADER_GLOBAL_BONES> mObject_BONES = {};
	SHADER_GLOBAL_BONES sgb[2] = {};

	MY_VERTEX_S** pvVB = nullptr;//使用後保持するか破棄するかフラグで決める,通常は破棄
	VulkanBasicPolygonRt::Vertex3D_t** pvVBM = nullptr;
	uint32_t*** newIndex = nullptr;
	uint32_t** NumNewIndex = nullptr;
	VulkanDevice::textureIdSetInput** textureId = nullptr;
	bool pvVB_delete_f = true;

	//ボーン
	int* numBone = nullptr;
	int maxNumBone = 0;
	int maxNumBoneMeshIndex = 0;
	BONE* m_BoneArray = nullptr;
	char* boneName = nullptr;

	//FBX
	int numMesh = 0;
	SkinMesh_sub* fbx = nullptr;
	Deformer** m_ppSubAnimationBone = nullptr;//その他アニメーションボーンポインタ配列
	CoordTf::MATRIX* m_pLastBoneMatrix = nullptr;
	int AnimLastInd;
	float BoneConnect;
	VulkanBasicPolygonRt* mObj = nullptr;
	CoordTf::MATRIX Axis = {};
	std::unique_ptr<bool[]> noUseMesh = nullptr;

	void DestroyFBX();
	bool InitFBX(char* szFileName, int p);
	void ReadSkinInfo(FbxMeshNode* mesh, MY_VERTEX_S* pvVB);
	CoordTf::MATRIX GetCurrentPoseMatrix(int index);
	void MatrixMap_Bone(SHADER_GLOBAL_BONES* sbB);
	bool SetNewPoseMatrices(float time, int ind, int InternalAnimationIndex);
	void CreateRotMatrix(float thetaZ, float thetaY, float thetaX, int ind);
	void createMaterial(int meshInd, uint32_t numMaterial, FbxMeshNode* mesh, char* uv0Name, char* uv1Name, int* uvSw);
	void swapTex(void* vb, uint32_t Stride, FbxMeshNode* mesh, int* uvSw);
	void splitIndex(uint32_t numMaterial, FbxMeshNode* mesh, int meshIndex);
	void normalRecalculation(bool lclOn, double** nor, FbxMeshNode* mesh);
	void createAxis();
	void LclTransformation(FbxMeshNode* mesh, CoordTf::VECTOR3* vec);

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
		void createVertexBuffer(uint32_t comIndex, MY_VERTEX_S* ver, uint32_t num);
		void CreateLayouts();
		void CreateComputePipeline();
		void CreateDescriptorSets(VkDescriptorBufferInfo* output, VkDescriptorBufferInfo* bone);
		void Skinned(uint32_t comIndex);
	};
	SkinningCom* sk = nullptr;

public:
	VulkanSkinMeshRt();
	~VulkanSkinMeshRt();

	void ObjCentering(bool f, int ind);
	void ObjCentering(float x, float y, float z, float thetaZ, float thetaY, float thetaX, int ind);
	void ObjOffset(float x, float y, float z, float thetaZ, float thetaY, float thetaX, int ind);
	void SetConnectStep(int ind, float step);
	void Vertex_hold();
	void GetFbx(char* szFileName);
	void GetBuffer(int num_end_frame, float* end_frame, bool singleMesh = false, bool deformer = true);
	void GetBuffer(float end_frame, bool singleMesh = false, bool deformer = true);
	void noUseMeshIndex(int meshIndex);
	void SetVertex(bool lclOn = false, bool axisOn = false);

	void SetDiffuseTextureName(char* textureName, int materialIndex, int meshIndex);
	void SetNormalTextureName(char* textureName, int materialIndex, int meshIndex);
	void SetSpeculerTextureName(char* textureName, int materialIndex, int meshIndex);

	void setMaterialType(vkMaterialType type, uint32_t meshIndex, uint32_t matIndex);

	void LightOn(bool on, uint32_t meshIndex, uint32_t InstanceIndex, uint32_t matIndex,
		float range = 100.0f, float att1 = 0.1f, float att2 = 0.001f, float att3 = 0.001f);

	bool CreateFromFBX(uint32_t comIndex, bool useAlpha, uint32_t numInstance);
	void GetFbxSub(char* szFileName, int ind);
	void GetBuffer_Sub(int ind, int num_end_frame, float* end_frame);
	void GetBuffer_Sub(int ind, float end_frame);
	void CreateFromFBX_SubAnimation(int ind);

	void Instancing(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale);

	bool InstancingUpdate(uint32_t comIndex, int AnimationIndex, float time, int InternalAnimationIndex = 0);

	bool Update(uint32_t comIndex,
		int AnimationIndex, float time,
		CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale,
		int InternalAnimationIndex = 0);

	CoordTf::VECTOR3 GetVertexPosition(int meshIndex, int verNum, float adjustZ, float adjustY, float adjustX,
		float thetaZ, float thetaY, float thetaX, float scale);

	int getNumMesh() { return numMesh; }

	void setMaterialColor(CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient,
		uint32_t meshIndex,
		uint32_t materialIndex = 0);

	void setMaterialShininess(float shininess,
		uint32_t meshIndex,
		uint32_t materialIndex = 0);

	void setMaterialRefractiveIndex(float RefractiveIndex,
		uint32_t meshIndex,
		uint32_t materialIndex = 0);

	std::vector<VulkanBasicPolygonRt::RtData>& getRtData(uint32_t meshIndex) {
		return mObj[meshIndex].Rdata;
	}
};

#endif