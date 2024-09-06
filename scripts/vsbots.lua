
require('includes/timers')

print("Starting vsBots scripts");

g_isMapLoad = true
g_botTeam = 2
g_humanTeam = 3
g_botCount = 45
g_isBot = {}
g_botName = {}
g_playerPawn = {}

FCVAR_RELEASE = bit.lshift(1, 19)

Convars:RegisterConvar("server_booted", "1", "vsBots Level", FCVAR_RELEASE);
Convars:RegisterConvar("vsbots_level", "0", "vsBots Level", FCVAR_REPLICATED + FCVAR_RELEASE);

g_playerCnt = 0
g_expExplodeTimer = nil
clientcmd = nil
mapName = GetMapName()
tMap = mapName:find("de_") and RandomInt(1, 14) ~= 7
if tMap then
    g_humanTeam = 2
    g_botTeam = 3
    
else
    g_humanTeam = 3
    g_botTeam = 2
end

Convars:SetInt("bot_quota", g_botCount)

if tListenerIds then
    for k, v in ipairs(tListenerIds) do
        StopListeningToGameEvent(v)
    end
end

function OnRoundPreStart(event)
    g_playerCnt = 0
end


function OnRoundStart(event)

    if Convars:GetInt("server_booted") == 1 then
        Convars:SetInt("server_booted", 0)
        SendToServerConsole("changelevel cs_office")
        return
    end
    
    if not ScriptIsWarmupPeriod() then
        local daffyy = Timers:CreateTimer(0.1, function()
            print(g_playerCnt)
            BotAddHuman()
            if daffyy ~= nil then 
              Timers:RemoveTimer(daffyy)
            end
        end)
    end

    if g_expExplodeTimer ~= nil then 
        Timers:RemoveTimer(g_expExplodeTimer)
    end

    if g_isMapLoad == true then
        g_isMapLoad = false
        
        --PrecacheResource("characters/models/tm_phoenix_heavy/tm_phoenix_heavy.vmdl")
        if GetMapName() == "de_ancient" then
            DuplicateSpawnCount(g_humanTeam)
        end
        -- legacy, 강제 팀 변경 로직을 쓸 경우에는 할 필요 없음
        --DuplicateEnemySpawnCount()
        return
    end
end

function OnRoundEnd(event)
    if event.winner <= 1 then
        return
    end

    local level = Convars:GetInt("vsbots_level")
    local oldLevel = level
    if event.winner == g_humanTeam then
        level = math.min(12, level + 1)
    else
        level = math.max(0, level - 1)
    end

    Convars:SetInt("vsbots_level", level)
    Convars:SetInt("cash_player_respawn_amount", level)

    local oldLevelTag = "\x01"
    local levelTag = "\x01"
    if (oldLevel >= 7) then
        oldLevelTag = "\x02"
    end
    if (level >= 7) then
        levelTag = "\x02"
    end
    
    --ScriptPrintMessageChatAll(string.format("\x01 \x02[Level]%s %d \x01→%s %d", oldLevelTag, oldLevel, levelTag, level))
end

function OnPlayerConnect(event)
    g_isBot[event.userid] = 0
    g_botName[event.userid] = event.name
    print(string.format("userid : %d", event.userid))
    -- if event.xuid == 76561198002384750 then
    -- if event.xuid == 76561199583617008 then
    -- if event.name == "Stone" then
    --     g_botName[event.userid] = "Stoooone"
    -- end
end

function OnPlayerDisconnect(event)
    g_isBot[event.userid] = 0
    g_botName[event.userid] = ''
    g_playerPawn[event.userid] = nil
end

function OnPlayerTeam(event)
    if event.disconnect == 1 then
        return
    end

    g_isBot[event.userid] = event.isbot
end

function BotAddHuman()
    if g_playerCnt > 0 then
        local maxBotCnt = 8;
        local needBotCnt = maxBotCnt - g_playerCnt;
        for i = 1, needBotCnt do
            local cmdName = g_humanTeam == 2 and "bot_add_t" or "bot_add_ct"
            SendToServerConsole(string.format("%s \"[Human] PLAYER%d\"", cmdName, i))
        end
        for i = needBotCnt + 1, maxBotCnt, 1 do
            SendToServerConsole(string.format("bot_kick \"[Human] PLAYER%d\"", i))
        end
    end
end

function GetPlayersCount()
    local count = 0
    local tPlayerTable = Entities:FindAllByClassname("player")
    for _, player in ipairs(tPlayerTable) do
        count = count + 1
    end
    return count
end

function OnPlayerSpawn(event)
    local hPlayer = EHandleToHScript(event.userid_pawn)
    local botLevel = Convars:GetInt("vsbots_level")

    g_playerPawn[event.userid] = hPlayer

    if not hPlayer:IsAlive() then
        return
    end

    if hPlayer:GetTeam() == g_humanTeam then
        g_playerCnt = g_playerCnt + 1
    end
end

