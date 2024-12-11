//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanSkinMeshRt.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "VulkanSkinMeshRt.h"
#include <string.h>
#include "Shader/Shader_Skinning.h"

using namespace std;

VulkanSkinMeshRt::VulkanSkinMeshRt() {
}

VulkanSkinMeshRt::~VulkanSkinMeshRt() {

	for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
		vkUtil::ARR_DELETE(sk[i]);
	}
	if (pvVB) {
		for (int i = 0; i < getNumMesh(); i++) {
			vkUtil::ARR_DELETE(pvVB[i]);
		}
	}
	if (pvVBM) {
		for (int i = 0; i < getNumMesh(); i++) {
			vkUtil::ARR_DELETE(pvVBM[i]);
		}
	}
	vkUtil::ARR_DELETE(pvVB);
	vkUtil::ARR_DELETE(pvVBM);
	vkUtil::ARR_DELETE(mObj);
	vkUtil::S_DELETE(mObject_BONES);
}

void VulkanSkinMeshRt::Vertex_hold() {
	pvVB_delete_f = false;
}

void VulkanSkinMeshRt::CreateBuffer(float end_frame, bool singleMesh, bool deformer) {
	float ef[1] = { end_frame };
	CreateBuffer(1, ef, singleMesh, deformer);
}

void VulkanSkinMeshRt::CreateBuffer(int num_end_frame, float* end_frame, bool singleMesh, bool deformer) {

	getBuffer(num_end_frame, end_frame, singleMesh, deformer);

	using namespace CoordTf;

	textureId = NEW VulkanDevice::textureIdSetInput * [getNumMesh()];

	if (maxNumBone > 0) {
		mObject_BONES = NEW VulkanDevice::Uniform<SHADER_GLOBAL_BONES>(1);
	}

	pvVB = NEW MY_VERTEX_S * [getNumMesh()];
	pvVBM = NEW VulkanBasicPolygonRt::Vertex3D_t * [getNumMesh()];
	mObj = NEW VulkanBasicPolygonRt[getNumMesh()];
	if (deformer) {
		for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
			sk[i] = NEW SkinningCom[getNumMesh()];
		}
	}
}

void VulkanSkinMeshRt::SetVertex(bool lclOn, bool axisOn) {

	Skin_VERTEX_Set vset = setVertex(lclOn, axisOn, false);
	NumNewIndex = vset.NumNewIndex;
	newIndex = vset.newIndex;

	using Vertex3D_t = VulkanBasicPolygonRt::Vertex3D_t;

	using namespace CoordTf;
	for (int m = 0; m < getNumMesh(); m++) {

		FbxLoader* fbL = getFbxLoader();
		FbxMeshNode* mesh = fbL->getFbxMeshNode(m);

		Skin_VERTEX* svb = vset.pvVB[m];
		Vertex_M* vb_m = vset.pvVB_M[m];

		MY_VERTEX_S* vb = pvVB[m] = NEW MY_VERTEX_S[mesh->getNumPolygonVertices()];
		Vertex3D_t* vbm = pvVBM[m] = NEW Vertex3D_t[mesh->getNumPolygonVertices()];

		for (unsigned int i = 0; i < mesh->getNumPolygonVertices(); i++) {
			Vertex3D_t* v = &vbm[i];
			Vertex_M* vv = &vb_m[i];
			v->pos[0] = vv->Pos.x;
			v->pos[1] = vv->Pos.y;
			v->pos[2] = vv->Pos.z;
			v->normal[0] = vv->normal.x;
			v->normal[1] = vv->normal.y;
			v->normal[2] = vv->normal.z;
			v->tangent[0] = vv->tangent.x;
			v->tangent[1] = vv->tangent.y;
			v->tangent[2] = vv->tangent.z;
			v->difUv[0] = vv->tex0.x;
			v->difUv[1] = vv->tex0.y;
			v->speUv[0] = vv->tex1.x;
			v->speUv[1] = vv->tex1.y;

			MY_VERTEX_S* vs = &vb[i];
			Skin_VERTEX* vvs = &svb[i];
			vs->vPos = vvs->vPos;
			vs->vNorm = vvs->vNorm;
			vs->vTangent = vvs->vTangent;
			vs->vTex0 = vvs->vTex0;
			vs->vTex1 = vvs->vTex1;
			memcpy(vs->bBoneIndex, vvs->bBoneIndex, sizeof(uint32_t) * 4);
			memcpy(vs->bBoneWeight, vvs->bBoneWeight, sizeof(float) * 4);
		}
		vkUtil::ARR_DELETE(vset.pvVB[m]);
		vkUtil::ARR_DELETE(vset.pvVB_M[m]);

		char* uv0Name = nullptr;      //テクスチャUVSet名0
		char* uv1Name = nullptr;      //テクスチャUVSet名1
		if (mesh->getNumUVObj() > 1) {
			uv0Name = mesh->getUVName(0);
			uv1Name = mesh->getUVName(1);
		}
		auto numMaterial = mesh->getNumMaterial();
		int* uvSw = NEW int[numMaterial];
		createMaterial(m, numMaterial, mesh, uv0Name, uv1Name, uvSw);
		swapTex(vb, sizeof(MY_VERTEX_S), mesh, uvSw);
		swapTex(vbm, sizeof(VulkanBasicPolygonRt::Vertex3D_t), mesh, uvSw);
		vkUtil::ARR_DELETE(uvSw);
	}
	vkUtil::ARR_DELETE(vset.pvVB);
	vkUtil::ARR_DELETE(vset.pvVB_M);
}

