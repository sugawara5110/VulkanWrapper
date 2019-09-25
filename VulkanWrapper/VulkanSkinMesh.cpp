//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanSkinMesh.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanSkinMesh.h"
#include "Shader/ShaderSkinMesh.h"

VulkanSkinMesh::VulkanSkinMesh(Device* dev, uint32_t comindex) {
	device = dev;
	comIndex = comindex;
}

VulkanSkinMesh::~VulkanSkinMesh() {
	//for (uint32_t i = 0; i < numMesh; i++)S_DELETE(bp[i]);
}

void VulkanSkinMesh::create(char* pass, float endfra) {
	//フレーム数
	endframe = endfra;
	//fbxファイルのpass入力する事で内部でデータの取得, 圧縮データの解凍を行ってます
	fbx.setFbxFile(pass);
	//mesh数取得
	numMesh = fbx.getNumFbxMeshNode();
	//bone数取得
	numBone = fbx.getFbxMeshNode(0)->getNumDeformer();
	//mesh数分BasicPolygon生成
	bp = std::make_unique<BasicPolygon[]>(numMesh);

	//各mesh読み込み
	for (uint32_t mI = 0; mI < numMesh; mI++) {
		auto mesh = fbx.getFbxMeshNode(mI);
		auto index = mesh->getPolygonVertices();
		auto ver = mesh->getVertices();
		auto nor = mesh->getNormal(0);
		auto uv = mesh->getAlignedUV(0);

	}
}