//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanSkinMesh.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "VulkanSkinMesh.h"
#include "Shader/ShaderSkinMesh.h"

void VulkanSkinMesh::setfbx() {
	//mesh数取得
	numMesh = fbxObj[0]->fbx.getNumFbxMeshNode();
	//bone数取得
	numBone = fbxObj[0]->fbx.getFbxMeshNode(0)->getNumDeformer();
	fbxObj[0]->defo = new Deformer * [numBone];
	for (uint32_t i = 0; i < numBone; i++) {
		fbxObj[0]->defo[i] = fbxObj[0]->fbx.getFbxMeshNode(0)->getDeformer(i);
	}
	//mesh数分BasicPolygon生成
	bp = std::make_unique<VulkanBasicPolygon* []>(numMesh);
	bone = std::make_unique<Bone[]>(numBone);
	outPose = std::make_unique<CoordTf::MATRIX[]>(numBone);

	cTexId = std::make_unique<VulkanDevice::textureIdSet* []>(numMesh);
	for (uint32_t i = 0; i < numMesh; i++)cTexId[i] = new VulkanDevice::textureIdSet[fbxObj[0]->fbx.getFbxMeshNode(i)->getNumMaterial()];
}

void VulkanSkinMesh::setFbx(char* pass, float endfra) {
	fbxObj[0] = new FbxObj();
	//フレーム数
	fbxObj[0]->endframe = endfra;
	//fbxファイルのpass入力する事で内部でデータの取得, 圧縮データの解凍を行ってます
	fbxObj[0]->fbx.setFbxFile(pass);
	setfbx();
}

void VulkanSkinMesh::setFbxInByteArray(char* byteArray, unsigned int size, float endfra) {
	fbxObj[0] = new FbxObj();
	fbxObj[0]->endframe = endfra;
	fbxObj[0]->fbx.setBinaryInFbxFile(byteArray, size);
	setfbx();
}

static char* getName(char* in) {
	char* out = in;
	int len = (int)strlen(out);
	out += len;
	int cnt = 0;
	for (cnt = 0; cnt < len; cnt++) {
		out--;
		if (*out == ' ') {
			out++;
			break;
		}
	}
	return out;
}

void VulkanSkinMesh::setAnimation() {
	fbxObj[numFbxObj]->defo = new Deformer * [numBone];
	uint32_t numNoneMeshBone = fbxObj[numFbxObj]->fbx.getNumNoneMeshDeformer();
	for (uint32_t j = 0; j < numBone; j++) {
		char* bName1 = getName(fbxObj[0]->defo[j]->getName());
		int b1ln = (int)strlen(bName1);
		for (uint32_t i = 0; i < numNoneMeshBone; i++) {
			char* bName2 = getName(fbxObj[numFbxObj]->fbx.getNoneMeshDeformer(i)->getName());
			if (b1ln == strlen(bName2) && !strcmp(bName1, bName2)) {
				fbxObj[numFbxObj]->defo[j] = fbxObj[numFbxObj]->fbx.getNoneMeshDeformer(i);
				break;
			}
		}
	}
	numFbxObj++;
}

void VulkanSkinMesh::additionalAnimation(char* pass, float endframe) {
	fbxObj[numFbxObj] = new FbxObj();
	fbxObj[numFbxObj]->endframe = endframe;
	fbxObj[numFbxObj]->fbx.setFbxFile(pass);
	setAnimation();
}

void VulkanSkinMesh::additionalAnimationInByteArray(char* byteArray, unsigned int size, float endframe) {
	fbxObj[numFbxObj] = new FbxObj();
	fbxObj[numFbxObj]->endframe = endframe;
	fbxObj[numFbxObj]->fbx.setBinaryInFbxFile(byteArray, size);
	setAnimation();
}

VulkanSkinMesh::~VulkanSkinMesh() {
	for (uint32_t i = 0; i < numMesh; i++) {
		vkUtil::ARR_DELETE(cTexId[i]);
		vkUtil::S_DELETE(bp[i]);
	}
	for (uint32_t i = 0; i < numFbxObj; i++) {
		vkUtil::S_DELETE(fbxObj[i]);
	}
}