void VulkanSkinMeshRt::createMaterial(int meshInd, uint32_t numMaterial, FbxMeshNode* mesh,
	char* uv0Name, char* uv1Name, int* uvSw) {

	int m = meshInd;

	VulkanDevice* dev = VulkanDevice::GetInstance();

	textureId[m] = NEW VulkanDevice::textureIdSetInput[numMaterial];
	VulkanDevice::textureIdSetInput* texId = textureId[m];

	char difUvName[256] = {};
	char norUvName[256] = {};
	char speUvName[256] = {};

	for (uint32_t i = 0; i < numMaterial; i++) {
		//ディフェーズテクスチャId取得
		for (int tNo = 0; tNo < mesh->getNumDiffuseTexture(i); tNo++) {
			textureType type = mesh->getDiffuseTextureType(i, tNo);
			if (type.DiffuseColor && !type.SpecularColor ||
				mesh->getNumDiffuseTexture(i) == 1) {
				auto diffName = vkUtil::getNameFromPass(mesh->getDiffuseTextureName(i, tNo));
				texId[i].diffuseId = dev->getTextureNo(diffName);
				auto str = mesh->getDiffuseTextureUVName(i, tNo);
				if (str)strcpy(difUvName, str);
				break;
			}
		}
		//スペキュラテクスチャId取得
		for (int tNo = 0; tNo < mesh->getNumDiffuseTexture(i); tNo++) {
			textureType type = mesh->getDiffuseTextureType(i, tNo);
			if (type.SpecularColor) {
				auto speName = vkUtil::getNameFromPass(mesh->getDiffuseTextureName(i, tNo));
				texId[i].specularId = dev->getTextureNo(speName);
				auto str = mesh->getDiffuseTextureUVName(i, tNo);
				if (str)strcpy(speUvName, str);
				break;
			}
		}
		//ノーマルテクスチャId取得
		for (int tNo = 0; tNo < mesh->getNumNormalTexture(i); tNo++) {
			//ディフェーズテクスチャ用のノーマルマップしか使用しないので
			//取得済みのディフェーズテクスチャUV名と同じUV名のノーマルマップを探す
			auto uvNorName = mesh->getNormalTextureUVName(i, tNo);
			if (mesh->getNumNormalTexture(i) == 1 ||
				uvNorName && !strcmp(difUvName, uvNorName)) {
				auto norName = vkUtil::getNameFromPass(mesh->getNormalTextureName(i, tNo));
				texId[i].normalId = dev->getTextureNo(norName);
				auto str = mesh->getNormalTextureUVName(i, tNo);
				if (str)strcpy(norUvName, str);
				break;
			}
		}
		uvSw[i] = 0;
		if (uv0Name != nullptr) {
			//uv逆転
			if (!strcmp(difUvName, uv1Name) &&
				(!strcmp(speUvName, "") || !strcmp(speUvName, uv0Name)))
				uvSw[i] = 1;
			//どちらもuv0
			if (!strcmp(difUvName, uv0Name) &&
				(!strcmp(speUvName, "") || !strcmp(speUvName, uv0Name)))
				uvSw[i] = 2;
			//どちらもuv1
			if (!strcmp(difUvName, uv1Name) &&
				(!strcmp(speUvName, "") || !strcmp(speUvName, uv1Name)))
				uvSw[i] = 3;
		}
	}
}

