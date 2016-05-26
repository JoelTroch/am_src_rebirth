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

/*

  Please Read:
  The main problem in SP games is the save/restore function. The original code only was created for
  a Multiplayer game, so that, it's seems to be impossible save or restore anything using the original code.

  I have been creating a lot of particles files when I get a error in game. The problem appears when I tried
  to use "slowmotion" command. This calls SlowMotion() func, in player.cpp. It fires a sound and change the
  cvar "host_framerate" to 0.01 (and disable the player model). I really exactly don't know if there is a
  problem whit Slowmotion. It's seems to be the host_framerate, when it's changed, the particle's engine just
  fucked up. 
  I used the Debug metod to detect the exact time when the engine goes wrong, but I disable the SlowMotion()
  func first. After a few minutes, the debugger shows me a func related to particle's collision. Well... I
  delete each line who starts whit that word... the crash still happening.

  I think the Particle's engine works perfect, but maybe my own way to try to make a metod to save and restore
  it could be wrong. But wait a minute, I've been trying to save the value of "sParticleDefintionFile", but when
  I tried, the engine crash. I used FIELD_CHARACTER, FIELD_BOOLEAN ... and so on... It's seems there is no way
  to save it.

  The save/restore func it's... hard-coded... The particles doens't save each value, they are created every time
  when you load a game...

  However, the crash only it's related to this entity. The func_grass appears to be Ok, and I didn't see any
  problems when I spawn particles that dies in a few seconds (used for blood, chunks, bullet impacts, explosions)

  Well, the code it's here. If you're a good person try to make a clean entity, whitout any crashes, that can
  be saved and restored perfectly.

*/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "particle_emitter.h"
#include <ctype.h>

//Mod developers: if you get a message error about "memfgets" in your code, modify the same call at sound.cpp
extern char *memfgets( byte *pMemFile, int fileSize, int &filePos, char *pBuffer, int bufferSize );

// create ourselves a particle emitter
void CParticleEmitter::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL (ENT(pev), STRING(pev->model));
	pev->effects |= EF_NODRAW;
	
	UTIL_SetOrigin( pev, pev->origin ); 		
	UTIL_SetSize( pev, pev->absmin, pev->absmax );

    SetUse ( &CParticleEmitter::Use );

	if (pev->spawnflags & SF_START_ON)
	{
		ALERT (at_console, "CParticleEmitter Spawn in <ON> Mode\nNextthink in 0.5 Seconds\n");

		bIsOn = true;
		iID = ++iParticleIDCount;

		SetThink( TurnOn );
		pev->nextthink = gpGlobals->time + 0.5;
	}
	else
	{
		bIsOn = false;
	}

	IsTriggered(NULL);
	iID = ++iParticleIDCount;
	flTimeTurnedOn = 0.0;
}

//Load values from the bsp
//Sys0p: This is not really neccessary
void CParticleEmitter::KeyValue( KeyValueData* pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "definition_file") )
	{
		strncat(sParticleDefintionFile, pkvd->szValue, strlen(pkvd->szValue));
		pkvd->fHandled = true;
	} else {
		CBaseEntity::KeyValue( pkvd );
	}
}

extern int gmsgParticles;

void CParticleEmitter :: Precache ( void )
{
	if (pev->spawnflags & SF_START_ON)
	{
		SetThink(&CParticleEmitter :: ResumeThink );

		pev->nextthink = 0.1;
	}
}
void CParticleEmitter :: TurnOn ( void )
{
	ALERT(at_console, "CParticleEmitter :: TurnOn, Current Time: %f\n", gpGlobals->time);

	pev->spawnflags |= SF_START_ON;

	//the first time the particle spawns

	// increment the count just in case
	iID = ++iParticleIDCount;

	// lets give them everything
	if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);//enviar a todos... qué importa??
			WRITE_SHORT(iID);
			WRITE_BYTE(0);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_COORD(pev->angles.x);
			WRITE_COORD(pev->angles.y);
			WRITE_COORD(pev->angles.z);
			WRITE_SHORT(0);
			WRITE_STRING(STRING(pev->message));
	//		WRITE_STRING(sParticleDefintionFile);
		MESSAGE_END();
	}
}

