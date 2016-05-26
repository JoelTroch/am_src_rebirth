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
  
+new particle effects
+new effects
+new grenade types

***/
/*

===== generic grenade.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"

#include "game.h"

#include "shake.h"
#include "math.h" //SP For for FB
#include "gamerules.h"//SP: para la comprobacion del MP

#include "particle_defs.h"//to use particle def

extern int gmsgParticles;//define external message


//===================grenade

//extern cvar_t mp_am_expdetail;

LINK_ENTITY_TO_CLASS( grenade, CGrenade );

// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_DETONATE		0x0001
#define EXP_SCALE		100
//
// Grenade Explode
//
void CGrenade::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.
void CGrenade::Explode( TraceResult *pTrace, int bitsDamageType )
{
	int iContents = UTIL_PointContents ( pev->origin );//moved up

	float		flRndSound;// sound randomizer

	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	pev->takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (EXP_SCALE/*pev->dmg*/ - 24) * 0.6);
	}

	//the first and common explosion
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		if (iContents != CONTENTS_WATER)
		{
			WRITE_SHORT( g_sModelIndexFireball );
		}
		else
		{
			WRITE_SHORT( g_sModelIndexWExplosion );
		}
		WRITE_BYTE( (EXP_SCALE/*pev->dmg*/ - 50) * .60  ); // scale * 10
		WRITE_BYTE( 35  ); // framerate
		WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();
	
	if ( CVAR_GET_FLOAT( "cl_expdetail" ) != 0 )
	{
		if (iContents != CONTENTS_WATER)
		{
			if (RANDOM_LONG( 0, 99 ) < 80)//100%now
			{
				MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
					WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
					WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
					WRITE_COORD( pev->origin.y );
					WRITE_COORD( pev->origin.z );
						WRITE_SHORT( g_sModelIndexFireball2 );
					WRITE_BYTE( (EXP_SCALE/*pev->dmg*/ - 80) * .60  ); // scale * 10 -50
					WRITE_BYTE( 35  ); // framerate
					WRITE_BYTE( TE_EXPLFLAG_NONE );
				MESSAGE_END();
			}

			if (RANDOM_LONG( 0, 99 ) < 30)
			{
				MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
					WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
					WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
					WRITE_COORD( pev->origin.y );
					WRITE_COORD( pev->origin.z );
						WRITE_SHORT( g_sModelIndexFireball3 );
					WRITE_BYTE( (EXP_SCALE/*pev->dmg*/ - 50) * .60  ); // scale * 10 -50
					WRITE_BYTE( 35  ); // framerate 15
					WRITE_BYTE( TE_EXPLFLAG_NONE );
				MESSAGE_END();
			}

			if ( CVAR_GET_FLOAT( "cl_expdetail" ) == 1 )
			{
				CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", pev->origin, TRUE );

				pSprite->SetTransparency( kRenderTransAlpha, 50, 50, 50, 255, kRenderFxNone );
				pSprite->SetScale( 5.0 );
				pSprite->Expand( RANDOM_FLOAT( 10, 15 ), RANDOM_FLOAT( 18.0, 26.0 )  );//expand


				pSprite->pev->avelocity.x = RANDOM_FLOAT( -50, 50 );//rotar?
				pSprite->pev->avelocity.y = RANDOM_FLOAT( -50, 50 );
				pSprite->pev->avelocity.z = RANDOM_FLOAT( -50, 50 );

				pSprite->pev->velocity.z = RANDOM_FLOAT( 10, 30 );
				pSprite->pev->velocity.x = RANDOM_FLOAT( -10, 10 );
				pSprite->pev->velocity.y = RANDOM_FLOAT( -10, 10 );
			}

			TraceResult tr;
			tr.vecEndPos = pev->origin;
			tr.vecPlaneNormal = -1 * pev->velocity.Normalize();
		}

		UTIL_ScreenShake( pev->origin, 12.0, 100.0, 2.0, 1000 );

		Vector vecSpot;// shard origin
		Vector vecVelocity;// shard velocity
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->m_bloodColor = DONT_BLEED;
		pGib->pev->origin = pev->origin;
		pGib->pev->velocity = UTIL_RandomBloodVector() * RANDOM_FLOAT( 300, 500 );

		vecSpot = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.

		UTIL_RandomBloodVector() * RANDOM_FLOAT( 300, 500 );

		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSpot );
			WRITE_BYTE( TE_BREAKMODEL);	
			// position
			WRITE_COORD( vecSpot.x );
			WRITE_COORD( vecSpot.y );
			WRITE_COORD( vecSpot.z );		

			WRITE_COORD( pev->size.x);
			WRITE_COORD( pev->size.y);
			WRITE_COORD( pev->size.z);

			WRITE_COORD( RANDOM_FLOAT ( 10, 30 ) );
			WRITE_COORD( RANDOM_FLOAT ( 10, 30 ) );
			WRITE_COORD( RANDOM_FLOAT ( 10, 30 ) );

			WRITE_BYTE( RANDOM_FLOAT ( 15, 30 ) ); 
			WRITE_SHORT( g_sGrenadeGib );	//model id# //puede dar error esto!
			WRITE_BYTE( 35 ); //2
			WRITE_BYTE( 15 );// 1 seconds
			WRITE_BYTE( BREAK_CONCRETE );//no?
		MESSAGE_END();

		if ( CVAR_GET_FLOAT( "cl_expdetail" ) == 2 )
		{
			// create explosion particle system
			if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
			{
				MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
					WRITE_SHORT(0);
					WRITE_BYTE(0);
					WRITE_COORD( pev->origin.x );
					WRITE_COORD( pev->origin.y );
					WRITE_COORD( pev->origin.z );
					WRITE_COORD( 0 );
					WRITE_COORD( 0 );
					WRITE_COORD( 0 );
					WRITE_SHORT(iDefaultExplosion);
				MESSAGE_END();
			}
		}

	}

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );
	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // can't traceline attack owner if this is set

	RadiusDamage ( pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType );
