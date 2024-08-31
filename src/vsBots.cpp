
#pragma once

#include "commands.h"
#include "eventlistener.h"
#include "vsBots.h"
#include "ctimer.h"
#include "utils/entity.h"

extern IVEngineServer2* g_pEngineServer2;

#define BOSSMODEL_DEFAULT "characters/models/tm_phoenix_heavy/tm_phoenix_heavy.vmdl"

#define ITEMDEFINDEX_G3SG1 11
#define ITEMDEFINDEX_NEGEV 28
#define ITEMDEFINDEX_SCAR20 38
#define ITEMDEFINDEX_SMOKEGRENADE 45
#define ITEMDEFINDEX_MOLOTOV 46
#define ITEMDEFINDEX_INCGRENADE 48

const int DIFFICULTY_MIN = 0;
const int DIFFICULTY_MAX = 12;

int g_difficulty = 0;
int g_humanTeam = CS_TEAM_CT;
int g_botTeam = CS_TEAM_T;
FAKE_INT_CVAR(vsbots_difficulty, "Bot Difficulty", g_difficulty, false, false)
FAKE_INT_CVAR(vsbots_humanteam, "Human Team", g_humanTeam, false, false)
FAKE_INT_CVAR(vsbots_botteam, "Bot Team", g_botTeam, false, false)

void vsBots_OnLevelInit(char const* pMapName)
{
	if (strncmp(pMapName, "de_", 3) == 0)
	{
		g_humanTeam = CS_TEAM_T;
		g_botTeam = CS_TEAM_CT;

		g_pEngineServer2->ServerCommand("mp_humanteam t");
		g_pEngineServer2->ServerCommand("mp_teamname_1 \"인간 학살 팀\"");
		g_pEngineServer2->ServerCommand("mp_teamname_2 \"꿈도 희망도 없는 팀\"");
	}
	else
	{
		g_humanTeam = CS_TEAM_CT;
		g_botTeam = CS_TEAM_T;

		g_pEngineServer2->ServerCommand("mp_humanteam ct");
		g_pEngineServer2->ServerCommand("mp_teamname_2 \"인간 학살 팀\"");
		g_pEngineServer2->ServerCommand("mp_teamname_1 \"꿈도 희망도 없는 팀\"");
	}

	g_pEngineServer2->ServerCommand("exec cs2fixes/vsbots");
}

void vsBots_Precache(IEntityResourceManifest* pResourceManifest)
{
	pResourceManifest->AddResource(BOSSMODEL_DEFAULT);
}

void vsBots_OnRoundStart(IGameEvent* pEvent)
{
	g_pEngineServer2->ServerCommand("bot_difficulty 3");
	g_pEngineServer2->ServerCommand("mp_flinch_punch_scale 0.0");
}

void vsBots_OnRoundFreezeEnd(IGameEvent* pEvent)
{
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Zeisen Project Discord]\x01 https://discord.gg/tDZUnpaumD");
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Bot Level]\x01 %d", g_difficulty);
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Weapon Restrict]\x01 소이탄(Molotov/Incendiary), 연막탄(Smokegrenade), 네게브(Negev), 딱딱이(G3SG1, SCAR-20)은 \x02금지\x01되어 있습니다.");
}

void vsBots_OnRoundEnd(IGameEvent* pEvent)
{
	int winner = pEvent->GetInt("winner");
	if (winner <= 1)
		return;
	
	int oldLevel = g_difficulty;
	if (winner == g_humanTeam)
		g_difficulty = MIN(DIFFICULTY_MAX, g_difficulty + 1);
	else
		g_difficulty = MAX(DIFFICULTY_MIN, g_difficulty - 1);

	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Level]\x01 %d → %d", oldLevel, g_difficulty);
}

bool vsBots_IsBotHeadOnly(CCSBot* pBot)
{
	if (g_difficulty >= 7)
		return true;
	
	if (strncmp(pBot->m_name, "[Human]", 7) == 0)
		return true;

	return false;
}