static void swap(CoordTf::VECTOR2* a, CoordTf::VECTOR2* b) {
	float tmp;
	tmp = a->x;
	a->x = b->x;
	b->x = tmp;
	tmp = a->y;
	a->y = b->y;
	b->y = tmp;
}

static void swaptex(void* v, int uvSw) {
	char* byte = (char*)v;//頂点構造体の先頭アドレス
	CoordTf::VECTOR2* Tex0 = (CoordTf::VECTOR2*)(byte + sizeof(float) * 9);//pos + nor + tan = 9 * float  
	CoordTf::VECTOR2* Tex1 = (CoordTf::VECTOR2*)(byte + sizeof(float) * 11);//pos + nor + tan + tex0 = 11 * float

	switch (uvSw) {
	case 0:
		break;
	case 1:
		swap(Tex0, Tex1);
		break;
	case 2:
		Tex1 = Tex0;
		break;
	case 3:
		Tex0 = Tex1;
		break;
	}
}

void VulkanSkinMeshRt::swapTex(void* vb, uint32_t Stride, FbxMeshNode* mesh, int* uvSw) {
	char* byte = (char*)vb;
	int Icnt = 0;
	for (uint32_t i = 0; i < mesh->getNumPolygon(); i++) {
		uint32_t currentMatNo = mesh->getMaterialNoOfPolygon(i);
		uint32_t pSize = mesh->getPolygonSize(i);
		if (pSize >= 3) {
			for (uint32_t ps = 0; ps < pSize; ps++) {

				swaptex(&byte[(Icnt + ps) * Stride], uvSw[currentMatNo]);
			}
			Icnt += pSize;
		}
	}
}

void VulkanSkinMeshRt::SetDiffuseTextureName(char* textureName, int materialIndex, int meshIndex) {
	textureId[meshIndex][materialIndex].diffuseId = VulkanDevice::GetInstance()->getTextureNo(textureName);
}

void VulkanSkinMeshRt::SetNormalTextureName(char* textureName, int materialIndex, int meshIndex) {
	textureId[meshIndex][materialIndex].normalId = VulkanDevice::GetInstance()->getTextureNo(textureName);
}

void VulkanSkinMeshRt::SetSpeculerTextureName(char* textureName, int materialIndex, int meshIndex) {
	textureId[meshIndex][materialIndex].specularId = VulkanDevice::GetInstance()->getTextureNo(textureName);
}

void VulkanSkinMeshRt::setMaterialType(vkMaterialType type, uint32_t meshIndex, uint32_t matIndex) {
	mObj[meshIndex].Rdata[matIndex].mat.MaterialType.x = (float)type;
}

void VulkanSkinMeshRt::LightOn(bool on, uint32_t meshIndex, uint32_t InstanceIndex, uint32_t matIndex,
	float range, float att1, float att2, float att3) {

	mObj[meshIndex].LightOn(on, InstanceIndex, matIndex, range, att1, att2, att3);
}

