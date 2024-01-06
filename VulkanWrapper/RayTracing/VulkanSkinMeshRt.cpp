//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanSkinMeshRt.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#define FBX_PCS 32
#include "VulkanSkinMeshRt.h"
#include <string.h>
#include "Shader/Shader_Skinning.h"

using namespace std;

VulkanSkinMeshRt::SkinMesh_sub::SkinMesh_sub() {
	fbxL = NEW FbxLoader();
	CoordTf::MatrixIdentity(&rotZYX);
	connect_step = 3000.0f;
	InternalLastAnimationIndex = -1;
}

VulkanSkinMeshRt::SkinMesh_sub::~SkinMesh_sub() {
	vkUtil::S_DELETE(fbxL);
}

bool VulkanSkinMeshRt::SkinMesh_sub::Create(char* szFileName) {
	return fbxL->setFbxFile(szFileName);
}

bool VulkanSkinMeshRt::SkinMesh_sub::CreateSetBinary(char* byteArray, unsigned int size) {
	return fbxL->setBinaryInFbxFile(byteArray, size);
}

VulkanSkinMeshRt::VulkanSkinMeshRt() {
	memset(this, 0, sizeof(VulkanSkinMeshRt));
	fbx = NEW SkinMesh_sub[FBX_PCS];
	BoneConnect = -1.0f;
	AnimLastInd = -1;
}

VulkanSkinMeshRt::~VulkanSkinMeshRt() {

	for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
		vkUtil::ARR_DELETE(sk[i]);
	}
	if (pvVB) {
		for (int i = 0; i < numMesh; i++) {
			vkUtil::ARR_DELETE(pvVB[i]);
		}
	}
	if (pvVBM) {
		for (int i = 0; i < numMesh; i++) {
			vkUtil::ARR_DELETE(pvVBM[i]);
		}
	}
	vkUtil::ARR_DELETE(numBone);
	vkUtil::ARR_DELETE(pvVB);
	vkUtil::ARR_DELETE(pvVBM);
	vkUtil::ARR_DELETE(boneName);
	vkUtil::ARR_DELETE(m_ppSubAnimationBone);
	vkUtil::ARR_DELETE(m_pLastBoneMatrix);
	vkUtil::ARR_DELETE(m_BoneArray);
	vkUtil::ARR_DELETE(mObj);
	vkUtil::S_DELETE(mObject_BONES);

	DestroyFBX();
}

void VulkanSkinMeshRt::ObjCentering(bool f, int ind) {
	fbx[ind].centering = f;
	fbx[ind].offset = false;
	fbx[ind].cx = fbx[ind].cy = fbx[ind].cz = 0.0f;
}

void VulkanSkinMeshRt::CreateRotMatrix(float thetaZ, float thetaY, float thetaX, int ind) {
	CoordTf::MATRIX rotZ, rotY, rotX, rotZY;
	CoordTf::MatrixIdentity(&fbx[ind].rotZYX);
	CoordTf::MatrixRotationZ(&rotZ, thetaZ);
	CoordTf::MatrixRotationY(&rotY, thetaY);
	CoordTf::MatrixRotationX(&rotX, thetaX);
	CoordTf::MatrixMultiply(&rotZY, &rotZ, &rotY);
	CoordTf::MatrixMultiply(&fbx[ind].rotZYX, &rotZY, &rotX);
}

void VulkanSkinMeshRt::ObjCentering(float x, float y, float z, float thetaZ, float thetaY, float thetaX, int ind) {
	fbx[ind].centering = true;
	fbx[ind].offset = false;
	fbx[ind].cx = x;
	fbx[ind].cy = y;
	fbx[ind].cz = z;
	CreateRotMatrix(thetaZ, thetaY, thetaX, ind);
}

void VulkanSkinMeshRt::ObjOffset(float x, float y, float z, float thetaZ, float thetaY, float thetaX, int ind) {
	fbx[ind].centering = true;
	fbx[ind].offset = true;
	fbx[ind].cx = x;
	fbx[ind].cy = y;
	fbx[ind].cz = z;
	CreateRotMatrix(thetaZ, thetaY, thetaX, ind);
}

bool VulkanSkinMeshRt::InitFBX(char* szFileName, int p) {
	bool f = false;
	f = fbx[p].Create(szFileName);
	if (f)return true;
	return false;
}

