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

/**

  CHANGES ON THIS FILE:
  
	NONE

***/
/*

===== h_cycler.cpp ========================================================

  The Halflife Cycler Monsters

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "animation.h"
#include "weapons.h"
#include "player.h"


#define TEMP_FOR_SCREEN_SHOTS
#ifdef TEMP_FOR_SCREEN_SHOTS //===================================================

class CCycler : public CBaseMonster
{
public:
	void GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax);
	void GenericCineEntitySpawn(char *szModel, Vector vecMin, Vector vecMax);//used for cine_entity
	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() | FCAP_IMPULSE_USE); }
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void Spawn( void );
	void Think( void );
	//void Pain( float flDamage );
	void Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// Don't treat as a live target
	virtual BOOL IsAlive( void ) { return FALSE; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int			m_animate;
	
	BOOL bIsPlaying;
};

TYPEDESCRIPTION	CCycler::m_SaveData[] = 
{
	DEFINE_FIELD( CCycler, m_animate, FIELD_INTEGER ),
	DEFINE_FIELD( CCycler, bIsPlaying, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CCycler, CBaseMonster );


//
// we should get rid of all the other cyclers and replace them with this.
//
class CGenericCycler : public CCycler
{
public:
	void Spawn( void ) { GenericCyclerSpawn( (char *)STRING(pev->model), Vector(-16, -16, 0), Vector(16, 16, 72) ); }
};
LINK_ENTITY_TO_CLASS( cycler, CGenericCycler );



// Probe droid imported for tech demo compatibility
//
// PROBE DROID
//
class CCyclerProbe : public CCycler
{
public:	
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS( cycler_prdroid, CCyclerProbe );
void CCyclerProbe :: Spawn( void )
{
	pev->origin = pev->origin + Vector ( 0, 0, 16 );
	GenericCyclerSpawn( "models/prdroid.mdl", Vector(-16,-16,-16), Vector(16,16,16));
}



// Cycler member functions

void CCycler :: GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax)
{
	if (!szModel || !*szModel)
	{
		ALERT(at_error, "cycler at %.0f %.0f %0.f missing modelname", pev->origin.x, pev->origin.y, pev->origin.z );
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	pev->classname		= MAKE_STRING("cycler");
	PRECACHE_MODEL( szModel );
	SET_MODEL(ENT(pev),	szModel);

	CCycler::Spawn( );

	UTIL_SetSize(pev, vecMin, vecMax);
}

void CCycler :: GenericCineEntitySpawn(char *szModel, Vector vecMin, Vector vecMax)
{
	if (!szModel || !*szModel)
	{
		ALERT(at_error, "GenericCineEntitySpawn at %.0f %.0f %0.f missing modelname", pev->origin.x, pev->origin.y, pev->origin.z );
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	pev->classname		= MAKE_STRING("cine_entity");
	PRECACHE_MODEL( szModel );
	SET_MODEL(ENT(pev),	szModel);

	CCycler::Spawn( );

	UTIL_SetSize(pev, vecMin, vecMax);
}

void CCycler :: Spawn( )
{
	InitBoneControllers();
	pev->solid			= SOLID_SLIDEBOX;
	
	pev->movetype		= MOVETYPE_NONE;

	pev->takedamage		= DAMAGE_YES;
	pev->effects		= 0;
	pev->health			= 80000;// no cycler should die
	pev->yaw_speed		= 5;
	pev->ideal_yaw		= pev->angles.y;
	ChangeYaw( 360 );
	
	m_flFrameRate		= 75;
	m_flGroundSpeed		= 0;

	pev->nextthink		+= 1.0;

	ResetSequenceInfo( );

	if (pev->sequence != 0 || pev->frame != 0)
	{
		m_animate = 0;
		pev->framerate = 0;
	}
	else
	{
		m_animate = 1;
	}

	if ( FClassnameIs ( pev, "cine_entity" ) )
	{
		m_bloodColor = DONT_BLEED;

		pev->origin.z += 1;
		DROP_TO_FLOOR ( ENT(pev) );
	}
}


//
// cycler think
//
void CCycler :: Think( void )
{
	pev->nextthink = gpGlobals->time + 0.1;
	
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

	if ( FClassnameIs ( pev, "info_commentary" ) )
	{
		int iAllowCommentary = CVAR_GET_FLOAT( "commentary" );

		if (iAllowCommentary >= 1)
		{
//			if ( pPlayer )//only if we have a player, jo jo
//			pPlayer->g_bGodMode = TRUE;
		}
		else
		{
//			if ( pPlayer )//only if we have a player, jo jo
//			pPlayer->g_bGodMode = FALSE;

			//stop previous sound
			if (pev->message)
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/null.wav", 0.1, ATTN_NORM );

			UTIL_Remove( this );
		}

		if ( pPlayer )//only if we have a player, jo jo
		if ( pPlayer->g_bPlayerPointCommentary == TRUE)
		{
			if (pev->skin == 1)
			pev->framerate = 2.0;
			else
			pev->framerate = 1.0;

//			ALERT(at_console, "g_bPlayerPointCommentary == TRUE\n");
		}
		else
		{
			pev->animtime = gpGlobals->time;
			pev->framerate = 0.0;
			m_fSequenceFinished = FALSE;
					
//			ALERT(at_console, "g_bPlayerPointCommentary == FALSE\n");
		}
	}
	else
	{
		if (m_animate)
		{
			StudioFrameAdvance ( );
		}
		if (m_fSequenceFinished && !m_fSequenceLoops)
		{
			// ResetSequenceInfo();
			// hack to avoid reloading model every frame
			pev->animtime = gpGlobals->time;
			pev->framerate = 1.0;
			m_fSequenceFinished = FALSE;
			m_flLastEventCheck = gpGlobals->time;
			pev->frame = 0;
			if (!m_animate)
				pev->framerate = 0.0;	// FIX: don't reset framerate
		}
	}
}

//
// CyclerUse - starts a rotation trend
//
void CCycler :: Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( FClassnameIs ( pev, "info_commentary" ) )
	{
		pev->skin = 1;

		//stop previous sound
		if (pev->message)
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/null.wav", 0.1, ATTN_NORM );
			
		//stop previous message (BUGBUG: This doesn't clear the message)
		UTIL_ShowMessageAll( STRING(ALLOC_STRING("BLANK")));
		
		//play it everywhere
		if (pev->message)				
		EMIT_SOUND(ENT(pev), CHAN_ITEM, STRING(pev->message), 1, ATTN_NORM );

		//send message
		UTIL_ShowMessageAll( STRING(pev->message) );	//strings code

		/*
		if(!bIsPlaying)
		{
			pev->skin = 1;
			bIsPlaying = TRUE;
					
			//play it everywhere
			if (pev->message)				
			EMIT_SOUND(ENT(pev), CHAN_ITEM, STRING(pev->message), 1, ATTN_NORM );

			UTIL_ShowMessageAll( STRING(pev->message) );	//strings code
		}
		else
		{
			pev->skin = 0;
			bIsPlaying = FALSE;
						
		}
		*/
	}
	else
	{
		m_animate = !m_animate;
		if (m_animate)
			pev->framerate = 1.0;
		else
			pev->framerate = 0.0;
	}
}

