/*
	PARTICLE ENGINE FILE

    Copyright 2001 to 2007.

	File based on The Battle Grounds Team and Contributors
	(www.bg-mod.com)

	Additions by SysOp
	(www.arrangemode.tk)

	You should have received a copy of the Developers Readme File
    along with Arrange Mode's Source Code for Half-Life. If you
	are going to copy, modify, translate or distribute this file
	you should agree whit the terms of Developers Readme File.
*/

#ifndef PATICLE_EMITTER
#define PATICLE_EMITTER

#include "particle_defs.h"

const unsigned int SF_START_ON = 1;
const unsigned int SF_TRIGGERABLE = 2;
/*
class CParticleEmitter : public CPointEntity 
{
	char sParticleDefintionFile[MAX_PARTICLE_PATH];
	unsigned int iID;
	float flTimeTurnedOn;
public:
	bool bIsOn;
    void Spawn( void );
    void KeyValue( KeyValueData* pKeyValueData );
    void MakeAware( CBaseEntity* pEnt );
	void EXPORT Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() | FCAP_MASTER; }

	bool IsTriggered( CBaseEntity *pActivator );
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
};
*/

class CParticleEmitter : public CPointEntity 
{
	char sParticleDefintionFile[MAX_PARTICLE_PATH];
	unsigned int iID;
	float flTimeTurnedOn;
public:
    void Spawn( void );
	void Precache( void );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

    void KeyValue( KeyValueData* pKeyValueData );
    void MakeAware( CBaseEntity* pEnt );

	bool IsTriggered( CBaseEntity *pActivator );
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
		
	void EXPORT ResumeThink( void );
	void EXPORT TurnOn( void );
	void EXPORT TurnOff( void );

	void EXPORT Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	STATE GetState( void );
		
	bool bIsOn;
};

static unsigned int iParticleIDCount = 0;
#endif