bool VulkanSkinMeshRt::CreateFromFBX(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, uint32_t numInstance) {

	FbxLoader* fbL = getFbxLoader();

	for (int i = 0; i < getNumMesh(); i++) {

		FbxMeshNode* mesh = fbL->getFbxMeshNode(i);

		VulkanBasicPolygonRt& o = mObj[i];

		o.createMultipleMaterials(QueueIndex, comIndex, useAlpha, (uint32_t)mesh->getNumMaterial(),
			pvVBM[i], mesh->getNumPolygonVertices(), newIndex[i], NumNewIndex[i],
			textureId[i], numInstance, true);

		if (sk[0]) {
			vkUtil::createTangent((uint32_t)mesh->getNumMaterial(), NumNewIndex[i],
				pvVB[i], newIndex[i], sizeof(MY_VERTEX_S), 0, 3 * 4, 9 * 4, 6 * 4);

			for (uint32_t i1 = 0; i1 < VulkanBasicPolygonRt::numSwap; i1++) {
				sk[i1][i].createVertexBuffer(QueueIndex, comIndex, pvVB[i], mesh->getNumPolygonVertices());
				sk[i1][i].CreateLayouts();
				sk[i1][i].CreateComputePipeline();
				sk[i1][i].CreateDescriptorSets(&o.Rdata[0].vertexBuf[i1]->info, &mObject_BONES->getBufferSet()->info);
			}
		}

		if (pvVB_delete_f) {
			vkUtil::ARR_DELETE(pvVBM[i]);
			vkUtil::ARR_DELETE(pvVB[i]);
		}
		for (int m = 0; m < o.Rdata.size(); m++) {
			vkUtil::ARR_DELETE(newIndex[i][m]);

			float DiffuseFactor = (float)mesh->getDiffuseFactor(m);
			float SpecularFactor = (float)mesh->getSpecularFactor(m);
			float AmbientFactor = (float)mesh->getAmbientFactor(m);

			o.setMaterialColor(
				//拡散反射光
				{
			(float)mesh->getDiffuseColor(m, 0) * DiffuseFactor,
			(float)mesh->getDiffuseColor(m, 1) * DiffuseFactor,
			(float)mesh->getDiffuseColor(m, 2) * DiffuseFactor
				},
				//スペキュラー
				{
			(float)mesh->getSpecularColor(m, 0) * SpecularFactor,
			(float)mesh->getSpecularColor(m, 1) * SpecularFactor,
			(float)mesh->getSpecularColor(m, 2) * SpecularFactor
				},
				//アンビエント
				{
			(float)mesh->getAmbientColor(m, 0) * AmbientFactor,
			(float)mesh->getAmbientColor(m, 1) * AmbientFactor,
			(float)mesh->getAmbientColor(m, 2) * AmbientFactor
				},
				m);
		}
		vkUtil::ARR_DELETE(NumNewIndex[i]);
		vkUtil::ARR_DELETE(textureId[i]);
		vkUtil::ARR_DELETE(newIndex[i]);
	}
	if (pvVB_delete_f) {
		vkUtil::ARR_DELETE(pvVBM);
		vkUtil::ARR_DELETE(pvVB);
	}
	vkUtil::ARR_DELETE(NumNewIndex);
	vkUtil::ARR_DELETE(textureId);
	vkUtil::ARR_DELETE(newIndex);
	return true;
}

CoordTf::VECTOR3 VulkanSkinMeshRt::GetVertexPosition(int meshIndex, int verNum, float adjustZ, float adjustY, float adjustX,
	float thetaZ, float thetaY, float thetaX, float scal) {

	using namespace CoordTf;
	//頂点にボーン行列を掛け出力
	VECTOR3 ret, pos;
	MATRIX rotZ, rotY, rotX, rotZY, rotZYX;
	MATRIX scale, scaro;
	MatrixScaling(&scale, scal, scal, scal);
	MatrixRotationZ(&rotZ, thetaZ);
	MatrixRotationY(&rotY, thetaY);
	MatrixRotationX(&rotX, thetaX);
	MatrixMultiply(&rotZY, &rotZ, &rotY);
	MatrixMultiply(&rotZYX, &rotZY, &rotX);
	MatrixMultiply(&scaro, &scale, &rotZYX);
	ret.x = ret.y = ret.z = 0.0f;

	for (int i = 0; i < 4; i++) {
		pos.x = pvVB[meshIndex][verNum].vPos.x;
		pos.y = pvVB[meshIndex][verNum].vPos.y;
		pos.z = pvVB[meshIndex][verNum].vPos.z;
		MATRIX m = GetCurrentPoseMatrix(pvVB[meshIndex][verNum].bBoneIndex[i]);
		VectorMatrixMultiply(&pos, &m);
		VectorMultiply(&pos, pvVB[meshIndex][verNum].bBoneWeight[i]);
		VectorAddition(&ret, &ret, &pos);
	}
	ret.x += adjustX;
	ret.y += adjustY;
	ret.z += adjustZ;
	VectorMatrixMultiply(&ret, &scaro);
	return ret;
}

