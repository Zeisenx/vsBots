#pragma once

#include "schema.h"
#include "ccsplayerpawn.h"

class CCSPlayerPawn;

template < class PlayerType >
class CBot
{
public:
    DECLARE_SCHEMA_CLASS(CBot)
};

class CCSBot : public CBot< CCSPlayerPawn >
{
public:
    DECLARE_SCHEMA_CLASS(CCSBot)

    SCHEMA_FIELD_POINTER(char, m_name)
    SCHEMA_FIELD(bool, m_hasVisitedEnemySpawn)
    SCHEMA_FIELD(CHandle<CCSPlayerPawn>, m_enemy)
    SCHEMA_FIELD(bool, m_isEnemyVisible)
};