//
// CyclerPain , changes sequences when shot
//
//void CCycler :: Pain( float flDamage )
int CCycler :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if ( FClassnameIs ( pev, "info_commentary" ) )
	return 0;

	if (m_animate)
	{
		pev->sequence++;

		ResetSequenceInfo( );

		if (m_flFrameRate == 0.0)
		{
			pev->sequence = 0;
			ResetSequenceInfo( );
		}
		pev->frame = 0;
	}
	else
	{
		pev->framerate = 1.0;
		StudioFrameAdvance ( 0.1 );
		pev->framerate = 0;
//		ALERT( at_console, "sequence: %d, frame %.0f\n", pev->sequence, pev->frame );
	}

	return 0;
}

#endif


class CCyclerSprite : public CBaseEntity
{
public:
	void Spawn( void );
	void Think( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() | FCAP_DONT_SAVE | FCAP_IMPULSE_USE); }
	virtual int	TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void	Animate( float frames );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	inline int		ShouldAnimate( void ) { return m_animate && m_maxFrame > 1.0; }
	int			m_animate;
	float		m_lastTime;
	float		m_maxFrame;
};

LINK_ENTITY_TO_CLASS( cycler_sprite, CCyclerSprite );

TYPEDESCRIPTION	CCyclerSprite::m_SaveData[] = 
{
	DEFINE_FIELD( CCyclerSprite, m_animate, FIELD_INTEGER ),
	DEFINE_FIELD( CCyclerSprite, m_lastTime, FIELD_TIME ),
	DEFINE_FIELD( CCyclerSprite, m_maxFrame, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CCyclerSprite, CBaseEntity );


void CCyclerSprite::Spawn( void )
{
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_NONE;
	pev->takedamage		= DAMAGE_YES;
	pev->effects		= 0;

	pev->frame			= 0;
	pev->nextthink		= gpGlobals->time + 0.1;
	m_animate			= 1;
	m_lastTime			= gpGlobals->time;

	PRECACHE_MODEL( (char *)STRING(pev->model) );
	SET_MODEL( ENT(pev), STRING(pev->model) );

	m_maxFrame = (float) MODEL_FRAMES( pev->modelindex ) - 1;
}


void CCyclerSprite::Think( void )
{
	if ( ShouldAnimate() )
		Animate( pev->framerate * (gpGlobals->time - m_lastTime) );

	pev->nextthink		= gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}


void CCyclerSprite::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_animate = !m_animate;
//	ALERT( at_console, "Sprite: %s\n", STRING(pev->model) );
}