void RestrictWeapon(CCSPlayerPawn* pPawn, int itemDefIndex)
{
	CUtlVector<WeaponPurchaseCount_t>* weaponPurchases = pPawn->m_pActionTrackingServices->m_weaponPurchasesThisRound().m_weaponPurchases;
	bool found = false;
	FOR_EACH_VEC(*weaponPurchases, i)
	{
		WeaponPurchaseCount_t& purchase = (*weaponPurchases)[i];
		if (purchase.m_nItemDefIndex == itemDefIndex)
		{
			purchase.m_nCount += 999;
			found = true;
			break;
		}
	}

	if (!found)
	{
		WeaponPurchaseCount_t purchase = {};

		purchase.m_nCount = 999;
		purchase.m_nItemDefIndex = itemDefIndex;

		weaponPurchases->AddToTail(purchase);
	}
}

void vsBots_OnPlayerSpawn(CCSPlayerController *pController)
{
	if (!pController->IsAlive())
		return;
	
	CHandle<CCSPlayerController> handle = pController->GetHandle();
	new CTimer(0.05f, false, false, [handle]()
	{
		CCSPlayerController* pController = (CCSPlayerController*)handle.Get();
		if (!pController)
			return -1.0f;

		CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
		if (!pPawn)
			return -1.0f;

		if (pController->m_iTeamNum == g_humanTeam)
		{
			pPawn->m_iHealth = g_difficulty == 0 ? 999 : (500 - 50 * (g_difficulty - 1));
			pPawn->m_pItemServices->GiveNamedItem("weapon_healthshot");
		}

		if (!pController->IsBot())
		{
			CCSPlayerPawn* pPawn = pController->GetPlayerPawn();

			RestrictWeapon(pPawn, ITEMDEFINDEX_G3SG1);
			RestrictWeapon(pPawn, ITEMDEFINDEX_SCAR20);
			RestrictWeapon(pPawn, ITEMDEFINDEX_NEGEV);
			RestrictWeapon(pPawn, ITEMDEFINDEX_SMOKEGRENADE);
			RestrictWeapon(pPawn, ITEMDEFINDEX_MOLOTOV);
			RestrictWeapon(pPawn, ITEMDEFINDEX_INCGRENADE);

			return -1.0f;
		}


		CCSBot* pBot = pPawn->m_pBot;
		if (pController->m_iTeamNum == g_humanTeam && strncmp(pBot->m_name, "[Human]", 7) != 0)
		{
			pController->SwitchTeam(g_botTeam);
			pPawn->CommitSuicide(false, false);
			return -1.0f;
		}

		const bool bIsPistolRound = pController->m_pInGameMoneyServices->m_iAccount <= 1000;

		if (!bIsPistolRound)
			pController->m_pInGameMoneyServices->m_iAccount = 16000;

		pBot->m_hasVisitedEnemySpawn = true; // it makes bot doesn't rush to enemy spawn
		if (strncmp(pBot->m_name, "[Boss]", 6) == 0)
		{
			pPawn->SetModel(BOSSMODEL_DEFAULT);
			pPawn->m_ArmorValue = 9999;
			pPawn->m_pItemServices->m_bHasHelmet = true;

			pController->m_iScore = 50;
			pController->m_pInGameMoneyServices->m_iAccount = 0;
		}

		if (strcmp(pBot->m_name, "[Boss] Crusher") == 0)
		{
			if (!bIsPistolRound)
				pPawn->m_pItemServices->GiveNamedItem("weapon_xm1014");
			pController->m_iScore = 100;

			pPawn->m_clrRender = Color(255, 0, 0, 255);
		}

		if (strcmp(pBot->m_name, "[Boss] Stone") == 0)
		{
			pPawn->m_clrRender = Color(0, 0, 0, 255);
			UTIL_AddEntityIOEvent(pPawn, "SetScale", nullptr, nullptr, 1.0 + 0.12 * (MAX(1, g_difficulty) - 1));
			pPawn->m_iHealth = 599 + g_difficulty * 400;

			pController->GetZEPlayer()->SetSpeedMod(1.0 + g_difficulty * 0.03);
		}

		if (strcmp(pBot->m_name, "[Boss] Exp203") == 0)
		{
			pPawn->m_clrRender = Color(0, 255, 0, 255);
			pPawn->m_iHealth = 203;
		}

		return -1.0f;
	});
}

