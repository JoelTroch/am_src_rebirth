/***
*
*		   같
*			 �    같같같�
*			� �   �  �  � 
*		   �   �  �  �  �
*		  �     � �  �  �
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

/**

  CHANGES ON THIS FILE:
  
+func_wall_detail (a normal func_wall whit multiple level of details)
*func_shine -CODE BY Laurie Cheers, modified by SysOp- (a entity that displays a sprite image at the
surface -modified to save/restore properly on Am's source code)
+func_collision -for sparks, metal, smoke puff effects-
***/

/*

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

#include "particle_defs.h"
extern int gmsgParticles;

extern DLL_GLOBAL Vector		g_vecAttackDir;

#define		SF_BRUSH_ACCDCC	16// brush should accelerate and decelerate when toggled
#define		SF_BRUSH_HURT		32// rotating brush that inflicts pain based on rotation speed
#define		SF_ROTATING_NOT_SOLID	64	// some special rotating objects are not solid.

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define		noiseStart		noise1
#define		noiseStop		noise2
#define		noiseRunning	noise3

#define		SF_PENDULUM_SWING		2	// spawnflag that makes a pendulum a rope swing.
//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//
Vector VecBModelOrigin( entvars_t* pevBModel )
{
	return pevBModel->absmin + ( pevBModel->size * 0.5 );
}

// =================== FUNC_WALL ==============================================

/*QUAKED func_wall (0 .5 .8) ?
This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// Bmodels don't go across transitions
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS( func_wall, CFuncWall );

void CFuncWall :: Spawn( void )
{
	pev->angles		= g_vecZero;
	pev->movetype	= MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid		= SOLID_BSP;
	SET_MODEL( ENT(pev), STRING(pev->model) );
	
	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}


void CFuncWall :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ShouldToggle( useType, (int)(pev->frame)) )
		pev->frame = 1 - pev->frame;
}


#define SF_WALL_START_OFF		0x0001
#define SF_DONTCOLLIDE			8

class CFuncWallToggle : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	TurnOff( void );
	void	TurnOn( void );
	BOOL	IsOn( void );
};

LINK_ENTITY_TO_CLASS( func_wall_toggle, CFuncWallToggle );

void CFuncWallToggle :: Spawn( void )
{
	CFuncWall::Spawn();
	if ( pev->spawnflags & SF_WALL_START_OFF )
		TurnOff();
}


void CFuncWallToggle :: TurnOff( void )
{
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


void CFuncWallToggle :: TurnOn( void )
{
	pev->solid = SOLID_BSP;
	pev->effects &= ~EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


BOOL CFuncWallToggle :: IsOn( void )
{
	if ( pev->solid == SOLID_NOT )
		return FALSE;
	return TRUE;
}


void CFuncWallToggle :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int status = IsOn();

	if ( ShouldToggle( useType, status ) )
	{
		if ( status )
			TurnOff();
		else
			TurnOn();
	}
}

//////////
//////////

class CFuncCollideWall : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	TurnOff( void );
	void	TurnOn( void );
	BOOL	IsOn( void );
	
	// Overridden takedamage
//	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	
	void Touch( CBaseEntity *pOther );
	void EXPORT Think( void );
};

LINK_ENTITY_TO_CLASS( func_collision, CFuncCollideWall );

void CFuncCollideWall :: Spawn( void )
{
	CFuncWall::Spawn();

	pev->takedamage = DAMAGE_YES;
	pev->health			= 9999999;//bit of hack...

	if ( pev->spawnflags & SF_WALL_START_OFF )
		TurnOff();
				
	SetThink (Think);
	pev->nextthink = gpGlobals->time + 0.01;
	SetTouch (Touch);
	pev->nextthink = gpGlobals->time + 0.01;
}


void CFuncCollideWall :: TurnOff( void )
{
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
	pev->takedamage = DAMAGE_NO;
	pev->health			= 0;
	UTIL_SetOrigin( pev, pev->origin );
}


void CFuncCollideWall :: TurnOn( void )
{
	pev->solid = SOLID_BSP;
	pev->effects &= ~EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


BOOL CFuncCollideWall :: IsOn( void )
{
	if ( pev->solid == SOLID_NOT )
		return FALSE;
	return TRUE;
}


void CFuncCollideWall :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int status = IsOn();

	if ( ShouldToggle( useType, status ) )
	{
		if ( status )
			TurnOff();
		else
			TurnOn();
	}
}
void CFuncCollideWall::Touch( CBaseEntity *pOther )
{
	if ( pOther && (pOther->IsPlayer() || (pOther->pev->flags & FL_MONSTER)) )
	{
		ALERT ( at_notice, "pOther && (pOther->IsPlayer() || (pOther->pev->flags & FL_MONSTER\n");
		
		if (pev->spawnflags & SF_DONTCOLLIDE)// not collidable
		{
			pev->solid = SOLID_NOT;
			pev->nextthink = gpGlobals->time + 0.01;

			return;
		}
	}
}
void CFuncCollideWall::Think( void )
{
	ALERT ( at_notice, "Think()\n");
	
	if (pev->spawnflags & SF_DONTCOLLIDE)// not collidable
	{
		pev->solid = SOLID_BSP;//send solidify and return
		pev->nextthink = gpGlobals->time + 0.01;

		return;
	}
}

int CFuncCollideWall :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	ALERT ( at_notice, "TakeDamage()\n");

	TraceResult trace = UTIL_GetGlobalTrace( );

//== 1) //metal
//== 5) //wood
//== 5) //water
//== -1/0) //not defined (default concrete)

	float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range
	BOOL	b_CanMakeParticles = TRUE;

	if ( CVAR_GET_FLOAT( "r_paintball" ) == 0 )
	if ( CVAR_GET_FLOAT( "cl_wallpuff" ) >= 1 )
	{			
		if (pev->frags == 1) //metal
		{								
			UTIL_Ricochet( trace.vecEndPos, 0.5 );

			switch ( RANDOM_LONG(0,2) )
			{
				case 0: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 1: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 2: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
			}

			if (RANDOM_LONG( 0, 99 ) < 40)
			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 0, 5, 500, 500 );//chispas

			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 9, 5, 5, 100 );//puntos
			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 0, 5, 500, 20 );//chispas
												
			b_CanMakeParticles = FALSE;
		}
		if (pev->frags == 5) //wood
		{
			switch ( RANDOM_LONG(0,2) )
			{
				case 0: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/wood1.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 1: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/wood2.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 2: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/wood3.wav", flVolume, ATTN_NORM, 0, 100); break;
			}
		}
		else if (pev->frags == 6) //water
		{
			switch ( RANDOM_LONG(0,2) )
			{
				case 0: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/water1.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 1: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/water2.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 2: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/water3.wav", flVolume, ATTN_NORM, 0, 100); break;
			}
		}
		else if (pev->frags == 7) //gas canister
		{
			switch ( RANDOM_LONG(0,1) )
			{
				case 0: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/gascan1.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 1: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/gascan2.wav", flVolume, ATTN_NORM, 0, 100); break;
			}
		}

		else
		{
			switch ( RANDOM_LONG(0,2) )
			{
				case 0: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 1: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
				case 2: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
			}
		}	
		
		if (b_CanMakeParticles)
		{
			if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
			{
				MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
					WRITE_SHORT(0);
					WRITE_BYTE(0);
					WRITE_COORD( trace.vecEndPos.x );
					WRITE_COORD( trace.vecEndPos.y );
					WRITE_COORD( trace.vecEndPos.z );
					WRITE_COORD( 0 );
					WRITE_COORD( 0 );
					WRITE_COORD( 0 );
					if (pev->frags == 5) //wood
					WRITE_SHORT(iDefaultHitWood1);
					else if (pev->frags == 6) //water
					WRITE_SHORT(iDefaultWaterSplash);
					else if (pev->frags == 7) //gas canister
					WRITE_SHORT(iDefaultGasCanister);
					else
					WRITE_SHORT(iDefaultWallSmoke);
				MESSAGE_END();
			}
		}
	}//eo cvar check
	return 1;
}

//////////
class CFuncWallDetail : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	TurnOff( void );
	void	TurnOn( void );
	BOOL	IsOn( void );

	//
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	//
		
	void KeyValue( KeyValueData *pkvd );
	void EXPORT Think( void );

	string_t m_iszWallLow;
	string_t m_iszWallNormal;
	string_t m_iszWallDetail; 
};

LINK_ENTITY_TO_CLASS( func_wall_detail, CFuncWallDetail );

TYPEDESCRIPTION CFuncWallDetail::m_SaveData[] =
{
	DEFINE_FIELD( CFuncWallDetail, m_iszWallLow, FIELD_STRING ),
	DEFINE_FIELD( CFuncWallDetail, m_iszWallNormal, FIELD_STRING ),//letras
	DEFINE_FIELD( CFuncWallDetail, m_iszWallDetail, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CFuncWallDetail, CFuncWall );

void CFuncWallDetail::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iszWallLow"))
	{
		m_iszWallLow = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszWallNormal"))
	{
		m_iszWallNormal = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszWallDetail"))
	{
		m_iszWallDetail = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CFuncWall::KeyValue( pkvd );
	}
}

void CFuncWallDetail :: Spawn( void )
{
	CFuncWall::Spawn();
	if ( pev->spawnflags & SF_WALL_START_OFF )
		TurnOff();
		
	UTIL_SetOrigin(pev, pev->origin);		// set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);

	SetThink (Think);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CFuncWallDetail::Think( void )
{
	//MAKE FAKE Level Of Detail
	CBaseEntity *pEntity = NULL;

    while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, 9999 )) != NULL) 
	{
		if ( pEntity->IsPlayer() ) 
		{
			float flDist = (pEntity->Center() - pev->origin).Length();
							
			ALERT ( at_console, "flDist is: %f\n",flDist );

			if ( flDist >= 800)
			{
				//ALERT ( at_console, "m_iszWallNormal\n" );
				FireTargets( STRING(m_iszWallLow), this, this, USE_ON, 0 );//LOW
				FireTargets( STRING(m_iszWallNormal), this, this, USE_OFF, 0 );
				FireTargets( STRING(m_iszWallDetail), this, this, USE_OFF, 0 );
			}
			else if ( flDist >= 400)
			{
				//ALERT ( at_console, "m_iszWallLow\n" );
				FireTargets( STRING(m_iszWallLow), this, this, USE_OFF, 0 );
				FireTargets( STRING(m_iszWallNormal), this, this, USE_ON, 0 );//NORMAL
				FireTargets( STRING(m_iszWallDetail), this, this, USE_OFF, 0 );
			}
			else//detallado
			{
				//ALERT ( at_console, "m_iszWallDetail\n" );
				FireTargets( STRING(m_iszWallLow), this, this, USE_OFF, 0 );
				FireTargets( STRING(m_iszWallNormal), this, this, USE_OFF, 0 );
				FireTargets( STRING(m_iszWallDetail), this, this, USE_ON, 0 );//DETAILED
			}	
		}
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

void CFuncWallDetail :: TurnOff( void )
{
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


void CFuncWallDetail :: TurnOn( void )
{
	pev->solid = SOLID_BSP;
	pev->effects &= ~EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


BOOL CFuncWallDetail :: IsOn( void )
{
	if ( pev->solid == SOLID_NOT )
		return FALSE;
	return TRUE;
}


void CFuncWallDetail :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int status = IsOn();

	if ( ShouldToggle( useType, status ) )
	{
		if ( status )
			TurnOff();
		else
			TurnOn();
	}
}
//////////
//////////
//////////

#define SF_CONVEYOR_VISUAL		0x0001
#define SF_CONVEYOR_NOTSOLID	0x0002

class CFuncConveyor : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	UpdateSpeed( float speed );
};

LINK_ENTITY_TO_CLASS( func_conveyor, CFuncConveyor );
void CFuncConveyor :: Spawn( void )
{
	SetMovedir( pev );
	CFuncWall::Spawn();

	if ( !(pev->spawnflags & SF_CONVEYOR_VISUAL) )
		SetBits( pev->flags, FL_CONVEYOR );

	// HACKHACK - This is to allow for some special effects
	if ( pev->spawnflags & SF_CONVEYOR_NOTSOLID )
	{
		pev->solid = SOLID_NOT;
		pev->skin = 0;		// Don't want the engine thinking we've got special contents on this brush
	}

	if ( pev->speed == 0 )
		pev->speed = 100;

	UpdateSpeed( pev->speed );
}


// HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream
void CFuncConveyor :: UpdateSpeed( float speed )
{
	// Encode it as an integer with 4 fractional bits
	int speedCode = (int)(fabs(speed) * 16.0);

	if ( speed < 0 )
		pev->rendercolor.x = 1;
	else
		pev->rendercolor.x = 0;

	pev->rendercolor.y = (speedCode >> 8);
	pev->rendercolor.z = (speedCode & 0xFF);
}


void CFuncConveyor :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
    pev->speed = -pev->speed;
	UpdateSpeed( pev->speed );
}



// =================== FUNC_ILLUSIONARY ==============================================


/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseToggle 
{
public:
	void Spawn( void );
	void EXPORT SloshTouch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS( func_illusionary, CFuncIllusionary );

void CFuncIllusionary :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CFuncIllusionary :: Spawn( void )
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_NONE;  
	pev->solid = SOLID_NOT;// always solid_not 
	SET_MODEL( ENT(pev), STRING(pev->model) );
	
	// I'd rather eat the network bandwidth of this than figure out how to save/restore
	// these entities after they have been moved to the client, or respawn them ala Quake
	// Perhaps we can do this in deathmatch only.
	//	MAKE_STATIC(ENT(pev));
}

// =================== FUNC_SHINE ==============================================
#define SF_SHINE_ACTIVE 1

class CFuncShine : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }//?

	void EXPORT ResumeThink( void );
	void EXPORT TurnOn( void );
	void EXPORT TurnOff( void );

	void SendData( void );

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	STATE GetState( void );
};


STATE CFuncShine::GetState( void )
{
	if (pev->spawnflags & SF_SHINE_ACTIVE)
	{
		return STATE_ON;
	}
	else
	{
		return STATE_OFF;
	}
}

void CFuncShine :: Spawn ( void )
{
	pev->solid = SOLID_NOT;// always solid_not 
	SET_MODEL( ENT(pev), STRING(pev->model) );
	pev->effects |= EF_NODRAW;

	if ( FStringNull( pev->targetname ))
	{
		pev->spawnflags |= SF_SHINE_ACTIVE;
		ALERT(at_console, "WARNING: CFuncShine :: Spawn, Can't find targetname.\n");
	}

	if (pev->spawnflags & SF_SHINE_ACTIVE)
	{
		ALERT (at_console, "CFuncShine Spawn in <ON> Mode\nNextthink in 0.5 Seconds\n");

		SetThink( TurnOn );
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

void CFuncShine :: Precache ( void )
{
	// not that we actually need to precache it here, but we do need to make sure it exists
	PRECACHE_MODEL( (char*)STRING(pev->message) );

	if (pev->spawnflags & SF_SHINE_ACTIVE)
	{
		SetThink(&CFuncShine :: ResumeThink );

		pev->nextthink = 0.1;
	}
}

void CFuncShine :: TurnOn ( void )
{
	ALERT(at_console, "CFuncShine :: TurnOn, Current Time: %f\n", gpGlobals->time);

	pev->spawnflags |= SF_SHINE_ACTIVE;

	SendData();
}

void CFuncShine :: TurnOff ( void )
{
	ALERT(at_console, "CFuncShine :: TurnOff\n");
	SendData();
	ALERT(at_console, "CFuncShine :: DontThink();\n");
	DontThink();
}

void CFuncShine :: ResumeThink ( void )
{
	ALERT(at_console, "CFuncShine :: ResumeThink, Current Time: %f\n", gpGlobals->time);
	SetThink(&CFuncShine ::TurnOff);

	pev->nextthink = 0.1;
}


void CFuncShine :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (ShouldToggle(useType))
	{
		if (pev->spawnflags & SF_SHINE_ACTIVE)
			TurnOff();
		else
			TurnOn();
	}
}
extern int gmsgAddShine;

void CFuncShine :: SendData ( )
{
	ALERT(at_console, "CFuncShine :: SendData\n");
	
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgAddShine, NULL);
		WRITE_BYTE(pev->scale);
		WRITE_BYTE(pev->renderamt);
		WRITE_COORD(pev->absmin.x + 2); // take off 2: mins values are padded, but we just want to hug the surface
		WRITE_COORD(pev->absmax.x - 2);
		WRITE_COORD(pev->absmin.y + 2);
		WRITE_COORD(pev->absmax.y - 2);
		WRITE_COORD(pev->absmin.z + 2);
		WRITE_STRING(STRING(pev->message));
	MESSAGE_END();
}

LINK_ENTITY_TO_CLASS( func_shine, CFuncShine );

// -------------------------------------------------------------------------------
//
// Monster only clip brush
// 
// This brush will be solid for any entity who has the FL_MONSTERCLIP flag set
// in pev->flags
//
// otherwise it will be invisible and not solid.  This can be used to keep 
// specific monsters out of certain areas
//
// -------------------------------------------------------------------------------
class CFuncMonsterClip : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) {}		// Clear out func_wall's use function
};

LINK_ENTITY_TO_CLASS( func_monsterclip, CFuncMonsterClip );

void CFuncMonsterClip::Spawn( void )
{
	CFuncWall::Spawn();
	if ( CVAR_GET_FLOAT("showtriggers") == 0 )
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}


// =================== FUNC_ROTATING ==============================================
class CFuncRotating : public CBaseEntity
{
public:
	// basic functions
	void Spawn( void  );
	void Precache( void  );
	void EXPORT SpinUp ( void );
	void EXPORT SpinDown ( void );
	void KeyValue( KeyValueData* pkvd);
	void EXPORT HurtTouch ( CBaseEntity *pOther );
	void EXPORT RotatingUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT Rotate( void );
	void RampPitchVol (int fUp );
	void Blocked( CBaseEntity *pOther );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flFanFriction;
	float m_flAttenuation;
	float m_flVolume;
	float m_pitch;
	int	  m_sounds;
};

TYPEDESCRIPTION	CFuncRotating::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncRotating, m_flFanFriction, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_flAttenuation, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_flVolume, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_pitch, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncRotating, m_sounds, FIELD_INTEGER )
};

IMPLEMENT_SAVERESTORE( CFuncRotating, CBaseEntity );


LINK_ENTITY_TO_CLASS( func_rotating, CFuncRotating );

void CFuncRotating :: KeyValue( KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "fanfriction"))
	{
		m_flFanFriction = atof(pkvd->szValue)/100;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "Volume"))
	{
		m_flVolume = atof(pkvd->szValue)/10.0;

		if (m_flVolume > 1.0)
			m_flVolume = 1.0;
		if (m_flVolume < 0.0)
			m_flVolume = 0.0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnorigin"))
	{
		Vector tmp;
		UTIL_StringToVector( (float *)tmp, pkvd->szValue );
		if ( tmp != g_vecZero )
			pev->origin = tmp;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseEntity::KeyValue( pkvd );
}

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS
You need to have an origin brush as part of this entity.  The  
center of that brush will be
the point around which it is rotated. It will rotate around the Z  
axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
*/


