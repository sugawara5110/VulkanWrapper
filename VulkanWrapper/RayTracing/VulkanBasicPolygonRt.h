//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygonRt.h                                   **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBasicPolygonRt_Header
#define VulkanBasicPolygonRt_Header

#include "AccelerationStructure.h"

enum vkMaterialType {
	NONREFLECTION = 0b100000,
	METALLIC = 0b010000,
	EMISSIVE = 0b001000,
	TRANSLUCENCE = 0b000100
};

class VulkanBasicPolygonRt {

public:
	static const uint32_t numSwap = 3;

	struct Instance {
		VkAccelerationStructureInstanceKHR vkInstance[numSwap] = {};
		CoordTf::MATRIX world = {};
		CoordTf::MATRIX mvp = {};
		VkTransformMatrixKHR vkWorld = {};
		CoordTf::VECTOR4 lightst = { 100.0f,0.1f, 0.001f, 0.001f };//�����W, ����1, ����2, ����3
		float lightOn = 0.0f;
		float OutlineSize = 0.0f;//�p�X�g���̃��C�g��PDF�v�Z�Ɏg�p (�����̂̕\�ʐςŌv�Z)
		CoordTf::VECTOR4 addColor = {};
	};
	struct RtMaterial {
		CoordTf::VECTOR4 vDiffuse = { 0.8f,0.8f,0.8f,1.0f };//�f�B�t���[�Y�F
		CoordTf::VECTOR4 vSpeculer = { 0.2f,0.2f,0.2f,1.0f };//�X�؃L�����F
		CoordTf::VECTOR4 vAmbient = { 0.0f,0.0f,0.0f,0.0f };//�A���r�G���g
		CoordTf::VECTOR4 shininess = { 4.0f,0.0f,0.0f,0.0f };//x:�X�y�L��������
		CoordTf::VECTOR4 RefractiveIndex_roughness = { 1.0f,0.05f,0.0f,0.0f };//x:���ܗ�:1.0f:��C(�), 1.33f:��, 1.5f:�K���X, 2.4f:�_�C��, y:�e��
		CoordTf::VECTOR4 MaterialType = { NONREFLECTION,0.0f,0.0f,0.0f };//x:
	};

	struct RtData {
		BufferSetRt* vertexBuf[numSwap] = {};//�|�C���^�󂯎�邾��
		uint32_t vertexCount = 0;
		uint32_t vertexStride = 0;
		BufferSetRt indexBuf = {};
		uint32_t indexCount = 0;

		VulkanDevice::textureIdSet texId = {};

		RtMaterial mat = {};

		AccelerationStructure BLAS[numSwap] = {};
		std::vector<Instance> instance = {};
		uint32_t hitShaderIndex = 0;

		float LmaxX = 0.0f;
		float LminX = 0.0f;
		float LmaxY = 0.0f;
		float LminY = 0.0f;
		float LmaxZ = 0.0f;
		float LminZ = 0.0f;
		bool setvSize_first = false;

		void setvSize(CoordTf::VECTOR3 v);
		void createOutlineSize(CoordTf::VECTOR3 scale, int InstanceIndex);
	};
	struct Vertex3D_t {
		CoordTf::VECTOR3 pos = {};
		CoordTf::VECTOR3 normal = {};
		CoordTf::VECTOR3 tangent = {};
		CoordTf::VECTOR2 difUv = {};
		CoordTf::VECTOR2 speUv = {};
	};

private:
	bool rdataCreateF = false;
	uint32_t InstanceCnt = 0;
	BufferSetRt vertexBuf[numSwap] = {};
	uint32_t vertexCount = 0;
	uint32_t vertexStride = 0;

	bool useAlpha = false;
	bool UpdateBLAS_On = false;

	void createVertexBuffer(uint32_t QueueIndex, uint32_t comIndex, Vertex3D_t* ver, uint32_t num);
	void createIndexBuffer(RtData& rdata, uint32_t QueueIndex, uint32_t comIndex, uint32_t* ind, uint32_t indNum);
	void createBLAS(RtData& rdata, uint32_t QueueIndex, uint32_t comIndex);
	void updateInstance(uint32_t swapIndex, RtData& rdata);
	void createTexture(RtData& rdata, uint32_t QueueIndex, uint32_t comIndex, VulkanDevice::textureIdSetInput& texId);
	void updateBLAS(uint32_t swapIndex, RtData& rdata, uint32_t QueueIndex, uint32_t comIndex);

public:
	std::vector<RtData> Rdata;

	VulkanBasicPolygonRt();
	~VulkanBasicPolygonRt();

	void createMultipleMaterials(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, uint32_t numMat,
		Vertex3D_t* ver, uint32_t num, uint32_t** ind, uint32_t* indNum,
		VulkanDevice::textureIdSetInput* texid, uint32_t numInstance, bool updateBLAS_on);

	void setMaterialType(vkMaterialType type, uint32_t matIndex);

	void LightOn(bool on, uint32_t InstanceIndex, uint32_t matIndex,
		float range = 100.0f, float att1 = 0.1f, float att2 = 0.001f, float att3 = 0.001f);

	void create(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha,
		VulkanDevice::Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum,
		int32_t difTexInd, int32_t norTexInd, int32_t speTexInd, uint32_t numInstance);

	void setMaterialColor(CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient,
		uint32_t materialIndex = 0);

	void setMaterialShininess(float shininess, uint32_t materialIndex = 0);

	void setMaterialRefractiveIndex(float RefractiveIndex, uint32_t materialIndex = 0);

	void setMaterialRoughness(float Roughness, uint32_t materialIndex = 0);

	void instancing(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor);
	void instancingUpdate(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex);

	void update(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex,
		CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor);
};

#endif