int	CCyclerSprite::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if ( m_maxFrame > 1.0 )
	{
		Animate( 1.0 );
	}
	return 1;
}

void CCyclerSprite::Animate( float frames )
{ 
	pev->frame += frames;
	if ( m_maxFrame > 0 )
		pev->frame = fmod( pev->frame, m_maxFrame );
}







class CWeaponCycler : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	int iItemSlot( void ) { return 1; }
	int GetItemInfo(ItemInfo *p) {return 0; }

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iszModel;
	int m_iModel;
};
LINK_ENTITY_TO_CLASS( cycler_weapon, CWeaponCycler );


void CWeaponCycler::Spawn( )
{
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_NONE;

	PRECACHE_MODEL( (char *)STRING(pev->model) );
	SET_MODEL( ENT(pev), STRING(pev->model) );
	m_iszModel = pev->model;
	m_iModel = pev->modelindex;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch( DefaultTouch );
}



BOOL CWeaponCycler::Deploy( )
{
	m_pPlayer->pev->viewmodel = m_iszModel;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	SendWeaponAnim( 0 );
	m_iClip = 0;
	return TRUE;
}


void CWeaponCycler::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}


void CWeaponCycler::PrimaryAttack()
{

	SendWeaponAnim( pev->sequence );

	m_flNextPrimaryAttack = gpGlobals->time + 0.3;
}


void CWeaponCycler::SecondaryAttack( void )
{
	float flFrameRate, flGroundSpeed;

	pev->sequence = (pev->sequence + 1) % 8;

	pev->modelindex = m_iModel;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	GetSequenceInfo( pmodel, pev, &flFrameRate, &flGroundSpeed );
	pev->modelindex = 0;

	if (flFrameRate == 0.0)
	{
		pev->sequence = 0;
	}

	SendWeaponAnim( pev->sequence );

	m_flNextSecondaryAttack = gpGlobals->time + 0.3;
}



// Flaming Wreakage
class CWreckage : public CBaseMonster
{
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	void Think( void );

	int m_flStartTime;
};
TYPEDESCRIPTION	CWreckage::m_SaveData[] = 
{
	DEFINE_FIELD( CWreckage, m_flStartTime, FIELD_TIME ),
};
IMPLEMENT_SAVERESTORE( CWreckage, CBaseMonster );


LINK_ENTITY_TO_CLASS( cycler_wreckage, CWreckage );

void CWreckage::Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	pev->takedamage		= 0;
	pev->effects		= 0;

	pev->frame			= 0;
	pev->nextthink		= gpGlobals->time + 0.1;

	if (pev->model)
	{
		PRECACHE_MODEL( (char *)STRING(pev->model) );
		SET_MODEL( ENT(pev), STRING(pev->model) );
	}
	// pev->scale = 5.0;

	m_flStartTime		= gpGlobals->time;
}

void CWreckage::Precache( )
{
	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING(pev->model) );
}

void CWreckage::Think( void )
{
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->dmgtime)
	{
		if (pev->dmgtime < gpGlobals->time)
		{
			UTIL_Remove( this );
			return;
		}
		else if (RANDOM_FLOAT( 0, pev->dmgtime - m_flStartTime ) > pev->dmgtime - gpGlobals->time)
		{
			return;
		}
	}
	
	Vector VecSrc;
	
	VecSrc.x = RANDOM_FLOAT( pev->absmin.x, pev->absmax.x );
	VecSrc.y = RANDOM_FLOAT( pev->absmin.y, pev->absmax.y );
	VecSrc.z = RANDOM_FLOAT( pev->absmin.z, pev->absmax.z );

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, VecSrc );
		WRITE_BYTE( TE_SMOKE );
		WRITE_COORD( VecSrc.x );
		WRITE_COORD( VecSrc.y );
		WRITE_COORD( VecSrc.z );
		WRITE_SHORT( g_sModelIndexSmoke );
		WRITE_BYTE( RANDOM_LONG(0,49) + 50 ); // scale * 10
		WRITE_BYTE( RANDOM_LONG(0, 3) + 8  ); // framerate
	MESSAGE_END();
}

class CCine01 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/zombie.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_zombie, CCine01 );

class CCine02 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/zombie_fast.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_zombie_fast, CCine02 );

class CCine03 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/zombie_poison.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_zombie_poison, CCine03 );

class CCine04 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/headcrab_fast.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_headcrab_fast, CCine04 );