void CFuncRotating :: Spawn( )
{
	// set final pitch.  Must not be PITCH_NORM, since we
	// plan on pitch shifting later.

	m_pitch = PITCH_NORM - 1;

	// maintain compatibility with previous maps
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	// if the designer didn't set a sound attenuation, default to one.
	m_flAttenuation = ATTN_NORM;
	
	if ( FBitSet ( pev->spawnflags, SF_BRUSH_ROTATE_SMALLRADIUS) )
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if ( FBitSet ( pev->spawnflags, SF_BRUSH_ROTATE_MEDIUMRADIUS) )
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if ( FBitSet ( pev->spawnflags, SF_BRUSH_ROTATE_LARGERADIUS) )
	{
		m_flAttenuation = ATTN_NORM;
	}

	// prevent divide by zero if level designer forgets friction!
	if ( m_flFanFriction == 0 )
	{
		m_flFanFriction = 1;
	}
	
	if ( FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_Z_AXIS) )
		pev->movedir = Vector(0,0,1);
	else if ( FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_X_AXIS) )
		pev->movedir = Vector(1,0,0);
	else 
		pev->movedir = Vector(0,1,0);	// y-axis

	// check for reverse rotation
	if ( FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_BACKWARDS) )
		pev->movedir = pev->movedir * -1;

	// some rotating objects like fake volumetric lights will not be solid.
	if ( FBitSet(pev->spawnflags, SF_ROTATING_NOT_SOLID) )
	{
		pev->solid = SOLID_NOT;
		pev->skin = CONTENTS_EMPTY;
		pev->movetype	= MOVETYPE_PUSH;
	}
	else
	{
		pev->solid		= SOLID_BSP;
		pev->movetype	= MOVETYPE_PUSH;
	}

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL( ENT(pev), STRING(pev->model) );

	SetUse( RotatingUse );
	// did level designer forget to assign speed?
	if (pev->speed <= 0)
		pev->speed = 0;

	// Removed this per level designers request.  -- JAY
	//	if (pev->dmg == 0)
	//		pev->dmg = 2;

	// instant-use brush?
	if ( FBitSet( pev->spawnflags, SF_BRUSH_ROTATE_INSTANT) )
	{		
		SetThink( SUB_CallUseToggle );
		pev->nextthink = pev->ltime + 1.5;	// leave a magic delay for client to start up
	}	
	// can this brush inflict pain?
	if ( FBitSet (pev->spawnflags, SF_BRUSH_HURT) )
	{
		SetTouch( HurtTouch );
	}
	
	Precache( );
}