function DelaySetHealth(player, health)
    local daffyy = Timers:CreateTimer(0.1, function()
        player:SetHealth(health)
        player:SetMaxHealth(health)
        if daffyy ~= nil then 
          Timers:RemoveTimer(daffyy)
        end
    end)
end

function OnPlayerHurt(event)
    if event.weapon == "env_explosion" then
        return
    end

    if event.userid == event.attacker or event.weapon == "" or event.attacker_pawn == nil or event.dmg_health >= 999 then
        return
    end

    local hAttacker = EHandleToHScript(event.attacker_pawn)
    local hVictim = EHandleToHScript(event.userid_pawn)

    local botLevel = Convars:GetInt("vsbots_level")

    if event.health > 0 then
        if g_botName[event.userid] == "[Boss] Crusher" then
            if crusherRegenTimer ~= nil then 
                Timers:RemoveTimer(crusherRegenTimer)
            end
            
            local regenTime = 0.15
            if (botLevel >= 8) then
                regenTime = 0.1;
            end 

            crusherRegenTimer = Timers:CreateTimer(regenTime, function()
                hVictim:SetHealth(100)
            end)
        end
        if g_botName[event.attacker] == "[Boss] Stone" or (g_botName[event.attacker] == "[Boss] Crusher" and botLevel > 0) then
            local daffyy = Timers:CreateTimer(0.0, function()
                local cDamageInfo = CreateDamageInfo(hAttacker, nil, Vector(0, 0, 100), Vector(0, 0, 0), 9999, DMG_BULLET)
                hVictim:TakeDamage(cDamageInfo)
                DestroyDamageInfo(cDamageInfo)
                if daffyy ~= nil then 
                  Timers:RemoveTimer(daffyy)
                end
            end)
        end
    end
end

function OnPlayerDeath(event)
    if event.userid == event.attacker or event.weapon == "" or event.attacker_pawn == nil or event.dmg_health >= 999 then
        return
    end

    local hVictim = EHandleToHScript(event.userid_pawn)

    if g_isBot[event.userid] == 1 and g_botName[event.userid] == "[Boss] Exp203" then
        ScriptPrintMessageChatAll(string.format("\x01 \x02[Exp203 Explode] \x043초\x01 후에 \x02자폭\x01을 시도합니다.", g_botName[event.userid]))

        local timer1 = Timers:CreateTimer(1.0, function()
            ScriptPrintMessageChatAll(string.format("\x01 \x02[Exp203 Explode] \x042초\x01 후에 \x02자폭\x01을 시도합니다.", g_botName[event.userid]))
            if timer1 ~= nil then 
                Timers:RemoveTimer(timer1)
            end
        end)

        local timer1 = Timers:CreateTimer(2.0, function()
            ScriptPrintMessageChatAll(string.format("\x01 \x02[Exp203 Explode] \x041초\x01 후에 \x02자폭\x01을 시도합니다.", g_botName[event.userid]))
            if timer1 ~= nil then 
                Timers:RemoveTimer(timer1)
            end
        end)

        local origin = hVictim:GetOrigin()
        g_expExplodeTimer = Timers:CreateTimer(3.0, function()
            local explosion = SpawnEntityFromTableSynchronous("env_explosion", { iMagnitude = 9999, iRadiusOverride = 9999, teamnum = hVictim:GetTeam() })
            if explosion == nil then
                return
            end

            explosion:SetOrigin(origin)
            explosion:SetOwner(hVictim)
            DoEntFireByInstanceHandle(explosion, "Explode", "", 0.0, nil, nil)
            DoEntFireByInstanceHandle(explosion, "Kill", "", 0.1, nil, nil)
            g_expExplodeTimer = nil
        end)
    end


end

function OnWeaponFire(event)
    if g_botName[event.userid] == "[Boss] Crusher" then
        local hPlayer = EHandleToHScript(event.userid_pawn)
        local tInventory = hPlayer:GetEquippedWeapons()
        for key, weapon in ipairs(tInventory) do
            DoEntFireByInstanceHandle(weapon, "SetClipPrimary", tostring(200), 0.0, nil, nil)
        end
    end
end