/*
	if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
	{
		//smoke the floor
		Vector		vecEndPos;
		TraceResult tr;

		vecEndPos = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
		UTIL_TraceLine ( vecEndPos, vecEndPos + Vector ( 0, 0, -64 ),  ignore_monsters, ENT(pev), &tr);

		if (tr.flFraction != 1.0) // != 1.0)// Hemos tocado algo
		{
			MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
				WRITE_SHORT(0);
				WRITE_BYTE(0);
				WRITE_COORD( tr.vecEndPos.x );
				WRITE_COORD( tr.vecEndPos.y );
				WRITE_COORD( tr.vecEndPos.z );
				WRITE_COORD( 0 );
				WRITE_COORD( 0 );
				WRITE_COORD( 0 );
				WRITE_SHORT(iDefaultScorch);
			MESSAGE_END();
		}
	}
	else
	{*/
		if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
		{
			UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
		}
		else
		{
			UTIL_DecalTrace( pTrace, DECAL_SCORCH2 );
		}
//	}

	flRndSound = RANDOM_FLOAT( 0 , 1 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}

	pev->effects |= EF_NODRAW;
	SetThink( Smoke );
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.3;
}


void CGrenade::Smoke( void ) 
{
  if (UTIL_PointContents ( pev->origin ) == CONTENTS_WATER) 
  {
    UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
  } 
  else 
  {

//	  if ( CVAR_GET_FLOAT( "cl_expdetail" ) != 1 )
//	  {//si esta desactivado
	
		//	 ALERT( at_console, "# Iniciando MESSAGE... \n");
#if SYS	  
			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
				WRITE_BYTE( TE_SMOKE );
				WRITE_COORD( pev->origin.x );
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );
				WRITE_SHORT( g_sModelIndexSmoke );
				if (m_bIsFlashbang) 
				{
		//			ALERT( at_console, "> Es FlashBang... \n");

					WRITE_BYTE( 80 );
				} 
				else 
				{
					WRITE_BYTE( (EXP_SCALE/*pev->dmg*/ - 50) * 0.80 ); // scale * 10
				}
				WRITE_BYTE( 6  ); // framerate
			MESSAGE_END();
#endif
//		}

/*
	CBaseEntity *pSmoker = CBaseEntity::Create( "env_smoker", pev->origin, g_vecZero, NULL );

	switch (RANDOM_LONG(0,1))
	{
		case 0: pSmoker->pev->health = 15;	//
			break;
		case 1: pSmoker->pev->health = 55;	//
			break;
		case 2: pSmoker->pev->health = 5;	//
			break;
	}
	pSmoker->pev->scale = 35;	// 4.6X normal size
	pSmoker->pev->dmg = 15;		// 0 radial distribution
	pSmoker->pev->nextthink = gpGlobals->time + 0.1;	// Start in 2.5 seconds
 */ 
	}

//  ALERT( at_console, "# Terminando MESSAGE y removiendo... \n");
  
  UTIL_Remove( this );
//  ALERT( at_console, "# Creacion completa! \n");
}

void CGrenade::Killed( entvars_t *pevAttacker, int iGib )
{
	//sys test
//	pev->takedamage = DAMAGE_NO;

	Detonate( );
}


// Timed grenade, this think is called when time runs out.
void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_bIsSemtex = TRUE;
	SetThink( Detonate );
	pev->nextthink = gpGlobals->time;
}

void CGrenade::PreDetonate( void )
{
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

	SetThink( Detonate );
	pev->nextthink = gpGlobals->time + 1;
}


void CGrenade::Detonate( void ) 
{
	TraceResult tr;
	Vector vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

	if ( m_bIsFlashbang == TRUE ) 
	{
		Flash( &tr );
	} 
	else if ( m_bIsSmoke == TRUE ) 
	{
		SmokeGren( &tr );
	}	
	else if ( m_bIsSfera == TRUE ) 
	{
		SferaExplode( &tr );
	}	
	else if ( m_bIsSemtex == TRUE ) 
	{
		ALERT( at_console, "m_bIsSemtex REGISTERED\n");

		Explode( &tr, DMG_CRUSH );
	}
	else
	{
		Explode( &tr, DMG_BLAST ); //despues cambiar esto a CONCUSS
	}
}


//
// Contact grenade, explode when it touches something
// 
void CGrenade::ExplodeTouch( CBaseEntity *pOther )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	pev->enemy = pOther->edict();

	vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr );

	Explode( &tr, DMG_BLAST );
}


void CGrenade::DangerSoundThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length( ), 0.2 );
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
	}
}


void CGrenade::BounceTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// only do damage if we're moving fairly fast
	if (m_flNextAttack < gpGlobals->time && pev->velocity.Length() > 100)
	{
		entvars_t *pevOwner = VARS( pev->owner );
		if (pevOwner)
		{
			if ( FClassnameIs(pev, "combine_ball") )
			{
				TraceResult tr = UTIL_GetGlobalTrace( );
				ClearMultiDamage( );
				pOther->TraceAttack(pevOwner, 100, gpGlobals->v_forward, &tr, DMG_CLUB ); 
				ApplyMultiDamage( pev, pevOwner);
			}
			else
			{
				TraceResult tr = UTIL_GetGlobalTrace( );
				ClearMultiDamage( );
				pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB ); 
				ApplyMultiDamage( pev, pevOwner);
			}
		}
		m_flNextAttack = gpGlobals->time + 1.0; // debounce
	}

	Vector vecTestVelocity;
	// pev->avelocity = Vector (300, 300, 300);

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity. 
	// trimming the Z velocity a bit seems to help quite a bit.
	vecTestVelocity = pev->velocity; 
	vecTestVelocity.z *= 0.45;

	if ( !m_fRegisteredSound && vecTestVelocity.Length() <= 60 )
	{
//		ALERT( at_console, "Grenade Registered!: %f\n", vecTestVelocity.Length() );

		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving. 
		// go ahead and emit the danger sound.
		
		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4, 0.3 );
		m_fRegisteredSound = TRUE;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.8;

		pev->sequence = RANDOM_LONG( 1, 1 );
	}
	else
	{
		// play bounce sound
		if(m_bIsSfera)
		SferaBounceSound();
		else
		BounceSound();
	}
	pev->framerate = pev->velocity.Length() / 200.0;
	if (pev->framerate > 1.0)
		pev->framerate = 1;
	else if (pev->framerate < 0.5)
		pev->framerate = 0;

}