void CFuncRotating :: Precache( void )
{
	char* szSoundFile = (char*) STRING(pev->message);

	// set up fan sounds

	if (!FStringNull( pev->message ) && strlen( szSoundFile ) > 0)
	{
		// if a path is set for a wave, use it

		PRECACHE_SOUND(szSoundFile);
			
		pev->noiseRunning = ALLOC_STRING(szSoundFile);
	} else
	{
		// otherwise use preset sound
		switch (m_sounds)
		{
		case 1:
			PRECACHE_SOUND ("fans/fan1.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan1.wav");
			break;
		case 2:
			PRECACHE_SOUND ("fans/fan2.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan2.wav");
			break;
		case 3:
			PRECACHE_SOUND ("fans/fan3.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan3.wav");
			break;
		case 4:
			PRECACHE_SOUND ("fans/fan4.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan4.wav");
			break;
		case 5:
			PRECACHE_SOUND ("fans/fan5.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan5.wav");
			break;

		case 0:
		default:
			if (!FStringNull( pev->message ) && strlen( szSoundFile ) > 0)
			{
				PRECACHE_SOUND(szSoundFile);
				
				pev->noiseRunning = ALLOC_STRING(szSoundFile);
				break;
			} else
			{
				pev->noiseRunning = ALLOC_STRING("common/null.wav");
				break;
			}
		}
	}
	
	if (pev->avelocity != g_vecZero )
	{
		// if fan was spinning, and we went through transition or save/restore,
		// make sure we restart the sound.  1.5 sec delay is magic number. KDB

		SetThink ( SpinUp );
		pev->nextthink = pev->ltime + 1.5;
	}
}



