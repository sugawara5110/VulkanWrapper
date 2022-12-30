
#define _CRT_SECURE_NO_WARNINGS

#include "../VulkanWrapper/RayTracing/VulkanDeviceRt.h"
#include "../PNGLoader/PNGLoader.h"
#include "../JPGLoader/JPGLoader.h"
#include "../T_float/T_float.h"
#include"../CreateGeometry/CreateGeometry.h"
#include "../VulkanWrapper/Rasterize/VulkanSkinMesh.h"
#include "../VulkanWrapper/PostEffect/VulkanBloom.h"

HWND hWnd;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
using namespace CoordTf;

static bool close = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CLOSE: 
		
		close = true;
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch ((CHAR)wParam) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_CONTROL:
			break;
		case VK_DELETE:
			
			break;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND initApp(HINSTANCE hInstance) {
	//ウインドウクラスの初期化
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); //この構造体のサイズ
	wcex.style = NULL;               //ウインドウスタイル(default)
	wcex.lpfnWndProc = WndProc;  //メッセージ処理関数の登録
	wcex.cbClsExtra = 0;       //通常は0	                
	wcex.cbWndExtra = 0;      //通常は0					
	wcex.hInstance = hInstance; //インスタンスへのハンドル				
	wcex.hIcon = NULL;         //アイコン (無し)				
	wcex.hCursor = NULL;      //カーソルの形				
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //背景				
	wcex.lpszMenuName = NULL;                       //メニュー無し				
	wcex.lpszClassName = L"VulkanTest";          //クラス名               
	wcex.hIconSm = NULL;                          //小アイコン			   

	//ウインドウクラスの登録(RegisterClassEx関数)
	if (!RegisterClassEx(&wcex))return nullptr;

	//ウインドウ生成ウインドウモード
	return CreateWindow(wcex.lpszClassName, //登録クラス名
		wcex.lpszClassName,                      //ウインドウ名
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE,//ウインドウスタイル
		CW_USEDEFAULT, //ウインドウ横位置
		0,            //ウインドウ縦位置
		WINDOW_WIDTH,             //ウインドウ幅
		WINDOW_HEIGHT,            //ウインドウ高さ
		NULL,          //親ウインドウハンドル
		NULL,         //メニュー,子ウインドウハンドル
		hInstance,   //アプリケーションインスタンスハンドル
		NULL);     //ウインドウ作成データ
}

#include "../VulkanWrapper/RayTracing/VulkanRendererRt.h"
#include "../VulkanWrapper/RayTracing/VulkanBasicPolygonRt.h"
#include "../VulkanWrapper/RayTracing/VulkanSkinMeshRt.h"

