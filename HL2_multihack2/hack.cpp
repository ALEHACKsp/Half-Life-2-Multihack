#include "hack.h"
#include "phook.h"
#include "draw.h"

//Screen
const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
RECT screen;

//Main Data
UINT KeyHook[0xFE];
bool showMenu;
bool detach;

//Data

PTR Game::client;
PTR Game::engine;
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
char Game::oVPunch[4];
char Game::oHPunch[5];
DWORD* rDrawOtherModels;
DWORD* FOV;
float* DuckTime;
float* SetSpeed;
VecPos savedPos;
VecPos teleportPos;
VecPos* pos;

//Menu

bool bhop = false;
bool inst_duck = false;
bool brk_speed = false;
bool triggerbot = false;
bool inf_health = false;
bool inf_armor = false;
bool inf_aux = false;
bool inf_ammo = false;
bool xhair = false;
bool wireframe = false;
bool no_punch = false;
bool fov_changer = false;
int fovVal =  90;
bool savingPos = false;
bool teleporting = false;
bool teleporting_saved = false;

//Functions
void SetPlayerHealth(DWORD value);
void SetPlayerArmor(DWORD value);
void SetPlayerAuxPower(float value);
void ForceAttack();
void ForceJump();
void InfiniteAmmo();
void NoViewPunch();
void SavePos();
void Teleport();

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
		ImGui::Checkbox("Instant Duck", &inst_duck);
		ImGui::Checkbox("Break Max Speed", &brk_speed);
		ImGui::Checkbox("Triggerbot", &triggerbot);
		ImGui::Checkbox("Infinite Health", &inf_health);
		ImGui::Checkbox("Infinite Armor", &inf_armor);
		ImGui::Checkbox("Infinite Aux Power", &inf_aux);
		if (ImGui::Checkbox("Infinite Ammo", &inf_ammo))
			InfiniteAmmo();
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Visuals"))
	{
		ImGui::Checkbox("Wireframe", &wireframe);
		if (ImGui::Checkbox("No Visual Punch", &no_punch))
			NoViewPunch();
		ImGui::Checkbox("Enable FOV Changer", &fov_changer);
		ImGui::SliderInt("FOV (default: 90)", &fovVal, 0, 90);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Visual Teleport"))
	{
		if(Game::localPlayerAddr != NULL)
			ImGui::Text("Current Position: (%f, %f, %f)", pos->x, pos->y, pos->z);
		ImGui::InputFloat("Position X", &teleportPos.x);
		ImGui::InputFloat("Position Y", &teleportPos.y);
		ImGui::InputFloat("Position Z", &teleportPos.z);
		if (ImGui::Button("Save Current Position"))
		{
			savingPos = true;
			teleporting = false;
			teleporting_saved = false;
		}
		if (ImGui::Button("Teleport"))
		{
			savingPos = false;
			teleporting = true;
			teleporting_saved = false;
		}
		if (ImGui::Button("Teleport to Saved Pos"))
		{
			savingPos = false;
			teleporting = false;
			teleporting_saved = true;
		}
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Custom Crosshair"))
	{
		ImGui::Checkbox("Enable", &xhair);
		ImGui::SliderInt("Crosshair Width", &crosshair.size.x, 1, SCREEN_WIDTH - 1);
		ImGui::SliderInt("Crosshair Height", &crosshair.size.y, 1, SCREEN_HEIGHT - 1);
		ImGui::ColorPicker3("Crosshair Color", crosshair.color);
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();
}

DWORD WINAPI Hack::Main(LPVOID lpReserved)
{
	Game::client = (PTR)GetModuleHandle("client.dll");
	Game::engine = (PTR)GetModuleHandle("engine.dll");
	Game::server = (PTR)GetModuleHandle("server.dll");
	Game::shaderapi = (PTR)GetModuleHandle("shaderapidx9.dll");
	memcpy(&Game::oDecAmmo, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo), 2);
	memcpy(&Game::oDecAmmo2, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo2), 6);
	memcpy(&Game::oDecAmmo3, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo3), 6);
	memcpy(&Game::oDecAmmo4, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo4), 2);
	memcpy(&Game::oDecAmmo5, reinterpret_cast<char*>(Game::server + HL2::fDecAmmo5), 2);
	memcpy(&Game::oVPunch, reinterpret_cast<char*>(Game::server + HL2::fVPunch), 4);
	memcpy(&Game::oHPunch, reinterpret_cast<char*>(Game::server + HL2::fHPunch), 5);
	crosshair.size.x = 1;
	crosshair.size.y = 11;
	crosshair.color[0] = 1.f;
	crosshair.color[1] = 0.5f;
	crosshair.color[2] = 0.f;
	bool init_vars = false;
	while (!detach)
	{
		GetClientRect(window, &screen);
		Game::shadersLoaded = *(bool*)(Game::shaderapi + HL2::bShadersLoaded);
		if (Game::shadersLoaded)
		{
			Game::localPlayerAddr = *(PTR*)(Game::server + HL2::dwLocalPlayer);
			if (Game::localPlayerAddr != NULL)
			{
				//vars

				rDrawOtherModels != (DWORD*)(Game::client + HL2::r_drawothermodels) ?
					rDrawOtherModels = (DWORD*)(Game::client + HL2::r_drawothermodels) : 0;
				FOV != (DWORD*)((*(DWORD*)(Game::engine + HL2::dwFovBase)) + HL2::dwFov) ?
					FOV = (DWORD*)((*(DWORD*)(Game::engine + HL2::dwFovBase)) + HL2::dwFov) : 0;
				DuckTime != (float*)((*(DWORD*)(Game::server + HL2::dwDuckTimeBase)) + HL2::flDuckTime) ?
					DuckTime = (float*)((*(DWORD*)(Game::server + HL2::dwDuckTimeBase)) + HL2::flDuckTime) : 0;
				SetSpeed != (float*)((*(DWORD*)(Game::server + HL2::dwSetSpeedBase)) + HL2::flSetSpeed) ?
					SetSpeed = (float*)((*(DWORD*)(Game::server + HL2::dwSetSpeedBase)) + HL2::flSetSpeed) : 0;
				pos != (VecPos*)((*(DWORD*)(Game::engine + HL2::dwPosBase)) + HL2::flPos) ?
					pos = (VecPos*)((*(DWORD*)(Game::engine + HL2::dwPosBase)) + HL2::flPos) : 0;

				//Cheats

				showCrosshair = (bool*)(Game::client + HL2::bShowCrosshair);
				Game::localPlayer = (Player*)(Game::localPlayerAddr);
				triggerbot&& Game::localPlayer->OnTarget == true ? ForceAttack() : void();
				inf_health && Game::localPlayer->Health < MAX_HEALTH ? SetPlayerHealth(MAX_HEALTH) : void();
				inf_armor && Game::localPlayer->Armor < MAX_ARMOR ? SetPlayerArmor(MAX_ARMOR) : void();
				inf_aux && Game::localPlayer->AuxPower < MAX_AUX ? SetPlayerAuxPower(MAX_AUX) : void();
				bhop && KeyHook[KEY_BHOP] == WM_KEYDOWN && !(Game::localPlayer->isOnAir) ? ForceJump() : void();
				inst_duck && *DuckTime > 0 ? *DuckTime = 0 : 0;
				brk_speed && *SetSpeed != 320 ? *SetSpeed = 320 : 0;
				xhair && *showCrosshair != 0 ? *showCrosshair = 0 : 0;
				!xhair && *showCrosshair != 1 ? *showCrosshair = 1 : 0;
				wireframe && *rDrawOtherModels != 2 ? *rDrawOtherModels = 2 : 0;
				!wireframe && *rDrawOtherModels == 2 ? *rDrawOtherModels = 1 : 0;
				fov_changer && *FOV != (DWORD)fovVal ? *FOV = (DWORD)fovVal : 0;
				SavePos();
				Teleport();
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

void ForceJump()
{
	*(DWORD*)(Game::client + HL2::dwForceJump) = 6;
}

void ForceAttack()
{
	*(DWORD*)(Game::client + HL2::dwForceAttack) = 6;
}

void SavePos()
{
	if (savingPos && !teleporting && !teleporting_saved)
	{
		savedPos.x = pos->x;
		savedPos.y = pos->y;
		savedPos.z = pos->z;
		savingPos = false;
	}
}

void Teleport()
{
	if (teleporting && !teleporting_saved && !savingPos)
	{
		pos->x = teleportPos.x;
		pos->y = teleportPos.y;
		pos->z = teleportPos.z;
		teleporting = false;
	}

	else if (teleporting_saved && !teleporting && !savingPos)
	{
		pos->x = savedPos.x;
		pos->y = savedPos.y;
		pos->z = savedPos.z;
		teleporting_saved = false;
	}
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

void NoViewPunch()
{
	if (no_punch)
	{
		char nop[5] = "\x90\x90\x90\x90";
		char nop5[6] = "\x90\x90\x90\x90\x90";
		PHook::WriteBytes((Game::server + HL2::fVPunch), nop, 4);
		PHook::WriteBytes((Game::server + HL2::fHPunch), nop5, 5);
	}

	else
	{
		PHook::WriteBytes((Game::server + HL2::fVPunch), Game::oVPunch, 4);
		PHook::WriteBytes((Game::server + HL2::fHPunch), Game::oHPunch, 5);
	}
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