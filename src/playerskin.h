
class CCSPlayerController;
class IEntityResourceManifest;

void PlayerSkin_OnLevelInit(char const* pMapName);
void PlayerSkin_Precache(IEntityResourceManifest* pResourceManifest);
void PlayerSkin_OnPlayerSpawn(CCSPlayerController* pController);