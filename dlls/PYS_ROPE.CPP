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

//****************
//RYOKEEN's CODE
//****************/

// Ryo, you know I love you, but I cannot make this work. I need to keep looking
// in the whole code, but I haven't time.

//TO DO: Need I to add the CPointEntity::Activate() and UTIL_DesiredAction(). I'm afraid of crashes. 
//Anyway I can't make it work in any SoHL version.


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
//#include "locus.h" //LRC - locus utilities
//#include "movewith.h" //LRC - the DesiredThink system
#include "pys_rope.h"

LINK_ENTITY_TO_CLASS( pys_rope, CPys_Rope );

#define START_ON 1

void CPys_Rope::Spawn( void )
{
	ALERT(at_console, "CPys_Rope Spawned!\n");

	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	//pev->renderamt		= 128;
	pev->rendermode		= kRenderTransAdd;

	Precache();

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(edict(), "sprites/null.spr");

	pev->body = (pev->spawnflags & START_ON) != 0;

	SetThink (Think);
	pev->nextthink = gpGlobals->time + 1.0;
}

void CPys_Rope::Precache( void )
{
	PRECACHE_MODEL("sprites/null.spr");
//	PRECACHE_MODEL("sprites/rope.spr");
}

extern int gmsgAddRope;

void CPys_Rope::Activate(void)
{	
//	CPointEntity::Activate();
//	UTIL_DesiredAction(this);
}

void CPys_Rope::DesiredAction( void )
{
//	pev->nextthink = gpGlobals->time + 0.5;
}
void CPys_Rope::Think( void )
{
	/*
	ALERT(at_console, "CPys_Rope is Thinking!\n");

	CBaseEntity *pTarget = NULL;
	pTarget = UTIL_FindEntityByTargetname( pTarget, STRING(pev->target) );

	if(pTarget != NULL)
	{
		MESSAGE_BEGIN( MSG_ALL, gmsgAddRope,NULL);
		WRITE_BYTE(entindex());
		WRITE_BYTE( pTarget->entindex() );
		WRITE_STRING(STRING(pev->message) );
		MESSAGE_END();

		ALERT(at_console, "File %s/n",STRING(pev->message));
	}
	*/
}
void CPys_Rope::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{	
	if ( ShouldToggle( useType, pev->body ) )
		pev->body = !pev->body;

	CBaseEntity *pTarget = NULL;
	pTarget = UTIL_FindEntityByTargetname( pTarget, STRING(pev->target) );

	if(pTarget != NULL)
	{
		MESSAGE_BEGIN( MSG_ALL, gmsgAddRope,NULL);
		WRITE_BYTE(entindex());
		WRITE_BYTE( pTarget->entindex() );
		WRITE_STRING(STRING(pev->message) );
		MESSAGE_END();

//		ALERT(at_console, "File %s/n",STRING(pev->message));
	}
}
