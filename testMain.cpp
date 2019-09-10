
#include "../../../VulkanWrapper/VulkanInstance.h"
#include "../../../VulkanWrapper/Vulkan2D.h"

std::function<void()> g_RenderFunc;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	/*switch (uMsg)
	{
	case WM_DESTROY: PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);*/

	switch (uMsg)
	{
	case WM_DESTROY: PostQuitMessage(0); break;
	case WM_PAINT:
		BeginPaint(hWnd, nullptr); EndPaint(hWnd, nullptr);
		g_RenderFunc();
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
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

	static Vertex2D ver[] = {
	{ { 0.0f, -0.70f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, 0.0f }, { 1.0f, 0.5f, 0.0f, 1.0f } },
	{ { 0.5f, 0.0f }, { 0.0f, 0.5f, 1.0f, 1.0f } }
	};
	static Vertex2D ver1[] = {
	{ { 0.0f, -0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, 0.70f }, { 1.0f, 0.5f, 0.0f, 1.0f } },
	{ { 0.5f, 0.70f }, { 0.0f, 0.5f, 1.0f, 1.0f } }
	};

	Vulkan2D* v2 = new Vulkan2D(device);
	Vulkan2D* v21 = new Vulkan2D(device);
	v2->create(ver, 3);
	v21->create(ver1, 3);
	int i = 0;
	g_RenderFunc = [&]()
	{
		device->beginCommand(0);
        v2->draw();
		v21->draw();
		device->endCommand(0);
		device->waitFence(0);
	};
	ShowWindow(hWnd, nCmdShow);
	//ValidateRect(hWnd, 0);// WM_PAINTが呼ばれないようにする
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}




	/*while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {	// PostQuitMessage()が呼ばれた(×押された)
				return -1;	//アプリ終了
			}
			else {
				// メッセージの翻訳とディスパッチWindowProc呼び出し
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//ループ内処理
		device->d2testDraw();
		//ループ内処理
	}*/

	S_DELETE(v2);
	S_DELETE(device);
	S_DELETE(vins);
	return msg.wParam;
}