void CGrenade::SlideTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;

		if (pev->velocity.x != 0 || pev->velocity.y != 0)
		{
			// maintain sliding sound
		}
	}
	else
	{
		if(m_bIsSfera)
		SferaBounceSound();
		else
		BounceSound();
	}
}

void CGrenade :: BounceSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.5, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.5, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.5, ATTN_NORM);	break;
	}
/*
	MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
		WRITE_SHORT(0);
		WRITE_BYTE(0);
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( 0 );
		WRITE_COORD( 0 );
		WRITE_COORD( 0 );
		WRITE_SHORT(iDefaultDrop);
	MESSAGE_END();*/
}

void CGrenade :: SferaBounceSound( void )
{
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/energy_bounce1.wav", 0.85, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/energy_bounce2.wav", 0.85, ATTN_NORM);	break;
//	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/energy_bounce3.wav", 0.85, ATTN_NORM);	break;
	}
}

void CGrenade :: TumbleThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( Detonate );
	}
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}
		
	if (!(m_bIsCombineGrenade) ) 
	{
		if (!m_bIsSfera)
		{
			if (m_flNextAttack < gpGlobals->time)
			{
				CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", pev->origin, TRUE );
					
				pSprite->SetTransparency( kRenderTransAlpha, 150, 150, 150, 255, kRenderFxNone );
				pSprite->SetScale( RANDOM_FLOAT( 0.5, 1 ) );
				
				pSprite->Expand( 1, RANDOM_FLOAT( 125.0, 135.0 )  );//????????????
				pSprite->pev->frame = 0;

				m_flNextAttack = gpGlobals->time + 0.001; // debounce //1.0
			}
		}
		else
		{
			CSprite *pSprite = CSprite::SpriteCreate( "sprites/alt_fire.spr", pev->origin, FALSE );

			pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 155, kRenderFxNone );
			pSprite->SetScale( 0.2 );		
			pSprite->SetAttachment( edict(), 0 );
			pSprite->pev->frame = 0;
			pSprite->pev->framerate = 0;
			pSprite->Expand( 0.001, 333  );//150
		}
	}

	if ( m_bIsCombineGrenade ) 
	{
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/laserdot.spr", pev->origin, FALSE );

		pSprite->SetTransparency( kRenderTransAdd, 255, 0, 0, 155, kRenderFxNone );
		pSprite->SetScale( 0.3 );		
		pSprite->SetAttachment( edict(), 1 );
		//pSprite->AnimateAndDie( 10 );
		pSprite->Expand( 0.001, 333  );//150


		if (b_iNextPipSound < gpGlobals->time)
		{
			EMIT_SOUND(ENT(pev), CHAN_STATIC, "weapons/tick1.wav", 0.9, ATTN_NORM);

			b_iNextPipSound = gpGlobals->time + 0.5;//0.5
		}
	}
	
	if (m_bIsSfera)
	{
	//	pev->effects |= EF_LIGHT;
		pev->velocity = pev->velocity.Normalize() * 2000;
	//	pev->effects = EF_LIGHT;
			
		UTIL_ScreenShake( pev->origin, 12.0, 50.0, 1.0, 1000 );
	}
}


void CGrenade:: Spawn( void )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->classname = MAKE_STRING( "grenade" );
		
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/grenade.mdl");

	UTIL_SetSize(pev, Vector( -16, -16, 0), Vector(16, 16, 36));
	UTIL_SetOrigin( pev, pev->origin );
		
	pev->flags |= FL_MONSTER;
	pev->takedamage		= DAMAGE_YES;
	pev->health			= 20;

	pev->dmg = 100;
	m_fRegisteredSound = FALSE;
}


CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	// contact grenades arc lower
	pGrenade->pev->gravity = 0.5;// lower gravity since grenade is aerodynamic and engine doesn't know it.
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles (pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	// make monsters afaid of it while in the air
	pGrenade->SetThink( DangerSoundThink );
	pGrenade->pev->nextthink = gpGlobals->time;
	
	// Tumble in air
	pGrenade->pev->avelocity.x = RANDOM_FLOAT ( -100, -500 );
	
	// Explode on contact
	pGrenade->SetTouch( ExplodeTouch );

	pGrenade->pev->dmg = gSkillData.plrDmgM203Grenade;

	return pGrenade;
}

CGrenade * CGrenade:: FlashShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( FlashTumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/grenade.mdl");
//	pGrenade->pev->dmg = 100;
	pGrenade->pev->dmg = 0;
	pGrenade->pev->classname = MAKE_STRING("flash_grenade"); // UGLY HACK!

	return pGrenade;
}

void CGrenade :: FlashTumbleThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( SmokeDetonate );//Detonate
	}

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}
	//sys test
	//ejejjeeem...... very funny...
	
	pev->effects |= EF_LIGHT;

