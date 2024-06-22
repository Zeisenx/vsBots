
#pragma once

#include "commands.h"
#include "eventlistener.h"
#include "vsBots.h"
#include "ctimer.h"

extern IVEngineServer2* g_pEngineServer2;

int g_difficulty = 1;
int g_humanTeam = CS_TEAM_CT;
int g_zombieTeam = CS_TEAM_T;
FAKE_INT_CVAR(vsbots_difficulty, "Bot Difficulty", g_difficulty, false, false)
FAKE_INT_CVAR(vsbots_humanteam, "Human Team", g_humanTeam, false, false)
FAKE_INT_CVAR(vsbots_zombieteam, "Zombie Team", g_zombieTeam, false, false)

void vsBots_OnLevelInit(char const* pMapName)
{
	if (strncmp(pMapName, "de_", 3) == 0)
	{
		g_humanTeam = CS_TEAM_T;
		g_zombieTeam = CS_TEAM_CT;

		g_pEngineServer2->ServerCommand("mp_humanteam t");
	}
	else
	{
		g_humanTeam = CS_TEAM_CT;
		g_zombieTeam = CS_TEAM_T;

		g_pEngineServer2->ServerCommand("mp_humanteam ct");
	}
}

void vsBots_OnPlayerSpawn(CCSPlayerController *pController)
{
	if (!pController->IsAlive())
		return;
	
	if (!pController->IsBot())
	{
		CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
		pPawn->m_pItemServices->GiveNamedItem("weapon_healthshot");
		return;
	}

	CHandle<CCSPlayerController> handle = pController->GetHandle();
	new CTimer(0.05f, false, false, [handle]()
	{
		CCSPlayerController* pController = (CCSPlayerController*)handle.Get();
		if (!pController)
			return -1.0f;

		CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
		if (!pPawn)
			return -1.0f;

		const bool bIsPistolRound = pController->m_pInGameMoneyServices->m_iAccount <= 1000;

		if (!bIsPistolRound)
			pController->m_pInGameMoneyServices->m_iAccount = 16000;

		CCSBot *pBot = pPawn->m_pBot;
		pBot->m_hasVisitedEnemySpawn = true; // it makes bot doesn't rush to enemy spawn
		if (strncmp(pBot->m_name, "[Boss]", 6) == 0)
		{
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
		}

		return -1.0f;
	});
}