bool VulkanSkinMeshRt::InitFBXSetBinary(char* byteArray, unsigned int size, int p) {
	bool f = false;
	f = fbx[p].CreateSetBinary(byteArray, size);
	if (f)return true;
	return false;
}

void VulkanSkinMeshRt::DestroyFBX() {
	vkUtil::ARR_DELETE(fbx);
}

void VulkanSkinMeshRt::ReadSkinInfo(FbxMeshNode* mesh, MY_VERTEX_S* pvVB) {

	int numbone = mesh->getNumDeformer();
	//各Boneのウエイト,インデックスを調べ頂点配列に加える
	if (numbone <= 0)return;
	for (int i = 0; i < numbone; i++) {
		Deformer* defo = mesh->getDeformer(i);
		int iNumIndex = defo->getIndicesCount();//このボーンに影響を受ける頂点インデックス数
		int* piIndex = defo->getIndices();     //このボーンに影響を受ける頂点のインデックス配列
		double* pdWeight = defo->getWeights();//このボーンに影響を受ける頂点のウエイト配列

		for (int k = 0; k < iNumIndex; k++) {
			int index = piIndex[k];      //影響を受ける頂点
			double weight = pdWeight[k];//ウエイト
			for (int m = 0; m < 4; m++) {
				//各Bone毎に影響を受ける頂点のウエイトを一番大きい数値に更新していく
				if (weight > pvVB[index].bBoneWeight[m]) {//調べたウエイトの方が大きい
					pvVB[index].bBoneIndex[m] = i;//Boneインデックス登録
					pvVB[index].bBoneWeight[m] = (float)weight;//ウエイト登録
					break;
				}
			}
		}
	}
	//ウエイト正規化
	for (uint32_t i = 0; i < mesh->getNumVertices(); i++) {
		float we = 0;
		for (int j = 0; j < 4; j++) {
			we += pvVB[i].bBoneWeight[j];
		}
		float we1 = 1.0f / we;
		for (int j = 0; j < 4; j++) {
			pvVB[i].bBoneWeight[j] *= we1;
		}
	}
}

void VulkanSkinMeshRt::SetConnectStep(int ind, float step) {
	fbx[ind].connect_step = step;
}

void VulkanSkinMeshRt::Vertex_hold() {
	pvVB_delete_f = false;
}

void VulkanSkinMeshRt::SetFbx(char* szFileName) {
	//FBXローダーを初期化
	if (!InitFBX(szFileName, 0))
	{
		throw std::runtime_error("FBXローダー初期化失敗");
	}
}

void VulkanSkinMeshRt::SetFbxSetBinary(char* byteArray, unsigned int size) {
	//FBXローダーを初期化
	if (!InitFBXSetBinary(byteArray, size, 0))
	{
		throw std::runtime_error("FBXローダー初期化失敗");
	}
}

void VulkanSkinMeshRt::CreateBuffer(float end_frame, bool singleMesh, bool deformer) {
	float ef[1] = { end_frame };
	CreateBuffer(1, ef, singleMesh, deformer);
}

void VulkanSkinMeshRt::CreateBuffer(int num_end_frame, float* end_frame, bool singleMesh, bool deformer) {

	using namespace CoordTf;
	fbx[0].end_frame = std::make_unique<float[]>(num_end_frame);
	memcpy(fbx[0].end_frame.get(), end_frame, num_end_frame * sizeof(float));
	FbxLoader* fbL = fbx[0].fbxL;
	if (singleMesh)fbL->createFbxSingleMeshNode();
	numMesh = fbL->getNumFbxMeshNode();
	noUseMesh = std::make_unique<bool[]>(numMesh);
	newIndex = NEW uint32_t * *[numMesh];
	NumNewIndex = NEW uint32_t * [numMesh];
	textureId = NEW VulkanDevice::textureIdSetInput * [numMesh];
	numBone = NEW int[numMesh];

	for (int i = 0; i < numMesh; i++) {
		FbxMeshNode* mesh = fbL->getFbxMeshNode(i);

		noUseMesh[i] = false;
		if (deformer) {
			numBone[i] = mesh->getNumDeformer();
			if (maxNumBone < numBone[i]) {
				maxNumBone = numBone[i];
				maxNumBoneMeshIndex = i;
			}
		}
		else {
			numBone[i] = 0;
		}
	}

	if (maxNumBone > 0) {
		boneName = NEW char[maxNumBone * 255];
		m_BoneArray = NEW BONE[maxNumBone];
		m_pLastBoneMatrix = NEW MATRIX[maxNumBone];

		FbxMeshNode* mesh = fbL->getFbxMeshNode(maxNumBoneMeshIndex);
		for (int i = 0; i < maxNumBone; i++) {
			Deformer* defo = mesh->getDeformer(i);
			const char* name = defo->getName();
			strcpy(&boneName[i * 255], name);//ボーンの名前保持

			//初期姿勢行列読み込み
			//GetCurrentPoseMatrixで使う
			for (int x = 0; x < 4; x++) {
				for (int y = 0; y < 4; y++) {
					m_BoneArray[i].mBindPose.m[y][x] = (float)defo->getTransformLinkMatrix(y, x);
				}
			}
		}

		mObject_BONES = NEW VulkanDevice::Uniform<SHADER_GLOBAL_BONES>(1);
	}

	pvVB = NEW MY_VERTEX_S * [numMesh];
	pvVBM = NEW VulkanBasicPolygonRt::Vertex3D_t * [numMesh];
	mObj = NEW VulkanBasicPolygonRt[numMesh];
	if (deformer) {
		for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
			sk[i] = NEW SkinningCom[numMesh];
		}
	}
}

