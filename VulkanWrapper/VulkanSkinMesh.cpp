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
	bp = std::make_unique<VulkanBasicPolygon * []>(numMesh);
	bone = std::make_unique<Bone[]>(numBone);
	outPose = std::make_unique<MATRIX[]>(numBone);

	//各mesh読み込み
	for (uint32_t mI = 0; mI < numMesh; mI++) {
		bp[mI] = new VulkanBasicPolygon(device, comIndex);
		FbxMeshNode* mesh = fbx.getFbxMeshNode(mI);//mI番目のMesh取得
		auto index = mesh->getPolygonVertices();//頂点Index取得(頂点xyzに対してのIndex)
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

		if (cTexId) {
			if (cTexId[mI].diffuseId != -1)diffTexId = cTexId[mI].diffuseId;
			if (cTexId[mI].normalId != -1)norTexId = cTexId[mI].normalId;
		}

		//マテリアルカラー取得
		VECTOR3 diffuse = { (float)mesh->getDiffuseColor(0,0),(float)mesh->getDiffuseColor(0,1),(float)mesh->getDiffuseColor(0,2) };
		VECTOR3 specular = { (float)mesh->getSpecularColor(0,0),(float)mesh->getSpecularColor(0,1),(float)mesh->getSpecularColor(0,2) };
		VECTOR3 ambient = { (float)mesh->getAmbientColor(0,0),(float)mesh->getAmbientColor(0,1),(float)mesh->getAmbientColor(0,2) };

		//ボーン取得
		const uint32_t numBoneWei = 4;
		const uint32_t numArr = numBoneWei * mesh->getNumVertices();
		auto* boneWeiArr = new float[numArr];
		auto* boneWeiIndArr = new int32_t[numArr];
		for (uint32_t i = 0; i < numArr; i++) {
			boneWeiArr[i] = 0.0f;
			boneWeiIndArr[i] = 0;
		}
		for (uint32_t bI = 0; bI < mesh->getNumDeformer(); bI++) {
			Deformer* defo = mesh->getDeformer(bI);//meshのDeformer(bI)
			auto bNum = defo->getIndicesCount();//このボーンに影響を受ける頂点インデックス数
			auto bInd = defo->getIndices();//このボーンに影響を受ける頂点のインデックス配列
			auto bWei = defo->getWeights();//このボーンに影響を受ける頂点のウエイト配列
			for (int wI = 0; wI < bNum; wI++) {
				auto bindex = bInd[wI];//影響を受ける頂点
				auto weight = bWei[wI];//ウエイト
				for (int m = 0; m < numBoneWei; m++) {
					//各Bone毎に影響を受ける頂点のウエイトを一番大きい数値に更新していく
					auto ind = bindex * numBoneWei + m;
					if (weight > boneWeiArr[ind]) {//調べたウエイトの方が大きい
						boneWeiIndArr[ind] = bI;//Boneインデックス登録
						boneWeiArr[ind] = (float)weight;//ウエイト登録
						break;
					}
				}
			}
		}

		//ウエイト正規化
		for (uint32_t i1 = 0; i1 < mesh->getNumVertices(); i1++) {
			auto we = 0.0f;
			for (uint32_t m = 0; m < numBoneWei; m++) {
				auto ind = i1 * numBoneWei + m;
				we += boneWeiArr[ind];
			}
			auto we1 = 1.0f / we;
			for (uint32_t m = 0; m < numBoneWei; m++) {
				auto ind = i1 * numBoneWei + m;
				boneWeiArr[ind] *= we1;
			}
		}

		VertexSkin* verSkin = new VertexSkin[mesh->getNumPolygonVertices()];
		for (uint32_t vI = 0; vI < mesh->getNumPolygonVertices(); vI++) {
			VertexSkin* v = &verSkin[vI];
			v->pos[0] = (float)ver[index[vI] * 3];
			v->pos[1] = (float)ver[index[vI] * 3 + 1];
			v->pos[2] = (float)ver[index[vI] * 3 + 2];
			v->normal[0] = (float)nor[vI * 3];
			v->normal[1] = (float)nor[vI * 3 + 1];
			v->normal[2] = (float)nor[vI * 3 + 2];
			v->uv[0] = (float)uv[vI * 2];
			v->uv[1] = 1.0f - (float)uv[vI * 2 + 1];
			for (int vbI = 0; vbI < 4; vbI++) {
				v->bBoneWeight[vbI] = (float)boneWeiArr[index[vI] * 4 + vbI];
				v->bBoneIndex[vbI] = (float)boneWeiIndArr[index[vI] * 4 + vbI];
			}
		}
		ARR_DELETE(boneWeiArr);
		ARR_DELETE(boneWeiIndArr);

		//4頂点ポリゴン分割後のIndex数カウント
		uint32_t numNewIndex = 0;
		for (uint32_t i1 = 0; i1 < mesh->getNumPolygon(); i1++) {
			if (mesh->getPolygonSize(i1) == 3) {
				numNewIndex += 3;
			}
			if (mesh->getPolygonSize(i1) == 4) {
				numNewIndex += 6;
			}
		}

		//分割後のIndex生成, 順番を逆にする
		auto* newIndex = new uint32_t[numNewIndex];
		int nIcnt = 0;
		int Icnt = 0;
		for (uint32_t i1 = 0; i1 < mesh->getNumPolygon(); i1++) {
			if (mesh->getPolygonSize(i1) == 3) {
				newIndex[nIcnt++] = Icnt;
				newIndex[nIcnt++] = Icnt + 2;
				newIndex[nIcnt++] = Icnt + 1;
				Icnt += 3;
			}
			if (mesh->getPolygonSize(i1) == 4) {
				newIndex[nIcnt++] = Icnt;
				newIndex[nIcnt++] = Icnt + 2;
				newIndex[nIcnt++] = Icnt + 1;
				newIndex[nIcnt++] = Icnt;
				newIndex[nIcnt++] = Icnt + 3;
				newIndex[nIcnt++] = Icnt + 2;
				Icnt += 4;
			}
		}

		static VkVertexInputAttributeDescription attrDescs[] =
		{
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
			{ 3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8 },
			{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12 }
		};

		bp[mI]->create0<VertexSkin>(diffTexId, norTexId, verSkin, (uint32_t)mesh->getNumPolygonVertices(),
			newIndex, numNewIndex, attrDescs, 5, vsShaderSkinMesh, bp[mI]->fs);

		bp[mI]->setMaterialParameter(diffuse, specular, ambient);

		ARR_DELETE(verSkin);
		ARR_DELETE(newIndex);
	}

	//初期姿勢行列読み込み
	FbxMeshNode* mesh = fbx.getFbxMeshNode(0);//0番目のメッシュ取得,ここから行列を取得
	for (uint32_t i = 0; i < numBone; i++) {
		auto defo = mesh->getDeformer(i);
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				bone[i].bindPose.m[y][x] = (float)defo->getTransformLinkMatrix(y, x);
			}
		}
	}
}

