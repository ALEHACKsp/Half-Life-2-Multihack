#include "includes.h"
#include "phook.h"
#include "hack.h"

HWND window;
void* pDevice[119];
LPDIRECT3DDEVICE9 p_device;
PTR EndSceneAddress;
EndScene oEndScene;
WNDPROC oWndProc;

void InitImGui(LPDIRECT3DDEVICE9 device)
{
	ImGui::CreateContext();
	p_device = device;
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

bool initialized = false;
long __stdcall PHook::hkEndScene(LPDIRECT3DDEVICE9 device)
{
	if (!initialized)
	{
		InitImGui(device);
		initialized = true;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (showMenu)
		Hack::DrawMenu();
	Hack::CustomCrosshair();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return oEndScene(device);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall PHook::WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	if (uMsg == WM_KEYDOWN)
	{
		KeyHook[wParam] = WM_KEYDOWN;
		Hack::KeyHandler();
	}
	if (uMsg == WM_KEYUP)
	{
		KeyHook[wParam] = WM_KEYUP;
		Hack::KeyHandler();
	}
	if (showMenu)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)PHook::Init, hMod, 0, nullptr);
		CreateThread(nullptr, 0, Hack::Main, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)PHook::Shutdown, hMod, 0, nullptr);
		detach = true;
		break;
	}
	return TRUE;
}