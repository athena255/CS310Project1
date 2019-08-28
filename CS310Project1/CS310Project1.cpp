#include <iostream>
#include <vector>
#include "csgo.hpp"
#include "Mem.h"
#include "vector3.h"
using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

Mem MemMan;

struct localVars
{
	uintptr_t localPlayer;
	uintptr_t gameModule;
	uintptr_t engineModule;
	uintptr_t glowObjectMan;
	uintptr_t playerState;
} var;

struct GlowStruct
{
	BYTE pad_0[4];
	float r;
	float g;
	float b;
	float a;
	BYTE pad_20[16];
	bool renderWhenOccluded;
	bool renderWhenUnOccluded;
	bool fullBloom;
	BYTE pad_40[1];
	BYTE pad_42[4];
};

struct Cache
{
	std::vector<uintptr_t> enemies;
	std::vector<uintptr_t> allies;
	uintptr_t lastTarget;
	int myTeam;
	Vector3 viewAngles;
	uintptr_t glowObjectManager;
} cache;

struct ClrRender
{
	BYTE r, g, b;
};
ClrRender clrTeam, clrEnemy;

void smoothAngle(const Vector3& targetAngle)
// 
{
	Vector3 viewAngles = cache.viewAngles;
	Vector3 delta;
	subtract(targetAngle, viewAngles, &delta);
	// only smooth the angle if delta is large
	float len = mag(delta);
	Vector3 temp = viewAngles;
	if (len > 3.0f) {
		delta.vecScale(2.0 / (len + 1));
		Vector3 temp = viewAngles;
		temp.vecAdd(delta);
		MemMan.writeMem<Vector3>(var.playerState + dwClientState_ViewAngles, temp);
	}
	else
		MemMan.writeMem<Vector3>(var.playerState + dwClientState_ViewAngles, targetAngle);
}

void getHeadPos(uintptr_t bonePtr, Vector3& headPos)
{
	int boneIndex = 0x8;  // head is boneIndex 8
	headPos.x = MemMan.readMem<float>(bonePtr + 0x30*boneIndex + 0xC);
	headPos.y = MemMan.readMem<float>(bonePtr + 0x30 * boneIndex + 0x1C);// *(float*)((bonePtr + 0x30 * boneIndex + 0x1C));
	headPos.z = MemMan.readMem<float>(bonePtr + 0x30 * boneIndex + 0x2C); //*(float*)((bonePtr + 0x30 * boneIndex + 0x2C));
}


bool inFOV(const Vector3& viewAngles, const Vector3& targetAngle)
{
	// inner product needs to be > 0
	return (innerProd(viewAngles, targetAngle) > -90);
}

// get closest target by distance
uintptr_t getBestTarget()
{
	int best = NULL;
	float curDistance = FLT_MAX;
	float newDistance = 0;

	for (int i = 0; i < (int)cache.enemies.size(); i++)
	{
		uintptr_t ent = cache.enemies[i];
		bool dormant = MemMan.readMem<bool>(ent + m_bDormant);
		int eHealth = MemMan.readMem<int>(ent + m_iHealth);
		Vector3 myOrigin = MemMan.readMem<Vector3>(var.localPlayer + m_vecOrigin);
		Vector3 eOrigin = MemMan.readMem<Vector3>(ent + m_vecOrigin);
		Vector3 viewAngles = cache.viewAngles;
		if ( dormant == 0 && (eHealth > 1))
		{
			Vector3 angle;
			// Don't aim at enemies not in our FOV
			calcAngle(myOrigin, eOrigin, &angle);
			//if (!inFOV(viewAngles, angle))
			//	continue;
			newDistance = dist(viewAngles, angle);
			if (newDistance < curDistance)
			{
				curDistance = newDistance;
				best = ent;
			}
		}
	}
	return best;
}

uintptr_t getCrossHairEnt()
{
	uintptr_t ent = NULL;
	int crosshairId = MemMan.readMem<int>(var.localPlayer + m_iCrosshairId);
	if (crosshairId != 0 && crosshairId < 64)
	{
		ent = MemMan.readMem<uintptr_t>(var.engineModule + dwEntityList + crosshairId * 0x10);
		int team = MemMan.readMem<uintptr_t>(ent + m_iTeamNum);
		if (team != cache.myTeam) {
			cache.lastTarget = ent;
		}
	}
	return ent;
}


void SetBrightness()
{
	clrTeam.r = 0;
	clrTeam.g = 0;
	clrTeam.b = 255;

	clrEnemy.r = 255;
	clrEnemy.g = 0;
	clrEnemy.b = 0;

	float brightness = 5.0f;
	int ptr = MemMan.readMem<int>(var.engineModule + model_ambient_min);
	int xorptr = *(int*)&brightness^ptr;
	MemMan.writeMem<int>(var.engineModule + model_ambient_min, xorptr);
}

