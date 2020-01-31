#include "includes.h"
#include "phook.h"
#include "hack.h"
#include "colors.h"
#define IMGUI_WINDOW_W 400
#define IMGUI_WINDOW_H 440

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
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMinSize = ImVec2(IMGUI_WINDOW_W, IMGUI_WINDOW_H);
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = COLOR_WHITE;
	colors[ImGuiCol_WindowBg] = COLOR_BLACK;
	colors[ImGuiCol_TitleBg] = COLOR_DARK_ORANGE;
	colors[ImGuiCol_TitleBgActive] = COLOR_ORANGE;
	colors[ImGuiCol_Tab] = COLOR_DARK_ORANGE;
	colors[ImGuiCol_TabHovered] = COLOR_LIGHT_ORANGE;
	colors[ImGuiCol_TabActive] = COLOR_ORANGE;
	colors[ImGuiCol_CheckMark] = COLOR_ORANGE;
	colors[ImGuiCol_FrameBg] = COLOR_DARK_GRAY;
	colors[ImGuiCol_FrameBgHovered] = COLOR_DARK_GRAY_2;
	colors[ImGuiCol_FrameBgActive] = COLOR_GRAY;
	colors[ImGuiCol_SliderGrab] = COLOR_ORANGE;
	colors[ImGuiCol_SliderGrabActive] = COLOR_LIGHT_ORANGE;
	colors[ImGuiCol_Button] = COLOR_DARK_ORANGE;
	colors[ImGuiCol_ButtonHovered] = COLOR_ORANGE;
	colors[ImGuiCol_ButtonActive] = COLOR_LIGHT_ORANGE;
	colors[ImGuiCol_Border] = COLOR_ORANGE;
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
		if(ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
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