
#pragma once

#include "commands.h"
#include "eventlistener.h"
#include "vsbots.h"
#include "ctimer.h"
#include "utils/entity.h"
#include "entity/cgamerules.h"
#include "entity/cenvexplosion.h"
#include "entity/ccsbot.h"
#include "recipientfilters.h"
#include "cs_usercmd.pb.h"
#include "database.h"
#include <fstream>
#include <regex>

BotProfileManager* g_pTheBotProfiles = nullptr;

extern IVEngineServer2* g_pEngineServer2;
extern CCSGameRules* g_pGameRules;
extern CGlobalVars* gpGlobals;

#define BOSSMODEL_DEFAULT "characters/models/tm_phoenix_heavy/tm_phoenix_heavy.vmdl"

enum ItemDefIndexIDs : short
{
	ITEMDEFINDEX_DEAGLE = 1,
	ITEMDEFINDEX_AUG = 8,
	ITEMDEFINDEX_AWP = 9,
	ITEMDEFINDEX_FAMAS = 10,
	ITEMDEFINDEX_G3SG1 = 11,
	ITEMDEFINDEX_M249 = 14,
	ITEMDEFINDEX_M4A1 = 16,
	ITEMDEFINDEX_MAC10 = 17,
	ITEMDEFINDEX_P90 = 19,
	ITEMDEFINDEX_MP5SD = 23,
	ITEMDEFINDEX_UMP45 = 24,
	ITEMDEFINDEX_XM1014 = 25,
	ITEMDEFINDEX_BIZON = 26,
	ITEMDEFINDEX_MAG7 = 27,
	ITEMDEFINDEX_NEGEV = 28,
	ITEMDEFINDEX_SAWEDOFF = 29,
	ITEMDEFINDEX_TASER = 31,
	ITEMDEFINDEX_MP7 = 33,
	ITEMDEFINDEX_MP9 = 34,
	ITEMDEFINDEX_NOVA = 35,
	ITEMDEFINDEX_P250 = 36,
	ITEMDEFINDEX_SCAR20 = 38,
	ITEMDEFINDEX_SSG08 = 40,
	ITEMDEFINDEX_SMOKEGRENADE = 45,
	ITEMDEFINDEX_MOLOTOV = 46,
	ITEMDEFINDEX_INCGRENADE = 48,
	ITEMDEFINDEX_CZ75A = 63,
	ITEMDEFINDEX_M4A1_SILENCER = 60,
	ITEMDEFINDEX_USP_SILENCER = 61,
	ITEMDEFINDEX_REVOLVER = 64,
};

const int DIFFICULTY_MIN = 1;
const int DIFFICULTY_MAX = 12;

bool g_bCrusherHasShotgun = false;
int g_humanTeam = CS_TEAM_CT;
int g_botTeam = CS_TEAM_T;

bool g_bScoreboardRankUpdate = true;

int g_iSwitchChance = 50;
int g_iDifficulty = DIFFICULTY_MIN;
bool g_bPlayerGlowEnabled = false;
FAKE_INT_CVAR(vsbots_switchchance, "Switch chance", g_iSwitchChance, 50, false)
FAKE_INT_CVAR(vsbots_difficulty, "Bot Difficulty", g_iDifficulty, DIFFICULTY_MIN, false)
FAKE_BOOL_CVAR(vsbots_player_glow, "Player Glow", g_bPlayerGlowEnabled, false, false)

KeyValues* g_pKVPrintText;

int GetPlayerRating(CCSPlayerController* pController);
void RestrictWeapons(CCSPlayerPawn* pPawn);
void RestrictWeapon(CCSPlayerPawn* pPawn, int itemDefIndex);
void LoadPrintTextKV();
void DuplicateSpawnPoint(int team, int maxCount);
void AddHumanBots();
void SetHumanHealth();
void EmitSoundToAll(const char* szPath, float volume = 1.0f);
void vsBots_OnSayTextPost(CCSPlayerController* pAuthor, const char* pText);

void vsBots_OnLevelInit(char const* pMapName)
{
	LoadPrintTextKV();

	char cmdFmt[128];

	bool isHumanTSide = strncmp(pMapName, "de_", 3) == 0;
	if (rand() % 100 < g_iSwitchChance)
		isHumanTSide = !isHumanTSide;

	if (isHumanTSide)
	{
		g_humanTeam = CS_TEAM_T;
		g_botTeam = CS_TEAM_CT;

		g_pEngineServer2->ServerCommand("mp_humanteam t");

		V_snprintf(cmdFmt, sizeof(cmdFmt), "mp_teamname_1 \"%s\"", g_pKVPrintText->GetString("TeamName_BotTeam", "Bot Team"));
		g_pEngineServer2->ServerCommand(cmdFmt);
		V_snprintf(cmdFmt, sizeof(cmdFmt), "mp_teamname_2 \"%s\"", g_pKVPrintText->GetString("TeamName_HumanTeam", "Human Team"));
		g_pEngineServer2->ServerCommand(cmdFmt);
	}
	else
	{
		g_humanTeam = CS_TEAM_CT;
		g_botTeam = CS_TEAM_T;

		g_pEngineServer2->ServerCommand("mp_humanteam ct");

		V_snprintf(cmdFmt, sizeof(cmdFmt), "mp_teamname_2 \"%s\"", g_pKVPrintText->GetString("TeamName_BotTeam", "Bot Team"));
		g_pEngineServer2->ServerCommand(cmdFmt);
		V_snprintf(cmdFmt, sizeof(cmdFmt), "mp_teamname_1 \"%s\"", g_pKVPrintText->GetString("TeamName_HumanTeam", "Human Team"));
		g_pEngineServer2->ServerCommand(cmdFmt);
	}

	g_pEngineServer2->ServerCommand("exec cs2fixes/vsbots");
}

void LoadPrintTextKV()
{
	if (g_pKVPrintText)
		delete g_pKVPrintText;

	g_pKVPrintText = new KeyValues("PrintText");

	const char* pszPath = "addons/cs2fixes/configs/vsbots/printtext.cfg";

	if (!g_pKVPrintText->LoadFromFile(g_pFullFileSystem, pszPath))
	{
		Warning("Failed to load %s\n", pszPath);
		return;
	}
}

