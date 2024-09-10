
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
void vsBots_LoadBotNames();