
#include "../../../VulkanWrapper/VulkanInstance.h"
#include "../../../VulkanWrapper/Vulkan2D.h"
#include "../../../VulkanWrapper/VulkanBasicPolygon.h"
#include "../../../VulkanWrapper/VulkanSkinMesh.h"
#include "../../../T_float/T_float.h"
#include "../../../CNN/PPMLoader.h"
#include <iostream>

std::function<void()> g_RenderFunc;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_DESTROY: PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);

	/*switch (uMsg)
	{
	case WM_DESTROY: PostQuitMessage(0); break;
	case WM_PAINT:
		BeginPaint(hWnd, nullptr); EndPaint(hWnd, nullptr);
		g_RenderFunc();
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);*/
}

auto initApp(HINSTANCE hInstance) {
	WNDCLASSEX wce{};

	wce.cbSize = sizeof wce;
	wce.hInstance = hInstance;
	wce.lpszClassName = L"VulkanTest";
	wce.lpfnWndProc = &WndProc;
	wce.style = CS_OWNDC;
	wce.hCursor = LoadCursor(nullptr, IDC_ARROW);
	if (!RegisterClassEx(&wce)) return (HWND)nullptr;

	RECT rc;
	SetRect(&rc, 0, 0, 640, 480);
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, false, 0);
	return CreateWindowEx(0, wce.lpszClassName, L"vkTest", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	auto hWnd = initApp(hInstance);
	if (hWnd == nullptr) return 1;


	VulkanInstance* vins = new VulkanInstance();
	vins->createInstance(hWnd, "vulkanTest");
	auto pd = vins->getPhysicalDevice(0);
	auto sur = vins->getSurface();
	Device* device = new Device(pd, sur, 2, false);
	device->createDevice();
	device->updateProjection();


	static Vertex2D ver[] = {
	{ { 0.0f, -0.70f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, 0.0f }, { 1.0f, 0.5f, 0.0f, 1.0f } },
	{ { 0.5f, 0.0f }, { 0.0f, 0.5f, 1.0f, 1.0f } }
	};
	static Vertex2D ver0[] = {
	{ { 0.3f, -0.70f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.2f, 0.0f }, { 1.0f, 0.5f, 0.0f, 1.0f } },
	{ { 0.8f, 0.0f }, { 0.0f, 0.5f, 1.0f, 1.0f } }
	};
	/*static Vertex3D ver1[] = {
	{ { 0.0f, -0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, 0.70f }, { 1.0f, 0.5f, 0.0f, 1.0f } },
	{ { 0.5f, 0.70f }, { 0.0f, 0.5f, 1.0f, 1.0f } }
	};*/
	static Vertex3D ver11[] = {
		//�O
		{ {-0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,0.0f}},
		{ { 0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 0.1f, 0.1f, 1.0f} ,{1.0f,1.0f}},
		//��
		{ {-0.5f, -0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{0.0f,0.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{1.0f,1.0f}},
		//��
		{ {-0.5f, -0.5f,0.5f }, { -1.0f, 0.0f, 0.0f} ,{0.0f,0.0f}},
		{ { -0.5f, -0.5f,-0.5f }, { -1.0f, 0.0f, 0.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,0.5f }, { -1.0f, 0.0f, 0.0f} ,{0.0f,1.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { -1.0f, 0.0f, 0.0f} ,{1.0f,1.0f}},
		//�E
		{ {0.5f, -0.5f,0.5f }, { 1.0f, 0.0f, 0.0f} ,{0.0f,0.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f} ,{1.0f,0.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 1.0f, 0.0f, 0.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f} ,{1.0f,1.0f}},
		//��
		{ {-0.5f, 0.5f,0.5f }, { 0.0f, 1.0f, 0.0f} ,{0.0f,0.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 0.0f, 1.0f, 0.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { 0.0f, 1.0f, 0.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 0.0f, 1.0f, 0.0f} ,{1.0f,1.0f}},
		//��
		{ {-0.5f, -0.5f,0.5f }, { 0.0f, -1.0f, 0.0f} ,{0.0f,0.0f}},
		{ { 0.5f, -0.5f,0.5f }, { 0.0f, -1.0f, 0.0f} ,{1.0f,0.0f}},
		{ { -0.5f, -0.5f,-0.5f }, { 0.0f, -1.0f, 0.0f} ,{0.0f,1.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 0.0f, -1.0f, 0.0f} ,{1.0f,1.0f}}
	};
	uint32_t index[3] = { 0,1,2 };
	uint32_t index1[] = { 0, 2, 1, 1, 2, 3, 4, 5, 6, 5, 7, 6,
				8, 9, 10, 9, 11, 10, 12, 14, 13, 13, 14, 15,
				16, 18, 17, 17, 18, 19, 20, 21, 22, 21, 23,
				22 };
	PPMLoader* ppm = nullptr;


	//4ch�ɕϊ����鏈���ǉ���
	wchar_t pass[16][60] = {
		L"../../../wall1.ppm",
		L"../../../wallNor1.ppm",
		L"../../../texturePPM/boss1.ppm",
		L"../../../texturePPM/boss1_normal.ppm",
		L"../../../texturePPM/brown_eye.ppm",
		L"../../../texturePPM/classicshoes_texture_diffuse.ppm",
		L"../../../texturePPM/classicshoes_texture_normals.ppm",
		L"../../../texturePPM/eyebrow001.ppm",
		L"../../../texturePPM/jacket01_diffuse.ppm",
		L"../../../texturePPM/jacket01_normals.ppm",
		L"../../../texturePPM/jeans01_black_diffuse.ppm",
		L"../../../texturePPM/jeans01_normals.ppm",
		L"../../../texturePPM/male01_diffuse_black.ppm",
		L"../../../texturePPM/young_lightskinned_male_diffuse.ppm",
		L"../../../Black Dragon NEW/textures/Dragon_Bump_Col2.ppm",
		//L"../../../color_grid.ppm",
		L"../../../Black Dragon NEW/textures/Dragon_Nor_mirror2.ppm"
	};

	int tex1, tex2, tex3, tex4, tex5, tex6;
	int fnum = 16;
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
		S_DELETE(ppm);
	}
	try {
		device->GetTexture(0, "../../../wall1.ppm", ima[0], 256, 256);
		device->GetTexture(0, "../../../wallNor1.ppm", ima[1], 256, 256);
		device->GetTexture(0, "../../../texturePPM/boss1.jpg", ima[2], 256, 256);
		device->GetTexture(0, "../../../texturePPM/boss1_normal.png", ima[3], 256, 256);
		device->GetTexture(0, "../../../texturePPM/brown_eye.png", ima[4], 256, 256);
		device->GetTexture(0, "../../../texturePPM/classicshoes_texture_diffuse.png", ima[5], 256, 256);//
		device->GetTexture(0, "../../../texturePPM/classicshoes_texture_normals.png", ima[6], 256, 256);
		device->GetTexture(0, "../../../texturePPM/eyebrow001.png", ima[7], 256, 256);
		device->GetTexture(0, "../../../texturePPM/jacket01_diffuse.png", ima[8], 256, 256);
		device->GetTexture(0, "../../../texturePPM/jacket01_normals.png", ima[9], 256, 256);
		device->GetTexture(0, "../../../texturePPM/jeans01_black_diffuse.png", ima[10], 256, 256);//
		device->GetTexture(0, "../../../texturePPM/jeans01_normals.png", ima[11], 256, 256);
		device->GetTexture(0, "../../../texturePPM/male01_diffuse_black.png", ima[12], 256, 256);
		device->GetTexture(0, "../../../texturePPM/young_lightskinned_male_diffuse.png", ima[13], 256, 256);
		device->GetTexture(0, "../../../Black Dragon NEW/textures/Dragon_Bump_Col2.jpg", ima[14], 256, 256);
		//device->GetTexture(0,"../../../color_grid.png", ima[14], 256, 256);
		device->GetTexture(0, "../../../Black Dragon NEW/textures/Dragon_Nor_mirror2.jpg", ima[15], 256, 256);

		tex1 = device->getTextureNo("wall1.ppm");
		tex2 = device->getTextureNo("wallNor1.ppm");
		tex3 = device->getTextureNo("boss1.jpg");
		tex4 = device->getTextureNo("boss1_normal.png");
		tex5 = device->getTextureNo("Dragon_Bump_Col2.jpg");
		//tex5 = device->getTextureNo("color_grid.png");
		tex6 = device->getTextureNo("Dragon_Nor_mirror2.jpg");
	}
	catch (std::runtime_error e) {
		std::cerr << "runtime_error: " << e.what() << std::endl;
	}

	Vulkan2D* v2 = new Vulkan2D(device);
	v2->create(ver, 3);
	Vulkan2D* v20 = new Vulkan2D(device);
	v20->create(ver0, 3);
	//VulkanBasicPolygon* v21 = new VulkanBasicPolygon(device);
	//v21->create(ver1, 3, index, 3);
	const int Num = 1;
	VulkanBasicPolygon* v22[Num];
	for (int i = 0; i < Num; i++) {
		v22[i] = new VulkanBasicPolygon(device);
		v22[i]->create(0, 1, ver11, 24, index1, 36);
	}
	VulkanSkinMesh* sk = new VulkanSkinMesh(device);
	VulkanSkinMesh* sk1 = new VulkanSkinMesh(device);
	VulkanSkinMesh* sk2 = new VulkanSkinMesh(device);
	//VulkanSkinMesh* sk3 = new VulkanSkinMesh(device);

	sk2->createChangeTextureArray(1);
	sk2->setChangeTexture(0, tex5, tex6);
	sk2->create("../../../Black Dragon NEW/Dragon_Baked_Actions2.fbx", 100);
	//sk3->create("../../../39-alienanimal_fbx/untitled.fbx", 100);
	sk->createChangeTextureArray(1);
	sk->setChangeTexture(0, -1, 3);
	sk->create("../../../texturePPM/boss1bone.fbx", 200.0f);
	sk->setMaterialParameter(0, { 1,1,1 }, { 0.1f,0.1f,0.1f }, { 0.3f,0.3f,0.3f });
	sk1->create("../../../texturePPM/player1_fbx_att.fbx", 500.0f);
	float the = 180.0f;
	float frame = 0;
	/*g_RenderFunc = [&]()
	{

		if (the++ > 360.0f)the = 0.0f;
		device->updateView({ 0,0,0 }, { 0,0,5 }, { 0,1,0 });
		device->beginCommand(0);
		v2->draw();
		v21->draw({ 0,0,1 }, {0,the++,0});
		device->endCommand(0);
		device->waitFence(0);
	};*/
	ShowWindow(hWnd, nCmdShow);
	ValidateRect(hWnd, 0);// WM_PAINT���Ă΂�Ȃ��悤�ɂ���
	MSG msg;

	/*while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		DispatchMessage(&msg);

	}*/

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {	// PostQuitMessage()���Ă΂ꂽ(�~�����ꂽ)
				break;	//�A�v���I��
			}
			else {
				// ���b�Z�[�W�̖|��ƃf�B�X�p�b�`WindowProc�Ăяo��
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//���[�v������
		T_float::GetTime(hWnd);
		if (the++ > 360.0f)the = 0.0f;
		MATRIX thetaY;
		VECTOR3 light1 = { 0.3f,0.4f,-2.0f };
		VECTOR3 light2 = { -0.3f,-0.4f,2.0f };
		MatrixRotationY(&thetaY, the);
		VectorMatrixMultiply(&light1, &thetaY);
		VectorMatrixMultiply(&light2, &thetaY);
		if (frame++ > 600.0f)frame = 0.0f;
		device->updateView({ 0,-0.2f,-8 }, { 0,0,25 }, { 0,1,0 });
		device->setNumLight(2);
		device->setLight(0, light1, { 1.0f,1.0f,1.0f });
		device->setLight(1, light2, { 1,0.3f,0.3f });
		device->beginCommand(0);
		//v2->draw();
		//v20->draw();
		//v21->draw({ 0.03f,0.0f,10.0f }, { 0,the,0 });
		for (int i = 0; i < Num; i++) {
			v22[i]->setMaterialParameter({ 0.5f,0.5f,0.5f }, { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f });
			v22[i]->draw({ 0.4f * (float)i - 0.7f ,0.7f,0.0f }, { 0,the,0 });
		}
		sk->draw(frame, { 0,0,0 }, { 180,0,0 }, { 2.0f,2.0f,2.0f });
		sk1->draw(frame, { 2,0,0 }, { 90,0.0f,0 }, { 0.2f,0.2f,0.2f });
		sk2->draw(frame, { -2,0,0 }, { 90,0,0 }, { 0.1f,0.1f,0.1f });
		//sk3->draw(frame, { -3,0,0 }, { 90,0,0 }, { 0.1f,0.1f,0.1f });
		device->endCommand(0);
		device->Present(0);
		//���[�v������
	}
	for (int i = 0; i < fnum; i++)ARR_DELETE(ima[i]);
	ARR_DELETE(ima);
	S_DELETE(sk);
	S_DELETE(sk1);
	S_DELETE(sk2);
	//S_DELETE(sk3);
	S_DELETE(v2);
	S_DELETE(v20);
	//S_DELETE(v21);
	for (int i = 0; i < Num; i++)S_DELETE(v22[i]);
	S_DELETE(device);
	S_DELETE(vins);
	return (int)msg.wParam;
}
