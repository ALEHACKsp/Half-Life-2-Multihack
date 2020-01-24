#include "hack.h"
#include "phook.h"
#include "draw.h"
//edits
const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
RECT screen;

UINT KeyHook[0xFE];
bool showMenu;
bool detach;

PTR Game::client;
PTR Game::server;
PTR Game::shaderapi;
bool Game::shadersLoaded;
PTR Game::localPlayerAddr;
Player* Game::localPlayer;
bool* showCrosshair;
Crosshair crosshair;
char Game::oDecAmmo[2];
char Game::oDecAmmo2[6];
char Game::oDecAmmo3[6];
char Game::oDecAmmo4[2];
char Game::oDecAmmo5[2];

bool bhop = false;
bool inf_health = false;
bool inf_armor = false;
bool inf_aux = false;
bool inf_ammo = false;
bool xhair = false;

//Functions
void SetPlayerHealth(DWORD value);
void SetPlayerArmor(DWORD value);
void SetPlayerAuxPower(float value);
void InfiniteAmmo();
void ForceJump();

void Hack::KeyHandler()
{
	if (KeyHook[KEY_MENU] == WM_KEYDOWN)
		showMenu = !showMenu;
}

void Hack::DrawMenu()
{
	ImGui::Begin("HL2 multihack by rdbo");
	ImGui::BeginTabBar("navbar");

	if (ImGui::BeginTabItem("Cheats"))
	{
		ImGui::Checkbox("Bunnyhop", &bhop);
		ImGui::Checkbox("Infinite Health", &inf_health);
		ImGui::Checkbox("Infinite Armor", &inf_armor);
		ImGui::Checkbox("Infinite Aux Power", &inf_aux);
		if (ImGui::Checkbox("Infinite Ammo", &inf_ammo))
			InfiniteAmmo();
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Custom Crosshair"))
	{
		ImGui::Checkbox("Enable", &xhair);
		ImGui::SliderInt("Crosshair Width", &crosshair.size.x, 1, SCREEN_WIDTH);
		ImGui::SliderInt("Crosshair Height", &crosshair.size.y, 1, SCREEN_HEIGHT);
		ImGui::ColorPicker3("Crosshair Color", crosshair.color);
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();
}

DWORD WINAPI Hack::Main(LPVOID lpReserved)
{
	Game::client = (PTR)GetModuleHandle("client.dll");
	Game::server = (PTR)GetModuleHandle("server.dll");
	Game::shaderapi = (PTR)GetModuleHandle("shaderapidx9.dll");
	memcpy(&Game::oDecAmmo, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo), 2);
	memcpy(&Game::oDecAmmo2, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo2), 6);
	memcpy(&Game::oDecAmmo3, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo3), 6);
	memcpy(&Game::oDecAmmo4, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo4), 2);
	memcpy(&Game::oDecAmmo5, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo5), 2);
	crosshair.size.x = 1;
	crosshair.size.y = 11;
	crosshair.color[0] = 1.f;
	crosshair.color[1] = 0.5f;
	crosshair.color[2] = 0.f;
	while (!detach)
	{
		GetClientRect(window, &screen);
		Game::shadersLoaded = *(bool*)(Game::shaderapi + HL2::bShadersLoaded);
		if (Game::shadersLoaded)
		{
			Game::localPlayerAddr = *(PTR*)(Game::server + HL2::dwLocalPlayer);
			if (Game::localPlayerAddr != NULL)
			{
				showCrosshair = (bool*)(Game::client + HL2::bShowCrosshair);
				Game::localPlayer = (Player*)(Game::localPlayerAddr);
				inf_health && Game::localPlayer->Health < MAX_HEALTH ? SetPlayerHealth(MAX_HEALTH) : void();
				inf_armor && Game::localPlayer->Armor < MAX_ARMOR ? SetPlayerArmor(MAX_ARMOR) : void();
				inf_aux && Game::localPlayer->AuxPower < MAX_AUX ? SetPlayerAuxPower(MAX_AUX) : void();
				bhop && KeyHook[KEY_BHOP] == WM_KEYDOWN && !(Game::localPlayer->isOnAir) ? ForceJump() : void();
				xhair && *showCrosshair != 0 ? *showCrosshair = 0 : 0;
				!xhair && *showCrosshair != 1 ? *showCrosshair = 1 : 0;
			}
		}
	}
	return TRUE;
}

void SetPlayerHealth(DWORD value)
{
	Game::localPlayer->Health = value;
}

void SetPlayerArmor(DWORD value)
{
	Game::localPlayer->Armor = value;
}

void SetPlayerAuxPower(float value)
{
	Game::localPlayer->AuxPower = value;
}

void InfiniteAmmo()
{
	if (inf_ammo)
	{
		char nop[3] = "\x90\x90";
		char nop6[7] = "\x90\x90\x90\x90\x90\x90";
		PHook::WriteBytes((Game::server + HL2::fDecAmmo), nop, 2);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo2), nop6, 6);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo3), nop6, 6);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo4), nop, 2);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo5), nop, 2);
	}

	else
	{
		PHook::WriteBytes((Game::server + HL2::fDecAmmo), Game::oDecAmmo, 2);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo2), Game::oDecAmmo2, 6);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo3), Game::oDecAmmo3, 6);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo4), Game::oDecAmmo4, 2);
		PHook::WriteBytes((Game::server + HL2::fDecAmmo5), Game::oDecAmmo5, 2);
	}
}

void ForceJump()
{
	*(DWORD*)(Game::client + HL2::dwForceJump) = 6;
}

void Hack::CustomCrosshair()
{
	if (Game::shadersLoaded && xhair)
	{
		D3DCOLOR color = D3DCOLOR_ARGB((int)(255), (int)(crosshair.color[0] * 255), (int)(crosshair.color[1] * 255), (int)(crosshair.color[2] * 255));
		crosshair.size.x % 2 == 1 ? 0 : crosshair.size.x += 1;
		crosshair.size.y % 2 == 1 ? 0 : crosshair.size.y += 1;
		int screenW = screen.right - screen.left;
		int screenH = screen.bottom - screen.top;
		int screenCenterX = (int)(screenW / 2);
		int screenCenterY = (int)(screenH / 2);
		int centerX = (int)(crosshair.size.x / 2);
		int centerY = (int)(crosshair.size.y / 2);
		Draw::DrawFilledRect(screenCenterX - centerX, screenCenterY - centerY, crosshair.size.x, crosshair.size.y, color);
		Draw::DrawFilledRect(screenCenterX - centerY, screenCenterY - centerX, crosshair.size.y, crosshair.size.x, color);
	}
}