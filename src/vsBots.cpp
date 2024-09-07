
#pragma once

#include "commands.h"
#include "eventlistener.h"
#include "vsBots.h"
#include "ctimer.h"
#include "utils/entity.h"
#include "entity/cgamerules.h"
#include <fstream>
#include <regex>
#include <entity/cenvexplosion.h>

extern IVEngineServer2* g_pEngineServer2;
extern CCSGameRules* g_pGameRules;
extern CGlobalVars* gpGlobals;

#define BOSSMODEL_DEFAULT "characters/models/tm_phoenix_heavy/tm_phoenix_heavy.vmdl"

#define ITEMDEFINDEX_G3SG1 11
#define ITEMDEFINDEX_NEGEV 28
#define ITEMDEFINDEX_SCAR20 38
#define ITEMDEFINDEX_SMOKEGRENADE 45
#define ITEMDEFINDEX_MOLOTOV 46
#define ITEMDEFINDEX_INCGRENADE 48

const int DIFFICULTY_MIN = 0;
const int DIFFICULTY_MAX = 12;

bool g_bCrusherHasShotgun = false;
int g_difficulty = 0;
int g_humanTeam = CS_TEAM_CT;
int g_botTeam = CS_TEAM_T;
bool g_bForceSwitch;
FAKE_INT_CVAR(vsbots_forceswitch, "Force team switch", g_bForceSwitch, false, false)
FAKE_INT_CVAR(vsbots_difficulty, "Bot Difficulty", g_difficulty, false, false)
FAKE_INT_CVAR(vsbots_humanteam, "Human Team", g_humanTeam, false, false)
FAKE_INT_CVAR(vsbots_botteam, "Bot Team", g_botTeam, false, false)
std::vector<std::string> g_vecBotNamesList;
KeyValues* g_pKVPrintText;

void LoadPrintTextKV();
void DuplicateSpawnPoint();
void AddHumanBots();

void vsBots_OnLevelInit(char const* pMapName)
{
	LoadPrintTextKV();

	char cmdFmt[128];

	bool isHumanTSide = strncmp(pMapName, "de_", 3) == 0;
	if (rand() % 5 != 0 || g_bForceSwitch)
		isHumanTSide = !isHumanTSide;

	g_bForceSwitch = false;
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

	vsBots_LoadBotNames();
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

void DuplicateSpawnPoint()
{
	size_t botsCount = g_vecBotNamesList.size();

	CUtlVector<SpawnPoint*>* botTeamSpawns = g_botTeam == CS_TEAM_T ? g_pGameRules->m_TerroristSpawnPoints() : g_pGameRules->m_CTSpawnPoints();

	size_t addCount = botsCount - botTeamSpawns->Count();
	Message("DuplicateSpawnPoint() : addCount : %d\n", addCount);
	if (addCount <= 0)
		return;

	for (int i = 1; i <= addCount; i++)
	{
		SpawnPoint* spawnEntity = CreateEntityByName<SpawnPoint>(g_botTeam == CS_TEAM_T ? "info_player_terrorist" : "info_player_counterterrorist");

		CEntityKeyValues* pKeyValues = new CEntityKeyValues();

		spawnEntity->SetAbsOrigin((*botTeamSpawns)[i % botTeamSpawns->Count()]->GetAbsOrigin());
		spawnEntity->SetAbsRotation((*botTeamSpawns)[i % botTeamSpawns->Count()]->GetAbsRotation());
		spawnEntity->DispatchSpawn();
	}
}

void vsBots_LoadBotNames()
{
	Message("vsBots_LoadBotNames()\n");
	g_vecBotNamesList.clear();
	const char* pszFilePath = "addons/cs2fixes/configs/vsBots/botnames.txt";
	char szPath[MAX_PATH];
	V_snprintf(szPath, sizeof(szPath), "%s%s%s", Plat_GetGameDirectory(), "/csgo/", pszFilePath);
	std::ifstream infile(szPath);
	std::string botName;
	while (std::getline(infile, botName))
	{
		g_vecBotNamesList.push_back(botName);
	}
}


void vsBots_Precache(IEntityResourceManifest* pResourceManifest)
{
	pResourceManifest->AddResource(BOSSMODEL_DEFAULT);
}

void vsBots_OnRoundStart(IGameEvent* pEvent)
{
	g_pEngineServer2->ServerCommand("bot_difficulty 3");
	g_pEngineServer2->ServerCommand("mp_flinch_punch_scale 0.0");

	DuplicateSpawnPoint();
	for (auto& name : g_vecBotNamesList)
	{
		char szBotAddCmd[128];
		V_snprintf(szBotAddCmd, sizeof(szBotAddCmd), "bot_add \"%s\"", name.c_str());
		g_pEngineServer2->ServerCommand(szBotAddCmd);
	}

	AddHumanBots();
}

void vsBots_OnRoundFreezeEnd(IGameEvent* pEvent)
{
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Zeisen Project Discord]\x01 https://discord.gg/tDZUnpaumD");
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Bot Level]\x01 %d", g_difficulty);
	CPrintChatToAll(g_pKVPrintText->GetString("Message_WeaponRestrict", "Weapon Restrict"));
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

	ClientPrintAll(HUD_PRINTTALK, "\x01 \x02[Level]\x01 %d -> %d", oldLevel, g_difficulty);
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
			int health = MAX(100, g_difficulty == 0 ? 999 : (500 - 50 * (g_difficulty - 1)));
			pPawn->m_iHealth = health;
			pPawn->m_iMaxHealth = health;
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

		const bool bIsPistolRound = pController->m_pInGameMoneyServices->m_iAccount <= 1000;

		if (!bIsPistolRound)
			pController->m_pInGameMoneyServices->m_iAccount = 16000;
		
		pBot->m_hasVisitedEnemySpawn = true; // it makes bot doesn't rush to enemy spawn
		if (V_strncmp(pBot->m_name, "[Boss]", 6) == 0)
		{
			pPawn->SetModel(BOSSMODEL_DEFAULT);
			pPawn->m_ArmorValue = 9999;
			if (!bIsPistolRound)
				pPawn->m_pItemServices->m_bHasHelmet = true;

			pController->m_iScore = 50;
			pController->m_pInGameMoneyServices->m_iAccount = 0;
		}

		if (V_strncmp(pBot->m_name, "[Boss] Crusher", 14) == 0)
		{
			if (!bIsPistolRound)
			{
				pPawn->m_pItemServices->GiveNamedItem("weapon_xm1014");
				g_bCrusherHasShotgun = true;
			}
			else
				g_bCrusherHasShotgun = false;

			pController->m_iScore = 100;

			pPawn->m_clrRender = Color(255, 0, 0, 255);
		}

		if (V_strncmp(pBot->m_name, "[Boss] Stone", 12) == 0)
		{
			pPawn->m_clrRender = Color(0, 0, 0, 255);
			UTIL_AddEntityIOEvent(pPawn, "SetScale", nullptr, nullptr, 1.0 + 0.12 * (MAX(1, g_difficulty) - 1));

			int health = 600 + g_difficulty * 650;
			pPawn->m_iHealth = health;

			pController->GetZEPlayer()->SetSpeedMod(1.0 + g_difficulty * 0.03);
		}

		if (V_strncmp(pBot->m_name, "[Boss] Exp203", 13) == 0)
		{
			pPawn->m_clrRender = Color(0, 255, 0, 255);
			pPawn->m_iHealth = 203;

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
	}


	if (pAttackerController->IsBot())
	{
		if ((g_difficulty >= 1 && V_strncmp(pAttackerController->GetPlayerName(), "[Boss] Crusher", 14) == 0) ||
			V_strncmp(pAttackerController->GetPlayerName(), "[Boss] Stone", 12) == 0)
		{
			inputInfo->m_flDamage = 9999.0f;
		}
	}

	return true;
}

