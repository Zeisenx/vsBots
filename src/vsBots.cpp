
#pragma once

#include "commands.h"
#include "eventlistener.h"
#include "vsBots.h"
#include "ctimer.h"
#include "utils/entity.h"

extern IVEngineServer2* g_pEngineServer2;

const int DIFFICULTY_MIN = 1;
const int DIFFICULTY_MAX = 12;

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
		g_pEngineServer2->ServerCommand("mp_teamname_1 \"인간 학살 팀\"");
		g_pEngineServer2->ServerCommand("mp_teamname_2 \"꿈도 희망도 없는 팀\"");
	}
	else
	{
		g_humanTeam = CS_TEAM_CT;
		g_zombieTeam = CS_TEAM_T;

		g_pEngineServer2->ServerCommand("mp_humanteam ct");
		g_pEngineServer2->ServerCommand("mp_teamname_2 \"인간 학살 팀\"");
		g_pEngineServer2->ServerCommand("mp_teamname_1 \"꿈도 희망도 없는 팀\"");
	}
}

void vsBots_OnRoundStart(IGameEvent* pEvent)
{
	g_pEngineServer2->ServerCommand("bot_difficulty 3");
	g_pEngineServer2->ServerCommand("mp_flinch_punch_scale 0.0");
}

void vsBots_OnRoundEnd(IGameEvent* pEvent)
{
	int winner = pEvent->GetInt("winner");
	if (winner <= 1)
		return;
	
	int oldLevel = g_difficulty;
	if (winner == g_humanTeam)
		g_difficulty = MAX(DIFFICULTY_MAX, g_difficulty + 1);
	else
		g_difficulty = MIN(DIFFICULTY_MIN, g_difficulty - 1);

	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Level]\x01 %d → %d", oldLevel, g_difficulty);
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

		if (!pController->IsBot())
		{
			CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
			pPawn->m_pItemServices->GiveNamedItem("weapon_healthshot");
			return -1.0f;
		}

		const bool bIsPistolRound = pController->m_pInGameMoneyServices->m_iAccount <= 1000;

		if (!bIsPistolRound)
			pController->m_pInGameMoneyServices->m_iAccount = 16000;

		CCSBot *pBot = pPawn->m_pBot;
		pBot->m_hasVisitedEnemySpawn = true; // it makes bot doesn't rush to enemy spawn
		if (strncmp(pBot->m_name, "[Boss]", 6) == 0)
		{
			pPawn->SetModel("characters/models/tm_phoenix_heavy/tm_phoenix_heavy.vmdl");
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

			pPawn->m_clrRender = Color(255, 0, 0);
		}

		if (strcmp(pBot->m_name, "[Boss] Stone") == 0)
		{
			pPawn->m_clrRender = Color(0, 0, 0);
			UTIL_AddEntityIOEvent(pPawn, "SetScale", nullptr, nullptr, 1.0 + 0.12 * (MAX(1, g_difficulty) - 1));
			pPawn->m_iHealth = 599 * g_difficulty * 400;
		}

		if (strcmp(pBot->m_name, "[Boss] Exp203") == 0)
		{
			pPawn->m_clrRender = Color(0, 255, 0);
			pPawn->m_iHealth = 203;
		}

		return -1.0f;
	});
}