static VulkanDevice::Vertex3D ver24aa[] =
{
	{ {-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
	{ {1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
	{ {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
	{ {-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f},{0.0f,1.0f} ,{0.0f,1.0f}},

	{ {-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f},{0.0f,0.0f},{0.0f,0.0f} },
	{ {1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
	{ {1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
	{ {-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} ,{0.0f,1.0f},{0.0f,1.0f}},

	{ {-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
	{ {-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
	{ {-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
	{ {-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} ,{0.0f,1.0f} ,{0.0f,1.0f}},

	{ {1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
	{ {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
	{ {1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
	{ {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} ,{0.0f,1.0f},{0.0f,1.0f}},

	{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
	{ {1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
	{ {1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
	{ {-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f} ,{0.0f,1.0f},{0.0f,1.0f}},

	{ {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
	{ {1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
	{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
	{ {-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
};


static UINT index36[] =
{
	3,1,0,
	2,1,3,

	6,4,5,
	7,4,6,

	11,9,8,
	10,9,11,

	14,12,13,
	15,12,14,

	19,17,16,
	18,17,19,

	22,20,21,
	23,20,22
};

static VulkanDevice::Vertex3D ver24aaRev[] =
{
	{ {-1.0f, 1.0f, -1.0f}, {0.0f, -1.0f, 0.0f} ,{0.0f,0.0f}},
	{ {1.0f, 1.0f, -1.0f}, {0.0f, -1.0f, 0.0f} ,{1.0f,0.0f}},
	{ {1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} ,{1.0f,1.0f}},
	{ {-1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f},{0.0f,1.0f} },

	{ {-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f},{0.0f,0.0f} },
	{ {1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} ,{1.0f,0.0f}},
	{ {1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} ,{1.0f,1.0f}},
	{ {-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} ,{0.0f,1.0f}},

	{ {-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} ,{0.0f,0.0f}},
	{ {-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} ,{1.0f,0.0f}},
	{ {-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} ,{1.0f,1.0f}},
	{ {-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} ,{0.0f,1.0f}},

	{ {1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} ,{0.0f,0.0f}},
	{ {1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f} ,{1.0f,0.0f}},
	{ {1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f} ,{1.0f,1.0f}},
	{ {1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} ,{0.0f,1.0f}},

	{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f} ,{0.0f,0.0f}},
	{ {1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f} ,{1.0f,0.0f}},
	{ {1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f} ,{1.0f,1.0f}},
	{ {-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f} ,{0.0f,1.0f}},

	{ {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} ,{0.0f,0.0f}},
	{ {1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} ,{1.0f,0.0f}},
	{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} ,{1.0f,1.0f}},
	{ {-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} ,{0.0f,1.0f}},
};

static UINT index36Rev[] =
{
	1,3,0,
	1,2,3,

	4,6,5,
	4,7,6,

	9,11,8,
	9,10,11,

	12,14,13,
	12,15,14,

	17,19,16,
	17,18,19,

	20,22,21,
	20,23,22
};


static VulkanBasicPolygonRt* cube;
static VulkanBasicPolygonRt* plane;
static VulkanBasicPolygonRt* emissiv;
static VulkanSkinMeshRt* skin;
static VulkanSkinMesh* sk;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

	hWnd = initApp(hInstance);
	if (hWnd == nullptr) return 1;

	///////////////////////vulkan準備
	VulkanInstance* vins;
	vins = new VulkanInstance();
	vins->createInstance("vulkanTest", VK_API_VERSION_1_2);
	auto pd = vins->getPhysicalDevice(0);
	vins->createSurfaceHwnd(hWnd);
	auto sur = vins->getSurface();

	std::unique_ptr<VulkanDeviceRt> devRt;
	devRt = std::make_unique<VulkanDeviceRt>();

	std::vector<VkDescriptorPoolSize> poolSize = {
	  { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
	  { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
	  { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
	  { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
	  { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 100 },
	};

	devRt->createDevice(vins->getInstance(), pd, vins->getApiVersion(), 1, false, & poolSize, 100);

	VulkanRendererRt theApp;
	
	VulkanDevice* vDev = VulkanDevice::GetInstance();

	RasterizeDescriptor::InstanceCreate();
	RasterizeDescriptor* rd = RasterizeDescriptor::GetInstance();

	JPGLoader jpg;
	PNGLoader png;

	using namespace vkUtil;

	unsigned char* wall1 = jpg.loadJPG("../wall1.jpg", 256, 256);
	unsigned char* wall2 = jpg.loadJPG("../wood.jpg", 256, 256);
	unsigned char* wallNor = png.loadPNG("../wall1Nor.png", 256, 256);
	unsigned char* woodNor = png.loadPNG("../woodNor.png", 256, 256);
	unsigned char* brown_eye = png.loadPNG("../texturePPM/brown_eye.png", 256, 256);
	unsigned char* classicshoes_texture_diffuse = png.loadPNG("../texturePPM/classicshoes_texture_diffuse.png", 256, 256);
	unsigned char* classicshoes_texture_normals = png.loadPNG("../texturePPM/classicshoes_texture_normals.png", 256, 256);
	unsigned char* eyebrow001 = png.loadPNG("../texturePPM/eyebrow001.png", 256, 256);
	unsigned char* jacket01_diffuse = png.loadPNG("../texturePPM/jacket01_diffuse.png", 256, 256);
	unsigned char* jacket01_normals = png.loadPNG("../texturePPM/jacket01_normals.png", 256, 256);
	unsigned char* jeans01_black_diffuse = png.loadPNG("../texturePPM/jeans01_black_diffuse.png", 256, 256);
	unsigned char* jeans01_normals = png.loadPNG("../texturePPM/jeans01_normals.png", 256, 256);
	unsigned char* male01_diffuse_black = png.loadPNG("../texturePPM/male01_diffuse_black.png", 256, 256);
	unsigned char* young_lightskinned_male_diffuse = png.loadPNG("../texturePPM/young_lightskinned_male_diffuse.png", 256, 256);

	VulkanDevice* device = VulkanDevice::GetInstance();
	device->GetTexture(0, "wall1.jpg", wall1, 256, 256); vkUtil::ARR_DELETE(wall1);
	device->GetTexture(0, "wood.jpg", wall2, 256, 256); vkUtil::ARR_DELETE(wall2);
	device->GetTexture(0, "wallNor.png", wallNor, 256, 256); vkUtil::ARR_DELETE(wallNor);
	device->GetTexture(0, "woodNor.png", woodNor, 256, 256); vkUtil::ARR_DELETE(woodNor);
	device->GetTexture(0, "brown_eye.png", brown_eye, 256, 256); ARR_DELETE(brown_eye);
	device->GetTexture(0, "classicshoes_texture_diffuse.png", classicshoes_texture_diffuse, 256, 256); ARR_DELETE(classicshoes_texture_diffuse);
	device->GetTexture(0, "classicshoes_texture_normals.png", classicshoes_texture_normals, 256, 256); ARR_DELETE(classicshoes_texture_normals);
	device->GetTexture(0, "eyebrow001.png", eyebrow001, 256, 256); ARR_DELETE(eyebrow001);
	device->GetTexture(0, "jacket01_diffuse.png", jacket01_diffuse, 256, 256); ARR_DELETE(jacket01_diffuse);
	device->GetTexture(0, "jacket01_normals.png", jacket01_normals, 256, 256); ARR_DELETE(jacket01_normals);
	device->GetTexture(0, "jeans01_black_diffuse.png", jeans01_black_diffuse, 256, 256); ARR_DELETE(jeans01_black_diffuse);
	device->GetTexture(0, "jeans01_normals.png", jeans01_normals, 256, 256); ARR_DELETE(jeans01_normals);
	device->GetTexture(0, "male01_diffuse_black.png", male01_diffuse_black, 256, 256); ARR_DELETE(male01_diffuse_black);
	device->GetTexture(0, "young_lightskinned_male_diffuse.png", young_lightskinned_male_diffuse, 256, 256); ARR_DELETE(young_lightskinned_male_diffuse);
	
	emissiv = new VulkanBasicPolygonRt();
	cube = new VulkanBasicPolygonRt();
	plane = new VulkanBasicPolygonRt();
	skin = new VulkanSkinMeshRt();
	sk = new VulkanSkinMesh();

	std::vector<VulkanDevice::Vertex3D> vertices;
	std::vector<uint32_t> index;

	emissiv->create(0, true, ver24aa, 24, index36, 36,
		-1,
		-1,
		-1,
		2);

	emissiv->setMaterialType(EMISSIVE, 0);
	
	emissiv->LightOn(true, 0, 0, 100, 0.8f);
	emissiv->LightOn(true, 1, 0, 100, 0.7f);

	VECTOR3 v3[2] = { {},{5,0,0} };
	VECTOR3 v3s[2] = { {1.5,1.5,1.5},{2,2,2} };

	int numSphereVer = 10;
	int numSphereArr = 1;

	CreateGeometry::ver* sv = (CreateGeometry::ver*)CreateGeometry::createSphere(numSphereVer, numSphereVer, numSphereArr, v3, v3s, false);
	//個数がわからん・・後で修正((numSphereVer+1) * (numSphereVer+1) * 1)
	unsigned int* svI = CreateGeometry::createSphereIndex(numSphereVer, numSphereVer, numSphereArr);
	//個数がわからん・・後で修正(numSphereVer * numSphereVer * 6 * numSphereArr)

	CreateGeometry::ver* sv1 = (CreateGeometry::ver*)CreateGeometry::createCube(1, v3, v3s, false);
	unsigned int* svI1 = CreateGeometry::createCubeIndex(1);

	VulkanDevice::Vertex3D* svv = new VulkanDevice::Vertex3D[(numSphereVer + 1) * (numSphereVer + 1) * numSphereArr];
	VulkanDevice::Vertex3D* svv1 = new VulkanDevice::Vertex3D[24];


	for (int i = 0; i < (numSphereVer + 1) * (numSphereVer + 1) * numSphereArr; i++) {
		svv[i].pos[0] = sv[i].Pos.x;
		svv[i].pos[1] = sv[i].Pos.y;
		svv[i].pos[2] = sv[i].Pos.z;
		VECTOR3 nor;
		CoordTf::VectorNormalize(&nor, &sv[i].normal);
		memcpy(&svv[i].normal, &nor, sizeof(VECTOR3));
		svv[i].difUv[0] = svv[i].speUv[0] = sv[i].tex.x;
		svv[i].difUv[1] = svv[i].speUv[1] = sv[i].tex.y;
	}
	for (int i = 0; i < 24; i++) {
		svv1[i].pos[0] = sv1[i].Pos.x;
		svv1[i].pos[1] = sv1[i].Pos.y;
		svv1[i].pos[2] = sv1[i].Pos.z;
		svv1[i].normal[0] = sv1[i].normal.x;
		svv1[i].normal[1] = sv1[i].normal.y;
		svv1[i].normal[2] = sv1[i].normal.z;
		svv1[i].difUv[0] = svv1[i].speUv[0] = sv1[i].tex.x;
		svv1[i].difUv[1] = svv1[i].speUv[1] = sv1[i].tex.y;
	}

	cube->create(0, true, svv, (numSphereVer + 1) * (numSphereVer + 1) * numSphereArr, svI, numSphereVer * numSphereVer * 6 * numSphereArr,
	-1,//	VulkanDevice::GetInstance()->getTextureNo("wall1.jpg"),
	-1,//	VulkanDevice::GetInstance()->getTextureNo("wallNor.png"),
		-1,//VulkanDevice::GetInstance()->getTextureNo("wallNor.png"),
		2);
	cube->setMaterialType(METALLIC, 0);

	vkUtil::ARR_DELETE(sv);
	vkUtil::ARR_DELETE(svI);
	vkUtil::ARR_DELETE(svv);
	vkUtil::ARR_DELETE(sv1);
	vkUtil::ARR_DELETE(svI1);
	vkUtil::ARR_DELETE(svv1);

	plane->create(0, true, ver24aa, 24, index36, 36,
    	VulkanDevice::GetInstance()->getTextureNo("wood.jpg"),
	-1,//	VulkanDevice::GetInstance()->getTextureNo("woodNor.png"), 
		-1,
		1);
	plane->setMaterialType(METALLIC, 0);

	skin->SetFbx("../texturePPM/player1_fbx_att.fbx");
	skin->CreateBuffer(1000.0f);
	skin->SetVertex();
	skin->CreateFromFBX(0, true, 1);

	devRt->CreateSwapchain(sur, 1280, 720);

	sk->setFbx("../texturePPM/player1_fbx_att.fbx",300.0f);
	sk->create(0, true);

	
	
	//各メッシュのマテリアルが2以上の時どうする？
	std::vector<VulkanBasicPolygonRt::RtData*>rt = {
		&emissiv->Rdata[0],
		cube->Rdata.data(),
		plane->Rdata.data(),
		skin->getRtData(0).data(),
		skin->getRtData(1).data(),
		skin->getRtData(2).data(),
        skin->getRtData(3).data(),
		skin->getRtData(4).data(),
		skin->getRtData(5).data(),
		skin->getRtData(6).data(),
	};

	//theApp.TestModeOn(VulkanRendererRt::NormalMap);
	//theApp.TestModeOn(VulkanRendererRt::InstanceIdMap);
	//theApp.TestModeOn(VulkanRendererRt::DepthMap);

	theApp.Init(0, rt);

	////////////////////////



	ShowWindow(hWnd, nCmdShow);
	ValidateRect(hWnd, 0);// WM_PAINTが呼ばれないようにする

	MSG msg;

	float theta = 0.0f;
	float thetaCam = 0.0f;
	float theta0 = 0.0f;

	theApp.setGlobalAmbientColor({ 0.1f, 0.1f, 0.1f });

	vDev->updateProjection(45.0f,1.0f,500.0f);

	rd->setNumLight(2);

	VulkanBloom* bl = nullptr;
	bl = new VulkanBloom();

	int w = VulkanDevice::GetInstance()->getSwapchainObj()->getSize().width;
	int h = VulkanDevice::GetInstance()->getSwapchainObj()->getSize().height;

	VulkanBloom::InstanceParam ipa;
	ipa.bloomStrength = 10.0f;
	ipa.EmissiveInstanceId = 0;
	ipa.thresholdLuminance = 0.3f;

	VulkanBloom::InstanceParam ipa1;
	ipa.bloomStrength = 1.0f;
	ipa.EmissiveInstanceId = 1;
	ipa.thresholdLuminance = 0.3f;

	std::vector<uint32_t> ga = { 256,128,64,32 };

	bl->setImage(theApp.getRenderedImage(), theApp.getInstanceIdMap(), w, h, ipa,ga);
	bl->Create(0);

	while (1)
	{
		T_float::GetTime(hWnd);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// メッセージの翻訳とディスパッチWindowProc呼び出し
			if (msg.message == WM_QUIT) {	// PostQuitMessage()が呼ばれた(×押された)
				break;	//アプリ終了
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// ループ内処理
		if (!close) {
			if (theta0 > 360.0f)theta0 = 0.0f;
			theta0 += 0.1f;
			if (theta > 360.0f)theta = 0.0f;
			theta += 0.04f;
			if (thetaCam > 360.0f)thetaCam = 0.0f;
			//thetaCam += 0.1f;
			MATRIX Yc;
			VECTOR3 cam{ 3.0f, 4.5f, 35.0f };
			MATRIX Y;
			VECTOR3 epos1{ -16.0f, 1.0f, 0.0f };
			VECTOR3 epos2{ +15.0f, 1.0f, 1.0f };

			MatrixRotationY(&Yc, thetaCam);
			VectorMatrixMultiply(&cam, &Yc);
			MatrixRotationY(&Y, theta);
			VectorMatrixMultiply(&epos1, &Y);
			VectorMatrixMultiply(&epos2, &Y);
			vDev->updateView(cam, { 0,0,0 });

			vDev->beginCommandNextImage(0);

			rd->setLight(0, epos1, { 1.0f,1.0f,1.0f });
			rd->setLight(1, epos2, { 1.0f,0.0f,0.0f });

			emissiv->instancing(epos1, { 0,0,0 }, { 1,1,1 });
			emissiv->instancing(epos2, { 0,0,0 }, { 1,1,1 });
			emissiv->instancingUpdate(0);
			cube->setMaterialColor({ 0.8f,0.8f,0.8f }, { 0.5f,0.5f,0.5f }, { 0,0,0 }, 0);
			cube->instancing({ -2.0f, -1.0f, 0.0f }, { theta0,0,0 }, { 2,2,2 });
			cube->instancing({ +2.0f, -1.0f, 1.0f }, { 0,theta0,0 }, { 1,1,1 });
			cube->instancingUpdate(0);
			plane->update(0,{ 0,-10,0 }, { 0,0,0 }, { 15,5,15 });
			skin->Instancing({ 8,3.5f,1 }, { -90,0,theta0 }, { 1.0f,1.0f,1.0f });
			//skin->Instancing({ 3,10,1 }, { -90,0,0 }, { 0.5,0.5,0.5 });
			skin->InstancingUpdate(0, 0, 0.7f);
	

			theApp.Update(5);
			theApp.Render(0,true);

			vDev->beginDraw(0);

			//通常のレンダリングする場合ここで処理/////////////////////
			sk->autoUpdate(0, 0, 0.1f, { -8,0.5f,1 }, { 90,0,0 }, { 2.0f,2.0f,2.0f });
			sk->draw(0, 0);
			 // レンダーパスが終了するとバックバッファは
			 // TRANSFER_DST_OPTIMAL->PRESENT_SRC_KHR へレイアウト変更が適用される.
			vDev->endDraw(0);
			//bl->Compute(0);
			vDev->endCommand(0);
			vDev->Present(0);

			int kk = 0;

		}
		//ループ内処理
	}
	VulkanDevice::GetInstance()->DeviceWaitIdle();
	vkUtil::S_DELETE(sk);
	vkUtil::S_DELETE(bl);
	RasterizeDescriptor::DeleteInstance();
	vkUtil::S_DELETE(skin);
	vkUtil::S_DELETE(emissiv);
	vkUtil::S_DELETE(cube);
	vkUtil::S_DELETE(plane);
	theApp.destroy();
	devRt->destroy();
	S_DELETE(vins);
	return (int)msg.wParam;
}