void CPrintChatToAll(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	std::string bufStr = buf;

	bufStr = std::regex_replace(bufStr, std::regex("\\{default\\}"), "\x01");
	bufStr = std::regex_replace(bufStr, std::regex("\\{red\\}"), "\x02");
	bufStr = std::regex_replace(bufStr, std::regex("\\{purple\\}"), "\x04");
	bufStr = std::regex_replace(bufStr, std::regex("\\{green\\}"), "\x04");
	bufStr = std::regex_replace(bufStr, std::regex("\\{gold\\}"), "\x09");
	bufStr = std::regex_replace(bufStr, std::regex("\\{blue\\}"), "\x0C");
	ClientPrintAll(HUD_PRINTTALK, bufStr.c_str());
}

void DuplicateSpawnPoint(int team, int maxCount)
{
	CUtlVector<SpawnPoint*>* botTeamSpawns = team == CS_TEAM_T ? g_pGameRules->m_TerroristSpawnPoints() : g_pGameRules->m_CTSpawnPoints();

	const int botsCount = 50;
	size_t addCount = botsCount - botTeamSpawns->Count();
	Message("DuplicateSpawnPoint() : addCount : %d\n", addCount);
	if (addCount <= 0)
		return;

	for (int i = 1; i <= addCount; i++)
	{
		SpawnPoint* spawnEntity = CreateEntityByName<SpawnPoint>(team == CS_TEAM_T ? "info_player_terrorist" : "info_player_counterterrorist");

		CEntityKeyValues* pKeyValues = new CEntityKeyValues();

		spawnEntity->SetAbsOrigin((*botTeamSpawns)[i % botTeamSpawns->Count()]->GetAbsOrigin());
		spawnEntity->SetAbsRotation((*botTeamSpawns)[i % botTeamSpawns->Count()]->GetAbsRotation());
		spawnEntity->DispatchSpawn();
	}
}

static char* CloneString(const char* str)
{
	char* cloneStr = new char[strlen(str) + 1];
	strcpy(cloneStr, str);
	return cloneStr;
}

void vsBots_Detour_BotProfileManager_InitPost(BotProfileManager* botProfileManager, const char* filename, unsigned int* checksum)
{
	vsBots_LoadBotNames();
}

void vsBots_LoadBotNames()
{
	Message("vsBots_LoadBotNames()\n");
	const char* pszFilePath = "addons/cs2fixes/configs/vsBots/botnames.txt";
	char szPath[MAX_PATH];
	V_snprintf(szPath, sizeof(szPath), "%s%s%s", Plat_GetGameDirectory(), "/csgo/", pszFilePath);
	std::ifstream infile(szPath);
	std::string botName;
	while (std::getline(infile, botName))
	{
		if (g_pTheBotProfiles && !g_pTheBotProfiles->GetProfile(botName.c_str()))
		{
			BotProfile* pBotProfile = new BotProfile();
			pBotProfile->m_name = CloneString(botName.c_str());

			g_pTheBotProfiles->m_profileList.AddToTail(pBotProfile);
		}
	}
}

void vsBots_Precache(IEntityResourceManifest* pResourceManifest)
{
	pResourceManifest->AddResource(BOSSMODEL_DEFAULT);
	
	pResourceManifest->AddResource("particles/explosions_fx/explosion_c4_500.vpcf");
	pResourceManifest->AddResource("particles/cs2fixes/leader_tracer.vpcf");
}

void vsBots_OnGameEnd()
{
	g_bScoreboardRankUpdate = false;
}

void vsBots_OnRoundStart(IGameEvent* pEvent)
{
	g_bScoreboardRankUpdate = true;
	g_pEngineServer2->ServerCommand("bot_difficulty 3");
	g_pEngineServer2->ServerCommand("mp_flinch_punch_scale 0.0");

	DuplicateSpawnPoint(g_botTeam, 50);
	DuplicateSpawnPoint(g_humanTeam, 15);

	FOR_EACH_LL(g_pTheBotProfiles->m_profileList, it)
	{
		BotProfile* profile = g_pTheBotProfiles->m_profileList[it];

		if (V_strncmp(profile->GetName(), "[Boss]", 6) != 0 &&
			V_strncmp(profile->GetName(), "[  *  ]", 7) != 0)
			continue;

		char szBotAddCmd[128];
		V_snprintf(szBotAddCmd, sizeof(szBotAddCmd), "bot_add \"%s\"", profile->GetName());
		g_pEngineServer2->ServerCommand(szBotAddCmd);
	}

	AddHumanBots();
}

void vsBots_OnRoundFreezeEnd(IGameEvent* pEvent)
{
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Zeisen Project Discord]\x01 https://discord.gg/tDZUnpaumD");
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Bot Level]\x01 %d", g_iDifficulty);
	CPrintChatToAll(g_pKVPrintText->GetString("Message_WeaponRestrict", "Weapon Restrict"));
}

void vsBots_OnRoundEnd(IGameEvent* pEvent)
{
	int winner = pEvent->GetInt("winner");
	if (winner <= 1)
		return;
	
	int oldLevel = g_iDifficulty;
	if (winner == g_humanTeam)
		g_iDifficulty = MIN(DIFFICULTY_MAX, g_iDifficulty + 1);
	else
		g_iDifficulty = MAX(DIFFICULTY_MIN, g_iDifficulty - 1);

	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Level]\x01 %d → %d", oldLevel, g_iDifficulty);

	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || !pController->IsConnected())
			continue;

		CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
		if (!pPawn)
			continue;

		RestrictWeapons(pPawn);

		if (!pController->IsBot() && pPawn->m_iTeamNum == winner)
		{
			DBInfo info = pController->GetZEPlayer()->GetDBInfo();
			info.iWinPoint += oldLevel;
			pController->GetZEPlayer()->SetDBInfo(info);
		}
	}

}