void VulkanSkinMeshRt::Instancing(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor) {

	for (int i = 0; i < getNumMesh(); i++) {
		if (!isNoUseMesh(i))
			mObj[i].instancing(pos, theta, scale, addColor);
	}
}

bool VulkanSkinMeshRt::InstancingUpdate(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex, int AnimationIndex, float ti, int InternalAnimationIndex) {

	bool frame_end = false;
	int insnum = 0;
	if (ti != -1.0f)frame_end = SetNewPoseMatrices(ti, AnimationIndex, InternalAnimationIndex);
	MatrixMap_Bone(&sgb[0], false);//Vulkanは転置無し

	if (sk[swapIndex])mObject_BONES->update(0, &sgb[0]);

	for (int i = 0; i < getNumMesh(); i++) {
		if (!isNoUseMesh(i)) {
			if (sk[swapIndex])sk[swapIndex][i].Skinned(QueueIndex, comIndex);
			mObj[i].instancingUpdate(swapIndex, QueueIndex, comIndex);
		}
	}

	return frame_end;
}

bool VulkanSkinMeshRt::Update(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex,
	int AnimationIndex, float time,
	CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor,
	int InternalAnimationIndex) {

	Instancing(pos, theta, scale, addColor);
	return InstancingUpdate(swapIndex, QueueIndex, comIndex, AnimationIndex, time, InternalAnimationIndex);
}

void VulkanSkinMeshRt::setMaterialColor(CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient,
	uint32_t meshIndex,
	uint32_t materialIndex) {

	mObj[meshIndex].setMaterialColor(diffuse, specular, ambient, materialIndex);
}

void VulkanSkinMeshRt::setMaterialShininess(float shininess,
	uint32_t meshIndex,
	uint32_t materialIndex) {

	mObj[meshIndex].setMaterialShininess(shininess, materialIndex);
}

void VulkanSkinMeshRt::setMaterialRefractiveIndex(float RefractiveIndex,
	uint32_t meshIndex,
	uint32_t materialIndex) {

	mObj[meshIndex].setMaterialRefractiveIndex(RefractiveIndex, materialIndex);
}

VulkanSkinMeshRt::SkinningCom::~SkinningCom() {
	VulkanDevice* dev = VulkanDevice::GetInstance();
	Buf.destroy();
	_vkDestroyPipeline(dev->getDevice(), computeSkiningPipeline, nullptr);
	_vkDestroyPipelineLayout(dev->getDevice(), pipelineLayoutSkinned, nullptr);
	_vkDestroyDescriptorSetLayout(dev->getDevice(), dsLayoutSkinned, nullptr);
	dev->DeallocateDescriptorSet(descriptorSetCompute);
}

void VulkanSkinMeshRt::SkinningCom::createVertexBuffer(uint32_t QueueIndex, uint32_t comIndex, MY_VERTEX_S* ver, uint32_t num) {

	struct MY_VERTEX_S_4 {
		CoordTf::VECTOR4 vPos = {};
		CoordTf::VECTOR4 vNorm = {};
		CoordTf::VECTOR4 vTangent = {};
		CoordTf::VECTOR4 vTex0 = {};
		CoordTf::VECTOR4 vTex1 = {};
		CoordTf::VECTOR4 bBoneIndex = {};
		CoordTf::VECTOR4 bBoneWeight = {};
	};

	MY_VERTEX_S_4* v = NEW MY_VERTEX_S_4[num];
	numVer = num;

	for (uint32_t i = 0; i < num; i++) {
		memcpy(&v[i].vPos, &ver[i].vPos, sizeof(float) * 3);
		memcpy(&v[i].vNorm, &ver[i].vNorm, sizeof(float) * 3);
		memcpy(&v[i].vTangent, &ver[i].vTangent, sizeof(float) * 3);
		memcpy(&v[i].vTex0, &ver[i].vTex0, sizeof(float) * 2);
		memcpy(&v[i].vTex1, &ver[i].vTex1, sizeof(float) * 2);
		v[i].bBoneIndex.as(
			(float)ver[i].bBoneIndex[0],
			(float)ver[i].bBoneIndex[1],
			(float)ver[i].bBoneIndex[2],
			(float)ver[i].bBoneIndex[3]);
		memcpy(&v[i].bBoneWeight, &ver[i].bBoneWeight, sizeof(float) * 4);
	}

	VkBufferUsageFlags usageForRT =
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr,
	};
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	void* pNext = &memoryAllocateFlagsInfo;

	Buf.createVertexBuffer(QueueIndex, comIndex, v, num, false, pNext, &usageForRT);
	vkUtil::ARR_DELETE(v);
}