void SetTeamGlow(uintptr_t ent, int glowIdx, bool isFriendly)
{
	GlowStruct TGlow = MemMan.readMem<GlowStruct>(var.glowObjectMan + (glowIdx * 0x38));
	if (isFriendly)
	{
		TGlow.r = 0.0f;
		TGlow.b = 1.0f;
		TGlow.g = 0.0f;
	}
	else {
		TGlow.r = 1.0f;
		TGlow.g = 0.0f;
		TGlow.b = 0.0f;
	}
	TGlow.a = 1.0f;
	TGlow.renderWhenOccluded = true;
	TGlow.renderWhenUnOccluded = false;
	MemMan.writeMem<GlowStruct>(var.glowObjectMan + (glowIdx * 0x38), TGlow);
}

void resetCache()
{
	cache.enemies.clear();
	cache.allies.clear();
	var.localPlayer = MemMan.readMem<uintptr_t>(var.gameModule + dwLocalPlayer);

	do {
		var.localPlayer = MemMan.readMem<uintptr_t>(var.gameModule + dwLocalPlayer);
	} 
	while (!var.localPlayer);

	var.playerState = MemMan.readMem<uintptr_t>(var.engineModule + dwClientState);
	cache.glowObjectManager = MemMan.readMem<uintptr_t>(var.gameModule + dwGlowObjectManager);
	SetBrightness();

	int myTeam = MemMan.readMem<int>(var.localPlayer + m_iTeamNum);

	if (myTeam == 0) {
		resetCache();
		return;
	}

	cache.myTeam = myTeam;
	for (size_t i = 0; i < 64; i++)
	{
		uintptr_t ent = MemMan.readMem<uintptr_t>(var.gameModule + dwEntityList + i * 0x10);
		if (ent)
		{
			int eTeam = MemMan.readMem<int>(ent + m_iTeamNum);
		
			if (eTeam == myTeam)
				cache.allies.push_back(ent);
			else if (abs(eTeam - myTeam) == 1)
				cache.enemies.push_back(ent);
		}
	}
}

void HandleAim()
{
	Vector3 viewAngles = MemMan.readMem<Vector3>(var.playerState + dwClientState_ViewAngles);
	cache.viewAngles = viewAngles;
	//uintptr_t ent = getCrossHairEnt();
	//if (ent == NULL)
		uintptr_t ent = getBestTarget();
	if (ent) // If we have a target
	{
	/*	uintptr_t entHeadPtr = MemMan.readMem<uintptr_t>(ent + m_dwBoneMatrix);*/
		//Vector3 entHeadPos;
		//getHeadPos(entHeadPtr, entHeadPos);
		// get my eye position
		Vector3 eyePos = MemMan.readMem<Vector3>(var.localPlayer + m_vecOrigin);
		
		uintptr_t entListPtr = MemMan.readMem<uintptr_t>(var.gameModule + dwEntityList);
		Vector3 entOrigin = MemMan.readMem<Vector3>(ent + m_vecOrigin);
		float eyeOffset = MemMan.readMem<float>(entListPtr + m_vecViewOffset + 0x8);
		eyePos.z += eyeOffset;
		// calculate the angle we want to move to
		Vector3 newAngle;
		calcAngle(eyePos, entOrigin, &newAngle);
		// clamp and smooth out the angle
		clamp(viewAngles, newAngle);
		smoothAngle(newAngle);
	}
}

void HandleGlow()
{
	var.glowObjectMan = cache.glowObjectManager;

	for (size_t i = 0; i < cache.enemies.size(); ++i)
	{
		uintptr_t ent = cache.enemies[i];
		int glowIdx = MemMan.readMem<int>(ent + m_iGlowIndex);
		bool dormant = MemMan.readMem<bool>(ent + m_bDormant);
		if (!dormant)
		{
			MemMan.writeMem<ClrRender>(ent + m_clrRender, clrEnemy);
			SetTeamGlow(ent, glowIdx, false);
		}
	}

	for (size_t i = 0; i < cache.allies.size(); ++i)
	{
		uintptr_t ent = cache.allies[i];
		int glowIdx = MemMan.readMem<int>(ent + m_iGlowIndex);
		MemMan.writeMem<ClrRender>(ent + m_clrRender, clrTeam);
		SetTeamGlow(ent, glowIdx, true);
	}
}

int main()
{
	int procId = MemMan.getProcess(L"csgo.exe");
	var.gameModule = MemMan.getModule(procId, L"client_panorama.dll");
	var.engineModule = MemMan.getModule(procId, L"engine.dll");

	resetCache();
	bool bGlow = true, bAim = true, bKeyHeld = false;
	while (true)
	{
		int myHealth = MemMan.readMem<int>(var.localPlayer + m_iHealth);
		if (GetAsyncKeyState(VK_F7) & 1)
			bGlow = !bGlow;

		if (GetAsyncKeyState(VK_F1) & 1)
			resetCache();

		if (GetAsyncKeyState(VK_F6) & 1)
			bAim = !bAim;

		if (GetAsyncKeyState(VK_RBUTTON) == -32768 && !bKeyHeld)
			bKeyHeld = true;


		if (GetAsyncKeyState(VK_RBUTTON) == 0 && bKeyHeld)
			bKeyHeld = false;
		
		if (bGlow && (myHealth > 0))
		{
			HandleGlow();
		}

		if (bAim && bKeyHeld)
		{
			HandleAim();
		}


		Sleep(1);
	}
	return 0;
}
