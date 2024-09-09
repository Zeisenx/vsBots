
#include "utlvector.h"
#include "strtools.h"
#include "commands.h"
#include "ctimer.h"
#include <string>
#include <fstream>

int g_mapListIndex = 0;

CUtlVector<std::string> g_mapList;

extern IVEngineServer2* g_pEngineServer2;

void MapCycle_OnLevelInit(const char* currentMapName)
{
	const char* pszFilePath = "addons/cs2fixes/configs/mapcycle.txt";
	char szPath[MAX_PATH];
	V_snprintf(szPath, sizeof(szPath), "%s%s%s", Plat_GetGameDirectory(), "/csgo/", pszFilePath);
	std::ifstream infile(szPath);
	std::string mapName;
	while (std::getline(infile, mapName))
	{
		g_mapList.AddToTail(mapName);
	}
}

void MapCycle_OnGameEnd()
{
	g_mapListIndex++;
	if (g_mapListIndex >= g_mapList.Count())
		g_mapListIndex = 0;

	ClientPrintAll(HUD_PRINTTALK, "\x01 \x04[Next Map]\x01 %s", g_mapList[g_mapListIndex].c_str());

	new CTimer(10.0f, false, false, []()
	{
		const char* nextMapName = g_mapList[g_mapListIndex].c_str();
		bool isWorkshop = V_strncmp(nextMapName, "de_", 3) != 0 && V_strncmp(nextMapName, "cs_", 3) != 0;

		char command[128];
		V_snprintf(command, sizeof(command), "%s %s", isWorkshop ? "host_workshop_map" : "changelevel", nextMapName);
		g_pEngineServer2->ServerCommand(command);

		return -1.0f;
	});
}