void RestrictWeapons(CCSPlayerPawn* pPawn)
{
	RestrictWeapon(pPawn, ITEMDEFINDEX_NEGEV);
	RestrictWeapon(pPawn, ITEMDEFINDEX_SMOKEGRENADE);
	RestrictWeapon(pPawn, ITEMDEFINDEX_TASER);
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

// @Todo : Clean up this shit, I guess no one can read this expect me
void vsBots_OnPlayerSpawn(CCSPlayerController *pController)
{
	if (!pController->IsAlive())
		return;
	
	CHandle<CCSPlayerController> handle = pController->GetHandle();
	new CTimer(0.0f, false, false, [handle]()
	{
		CCSPlayerController* pController = (CCSPlayerController*)handle.Get();
		if (!pController)
			return -1.0f;

		CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
		if (!pPawn)
			return -1.0f;

		CCSBot* pBot = pPawn->m_pBot;
		if (pBot && pController->m_iTeamNum == g_humanTeam && V_strncmp(pBot->m_name, "[Human]", 7) != 0)
		{
			pController->SwitchTeam(g_botTeam);

			// Todo : silent team change
			Vector pos = Vector(0, 0, 0);
			pPawn->Teleport(&pos, nullptr, nullptr);
			pPawn->CommitSuicide(false, false);
			return -1.0f;
		}

		if (pController->m_iTeamNum == g_humanTeam)
		{
			int health = MAX(100, g_iDifficulty == 0 ? 999 : (500 - 50 * (g_iDifficulty - 1)));
			SetHumanHealth();

			if (!g_pGameRules->m_bWarmupPeriod)
				pPawn->m_pItemServices->GiveNamedItem("weapon_healthshot");

			RestrictWeapons(pPawn);

			ZEPlayer* pPlayerTarget = pController->GetZEPlayer();
			if (g_bPlayerGlowEnabled && !pPlayerTarget->GetGlowModel())
				pPlayerTarget->StartGlow(Color(0, 255, 0, 255), -1);

			if (pController->IsBot())
			{
				BotProfile* pBotProfile = pBot->GetLocalProfile();
				pBotProfile->m_skill = 1.0f;
				pBotProfile->m_aggression = 1.0f;
				pBotProfile->m_teamwork = 1.0f;

				pBotProfile->m_reactionTime = 0.0f;
				pBotProfile->m_attackDelay = 0.0f;
			}
		}

		if (!pBot)
			return -1.0f;
		
		const bool bIsPistolRound = pController->m_pInGameMoneyServices->m_iAccount <= 1000;

		if (!bIsPistolRound)
			pController->m_pInGameMoneyServices->m_iAccount = 16000;
		
		pBot->m_hasVisitedEnemySpawn = true; // it makes bot doesn't rush to enemy spawn

		BotProfile* pBotProfile = pBot->GetLocalProfile();
		if (pController->m_iTeamNum == g_botTeam)
		{
			float skill = MIN(1.0f, 0.2f + 0.08f * (g_iDifficulty-1));
			pBotProfile->m_skill = skill;
			pBotProfile->m_aggression = skill;
			pBotProfile->m_teamwork = 0.5f;

			float reactionTime = MAX(0.0f, 3.0f - 0.3f * (g_iDifficulty-1));
			pBotProfile->m_reactionTime = reactionTime;
			pBotProfile->m_attackDelay = reactionTime;
		}

		if (V_strncmp(pBot->m_name, "[Boss]", 6) == 0)
		{
			pPawn->SetModel(BOSSMODEL_DEFAULT);
			pPawn->m_ArmorValue = 9999;
			if (!bIsPistolRound)
				pPawn->m_pItemServices->m_bHasHelmet = true;

			pController->m_pActionTrackingServices->m_matchStats().m_iDamage = 9999.0f;

			pController->m_iScore = 50;
			pController->m_pInGameMoneyServices->m_iAccount = 0;

			pBotProfile->m_skill = 1.0f;
			pBotProfile->m_aggression = 1.0f;
			pBotProfile->m_reactionTime = 0.0f;
			pBotProfile->m_attackDelay = 0.0f;
		}

		if (V_strncmp(pBot->m_name, "[Boss] Crusher", 14) == 0)
		{
			if (!bIsPistolRound)
			{
				pPawn->m_pItemServices->StripPlayerWeapons(false);
				pPawn->m_pItemServices->GiveNamedItem("weapon_xm1014");
				g_bCrusherHasShotgun = true;
			}
			else
				g_bCrusherHasShotgun = false;

			pController->m_iScore = 100;

			pPawn->m_clrRender = Color(255, 0, 0, 255);
			pController->m_pActionTrackingServices->m_matchStats().m_iDamage = 99999.0f;
		}

		if (V_strncmp(pBot->m_name, "[Boss] Stone", 12) == 0)
		{
			pPawn->m_clrRender = Color(0, 0, 0, 255);
			UTIL_AddEntityIOEvent(pPawn, "SetScale", nullptr, nullptr, 1.0 + 0.12 * (MAX(1, g_iDifficulty) - 1));

			int health = 600 + g_iDifficulty * 400;
			pPawn->m_iHealth = health;

			pController->GetZEPlayer()->SetSpeedMod(1.0 + g_iDifficulty * 0.03);
		}

		if (V_strncmp(pBot->m_name, "[Boss] Exp203", 13) == 0)
		{
			pPawn->m_clrRender = Color(0, 255, 0, 255);
			pPawn->m_iHealth = 203;
			pPawn->m_iMaxHealth = 203;

			if (!bIsPistolRound)
				pPawn->m_pItemServices->GiveNamedItem("weapon_m249");
		}

		return -1.0f;
	});
}


bool vsBots_Detour_CBaseEntity_TakeDamageOld(CBaseEntity* pThis, CTakeDamageInfo* inputInfo)
{
	if (!pThis->IsPawn())
		return true;

	CCSPlayerPawn* pVictimPawn = (CCSPlayerPawn*)pThis;
	CCSPlayerPawn* pAttackerPawn = (CCSPlayerPawn*)inputInfo->m_hAttacker.Get();

	if (!(pAttackerPawn && pVictimPawn && pAttackerPawn->IsPawn() && pVictimPawn->IsPawn()))
		return true;

	CCSPlayerController* pVictimController = CCSPlayerController::FromPawn(pVictimPawn);
	CCSPlayerController* pAttackerController = CCSPlayerController::FromPawn(pAttackerPawn);
	if (!pVictimController || !pAttackerController)
		return true;

	if (pVictimPawn != pAttackerPawn)
	{
		CBaseEntity* pInflictor = inputInfo->m_hInflictor.Get();
		const char* pszInflictorClass = pInflictor ? pInflictor->GetClassname() : "";
		if (V_strncmp(pszInflictorClass, "player", 6) == 0)
		{
			CBasePlayerWeapon* pWeapon = pAttackerPawn->m_pWeaponServices->m_hActiveWeapon.Get();
			if (pWeapon && V_strncmp(pWeapon->GetClassname(), "weapon_knife", 12) == 0)
				inputInfo->m_flDamage = 120.0f;
		}
		if (V_stristr(pszInflictorClass, "_projectile") && !(inputInfo->m_bitsDamageType & DMG_BLAST))
			inputInfo->m_flDamage = 120.0f;
	}


	if (pVictimPawn->IsBot())
	{
		if (V_strcmp(pAttackerController->GetPlayerName(), "[Boss] Crusher") == 0 ||
			V_strcmp(pAttackerController->GetPlayerName(), "[Boss] Exp203") == 0)
		{
			if (inputInfo->m_bitsDamageType & DMG_BLAST)
				return false;
		}
	}

	if (pAttackerController->IsBot())
	{
		if (V_strcmp(pAttackerController->GetPlayerName(), "[Boss] Crusher") == 0)
			inputInfo->m_flDamage *= 1.0f + g_iDifficulty;

		if (V_strcmp(pAttackerController->GetPlayerName(), "[Boss] Stone") == 0)
			inputInfo->m_flDamage = 9999.0f;
	}

	return true;
}

void vsBots_OnPlayerHurt(IGameEvent* pEvent)
{
	CCSPlayerController* pAttacker = (CCSPlayerController*)pEvent->GetPlayerController("attacker");
	CCSPlayerController* pVictim = (CCSPlayerController*)pEvent->GetPlayerController("userid");

	if (!pAttacker || !pVictim)
		return;

	if (pVictim->m_iTeamNum == g_humanTeam)
	{
		ZEPlayer* pVictimPlayer = pVictim->GetZEPlayer();
		CBaseModelEntity* pGlowModelEnt = pVictimPlayer->GetGlowModel();
		CCSPlayerPawn* pVictimPawn = pVictim->GetPlayerPawn();
		if (pGlowModelEnt && pVictimPawn)
		{
			float ratio = pVictimPawn->m_iHealth / (float)pVictimPawn->m_iMaxHealth;

			int green = (int)(255 * ratio);
			int red = 255 - green;

			pGlowModelEnt->m_Glow().m_glowColorOverride = Color(red, green, 0, 255);
		}
	}

	if (pVictim->IsBot())
	{
		if (V_strncmp(pVictim->GetPlayerName(), "[Boss] Crusher", 14) == 0)
		{
			float regenTime = 0.5f;

			CHandle<CCSPlayerController> victimHandle = pVictim->GetHandle();
			new CTimer(regenTime, false, false, [victimHandle]()
			{
				CCSPlayerController* pVictim = (CCSPlayerController*)victimHandle.Get();
				if (!pVictim || !pVictim->IsAlive())
					return -1.0f;

				CCSPlayerPawn* pVictimPawn = pVictim->GetPlayerPawn();
				if (!(pVictimPawn && pVictimPawn->IsPawn()))
					return -1.0f;

				pVictimPawn->m_iHealth = pVictimPawn->m_iMaxHealth;

				return -1.0f;
			});
		}
	}
}

void vsBots_OnPlayerDeath(IGameEvent* pEvent)
{
	CCSPlayerController* pAttacker = (CCSPlayerController*)pEvent->GetPlayerController("attacker");
	CCSPlayerController* pVictim = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	CCSPlayerController* pAssister = (CCSPlayerController*)pEvent->GetPlayerController("assister");

	if (!pAttacker || !pVictim)
		return;

	if (pVictim == pAttacker)
		return;
	
	CCSPlayerPawn* pVictimPawn = pVictim->GetPlayerPawn();
	if (!pVictimPawn)
		return;

	if (pAssister && !pAssister->IsBot() && pAssister->IsConnected())
	{
		DBInfo info = pAssister->GetZEPlayer()->GetDBInfo();
		info.iAssists += 1;

		if (pVictim->IsBot() && V_strncmp(pVictim->GetPlayerName(), "[Boss]", 6) == 0)
			info.iBossAssists += 1;

		pAssister->GetZEPlayer()->SetDBInfo(info);
	}

	if (pAttacker->IsBot())
	{
		if (V_strcmp(pAttacker->GetPlayerName(), "[Boss] Exp203") == 0)
		{
			pAttacker->GetPlayerPawn()->m_iHealth = pAttacker->GetPlayerPawn()->m_iMaxHealth;
		}
	}

	if (pVictim->IsBot())
	{
		if (!pAttacker->IsBot() && !g_pGameRules->m_bWarmupPeriod)
		{
			DBInfo info = pAttacker->GetZEPlayer()->GetDBInfo();
			info.iKills += 1;

			if (V_strncmp(pVictim->GetPlayerName(), "[Boss]", 6) == 0)
				info.iBossKills += 1;

			pAttacker->GetZEPlayer()->SetDBInfo(info);
		}

		if (V_strncmp(pVictim->GetPlayerName(), "[Boss]", 6) == 0)
		{
			CPrintChatToAll(g_pKVPrintText->GetString("Message_BossKill", "BossKill %s %s"), 
				pAttacker->GetPlayerName(), pVictim->GetPlayerName());
			EmitSoundToAll("UI.ArmsRace.BecomeTeamLeader");

			if (V_strcmp(pVictim->GetPlayerName(), "[Boss] Exp203") == 0)
			{
				auto origin = pVictimPawn->GetAbsOrigin();

				// Todo : solve this to single timer
				CPrintChatToAll(g_pKVPrintText->GetString("Message_Exp203_Explode_3secs", "Exp203_Explode_3Secs"));
				pVictimPawn->EmitSound("tr.Countdown");

				CHandle<CCSPlayerPawn> victimHandle = pVictimPawn->GetHandle();
				new CTimer(1.0f, false, false, []()
				{
					CPrintChatToAll(g_pKVPrintText->GetString("Message_Exp203_Explode_2secs", "Exp203_Explode_2Secs"));
					return -1.0f;
				});
				new CTimer(2.0f, false, false, []()
				{
					CPrintChatToAll(g_pKVPrintText->GetString("Message_Exp203_Explode_1sec", "Exp203_Explode_1Sec"));
					return -1.0f;
				});
				new CTimer(3.0f, false, false, [victimHandle, origin]()
				{
					CCSPlayerPawn* pVictimPawn = (CCSPlayerPawn*)victimHandle.Get();
					if (!pVictimPawn)
						return -1.0f;

					CEnvExplosion* pExplosion = CreateEntityByName<CEnvExplosion>("env_explosion");

					CEntityKeyValues* pKeyValues = new CEntityKeyValues();

					pKeyValues->SetInt("iMagnitude", 9999);
					pKeyValues->SetInt("iRadiusOverride", 9999);

					pExplosion->m_iTeamNum = pVictimPawn->m_iTeamNum;
					pExplosion->m_hOwnerEntity.Set(pVictimPawn);

					Vector explosionOrigin = origin;
					explosionOrigin.z += 30.0f;
					pExplosion->Teleport(&explosionOrigin, nullptr, nullptr);
					pExplosion->DispatchSpawn(pKeyValues);
					
					pExplosion->EmitSound("tr.C4Explode");
					pExplosion->AcceptInput("Explode");

					{
						CParticleSystem* pEffect = CreateEntityByName<CParticleSystem>("info_particle_system");

						Vector vecOrigin = pVictimPawn->GetAbsOrigin();
						vecOrigin.z += 10;

						CEntityKeyValues* pKeyValues = new CEntityKeyValues();

						pKeyValues->SetString("effect_name", "particles/explosions_fx/explosion_c4_500.vpcf");
						pKeyValues->SetVector("origin", vecOrigin);
						pKeyValues->SetBool("start_active", true);

						pEffect->DispatchSpawn(pKeyValues);

						float duration = 5.0;
						UTIL_AddEntityIOEvent(pEffect, "DestroyImmediately", nullptr, nullptr, "", duration);
						UTIL_AddEntityIOEvent(pEffect, "Kill", nullptr, nullptr, "", duration + 0.02f);
					}

					return -1.0f;
				});
			}
		}
	}
}

void vsBots_OnPlayerBlind(IGameEvent* pEvent)
{
	CCSPlayerController* pVictim = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	CCSPlayerController* pAttacker = (CCSPlayerController*)pEvent->GetPlayerController("attacker");
	if (!pVictim || !pAttacker || pVictim == pAttacker)
		return;

	CCSPlayerPawn* pVictimPawn = (CCSPlayerPawn*)pVictim->GetPlayerPawn();
	if (!pVictimPawn)
		return;

	pVictimPawn->m_flFlashDuration = 0.0;
	pVictimPawn->m_flFlashMaxAlpha = 0.0;
}

void vsBots_OnWeaponFire(IGameEvent* pEvent)
{
	CCSPlayerController* pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController)
		return;

	if (pController->IsBot() && 
		(V_strncmp(pController->GetPlayerName(), "[Boss] Crusher", 14) == 0 || V_strncmp(pController->GetPlayerName(), "[Boss] Exp203", 13) == 0))
	{
		auto pPawn = pController->GetPawn();
		if (!pPawn)
			return;

		CCSPlayer_WeaponServices* pWeaponServices = pPawn->m_pWeaponServices;
		if (!pWeaponServices)
			return;

		CUtlVector<CHandle<CBasePlayerWeapon>>* weapons = pWeaponServices->m_hMyWeapons();

		FOR_EACH_VEC(*weapons, i)
		{
			CBasePlayerWeapon* weapon = (*weapons)[i].Get();

			if (!weapon)
				continue;

			if (weapon->GetWeaponVData()->m_GearSlot() == GEAR_SLOT_RIFLE || weapon->GetWeaponVData()->m_GearSlot() == GEAR_SLOT_PISTOL)
				weapon->AcceptInput("SetClipPrimary", "999");
		}
	}
}