//
// Touch - will hurt others based on how fast the brush is spinning
//
void CFuncRotating :: HurtTouch ( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	// we can't hurt this thing, so we're not concerned with it
	if ( !pevOther->takedamage )
		return;

	// calculate damage based on rotation speed
	pev->dmg = pev->avelocity.Length() / 10;

	pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH);
	
	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev) ).Normalize() * pev->dmg;
}

//
// RampPitchVol - ramp pitch and volume up to final values, based on difference
// between how fast we're going vs how fast we plan to go
//
#define FANPITCHMIN		30
#define FANPITCHMAX		100

void CFuncRotating :: RampPitchVol (int fUp)
{

	Vector vecAVel = pev->avelocity;
	vec_t vecCur;
	vec_t vecFinal;
	float fpct;
	float fvol;
	float fpitch;
	int pitch;
	
	// get current angular velocity

	vecCur = abs(vecAVel.x != 0 ? vecAVel.x : (vecAVel.y != 0 ? vecAVel.y : vecAVel.z));
	
	// get target angular velocity

	vecFinal = (pev->movedir.x != 0 ? pev->movedir.x : (pev->movedir.y != 0 ? pev->movedir.y : pev->movedir.z));
	vecFinal *= pev->speed;
	vecFinal = abs(vecFinal);

	// calc volume and pitch as % of final vol and pitch

	fpct = vecCur / vecFinal;
//	if (fUp)
//		fvol = m_flVolume * (0.5 + fpct/2.0); // spinup volume ramps up from 50% max vol
//	else
		fvol = m_flVolume * fpct;			  // slowdown volume ramps down to 0

	fpitch = FANPITCHMIN + (FANPITCHMAX - FANPITCHMIN) * fpct;	
	
	pitch = (int) fpitch;
	if (pitch == PITCH_NORM)
		pitch = PITCH_NORM-1;

	// change the fan's vol and pitch

	EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
		fvol, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, pitch);

}

