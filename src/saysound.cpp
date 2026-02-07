
#include "entity/ccsplayercontroller.h"
#include "recipientfilters.h"
#include "commands.h"
#include "saysound.h"
#include <regex>

KeyValues* g_pSaySoundsKV;

void LoadSaySoundsKV();

CON_COMMAND_F(cs2f_saysound_reload, "Reload Saysound", FCVAR_LINKED_CONCOMMAND | FCVAR_SPONLY | FCVAR_PROTECTED)
{
	LoadSaySoundsKV();
	Message("Reloaded Saysounds config.\n");
}

void EmitSoundToAll(const char* pszSound)
{
	for (int i = 0; i < GetGlobals()->maxClients; i++)
	{
		CCSPlayerController* pController = CCSPlayerController::FromSlot(i);
		if (!pController || !pController->IsConnected() || pController->IsBot())
			continue;

		CSingleRecipientFilter filter(pController->GetPlayerSlot());
		pController->StopSound(pszSound);
		pController->EmitSoundFilter(filter, pszSound);
	}
}

void LoadSaySoundsKV()
{
	if (g_pSaySoundsKV)
		delete g_pSaySoundsKV;

	g_pSaySoundsKV = new KeyValues("SaySounds");

	const char* pszPath = "addons/cs2fixes/configs/saysounds.cfg";

	if (!g_pSaySoundsKV->LoadFromFile(g_pFullFileSystem, pszPath))
	{
		Warning("Failed to load saysound file %s\n", pszPath);
		return;
	}
}

void SaySound_Init()
{
	LoadSaySoundsKV();
}

void SaySound_Precache(IEntityResourceManifest* pResourceManifest)
{
	pResourceManifest->AddResource("soundevents/soundevents_saysounds.vsndevts");
}

bool SaySound_OnChat(CCSPlayerController* pController, const char* pMessage)
{
	if (!g_pSaySoundsKV)
		return false;

	std::string bufStr = pMessage;
	bufStr = std::regex_replace(bufStr, std::regex("\\!"), "EXCL");
	bufStr = std::regex_replace(bufStr, std::regex("\\~"), "tilda");
	bufStr = std::regex_replace(bufStr, std::regex("\\-"), "_");
	bufStr = std::regex_replace(bufStr, std::regex("\\^"), "CARET");

	for (KeyValues* pKey = g_pSaySoundsKV->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
	{
		const char* pszName = pKey->GetName();
		if (V_strcmp(pszName, "#Settings") == 0)
			continue;

		if (V_stricmp(pszName, bufStr.c_str()) == 0)
		{
			ClientPrintAll(HUD_PRINTTALK, " \x03%s \x01played \x03%s", pController->GetPlayerName(), pMessage);
			EmitSoundToAll(pKey->GetString("sound_trigger"));
			return true;
		}
	}

	return false;
}