//	pev->velocity = pev->velocity.Normalize() * 1000;
	pev->effects = EF_LIGHT;

	// Teh_Freak: World Lighting! TE_DLIGHT
		 MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			  WRITE_BYTE( TE_DLIGHT );
			  WRITE_COORD( pev->origin.x ); // origin
			  WRITE_COORD( pev->origin.y );
			  WRITE_COORD( pev->origin.z );
			  WRITE_BYTE( 20 );     // radius
			  WRITE_BYTE( 255 );     // R
			  WRITE_BYTE( 255 );     // G
			  WRITE_BYTE( 128 );     // B
			  WRITE_BYTE( 0 );     // life * 10
			  WRITE_BYTE( 0 ); // decay
		 MESSAGE_END();
		// Teh_Freak: World Lighting!

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_ELIGHT );
			WRITE_SHORT( entindex( ) + 0x3000 );		// entity, attachment
			WRITE_COORD( pev->origin.x );		// origin
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( 256 );	// radius
			WRITE_BYTE( 128 );	// R
			WRITE_BYTE( 128 );	// G
			WRITE_BYTE( 255 );	// B
			WRITE_BYTE( 0 );	// life * 10
			WRITE_COORD( 128 ); // decay
		MESSAGE_END();

		//asdasd
		
		if (m_flNextAttack < gpGlobals->time && pev->flags & FL_ONGROUND)
		{		
			if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
			{
				MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
					WRITE_SHORT(0);
					WRITE_BYTE(0);
					WRITE_COORD( pev->origin.x );
					WRITE_COORD( pev->origin.y );
					WRITE_COORD( pev->origin.z );
					WRITE_COORD( 0 );
					WRITE_COORD( 0 );
					WRITE_COORD( 0 );
					WRITE_SHORT(iDefaultBangalorSmoke);//iDefaultBangalorSmoke
				MESSAGE_END();
			}

			m_flNextAttack = gpGlobals->time + 0.1; // debounce //1.0
	}

}

CGrenade * CGrenade:: ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/weapons/handgrenade/w_grenade.mdl");
//	pGrenade->pev->dmg = 100;
	pGrenade->pev->dmg = gSkillData.plrDmgHandGrenade;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(pGrenade->entindex());	// entity
		WRITE_SHORT(g_sModelIndexLaser );	// model
		WRITE_BYTE( 5 ); // life
		WRITE_BYTE( 7 );  // width
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 111 );	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	return pGrenade;
}


CGrenade * CGrenade :: ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->pev->movetype = MOVETYPE_BOUNCE;
	pGrenade->pev->classname = MAKE_STRING( "grenade" );
	
	pGrenade->pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pGrenade->pev), "models/grenade.mdl");	// Change this to satchel charge model

	UTIL_SetSize(pGrenade->pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pGrenade->pev->dmg = 200;
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = g_vecZero;
	pGrenade->pev->owner = ENT(pevOwner);
	
	// Detonate in "time" seconds
	pGrenade->SetThink( SUB_DoNothing );
	pGrenade->SetUse( DetonateUse );
	pGrenade->SetTouch( SlideTouch );
	pGrenade->pev->spawnflags = SF_DETONATE;

	pGrenade->pev->friction = 0.9;

	//UGLY HACK!
//	pGrenade->m_bIsSemtex = TRUE;
//	pGrenade->m_bSemtexInWorld = TRUE;

	return pGrenade;
}



void CGrenade :: UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code )
{
	edict_t *pentFind;
	edict_t *pentOwner;

	if ( !pevOwner )
		return;

	CBaseEntity	*pOwner = CBaseEntity::Instance( pevOwner );

	pentOwner = pOwner->edict();

	pentFind = FIND_ENTITY_BY_CLASSNAME( NULL, "grenade" );
	while ( !FNullEnt( pentFind ) )
	{
		CBaseEntity *pEnt = Instance( pentFind );
		if ( pEnt )
		{
			if ( FBitSet( pEnt->pev->spawnflags, SF_DETONATE ) && pEnt->pev->owner == pentOwner )
			{
				if ( code == SATCHEL_DETONATE )
					pEnt->Use( pOwner, pOwner, USE_ON, 0 );
				else	// SATCHEL_RELEASE
					pEnt->pev->owner = NULL;
			}
		}
		pentFind = FIND_ENTITY_BY_CLASSNAME( pentFind, "grenade" );
	}
}
//This is almost unchanged ShootTimed except for the fact it calls an ClusterTumbleThink
CGrenade * CGrenade:: ShootTimedCluster( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
pGrenade->Spawn();
UTIL_SetOrigin( pGrenade->pev, vecStart );
pGrenade->pev->velocity = vecVelocity;
pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
pGrenade->pev->owner = ENT(pevOwner);

//pGrenade->SetTouch( ExplodeTouch );
//pGrenade->SetTouch( BounceTouch ); // Bounce if touched
	
// Explode on contact
pGrenade->SetTouch( ClusterDetonate );

// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

pGrenade->pev->dmgtime = gpGlobals->time + time;
pGrenade->SetThink( DangerSoundThink );

pGrenade->pev->nextthink = gpGlobals->time + 0.1;
if (time < 0.1)
{
pGrenade->pev->nextthink = gpGlobals->time;
pGrenade->pev->velocity = Vector( 0, 0, 0 );
}

pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
pGrenade->pev->framerate = 1.0;

// Tumble through the air
// pGrenade->pev->avelocity.x = -400;

pGrenade->pev->gravity = 0.5;
pGrenade->pev->friction = 0.8;

SET_MODEL(ENT(pGrenade->pev), "models/weapons/w_cluster.mdl");
//pGrenade->pev->dmg = 100;
	pGrenade->pev->dmg = gSkillData.plrDmgHandGrenade;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(pGrenade->entindex());	// entity
		WRITE_SHORT(g_sModelIndexLaser );	// model
		WRITE_BYTE( 5 ); // life
		WRITE_BYTE( 7 );  // width
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 111 );	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

return pGrenade;
}

//Now calls the ClusterDetonate routine instead of the Detonate one
void CGrenade :: ClusterTumbleThink( void )
{
	/*
if (!IsInWorld())
{
UTIL_Remove( this );
return;
}

StudioFrameAdvance( );
pev->nextthink = gpGlobals->time + 0.1;

if (pev->dmgtime - 1 < gpGlobals->time)
{
CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
}

if (pev->dmgtime <= gpGlobals->time)
{
SetThink( ClusterDetonate );
}
if (pev->waterlevel != 0)
{
pev->velocity = pev->velocity * 0.5;
pev->framerate = 0.2;
}
*/
}

