#pragma once

#include "schema.h"
#include "ccsplayerpawn.h"

class CCSPlayerPawn;

enum VisiblePartType
{
    NONE = 0x00,
    GUT = 0x01,
    HEAD = 0x02,
    LEFT_SIDE = 0x04,			///< the left side of the object from our point of view (not their left side)
    RIGHT_SIDE = 0x08,			///< the right side of the object from our point of view (not their right side)
    FEET = 0x10
};

class CountdownTimer
{
private:
    [[maybe_unused]] uint8_t __pad0000[0x8]; // 0x0
public:
    // MNetworkEnable
    float m_duration; // 0x8	
    // MNetworkEnable
    GameTime_t m_timestamp; // 0xc	
    // MNetworkEnable
    float m_timescale; // 0x10	
    // MNetworkEnable
    WorldGroupId_t m_nWorldGroupId; // 0x14	
};

class BotState
{
public:
    virtual void OnEnter(CCSBot* bot) { }				///< when state is entered
    virtual void OnUpdate(CCSBot* bot) { }			///< state behavior
    virtual void OnExit(CCSBot* bot) { }				///< when state exited
    virtual const char* GetName(void) const = 0;		///< return state name
};

class AttackState : public BotState
{
public:
    virtual void OnEnter(CCSBot* bot);
    virtual void OnUpdate(CCSBot* bot);
    virtual void OnExit(CCSBot* bot);
    virtual const char* GetName(void) const { return "Attack"; }

    void SetCrouchAndHold(bool crouch) { m_crouchAndHold = crouch; }

protected:
    enum DodgeStateType
    {
        STEADY_ON,
        SLIDE_LEFT,
        SLIDE_RIGHT,
        JUMP,

        NUM_ATTACK_STATES
    };
    DodgeStateType m_dodgeState;
    float m_nextDodgeStateTimestamp;

    CountdownTimer m_repathTimer;
    float m_scopeTimestamp;

    bool m_haveSeenEnemy;										///< false if we haven't yet seen the enemy since we started this attack (told by a friend, etc)
    bool m_isEnemyHidden;										///< true we if we have lost line-of-sight to our enemy
    float m_reacquireTimestamp;									///< time when we can fire again, after losing enemy behind cover
    float m_shieldToggleTimestamp;								///< time to toggle shield deploy state
    bool m_shieldForceOpen;										///< if true, open up and shoot even if in danger

    float m_pinnedDownTimestamp;								///< time when we'll consider ourselves "pinned down" by the enemy

    bool m_crouchAndHold;
    bool m_didAmbushCheck;
    bool m_shouldDodge;
    bool m_firstDodge;

    bool m_isCoward;											///< if true, we'll retreat if outnumbered during this fight
    CountdownTimer m_retreatTimer;

    void StopAttacking(CCSBot* bot);
    void Dodge(CCSBot* bot);									///< do dodge behavior
};

class BotProfile
{
public:
    BotProfile(void)
    {
        m_name = NULL;

        m_aggression = 1.0f;
        m_skill = 1.0f;
        m_teamwork = 0.5f;
        m_weaponPreferenceCount = 0;

        m_aimFocusInitial = 0.0f;
        m_aimFocusDecay = 0.01f;
        m_aimFocusOffsetScale = 0.0f;
        m_aimFocusInterval = 0.01f;

        m_cost = 0;
        m_skin = 0;
        m_difficultyFlags = 0;
        m_voicePitch = 100;
        m_reactionTime = 0.3f;
        m_attackDelay = 0.0f;
        m_lookAngleMaxAccelNormal = 3000.0f;
        m_lookAngleStiffnessNormal = 300.0f;
        m_lookAngleDampingNormal = 25.0f;
        m_lookAngleMaxAccelAttacking = 3000.0f;
        m_lookAngleStiffnessAttacking = 300.0f;
        m_lookAngleDampingAttacking = 30.0f;

        m_teams = CS_TEAM_NONE;
        m_voiceBank = 0;
        m_prefersSilencer = false;
    }

    char* m_name;							// the bot's name
    float m_aggression;						// percentage: 0 = coward, 1 = berserker
    float m_skill;							// percentage: 0 = terrible, 1 = expert
    float m_teamwork;						// percentage: 0 = rogue, 1 = complete obeyance to team, lots of comm

