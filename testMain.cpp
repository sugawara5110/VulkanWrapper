
#define _CRT_SECURE_NO_WARNINGS
#include "../../../VulkanWrapper/Rasterize/RasterizeDescriptor.h"
#include "../../../VulkanWrapper/Rasterize/Vulkan2D.h"
#include "../../../VulkanWrapper/Rasterize/VulkanBasicPolygon.h"
#include "../../../VulkanWrapper/Rasterize/VulkanSkinMesh.h"
#include "../../../T_float/T_float.h"
#include "../../../PPMLoader/PPMLoader.h"
#include "../../../PNGLoader/PNGLoader.h"
#include "../../../JPGLoader/JPGLoader.h"
#include <iostream>
#include <thread>

using namespace CoordTf;
using namespace vkUtil;
HWND hWnd;
VulkanInstance* vins;
float the = 180.0f;
float frame = 1.0f;
Vulkan2D* v2;
const int Num = 1;
VulkanBasicPolygon* v22[Num];
VulkanSkinMesh* sk;
VulkanSkinMesh* sk1;
VulkanSkinMesh* sk2;
volatile bool loop = true;
volatile bool firstDraw = false;
volatile bool sync = false;
volatile bool run = true;

void update(uint32_t sw);
void draw(uint32_t& sw);
static int para = 0;

