#pragma once
#include "includes.h"
#define MAX_HEALTH 100
#define MAX_ARMOR 100
#define MAX_AUX 100

#define KEY_MENU VK_INSERT
#define KEY_BHOP VK_SPACE

extern bool showMenu;
extern UINT KeyHook[0xFE];
extern bool detach;

namespace Hack
{
	void DrawMenu();
	void KeyHandler();
	void CustomCrosshair();
	DWORD WINAPI Main(LPVOID lpReserved);
}

namespace HL2
{
	//client.dll
	const PTR dwForceJump = 0x48BF5C;
	const PTR bShowCrosshair = 0x489608;
	//server.dll
	const PTR dwLocalPlayer = 0x634174;
	const PTR fDecAmmo = 0xE71CA; // 2 bytes
	const PTR fDecAmmo2 = 0x3D900B; // 6 bytes
	const PTR fDecAmmo3 = 0x3D94E1; // 6 bytes
	const PTR fDecAmmo4 = 0xF0984; // 2 bytes
	const PTR fDecAmmo5 = 0x3BF4B9; // 2 bytes
	//shaderapidx9.dll
	const PTR bShadersLoaded = 0x184778;
	//player
	const PTR dwHealth = 0xE0;
	const PTR bOnAir = 0xB60;
	const PTR dwArmor = 0xD30;
	const PTR strNick = 0xE4C;
	const PTR flAuxPower = 0x10CC;
}

struct iVec2
{
	int x, y;
};

class Crosshair
{
public:
	iVec2 size;
	float color[3];
};

struct Player
{
	char pad[HL2::dwHealth];
	DWORD Health;
	char pad_[HL2::bOnAir - (HL2::dwHealth + sizeof(Health))];
	bool isOnAir;
	char pad__[HL2::dwArmor - (HL2::bOnAir + sizeof(isOnAir))];
	DWORD Armor;
	char pad___[HL2::flAuxPower - (HL2::dwArmor + sizeof(Armor))];
	float AuxPower;
};

namespace Game
{
	extern PTR client;
	extern PTR server;
	extern PTR shaderapi;
	extern bool shadersLoaded;
	extern PTR localPlayerAddr;
	extern Player* localPlayer;
	extern char oDecAmmo[2];
	extern char oDecAmmo2[6];
	extern char oDecAmmo3[6];
	extern char oDecAmmo4[2];
	extern char oDecAmmo5[2];
}