void VulkanSkinMesh::create(uint32_t comIndex, bool useAlpha) {
	//各mesh読み込み
	for (uint32_t mI = 0; mI < numMesh; mI++) {
		bp[mI] = new VulkanBasicPolygon();
		FbxMeshNode* mesh = fbxObj[0]->fbx.getFbxMeshNode(mI);//mI番目のMesh取得
		auto index = mesh->getPolygonVertices();//頂点Index取得(頂点xyzに対してのIndex)
		auto ver = mesh->getVertices();//頂点取得
		auto nor = mesh->getNormal(0);//法線取得

		double* uv0 = mesh->getAlignedUV(0);//テクスチャUV0
		char* uv0Name = nullptr;            //テクスチャUVSet名0
		double* uv1 = nullptr;              //テクスチャUV1
		char* uv1Name = nullptr;            //テクスチャUVSet名1
		if (mesh->getNumUVObj() > 1) {
			uv1 = mesh->getAlignedUV(1);
			uv0Name = mesh->getUVName(0);
			uv1Name = mesh->getUVName(1);
		}
		else {
			uv1 = mesh->getAlignedUV(0);
		}

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
			v->difUv[0] = (float)uv0[vI * 2];
			v->difUv[1] = 1.0f - (float)uv0[vI * 2 + 1];
			v->speUv[0] = (float)uv1[vI * 2];
			v->speUv[1] = 1.0f - (float)uv1[vI * 2 + 1];
			for (int vbI = 0; vbI < 4; vbI++) {
				v->bBoneWeight[vbI] = (float)boneWeiArr[index[vI] * 4 + vbI];
				v->bBoneIndex[vbI] = (float)boneWeiIndArr[index[vI] * 4 + vbI];
			}
		}
		vkUtil::ARR_DELETE(boneWeiArr);
		vkUtil::ARR_DELETE(boneWeiIndArr);

		//4頂点ポリゴン分割後のIndex数カウント
		auto numMaterial = mesh->getNumMaterial();
		uint32_t* numNewIndex = new uint32_t[numMaterial];
		memset(numNewIndex, 0, sizeof(uint32_t) * numMaterial);
		int32_t currentMatNo = -1;
		for (uint32_t i1 = 0; i1 < mesh->getNumPolygon(); i1++) {
			if (mesh->getMaterialNoOfPolygon(i1) != currentMatNo) {
				currentMatNo = mesh->getMaterialNoOfPolygon(i1);
			}
			if (mesh->getPolygonSize(i1) == 3) {
				numNewIndex[currentMatNo] += 3;
			}
			if (mesh->getPolygonSize(i1) == 4) {
				numNewIndex[currentMatNo] += 6;
			}
		}

		//分割後のIndex生成, 順番を逆にする
		uint32_t** newIndex = new uint32_t * [numMaterial];
		for (uint32_t ind1 = 0; ind1 < numMaterial; ind1++) {
			if (numNewIndex[ind1] <= 0) {
				newIndex[ind1] = nullptr;
				continue;
			}
			newIndex[ind1] = new uint32_t[numNewIndex[ind1]];
		}
		std::unique_ptr<uint32_t[]> indexCnt;
		indexCnt = std::make_unique<uint32_t[]>(numMaterial);
		currentMatNo = -1;
		int Icnt = 0;
		for (uint32_t i1 = 0; i1 < mesh->getNumPolygon(); i1++) {
			if (mesh->getMaterialNoOfPolygon(i1) != currentMatNo) {
				currentMatNo = mesh->getMaterialNoOfPolygon(i1);
			}

			if (mesh->getPolygonSize(i1) == 3) {

				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt;
				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt + 2;
				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt + 1;

				Icnt += 3;
			}
			if (mesh->getPolygonSize(i1) == 4) {

				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt;
				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt + 2;
				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt + 1;
				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt;
				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt + 3;
				newIndex[currentMatNo][indexCnt[currentMatNo]++] = Icnt + 2;

				Icnt += 4;
			}
		}

		VulkanDevice::textureIdSet* texId = new VulkanDevice::textureIdSet[numMaterial];

		using namespace CoordTf;

		VulkanDevice* device = VulkanDevice::GetInstance();

		std::unique_ptr<VECTOR3[]> diffuse = std::make_unique<VECTOR3[]>(numMaterial);
		std::unique_ptr<VECTOR3[]> specular = std::make_unique<VECTOR3[]>(numMaterial);
		std::unique_ptr<VECTOR3[]> ambient = std::make_unique<VECTOR3[]>(numMaterial);
		float* uvSw = new float[numMaterial];
		memset(uvSw, 0, sizeof(float) * numMaterial);
		for (uint32_t matInd = 0; matInd < numMaterial; matInd++) {
			//ディフェーズテクスチャId取得, 無い場合ダミー
			for (int tNo = 0; tNo < mesh->getNumDiffuseTexture(matInd); tNo++) {
				textureType type = mesh->getDiffuseTextureType(matInd, tNo);
				if (type.DiffuseColor && !type.SpecularColor ||
					mesh->getNumDiffuseTexture(matInd) == 1) {
					auto diffName = vkUtil::getNameFromPass(
						mesh->getDiffuseTextureName(matInd, tNo));
					texId[matInd].diffuseId = device->getTextureNo(diffName);
					auto str = mesh->getDiffuseTextureUVName(matInd, tNo);
					strcpy(texId[matInd].difUvName, str);
					break;
				}
			}
			//スペキュラテクスチャId取得, 無い場合ダミー
			for (int tNo = 0; tNo < mesh->getNumDiffuseTexture(matInd); tNo++) {
				textureType type = mesh->getDiffuseTextureType(matInd, tNo);
				if (type.SpecularColor) {
					auto speName = vkUtil::getNameFromPass(
						mesh->getDiffuseTextureName(matInd, tNo));
					texId[matInd].specularId = device->getTextureNo(speName);
					auto str = mesh->getDiffuseTextureUVName(matInd, tNo);
					strcpy(texId[matInd].speUvName, str);
					break;
				}
			}
			//ノーマルテクスチャId取得, 無い場合ダミー
			for (int tNo = 0; tNo < mesh->getNumNormalTexture(matInd); tNo++) {
				//ディフェーズテクスチャ用のノーマルマップしか使用しないので
				//取得済みのディフェーズテクスチャUV名と同じUV名のノーマルマップを探す
				if (!strcmp(texId[matInd].difUvName, mesh->getNormalTextureUVName(matInd, tNo)) ||
					mesh->getNumNormalTexture(matInd) == 1) {
					auto norName = vkUtil::getNameFromPass(mesh->getNormalTextureName(matInd, tNo));
					texId[matInd].normalId = device->getTextureNo(norName);
					auto str = mesh->getNormalTextureUVName(matInd, tNo);
					strcpy(texId[matInd].norUvName, str);
					break;
				}
			}
			if (uv0Name != nullptr) {
				char* difName = texId[matInd].difUvName;
				char* speName = texId[matInd].speUvName;
				//uv逆転
				if (!strcmp(difName, uv1Name) &&
					(!strcmp(speName, "") || !strcmp(speName, uv0Name)))
					uvSw[matInd] = 1.0f;
				//どちらもuv0
				if (!strcmp(difName, uv0Name) &&
					(!strcmp(speName, "") || !strcmp(speName, uv0Name)))
					uvSw[matInd] = 2.0f;
				//どちらもuv1
				if (!strcmp(difName, uv1Name) &&
					(!strcmp(speName, "") || !strcmp(speName, uv1Name)))
					uvSw[matInd] = 3.0f;
			}

			if (cTexId[mI][matInd].diffuseId != -1)
				texId[matInd].diffuseId = cTexId[mI][matInd].diffuseId;
			if (cTexId[mI][matInd].normalId != -1)
				texId[matInd].normalId = cTexId[mI][matInd].normalId;
			if (cTexId[mI][matInd].specularId != -1)
				texId[matInd].specularId = cTexId[mI][matInd].specularId;

			//マテリアルカラー取得
			diffuse[matInd] = { (float)mesh->getDiffuseColor(matInd, 0),
							   (float)mesh->getDiffuseColor(matInd, 1),
							   (float)mesh->getDiffuseColor(matInd, 2) };
			specular[matInd] = { (float)mesh->getSpecularColor(matInd, 0),
								(float)mesh->getSpecularColor(matInd, 1),
								(float)mesh->getSpecularColor(matInd, 2) };
			ambient[matInd] = { (float)mesh->getAmbientColor(matInd, 0),
							   (float)mesh->getAmbientColor(matInd, 1),
							   (float)mesh->getAmbientColor(matInd, 2) };
		}

		uint32_t numattrDescs = 6;
		static VkVertexInputAttributeDescription attrDescs[] =
		{
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT,    0},
				{1, 0, VK_FORMAT_R32G32B32_SFLOAT,    sizeof(float) * 3},
				{2, 0, VK_FORMAT_R32G32_SFLOAT,       sizeof(float) * 6},
				{3, 0, VK_FORMAT_R32G32_SFLOAT,       sizeof(float) * 8},
				{4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10},
				{5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14}
		};
		bp[mI]->create0<VertexSkin>(comIndex, useAlpha, numMaterial, texId, uvSw, verSkin,
			(uint32_t)mesh->getNumPolygonVertices(),
			newIndex, numNewIndex, attrDescs, numattrDescs, vsShaderSkinMesh,
			bp[mI]->fs);

		for (uint32_t sw = 0; sw < bp[0]->numSwap; sw++) {
			for (uint32_t matInd = 0; matInd < numMaterial; matInd++) {
				if (numNewIndex[matInd] <= 0)continue;
				bp[mI]->setMaterialParameter(sw, diffuse[matInd], specular[matInd], ambient[matInd],
					matInd);
			}
		}

		using namespace vkUtil;
		for (uint32_t ind1 = 0; ind1 < numMaterial; ind1++)ARR_DELETE(newIndex[ind1]);
		ARR_DELETE(newIndex);
		ARR_DELETE(numNewIndex);
		ARR_DELETE(verSkin);
		ARR_DELETE(texId);
		ARR_DELETE(uvSw);
	}

	//初期姿勢行列読み込み
	FbxMeshNode* mesh = fbxObj[0]->fbx.getFbxMeshNode(0);//0番目のメッシュ取得,ここから行列を取得
	for (uint32_t i = 0; i < numBone; i++) {
		auto defo = mesh->getDeformer(i);
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				bone[i].bindPose.m[y][x] = (float)defo->getTransformLinkMatrix(y, x);
			}
		}
	}
}