static void createAxisSub(CoordTf::VECTOR3& v3, int axis, int sign) {
	switch (axis) {
	case 0:
		v3.as((float)sign, 0.0f, 0.0f);
		break;
	case 1:
		v3.as(0.0f, (float)sign, 0.0f);
		break;
	case 2:
		v3.as(0.0f, 0.0f, (float)sign);
		break;
	}
}
void VulkanSkinMeshRt::createAxis() {
	GlobalSettings gSet = fbx[0].fbxL->getGlobalSettings();
	CoordTf::VECTOR3 upVec = {};
	createAxisSub(upVec, gSet.UpAxis, gSet.UpAxisSign);
	CoordTf::VECTOR3 frontVec = {};
	createAxisSub(frontVec, gSet.FrontAxis, gSet.FrontAxisSign);
	CoordTf::VECTOR3 coordVec = {};
	createAxisSub(coordVec, gSet.CoordAxis, gSet.CoordAxisSign);

	Axis._11 = coordVec.x; Axis._12 = coordVec.y; Axis._13 = coordVec.z; Axis._14 = 0.0f;
	Axis._21 = upVec.x;    Axis._22 = upVec.y;    Axis._23 = upVec.z;    Axis._24 = 0.0f;
	Axis._31 = frontVec.x; Axis._32 = frontVec.y; Axis._33 = frontVec.z; Axis._34 = 0.0f;
	Axis._41 = 0.0f;       Axis._42 = 0.0f;       Axis._43 = 0.0f;       Axis._44 = 1.0f;
}

void VulkanSkinMeshRt::LclTransformation(FbxMeshNode* mesh, CoordTf::VECTOR3* vec) {
	using namespace CoordTf;
	MATRIX mov;
	MATRIX rotZ, rotY, rotX, rotZY, rotZYX;
	MATRIX scale;
	MATRIX scro;
	MATRIX world;

	MatrixScaling(&scale,
		(float)mesh->getLcl().Scaling[0],
		(float)mesh->getLcl().Scaling[1],
		(float)mesh->getLcl().Scaling[2]);

	MatrixRotationZ(&rotZ, (float)mesh->getLcl().Rotation[2]);
	MatrixRotationY(&rotY, (float)mesh->getLcl().Rotation[1]);
	MatrixRotationX(&rotX, (float)mesh->getLcl().Rotation[0]);
	MatrixMultiply(&rotZY, &rotZ, &rotY);
	MatrixMultiply(&rotZYX, &rotZY, &rotX);

	MatrixTranslation(&mov,
		(float)mesh->getLcl().Translation[0],
		(float)mesh->getLcl().Translation[1],
		(float)mesh->getLcl().Translation[2]);

	MatrixMultiply(&scro, &scale, &rotZYX);
	MatrixMultiply(&world, &scro, &mov);
	CoordTf::VectorMatrixMultiply(vec, &world);
}

