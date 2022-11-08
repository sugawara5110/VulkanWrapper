//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygonRt.h                                   **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBasicPolygonRt_Header
#define VulkanBasicPolygonRt_Header

#include "VulkanDeviceRt.h"
#include "AccelerationStructure.h"

enum vkMaterialType {
	NONREFLECTION = 0b0000,
	METALLIC = 0b1000,
	EMISSIVE = 0b0100,
	DIRECTIONLIGHT = 0b0010,
	TRANSLUCENCE = 0b0001
};

class VulkanBasicPolygonRt {

public:
	struct Instance {
		VkAccelerationStructureInstanceKHR vkInstance = {};
		CoordTf::MATRIX world = {};
		VkTransformMatrixKHR vkWorld = {};
	};
	struct RtMaterial {
		CoordTf::VECTOR4 vDiffuse = { 0.8f,0.8f,0.8f,1.0f };//�f�B�t���[�Y�F
		CoordTf::VECTOR4 vSpeculer = { 0.6f,0.6f,0.6f,1.0f };//�X�؃L�����F
		CoordTf::VECTOR4 vAmbient = { 0.1f,0.1f,0.1f,0.0f };//�A���r�G���g
		float shininess = 4.0f;//�X�y�L��������
		float RefractiveIndex = 0.0f;//���ܗ�
		bool useAlpha = false;
		uint32_t MaterialType = NONREFLECTION;
		CoordTf::VECTOR4 lightst = { 100.0f,0.01f, 0.001f, 0.001f };//�����W, ����1, ����2, ����3
	};
	struct RtData {
		BufferSetRt vertexBuf = {};
		BufferSetRt indexBuf = {};
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		uint32_t vertexStride = 0;

		VulkanDevice::textureIdSet texId = {};

		VulkanDevice::Uniform<RtMaterial> mat = {};

		AccelerationStructure BLAS = {};
		std::vector<Instance> instance;
	};

private:
	struct Vertex3D_t {
		float pos[3] = {};
		float normal[3] = {};
		float tangent[3] = {};
		float difUv[2] = {};
		float speUv[2] = {};
	};

	bool rdataCreateF = false;
	uint32_t InstanceCnt = 0;

	void createVertexBuffer(RtData& rdata, uint32_t comIndex, Vertex3D_t* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void createBLAS(RtData& rdata);
	void updateInstance(RtData& rdata);
	void createTexture(RtData& rdata, uint32_t comIndex, VulkanDevice::textureIdSet& texId);

public:
	std::vector<RtData> Rdata;

	VulkanBasicPolygonRt();
	~VulkanBasicPolygonRt();

	void createMultipleMaterials(uint32_t comIndex, bool useAlpha, uint32_t numMat,
		Vertex3D_t* ver, uint32_t num, uint32_t** ind, uint32_t* indNum,
		VulkanDevice::textureIdSet* texid, uint32_t InstanceSize);

	void setMaterialType(vkMaterialType type, uint32_t matIndex);

	void create(uint32_t comIndex, bool useAlpha,
		VulkanDevice::Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum,
		int32_t difTexInd, int32_t norTexInd, int32_t speTexInd, uint32_t InstanceSize);

	void setMaterialParameter(CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient,
		uint32_t materialIndex = 0, float shininess = 4.0f, float RefractiveIndex = 0.0f);

	void instancing(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale);
	void instancingUpdate();
	void update(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale);
};

#endif