void VulkanSkinMesh::setNewPoseMatrix(uint32_t animationIndex, float time) {
	Deformer** defoArr = fbxObj[animationIndex]->defo;
	Deformer de;
	auto ti = de.getTimeFRAMES60((int)time);
	for (uint32_t bI = 0; bI < numBone; bI++) {
		auto defo = defoArr[bI];
		defo->EvaluateGlobalTransform(ti);
		Bone* bo = &bone[bI];
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				bo->newPose.m[y][x] = (float)defo->getEvaluateGlobalTransform(y, x);
			}
		}
	}
}

void VulkanSkinMesh::copyConnectionPoseMatrix(uint32_t nextAnimationIndex) {
	Deformer** defoArr = fbxObj[nextAnimationIndex]->defo;
	for (uint32_t bI = 0; bI < numBone; bI++) {
		auto defo = defoArr[bI];
		defo->EvaluateGlobalTransform(0);
		Bone* bo = &bone[bI];
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				bo->connectLastPose.m[y][x] = (float)defo->getEvaluateGlobalTransform(y, x);
				bo->connectFirstPose.m[y][x] = bo->newPose.m[y][x];
			}
		}
	}
}

void VulkanSkinMesh::setNewPoseMatrixConnection(float connectionRatio) {
	for (uint32_t bI = 0; bI < numBone; bI++) {
		Bone* bo = &bone[bI];
		StraightLinear(&bo->newPose, &bo->connectFirstPose, &bo->connectLastPose, connectionRatio);
	}
}

