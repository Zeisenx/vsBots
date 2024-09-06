
#pragma once

#include "cbasemodelentity.h"

class CModelPointEntity : public CBaseModelEntity
{
public:
	DECLARE_SCHEMA_CLASS(CModelPointEntity)
};

class CEnvExplosion : public CModelPointEntity
{
public:
	DECLARE_SCHEMA_CLASS(CEnvExplosion)

	SCHEMA_FIELD(int, m_iMagnitude)
	SCHEMA_FIELD(int, m_iRadiusOverride)
	SCHEMA_FIELD(CHandle<CBaseEntity>, m_hInflictor)
};