void VulkanSkinMeshRt::normalRecalculation(bool lclOn, double** nor, FbxMeshNode* mesh) {

	*nor = NEW double[mesh->getNumPolygonVertices() * 3];
	auto index = mesh->getPolygonVertices();//頂点Index取得(頂点xyzに対してのIndex)
	auto ver = mesh->getVertices();//頂点取得

	CoordTf::VECTOR3 tmpv[3] = {};
	uint32_t indexCnt = 0;
	for (uint32_t i = 0; i < mesh->getNumPolygon(); i++) {
		uint32_t pSize = mesh->getPolygonSize(i);//1ポリゴンでの頂点数
		if (pSize >= 3) {
			for (uint32_t i1 = 0; i1 < 3; i1++) {
				uint32_t ind = indexCnt + i1;
				tmpv[i1].as(
					(float)ver[index[ind] * 3],
					(float)ver[index[ind] * 3 + 1],
					(float)ver[index[ind] * 3 + 2]
				);
				if (lclOn) {
					LclTransformation(mesh, &tmpv[i1]);
				}
			}
			//上記3頂点から法線の方向算出
			CoordTf::VECTOR3 N = vkUtil::normalRecalculation(tmpv);
			for (uint32_t ps = 0; ps < pSize; ps++) {
				uint32_t ind = indexCnt + ps;
				(*nor)[ind * 3] = (double)N.x;
				(*nor)[ind * 3 + 1] = (double)N.y;
				(*nor)[ind * 3 + 2] = (double)N.z;
			}
			indexCnt += pSize;
		}
	}
}

void VulkanSkinMeshRt::noUseMeshIndex(int meshIndex) {
	noUseMesh[meshIndex] = true;
}

void VulkanSkinMeshRt::SetVertex(bool lclOn, bool axisOn) {

	FbxLoader* fbL = fbx[0].fbxL;
	if (axisOn) {
		createAxis();
	}
	else {
		CoordTf::MatrixIdentity(&Axis);
	}

	using namespace CoordTf;
	for (int m = 0; m < numMesh; m++) {

		FbxMeshNode* mesh = fbL->getFbxMeshNode(m);//メッシュ毎に処理する

		MY_VERTEX_S* tmpVB = NEW MY_VERTEX_S[mesh->getNumVertices()];
		//ボーンウエイト
		ReadSkinInfo(mesh, tmpVB);

		auto index = mesh->getPolygonVertices();//頂点Index取得(頂点xyzに対してのIndex)
		auto ver = mesh->getVertices();//頂点取得
		auto nor = mesh->getAlignedNormal(0);//法線取得

		bool norCreate = false;
		if (!nor) {
			normalRecalculation(lclOn, &nor, mesh);
			norCreate = true;
		}

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

		pvVB[m] = NEW MY_VERTEX_S[mesh->getNumPolygonVertices()];
		pvVBM[m] = NEW VulkanBasicPolygonRt::Vertex3D_t[mesh->getNumPolygonVertices()];

		MY_VERTEX_S* vb = pvVB[m];
		VulkanBasicPolygonRt::Vertex3D_t* vbm = pvVBM[m];
		for (unsigned int i = 0; i < mesh->getNumPolygonVertices(); i++) {
			//index順で頂点を整列しながら頂点格納
			MY_VERTEX_S* v = &vb[i];
			VulkanBasicPolygonRt::Vertex3D_t* vm = &vbm[i];
			v->vPos.x = (float)ver[index[i] * 3];
			v->vPos.y = (float)ver[index[i] * 3 + 1];
			v->vPos.z = (float)ver[index[i] * 3 + 2];
			if (lclOn) {
				LclTransformation(mesh, &v->vPos);
			}
			vm->pos[0] = v->vPos.x;
			vm->pos[1] = v->vPos.y;
			vm->pos[2] = v->vPos.z;

			int norInd = i;
			int uvInd = i;
			if (strcmp(mesh->getNormalMappingInformationType(0), "ByPolygonVertex"))norInd = index[i];
			if (strcmp(mesh->getUVMappingInformationType(0), "ByPolygonVertex"))uvInd = index[i];
			vm->normal[0] = v->vNorm.x = (float)nor[norInd * 3];
			vm->normal[1] = v->vNorm.y = (float)nor[norInd * 3 + 1];
			vm->normal[2] = v->vNorm.z = (float)nor[norInd * 3 + 2];
			vm->difUv[0] = v->vTex0.x = (float)uv0[uvInd * 2];
			vm->difUv[1] = v->vTex0.y = 1.0f - (float)uv0[uvInd * 2 + 1];//(1.0f-UV)
			vm->speUv[0] = v->vTex1.x = (float)uv1[uvInd * 2];
			vm->speUv[1] = v->vTex1.y = 1.0f - (float)uv1[uvInd * 2 + 1];//(1.0f-UV)

			if (numBone[m] > 0) {
				for (int bi = 0; bi < 4; bi++) {
					//ReadSkinInfo(tmpVB)で読み込んだ各パラメータコピー
					v->bBoneIndex[bi] = tmpVB[index[i]].bBoneIndex[bi];
					v->bBoneWeight[bi] = tmpVB[index[i]].bBoneWeight[bi];
				}
			}
		}
		vkUtil::ARR_DELETE(tmpVB);
		if (norCreate)vkUtil::ARR_DELETE(nor);

		auto numMaterial = mesh->getNumMaterial();
		int* uvSw = NEW int[numMaterial];
		createMaterial(m, numMaterial, mesh, uv0Name, uv1Name, uvSw);
		swapTex(vb, sizeof(MY_VERTEX_S), mesh, uvSw);
		swapTex(vbm, sizeof(VulkanBasicPolygonRt::Vertex3D_t), mesh, uvSw);
		vkUtil::ARR_DELETE(uvSw);
		splitIndex(numMaterial, mesh, m);
	}
}