function OnItemPickup(event)
    if g_botName[event.userid] == "[Boss] Crusher" and event.item ~= "knife" then
        local hPlayer = GetPlayerFromUserId(event.userid)
        local tInventory = hPlayer:GetEquippedWeapons()

        local hasShotgun = false;
        for key, value in ipairs(tInventory) do
            if value:GetClassname() == "weapon_xm1014" then
                hasShotgun = true
            end
        end
        if hasShotgun then
            for key, value in ipairs(tInventory) do
                if value:GetClassname() ~= "weapon_xm1014" then
                    DoEntFireByInstanceHandle(value, "Kill", "", 0.0, nil, nil)
                end
            end
        end
    end

    if g_isBot[event.userid] == 1 then
        if event.item == "hegrenade" or event.item == "flashbang" or event.item == "decoy" then
            local hPlayer = GetPlayerFromUserId(event.userid)
            for key, weapon in ipairs(hPlayer:GetEquippedWeapons()) do
                if weapon:GetClassname() == "weapon_"..event.item then
                    DoEntFireByInstanceHandle(weapon, "Kill", "", 0.0, nil, nil)
                end
            end
        end
    end

    -- if g_isBot[event.userid] == 1 and event.item == "famas" then
    --     local hPlayer = GetPlayerFromUserId(event.userid)
    --     local tInventory = hPlayer:GetEquippedWeapons()

    --     for key, value in ipairs(tInventory) do
    --         if value:GetClassname() == "weapon_famas" then
    --             value:Destroy()
    --         end
    --     end

    --     local giveWeapon = "weapon_m4a1"
    --     -- if RandomInt(1,2) == 2 then
    --     --     giveWeapon = "weapon_m4a1_silencer"
    --     -- end
    --     SendToServerConsole(string.format("giveweapon %d %s", event.userid, giveWeapon))
    -- end

    if g_botName[event.userid] == "[Boss] Stone" and event.item ~= "knife" then
        local hPlayer = GetPlayerFromUserId(event.userid)
        local tInventory = hPlayer:GetEquippedWeapons()

        for key, value in ipairs(tInventory) do
            if value:GetClassname() ~= "weapon_knife" then
                value:Destroy()
                -- doesn't work for fake player
                -- DoEntFireByInstanceHandle(clientcmd, "command", "slot3", 0.5, hPlayer, hPlayer)
            end
        end
    end

    if event.item == "smokegrenade" or event.item == "incgrenade" or event.item == "molotov" or
        event.item == "g3sg1" or event.item == "scar20" or event.item == "negev" then
        for _, entity in ipairs(Entities:FindAllByClassname("weapon_"..event.item)) do
            entity:Destroy()
        end
    end
end

function OnBulletImpact(event)
    -- @type CBasePlayerPawn
    local hPlayer = EHandleToHScript(event.userid_pawn)
    if g_botName[event.userid] == "Zeisen" then
        
        local origin = hPlayer:GetOrigin();
        origin.z = origin.z + 64;

        local dest = Vector(event.x, event.y, event.z)
        DebugDrawLine(origin, dest, RandomInt(0, 255), RandomInt(0, 255), RandomInt(0, 255), true, 0.2)
    end
    if g_botName[event.userid] == "[Boss] Crusher" then
        local origin = hPlayer:GetOrigin();
        origin.z = origin.z + 64;

        local dest = Vector(event.x, event.y, event.z)
        DebugDrawLine(origin, dest, 255, 0, 0, true, 0.5)
    end
end

tListenerIds = {
    ListenToGameEvent("round_prestart", OnRoundPreStart, nil),
    ListenToGameEvent("round_start", OnRoundStart, nil),
    ListenToGameEvent("round_start", OnRoundStart, nil),
    ListenToGameEvent("round_end", OnRoundEnd, nil),
    ListenToGameEvent("player_connect", OnPlayerConnect, nil),
    ListenToGameEvent("player_spawn", OnPlayerSpawn, nil),
    ListenToGameEvent("player_team", OnPlayerTeam, nil),
    ListenToGameEvent("player_hurt", OnPlayerHurt, nil),
    ListenToGameEvent("player_death", OnPlayerDeath, nil),
    ListenToGameEvent("item_pickup", OnItemPickup, nil),
    ListenToGameEvent("weapon_fire", OnWeaponFire, nil),
    ListenToGameEvent("player_chat", OnPlayerChat, nil),
    -- ListenToGameEvent("bullet_impact", OnBulletImpact, nil),
}

function DuplicateSpawnCount(team)
    local spawnCount = 0
    spawnEntityName = team == 2 and "info_player_terrorist" or "info_player_counterterrorist"
    local spawnTable = Entities:FindAllByClassname(spawnEntityName);
    for _, spawnEntity in ipairs(spawnTable) do
        for i = 1, 2, 1 do
            SpawnEntityFromTableSynchronous(spawnEntityName, {origin = spawnEntity:GetOrigin() });
            -- local angles = spawnEntity:GetAnglesAsVector()
            -- newSpawnEntity:SetOrigin(spawnEntity:GetOrigin())
            -- newSpawnEntity:SetAngles(angles.x, angles.y, angles.z)
            spawnCount = spawnCount + 1
        end
    end
    print("spawn count : "..spawnCount)
end

function EHandleToHScript(iPawnId)
    return EntIndexToHScript(bit.band(iPawnId, 0x3FFF))
end

function GetPlayerFromUserId(userid)
    --return UserIDToControllerHScript(userid)
    return g_playerPawn[userid]
end

---@return CBasePlayerPawn
function EntIndexToPawn(entIndex)
    return EntIndexToHScript(entIndex)
end

---@return CBasePlayerController
function EntIndexToController(entIndex)
    return EntIndexToHScript(entIndex)
end

---@return CBaseEntity
function EntIndexToEntity(entIndex)
    return EntIndexToHScript(entIndex)
end