    float m_aimFocusInitial;				// initial minimum aim error on first attack
    float m_aimFocusDecay;					// how quickly our focus error decays (scale/sec)
    float m_aimFocusOffsetScale;			// how much aim focus error we get based on maximum angle distance from our view angle
    float m_aimFocusInterval;				// how frequently we update our focus

    enum { MAX_WEAPON_PREFS = 16 };
    short m_weaponPreference[MAX_WEAPON_PREFS];	///< which weapons this bot likes to use, in order of priority
    int m_weaponPreferenceCount;

    int m_cost;								///< reputation point cost for career mode
    int m_skin;								///< "skin" index
    unsigned char m_difficultyFlags;		///< bits set correspond to difficulty levels this is valid for
    int m_voicePitch;						///< the pitch shift for bot chatter (100 = normal)
    float m_reactionTime;					///< our reaction time in seconds
    float m_attackDelay;					///< time in seconds from when we notice an enemy to when we open fire
    int m_teams;							///< teams for which this profile is valid

    bool m_prefersSilencer;					///< does the bot prefer to use silencers?

    int m_voiceBank;						///< Index of the BotChatter.db voice bank this profile uses (0 is the default)

    float m_lookAngleMaxAccelNormal;		// Acceleration of look angle spring under normal conditions
    float m_lookAngleStiffnessNormal;		// Stiffness of look angle spring under normal conditions
    float m_lookAngleDampingNormal;			// Damping of look angle spring under normal conditions

    float m_lookAngleMaxAccelAttacking;		// Acceleration of look angle spring under attack conditions
    float m_lookAngleStiffnessAttacking;	// Stiffness of look angle spring under attack conditions
    float m_lookAngleDampingAttacking;		// Damping of look angle spring under attack conditions

    CUtlVector< const BotProfile* > m_templates;							///< List of templates we inherit from

    const char* GetName(void) const { return m_name; }		///< return bot's name
};
typedef CUtlLinkedList<BotProfile*> BotProfileList;

#define NumCustomSkins 100
class BotProfileManager
{
public:
    typedef CUtlVector<char*> VoiceBankList;

    BotProfileList m_profileList;
    BotProfileList m_templateList;

    VoiceBankList m_voiceBanks;

    char* m_skins[NumCustomSkins];						///< Custom skin names
    char* m_skinModelnames[NumCustomSkins];				///< Custom skin modelnames
    char* m_skinFilenames[NumCustomSkins];				///< Custom skin filenames
    int m_nextSkin;											///< Next custom skin to allocate

    BotProfile* GetProfile(const char* name)
    {
        FOR_EACH_LL(m_profileList, it)
        {
            BotProfile* profile = m_profileList[it];

            if (!V_stricmp(name, profile->GetName()))
                return profile;
        }

        return NULL;
    }
};

class CBot
{
public:
    DECLARE_SCHEMA_CLASS(CBot)

    SCHEMA_FIELD(CCSPlayerController*, m_pController)
    SCHEMA_FIELD(CCSPlayerPawn*, m_pPlayer)
};

class CCSBot : public CBot
{
public:
    DECLARE_SCHEMA_CLASS(CCSBot)

    SCHEMA_FIELD_POINTER(char, m_name)
    SCHEMA_FIELD(bool, m_hasVisitedEnemySpawn)
    SCHEMA_FIELD(float, m_lookPitch)
    SCHEMA_FIELD(float, m_lookPitchVel)
    SCHEMA_FIELD(float, m_lookYaw)
    SCHEMA_FIELD(float, m_lookYawVel)
    SCHEMA_FIELD(Vector, m_targetSpot)
    SCHEMA_FIELD(CHandle<CCSPlayerPawn>, m_enemy)
    SCHEMA_FIELD(bool, m_isEnemyVisible)
    SCHEMA_FIELD(uint8_t, m_visibleEnemyParts)
    SCHEMA_FIELD(float, m_lastSawEnemyTimestamp)
    SCHEMA_FIELD(GameTime_t, m_fireWeaponTimestamp)

    BotProfile* GetLocalProfile()
    {
        return *reinterpret_cast<std::add_pointer_t<BotProfile*>>((uintptr_t)(this) + g_GameConfig->GetOffset("CCSBot::m_pLocalProfile"));
    }
};