//
// SpinUp - accelerates a non-moving func_rotating up to it's speed
//
void CFuncRotating :: SpinUp( void )
{
	Vector	vecAVel;//rotational velocity

	pev->nextthink = pev->ltime + 0.1;
	pev->avelocity = pev->avelocity + ( pev->movedir * ( pev->speed * m_flFanFriction ) );

	vecAVel = pev->avelocity;// cache entity's rotational velocity

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (	abs(vecAVel.x) >= abs(pev->movedir.x * pev->speed)	&&
			abs(vecAVel.y) >= abs(pev->movedir.y * pev->speed)	&&
			abs(vecAVel.z) >= abs(pev->movedir.z * pev->speed) )
	{
		pev->avelocity = pev->movedir * pev->speed;// set speed in case we overshot
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
			m_flVolume, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, FANPITCHMAX);
		
		SetThink( Rotate );
		Rotate();
	} 
	else
	{
		RampPitchVol(TRUE);
	}
}

//
// SpinDown - decelerates a moving func_rotating to a standstill.
//
void CFuncRotating :: SpinDown( void )
{
	Vector	vecAVel;//rotational velocity
	vec_t vecdir;

	pev->nextthink = pev->ltime + 0.1;

	pev->avelocity = pev->avelocity - ( pev->movedir * ( pev->speed * m_flFanFriction ) );//spin down slower than spinup

	vecAVel = pev->avelocity;// cache entity's rotational velocity

	if (pev->movedir.x != 0)
		vecdir = pev->movedir.x;
	else if (pev->movedir.y != 0)
		vecdir = pev->movedir.y;
	else
		vecdir = pev->movedir.z;

	// if we've met or exceeded target speed, set target speed and stop thinking
	// (note: must check for movedir > 0 or < 0)
	if (((vecdir > 0) && (vecAVel.x <= 0 && vecAVel.y <= 0 && vecAVel.z <= 0)) || 
		((vecdir < 0) && (vecAVel.x >= 0 && vecAVel.y >= 0 && vecAVel.z >= 0)))
	{
		pev->avelocity = g_vecZero;// set speed in case we overshot
		
		// stop sound, we're done
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning /* Stop */), 
				0, 0, SND_STOP, m_pitch);

		SetThink( Rotate );
		Rotate();
	} 
	else
	{
		RampPitchVol(FALSE);
	}
}