void VulkanSkinMeshRt::SkinningCom::CreateLayouts() {

	VkDescriptorSetLayoutBinding layoutIn{};
	layoutIn.binding = 0;
	layoutIn.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutIn.descriptorCount = 1;
	layoutIn.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutBinding layoutUBO{};
	layoutUBO.binding = 1;
	layoutUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutUBO.descriptorCount = 1;
	layoutUBO.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutBinding layoutOut{};
	layoutOut.binding = 2;
	layoutOut.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutOut.descriptorCount = 1;
	layoutOut.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bind = { layoutIn,layoutUBO,layoutOut };

	VkDescriptorSetLayoutCreateInfo dsLayoutCI{
	   VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
	};
	dsLayoutCI.bindingCount = uint32_t(bind.size());
	dsLayoutCI.pBindings = bind.data();

	VulkanDevice* dev = VulkanDevice::GetInstance();

	_vkCreateDescriptorSetLayout(
		dev->getDevice(), &dsLayoutCI, nullptr, &dsLayoutSkinned);

	VkPipelineLayoutCreateInfo pipelineLayoutCI{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
	};
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &dsLayoutSkinned;
	_vkCreatePipelineLayout(dev->getDevice(),
		&pipelineLayoutCI, nullptr, &pipelineLayoutSkinned);
}

void VulkanSkinMeshRt::SkinningCom::CreateComputePipeline() {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	auto shaderStage = dev->createShaderModule("SkinMesh", Shader_Skinning, VK_SHADER_STAGE_COMPUTE_BIT);

	VkComputePipelineCreateInfo compPipelineCI{
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
	};
	compPipelineCI.layout = pipelineLayoutSkinned;
	compPipelineCI.stage = shaderStage;

	_vkCreateComputePipelines(dev->getDevice(), VK_NULL_HANDLE, 1, &compPipelineCI, nullptr, &computeSkiningPipeline);
	_vkDestroyShaderModule(dev->getDevice(), shaderStage.module, nullptr);
}

void VulkanSkinMeshRt::SkinningCom::CreateDescriptorSets(VkDescriptorBufferInfo* output, VkDescriptorBufferInfo* bone) {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	descriptorSetCompute = dev->AllocateDescriptorSet(dsLayoutSkinned);

	VkWriteDescriptorSet writeIn{
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
	};
	writeIn.dstSet = descriptorSetCompute;
	writeIn.dstBinding = 0;
	writeIn.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeIn.descriptorCount = 1;
	writeIn.pBufferInfo = &Buf.info;

	VkWriteDescriptorSet writeUBO{
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
	};
	writeUBO.dstSet = descriptorSetCompute;
	writeUBO.dstBinding = 1;
	writeUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeUBO.descriptorCount = 1;
	writeUBO.pBufferInfo = bone;

	VkWriteDescriptorSet writeOut{
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
	};
	writeOut.dstSet = descriptorSetCompute;
	writeOut.dstBinding = 2;
	writeOut.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeOut.descriptorCount = 1;
	writeOut.pBufferInfo = output;

	std::vector<VkWriteDescriptorSet> writeSets = { writeIn, writeUBO, writeOut };

	_vkUpdateDescriptorSets(dev->getDevice(), uint32_t(writeSets.size()), writeSets.data(), 0, nullptr);
}

void VulkanSkinMeshRt::SkinningCom::Skinned(uint32_t QueueIndex, uint32_t comIndex) {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	auto command = dev->getCommandObj(QueueIndex)->getCommandBuffer(comIndex);

	_vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, computeSkiningPipeline);
	_vkCmdBindDescriptorSets(
		command, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipelineLayoutSkinned, 0,
		1, &descriptorSetCompute,
		0,
		nullptr);

	_vkCmdDispatch(command, numVer, 1, 1);

	VkMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,
	};
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	_vkCmdPipelineBarrier(
		command,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0, 1, &barrier,
		0, nullptr,
		0, nullptr);
}