void CParticleEmitter :: TurnOff ( void )
{
//	ALERT(at_console, "CParticleEmitter :: TurnOff\n");
	ALERT(at_console, "CParticleEmitter :: TurnOff Call Treated as MakeAware\n");

	// increment the count just in case
	iID = ++iParticleIDCount;

	// lets give them everything
	if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);//enviar a todos... qué importa??
			WRITE_SHORT(iID);
			WRITE_BYTE(0);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_COORD(pev->angles.x);
			WRITE_COORD(pev->angles.y);
			WRITE_COORD(pev->angles.z);
			WRITE_SHORT(0);
			WRITE_STRING(STRING(pev->message));
	//		WRITE_STRING(sParticleDefintionFile);
		MESSAGE_END();
	}

//	MakeAware( this );
	ALERT(at_console, "CParticleEmitter :: DontThink();\n");
	DontThink();
}

void CParticleEmitter :: ResumeThink ( void )
{
	ALERT(at_console, "CParticleEmitter :: ResumeThink, Current Time: %f\n", gpGlobals->time);
	SetThink(&CParticleEmitter ::TurnOff);

	pev->nextthink = 0.1;
}


void CParticleEmitter :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (ShouldToggle(useType))
	{
		if (pev->spawnflags & SF_START_ON)
		{
			TurnOff();
		}
		else
			TurnOn();
	}
}

// Let the player know there is a particleemitter
void CParticleEmitter::MakeAware( CBaseEntity *pEnt )
{/*
	ALERT(at_console, "CParticleEmitter :: MakeAware\n");
	
	bool bTurnOn = true;

	// lets give them everything
	MESSAGE_BEGIN(MSG_ONE, gmsgParticles, NULL, pEnt->pev);
		WRITE_SHORT(iID);
		WRITE_BYTE(0);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(pev->angles.x);
		WRITE_COORD(pev->angles.y);
		WRITE_COORD(pev->angles.z);
		WRITE_SHORT(0);
//		WRITE_STRING(STRING(pev->message));//test
		WRITE_STRING(sParticleDefintionFile);
	MESSAGE_END();*/
}

STATE CParticleEmitter::GetState( void )
{
	if (pev->spawnflags & SF_START_ON)
	{
		return STATE_ON;
	}
	else
	{
		return STATE_OFF;
	}
}

// is the particle system on.
bool CParticleEmitter::IsTriggered( CBaseEntity* ) 
{
	// not on so it isn't triggered
	if(bIsOn == false)
		return false;

	int iFileSize = 0; int iPos = 0;
	byte *pFile = g_engfuncs.pfnLoadFileForMe(sParticleDefintionFile, &iFileSize);

	if(!pFile) 
	{
		ALERT(at_console, "Bad Mapped Particle definition file specified %s\n", sParticleDefintionFile);
		return false;
	}

	char sBuffer[512]; char sSetting[64]; char sValue[64]; bool bFound = false;
	memset(sBuffer, 0, 512); memset(sSetting, 0, 64); memset(sValue, 0, 64);

	// loop through each line
	int i = 0; int j = 0;
	while( memfgets(pFile, iFileSize, iPos, sBuffer, sizeof(sBuffer) - 1) != NULL) {
		i = 0; j = 0;
		// trim leading white spaces
		while(sBuffer[i] && isspace(sBuffer[i]))
			i++;

		// comment so next line
		if(sBuffer[i] == '/' && sBuffer[i+1] == '/')
			continue;

		// read the setting
		while(i < 63 && sBuffer[i] && !isspace(sBuffer[i])) {
			sSetting[i] = sBuffer[i++];
		}
		sSetting[i] = '\0';

		// if the setting isn't the system_life move onto the next line
		if(strcmp(sSetting, "system_life"))
			continue;

		// remove the spaces between setting and value
		while(sBuffer[i] && isspace(sBuffer[i]))
			i++;

		// read the value
		while(j < 63 && sBuffer[i] && !isspace(sBuffer[i])) {
			sValue[j++] = sBuffer[i++];
		}
		sValue[j] = '\0';
		bFound = true;

		// have the value, run for the hills
		break;
	}

	g_engfuncs.pfnFreeFile(pFile);

	// defaults to -1
	if(bFound == false)
		return true;

	// infinite so it must be on
	if(atof(sValue) == -1.0)
		return true;

	// the time the system life + the time it was turned on is in the future
	if(atof(sValue) + flTimeTurnedOn + 0.1 > gpGlobals->time)
		return true;

	// not in the future so its dead
	return false;
}

TYPEDESCRIPTION	CParticleEmitter::m_SaveData[] = 
{
	DEFINE_FIELD( CParticleEmitter, bIsOn, FIELD_BOOLEAN )
};

IMPLEMENT_SAVERESTORE( CParticleEmitter, CPointEntity );

LINK_ENTITY_TO_CLASS( env_particleemitter, CParticleEmitter );