void vsBots_BulletImpact(IGameEvent* pEvent)
{
	CCSPlayerPawn* pPawn = (CCSPlayerPawn*)pEvent->GetPlayerPawn("userid");
	if (!pPawn)
		return;
	
	CCSPlayerController* pController = pPawn->GetOriginalController();
	if (!pController)
		return;

	Color clTint = Color(0, 0, 0, 0);
	if (pController->IsBot())
	{
		if (V_strncmp(pController->GetPlayerName(), "[Boss] Crusher", 14) == 0)
			clTint = Color(255, 0, 0, 255);
		if (V_strncmp(pController->GetPlayerName(), "[Boss] Exp203", 14) == 0)
			clTint = Color(0, 255, 0, 255);
	}
	else
	{
		ZEPlayer *pPlayer = pController->GetZEPlayer();
		if (pPlayer->GetAdminFlags() & ADMFLAG_GENERIC)
		{
			Color colors[] = { Color(232, 20, 22, 255), Color(255, 165, 0, 255), Color(255, 235, 54, 255), Color(121, 195, 20, 255), Color(72, 125, 231, 255), Color(75, 54, 157, 255), Color(112, 54, 157, 255) };
			clTint = colors[rand() % sizeof(colors) / sizeof(Color)];
		}
	}

	if (clTint.a() == 0)
		return;

	CBasePlayerWeapon* pWeapon = pPawn->m_pWeaponServices->m_hActiveWeapon.Get();

	CParticleSystem* particle = CreateEntityByName<CParticleSystem>("info_particle_system");

	// Teleport particle to muzzle_flash attachment of player's weapon
	particle->AcceptInput("SetParent", "!activator", pWeapon, nullptr);
	particle->AcceptInput("SetParentAttachment", "muzzle_flash");

	CEntityKeyValues* pKeyValues = new CEntityKeyValues();

	// Event contains other end of the particle
	Vector vecData = Vector(pEvent->GetFloat("x"), pEvent->GetFloat("y"), pEvent->GetFloat("z"));

	pKeyValues->SetString("effect_name", "particles/cs2fixes/leader_tracer.vpcf");
	pKeyValues->SetInt("data_cp", 1);
	pKeyValues->SetVector("data_cp_value", vecData);
	pKeyValues->SetInt("tint_cp", 2);
	pKeyValues->SetColor("tint_cp_color", clTint);
	pKeyValues->SetBool("start_active", true);

	particle->DispatchSpawn(pKeyValues);

	UTIL_AddEntityIOEvent(particle, "DestroyImmediately", nullptr, nullptr, "", 0.1f);
	UTIL_AddEntityIOEvent(particle, "Kill", nullptr, nullptr, "", 0.12f);
}