CoordTf::MATRIX VulkanSkinMesh::getCurrentPoseMatrix(uint32_t index) {

	using namespace CoordTf;
	MATRIX inv;
	MatrixIdentity(&inv);
	MatrixInverse(&inv, &bone[index].bindPose);
	MATRIX ret;
	MatrixIdentity(&ret);
	MatrixMultiply(&ret, &inv, &bone[index].newPose);
	return ret;
}

void VulkanSkinMesh::setMaterialParameter(uint32_t swapIndex, uint32_t meshIndex, uint32_t materialIndex,
	CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient) {

	bp[meshIndex]->setMaterialParameter(swapIndex, diffuse, specular, ambient, materialIndex);
}

void VulkanSkinMesh::setChangeTexture(uint32_t meshIndex, uint32_t materialIndex, int diffuseTexId, int normalTexId, int specularTexId) {
	cTexId[meshIndex][materialIndex].diffuseId = diffuseTexId;
	cTexId[meshIndex][materialIndex].normalId = normalTexId;
	cTexId[meshIndex][materialIndex].specularId = specularTexId;
}

void VulkanSkinMesh::subUpdate(uint32_t swapIndex, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale) {

	for (uint32_t i = 0; i < numBone; i++)
		outPose[i] = getCurrentPoseMatrix(i);

	for (uint32_t i = 0; i < numMesh; i++)
		bp[i]->update0(swapIndex, pos, theta, scale, outPose.get(), numBone);
}

