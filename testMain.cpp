
#include "../../../VulkanWrapper/VulkanInstance.h"
#include "../../../VulkanWrapper/Vulkan2D.h"
#include "../../../VulkanWrapper/VulkanBasicPolygon.h"
#include "../../../T_float/T_float.h"
#include "../../../ppmLoader/PPMLoader.h"
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
	Device* device = new Device(pd, sur);
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
		//前
		{ {-0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,0.0f}},
		{ { 0.5f, -0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,0.5f }, { 0.0f, 0.0f, 1.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 0.1f, 0.1f, 1.0f} ,{1.0f,1.0f}},
		//後
		{ {-0.5f, -0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{0.0f,0.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 0.0f, 0.0f, -1.0f} ,{1.0f,1.0f}},
		//左
		{ {-0.5f, -0.5f,0.5f }, { -1.0f, 0.0f, 0.0f} ,{0.0f,0.0f}},
		{ { -0.5f, -0.5f,-0.5f }, { -1.0f, 0.0f, 0.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,0.5f }, { -1.0f, 0.0f, 0.0f} ,{0.0f,1.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { -1.0f, 0.0f, 0.0f} ,{1.0f,1.0f}},
		//右
		{ {0.5f, -0.5f,0.5f }, { 1.0f, 0.0f, 0.0f} ,{0.0f,0.0f}},
		{ { 0.5f, -0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f} ,{1.0f,0.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 1.0f, 0.0f, 0.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f} ,{1.0f,1.0f}},
		//上
		{ {-0.5f, 0.5f,0.5f }, { 0.0f, 1.0f, 0.0f} ,{0.0f,0.0f}},
		{ { 0.5f, 0.5f,0.5f }, { 0.0f, 1.0f, 0.0f} ,{1.0f,0.0f}},
		{ { -0.5f, 0.5f,-0.5f }, { 0.0f, 1.0f, 0.0f} ,{0.0f,1.0f}},
		{ { 0.5f, 0.5f,-0.5f }, { 0.0f, 1.0f, 0.0f} ,{1.0f,1.0f}},
		//底
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


	//4chに変換する処理追加↓
	wchar_t pass[2][50] = {
		L"../../../wall1.ppm",
		L"../../../wallNor1.ppm"
	};

	int tex1, tex2;
	unsigned char ima[2][256 * 4 * 256];
	for (int j = 0; j < 2; j++) {
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
		device->GetTexture("../../../wall1.ppm", ima[0], 256, 256);
		device->GetTexture("../../../wallNor1.ppm", ima[1], 256, 256);
		tex1 = device->getTextureNum("wall1.ppm");
		tex2 = device->getTextureNum("wallNor1.ppm");
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
		v22[i]->create(tex1, tex2, ver11, 24, index1, 36);
	}
	float the = 180.0f;
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
	ValidateRect(hWnd, 0);// WM_PAINTが呼ばれないようにする
	MSG msg;

	/*while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		DispatchMessage(&msg);

	}*/

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {	// PostQuitMessage()が呼ばれた(×押された)
				break;	//アプリ終了
			}
			else {
				// メッセージの翻訳とディスパッチWindowProc呼び出し
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//ループ内処理
		T_float::GetTime(hWnd);
		if (the++ > 360.0f)the = 0.0f;
		device->updateView({ 0,-0.2f,0 }, { 0,0,30 }, { 0,1,0 });
		device->setNumLight(2);
		device->setLight(0, { 0.3f,0.4f,2.3f }, { 1.0f,1.0f,1.0f });
		device->setLight(1, { -0.3f,-0.4f,2.3f }, { 1,0.3f,0.3f });
		device->beginCommand(0);
		//v2->draw();
		//v20->draw();
		//v21->draw({ 0.03f,0.0f,10.0f }, { 0,the,0 });
		for (int i = 0; i < Num; i++) {
			v22[i]->setMaterialParameter({ 0.5f,0.5f,0.5f }, { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f });
			v22[i]->draw({ 0.4f * (float)i ,0.0f,3.0f }, { 0,the,0 });
		}
		device->endCommand(0);
		device->waitFence(0);
		//ループ内処理
	}

	S_DELETE(v2);
	S_DELETE(v20);
	//S_DELETE(v21);
	for (int i = 0; i < Num; i++)S_DELETE(v22[i]);
	S_DELETE(device);
	S_DELETE(vins);
	return msg.wParam;
}
