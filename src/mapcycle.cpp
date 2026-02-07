
#include "utlvector.h"
#include "strtools.h"
#include "commands.h"
#include "ctimer.h"
#include <string>
#include <fstream>

using namespace std;

CConVar<int> g_cvarIndex("cs2f_mapcycle_index", FCVAR_NONE, "Skin Enable", 1);
CConVar<CUtlString> g_cvarNextMap("cs2f_mapcycle_nextmap", FCVAR_NONE, "Skin Enable", "");
CConVar<float> g_cvarChangeTime("cs2f_mapcycle_change_time", FCVAR_NONE, "Skin Enable", 15.0f);
CConVar<CUtlString> g_cvarWorkshopNameType("cs2f_mapcycle_workshop_name_type", FCVAR_NONE, "Skin Enable", "source_folder");
CConVar<bool> g_cvarRandom("cs2f_mapcycle_random", FCVAR_NONE, "Skin Enable", true);

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

//CON_COMMAND_CHAT(nextmap, "- show nextmap")
//{
//	if (!player)
//	{
//		ClientPrint(player, HUD_PRINTCONSOLE, "You cannot use this command from the server console.");
//		return;
//	}
//
//	ClientPrintAll(HUD_PRINTTALK, " \x04[Next Map]\x01 %s", GetMapName(g_mapList[g_cvarIndex.Get()].c_str()));
//}

void ChangeLevel(string mapName)
{
	return;
	char command[128];
	bool isWorkshop = mapName.find_first_not_of("0123456789") == string::npos;
	V_snprintf(command, sizeof(command), "%s %s", isWorkshop ? "host_workshop_map" : "changelevel", mapName);
	g_pEngineServer2->ServerCommand(command);
}

void MapCycle_OnLevelInit(const char* mapName)
{
	return;
	LoadMapCycle();
}

void LoadMapCycle(bool force)
{
	return;

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

		if (g_cvarRandom.Get())
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

		return pPublishDataKV->GetString("title");
	}

	return mapName;
}

void MapCycle_OnGameEnd()
{
	ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Next Map]\x01 %s", GetMapName(g_mapList[g_cvarIndex.Get()].c_str()));

	int mapIndex = g_cvarIndex.Get();
	
	CTimer::Create(g_cvarChangeTime.Get(), TIMERFLAG_MAP | TIMERFLAG_ROUND, [mapIndex]()
	{
		const char* nextMapName = g_cvarNextMap.Get().String()[0] == '\0' ? g_mapList[mapIndex].c_str() : g_cvarNextMap.Get().String();

		ChangeLevel(nextMapName);
		return -1.0f;
	});

	g_cvarIndex.Set(g_cvarIndex.Get());
	if (g_cvarIndex.Get() >= g_mapList.Count())
		g_cvarIndex.Set(0);
}