bool vsBots_Detour_CCSPlayer_WeaponServices_CanUse(CCSPlayer_WeaponServices* pWeaponServices, CBasePlayerWeapon* pPlayerWeapon)
{
	CCSPlayerPawn* pPawn = pWeaponServices->__m_pChainEntity();
	if (!pPawn)
		return false;

	CCSPlayerController* pController = CCSPlayerController::FromPawn(pPawn);
	if (!pController)
		return false;

	if (pController->IsBot())
	{
		const char* pszWeaponClassname = pPlayerWeapon->GetClassname();
		if (V_strcmp(pController->GetPlayerName(), "[Boss] Crusher") == 0 && g_bCrusherHasShotgun)
		{
			if (V_strncmp(pszWeaponClassname, "weapon_xm1014", 13) != 0 && V_strncmp(pszWeaponClassname, "weapon_knife", 12) != 0)
				return false;
		}
		if (V_strcmp(pController->GetPlayerName(), "[Boss] Stone") == 0)
		{
			if (V_strncmp(pszWeaponClassname, "weapon_knife", 12) != 0)
				return false;
		}

		if (V_strncmp(pszWeaponClassname, "weapon_hegrenade", 16) == 0 ||
			V_strncmp(pszWeaponClassname, "weapon_flashbang", 16) == 0 ||
			V_strncmp(pszWeaponClassname, "weapon_decoy", 12) == 0)
			return false;
	}

	return true;
}