void vsBots_OnPlayerHurt(IGameEvent* pEvent)
{
	CCSPlayerController* pAttacker = (CCSPlayerController*)pEvent->GetPlayerController("attacker");
	CCSPlayerController* pVictim = (CCSPlayerController*)pEvent->GetPlayerController("userid");

	if (!pAttacker || !pVictim)
		return;

	if (pVictim->IsBot())
	{
		if (V_strncmp(pVictim->GetPlayerName(), "[Boss] Crusher", 14) == 0)
		{
			float regenTime = 0.2f;
			if (g_difficulty >= 4) regenTime = 0.15f;
			if (g_difficulty >= 8) regenTime = 0.1f;

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

	if (!pAttacker || !pVictim)
		return;

	if (pVictim == pAttacker)
		return;
	
	CCSPlayerPawn* pVictimPawn = pVictim->GetPlayerPawn();
	if (!pVictimPawn)
		return;

	if (pVictim->IsBot())
	{
		if (V_strncmp(pVictim->GetPlayerName(), "[Boss]", 6) == 0)
		{
			CPrintChatToAll(g_pKVPrintText->GetString("Message_BossKill", "BossKill %s %s"), 
				pAttacker->GetPlayerName(), pVictim->GetPlayerName());

			if (V_strncmp(pVictim->GetPlayerName(), "[Boss] Exp203", 14) == 0)
			{
				auto origin = pVictimPawn->GetAbsOrigin();

				// Todo : solve this to single timer
				CPrintChatToAll(g_pKVPrintText->GetString("Message_Exp203_Explode_3secs", "Exp203_Explode_3Secs"));

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
					
					pExplosion->AcceptInput("Explode");

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

	if (pController->IsBot() && V_strncmp(pController->GetPlayerName(), "[Boss] Crusher", 14) == 0)
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

		if (V_strncmp(pController->GetPlayerName(), "[Boss] Crusher", 14) == 0)
		{
			CCSBot* pBot = pPawn->m_pBot;
			if (!pBot)
				return;

			CCSPlayer_WeaponServices* pWeaponServices = pPawn->m_pWeaponServices;
			if (!pWeaponServices)
				return;

			CBasePlayerWeapon* pBaseWeapon = pWeaponServices->m_hActiveWeapon.Get();
			CCSWeaponBase* pWeapon = (CCSWeaponBase*)pBaseWeapon;
			if (!pWeapon)
				return;

			if (pBot->m_isEnemyVisible)
			{
				if (gpGlobals->curtime >= pWeaponServices->m_flNextAttack().m_Value)
					pThis->m_nButtons().m_pButtonStates[0] |= IN_ATTACK;
			}

			if (g_difficulty >= 9)
			{
				pWeaponServices->m_flNextAttack().m_Value = 0.0;
				pWeapon->m_nNextPrimaryAttackTick = 1;
				pWeapon->m_fAccuracyPenalty = 0.0;
				pPawn->m_iShotsFired = 0;
				pPawn->m_aimPunchAngle = QAngle(0, 0, 0);
				pPawn->m_aimPunchAngleVel = QAngle(0, 0, 0);
			}
		}
	}
}

void AddHumanBots()
{
	int playerCount = 0;
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || !pController->IsConnected() || pController->IsBot() || pController->m_iTeamNum() <= CS_TEAM_SPECTATOR)
			continue;

		playerCount++;
	}

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