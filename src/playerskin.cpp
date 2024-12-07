

#include "commands.h"
#include "eventlistener.h"
#include "utils/entity.h"
#include "ctimer.h"
#include "playerskin.h"
 
KeyValues* g_pKVConfig;

void LoadSkinConfig();

void PlayerSkin_OnLevelInit(char const* pMapName)
{
	LoadSkinConfig();
}

void PlayerSkin_Precache(IEntityResourceManifest* pResourceManifest)
{
	if (g_pKVConfig)
	{
		for (KeyValues* pKey = g_pKVConfig->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
		{
			pResourceManifest->AddResource(pKey->GetString("path"));
		}
	}
}

void LoadSkinConfig()
{
	if (g_pKVConfig)
		delete g_pKVConfig;

	g_pKVConfig = new KeyValues("PlayerSkin");
	const char* pszPath = "addons/cs2fixes/configs/playerskin.cfg";
	if (!g_pKVConfig->LoadFromFile(g_pFullFileSystem, pszPath))
	{
		Warning("Failed to load %s\n", pszPath);
		return;
	}
}

bool IsValidSkin(const char* keyName)
{
	if (!g_pKVConfig)
		return false;

	for (KeyValues* pKey = g_pKVConfig->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
	{
		if (V_strcmp(pKey->GetName(), keyName) != 0)
			continue;

		return true;
	}

	return false;
}

bool SetSkin(CCSPlayerPawn* pPawn, std::string keyName)
{
	if (!g_pKVConfig)
		return false;

	for (KeyValues* pKey = g_pKVConfig->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
	{
		if (V_strcmp(pKey->GetName(), keyName.c_str()) != 0)
			continue;

		pPawn->SetModel(pKey->GetString("path"));
		return true;
	}
	
	return false;
}

CON_COMMAND_CHAT(skin, "- set skin")
{
	if (!player)
	{
		ClientPrint(player, HUD_PRINTCONSOLE, CHAT_PREFIX "You cannot use this command from the server console.");
		return;
	}

	if (args.ArgC() < 2)
	{
		ClientPrint(player, HUD_PRINTTALK, CHAT_PREFIX "Command : !skin <skin name>");
		return;
	}

	if (!IsValidSkin(args[1]))
	{
		ClientPrint(player, HUD_PRINTTALK, CHAT_PREFIX "Skin is invalid.");
		return;
	}

	ZEPlayer *pPlayer = player->GetZEPlayer();
	auto dbInfo = pPlayer->GetDBInfo();
	dbInfo.pszSkinName = args[1];
	pPlayer->SetDBInfo(dbInfo);

	ClientPrint(player, HUD_PRINTTALK, CHAT_PREFIX "Skin selected to %s", args[1]);
}

void PlayerSkin_OnPlayerSpawn(CCSPlayerController* pController)
{
	CHandle<CCSPlayerController> handle = pController->GetHandle();
	new CTimer(0.0f, false, false, [handle]()
	{
		CCSPlayerController* pController = (CCSPlayerController*)handle.Get();
		if (!pController)
			return -1.0f;

		CCSPlayerPawn* pPawn = pController->GetPlayerPawn();
		if (!pPawn)
			return -1.0f;

		if (!pPawn->IsAlive() || pController->IsBot())
			return -1.0f;

		DBInfo info = pController->GetZEPlayer()->GetDBInfo();
		if (!info.bDataLoaded)
			return -1.0f;

		Message("%s : %s\n", pController->GetPlayerName(), info.pszSkinName.c_str());
		if (!info.pszSkinName.empty())
		{
			SetSkin(pPawn, info.pszSkinName);
		}

		return -1.0f;
	});
}
