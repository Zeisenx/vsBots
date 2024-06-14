#pragma once

#include "schema.h"
#include "ccsplayerpawn.h"

class BotProfile;

enum VisiblePartType
{
    NONE		= 0x00,
    GUT			= 0x01,
    HEAD		= 0x02,
    LEFT_SIDE	= 0x04,			///< the left side of the object from our point of view (not their left side)
    RIGHT_SIDE	= 0x08,			///< the right side of the object from our point of view (not their right side)
    FEET		= 0x10
};

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
    SCHEMA_FIELD(uint8_t, m_visibleEnemyParts)
};

class BotProfile
{
public:
    float GetAggression()
    {
        return (float)((int)this + 8);
    };
    
    float GetSkill()
    {
        return (float)((int)this + 12);
    };
};