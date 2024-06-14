
#pragma once

#include "commands.h"
#include "eventlistener.h"
#include "vsBots.h"

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

void vsBots_OnPlayerSpawn(IGameEvent* pEvent)
{
}