//this make the fragments for the cluster
CGrenade * CGrenade:: ShootFragment( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( BounceTouch );	// Bounce if touched
	
	//pGrenade->FragGibs ();
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.6;
	pGrenade->pev->friction = 0.9;

	SET_MODEL(ENT(pGrenade->pev), "models/weapons/w_frag.mdl");
	pGrenade->pev->dmg = 100;
	//pGrenade->pev->dmg = gSkillData.plrDmgHandGrenade;
	return pGrenade;
}


//This one launches the cluster fragments
void CGrenade::ClusterDetonate( CBaseEntity *pOther )
{
TraceResult tr;
Vector vecSpot;// trace starts here!
//pGrenade->pev->owner = ENT(pevOwner);

vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ), ignore_monsters, ENT(pev), & tr);


Explode( &tr, DMG_BLAST );

//Launch 6 grenades at random angles
CGrenade::ShootFragment( pev, pev->origin, Vector(RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200))*RANDOM_LONG( 5, 10), RANDOM_FLOAT( 1.00, 5.00));
CGrenade::ShootFragment( pev, pev->origin, Vector(RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200))*RANDOM_LONG( 5, 10), RANDOM_FLOAT( 1.00, 5.00));
CGrenade::ShootFragment( pev, pev->origin, Vector(RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200))*RANDOM_LONG( 5, 10), RANDOM_FLOAT( 1.00, 5.00));
CGrenade::ShootFragment( pev, pev->origin, Vector(RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200))*RANDOM_LONG( 5, 10), RANDOM_FLOAT( 1.00, 5.00));
CGrenade::ShootFragment( pev, pev->origin, Vector(RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200))*RANDOM_LONG( 5, 10), RANDOM_FLOAT( 1.00, 5.00));
CGrenade::ShootFragment( pev, pev->origin, Vector(RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200), RANDOM_LONG( -200, 200))*RANDOM_LONG( 5, 10), RANDOM_FLOAT( 1.00, 5.00));
}


CGrenade * CGrenade::ShootFlashbang( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time ) 
{
//  ALERT( at_console, "#Creando granada FlashBang... \n");
  CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
  pGrenade->Spawn();
  UTIL_SetOrigin( pGrenade->pev, vecStart );
  pGrenade->pev->velocity = vecVelocity;
  pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
  pGrenade->pev->owner = ENT(pevOwner);
  // we'll use the handgrenade's bouncetouch
  pGrenade->SetTouch( BounceTouch );
  // this is when our flashbang is going to explode
  pGrenade->pev->dmgtime = gpGlobals->time + time;
  pGrenade->SetThink( TumbleThink );
  // just some cosmetic stuff here
  pGrenade->pev->avelocity.y = RANDOM_FLOAT ( -5, -20 );
  pGrenade->pev->nextthink = gpGlobals->time + 0.1;
  if (time < 0.1) {
    pGrenade->pev->nextthink = gpGlobals->time;
    pGrenade->pev->velocity = Vector( 0, 0, 0 );
  }
  // sets the model - PLEASE don't email me asking "where's w_flashbang.mdl?"
//     ALERT( at_console, "> Seteando Model -models/weapons/w_flashbang-... \n");
  SET_MODEL(ENT(pGrenade->pev), "models/weapons/w_flashbang.mdl");
  if ( RANDOM_LONG( 1, 2 ) == 1 ) {
    pGrenade->pev->sequence = 2;
  } else {
    pGrenade->pev->sequence = 3;
  }
  pGrenade->pev->animtime = gpGlobals->time;
  pGrenade->pev->framerate = 1.0;
  pGrenade->pev->gravity = 0.55;
  pGrenade->pev->friction = 0.6;
  // important - if this is false, the flashbang explodes like a normal grenade.
  pGrenade->m_bIsFlashbang = TRUE;
  pGrenade->pev->dmg = 0;
  return pGrenade;
}

void CGrenade::Flash( TraceResult *pTrace ) 
{
//  ALERT( at_console, "#Iniciando Flash... \n");

  pev->model = iStringNull;
  pev->solid = SOLID_NOT;
  pev->takedamage = DAMAGE_NO;
  if ( pTrace->flFraction != 1.0 ) 
  {
    pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * 100 * 0.1);
  }
  // flash, apparently, doesn't work underwater.
  int iContents = UTIL_PointContents ( pev->origin );
  if (iContents != CONTENTS_WATER) 
  {
//	 ALERT( at_console, "> No esta en agua... \n");
    // replace this with YOUR flash-sound. you'll also want to precache the sound in weapons.cpp.
//    	 ALERT( at_console, "> Creando sonidos y decals... \n");
	 EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/bang.wav", 0.9, ATTN_NORM);
    // cool decal
    UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
    CBaseEntity *pEntity = NULL;
    // we'll loop thru all entities 2000 units or less away from us
    while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, 2000 )) != NULL) 
	{
      // only affect players, duh
      if ( pEntity->IsPlayer() ) 
	  {
        // get the distance between the flash & the player
        float flDist = (pEntity->Center() - pev->origin).Length();
        // we'll only fade the player's view if he saw the flash
        if ( FVisible( pEntity ) ) 
		{
          // you can mess with these values if you're not happy with the original durations
          float flFadeTime = 150000 / powf(flDist, 2);
          // we'll use 30 as the maximum limit - else the player's view won't fade back in hours if he stares at the grenade from short distances.
          // even 30 is kind of high, you'll probably want to lower this.
          if (flFadeTime > 20) 
		  { 
			  flFadeTime = 20; 
			 // SERVER_COMMAND("volume 0 \n");
		  }
          else if (flFadeTime <= 2.0) 
		  { 
			  flFadeTime = 2.0; 
			//  SERVER_COMMAND("volume 0.1 \n");
			//  ALERT( at_console, "> Sonido Desactivado \n");
		  }
//		  ALERT( at_console, "> Disparando UTIL_ScreenFade... \n");
          UTIL_ScreenFade( pEntity, Vector(255,255,255), flFadeTime, (flFadeTime / 4), 255, FFADE_IN );
          
		  if ( g_pGameRules->IsMultiplayer() )
		  {
			  //hola
		  }
		  else
		  {
			 if (flFadeTime < 3) { SERVER_COMMAND("volume 0.4 \n"); }
			 else if (flFadeTime >= 3 && flFadeTime < 4) { SERVER_COMMAND("volume 0.3 \n"); }
			 else if (flFadeTime >= 4 && flFadeTime < 5.5) { SERVER_COMMAND("volume 0.2 \n"); }
			 else if (flFadeTime >= 5.5 && flFadeTime < 12) { SERVER_COMMAND("volume 0.1 \n"); }
			 else if (flFadeTime >= 12) { SERVER_COMMAND("volume 0 \n"); }
		  }

		  // now the fun part - concussion!
          // they didn't have this in CS ;D - but I think it really adds to the effect.
         /*
		  extern int gmsgConcussion;//SION

          MESSAGE_BEGIN( MSG_ONE, gmsgConcussion, NULL, pEntity->pev );
          if (flFadeTime < 3) { WRITE_BYTE( 20 ); }
          else if (flFadeTime >= 3 && flFadeTime < 4) { WRITE_BYTE( 34 ); }
          else if (flFadeTime >= 4 && flFadeTime < 5.5) { WRITE_BYTE( 50 ); }
          else if (flFadeTime >= 5.5 && flFadeTime < 12) { WRITE_BYTE( 72 ); }
          else if (flFadeTime >= 12) { WRITE_BYTE( 80 ); }
          MESSAGE_END();
		  */
        }
     }//si es player
    }

    // heavy sparkage!