std::function<void()> g_RenderFunc;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CLOSE: 
		run = false;
		while (true) {
			if (sync)break;
		}
		loop = false;
		sync = false;
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch ((CHAR)wParam) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_CONTROL:
			para = 0;
			break;
		case VK_DELETE:
			para = 1;
			break;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND initApp(HINSTANCE hInstance) {
	//�E�C���h�E�N���X�̏�����
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); //���̍\���̂̃T�C�Y
	wcex.style = NULL;               //�E�C���h�E�X�^�C��(default)
	wcex.lpfnWndProc = WndProc;  //���b�Z�[�W�����֐��̓o�^
	wcex.cbClsExtra = 0;       //�ʏ��0	                
	wcex.cbWndExtra = 0;      //�ʏ��0					
	wcex.hInstance = hInstance; //�C���X�^���X�ւ̃n���h��				
	wcex.hIcon = NULL;         //�A�C�R�� (����)				
	wcex.hCursor = NULL;      //�J�[�\���̌`				
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //�w�i				
	wcex.lpszMenuName = NULL;                       //���j���[����				
	wcex.lpszClassName = L"VulkanTest";          //�N���X��               
	wcex.hIconSm = NULL;                          //���A�C�R��			   

	//�E�C���h�E�N���X�̓o�^(RegisterClassEx�֐�)
	if (!RegisterClassEx(&wcex))return nullptr;

	//�E�C���h�E�����E�C���h�E���[�h
	return CreateWindow(wcex.lpszClassName, //�o�^�N���X��
		wcex.lpszClassName,                      //�E�C���h�E��
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE,//�E�C���h�E�X�^�C��
		CW_USEDEFAULT, //�E�C���h�E���ʒu
		0,            //�E�C���h�E�c�ʒu
		800,             //�E�C���h�E��
		600,            //�E�C���h�E����
		NULL,          //�e�E�C���h�E�n���h��
		NULL,         //���j���[,�q�E�C���h�E�n���h��
		hInstance,   //�A�v���P�[�V�����C���X�^���X�n���h��
		NULL);     //�E�C���h�E�쐬�f�[�^
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	hWnd = initApp(hInstance);
	if (hWnd == nullptr) return 1;

	vins = new VulkanInstance();
	vins->createInstance("vulkanTest", VK_API_VERSION_1_0);
	auto pd = vins->getPhysicalDevice(0);

	VulkanDevice::InstanceCreate(pd, vins->getApiVersion(), 1);
	VulkanDevice* device = VulkanDevice::GetInstance();
	device->createDevice();
	vins->createSurfaceHwnd(hWnd);
	auto sur = vins->getSurface();
	VulkanSwapchain::InstanceCreate();
	VulkanSwapchain* sc = VulkanSwapchain::GetInstance();
	sc->create(0,0,pd, sur, true, false);
	device->updateProjection(sc->getSize());

	RasterizeDescriptor::InstanceCreate();
	RasterizeDescriptor* rd = RasterizeDescriptor::GetInstance();

	static Vulkan2D::Vertex2D ver[] = {
	{ { -0.1f, -0.1f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { 0.1f, -0.1f }, { 1.0f, 0.5f, 0.0f, 1.0f } },
	{ { -0.1f, 0.1f }, { 0.0f, 0.5f, 1.0f, 1.0f } },
	{ { 0.1f, 0.1f }, { 0.0f, 0.5f, 1.0f, 1.0f } }
	};
	static Vulkan2D::Vertex2DTex vertex[] = {
	{ { -0.1f, -0.1f }, {0.0f,0.0f} },
	{ { 0.1f, -0.1f }, {1.0f,0.0f} },
	{ { -0.1f, 0.1f }, {0.0f,1.0f} },
	{ { 0.1f, 0.1f }, {1.0f,1.0f} }
	};
	uint32_t index2d[] = { 0, 2, 1, 1, 2, 3 };
	static VulkanDevice::Vertex3D ver11[] = {
		//�O
		{ {-0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
		{ { 0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
		{ { -0.5f, 0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 0.1f, 0.1f, 1.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
		//��
		{ {-0.5f, -0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
		//��
		{ {-0.5f, -0.5f,0.5f }, { -1.0f, 0.0f, 0.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
		{ { -0.5f, -0.5f,-0.5f }, { -1.0f, 0.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
		{ { -0.5f, 0.5f,0.5f }, { -1.0f, 0.0f, 0.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { -1.0f, 0.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
		//�E
		{ {0.5f, -0.5f,0.5f }, { 1.0f, 0.0f, 0.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 1.0f, 0.0f, 0.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
		//��
		{ {-0.5f, 0.5f,0.5f }, { 0.0f, 1.0f, 0.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 0.0f, 1.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { 0.0f, 1.0f, 0.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 0.0f, 1.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
		//��
		{ {-0.5f, -0.5f,0.5f }, { 0.0f, -1.0f, 0.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
		{ { 0.5f, -0.5f,0.5f }, { 0.0f, -1.0f, 0.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
		{ { -0.5f, -0.5f,-0.5f }, { 0.0f, -1.0f, 0.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 0.0f, -1.0f, 0.0f} ,{1.0f,1.0f},{1.0f,1.0f}}
	};
	uint32_t index[3] = { 0,1,2 };
	uint32_t index1[] = { 0, 2, 1, 1, 2, 3, 4, 5, 6, 5, 7, 6,
				8, 9, 10, 9, 11, 10, 12, 14, 13, 13, 14, 15,
				16, 18, 17, 17, 18, 19, 20, 21, 22, 21, 23,
				22 };
	PPMLoader* ppm = nullptr;


	//4ch�ɕϊ����鏈���ǉ���
	wchar_t pass[6][60] = {
		L"../../../texturePPM/boss1.ppm",
		L"../../../texturePPM/boss1_normal.ppm",
		L"../../../Black Dragon NEW/textures/Dragon_Bump_Col2.ppm",
		L"../../../Black Dragon NEW/textures/Dragon_Nor_mirror2.ppm",
		L"../../../Black Dragon NEW/textures/Dragon_ground_color.ppm",
		L"../../../Black Dragon NEW/textures/Dragon_Nor.ppm"
	};

	int fnum = 6;
	int numstr = 256 * 4 * 256;
	unsigned char** ima = nullptr;
	ima = new unsigned char* [fnum];
	for (int i = 0; i < fnum; i++) {
		ima[i] = new unsigned char[numstr];
	}
	for (int j = 0; j < fnum; j++) {
		ppm = new PPMLoader(pass[j], 256, 256, NORMAL);
		unsigned char* image = ppm->GetImageArr();
		int imageCnt = 0;
		for (int i = 0; i < 256 * 4 * 256; i += 4) {
			ima[j][i + 0] = image[imageCnt++];
			ima[j][i + 1] = image[imageCnt++];
			ima[j][i + 2] = image[imageCnt++];
			ima[j][i + 3] = 255;
		}
		vkUtil::S_DELETE(ppm);
	}
	PNGLoader png;
	JPGLoader jpg;

	unsigned char* wall1 = jpg.loadJPG("../../../wall1.jpg", 256, 256);
	//unsigned char* wall1 = jpg.loadJPG("../../../ceiling1.jpg", 256, 256);

	//unsigned char* wall1 = png.loadPNG("../../../wall1.png", 256, 256);
	//unsigned char* wall1 = jpg.loadJPG("../../../resize/testG.jpg", 40, 40);
	//unsigned char* wall1 = jpg.loadJPG("../../../resize/color_grid.jpg", 256, 256);
	unsigned char* wallNor1 = png.loadPNG("../../../wall1Nor.png", 256, 256);
	unsigned char* MagicCircle1 = png.loadPNG("../../../MagicCircle1.png", 256, 256);

	unsigned char* brown_eye = png.loadPNG("../../../texturePPM/brown_eye.png", 256, 256);
	unsigned char* classicshoes_texture_diffuse = png.loadPNG("../../../texturePPM/classicshoes_texture_diffuse.png", 256, 256);
	unsigned char* classicshoes_texture_normals = png.loadPNG("../../../texturePPM/classicshoes_texture_normals.png", 256, 256);
	unsigned char* eyebrow001 = png.loadPNG("../../../texturePPM/eyebrow001.png", 256, 256);
	unsigned char* jacket01_diffuse = png.loadPNG("../../../texturePPM/jacket01_diffuse.png", 256, 256);
	unsigned char* jacket01_normals = png.loadPNG("../../../texturePPM/jacket01_normals.png", 256, 256);
	unsigned char* jeans01_black_diffuse = png.loadPNG("../../../texturePPM/jeans01_black_diffuse.png", 256, 256);
	unsigned char* jeans01_normals = png.loadPNG("../../../texturePPM/jeans01_normals.png", 256, 256);
	unsigned char* male01_diffuse_black = png.loadPNG("../../../texturePPM/male01_diffuse_black.png", 256, 256);
	unsigned char* young_lightskinned_male_diffuse = png.loadPNG("../../../texturePPM/young_lightskinned_male_diffuse.png", 256, 256);
	try {
		device->GetTexture("wall1.jpg", wall1, 256, 256); ARR_DELETE(wall1);
		device->GetTexture("wallNor1.png", wallNor1, 256, 256); ARR_DELETE(wallNor1);
		device->GetTexture("mahou2.png", MagicCircle1, 256, 256); ARR_DELETE(MagicCircle1);

		device->GetTexture("boss1.jpg", ima[0], 256, 256);
		device->GetTexture("boss1_normal.png", ima[1], 256, 256);

		device->GetTexture("brown_eye.png", brown_eye, 256, 256); ARR_DELETE(brown_eye);
		device->GetTexture("classicshoes_texture_diffuse.png", classicshoes_texture_diffuse, 256, 256); ARR_DELETE(classicshoes_texture_diffuse);
		device->GetTexture("classicshoes_texture_normals.png", classicshoes_texture_normals, 256, 256); ARR_DELETE(classicshoes_texture_normals);
		device->GetTexture("eyebrow001.png", eyebrow001, 256, 256); ARR_DELETE(eyebrow001);
		device->GetTexture("jacket01_diffuse.png", jacket01_diffuse, 256, 256); ARR_DELETE(jacket01_diffuse);
		device->GetTexture("jacket01_normals.png", jacket01_normals, 256, 256); ARR_DELETE(jacket01_normals);
		device->GetTexture("jeans01_black_diffuse.png", jeans01_black_diffuse, 256, 256); ARR_DELETE(jeans01_black_diffuse);
		device->GetTexture("jeans01_normals.png", jeans01_normals, 256, 256); ARR_DELETE(jeans01_normals);
		device->GetTexture("male01_diffuse_black.png", male01_diffuse_black, 256, 256); ARR_DELETE(male01_diffuse_black);
		device->GetTexture("young_lightskinned_male_diffuse.png", young_lightskinned_male_diffuse, 256, 256); ARR_DELETE(young_lightskinned_male_diffuse);

		device->GetTexture("Dragon_Bump_Col2.jpg", ima[2], 256, 256);
		device->GetTexture("Dragon_Nor_mirror2.jpg", ima[3], 256, 256);
		device->GetTexture("Dragon_ground_color.jpg", ima[4], 256, 256);
		device->GetTexture("Dragon_Nor.jpg", ima[5], 256, 256);
	}
	catch (std::runtime_error e) {
		std::cerr << "runtime_error: " << e.what() << std::endl;
	}

	v2 = new Vulkan2D();
	//v2->createColor(0, ver, 4, index2d, 6);
	v2->createTexture(0,0, vertex, 4, index2d, 6, 2);
	//for (int i = 0; i < Num; i++) {
	v22[0] = new VulkanBasicPolygon();
	v22[0]->create(0,0, false, 0, 1, -1, ver11, 24, index1, 36);
	//v22[1] = new VulkanBasicPolygon(device);
	//v22[1]->create(0, true, 2, -1, -1, ver11, 24, index1, 36);
	//}

	sk = new VulkanSkinMesh();
	sk->setFbx("../../../texturePPM/boss1bone.fbx", 100.0f);
	sk->additionalAnimation("../../../texturePPM/boss1bone_wait.fbx", 50.0f);
	sk1 = new VulkanSkinMesh();
	sk1->setFbx("../../../texturePPM/player1_fbx_att.fbx", 300.0f);
	sk1->additionalAnimation("../../../texturePPM/player1_fbx_walk_deform.fbx", 200.0f);
	sk2 = new VulkanSkinMesh();
	sk2->setFbx("../../../Black Dragon NEW/Dragon_Baked_Actions2.fbx", 300);

	sk2->create(0,0, true);
	sk1->create(0,0, true);
	sk->setChangeTexture(0, 0, -1, device->getTextureNo("boss1_normal.png"), -1);
	sk->create(0,0, true);
	sk->setMaterialParameter(0, 0, 0, { 1,1,1 }, { 0.1f,0.1f,0.1f }, { 0.3f,0.3f,0.3f });
	sk->setMaterialParameter(1, 0, 0, { 1,1,1 }, { 0.1f,0.1f,0.1f }, { 0.3f,0.3f,0.3f });
	ShowWindow(hWnd, nCmdShow);
	ValidateRect(hWnd, 0);// WM_PAINT���Ă΂�Ȃ��悤�ɂ���

	//�X���b�v�`�F�C��test
	/*device->destroySwapchain();
	vins->destroySurface();
	vins->createSurfaceHwnd(hWnd);
	device->createSwapchain(vins->getSurface(), true);
	device->updateProjection();
	*/
	MSG msg;
	uint32_t swap = 0;
	std::thread th(draw, std::ref(swap));

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// ���b�Z�[�W�̖|��ƃf�B�X�p�b�`WindowProc�Ăяo��
			if (msg.message == WM_QUIT) {	// PostQuitMessage()���Ă΂ꂽ(�~�����ꂽ)
				break;	//�A�v���I��
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		T_float::GetTime(hWnd);
		//���[�v������
		update(swap);
		while (run) {
			if (sync)break;
		}
		sync = false;
		swap = 1 - swap;
		//���[�v������
	}
	th.join();

	VulkanDevice::GetInstance()->DeviceWaitIdle();
	for (int i = 0; i < fnum; i++)ARR_DELETE(ima[i]);
	ARR_DELETE(ima);
	S_DELETE(v2);
	for (int i = 0; i < Num; i++)S_DELETE(v22[i]);
	S_DELETE(sk);
	S_DELETE(sk1);
	S_DELETE(sk2);
	VulkanSwapchain::DeleteInstance();
	RasterizeDescriptor::DeleteInstance();
	VulkanDevice::DeleteInstance();
	S_DELETE(vins);
	return (int)msg.wParam;
}

void update(uint32_t sw) {
	VulkanDevice* device = VulkanDevice::GetInstance();
	RasterizeDescriptor* rd = RasterizeDescriptor::GetInstance();
	if (the > 360.0f)the = 0.0f;
	the += 0.1f;
	MATRIX thetaY;
	VECTOR3 light1 = { 0.3f,0.4f,-2.0f };
	VECTOR3 light2 = { -0.3f,-0.4f,2.0f };
	MatrixRotationY(&thetaY, the);
	VectorMatrixMultiply(&light1, &thetaY);
	VectorMatrixMultiply(&light2, &thetaY);
	//device->updateView({ 0,-0.2f,-8 }, { 0,0,25 }, { 0,1,0 });
	//device->updateView({ 1.5f,-0.2f,-3 }, { 1.5f,0,25 }, { 0,1,0 });
	device->updateView({ -1.3f,0.0f,-13 }, { -1.3f,0,25 });
	rd->setNumLight(2);
	rd->setLight(0, light1, { 1.0f,1.0f,1.0f });
	rd->setLight(1, light2, { 1.0f,0.0f,0.0f });
	for (int i = 0; i < Num; i++) {
		v22[i]->setMaterialParameter(sw, { 0.5f,0.5f,0.5f }, { 0.1f,0.1f,0.1f }, { 0.0f,0.0f,0.0f });
		v22[i]->update(sw, { 0.4f * (float)i - 1.3f ,0.7f,0.0f }, { 0,the,0 }, { 1,1,1 });
	}
	sk->autoUpdate(sw, 0, frame, { 0,0,0 }, { 180,0,0 }, { 1.0f,1.0f,1.0f });
	sk1->autoUpdate(sw, para, frame, { 2,0,0 }, { 90,0.0f,0 }, { 0.2f,0.2f,0.2f });
	sk2->autoUpdate(sw, 0, frame, { -2,0,0 }, { 90,0,0 }, { 0.1f,0.1f,0.1f });
	v2->update(sw, { -0.5f,-0.5f });
	firstDraw = true;
}

void draw(uint32_t& sw0) {
	VulkanDevice* device = VulkanDevice::GetInstance();
	VulkanSwapchain* sc = VulkanSwapchain::GetInstance();
	auto com = device->getCommandObj(0);
	while (loop) {
		uint32_t sw = 1 - sw0;
		if (firstDraw) {
			sc->beginCommandNextImage(0,0);
			sc->beginDraw(0,0);
			v2->draw(sw, 0,0);
			for (int i = 0; i < Num; i++) {
				v22[i]->draw(sw, 0,0);
			}
			sk->draw(sw, 0,0);
			sk1->draw(sw, 0,0);
			sk2->draw(sw, 0,0);
			sc->endDraw(0,0);
			sc->endCommand(0,0);
			sc->submitCommands(0);
			sc->present(0);
		}
		sync = true;
		while (sync);
	}
}