void vsBots_Detour_ProcessMovement(CCSPlayer_MovementServices* pThis)
{
	CCSPlayerPawn* pPawn = pThis->GetPawn();
	CCSPlayerController* pController = pPawn->GetOriginalController();

	if (pPawn->IsBot())
	{
		// Prevent bot walking
		pThis->m_nButtons().m_pButtonStates[0] &= ~IN_SPEED;
		if (V_strncmp(pController->GetPlayerName(), "[Boss] Stone", 12) == 0)
			pThis->m_flStamina = 0.0;

		CCSBot* pBot = pPawn->m_pBot;
		if (!pBot)
			return;

		if (V_strncmp(pController->GetPlayerName(), "[Boss] Crusher", 14) == 0)
		{
			CCSPlayer_WeaponServices* pWeaponServices = pPawn->m_pWeaponServices;
			if (!pWeaponServices)
				return;

			CBasePlayerWeapon* pBaseWeapon = pWeaponServices->m_hActiveWeapon.Get();
			CCSWeaponBase* pWeapon = (CCSWeaponBase*)pBaseWeapon;
			if (!pWeapon)
				return;

			pBot->m_fireWeaponTimestamp().SetTime(0.0f);

			pPawn->m_iShotsFired = 0;
			if (g_iDifficulty >= 9)
			{
				pWeaponServices->m_flNextAttack().SetTime(0.0);
				pWeapon->m_nNextPrimaryAttackTick = 1;
				pWeapon->m_fAccuracyPenalty = 0.0;
				pPawn->m_aimPunchAngle = QAngle(0, 0, 0);
				pPawn->m_aimPunchAngleVel = QAngle(0, 0, 0);
			}
		}
		if (V_strncmp(pController->GetPlayerName(), "[Boss] Exp203", 13) == 0)
		{
			CCSPlayer_WeaponServices* pWeaponServices = pPawn->m_pWeaponServices;
			if (!pWeaponServices)
				return;

			CBasePlayerWeapon* pBaseWeapon = pWeaponServices->m_hActiveWeapon.Get();
			CCSWeaponBase* pWeapon = (CCSWeaponBase*)pBaseWeapon;
			if (!pWeapon)
				return;

			pPawn->m_iShotsFired = 0;
			float notSeenEnemyTime = gpGlobals->curtime - pBot->m_lastSawEnemyTimestamp;
			if (notSeenEnemyTime <= 3.0f &&
				gpGlobals->curtime >= pWeaponServices->m_flNextAttack().GetTime())
				pThis->m_nButtons().m_pButtonStates[0] |= IN_ATTACK;

			if (g_iDifficulty >= 10)
			{
				pWeapon->m_fAccuracyPenalty = 0.0;
				pPawn->m_aimPunchAngle = QAngle(0, 0, 0);
				pPawn->m_aimPunchAngleVel = QAngle(0, 0, 0);
			}
		}
	}
}

class CUserCmd
{
public:
	[[maybe_unused]] char pad0[0x10];
	CSGOUserCmdPB cmd;
	[[maybe_unused]] char pad1[0x38];
#ifdef PLATFORM_WINDOWS
	[[maybe_unused]] char pad2[0x8];
#endif
};

void vsBots_Detour_ProcessUsercmds(CCSPlayerController* pController, CUserCmd* cmds, int numcmds, bool paused, float margin)
{
	CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
	if (pPawn->IsBot())
	{
		QAngle viewAngles = pPawn->m_angEyeAngles();

		float useYaw = pPawn->m_pBot->m_lookYaw;
		float angleDiff = fabsf(useYaw - viewAngles.y);

		const float onTargetTolerance = 1.0f;
		if (angleDiff < onTargetTolerance)
		{
			for (int i = 0; i < numcmds; i++)
			{
				pPawn->m_pBot->m_lookYawVel = 0.0f;
				CMsgQAngle* pMsgQAngle = cmds[i].cmd.mutable_base()->mutable_viewangles();

				pMsgQAngle->set_y(pPawn->m_pBot->m_lookYaw);
			}
		}
	}
}