void VulkanSkinMeshRt::splitIndex(uint32_t numMaterial, FbxMeshNode* mesh, int m) {

	//ポリゴン分割後のIndex数カウント
	NumNewIndex[m] = NEW uint32_t[numMaterial];
	uint32_t* numNewIndex = NumNewIndex[m];
	memset(numNewIndex, 0, sizeof(uint32_t) * numMaterial);

	for (uint32_t i = 0; i < mesh->getNumPolygon(); i++) {
		uint32_t currentMatNo = mesh->getMaterialNoOfPolygon(i);
		uint32_t pSize = mesh->getPolygonSize(i);
		if (pSize >= 3) {
			numNewIndex[currentMatNo] += (pSize - 2) * 3;
		}
	}

	//分割後のIndex生成
	newIndex[m] = NEW uint32_t * [numMaterial];
	for (uint32_t ind1 = 0; ind1 < numMaterial; ind1++) {
		if (numNewIndex[ind1] <= 0) {
			newIndex[m][ind1] = nullptr;
			continue;
		}
		newIndex[m][ind1] = NEW uint32_t[numNewIndex[ind1]];
	}
	std::unique_ptr<uint32_t[]> indexCnt;
	indexCnt = std::make_unique<uint32_t[]>(numMaterial);

	int Icnt = 0;
	for (uint32_t i = 0; i < mesh->getNumPolygon(); i++) {
		uint32_t currentMatNo = mesh->getMaterialNoOfPolygon(i);
		uint32_t pSize = mesh->getPolygonSize(i);
		if (pSize >= 3) {
			for (uint32_t ps = 0; ps < pSize - 2; ps++) {
				newIndex[m][currentMatNo][indexCnt[currentMatNo]++] = Icnt;
				newIndex[m][currentMatNo][indexCnt[currentMatNo]++] = Icnt + 1 + ps;
				newIndex[m][currentMatNo][indexCnt[currentMatNo]++] = Icnt + 2 + ps;
			}
			Icnt += pSize;
		}
	}
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

	FbxLoader* fbL = fbx[0].fbxL;

	for (int i = 0; i < numMesh; i++) {

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

void VulkanSkinMeshRt::SetFbxSub(char* szFileName, int ind) {
	if (ind <= 0) {
		throw std::runtime_error("FBXローダー初期化失敗");
	}

	if (!InitFBX(szFileName, ind)) {
		throw std::runtime_error("FBXローダー初期化失敗");
	}
}

void VulkanSkinMeshRt::SetFbxSubSetBinary(char* byteArray, unsigned int size, int ind) {
	if (ind <= 0) {
		throw std::runtime_error("FBXローダー初期化失敗");
	}

	if (!InitFBXSetBinary(byteArray, size, ind)) {
		throw std::runtime_error("FBXローダー初期化失敗");
	}
}

void VulkanSkinMeshRt::CreateBuffer_Sub(int ind, float end_frame) {
	float ef[1] = { end_frame };
	CreateBuffer_Sub(ind, 1, ef);
}

void VulkanSkinMeshRt::CreateBuffer_Sub(int ind, int num_end_frame, float* end_frame) {

	fbx[ind].end_frame = std::make_unique<float[]>(num_end_frame);
	memcpy(fbx[ind].end_frame.get(), end_frame, num_end_frame * sizeof(float));

	int BoneNum = fbx[ind].fbxL->getNumNoneMeshDeformer();
	if (BoneNum == 0) {
		throw std::runtime_error("FBXローダー初期化失敗");
	}

	if (!m_ppSubAnimationBone) {
		m_ppSubAnimationBone = NEW Deformer * [(FBX_PCS - 1) * maxNumBone];
	}
}

void VulkanSkinMeshRt::CreateFromFBX_SubAnimation(int ind) {
	int loopind = 0;
	int searchCount = 0;
	int name2Num = 0;
	while (loopind < maxNumBone) {
		int sa_ind = (ind - 1) * maxNumBone + loopind;
		m_ppSubAnimationBone[sa_ind] = fbx[ind].fbxL->getNoneMeshDeformer(searchCount);
		searchCount++;
		const char* name = m_ppSubAnimationBone[sa_ind]->getName();
		if (!strncmp("Skeleton", name, 8))continue;
		char* name2 = &boneName[loopind * 255];//各Bone名の先頭アドレスを渡す
		//Bone名に空白が含まれている場合最後の空白以降の文字から終端までの文字を比較する為,
		//終端文字までポインタを進め, 終端から検査して空白位置手前まで比較する
		while (*name != '\0')name++;//終端文字までポインタを進める
		//終端文字までポインタを進める, 空白が含まれない文字の場合もあるので文字数カウントし,
		//文字数で比較完了を判断する
		while (*name2 != '\0') { name2++; name2Num++; }
		while (*(--name) == *(--name2) && *name2 != ' ' && (--name2Num) > 0);
		if (*name2 != ' ' && name2Num > 0) { name2Num = 0; continue; }
		name2Num = 0;
		loopind++;
	}
}

//ボーンを次のポーズ位置にセットする
bool VulkanSkinMeshRt::SetNewPoseMatrices(float ti, int ind, int InternalAnimationIndex) {
	if (maxNumBone <= 0)return true;
	if (AnimLastInd == -1)AnimLastInd = ind;//最初に描画するアニメーション番号で初期化
	if (fbx[ind].InternalLastAnimationIndex == -1)fbx[ind].InternalLastAnimationIndex = InternalAnimationIndex;

	bool ind_change = false;
	if (AnimLastInd != ind || fbx[ind].InternalLastAnimationIndex != InternalAnimationIndex) {//アニメーションが切り替わった
		ind_change = true;
		AnimLastInd = ind;
		fbx[ind].InternalLastAnimationIndex = InternalAnimationIndex;
		fbx[ind].current_frame = 0.0f;//アニメーションが切り替わってるのでMatrix更新前にフレームを0に初期化
	}
	bool frame_end = false;
	fbx[ind].current_frame += ti;
	if (fbx[ind].end_frame[InternalAnimationIndex] <= fbx[ind].current_frame) frame_end = true;

	if (frame_end || ind_change) {
		for (int i = 0; i < maxNumBone; i++) {
			for (int x = 0; x < 4; x++) {
				for (int y = 0; y < 4; y++) {
					m_pLastBoneMatrix[i].m[y][x] = m_BoneArray[i].mNewPose.m[y][x];
				}
			}
		}
		BoneConnect = 0.1f;
	}

	if (BoneConnect != -1.0f)fbx[ind].current_frame = 0.0f;

	int frame = (int)fbx[ind].current_frame;
	Deformer Time;
	int64_t time = Time.getTimeFRAMES60(frame / 10);

	bool subanm = true;
	if (ind <= 0 || ind > FBX_PCS - 1)subanm = false;

	Deformer* defo = nullptr;
	FbxLoader* fbL = fbx[0].fbxL;
	FbxMeshNode* mesh = fbL->getFbxMeshNode(maxNumBoneMeshIndex);
	if (!subanm) {
		defo = mesh->getDeformer(0);
	}
	else {
		defo = m_ppSubAnimationBone[(ind - 1) * maxNumBone];
	}
	defo->EvaluateGlobalTransform(time, InternalAnimationIndex);

	using namespace CoordTf;
	MATRIX mat0_mov;
	MatrixIdentity(&mat0_mov);
	if (fbx[ind].offset) {
		MatrixTranslation(&mat0_mov, fbx[ind].cx, fbx[ind].cy, fbx[ind].cz);
	}
	else {
		MatrixTranslation(&mat0_mov, (float)-defo->getEvaluateGlobalTransform(3, 0) + fbx[ind].cx,
			(float)-defo->getEvaluateGlobalTransform(3, 1) + fbx[ind].cy,
			(float)-defo->getEvaluateGlobalTransform(3, 2) + fbx[ind].cz);
	}

	MATRIX pose;
	for (int i = 0; i < maxNumBone; i++) {
		Deformer* de = nullptr;
		if (!subanm) {
			de = mesh->getDeformer(i);
		}
		else {
			de = m_ppSubAnimationBone[(ind - 1) * maxNumBone + i];
		}
		de->EvaluateGlobalTransform(time, InternalAnimationIndex);

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				if (fbx[ind].centering) {
					pose.m[y][x] = (float)de->getEvaluateGlobalTransform(y, x);
				}
				else {
					m_BoneArray[i].mNewPose.m[y][x] = (float)de->getEvaluateGlobalTransform(y, x);
				}
			}
		}

		if (fbx[ind].centering) {
			MATRIX tmp;
			MatrixMultiply(&tmp, &fbx[ind].rotZYX, &mat0_mov);
			MatrixMultiply(&m_BoneArray[i].mNewPose, &pose, &tmp);
		}
	}

	if (frame_end)fbx[ind].current_frame = 0.0f;

	if (BoneConnect != -1.0f) {
		if (fbx[ind].connect_step <= 0.0f || BoneConnect > 1.0f)BoneConnect = -1.0f;
		else {
			for (int i = 0; i < maxNumBone; i++) {
				StraightLinear(&m_BoneArray[i].mNewPose, &m_pLastBoneMatrix[i], &m_BoneArray[i].mNewPose, BoneConnect += (ti / fbx[ind].connect_step));
			}
		}
	}
	return frame_end;
}

