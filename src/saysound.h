#pragma once

class IEntityResourceManifest;
class CCSPlayerController;

void SaySound_Init();
void SaySound_Precache(IEntityResourceManifest* pResourceManifest);
bool SaySound_OnChat(CCSPlayerController* pController, const char* pMessage);