void vsBots_OnEntitySpawned(CEntityInstance* pEntity)
{
	if (V_strncmp(pEntity->GetClassname(), "weapon_", 7) != 0)
		return;

	CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pEntity;
	if (!pWeapon)
		return;

	CCSWeaponBaseVData* pVData = pWeapon->GetWeaponVData();
	if (!pVData)
		return;

	struct WeaponVDataMap_t
	{
		int itemDefIndex;
		int maxClip1;
		int maxAmmo;
		int killAward;
	};

	// Todo : move to config
	WeaponVDataMap_t WeaponMap[] = {
		{ITEMDEFINDEX_DEAGLE, -1, -1, 300},
		{ITEMDEFINDEX_CZ75A, -1, 60, 300},
		{ITEMDEFINDEX_REVOLVER, -1, 32, 450},
		{ITEMDEFINDEX_USP_SILENCER, -1, 48, -1},
		{ITEMDEFINDEX_P250, -1, 52, -1},
		{ITEMDEFINDEX_FAMAS, 35, 105, -1},
		{ITEMDEFINDEX_M4A1, 45, 135, 450},
		{ITEMDEFINDEX_AUG, 45, 135, 450},
		{ITEMDEFINDEX_M4A1_SILENCER, 40, 120, 450},
		{ITEMDEFINDEX_SSG08, -1, -1, 600},
		{ITEMDEFINDEX_AWP, -1, -1, 0},
		{ITEMDEFINDEX_TASER, -1, -1, 1500},
		{ITEMDEFINDEX_MAC10, 40, 120, 600},
		{ITEMDEFINDEX_MP9, 40, 120, 600},
		{ITEMDEFINDEX_UMP45, 35, 105, 600},
		{ITEMDEFINDEX_MP7, 45, 135, 600},
		{ITEMDEFINDEX_MP5SD, 45, 135, 600},
		{ITEMDEFINDEX_P90, 80, 160, 600},
		{ITEMDEFINDEX_BIZON, 100, 200, 900},
		{ITEMDEFINDEX_M249, 150, 300, -1},
		{ITEMDEFINDEX_SCAR20, 12, 80, 0},
		{ITEMDEFINDEX_G3SG1, 12, 80, 0},
		{ITEMDEFINDEX_XM1014, 12, 40, -1},
	};

	int weaponID = pWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex;
	for (int i = 0; i < sizeof(WeaponMap) / sizeof(*WeaponMap); i++)
	{
		if (weaponID != WeaponMap[i].itemDefIndex)
			continue;

		if (WeaponMap[i].maxClip1 != -1)
			pVData->m_iMaxClip1 = WeaponMap[i].maxClip1;
		if (WeaponMap[i].maxAmmo != -1)
			pVData->m_nPrimaryReserveAmmoMax = WeaponMap[i].maxAmmo;
		if (WeaponMap[i].killAward != -1)
			pVData->m_nKillAward = WeaponMap[i].killAward;
	}
}

int GetPlayerRating(CCSPlayerController* pController)
{
	DBInfo info = pController->GetZEPlayer()->GetDBInfo();
	if (!info.bDataLoaded)
		return 0;

	return info.iKills + info.iBossKills * 10 + info.iWinPoint + info.iAssists / 2 + info.iBossAssists * 5;
}

void vsBots_OnTick()
{
	if (!g_bScoreboardRankUpdate)
		return;
	
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || !pController->IsConnected())
			continue;

		pController->m_szClan = CUtlSymbolLarge(" ");
		if (pController->IsBot())
		{
			CCSBot* pBot = pController->GetPlayerPawn()->m_pBot;
			if (!pBot)
				continue;

			pController->m_iCompetitiveWins = 2;
			if (V_strncmp(pController->GetPlayerName(), "[  *  ]", 7) == 0)
			{
				BotProfile* pBotProfile = pBot->GetLocalProfile();
				pController->m_iCompetitiveRankType = 11;

				int skillScore = (int)(15000.0f * pBotProfile->m_skill);
				int attackDelayScore = (int)(15000.0f * (1.0 - (pBotProfile->m_attackDelay / 3.0)));

				pController->m_iCompetitiveRanking = skillScore + attackDelayScore;
			}
			else if (V_strncmp(pController->GetPlayerName(), "[Human]", 7) != 0)
			{
				pController->m_iCompetitiveRankType = 12;

				int rank = 18;
				if (V_strcmp(pController->GetPlayerName(), "[Boss] Crusher") == 0)
					rank = g_iDifficulty >= 9 ? 18 : 17;
				if (V_strcmp(pController->GetPlayerName(), "[Boss] Stone") == 0)
					rank = g_iDifficulty >= 11 ? 17 : 16;
				if (V_strcmp(pController->GetPlayerName(), "[Boss] Exp203") == 0)
					rank = g_iDifficulty >= 10 ? 17 : 16;

				pController->m_iCompetitiveRanking = rank;
			}
		}
		else
		{
			DBInfo info = pController->GetZEPlayer()->GetDBInfo();
			int rating = GetPlayerRating(pController);
			pController->m_iCompetitiveRankType = 11;
			pController->m_iCompetitiveRanking = rating;
		}
	}
}

void VSBots::OnDBConnected()
{
	ZDatabase::GetConnection()->Query(mysql_players_create, [](ISQLQuery* test) {});
}

void VSBots::OnAuthenticated(ZEPlayer* pPlayer)
{
	char query[1024];
	V_snprintf(query, sizeof(query), mysql_players_select, pPlayer->GetSteamId64());

	if (ZDatabase::GetConnection())
	{
		ZEPlayerHandle handle = pPlayer->GetHandle();
		ZDatabase::GetConnection()->Query(query, [handle](ISQLQuery* query)
		{
			ZEPlayer* pPlayer = handle.Get();
			if (!pPlayer)
				return;

			DBInfo info;

			ISQLResult* results = query->GetResultSet();
			if (results->FetchRow())
			{
				info.iKills = results->GetInt(0);
				info.iBossKills = results->GetInt(1);
				info.iPoint = results->GetInt(2);
				info.iWinPoint = results->GetInt(3);
				info.iAssists = results->GetInt(4);
				info.iBossAssists = results->GetInt(5);
				info.pszSkinName = results->GetString(6);
			}
			else
			{
				info.iKills = 0;
				info.iBossKills = 0;
				info.iPoint = 0;
				info.iWinPoint = 0;
				info.iAssists = 0;
				info.iBossAssists = 0;
				info.pszSkinName = "";
			}

			info.bDataLoaded = true;

			pPlayer->SetDBInfo(info);
		});
	}
}