//    ALERT( at_console, "> Creando -spark_shower-.. \n");
    int sparks = RANDOM_LONG(1,6);
    for ( int i = 0; i < sparks; i++ )
      Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
  }
  
		// Teh_Freak: World Lighting!
		 MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			  WRITE_BYTE( TE_DLIGHT );
			  WRITE_COORD( pev->origin.x ); // origin
			  WRITE_COORD( pev->origin.y );
			  WRITE_COORD( pev->origin.z );
			  WRITE_BYTE( 60 );     // radius
			  WRITE_BYTE( 255 );     // R
			  WRITE_BYTE( 255 );     // G
			  WRITE_BYTE( 255 );     // B
			  WRITE_BYTE( 2 );     // life * 10
			  WRITE_BYTE( 5 ); // decay
		 MESSAGE_END();
		// Teh_Freak: World Lighting!
		 ALERT( at_console, "> Luz creada \n");

  // don't draw the model after it has exploded.
  pev->effects |= EF_NODRAW;
  // emit some smoke, & remove self.
//  ALERT( at_console, "> Iniciando Humo... \n");
  SetThink( Smoke );
  pev->velocity = g_vecZero;
  pev->nextthink = gpGlobals->time + 0.3;

  SetThink( RestoreVol );
  pev->nextthink = gpGlobals->time + 5;
//  ALERT( at_console, "> Restaurando Sonido en 5 segs \n");

//  ALERT( at_console, "# Explosion de Flash con suceso! \n");
}

CGrenade * CGrenade:: ShootTimedCz( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/weapons/w_czgrenade.mdl");
//	pGrenade->pev->dmg = 100;
	pGrenade->pev->dmg = gSkillData.plrDmgHandGrenade;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(pGrenade->entindex());	// entity
		WRITE_SHORT(g_sModelIndexLaser );	// model
		WRITE_BYTE( 5 ); // life
		WRITE_BYTE( 7 );  // width
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 111 );	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	return pGrenade;
}

CGrenade * CGrenade:: ShootTimedCombine( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
	  if ( RANDOM_LONG( 1, 2 ) == 1 ) {
    pGrenade->pev->sequence = 2;
  } else {
    pGrenade->pev->sequence = 3;
  }	
//	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	pGrenade->m_bIsCombineGrenade = TRUE;//??????????????

	SET_MODEL(ENT(pGrenade->pev), "models/weapons/w_combine_he.mdl");
	pGrenade->pev->dmg = gSkillData.plrDmgHandGrenade;
		
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(pGrenade->entindex());	// entity
		WRITE_SHORT(g_sModelIndexLaser );	// model
		WRITE_BYTE( 4 ); // life
		WRITE_BYTE( 2 );  // width
		WRITE_BYTE( 222 );   // r, g, b
		WRITE_BYTE( 0 );   // r, g, b
		WRITE_BYTE( 0 );   // r, g, b
		WRITE_BYTE( 111 );	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	return pGrenade;
}

CGrenade * CGrenade:: ShootTimedPlayer( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, CBasePlayer *pPlayer )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( TumbleThink );
	pGrenade->Nextthink( 0.1 );
	if (time < 0.1)
	{
		pGrenade->Nextthink( 0.0 );
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	//******************
	 pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/weapons/handgrenade/w_grenade.mdl");
	pGrenade->pev->dmg = gSkillData.plrDmgHandGrenade;

	
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(pGrenade->entindex());	// entity
		WRITE_SHORT(g_sModelIndexLaser );	// model
		WRITE_BYTE( 5 ); // life
		WRITE_BYTE( 7 );  // width
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 150 );   // r, g, b
		WRITE_BYTE( 111 );	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

//pev->dmg = gSkillData.plrDmgHandGrenade;
	return pGrenade;
}

void CGrenade::RestoreVol( void )
{
 	  SERVER_COMMAND("volume 0.8\n");
//	  ALERT( at_console, "# Volume Restore \n");
}

CGrenade * CGrenade:: ShootC4( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	//new
	pGrenade->pev->movetype = MOVETYPE_NONE;//MOVETYPE_BOUNCE
	pGrenade->pev->solid = SOLID_NOT;//SOLID_BBOX

	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
//	pGrenade->SetTouch( BounceTouch );	// Bounce if touched
	pGrenade->SetTouch( SlideTouch );

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
//	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
//	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

//	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/weapons/semtex/w_c4.mdl");
//	pGrenade->pev->dmg = 100;
//	pGrenade->pev->dmg = 200;
	pGrenade->pev->dmg = 60;

	pGrenade->m_bIsSemtex = TRUE;
	pGrenade->m_bSemtexInWorld = TRUE;

//	FireTargets( "semtex_obj", pGrenade, pGrenade, USE_TOGGLE, 0 );

	return pGrenade;
}
//*******************

