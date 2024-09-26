
#include "utlvector.h"
#include "strtools.h"
#include "commands.h"
#include "ctimer.h"
#include <string>
#include <fstream>

using namespace std;

int g_mapListIndex = 1;
string g_forceNextMap;
float g_mapChangeTime = 15.0f;
string g_workshopNameType = "source_folder";
bool g_randomMapCycle = true;

FAKE_INT_CVAR(cs2f_mapcycle_index, "", g_mapListIndex, false, false)
FAKE_STRING_CVAR(cs2f_mapcycle_nextmap, "", g_forceNextMap, false)
FAKE_FLOAT_CVAR(cs2f_mapcycle_change_time, "", g_mapChangeTime, 15.0f, false)
FAKE_STRING_CVAR(cs2f_mapcycle_workshop_name_type, "", g_workshopNameType, false)
FAKE_BOOL_CVAR(cs2f_mapcycle_random, "", g_randomMapCycle, true, false)

CUtlVector<string> g_mapList;

extern IVEngineServer2* g_pEngineServer2;

void ChangeLevel(string mapName);
void LoadMapCycle(bool force=false);
string GetMapName(string mapName);

CON_COMMAND_F(cs2f_mapcycle_reload, "Reload Mapcycle", FCVAR_LINKED_CONCOMMAND | FCVAR_SPONLY | FCVAR_PROTECTED)
{
	LoadMapCycle(true);
	Message("Reloaded mapcycle config.\n");
}

CON_COMMAND_F(cs2f_mapcycle_list, "Show Mapcycle List", FCVAR_LINKED_CONCOMMAND | FCVAR_SPONLY | FCVAR_PROTECTED)
{
	FOR_EACH_VEC(g_mapList, i)
	{
		Message("%d : %s\n", i+1, g_mapList[i].c_str());
	}
}

CON_COMMAND_CHAT(nextmap, "- show nextmap")
{
	if (!player)
	{
		ClientPrint(player, HUD_PRINTCONSOLE, "You cannot use this command from the server console.");
		return;
	}

	ClientPrintAll(HUD_PRINTTALK, " \x04[Next Map]\x01 %s", GetMapName(g_mapList[g_mapListIndex].c_str()));
}

void ChangeLevel(string mapName)
{
	char command[128];
	bool isWorkshop = mapName.find_first_not_of("0123456789") == string::npos;
	V_snprintf(command, sizeof(command), "%s %s", isWorkshop ? "host_workshop_map" : "changelevel", mapName);
	g_pEngineServer2->ServerCommand(command);
}

void MapCycle_OnLevelInit(const char* mapName)
{
	LoadMapCycle();
}

void LoadMapCycle(bool force)
{
	const bool firstLoad = g_mapList.Count() == 0;
	
	if (firstLoad || force)
	{
		g_mapList.RemoveAll();

		const char* filePath = "addons/cs2fixes/configs/mapcycle.txt";
		char path[MAX_PATH];
		V_snprintf(path, sizeof(path), "%s%s%s", Plat_GetGameDirectory(), "/csgo/", filePath);
		fstream infile(path);
		string mapName;
		while (getline(infile, mapName))
		{
			size_t cutPos = mapName.find_first_of(" /");
			if (cutPos != string::npos)
				mapName = mapName.substr(0, cutPos);

			g_mapList.AddToTail(mapName);
		}

		if (g_randomMapCycle)
		{
			g_mapList.Sort([](const string* a, const string* b)
			{
				return rand() % 3 - 1;
			});
		}
	}

	if (firstLoad)
	{
		const char* pStartMap = g_mapList[0].c_str();
		ChangeLevel(pStartMap);
	}
}

string GetMapName(string mapName)
{
	if (!mapName.empty() && mapName.find_first_not_of("0123456789") == string::npos)
	{
		char path[MAX_PATH];
		V_snprintf(path, sizeof(path), "%s/bin/win64/steamapps/workshop/content/730/%s/publish_data.txt", Plat_GetGameDirectory(), mapName.c_str());

		KeyValues *pPublishDataKV = new KeyValues("publish_data");
		if (!pPublishDataKV->LoadFromFile(g_pFullFileSystem, path))
		{
			Warning("Failed to load file %s\n", path);
			return mapName;
		}

		return pPublishDataKV->GetString("source_folder");
	}

	return mapName;
}

void MapCycle_OnGameEnd()
{
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Next Map]\x01 %s", GetMapName(g_mapList[g_mapListIndex].c_str()));

	new CTimer(g_mapChangeTime, false, false, []()
	{
		const char* nextMapName = g_forceNextMap.size() == 0 ? g_mapList[g_mapListIndex].c_str() : g_forceNextMap.c_str();

		ChangeLevel(nextMapName);
		return -1.0f;
	});

	g_mapListIndex++;
	if (g_mapListIndex >= g_mapList.Count())
		g_mapListIndex = 0;
}