void CFuncRotating :: Rotate( void )
{
	pev->nextthink = pev->ltime + 10;
}

//=========================================================
// Rotating Use - when a rotating brush is triggered
//=========================================================
void CFuncRotating :: RotatingUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// is this a brush that should accelerate and decelerate when turned on/off (fan)?
	if ( FBitSet ( pev->spawnflags, SF_BRUSH_ACCDCC ) )
	{
		// fan is spinning, so stop it.
		if ( pev->avelocity != g_vecZero )
		{
			SetThink ( SpinDown );
			//EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop), 
			//	m_flVolume, m_flAttenuation, 0, m_pitch);

			pev->nextthink = pev->ltime + 0.1;
		}
		else// fan is not moving, so start it
		{
			SetThink ( SpinUp );
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
				0.01, m_flAttenuation, 0, FANPITCHMIN);

			pev->nextthink = pev->ltime + 0.1;
		}
	}
	else if ( !FBitSet ( pev->spawnflags, SF_BRUSH_ACCDCC ) )//this is a normal start/stop brush.
	{
		if ( pev->avelocity != g_vecZero )
		{
			// play stopping sound here
			SetThink ( SpinDown );

			// EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop), 
			//	m_flVolume, m_flAttenuation, 0, m_pitch);
			
			pev->nextthink = pev->ltime + 0.1;
			// pev->avelocity = g_vecZero;
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseRunning), 
				m_flVolume, m_flAttenuation, 0, FANPITCHMAX);
			pev->avelocity = pev->movedir * pev->speed;

			SetThink( Rotate );
			Rotate();
		}
	}
}