CGrenade * CGrenade:: ShootTimedSmoke( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
pGrenade->Spawn();
UTIL_SetOrigin( pGrenade->pev, vecStart );
pGrenade->pev->velocity = vecVelocity;
pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
pGrenade->pev->owner = ENT(pevOwner);

//pGrenade->SetTouch( ExplodeTouch );
pGrenade->SetTouch( BounceTouch ); // Bounce if touched

// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

pGrenade->pev->dmgtime = gpGlobals->time + time;
pGrenade->SetThink( SmokeTumbleThink );

pGrenade->pev->nextthink = gpGlobals->time + 0.1;
if (time < 0.1)
{
pGrenade->pev->nextthink = gpGlobals->time;
pGrenade->pev->velocity = Vector( 0, 0, 0 );
}

pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
pGrenade->pev->framerate = 1.0;

// Tumble through the air
// pGrenade->pev->avelocity.x = -400;

pGrenade->pev->gravity = 0.5;
pGrenade->pev->friction = 0.8;

pGrenade->pev->classname = MAKE_STRING("smoke_grenade"); // UGLY HACK!

pGrenade->m_bIsSmoke = TRUE;//??????????????

SET_MODEL(ENT(pGrenade->pev), "models/weapons/handgrenade/w_grenade.mdl");
pGrenade->pev->dmg = 0;

MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
	WRITE_BYTE( TE_BEAMFOLLOW );
	WRITE_SHORT(pGrenade->entindex());	// entity
	WRITE_SHORT(g_sModelIndexLaser );	// model
	WRITE_BYTE( 5 ); // life
	WRITE_BYTE( 7 );  // width
	WRITE_BYTE( 150 );   // r, g, b
	WRITE_BYTE( 150 );   // r, g, b
	WRITE_BYTE( 150 );   // r, g, b
	WRITE_BYTE( 111 );	// brightness
MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

return pGrenade;
}

void CGrenade :: SmokeTumbleThink( void )
{
if (!IsInWorld())
{
UTIL_Remove( this );
return;
}

StudioFrameAdvance( );
pev->nextthink = gpGlobals->time + 0.1;

if (pev->dmgtime - 1 < gpGlobals->time)
{
CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
}
//NO EXPLOTAR

if (pev->dmgtime <= gpGlobals->time)
{
SetThink( SmokeDetonate );
}

if (pev->waterlevel != 0)
{
pev->velocity = pev->velocity * 0.5;
pev->framerate = 0.2;
}
//test
	UTIL_MakeAimVectors( pev->angles );

	Vector vecForward = gpGlobals->v_forward;	
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	Vector vecSrc;

	if (m_flNextAttack < gpGlobals->time && pev->flags & FL_ONGROUND)
	{
//		vecSrc = pev->origin + vecForward * -64 + vecRight * -100 + vecUp * -96;

//		vecSrc = pev->origin + vecForward * RANDOM_FLOAT( -64, 64 ) + vecRight * RANDOM_FLOAT( -64, 64 ) + vecUp * RANDOM_FLOAT( 32, 64 );//!!-32??
	/*
		vecSrc = pev->origin;
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", vecSrc, TRUE );
			
		pSprite->SetTransparency( kRenderTransAlpha, 200, 200, 200, 255, kRenderFxNone );
		pSprite->SetScale( 7.0 );//5.0
		pSprite->Expand( 4 , RANDOM_FLOAT( 25.0, 35.0 )  );//5
		pSprite->pev->frame = 0;

		pSprite->pev->avelocity.x = RANDOM_FLOAT( -50, 50 );//rotar?
		pSprite->pev->avelocity.y = RANDOM_FLOAT( -50, 50 );
		pSprite->pev->avelocity.z = RANDOM_FLOAT( -50, 50 );

		pSprite->pev->velocity.z = RANDOM_FLOAT( 10, 20 );
		pSprite->pev->velocity.x = RANDOM_FLOAT( -50, 50 );
		pSprite->pev->velocity.y = RANDOM_FLOAT( -50, 50 );
*/
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "weapons/smoke_bang.wav", 0.5, ATTN_NORM);
		
		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_SHORT(iDefaultSmoke);
		MESSAGE_END();

		m_flNextAttack = gpGlobals->time + 0.3; // debounce //1.0
	}
//asdasd
	
//	TraceResult *pTrace;

 // if ( pTrace->flFraction != 1.0 ) 