//ポーズ行列を返す
CoordTf::MATRIX VulkanSkinMeshRt::GetCurrentPoseMatrix(int index) {
	using namespace CoordTf;
	MATRIX inv;
	MatrixIdentity(&inv);
	MatrixInverse(&inv, &m_BoneArray[index].mBindPose);//FBXのバインドポーズは初期姿勢（絶対座標）
	MATRIX fPose;
	MatrixIdentity(&fPose);
	MatrixMultiply(&fPose, &inv, &m_BoneArray[index].mNewPose);//バインドポーズの逆行列とフレーム姿勢行列をかける
	MATRIX ret;
	MatrixIdentity(&ret);
	MatrixMultiply(&ret, &fPose, &Axis);
	return ret;
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

void VulkanSkinMeshRt::MatrixMap_Bone(SHADER_GLOBAL_BONES* sgb) {

	using namespace CoordTf;
	for (int i = 0; i < maxNumBone; i++)
	{
		MATRIX mat = GetCurrentPoseMatrix(i);
		sgb->mBone[i] = mat;
	}
}

void VulkanSkinMeshRt::Instancing(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale) {

	for (int i = 0; i < numMesh; i++) {
		if (!noUseMesh[i])
			mObj[i].instancing(pos, theta, scale);
	}
}

bool VulkanSkinMeshRt::InstancingUpdate(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex, int AnimationIndex, float ti, int InternalAnimationIndex) {

	bool frame_end = false;
	int insnum = 0;
	if (ti != -1.0f)frame_end = SetNewPoseMatrices(ti, AnimationIndex, InternalAnimationIndex);
	MatrixMap_Bone(&sgb[0]);//後で切り替えるように変更

	if (sk[swapIndex])mObject_BONES->update(0, &sgb[0]);

	for (int i = 0; i < numMesh; i++) {
		if (!noUseMesh[i]) {
			if (sk[swapIndex])sk[swapIndex][i].Skinned(QueueIndex, comIndex);
			mObj[i].instancingUpdate(swapIndex, QueueIndex, comIndex);
		}
	}

	return frame_end;
}

bool VulkanSkinMeshRt::Update(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex,
	int AnimationIndex, float time,
	CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale,
	int InternalAnimationIndex) {

	Instancing(pos, theta, scale);
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