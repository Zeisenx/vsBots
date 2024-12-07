
/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023-2024 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "igameevents.h"
#include "entity/ccsplayercontroller.h"
#include "entity/ccsplayerpawn.h"

constexpr char mysql_players_create[] = R"(
    CREATE TABLE IF NOT EXISTS players ( 
        SteamID64 BIGINT UNSIGNED NOT NULL, 
        Name TEXT,
        Kills INTEGER NOT NULL DEFAULT 0, 
        BossKills INTEGER NOT NULL DEFAULT 0, 
        Point INTEGER NOT NULL DEFAULT 0, 
        WinPoint INTEGER NOT NULL DEFAULT 0, 
        Assists INTEGER NOT NULL DEFAULT 0, 
        BossAssists INTEGER NOT NULL DEFAULT 0, 
        Skin VARCHAR(32) NOT NULL DEFAULT '', 
        Created TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, 
        PRIMARY KEY(SteamID64)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
)";

constexpr char mysql_players_select[] = R"(
    SELECT Kills, BossKills, Point, WinPoint, Assists, BossAssists, Skin
        FROM players
    WHERE SteamID64 = %lld
)";

constexpr char mysql_players_upsert[] = R"(
    INSERT INTO players (SteamID64, Name, Kills, BossKills, Point, WinPoint, Assists, BossAssists, Skin)
            VALUES (%lld, '%s', %d, %d, %d, %d, %d, %d, '%s')
            ON DUPLICATE KEY UPDATE
            SteamID64=VALUES(SteamID64), Name=VALUES(Name),
            Kills=VALUES(Kills), BossKills=VALUES(BossKills), Point=VALUES(Point), WinPoint=VALUES(WinPoint),
            Assists=VALUES(Assists), BossAssists=VALUES(BossAssists), Skin=VALUES(Skin)
)";

constexpr char mysql_players_rank[] = R"(
    SELECT (SELECT COUNT(SteamID64) FROM players), 
    (SELECT COUNT(SteamID64) FROM players WHERE Kills + BossKills * 10 + WinPoint + (Assists / 2) + BossAssists * 5>%d AND SteamID64 != %lld) 
    FROM players WHERE SteamID64=%lld
)";

class VSBots
{
public:
    static void OnDBConnected();
    static void OnAuthenticated(ZEPlayer* pPlayer);
    static void OnClientDisconnect(CPlayerSlot slot);
    static void SaveDB();
};

void vsBots_OnLevelInit(char const* pMapName);
void vsBots_Precache(IEntityResourceManifest* pResourceManifest);
void vsBots_OnRoundStart(IGameEvent* pEvent);
void vsBots_OnRoundFreezeEnd(IGameEvent* pEvent);
void vsBots_OnRoundEnd(IGameEvent* pEvent);
void vsBots_OnPlayerSpawn(CCSPlayerController *pController);
void vsBots_OnPlayerHurt(IGameEvent* pEvent);
void vsBots_OnPlayerDeath(IGameEvent* pEvent);
void vsBots_OnPlayerBlind(IGameEvent* pEvent);
void vsBots_OnWeaponFire(IGameEvent* pEvent);
void vsBots_BulletImpact(IGameEvent* pEvent);
bool vsBots_Detour_CBaseEntity_TakeDamageOld(CBaseEntity* pThis, CTakeDamageInfo* inputInfo);
bool vsBots_Detour_CCSPlayer_WeaponServices_CanUse(CCSPlayer_WeaponServices* pWeaponServices, CBasePlayerWeapon* pPlayerWeapon);
void vsBots_Detour_ProcessMovement(CCSPlayer_MovementServices* pThis);
void vsBots_Detour_ProcessUsercmds(CCSPlayerController* pController, CUserCmd* cmds, int numcmds, bool paused, float margin);
void vsBots_Detour_BotProfileManager_InitPost(BotProfileManager* botProfileManager, const char* filename, unsigned int* checksum);
void vsBots_OnEntitySpawned(CEntityInstance* pEntity);
bool vsBots_OnSayText(CCSPlayerController* pAuthor, const char* pText);
void vsBots_LoadBotNames();
void vsBots_OnGameEnd();
void vsBots_OnTick();