//
// RotatingBlocked - An entity has blocked the brush
//
void CFuncRotating :: Blocked( CBaseEntity *pOther )

{
	pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH);
}






//#endif


class CPendulum : public CBaseEntity
{
public:
	void	Spawn ( void );
	void	KeyValue( KeyValueData *pkvd );
	void	EXPORT Swing( void );
	void	EXPORT PendulumUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT Stop( void );
	void	Touch( CBaseEntity *pOther );
	void	EXPORT RopeTouch ( CBaseEntity *pOther );// this touch func makes the pendulum a rope
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	void	Blocked( CBaseEntity *pOther );

	static	TYPEDESCRIPTION m_SaveData[];
	
	float	m_accel;			// Acceleration
	float	m_distance;			// 
	float	m_time;
	float	m_damp;
	float	m_maxSpeed;
	float	m_dampSpeed;
	vec3_t	m_center;
	vec3_t	m_start;
};

LINK_ENTITY_TO_CLASS( func_pendulum, CPendulum );

TYPEDESCRIPTION	CPendulum::m_SaveData[] = 
{
	DEFINE_FIELD( CPendulum, m_accel, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_distance, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_time, FIELD_TIME ),
	DEFINE_FIELD( CPendulum, m_damp, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_maxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_dampSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPendulum, m_center, FIELD_VECTOR ),
	DEFINE_FIELD( CPendulum, m_start, FIELD_VECTOR ),
};

IMPLEMENT_SAVERESTORE( CPendulum, CBaseEntity );



void CPendulum :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_distance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damp"))
	{
		m_damp = atof(pkvd->szValue) * 0.001;
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseEntity::KeyValue( pkvd );
}


