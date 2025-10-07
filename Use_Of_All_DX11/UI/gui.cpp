//本项目使用了imgui库，感谢原作者
//使用的是C++17语言
//大家可以去看看 https://github.com/pain1929/imgui_dx11 这位作者的项目

#include <Windows.h>
#include "..\ImGui\imgui.h"
#include "..\ImGui\imgui_impl_dx11.h"
#include "..\ImGui\imgui_impl_win32.h"
#include "..\Hooks\MinHook.h"
#include <d3d11.h>
#include <stdio.h>
#pragma comment(lib,"d3d11.lib")

static ID3D11Device* g_pd3dDevice = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static ID3D11RenderTargetView* view = nullptr;
static HWND g_hwnd = nullptr;
void* origin_present = nullptr;
WNDPROC origin_wndProc;
using Present = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);




namespace MainUI
{
	bool inited = false;
	bool ShowGui = true;
	bool Checkbox = false;
	float SliderFloat = 0.0f;
	char TextInput[256] = "https://github.com/pain1929/imgui_dx11";

	CHAR RenderBuffer[4096] = { 0 };
	LRESULT __stdcall WndProc(HWND hWnd,UINT uMsg,WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		{
			return true;
		}
		//设置“Home”键显示隐藏菜单

		/*if (uMsg == WM_KEYDOWN && wParam == VK_HOME)
		{
			ShowGui = !ShowGui;
		}
		if (ShowGui && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE))
		{
			return true;
		}*/
		return CallWindowProc(origin_wndProc, hWnd, uMsg, wParam, lParam);
	}

	HRESULT __stdcall My_Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{

		if (!inited)
		{
			pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice);
			g_pd3dDevice->GetImmediateContext(&g_pd3dContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			g_hwnd = sd.OutputWindow;
			ID3D11Texture2D* buf{};
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buf);
			g_pd3dDevice->CreateRenderTargetView(buf, nullptr, &view);
			buf->Release();
			origin_wndProc = (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.IniFilename = nullptr;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImVector<ImWchar> ranges;
			ImFontGlyphRangesBuilder builder;
			builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
			builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
			builder.BuildRanges(&ranges);
			io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyhbd.ttc", 20.0f, NULL, ranges.Data);
			ImGui_ImplWin32_Init(g_hwnd);
			ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);
			inited = true;
		}
		else
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			{
				//显示FPS
				sprintf(RenderBuffer, "FPS: %.2f", ImGui::GetIO().Framerate);
				ImGui::GetForegroundDrawList()->AddText({ 5.f, 5.f }, ImColor(0, 225, 225, 225), RenderBuffer);

				if (ShowGui)
				{
					//绘制主菜单
					ImGui::Begin("Elarcanine", nullptr, NULL);
					/*ImGui::SetWindowSize({ 800.f,600.f }, ImGuiCond_Always);*/  //设置窗口大小
					ImGui::Text(u8"原作者：github https://github.com/pain1929/imgui_dx11");
					ImGui::Text(u8"二改，给绘制拔出来，加中文：Elarcanine");
					ImGui::Button(u8"这是按钮");
					ImGui::Checkbox(u8"这是一个框框", &Checkbox);
					ImGui::SliderFloat(u8"这是一个滑动条", &SliderFloat, 0.0f, 10.0f);
					ImGui::InputText(u8"这是一个输入框", TextInput, sizeof(TextInput));
					ImGui::NewLine();
					ImGui::Text(u8"这个imgui的界面理论上适用与所有使用DX11的程序，无需更新。");

					if (ImGui::Button(u8"退出"))
					{
						TerminateProcess(GetCurrentProcess(), 0);
					}
					ImGui::End();
				}
			}
			ImGui::EndFrame();
			ImGui::Render();
			g_pd3dContext->OMSetRenderTargets(1, &view, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
		return ((Present)origin_present)(pSwapChain, SyncInterval, Flags);
	}



	DWORD create(void*)
	{
		const unsigned level_count = 2;
		D3D_FEATURE_LEVEL levels[level_count] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
		DXGI_SWAP_CHAIN_DESC sd{};
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = GetForegroundWindow();
		sd.SampleDesc.Count = 1;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		auto hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			levels,
			level_count,
			D3D11_SDK_VERSION,
			&sd,
			&g_pSwapChain,
			&g_pd3dDevice,
			nullptr,
			nullptr);

		if (g_pSwapChain) {
			auto vtable_ptr = (void***)(g_pSwapChain);
			auto vtable = *vtable_ptr;
			auto present = vtable[8];
			MH_Initialize();
			MH_CreateHook(present, &My_Present, &origin_present);
			MH_EnableHook(present);
			g_pd3dDevice->Release();
			g_pSwapChain->Release();
		}

		return 0;
	}
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		CreateThread(NULL, 0, MainUI::create, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{

	}
	return TRUE;
}