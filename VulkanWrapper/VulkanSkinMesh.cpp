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
	for (uint32_t i = 0; i < numMesh; i++)S_DELETE(bp[i]);
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
		FbxMeshNode* mesh = fbx.getFbxMeshNode(mI);//mI番目のMesh取得
		auto index = mesh->getPolygonVertices();//頂点Index取得
		auto ver = mesh->getVertices();//頂点取得
		auto nor = mesh->getNormal(0);//法線取得
		auto uv = mesh->getAlignedUV(0);//UV取得

		//ディフェーズテクスチャId取得, 無い場合ダミー
		int32_t diffTexId = -1;
		if (mesh->getDiffuseTextureName(0) != nullptr) {
			auto diffName = device->getNameFromPass(mesh->getDiffuseTextureName(0));
			diffTexId = device->getTextureNo(diffName);
		}
		//ノーマルテクスチャId取得, 無い場合ダミー
		int32_t norTexId = -1;
		if (mesh->getNormalTextureName(0) != nullptr) {
			auto norName = device->getNameFromPass(mesh->getNormalTextureName(0));
			norTexId = device->getTextureNo(norName);
		}

		//マテリアルカラー取得
		VECTOR3 diffuse = { (float)mesh->getDiffuseColor(0,0),(float)mesh->getDiffuseColor(0,1),(float)mesh->getDiffuseColor(0,2) };
		VECTOR3 specular = { (float)mesh->getSpecularColor(0,0),(float)mesh->getSpecularColor(0,1),(float)mesh->getSpecularColor(0,2) };
		VECTOR3 ambient = { (float)mesh->getAmbientColor(0,0),(float)mesh->getAmbientColor(0,1),(float)mesh->getAmbientColor(0,2) };

		//ボーン取得

	}
}