void CPendulum :: Spawn( void )
{
	// set the axis of rotation
	CBaseToggle :: AxisDir( pev );

	if ( FBitSet (pev->spawnflags, SF_DOOR_PASSABLE) )
		pev->solid		= SOLID_NOT;
	else
		pev->solid		= SOLID_BSP;
	pev->movetype	= MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model) );

	if ( m_distance == 0 )
		return;

	if (pev->speed == 0)
		pev->speed = 100;

	m_accel = (pev->speed * pev->speed) / (2 * fabs(m_distance));	// Calculate constant acceleration from speed and distance
	m_maxSpeed = pev->speed;
	m_start = pev->angles;
	m_center = pev->angles + (m_distance * 0.5) * pev->movedir;

	if ( FBitSet( pev->spawnflags, SF_BRUSH_ROTATE_INSTANT) )
	{		
		SetThink( SUB_CallUseToggle );
		pev->nextthink = gpGlobals->time + 0.1;
	}
	pev->speed = 0;
	SetUse( PendulumUse );

	if ( FBitSet( pev->spawnflags, SF_PENDULUM_SWING ) )
	{
		SetTouch ( RopeTouch );
	}
}


void CPendulum :: PendulumUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->speed )		// Pendulum is moving, stop it and auto-return if necessary
	{
		if ( FBitSet( pev->spawnflags, SF_PENDULUM_AUTO_RETURN ) )
		{		
			float	delta;

			delta = CBaseToggle :: AxisDelta( pev->spawnflags, pev->angles, m_start );

			pev->avelocity = m_maxSpeed * pev->movedir;
			pev->nextthink = pev->ltime + (delta / m_maxSpeed);
			SetThink( Stop );
		}
		else
		{
			pev->speed = 0;		// Dead stop
			SetThink( NULL );
			pev->avelocity = g_vecZero;
		}
	}
	else
	{
		pev->nextthink = pev->ltime + 0.1;		// Start the pendulum moving
		m_time = gpGlobals->time;		// Save time to calculate dt
		SetThink( Swing );
		m_dampSpeed = m_maxSpeed;
	}
}


void CPendulum :: Stop( void )
{
	pev->angles = m_start;
	pev->speed = 0;
	SetThink( NULL );
	pev->avelocity = g_vecZero;
}


void CPendulum::Blocked( CBaseEntity *pOther )
{
	m_time = gpGlobals->time;
}


void CPendulum :: Swing( void )
{
	float delta, dt;
	
	delta = CBaseToggle :: AxisDelta( pev->spawnflags, pev->angles, m_center );
	dt = gpGlobals->time - m_time;	// How much time has passed?
	m_time = gpGlobals->time;		// Remember the last time called

	if ( delta > 0 && m_accel > 0 )
		pev->speed -= m_accel * dt;	// Integrate velocity
	else 
		pev->speed += m_accel * dt;

	if ( pev->speed > m_maxSpeed )
		pev->speed = m_maxSpeed;
	else if ( pev->speed < -m_maxSpeed )
		pev->speed = -m_maxSpeed;
	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = pev->speed * pev->movedir;

	// Call this again
	pev->nextthink = pev->ltime + 0.1;

	if ( m_damp )
	{
		m_dampSpeed -= m_damp * m_dampSpeed * dt;
		if ( m_dampSpeed < 30.0 )
		{
			pev->angles = m_center;
			pev->speed = 0;
			SetThink( NULL );
			pev->avelocity = g_vecZero;
		}
		else if ( pev->speed > m_dampSpeed )
			pev->speed = m_dampSpeed;
		else if ( pev->speed < -m_dampSpeed )
			pev->speed = -m_dampSpeed;

	}
}


void CPendulum :: Touch ( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	if ( pev->dmg <= 0 )
		return;

	// we can't hurt this thing, so we're not concerned with it
	if ( !pevOther->takedamage )
		return;

	// calculate damage based on rotation speed
	float damage = pev->dmg * pev->speed * 0.01;

	if ( damage < 0 )
		damage = -damage;

	pOther->TakeDamage( pev, pev, damage, DMG_CRUSH );
	
	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev) ).Normalize() * damage;
}

void CPendulum :: RopeTouch ( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	if ( !pOther->IsPlayer() )
	{// not a player!
		ALERT ( at_console, "Not a client\n" );
		return;
	}

	if ( ENT(pevOther) == pev->enemy )
	{// this player already on the rope.
		return;
	}

	pev->enemy = pOther->edict();
	pevOther->velocity = g_vecZero;
	pevOther->movetype = MOVETYPE_NONE;
}