void VulkanSkinMesh::update(uint32_t swapIndex, uint32_t animationIndex, float time,
	CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale) {

	setNewPoseMatrix(animationIndex, time);
	subUpdate(swapIndex, pos, theta, scale);
}

void VulkanSkinMesh::setConnectionPitch(float pitch) {
	connectionPitch = pitch;
}

bool VulkanSkinMesh::autoUpdate(uint32_t swapIndex, uint32_t animationIndex, float pitchTime,
	CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale) {

	if (prevAnimationIndex == -1)prevAnimationIndex = animationIndex;
	if (!connectionOn && prevAnimationIndex != animationIndex) {
		fbxObj[prevAnimationIndex]->currentframe = 0.0f;
		copyConnectionPoseMatrix(animationIndex);
		connectionOn = true;
		prevAnimationIndex = animationIndex;
	}
	if (connectionOn) {
		if (ConnectionRatio > 1.0f) {
			ConnectionRatio = 0.0f;
			connectionOn = false;
		}
		else {
			setNewPoseMatrixConnection(ConnectionRatio);
			subUpdate(swapIndex, pos, theta, scale);
			ConnectionRatio += connectionPitch * pitchTime;
		}
	}
	else {

		float cuframe = fbxObj[animationIndex]->currentframe;
		float enframe = fbxObj[animationIndex]->endframe;
		if (cuframe > enframe) {
			fbxObj[animationIndex]->currentframe = 0.0f;
			return false;
		}
		setNewPoseMatrix(animationIndex, cuframe);
		subUpdate(swapIndex, pos, theta, scale);
		fbxObj[animationIndex]->currentframe += pitchTime;
	}
	return true;
}

void VulkanSkinMesh::draw(uint32_t swapIndex, uint32_t comIndex) {
	for (uint32_t i = 0; i < numMesh; i++) {
		bp[i]->draw(swapIndex, comIndex);
	}
}