bool vsBots_Hook_OnTakeDamage_Alive(CTakeDamageInfo* pInfo, CCSPlayerPawn* pVictimPawn)
{
	CCSPlayerPawn* pAttackerPawn = (CCSPlayerPawn*)pInfo->m_hAttacker.Get();

	if (!(pAttackerPawn && pVictimPawn && pAttackerPawn->IsPawn() && pVictimPawn->IsPawn()))
		return false;

	CCSPlayerController* pAttackerController = CCSPlayerController::FromPawn(pAttackerPawn);
	CCSPlayerController* pVictimController = CCSPlayerController::FromPawn(pVictimPawn);
	if (pAttackerController->IsBot())
	{
		if ((g_difficulty >= 1 && strcmp(pAttackerController->GetPlayerName(), "[Boss] Crusher") == 0) ||
			strcmp(pAttackerController->GetPlayerName(), "[Boss] Stone") == 0)
		{
			pInfo->m_flDamage = 9999.0;
		}
	}

	if (pVictimController->IsBot())
	{
		if (strcmp(pVictimController->GetPlayerName(), "[Boss] Crusher") == 0)
		{
			float regenTime = 0.2f;
			if (g_difficulty >= 4) regenTime = 0.15f;
			if (g_difficulty >= 8) regenTime = 0.1f;

			CHandle<CCSPlayerController> victimHandle = pVictimController->GetHandle();
			new CTimer(regenTime, false, false, [victimHandle]()
				{
					CCSPlayerController* pVictim = (CCSPlayerController*)victimHandle.Get();
					if (!pVictim || !pVictim->IsAlive())
						return -1.0f;

					CCSPlayerPawn* pVictimPawn = pVictim->GetPlayerPawn();
					if (!(pVictimPawn && pVictimPawn && pVictimPawn->IsPawn()))
						return -1.0f;

					pVictimPawn->m_iHealth = pVictimPawn->m_iMaxHealth;
				});
		}


		if (strcmp(pVictimController->GetPlayerName(), "[Boss] Stone") == 0)
		{
			CHandle<CCSPlayerController> victimHandle = pVictimController->GetHandle();
			new CTimer(0.0f, false, false, [victimHandle]()
			{
				CCSPlayerController* pVictimController = (CCSPlayerController*)victimHandle.Get();
				if (!pVictimController || !pVictimController->IsAlive())
					return -1.0f;

				CCSPlayerPawn* pVictimPawn = pVictimController->GetPlayerPawn();
				if (!(pVictimPawn && pVictimPawn && pVictimPawn->IsPawn()))
					return -1.0f;

				CPlayer_MovementServices* pMovementService = pVictimPawn->m_pMovementServices;
				CCSPlayer_MovementServices* pStaminaService = (CCSPlayer_MovementServices*)pMovementService;

				if (!pMovementService || !pStaminaService)
					return -1.0f;

				pVictimPawn->m_flVelocityModifier = 1.0;
				pStaminaService->m_flStamina = 0.0;
			});
		}
	}

	return false;
}

void vsBots_OnPlayerHurt(IGameEvent* pEvent)
{
	CCSPlayerController* pAttacker = (CCSPlayerController*)pEvent->GetPlayerController("attacker");
	CCSPlayerController* pVictim = (CCSPlayerController*)pEvent->GetPlayerController("userid");

	if (!pAttacker || !pVictim)
		return;
}

void vsBots_OnPlayerDeath(IGameEvent* pEvent)
{
	CCSPlayerController* pAttacker = (CCSPlayerController*)pEvent->GetPlayerController("attacker");
	CCSPlayerController* pVictim = (CCSPlayerController*)pEvent->GetPlayerController("userid");

	if (!pAttacker || !pVictim)
		return;

	if (pVictim->IsBot())
	{
		if (strncmp(pVictim->GetPlayerName(), "[Boss]", 6) == 0)
		{
			ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Boss Kill]\x0C %s 유저\x01가 \x02%s 보스를 처치했습니다!",
				pAttacker->GetPlayerName(), pVictim->GetPlayerName());
		}
	}
}