//  {
 //   pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * 100 * 0.1);
 // }

    CBaseEntity *pEntity = NULL;
    // we'll loop thru all entities 2000 units or less away from us
    while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, 2000 )) != NULL) 
	{
	 // only affect players, duh
      if ( pEntity->IsPlayer() ) 
	  {
        // get the distance between the flash & the player
        float flDist = (pEntity->Center() - pev->origin).Length();
        // we'll only fade the player's view if he saw the flash
        if ( FVisible( pEntity ) ) 
		{
			// get the distance between the flash & the player
			float flDist = (pEntity->Center() - pev->origin).Length();

			ALERT( at_console, "flDist %f\n", flDist);

			if (pev->flags & FL_ONGROUND)
			{
				float FadeAlpha;

				if (flDist >= 300)//más rapido
					FadeAlpha = 0;

				else if (flDist >= 195)
						FadeAlpha = 10;
				else if (flDist >= 190)
						FadeAlpha = 20;
				else if (flDist >= 185)
						FadeAlpha = 30;
				else if (flDist >= 180)
						FadeAlpha = 40;
				else if (flDist >= 275)
						FadeAlpha = 50;
				else if (flDist >= 170)
						FadeAlpha = 60;
				else if (flDist >= 165)
						FadeAlpha = 70;
				else if (flDist >= 160)
						FadeAlpha = 80;
				else if (flDist >= 155)
						FadeAlpha = 90;
				else if (flDist >= 150)
						FadeAlpha = 100;
				else if (flDist >= 145)
						FadeAlpha = 110;
				else if (flDist >= 140)
						FadeAlpha = 120;
				else if (flDist >= 235)
						FadeAlpha = 130;
				else if (flDist >= 130)
						FadeAlpha = 140;
				else if (flDist >= 125)
						FadeAlpha = 150;
				else if (flDist >= 120)
						FadeAlpha = 160;
				else if (flDist >= 115)
						FadeAlpha = 170;
				else if (flDist >= 110)
						FadeAlpha = 180;
				else if (flDist >= 100)
						FadeAlpha = 190;
				else if (flDist >= 95)
						FadeAlpha = 200;
				else if (flDist >= 90)
						FadeAlpha = 210;
				else if (flDist >= 85)
						FadeAlpha = 220;
				else if (flDist >= 80)
						FadeAlpha = 230;
				else if (flDist >= 75)
						FadeAlpha = 240;
				else
						FadeAlpha = 255;

				if (flDist <= 150)
				{
					UTIL_ScreenFade( pEntity, Vector(200,200,200), 0.2, 0.2, FadeAlpha, FFADE_IN );
				}
			}
		}
     }//si es player
    }
}

void CGrenade::SmokeDetonate( void )
{
	TraceResult tr;
	Vector vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

	SmokeGren( &tr );
}


void CGrenade::SmokeGren( TraceResult *pTrace ) 
{
  pev->model = iStringNull;
  pev->solid = SOLID_NOT;
  pev->takedamage = DAMAGE_NO;

  if ( pTrace->flFraction != 1.0 ) 
  {
    pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * 100 * 0.1);
  }

  int iContents = UTIL_PointContents ( pev->origin );

  if (iContents != CONTENTS_WATER) 
  {
  }
  pev->effects |= EF_NODRAW;

  pev->velocity = g_vecZero;
}

//======================end grenade


CGrenade * CGrenade::ShootSfera( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time ) 
{
  CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
  pGrenade->Spawn();

  UTIL_SetOrigin( pGrenade->pev, vecStart );
  pGrenade->pev->velocity = vecVelocity;
  pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
  pGrenade->pev->owner = ENT(pevOwner);

  pGrenade->SetTouch( BounceTouch );

  pGrenade->pev->dmgtime = gpGlobals->time + time;
  pGrenade->SetThink( TumbleThink );

  pGrenade->pev->avelocity.y = RANDOM_FLOAT ( -5, -20 );
  pGrenade->pev->nextthink = gpGlobals->time + 0.1;
  if (time < 0.1) {
    pGrenade->pev->nextthink = gpGlobals->time;
    pGrenade->pev->velocity = Vector( 0, 0, 0 );
  }

    SET_MODEL(ENT(pGrenade->pev), "models/combine_ball.mdl");//ugly hack

  //pGrenade->pev->animtime = gpGlobals->time;
  //pGrenade->pev->framerate = 1.0;

  pGrenade->pev->gravity = 0.1;//jeje
  pGrenade->pev->friction = 0.1;

  //pGrenade->pev->gravity = 0.2;//jeje
  //pGrenade->pev->friction = 0.1;

  pGrenade->m_bIsSfera = TRUE;
	pGrenade->pev->classname = MAKE_STRING("combine_ball"); // UGLY HACK!
	
  pGrenade->pev->dmg = 80;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(pGrenade->entindex());	// entity
		WRITE_SHORT(g_sModelIndexLaser );	// model
		WRITE_BYTE( 4 ); // life
		WRITE_BYTE( 8 );  // width
		WRITE_BYTE( 222 );   // r, g, b
		WRITE_BYTE( 222 );   // r, g, b
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 111 );	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)
		/*
	CSprite *pSprite = CSprite::SpriteCreate( "sprites/alt_fire.spr", pGrenade->pev->origin, FALSE );

	pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 155, kRenderFxNone );
	pSprite->SetScale( 0.2 );		
	pSprite->SetAttachment( pGrenade->edict(), 0 );
	pSprite->pev->frame = 0;
	pSprite->pev->framerate = 0;
	*/
  return pGrenade;
}

void CGrenade::SferaExplode( TraceResult *pTrace ) 
{
  pev->model = iStringNull;

  	CSprite *pSprite = CSprite::SpriteCreate( "sprites/null.spr", pev->origin, FALSE );

	pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 155, kRenderFxNone );
	pSprite->SetScale( 0.2 );		
	pSprite->SetAttachment( edict(), 0 );
	pSprite->pev->frame = 0;
	pSprite->pev->framerate = 0;

  pev->solid = SOLID_NOT;
  pev->takedamage = DAMAGE_NO;
  	
  pev->effects |= EF_LIGHT;

  if ( pTrace->flFraction != 1.0 ) 
  {
    pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * 100 * 0.1);
  }
  // flash, apparently, doesn't work underwater.
  int iContents = UTIL_PointContents ( pev->origin );
  if (iContents != CONTENTS_WATER) 
  {
//	 ALERT( at_console, "> No esta en agua... \n");
    // replace this with YOUR flash-sound. you'll also want to precache the sound in weapons.cpp.
//    	 ALERT( at_console, "> Creando sonidos y decals... \n");
	 EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/energy_explode.wav", 0.9, ATTN_NORM);
    // cool decal
    UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );

    // heavy sparkage!
//    ALERT( at_console, "> Creando -spark_shower-.. \n");
    int sparks = RANDOM_LONG(1,6);
    for ( int i = 0; i < sparks; i++ )
      Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
  }
  // don't draw the model after it has exploded.
  pev->effects |= EF_NODRAW;

 pev->velocity = g_vecZero;
		
	UTIL_ScreenShake( pev->origin, 12.0, 50.0, 2.0, 1000 );

 UTIL_Remove( this );
}