void VulkanSkinMesh::setNewPoseMatrix(float time) {
	FbxMeshNode* mesh = fbx.getFbxMeshNode(0);
	Deformer de;
	auto ti = de.getTimeFRAMES60((int)time);
	for (uint32_t bI = 0; bI < numBone; bI++) {
		auto defo = mesh->getDeformer(bI);
		defo->EvaluateGlobalTransform(ti);
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				bone[bI].newPose.m[y][x] = (float)defo->getEvaluateGlobalTransform(y, x);
			}
		}
	}
}

MATRIX VulkanSkinMesh::getCurrentPoseMatrix(uint32_t index) {
	MATRIX inv;
	MatrixIdentity(&inv);
	MatrixInverse(&inv, &bone[index].bindPose);
	MATRIX ret;
	MatrixIdentity(&ret);
	MatrixMultiply(&ret, &inv, &bone[index].newPose);
	return ret;
}

void VulkanSkinMesh::setMaterialParameter(uint32_t meshIndex, VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient) {
	bp[meshIndex]->setMaterialParameter(diffuse, specular, ambient);
}

void VulkanSkinMesh::createChangeTextureArray(uint32_t num) {
	cTexId = std::make_unique<changeTextureId[]>(num);
}

void VulkanSkinMesh::setChangeTexture(uint32_t meshIndex, int diffuseTexId, int normalTexId) {
	if (cTexId) {
		cTexId[meshIndex].diffuseId = diffuseTexId;
		cTexId[meshIndex].normalId = normalTexId;
	}
}

void VulkanSkinMesh::draw(float time, VECTOR3 pos, VECTOR3 theta, VECTOR3 scale) {

	setNewPoseMatrix(time);

	for (uint32_t i = 0; i < numBone; i++)
		outPose[i] = getCurrentPoseMatrix(i);

	for (uint32_t i = 0; i < numMesh; i++)
		bp[i]->draw0(pos, theta, scale, outPose.get(), numBone);
}