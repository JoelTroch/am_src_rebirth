/***
*
*		   °°
*			 °    °°°°°°°
*			° °   °  °  ° 
*		   °   °  °  °  °
*		  °     ° °  °  °
*	   HALF-LIFE: ARRANGEMENT
*
*	AR Software (c) 2004-2007. ArrangeMent, S2P Physics, Spirit Of Half-Life and their
*	logos are the property of their respective owners.
*
*	You should have received a copy of the Developers Readme File
*   along with Arrange Mode's Source Code for Half-Life. If you
*	are going to copy, modify, translate or distribute this file
*	you should agree whit the terms of Developers Readme File.
*
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*
*	This product includes information related to Half-Life 2 licensed from Valve 
*	(c) 2004. 
*
*	All Rights Reserved.
*
*	Modifications by SysOp (sysop_axis@hotmail).
*
***/

/*
	PARTICLE ENGINE FILE

    Copyright 2001 to 2007.

	File based on The Battle Grounds Team and Contributors
	(www.bg-mod.com)

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "func_grass.h"
#include "particle_emitter.h"

extern int gmsgGrassParticles;

void CGrass :: Spawn ( void )
{
	pev->spawnflags |= SF_GRASS_ACTIVE;

	pev->solid = SOLID_NOT;// always solid_not 
	SET_MODEL( ENT(pev), STRING(pev->model) );
	pev->effects |= EF_NODRAW;

	if ( FStringNull( pev->targetname ))
	{
		pev->spawnflags |= SF_GRASS_ACTIVE;
		ALERT(at_console, "WARNING: CGrass :: Spawn, Can't find targetname.\n");
	}

	if (pev->spawnflags & SF_GRASS_ACTIVE)
	{
		ALERT (at_console, "CGrass Spawn in <ON> Mode\nNextthink in 0.5 Seconds\n");

		bIsOn = true;
		iID = ++iParticleIDCount;

		SetThink( TurnOn );
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

void CGrass :: Precache ( void )
{
	if (pev->spawnflags & SF_GRASS_ACTIVE)
	{
		SetThink(&CGrass :: ResumeThink );

		pev->nextthink = 0.1;
	}
}

//Load values from the bsp
void CGrass::KeyValue( KeyValueData* pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "definition_file") )
	{
		strncat(sParticleDefintionFile, pkvd->szValue, strlen(pkvd->szValue));
		pkvd->fHandled = true;
	} else {
		CBaseEntity::KeyValue( pkvd );
	}
}

void CGrass :: TurnOn ( void )
{
	ALERT(at_console, "CGrass :: TurnOn, Current Time: %f\n", gpGlobals->time);

	pev->spawnflags |= SF_GRASS_ACTIVE;

	//the first time the particle spawns

	//	MakeAware( this);

	// increment the count just in case
	iID = ++iParticleIDCount;

	// tell everyone about our new grass
	if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgGrassParticles);
			WRITE_SHORT(iID);
			WRITE_BYTE(0);
			WRITE_COORD(pev->absmax.x);
			WRITE_COORD(pev->absmax.y);
			WRITE_COORD(pev->absmax.z);
			WRITE_COORD(pev->absmin.x);
			WRITE_COORD(pev->absmin.y);
			WRITE_COORD(pev->absmin.z);
	//		WRITE_STRING(sParticleDefintionFile);
			WRITE_STRING(STRING(pev->message));
		MESSAGE_END();
	}
}

void CGrass :: TurnOff ( void )
{
//	ALERT(at_console, "CGrass :: TurnOff\n");
	ALERT(at_console, "CGrass :: TurnOff Call Treated as MakeAware\n");

	// increment the count just in case
	iID = ++iParticleIDCount;

	// tell everyone about our new grass
	if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgGrassParticles);
			WRITE_SHORT(iID);
			WRITE_BYTE(0);
			WRITE_COORD(pev->absmax.x);
			WRITE_COORD(pev->absmax.y);
			WRITE_COORD(pev->absmax.z);
			WRITE_COORD(pev->absmin.x);
			WRITE_COORD(pev->absmin.y);
			WRITE_COORD(pev->absmin.z);
	//		WRITE_STRING(sParticleDefintionFile);
			WRITE_STRING(STRING(pev->message));
		MESSAGE_END();
	}

//	MakeAware( this );
	ALERT(at_console, "CGrass :: DontThink();\n");
	DontThink();
}

void CGrass :: ResumeThink ( void )
{
	ALERT(at_console, "CGrass :: ResumeThink, Current Time: %f\n", gpGlobals->time);
	SetThink(&CGrass ::TurnOff);

	pev->nextthink = 0.1;
}


void CGrass :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (ShouldToggle(useType))
	{
		if (pev->spawnflags & SF_GRASS_ACTIVE)
			TurnOff();
		else
			TurnOn();
	}
}

void CGrass::MakeAware( CBaseEntity *pEnt )
{
	/*
	ALERT(at_console, "CGrass :: MakeAware\n");
	
	bool bTurnOn = true;

	// increment the count just in case
	iID = ++iParticleIDCount;

	// tell everyone about our new grass
	MESSAGE_BEGIN(MSG_ALL, gmsgGrassParticles);
		WRITE_SHORT(iID);
		WRITE_BYTE(0);
		WRITE_COORD(pev->absmax.x);
		WRITE_COORD(pev->absmax.y);
		WRITE_COORD(pev->absmax.z);
		WRITE_COORD(pev->absmin.x);
		WRITE_COORD(pev->absmin.y);
		WRITE_COORD(pev->absmin.z);
//		WRITE_STRING(STRING(pev->message));//if you use this -> crash
		WRITE_STRING(sParticleDefintionFile);
	MESSAGE_END();*/
}

STATE CGrass::GetState( void )
{
	if (pev->spawnflags & SF_GRASS_ACTIVE)
	{
		return STATE_ON;
	}
	else
	{
		return STATE_OFF;
	}
}

LINK_ENTITY_TO_CLASS( func_grass, CGrass );