class CCine05 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/headcrab_poison.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_headcrab_poison, CCine05 );



class CCine06 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/combine.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_combine, CCine06 );

class CCine07 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/combine_metrocop.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_combine_metrocop, CCine07 );

class CCine08 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hl2/combine_supersoldier.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_npc_combine_supersoldier, CCine08 );



class CCine09 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hgrunt.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_human_grunt, CCine09 );

class CCine10 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hgrunt_opfor.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_hgrunt_opfor, CCine10 );


class CCine11 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hgrunt_rpg.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_human_grunt_rpg, CCine11 );

class CCine12 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/turret.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_turret, CCine12 );

//
class CCine13 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/islave.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_alien_slave, CCine13 );

class CCine14 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/agrunt.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_alien_grunt, CCine14 );

class CCine15 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/zombie.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_zombie, CCine15 );

class CCine16 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/gargantua.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_gargantua, CCine16 );

class CCine17 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/houndeye.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_houndeye, CCine17 );

class CCine18 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/scientist.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_scientist, CCine18 );

class CCine19 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/hevsci.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_hevsci, CCine19 );

class CCine20 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/barney.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_barney, CCine20 );

class CCine21 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/otis.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_otis, CCine21 );

class CCine22 : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/urban.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_agency_member, CCine22 );

class CCineTerrorist : public CCycler
{	public:		void Spawn( void ){ GenericCineEntitySpawn( "models/human_ak.mdl", Vector(-16,-16,-16), Vector(16,16,16)); } };
LINK_ENTITY_TO_CLASS( cine_monster_terrorist, CCineTerrorist );


/*

class CCineEnt : public CCycler
{
public:	
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS( cine_entity, CCineEnt );

void CCineEnt :: Spawn( void )
{
	CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);
	
	if ( pPlayer )
	{
		if(pPlayer->g_iNextEntToSpawn == NPC_HUMAN_GRUNT)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_HUMAN_GRUNT_RPG)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_HUMAN_GRUNT_OPFOR)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));

		if(pPlayer->g_iNextEntToSpawn == NPC_ZOMBIE_HL)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_HEADCRAB_HL)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_GARGANTUA)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));

		if(pPlayer->g_iNextEntToSpawn == NPC_COMBINE)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_COMBINE_METROCOP)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_COMBINE_SUPERSOLDIER)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));

		if(pPlayer->g_iNextEntToSpawn == NPC_ZOMBIE)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_ZOMBIE_FAST)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_ZOMBIE_POISON)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));

		if(pPlayer->g_iNextEntToSpawn == NPC_TERRORIST)
		GenericCyclerSpawn( "models/human_ak.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_AGENCY_MEMBER)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_BARNEY)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));

		if(pPlayer->g_iNextEntToSpawn == NPC_OTIS)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_HEVSCI)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_SCIENTIST)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));

		if(pPlayer->g_iNextEntToSpawn == NPC_HOUNDEYE)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_ALIEN_GRUNT)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
		if(pPlayer->g_iNextEntToSpawn == NPC_ALIEN_SLAVE)
		GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
	}
}
*/


class CInfoCommentary : public CCycler
{
public:	
	void Spawn( void );
	void Precache( void );

	void GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax);
	int  BloodColor( void ) { return DONT_BLEED; }
};
LINK_ENTITY_TO_CLASS( info_commentary, CInfoCommentary );

void CInfoCommentary :: Spawn( void )
{
//	pev->origin = pev->origin + Vector ( 0, 0, 32 );
	GenericCyclerSpawn( "models/commentary.mdl", Vector(-16,-16,-16), Vector(16,16,16));
}

void CInfoCommentary::Precache( )
{
	//ignored on first call
	char* szSoundFile = (char*) STRING(pev->message);

	if ( !FStringNull( pev->message ) && strlen( szSoundFile ) > 1 )
	{
		PRECACHE_SOUND(szSoundFile);
	}
	//ignored on first call
}

void CInfoCommentary :: GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax)
{
	if (!szModel || !*szModel)
	{
		ALERT(at_error, "cycler at %.0f %.0f %0.f missing modelname", pev->origin.x, pev->origin.y, pev->origin.z );
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	//ignored on subsequent calls
	char* szSoundFile = (char*) STRING(pev->message);

	if ( !FStringNull( pev->message ) && strlen( szSoundFile ) > 1 )
	{
		PRECACHE_SOUND(szSoundFile);
	}
	//ignored on subsequent calls

	pev->classname		= MAKE_STRING("info_commentary");
	PRECACHE_MODEL( szModel );
	SET_MODEL(ENT(pev),	szModel);

	CCycler::Spawn( );

	UTIL_SetSize(pev, vecMin, vecMax);
}