void VSBots::OnClientDisconnect(CPlayerSlot slot)
{
	SaveDB();
}

bool vsBots_OnSayText(CCSPlayerController* pAuthor, const char* pText)
{
	if (pText[0] == '\0')
		return true;

	DBInfo info = pAuthor->GetZEPlayer()->GetDBInfo();
	int level = 1 + GetPlayerRating(pAuthor) / 100;

	const char* pColorTag = pAuthor->m_iTeamNum == CS_TEAM_T ? "\x09" : "\x0C";

	ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Lv.%d]%s %s :\x01 %s", level, pColorTag, pAuthor->GetPlayerName(), pText);

	vsBots_OnSayTextPost(pAuthor, pText);

	return true;
}

void vsBots_OnSayTextPost(CCSPlayerController* pController, const char* pText)
{
	if (V_stricmp(pText, "!rank") == 0 && ZDatabase::GetConnection())
	{
		ZEPlayer *pPlayer = pController->GetZEPlayer();
		DBInfo dbInfo = pPlayer->GetDBInfo();
		if (!dbInfo.bDataLoaded)
			return;

		char query[1024];
		V_snprintf(query, sizeof(query), mysql_players_rank,
			GetPlayerRating(pController), pPlayer->GetSteamId64(), pPlayer->GetSteamId64());

		CHandle<CBasePlayerController> hController = pController->GetHandle();
		ZDatabase::GetConnection()->Query(query, [hController, dbInfo](ISQLQuery* query)
		{
			CCSPlayerController* pController = (CCSPlayerController*)hController.Get();
			if (!pController)
				return;

			auto results = query->GetResultSet();
			if (results->FetchRow())
			{
				int playerCnt = results->GetInt(0);
				int rank = 1 + results->GetInt(1);

				ClientPrintAll(HUD_PRINTTALK, " \x04[Rank - %s]\x01 %d킬 %d보스킬 %d승점포인트 %d어시 %d보스어시", pController->GetPlayerName(), 
					dbInfo.iKills, dbInfo.iBossKills, dbInfo.iWinPoint, dbInfo.iAssists, dbInfo.iBossAssists);
				ClientPrintAll(HUD_PRINTTALK, " \x04[Rank - %s]\x01 랭킹 %d/%d",
					pController->GetPlayerName(), rank, playerCnt);
			}
		});
	}
}

CON_COMMAND_CHAT_FLAGS(savedb, "Save DB", ADMFLAG_GENERIC)
{
	VSBots::SaveDB();
}

void VSBots::SaveDB()
{
	if (!ZDatabase::GetConnection())
		return;

	char query[1024];

	Transaction txn;

	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || pController->IsBot())
			continue;

		ZEPlayer* pPlayer = pController->GetZEPlayer();
		if (!pPlayer || !pPlayer->IsAuthenticated())
			continue;

		DBInfo dbInfo = pPlayer->GetDBInfo();
		if (!dbInfo.bDataLoaded)
			continue;
		
		// Don't save default data, and hacky fix some players data reseted.
		if (dbInfo.iKills == 0 && dbInfo.iBossKills == 0)
			continue;

		std::string escapedName = ZDatabase::GetConnection()->Escape(pController->GetPlayerName());
		V_snprintf(query, sizeof(query), mysql_players_upsert,
			pPlayer->GetSteamId64(), escapedName.c_str(), dbInfo.iKills, dbInfo.iBossKills, dbInfo.iPoint, dbInfo.iWinPoint, dbInfo.iAssists, dbInfo.iBossAssists, dbInfo.pszSkinName.c_str());
		txn.queries.push_back(query);
	}

	if (txn.queries.size() == 0)
		return;

	ZDatabase::GetConnection()->ExecuteTransaction(txn, ZDatabase::OnGenericTxnSuccess, ZDatabase::OnGenericTxnFailure);
}

void EmitSoundToAll(const char* szPath, float volume)
{
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || !pController->IsConnected() || pController->IsBot())
			continue;

		CSingleRecipientFilter filter(pController->GetPlayerSlot());
		pController->EmitSoundFilter(filter, szPath, volume);
	}
}

int GetHumanCount()
{
	int playerCount = 0;
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || !pController->IsConnected() || pController->IsBot() || pController->m_iTeamNum() != g_humanTeam)
			continue;

		playerCount++;
	}

	return playerCount;
}

void SetHumanHealth()
{
	int playerCount = GetHumanCount();

	int health = MAX(200, 500 - 30 * (playerCount - 1));

	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || !pController->IsConnected() || pController->m_iTeamNum() != g_humanTeam)
			continue;

		CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
		if (!pPawn || !pPawn->IsAlive())
			continue;

		pPawn->m_iHealth = health;
		pPawn->m_iMaxHealth = health;
	}
}

void AddHumanBots()
{
	if (g_pGameRules->m_bWarmupPeriod)
		return;

	int playerCount = GetHumanCount();
	if (playerCount == 0)
		return;

	const int maxBotCount = 8;
	int botCount = maxBotCount - playerCount;
	for (int i = 1; i <= botCount; i++)
	{
		char szBotAddCmd[128];
		V_snprintf(szBotAddCmd, sizeof(szBotAddCmd), "bot_add_%s \"[Human] PLAYER%d\"", g_humanTeam == 2 ? "t" : "ct", i);
		g_pEngineServer2->ServerCommand(szBotAddCmd);
	}

	for (int i = botCount + 1; i <= maxBotCount; i++)
	{
		char szBotAddCmd[128];
		V_snprintf(szBotAddCmd, sizeof(szBotAddCmd), "bot_kick \"[Human] PLAYER%d\"", i);
		g_pEngineServer2->ServerCommand(szBotAddCmd);
	}
}