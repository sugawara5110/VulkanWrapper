
#define _CRT_SECURE_NO_WARNINGS

#include "../VulkanWrapper/Rasterize/VulkanInstance.h"
#include "../PNGLoader/PNGLoader.h"
#include "../JPGLoader/JPGLoader.h"
#include "../T_float/T_float.h"

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
		WINDOW_WIDTH,             //�E�C���h�E��
		WINDOW_HEIGHT,            //�E�C���h�E����
		NULL,          //�e�E�C���h�E�n���h��
		NULL,         //���j���[,�q�E�C���h�E�n���h��
		hInstance,   //�A�v���P�[�V�����C���X�^���X�n���h��
		NULL);     //�E�C���h�E�쐬�f�[�^
}

#include "../VulkanWrapper/RayTracing/VulkanRendererRt.h"
#include "../VulkanWrapper/RayTracing/VulkanBasicPolygonRt.h"

///test
static void getCube(std::vector<VulkanDevice::Vertex3D>& vertices, std::vector<uint32_t>& index) {

	vertices = {
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
	index = { 0, 2, 1, 1, 2, 3, 4, 5, 6, 5, 7, 6,
				8, 9, 10, 9, 11, 10, 12, 14, 13, 13, 14, 15,
				16, 18, 17, 17, 18, 19, 20, 21, 22, 21, 23,22 };
}

static void getPlane(std::vector<VulkanDevice::Vertex3D>& vertices, std::vector<uint32_t>& index)
{
	vertices = {
		{ {-0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,0.0f},{0.0f,0.0f}},
		{ { 0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{1.0f,0.0f},{1.0f,0.0f}},
		{ { -0.5f, 0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,1.0f},{0.0f,1.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 0.1f, 0.1f, 1.0f} ,{1.0f,1.0f},{1.0f,1.0f}},
	};

	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].pos[0] *= 10.0f;
		vertices[i].pos[1] *= 10.0f;
		vertices[i].pos[2] *= 10.0f;
	}

	index = { 0, 2, 1, 1, 2, 3 };
}

static VulkanBasicPolygonRt* cube;
static VulkanBasicPolygonRt* plane;
static VulkanBasicPolygonRt* emissiv;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	hWnd = initApp(hInstance);
	if (hWnd == nullptr) return 1;

	VulkanInstance* vins;
	vins = new VulkanInstance();
	vins->createInstance("vulkanTest", VK_API_VERSION_1_2);
	auto pd = vins->getPhysicalDevice(0);
	vins->createSurfaceHwnd(hWnd);
	auto sur = vins->getSurface();

	VulkanRendererRt theApp;
	///////////////////////vulkan����
	theApp.m_device = std::make_unique<VulkanDeviceRt>();
	theApp.m_device->createDevice(vins->getInstance(), pd);
	VulkanDevice* vDev = VulkanDevice::GetInstance();

	JPGLoader jpg;

	unsigned char* wall1 = jpg.loadJPG("../wall1.jpg", 256, 256);
	unsigned char* wall2 = jpg.loadJPG("../wood.jpg", 256, 256);

	VulkanDevice::GetInstance()->GetTexture(0, "wall1.jpg", wall1, 256, 256); vkUtil::ARR_DELETE(wall1);
	VulkanDevice::GetInstance()->GetTexture(0, "wood.jpg", wall2, 256, 256); vkUtil::ARR_DELETE(wall2);

	emissiv = new VulkanBasicPolygonRt();
	cube = new VulkanBasicPolygonRt();
	plane = new VulkanBasicPolygonRt();

	std::vector<VulkanDevice::Vertex3D> vertices;
	std::vector<uint32_t> index;
	getCube(vertices, index);

	emissiv->create(0,true, vertices.data(), vertices.size(), index.data(), index.size(),
		-1, -1, -1,
		2);
	cube->create(0,true, vertices.data(), vertices.size(), index.data(), index.size(),
		VulkanDevice::GetInstance()->getTextureNo("wall1.jpg"), -1, -1,
		2);

	std::vector<VulkanDevice::Vertex3D> vertices2;
	std::vector<uint32_t> index2;
	getPlane(vertices2, index2);

	plane->create(0,true, vertices2.data(), vertices2.size(), index2.data(), index2.size(),
		VulkanDevice::GetInstance()->getTextureNo("wood.jpg"), -1, -1,
		1);

	theApp.m_device->CreateSwapchain(sur, 1280, 720);
	// �e�A�v���P�[�V�����ŗL�̏���������.
	std::vector<VulkanBasicPolygonRt::RtData*>rt = {
		emissiv->Rdata.data(),cube->Rdata.data(),plane->Rdata.data()
	};
	theApp.Init(rt);

	////////////////////////

	

	ShowWindow(hWnd, nCmdShow);
	ValidateRect(hWnd, 0);// WM_PAINT���Ă΂�Ȃ��悤�ɂ���

	MSG msg;

	float theta = 0.0f;

	vDev->updateProjection();
	while (1)
	{
		T_float::GetTime(hWnd);
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

		// ���[�v������
		if (!close) {
			vDev->updateView({ 0.0f, 4.0f, 15.0f }, { 0,0,0 }, { 0,1,0 });
			theApp.Update();
			if (theta > 360.0f)theta = 0.0f;
			theta += 0.1f;
			MATRIX Y;
			VECTOR3 epos1{ -4.0f, 1.0f, 0.0f };
			VECTOR3 epos2{ +4.0f, 0.5f, 1.0f };
			MatrixRotationY(&Y, theta);
			VectorMatrixMultiply(&epos1, &Y);
			VectorMatrixMultiply(&epos2, &Y);
			emissiv->instancing(epos1, { theta,0,0 }, { 1,1,1 });
			emissiv->instancing(epos2, { 45,45,theta }, { 1,1,1 });
			emissiv->instancingUpdate();
			cube->instancing({ -2.0f, 1.0f, 0.0f }, { theta,0,0 }, { 1,1,1 });
			cube->instancing({ +2.0f, 4.0f, 1.0f }, { 45,45,theta }, { 1,1,1 });
			cube->instancingUpdate();
			plane->update({ 0,3,0 }, { 90,0,0 }, { 1,1,1 });
			theApp.Render();
		}
		//���[�v������
	}
	VulkanDevice::GetInstance()->DeviceWaitIdle();
	vkUtil::S_DELETE(emissiv);
	vkUtil::S_DELETE(cube);
	vkUtil::S_DELETE(plane);
	theApp.destroy();
	return (int)msg.wParam;
}


