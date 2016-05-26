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
  
+S2P Physics
+Experience points system
+New fire bullets trough walls -CODE BY R2D2RIGO-
+Add new effects using particle system
+New gibs (solidify changed by Laurie Cheers)
*Other minor modifications

***/
/*
===== combat.cpp ========================================================

  functions dealing with damage infliction & death

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "decals.h"
#include "animation.h"
#include "weapons.h"
#include "func_break.h"

#include "player.h"

#include "gamerules.h"//SP: para la comprobacion del MP

#include "particle_defs.h"
extern int gmsgParticles;

extern cvar_t phys_save_angles;
extern cvar_t phys_normalize_angles;
extern cvar_t phys_globaltrace;
extern cvar_t phys_enable;
extern cvar_t phys_objects;
extern cvar_t phys_movehit;

extern DLL_GLOBAL Vector		g_vecAttackDir;
extern DLL_GLOBAL int			g_iSkillLevel;

extern Vector VecBModelOrigin( entvars_t* pevBModel );
extern entvars_t *g_pevLastInflictor;

extern cvar_t mp_am_noblastgibs;

extern cvar_t phys_flyattack;
extern cvar_t phys_movecorpses;
extern cvar_t mp_am_lenguage;
extern cvar_t phys_simulateragdoll;

#define GERMAN_GIB_COUNT		4
//#define	HUMAN_GIB_COUNT			6
#define ALIEN_GIB_COUNT			4

#define	HUMAN_GIB_COUNT			13
// HACKHACK -- The gib velocity equations don't work
void CGib :: LimitVelocity( void )
{
	float length = pev->velocity.Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if ( length > 1500.0 )
		pev->velocity = pev->velocity.Normalize() * 1500;		// This should really be sv_maxvelocity * 0.75 or something
}


void CGib :: SpawnStickyGibs( entvars_t *pevVictim, Vector vecOrigin, int cGibs )
{
	int i;

	if ( g_Language == LANGUAGE_GERMAN )
	{
		// no sticky gibs in germany right now!
		return; 
	}

	for ( i = 0 ; i < cGibs ; i++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

//		pGib->Spawn( "models/stickygib.mdl" );
		pGib->Spawn( "sprites/bloodfrag2.spr" );

		pGib->pev->body = RANDOM_LONG(0,2);

		if ( pevVictim )
		{
			//normal code
			/*
			pGib->pev->origin.x = vecOrigin.x + RANDOM_FLOAT( -3, 3 );
			pGib->pev->origin.y = vecOrigin.y + RANDOM_FLOAT( -3, 3 );
			pGib->pev->origin.z = vecOrigin.z + RANDOM_FLOAT( -3, 3 );

			*/
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) );
			

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.15, 0.15 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.15, 0.15 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.15, 0.15 );

			pGib->pev->velocity = pGib->pev->velocity * 500;//900

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 250, 400 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 250, 400 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
		
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			
			pGib->pev->movetype = MOVETYPE_TOSS;
					//test
			pGib->pev->gravity = 0.2;
			pGib->pev->rendermode = kRenderTransAlpha;
			pGib->pev->renderamt = 255;
			pGib->pev->scale = 0.8;

			pGib->pev->frame = RANDOM_LONG ( 1 , 3 );

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector ( 0, 0 ,0 ), Vector ( 0, 0, 0 ) );
			pGib->SetTouch ( StickyGibTouch );
			pGib->SetThink (NULL);
		}
		pGib->LimitVelocity();
	}
}

void CGib :: SpawnHeadGib( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	if ( g_Language == LANGUAGE_GERMAN )
	{
		pGib->Spawn( "models/germangibs.mdl" );// throw one head
		pGib->pev->body = 0;
	}
	else
	{
		pGib->Spawn( "models/hgibs.mdl" );// throw one head
		pGib->pev->body = 0;
	}

	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
		
		edict_t		*pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );
		
		if ( RANDOM_LONG ( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t	*pevPlayer;

			pevPlayer = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
		}


		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
	
		if ( pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}

void CGib :: SpawnRandomGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		if ( g_Language == LANGUAGE_GERMAN )
		{
			pGib->Spawn( "models/germangibs.mdl" );
			pGib->pev->body = RANDOM_LONG(0,GERMAN_GIB_COUNT-1);
		}
		else
		{
			if ( human )
			{
				// human pieces
				pGib->Spawn( "models/hgibs.mdl" );
				pGib->pev->body = RANDOM_LONG(1,HUMAN_GIB_COUNT-1);// start at one to avoid throwing random amounts of skulls (0th gib)
			}
			else
			{
				// aliens
				pGib->Spawn( "models/agibs.mdl" );
				pGib->pev->body = RANDOM_LONG(0,ALIEN_GIB_COUNT-1);
			}
		}

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}


BOOL CBaseMonster :: HasHumanGibs( void )
{
	int myClass = Classify();

	if ( myClass == CLASS_HUMAN_MILITARY ||
		 myClass == CLASS_PLAYER_ALLY	||
		 myClass == CLASS_HUMAN_PASSIVE  ||
		 //SP - News
		 myClass == CLASS_FACTION_A  || //assassins
		 myClass == CLASS_FACTION_B  || //??
		 myClass == CLASS_FACTION_C  || //terrors??

		 myClass == CLASS_PLAYER )

		 return TRUE;

	return FALSE;
}


BOOL CBaseMonster :: HasAlienGibs( void )
{
	int myClass = Classify();

	if ( myClass == CLASS_ALIEN_MILITARY ||
		 myClass == CLASS_ALIEN_MONSTER	||
		 myClass == CLASS_ALIEN_PASSIVE  ||
		 myClass == CLASS_INSECT  ||
		 myClass == CLASS_ALIEN_PREDATOR  ||
		 myClass == CLASS_ALIEN_PREY )

		 return TRUE;

	return FALSE;
}


void CBaseMonster::FadeMonster( void )
{
	StopAnimation();
	pev->velocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->avelocity = g_vecZero;
	pev->animtime = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
	SUB_StartFadeOut();
}

//=========================================================
// GibMonster - create some gore and get rid of a monster's
// model.
//=========================================================
void CBaseMonster :: GibMonster( void )
{
	TraceResult	tr;
	BOOL		gibbed = FALSE;

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);		

	// only humans throw skulls !!!UNDONE - eventually monsters will have their own sets of gibs
	if ( HasHumanGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_hgibs") != 0 )	// Only the player will ever get here
		{
			CGib::SpawnHeadGib( pev );
			CGib::SpawnRandomGibs( pev, 4, 1 );	// throw some human gibs.//4
		}
		gibbed = TRUE;
	}
	else if ( HasAlienGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_agibs") != 0 )	// Should never get here, but someone might call it directly
		{
			CGib::SpawnRandomGibs( pev, 4, 0 );	// Throw alien gibs
		}
		gibbed = TRUE;
	}

	if ( !IsPlayer() )
	{
		if ( gibbed )
		{
			// don't remove players!
			SetThink ( SUB_Remove );
			pev->nextthink = gpGlobals->time;
		}
		else
		{
			FadeMonster();
		}
	}
}

//=========================================================
// GetDeathActivity - determines the best type of death
// anim to play.
// SysOp edit: I've added S2P Physics here. Whit this you can
// show the best type of death (no more clipping on walls)
//=========================================================
Activity CBaseMonster :: GetDeathActivity ( void )
{
	Activity	deathActivity;
	BOOL		fTriedDirection;
	float		flDot;
	TraceResult	tr;
	Vector		vecSrc;

	if ( pev->deadflag != DEAD_NO )
	{
		// don't run this while dying.
		return m_IdealActivity;
	}

	vecSrc = Center();

	fTriedDirection = FALSE;
	deathActivity = ACT_DIESIMPLE;// in case we can't find any special deaths to do.

	UTIL_MakeVectors ( pev->angles );
	flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );

	if (m_bitsDamageType & DMG_BLAST) // si el daño es explosion	
	{
		fTriedDirection = TRUE;

		//si el daño viene de frente
		if ( flDot > 0.3 )
		{
			deathActivity = ACT_DIEFORWARD;
		}	
		//si el daño viene de atras
		else if ( flDot <= -0.3 )
		{
			deathActivity = ACT_DIEBACKWARD;
		}

		//*******		MONSTER HUMAN AK 
		if ( FClassnameIs( pev, "monster_terrorist" ) )	
			deathActivity = ACT_DIE_BACKSHOT;

		//si no podemos indicar animaciones segun la direccion (error!):
		if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
		{
			deathActivity = ACT_DIESIMPLE;//Fix para algunos models
		}
		//devolvamos animacion de muerte final
		return deathActivity;
	}

	switch ( m_LastHitGroup )
	{
		// try to pick a region-specific death.
	case HITGROUP_HEAD:
		{
			// make sure there's room to fall backward
			UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -64, dont_ignore_monsters, head_hull, edict(), &tr );

			if ( tr.flFraction != 1.0 )
			{
				deathActivity = ACT_DIEFORWARD;
				ALERT( at_console, ">NO ROOM TO FALL BACKWARD!\n");
			}
			else
			{
				if ( FClassnameIs( pev, "npc_zombine" ) )
				{
					if (bViolentHeadDamaged)
					deathActivity = ACT_SNIFF;
					else
					deathActivity = ACT_DIE_HEADSHOT;
				}
				else
				{
					deathActivity = ACT_DIE_HEADSHOT;
				}
			}
		}
		break;

	case HITGROUP_STOMACH:
		{
			// make sure there's room to fall backward
			UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -64, dont_ignore_monsters, head_hull, edict(), &tr );

			if ( tr.flFraction != 1.0 )
			{
				deathActivity = ACT_DIEFORWARD;
				ALERT( at_console, ">NO ROOM TO FALL BACKWARD!\n");
			}
			else
			{
				deathActivity = ACT_DIE_GUTSHOT;
			}
		}
		break;

	case HITGROUP_GENERIC:
		// try to pick a death based on attack direction
		fTriedDirection = TRUE;

		if ( flDot > 0.3 )
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if ( flDot <= -0.3 )
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;

	default:
		// try to pick a death based on attack direction
		fTriedDirection = TRUE;

		if ( flDot > 0.3 )
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if ( flDot <= -0.3 )
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;
	}

	// can we perform the prescribed death?
	if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		// no! did we fail to perform a directional death? 
		if ( fTriedDirection )
		{
			// if yes, we're out of options. Go simple.
			deathActivity = ACT_DIESIMPLE;
		}
		else
		{
			// cannot perform the ideal region-specific death, so try a direction.
			if ( flDot > 0.3 )
			{
				deathActivity = ACT_DIEFORWARD;
			}
			else if ( flDot <= -0.3 )
			{
				deathActivity = ACT_DIEBACKWARD;
			}
		}
	}

	if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		// if we're still invalid, simple is our only option.
		deathActivity = ACT_DIESIMPLE;
	}

	if ( deathActivity == ACT_DIEFORWARD )
	{
		// make sure there's room to fall forward
		UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );

		if ( tr.flFraction != 1.0 )
		{
			deathActivity = ACT_DIEBACKWARD;//backwards? -ACT_DIESIMPLE
			ALERT( at_console, ">NO ROOM TO FALL FORWARD!\n");
		}
	}

	if ( deathActivity == ACT_DIEBACKWARD )
	{
		// make sure there's room to fall backward
		//adelante??? tenemos que chequear atras!
		UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -64, dont_ignore_monsters, head_hull, edict(), &tr );

		if ( tr.flFraction != 1.0 )
		{
			//sys add!
			if ( FClassnameIs( pev, "monster_human_grunt" ) 
				|| FClassnameIs( pev, "monster_human_grunt_pistol" )
				|| FClassnameIs( pev, "monster_human_grunt_rpg" )
				|| FClassnameIs( pev, "monster_hgrunt_opfor" )
				)	
			{
				deathActivity = ACT_DIEVIOLENT;
			}
			else
			{
				// make sure there's room to fall backward
				//adelante??? tenemos que chequear atras!
				UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -64, dont_ignore_monsters, head_hull, edict(), &tr );

				if ( tr.flFraction != 1.0 )
				{
					deathActivity = ACT_DIEFORWARD;
					ALERT( at_console, ">NO ROOM TO FALL BACKWARD!\n");
				}
			}
		}
	}

	return deathActivity;
}

//=========================================================
// GetSmallFlinchActivity - determines the best type of flinch
// anim to play.
//=========================================================
Activity CBaseMonster :: GetSmallFlinchActivity ( void )
{
	Activity	flinchActivity;
	BOOL		fTriedDirection;
	float		flDot;

	fTriedDirection = FALSE;
	UTIL_MakeVectors ( pev->angles );
	flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );
	
	switch ( m_LastHitGroup )
	{
		// pick a region-specific flinch
	case HITGROUP_HEAD:
		flinchActivity = ACT_FLINCH_HEAD;
		break;
	case HITGROUP_STOMACH:
		flinchActivity = ACT_FLINCH_STOMACH;
		break;
	case HITGROUP_LEFTARM:
		flinchActivity = ACT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHTARM:
		flinchActivity = ACT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFTLEG:
		flinchActivity = ACT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHTLEG:
		flinchActivity = ACT_FLINCH_RIGHTLEG;
		break;
	case HITGROUP_GENERIC:
	default:
		// just get a generic flinch.
		flinchActivity = ACT_SMALL_FLINCH;
		break;
	}

	// do we have a sequence for the ideal activity?
	if ( LookupActivity ( flinchActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		flinchActivity = ACT_SMALL_FLINCH;
	}

	return flinchActivity;
}


void CBaseMonster::BecomeDead( void )
{//sys
	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.
	
	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.

	// make the corpse fly away from the attack vector

	//FIXED (thanks developers!)

	if ( (Classify() == CLASS_PLAYER_ALLY) || (Classify() == CLASS_HUMAN_PASSIVE))
		pev->movetype = MOVETYPE_TOSS;
	else if (StartBurning)//if a monster is burned, we don't want to fly it up (the fire can be bad placed)
		pev->movetype = MOVETYPE_TOSS;
	else
		pev->movetype = MOVETYPE_BOUNCE;
	
	//this is a talk monster, but it can be used to be enemy. In this case the check we did before
	//doesn't work, give it a toss movetype:
	if ( FClassnameIs( pev, "monster_human_grunt_pistol" ) )
		pev->movetype = MOVETYPE_TOSS;

	pev->gravity = CVAR_GET_FLOAT("phys_gravity");
	pev->friction = CVAR_GET_FLOAT("phys_friction");

	if ( phys_enable.value != 0  ) //no es cero esta _Activado_
	{
		if ( phys_flyattack.value != 0  ) //no es cero esta _Activado_
		{
			if (!(StartBurning))
			{
				//cuando el monster es creado por un monster maker
				//no saldra despedido... Cambiar esto por ahora.
				if ( (pev->spawnflags & SF_MONSTER_FADECORPSE) || !FNullEnt( pev->owner ) )
				{
					//the monster will be faded
				}
				else
				{
					if ( ( m_bitsDamageType & DMG_BLAST ) ) // si el daño es explosion
					{
						ALERT ( at_console, "# DMG_BLAST #\n" );
						pev->flags &= ~FL_ONGROUND;
						pev->origin.z += 2;//2??
						pev->velocity = g_vecAttackDir * -1;
						pev->velocity = pev->velocity * RANDOM_FLOAT( 400, 600 );
					}
					else
					{
						pev->flags &= ~FL_ONGROUND;
						pev->origin.z += 1;
						pev->velocity = g_vecAttackDir * -1;
						pev->velocity = pev->velocity * RANDOM_FLOAT( 150, 250 );//200 400 too much!
					}
				}
			}
			else
			{
				if ( (pev->spawnflags & SF_MONSTER_FADECORPSE) || !FNullEnt( pev->owner ) )
				{
					//the monster will be faded
					//UNDONE: Spawn a new fire as long as the same fade's time?
				}
				else
				{
					//the monster will rest in peace here, so start the news fire and smoke.
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
							WRITE_SHORT(iDefaultFinalFire);
						MESSAGE_END();
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
							WRITE_SHORT(iDefaultFinalSmoke);
						MESSAGE_END();*/
					}
								
					//Make black
					pev->renderfx = 21;
					pev->renderamt = 1;//+=1?

					//	just in case			
					pev->rendercolor.x = 0;
					pev->rendercolor.y = 0;
					pev->rendercolor.z = 0;

					//clear sound?
					//EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/null.wav", 0.5, ATTN_NORM );
					//STOP_SOUND( ENT(pev), CHAN_ITEM, "ambience/burning1.wav" );
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "ambience/burning1.wav", 0.5, ATTN_NORM );
				}
			}
		}
	}
}
void CBaseMonster :: NormalizeAngles( float *angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}
void CBaseMonster::BounceCorpse( CBaseEntity *pOther )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	if (pev->flags & FL_ONGROUND)
	{
	// add a bit of static friction
	pev->velocity = pev->velocity * CVAR_GET_FLOAT("phys_velocity" );//0.95
	pev->avelocity = pev->avelocity * CVAR_GET_FLOAT("phys_avelocity" );//0.9
	
	Vector savedangles = Vector( 0, 0, 0 );
	int negate = 0;
	TraceResult tr;
	// look down directly to know the surface we're lying.
			
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );

#ifndef M_PI
#define M_PI 3.14159265358979
#endif
#define ang2rad (2 * M_PI / 360)

	if ( phys_enable.value != 0  ) //no es cero esta _Activado_
	{
		if ( phys_normalize_angles.value != 0  ) //no es cero esta _Activado_
		{
			if ( tr.flFraction < 1.0 )
			{
				Vector forward, right, angdir, angdiry;
				Vector Angles = pev->angles;

				NormalizeAngles( Angles );
				
				UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
				angdir = forward;
				Vector left = -right;
				angdiry = left;
		//-?
				pev->angles.x = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
				pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
				pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			}
		}
	}
	#undef ang2rad
}
		
	if ( pOther->IsBSPModel() )
	{		
		Vector vecForward = gpGlobals->v_forward;	
		Vector vecRight = gpGlobals->v_right;
		Vector vecUp = gpGlobals->v_up;

		Vector vecSrc;

		vecSrc = pev->origin + vecForward * RANDOM_FLOAT( -4, 4 ) + vecRight * RANDOM_FLOAT( -4, 4 ) + vecUp * RANDOM_FLOAT( 2, 8 );//!!-32??

		int pitch = 95 + RANDOM_LONG(0,29);
/*
		switch (RANDOM_LONG(0,3))
		{
			case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "common/bodydrop1.wav", 1, ATTN_NORM, 0, pitch); break;
			case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "common/bodydrop2.wav", 1, ATTN_NORM, 0, pitch); break;
			case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "common/bodydrop3.wav", 1, ATTN_NORM, 0, pitch); break;
			case 3: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "common/bodydrop4.wav", 1, ATTN_NORM, 0, pitch); break;
		}*/	
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
	LimitVelocity();

	UTIL_SetOrigin( pev, pev->origin );// link into world.
}

void CBaseMonster :: LimitVelocity( void )
{
	float length = pev->velocity.Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if ( length > 1500.0 )
		pev->velocity = pev->velocity.Normalize() * 1500;		// This should really be sv_maxvelocity * 0.75 or something
}

BOOL CBaseMonster::ShouldGibMonster( int iGib )
{
	if ( ( iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE ) || ( iGib == GIB_ALWAYS ) )
		return TRUE;
	
	return FALSE;
}


void CBaseMonster::CallGibMonster( void )
{
	BOOL fade = FALSE;

	if ( HasHumanGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_hgibs") == 0 )
			fade = TRUE;
	}
	else if ( HasAlienGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_agibs") == 0 )
			fade = TRUE;
	}

	pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_NOT;// do something with the body. while monster blows up

	if ( fade )
	{
		FadeMonster();
	}
	else
	{
		pev->effects = EF_NODRAW; // make the model invisible.
		GibMonster();
	}

	pev->deadflag = DEAD_DEAD;
	FCheckAITrigger();

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}
	
	if ( ShouldFadeOnDeath() && !fade )
		UTIL_Remove(this);
}


/*
============
Killed
============
*/
void CBaseMonster :: Killed( entvars_t *pevAttacker, int iGib )
{
	unsigned int	cCount = 0;
	BOOL			fDone = FALSE;

	if ( HasMemory( bits_MEMORY_KILLED ) )
	{
		if ( ShouldGibMonster( iGib ) )
			CallGibMonster();
		return;
	}

	Remember( bits_MEMORY_KILLED );

	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
	m_IdealMonsterState = MONSTERSTATE_DEAD;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions( bits_COND_LIGHT_DAMAGE );

	//blood trace
	Vector		vecEndPos;
	TraceResult tr;

	vecEndPos = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
	UTIL_TraceLine ( vecEndPos, vecEndPos + Vector ( 0, 0, -64 ),  ignore_monsters, ENT(pev), &tr);

//	if(pev->flags & FL_ONGROUND )
//	{
		if (tr.flFraction != 1.0) // != 1.0)// Hemos tocado algo
		{
			/*
			if ( m_bloodColor == BLOOD_COLOR_RED )		
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
					WRITE_SHORT(iDefaultBloodRedPit);
				MESSAGE_END();
			}
			else
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
					WRITE_SHORT(iDefaultBloodGreenPit);
				MESSAGE_END();
			}
			*/
		}
//	}

	//sangre?
	/*
	if(pev->flags & FL_ONGROUND )
	{
	Vector vecForward = gpGlobals->v_forward;	

	Vector		vecEndPos;
	TraceResult tr;

//	vecEndPos = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
	vecEndPos = pev->origin;//move up a bit, and trace down.
	UTIL_TraceLine ( vecEndPos, vecEndPos + Vector ( 0, 0, -222 ),  ignore_monsters, ENT(pev), &tr);
			
	if (tr.flFraction != 1.0) // != 1.0)// Hemos tocado algo
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		float flDist = (pEntity->Center() - pev->origin).Length();

		if ( pEntity->IsBSPModel() )
		{
//			ALERT ( at_console, "I touch something!  %f\n", tr.flFraction );

			CSprite *pSprite = CSprite::SpriteCreate( "sprites/bloodspot.spr", tr.vecEndPos + Vector ( 0, 0, -20 ), TRUE );

			if ( m_bloodColor == BLOOD_COLOR_RED )		
			pSprite->SetTransparency( kRenderTransAlpha, 255, 0, 0, 255, kRenderFxNone );
			else
			pSprite->SetTransparency( kRenderTransAlpha, 0, 255, 10, 255, kRenderFxNone );

			pSprite->SetScale( 0.05);
				//scale fade
			pSprite->pev->frame = 0;

			pSprite->ExpandScaled( 0.05, 0, 0.4 );

			pSprite->pev->angles.z += RANDOM_LONG(-180,180);

			pSprite->pev->angles.y += 90;
			pSprite->pev->angles.x += 90;
			
		}
		else
		{
//			ALERT ( at_console, "Is not a BSP model  %f\n", tr.flFraction );
		}
	}

		extern short g_sModelIndexLaser;
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_BEAMPOINTS );
			WRITE_COORD( vecEndPos.x );
			WRITE_COORD( vecEndPos.y );
			WRITE_COORD( vecEndPos.z );

			WRITE_COORD( vecEndPos.x );
			WRITE_COORD( vecEndPos.y );
			WRITE_COORD( vecEndPos.z -222 );
			WRITE_SHORT( g_sModelIndexLaser );
			WRITE_BYTE( 0 ); // framerate
			WRITE_BYTE( 0 ); // framerate

			WRITE_BYTE( 55 ); // life
			WRITE_BYTE( 1 );  // width

			WRITE_BYTE( 0 );   // noise
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );	// brightness
			WRITE_BYTE( 111 );		// speed
		MESSAGE_END();
	}*/
	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if ( pOwner )
	{
		pOwner->DeathNotice( pev );
	}

	if	( ShouldGibMonster( iGib ) )
	{
		CallGibMonster();
		return;
	}
	else if ( pev->flags & FL_MONSTER )
	{
//		SetTouch( NULL );
		SetTouch( BounceCorpse );

		//para no sacarlo y agregarlo nuevamente
		BecomeDead();
	}
	
	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}
	
	//pev->enemy = ENT( pevAttacker );//why? (sjb)

	m_IdealMonsterState = MONSTERSTATE_DEAD;

	/////////////////////// XP POINTS

	//escribir en xp_points el valor de cada punto

	//TO DO: This is okey, but I need to make a new HUD (and make a new map in the tutorial showing this)
//#if CODE_TEST
	int iXPPoints = CVAR_GET_FLOAT( "xp_points" );
			
	int iMonstersKilled = CVAR_GET_FLOAT( "score_killed" );
	int iMonstersKilledbyHead = CVAR_GET_FLOAT( "score_head" );
	int iMonstersKilledbyKnife = CVAR_GET_FLOAT( "score_knifed" );

	//same cast?
	CBaseEntity *pPlayer = GetClassPtr((CBaseEntity *)pevAttacker);

	if( pPlayer && pPlayer->IsPlayer() )//only players, huh :I
	{ 
		if(	m_fSurrender)
		{
			UTIL_ShowMessageAll( STRING(ALLOC_STRING("PUNISH_SURR")));
			//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#PUNISH_SURR" );
			CVAR_SET_FLOAT( "xp_points", ( iXPPoints -= 50) );
		}
		else if ( (Classify() == CLASS_PLAYER_ALLY) || (Classify() == CLASS_HUMAN_PASSIVE) )
		{
			UTIL_ShowMessageAll( STRING(ALLOC_STRING("PUNISH_ALLY")));
			CVAR_SET_FLOAT( "xp_points", ( iXPPoints -= 5) );
		}
		else
		{
			if ( m_LastHitGroup == HITGROUP_HEAD)
			{
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("HEADSHOT_STYLE")));
				//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#HEADSHOT_STYLE" );

				CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 3) );
							
				CVAR_SET_FLOAT( "score_head", ( iMonstersKilledbyHead += 1) );

					
//+				b_SlowedMonster = TRUE;
			}
			else//simple
			{
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("SIMPLE_STYLE")));
				//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#SIMPLE_STYLE" );

				CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 1) );

				CVAR_SET_FLOAT( "score_killed", ( iMonstersKilled += 1) );

				if(iXPPoints >=20000)//vampire mode
				pPlayer->TakeHealth (10, DMG_GENERIC); //energia
/*
				pPlayer->g_fStartCouting += 5;

				if(pPlayer->g_fCoutingBonus == MASTERKILL)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("MASTERKILL")));
				if(pPlayer->g_fCoutingBonus == GODSAKE)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("GODSAKE")));
				if(pPlayer->g_fCoutingBonus == RIDICULOUS)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("RIDICULOUS")));
				if(pPlayer->g_fCoutingBonus == MONSTERKILL)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("MONSTERKILL")));
				if(pPlayer->g_fCoutingBonus == MEGAKILL)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("MEGAKILL")));
				if(pPlayer->g_fCoutingBonus == ULTRAKILL)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("ULTRAKILL")));
				if(pPlayer->g_fCoutingBonus == MULTIKILL)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("MULTIKILL")));
				if(pPlayer->g_fCoutingBonus == DOBLEKILL)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("DOBLEKILL")));*/
			}
		}
/*
			if ( NextTauntTime < gpGlobals->time )
			{
				CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);
				//porqueeeeé usar sonidos precacheados?!
				if ( mp_am_lenguage.value == 2  ) //SPANISH
				{
				//	pPlayer->SetSuitUpdate("!PLTAUNT_SPA", FALSE, SUIT_NEXT_IN_3SEC);
					EMIT_GROUPNAME_SUIT(ENT(pPlayer->pev), "PLTAUNT_SPA");

					
				//	switch ( RANDOM_LONG(0,3) )
				//	{
				//	case 0:	EMIT_SOUND_DYN(ENT(0), CHAN_STATIC, "Player/taunt/spa/byebye.wav", 1.0, ATTN_NONE, 0, PITCH_NORM); break;
				//	case 1:	EMIT_SOUND_DYN(ENT(0), CHAN_STATIC, "Player/taunt/spa/infierno.wav", 1.0, ATTN_NONE, 0, PITCH_NORM); break;
				//	case 2:	EMIT_SOUND_DYN(ENT(0), CHAN_STATIC, "Player/taunt/spa/muere.wav", 1.0, ATTN_NONE, 0, PITCH_NORM); break;
				//	case 3:	EMIT_SOUND_DYN(ENT(0), CHAN_STATIC, "Player/taunt/spa/remera.wav", 1.0, ATTN_NONE, 0, PITCH_NORM); break;
				//	}
					
				}
				else if ( mp_am_lenguage.value == 1  )
				{
					pPlayer->SetSuitUpdate("!PLTAUNT_ENG", FALSE, SUIT_NEXT_IN_3SEC);

					
				//	switch ( RANDOM_LONG(0,12) )
				//	{
				//	case 0:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/anotherone.wav", 1.0, ATTN_NONE); break;
				//	case 1:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/bastard.wav", 1.0, ATTN_NONE ); break;
				//	case 2:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/better.wav", 1.0, ATTN_NONE); break;
				//	case 3:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/die.wav", 1.0, ATTN_NONE); break;
				//	case 4:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/diebaby.wav", 1.0, ATTN_NONE); break;
				//	case 5:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/greetings.wav", 1.0, ATTN_NONE); break;
				//	case 6:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/hell.wav", 1.0, ATTN_NONE); break;
				//	case 7:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/hoohoo.wav", 1.0, ATTN_NONE); break;
				//	case 8:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/motherfucker.wav", 1.0, ATTN_NONE); break;
				//	case 9:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/mygun.wav", 1.0, ATTN_NONE); break;
				//	case 10:EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/oneshot.wav", 1.0, ATTN_NONE); break;
				//	case 11:EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/yeah.wav", 1.0, ATTN_NONE); break;
				//	case 12:EMIT_SOUND(ENT(pev), CHAN_STATIC, "Player/taunt/eng/youbitch.wav", 1.0, ATTN_NONE); break;
				//	}	
					
				}
				else
				{
					//uh, tomá te mate, pero soy tan malo que no digo nada... ¬¬
				}
				NextTauntTime = gpGlobals->time + 5;
			}
*/

		if(!m_fSurrender)
		{
			//check kung fu style
			if (m_bitsDamageType & (DMG_PLAYERKICK))
			{
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("KUNGFU_STYLE")));
				//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#KUNGFU_STYLE" );
				CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 5) );
			}
			
			//new!
			//check moto style
			/*
			if (m_bitsDamageType & (DMG_CRUSH))
			{
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("CRUSH_STYLE")));
				//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#CRUSH_STYLE" );
				CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 10) );
			}
*/
			//check burn style
			if (m_bitsDamageType & (DMG_BURN))
			{
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("BURN_STYLE")));
				//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#BURN_STYLE" );
				CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 3) );
			}
					
			//check knife style
			if (m_bitsDamageType & (DMG_CLUB))
			{
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("KNIFE_STYLE")));
				//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#KNIFE_STYLE" );
				CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 2) );
									
				CVAR_SET_FLOAT( "score_knifed", ( iMonstersKilledbyKnife += 1) );
			}
		}
	}
//#endif
}
//
// fade out - slowly fades a entity out, then removes it.
//
// DON'T USE ME FOR GIBS AND STUFF IN MULTIPLAYER! 
// SET A FUTURE THINK AND A RENDERMODE!!
void CBaseEntity :: SUB_StartFadeOut ( void )
{
	if (pev->rendermode == kRenderNormal)
	{
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
	}

	pev->solid = SOLID_NOT;
	pev->avelocity = g_vecZero;

	pev->nextthink = gpGlobals->time + 5;//0.1
	SetThink ( SUB_FadeOut );
}

void CBaseEntity :: SUB_FadeOut ( void  )
{
	if ( pev->renderamt > 7 )
	{
		pev->renderamt -= 7;
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else 
	{
		pev->renderamt = 0;
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink ( SUB_Remove );
	}
}

//=========================================================
// WaitTillLand - in order to emit their meaty scent from
// the proper location, gibs should wait until they stop 
// bouncing to emit their scent. That's what this function
// does.
//=========================================================
void CGib :: WaitTillLand ( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	if ( pev->velocity == g_vecZero )
	{
		SetThink (SUB_StartFadeOut);
		pev->nextthink = gpGlobals->time + m_lifeTime;

		// If you bleed, you stink!
		if ( m_bloodColor != DONT_BLEED )
		{
			// ok, start stinkin!
			CSoundEnt::InsertSound ( bits_SOUND_MEAT, pev->origin, 384, 25 );
		}
	}
	else
	{
		// wait and check again in another half second.
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

//
// Gib bounces on the ground or wall, sponges some blood down, too!
//
void CGib :: BounceGibTouch ( CBaseEntity *pOther )
{
	Vector	vecSpot;
	TraceResult	tr;
	
	if ( RANDOM_LONG(0,1) )
		return;// don't bleed everytime

	if (pev->flags & FL_ONGROUND)
	{
		pev->velocity = pev->velocity * 0.9;
		pev->angles.x = 0;
		pev->angles.z = 0;
		pev->avelocity.x = 0;
		pev->avelocity.z = 0;
	}
	else
	{/*
		//spawn particles		
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
		MESSAGE_END();	
*/
		if ( g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED )
		{
			vecSpot = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
			UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -24 ),  ignore_monsters, ENT(pev), & tr);

			UTIL_BloodDecalTrace( &tr, m_bloodColor );

			m_cBloodDecals--; 
		}

		if ( m_material != matNone && RANDOM_LONG(0,2) == 0 )
		{
			float volume;
			float zvel = fabs(pev->velocity.z);
		
			volume = 0.8 * min(1.0, ((float)zvel) / 450.0);

			CBreakable::MaterialSoundRandom( edict(), (Materials)m_material, volume );
		}
	}
}

//
// Sticky gib puts blood on the wall and stays put. 
//
void CGib :: StickyGibTouch ( CBaseEntity *pOther )
{
	Vector	vecSpot;
	TraceResult	tr;
	
	SetThink ( SUB_Remove );
	pev->nextthink = gpGlobals->time + 0.1;

//	pev->nextthink = gpGlobals->time + 10;

	if ( !FClassnameIs( pOther->pev, "worldspawn" ) )
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	UTIL_TraceLine ( pev->origin, pev->origin + pev->velocity * 32,  ignore_monsters, ENT(pev), & tr);

//	UTIL_BloodDecalTrace( &tr, m_bloodColor );
	UTIL_DecalTrace( &tr, DECAL_BLOODDRIP6 );

	pev->velocity = tr.vecPlaneNormal * -1;
	pev->angles = UTIL_VecToAngles ( pev->velocity );
	pev->velocity = g_vecZero; 
	pev->avelocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
}

//
// Throw a chunk
//
void CGib :: Spawn( const char *szGibModel )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->friction = 0.55; // deading the bounce a bit
	
	// sometimes an entity inherits the edict from a former piece of glass,
	// and will spawn using the same render FX or rendermode! bad!
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxNone;
	pev->solid = SOLID_TRIGGER;/// hopefully this will fix the VELOCITY TOO LOW crap

//	pev->solid = SOLID_SLIDEBOX;/// hopefully this will fix the VELOCITY TOO LOW crap
	pev->classname = MAKE_STRING("gib");

	SET_MODEL(ENT(pev), szGibModel);
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink ( WaitTillLand );
	SetTouch ( BounceGibTouch );

	m_material = matNone;
	m_cBloodDecals = 5;// how many blood decals this gib can place (1 per bounce until none remain). 
}

// take health
int CBaseMonster :: TakeHealth (float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return 0;

	// clear out any damage types we healed.
	// UNDONE: generic health should not heal any
	// UNDONE: time-based damage

	m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);
	
	return CBaseEntity::TakeHealth(flHealth, bitsDamageType);
}

/*
============
TakeDamage

The damage is coming from inflictor, but get mad at attacker
This should be the only function that ever reduces health.
bitsDamageType indicates the type of damage sustained, ie: DMG_SHOCK

Time-based damage: only occurs while the monster is within the trigger_hurt.
When a monster is poisoned via an arrow etc it takes all the poison damage at once.



GLOBALS ASSUMED SET:  g_iSkillLevel
============
*/
int CBaseMonster :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	float	flTake;
	Vector	vecDir;

	if (!pev->takedamage)
		return 0;

	if ( !IsAlive() )
	{
		return DeadTakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
	}

	if ( pev->deadflag == DEAD_NO )
	{
		// no pain sound during death animation.
		PainSound();// "Ouch!"
	}

	//!!!LATER - make armor consideration here!
	flTake = flDamage;

	// set damage type sustained
	m_bitsDamageType |= bitsDamageType;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}
	
	//new code
	if ( ( bitsDamageType & DMG_BLAST) || ( bitsDamageType & DMG_BURN) )
	{
		if ( FClassnameIs( pev, "monster_zombie" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "monster_zombie_soldier" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "monster_zombie_human" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "monster_zombie_torso" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "npc_zombie" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "npc_zombie_fast" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "npc_zombie_poison" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "npc_zombine" ))
			StartBurning = TRUE;

		//new
		//new
		/*
		else if (FClassnameIs( pev, "npc_combine" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "npc_combine_random" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "npc_combine_metrocop" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "npc_combine_supersoldier" ))
			StartBurning = TRUE;
		else if (FClassnameIs( pev, "monster_terrorist" ))//test
			StartBurning = TRUE;
		//new*/
		//new

		else if (FClassnameIs( pev, "monster_headcrab" ) || FClassnameIs( pev, "player" ) || FClassnameIs( pev, "npc_headcrab_poison" ) )
		{	
			if (RANDOM_LONG( 0, 99 ) < 40)
			StartBurning = TRUE;
		}
		else
		{

		}
	}

	if  ( bitsDamageType & DMG_SPECIALBURN) 
		StartBurning = TRUE;

	if  ( bitsDamageType & DMG_VELOCITY) 
		b_SlowedMonster = TRUE;

	if  ( bitsDamageType & DMG_EXPAND) 
		ExpandNow = TRUE;

	if  ( bitsDamageType & DMG_SHRINK) 
		SrhinkNow = TRUE;

	//el cuerpo recibe el daño de congelamiento
	//en RunAI, se paraliza el cuerpo y se agregan efectos
	if ( bitsDamageType & DMG_FREEZE )
	{
		FreezeNow = TRUE;
		NextDamageIfFreezed = gpGlobals->time + 1;
	}

	//el cuerpo esta congelado, si toma daño sera gibbeado
	//FIX: agregado delay porque cuando tiene el daño de congelamiento se gibbea al instante
	if ( FreezeNow )
	{	
		if ( bitsDamageType & DMG_FREEZE )
			bitsDamageType = DMG_BULLET;

		if ( NextDamageIfFreezed < gpGlobals->time )
		{
			GibMonster();
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "debris/bustglass1.wav", 0.9, ATTN_NORM, 0, PITCH_NORM);	
		}
	}





	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if ( IsPlayer() )
	{
		if ( pevInflictor )
			pev->dmg_inflictor = ENT(pevInflictor);

		pev->dmg_take += flTake;

		// check for godmode or invincibility
		if ( pev->flags & FL_GODMODE )
		{
			return 0;
		}
	}
//just remove for now
	//EDIT: WHY??
//#if SYS

	// if this is a player, move him around!
	if ( ( !FNullEnt( pevInflictor ) ) && (pev->movetype == MOVETYPE_WALK) && (!pevAttacker || pevAttacker->solid != SOLID_TRIGGER) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
	}

//#endif

	// do the damage
	pev->health -= flTake;
	
	// HACKHACK Don't kill monsters in a script.  Let them break their scripts first
	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		SetConditions( bits_COND_LIGHT_DAMAGE );
		return 0;
	}

	if ( pev->health <= 0 )
	{
		g_pevLastInflictor = pevInflictor;
//****
	if ( mp_am_noblastgibs.value != 0  ) //no es cero esta _Activado_
	{
//		Killed( pevAttacker, GIB_NEVER );
		//****
		if (RANDOM_LONG( 0, 99 ) < 80) 	//sometimes blow the corpses
		{
			Killed( pevAttacker, GIB_NEVER );//okey... no blast!
		}
		else //el resultado fue valido, ejecutar el codigo original
		{
			if ( bitsDamageType & DMG_ALWAYSGIB )//si es "siempre gibs", siempre gibs...
			{
				Killed( pevAttacker, GIB_ALWAYS );
			}
			else if ( bitsDamageType & DMG_NEVERGIB )//si "nunca gibs", nunca gibs...
			{
				Killed( pevAttacker, GIB_NEVER );
			}
			else // sino, normal...
			{
				Killed( pevAttacker, GIB_NORMAL );
			}
		}
		//****
	}
	else //desactivado... Ejecutar actual codigo
	{
		if ( bitsDamageType & DMG_ALWAYSGIB )
		{
			Killed( pevAttacker, GIB_ALWAYS );
		}
		else if ( bitsDamageType & DMG_NEVERGIB )
		{
			Killed( pevAttacker, GIB_NEVER );
		}
		else
		{
			Killed( pevAttacker, GIB_NORMAL );
		}
	}
//*******
		g_pevLastInflictor = NULL;

		return 0;
	}

	// react to the damage (get mad)
	if ( (pev->flags & FL_MONSTER) && !FNullEnt(pevAttacker) )
	{
		//LRC - new behaviours, for m_iPlayerReact.
		if (pevAttacker->flags & FL_CLIENT)
		{
			if (m_iPlayerReact == 2)
			{
				// just get angry.
				Remember( bits_MEMORY_PROVOKED );
			}
			else if (m_iPlayerReact == 3)
			{
				// try to decide whether it was deliberate... if I have an enemy, assume it was just crossfire.
				if ( m_hEnemy == NULL )
				{
					if ( (m_afMemory & bits_MEMORY_SUSPICIOUS) || UTIL_IsFacing( pevAttacker, pev->origin ) )
						Remember( bits_MEMORY_PROVOKED );
					else
						Remember( bits_MEMORY_SUSPICIOUS );
				}
			}
		}

		if ( pevAttacker->flags & (FL_MONSTER | FL_CLIENT) )
		{// only if the attack was a monster or client!
			
			// enemy's last known position is somewhere down the vector that the attack came from.
			if (pevInflictor)
			{
				if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY))
				{
					m_vecEnemyLKP = pevInflictor->origin;
				}
			}
			else
			{
				m_vecEnemyLKP = pev->origin + ( g_vecAttackDir * 64 ); 
			}

			MakeIdealYaw( m_vecEnemyLKP );

			// add pain to the conditions 
			// !!!HACKHACK - fudged for now. Do we want to have a virtual function to determine what is light and 
			// heavy damage per monster class?
			if ( flDamage > 0 )
			{
				SetConditions(bits_COND_LIGHT_DAMAGE);
			}

			if ( flDamage >= 20 )
			{
				SetConditions(bits_COND_HEAVY_DAMAGE);
			}
		}
	}

//	pev->frame = RANDOM_FLOAT ( 0, 20 );// pose a unos instantes antes... 255 es el final
//	ResetSequenceInfo( );

	return 1;
}


//=========================================================
// DeadTakeDamage - takedamage function called when a monster's
// corpse is damaged.
//=========================================================
int CBaseMonster :: DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	Vector			vecDir;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}
	//oh, q lindo :)
	
	if ( phys_simulateragdoll.value != 0  ) //no es cero esta _Activado
	{
		pev->frame = RANDOM_FLOAT ( 223, 225 );// pose a unos instantes antes... 255 es el final
		ResetSequenceInfo( );
	}

	if ( FClassnameIs( pev, "monster_human_ak" ) )
	{
		if ( ( bitsDamageType & DMG_BULLET) || ( bitsDamageType & DMG_SHOTGUN) )
		{
			if ( flDamage >= 10 )
			{
				if ( m_LastHitGroup == HITGROUP_HEAD)
					pev->skin = 4; //head
				else if ( m_LastHitGroup == HITGROUP_LEFTARM)
					pev->skin = 1;//arms
				else if ( m_LastHitGroup == HITGROUP_RIGHTARM)
					pev->skin = 1;//arms
				else if ( m_LastHitGroup == HITGROUP_LEFTLEG)
					pev->skin = 2;//legs
				else if ( m_LastHitGroup == HITGROUP_RIGHTLEG)
					pev->skin = 2;//legs
				else
					pev->skin = 3; //chest
			}
		}
	}
	

//SP ok...
	if ( phys_enable.value != 0  ) //no es cero esta _Activado_
	{
		if ( phys_movecorpses.value != 0  ) //no es cero esta _Activado_
		{
//			if ( !(pev->deadflag == DEAD_DEAD))//si esta muerto
//			{
				if ( FClassnameIs( pev, "monster_mini_gargantua" ) ) { }
				else
				{
					pev->flags &= ~FL_ONGROUND;
					pev->origin.z += 1;
					
					// let the damage scoot the corpse around a bit.
					if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
					{
						pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
					}
//				}
			}
		}
	}

	// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
	if ( bitsDamageType & DMG_GIB_CORPSE )
	{
		if ( pev->health <= flDamage )
		{
			pev->health = -50;
			Killed( pevAttacker, GIB_ALWAYS );
			return 0;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		pev->health -= flDamage * 0.1;
	}
	
	return 1;
}


float CBaseMonster :: DamageForce( float damage )
{ 
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
/*	
	if ( force > 1000.0) 
	{
		force = 1000.0;
	}
	*/
	if ( force > 80.0) 
	{
		force = 80.0;
	}
	return force;
}

//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
// 
// only damage ents that can clearly be seen by the explosion!

	
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType )
{
	CBaseEntity *pEntity = NULL;
	TraceResult	tr;
	float		flAdjustedDamage, falloff;
	Vector		vecSpot;

	if ( flRadius )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) == CONTENTS_WATER);

	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;

	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			vecSpot = pEntity->BodyTarget( vecSrc );
			
			UTIL_TraceLine ( vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr );

			if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
			{// the explosion can 'see' this entity, so hurt them!
				if (tr.fStartSolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}
				
				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;
			
				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;
				}
			
				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage( );
					pEntity->TraceAttack( pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), &tr, bitsDamageType );
					ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					pEntity->TakeDamage ( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType );
				}
			}
		}
	}
}


void CBaseMonster :: RadiusDamage(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}


void CBaseMonster :: RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}


//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount 
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
//
// Used for many contact-range melee attacks. Bites, claws, etc.
//=========================================================
CBaseEntity* CBaseMonster :: CheckTraceHullAttack( float flDist, int iDamage, int iDmgType )
{
	TraceResult tr;

	if (IsPlayer())
		UTIL_MakeVectors( pev->angles );
	else
		UTIL_MakeAimVectors( pev->angles );

	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist );

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		if ( iDamage > 0 )
		{
			pEntity->TakeDamage( pev, pev, iDamage, iDmgType );
		}

		return pEntity;
	}

	return NULL;
}


//=========================================================
// FInViewCone - returns true is the passed ent is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster :: FInViewCone ( CBaseEntity *pEntity )
{
	Vector2D	vec2LOS;
	float	flDot;

	UTIL_MakeVectors ( pev->angles );
	
	vec2LOS = ( pEntity->pev->origin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

	if ( flDot > m_flFieldOfView )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster :: FInViewCone ( Vector *pOrigin )
{
	Vector2D	vec2LOS;
	float		flDot;

	UTIL_MakeVectors ( pev->angles );
	
	vec2LOS = ( *pOrigin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

	if ( flDot > m_flFieldOfView )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target
//=========================================================
BOOL CBaseEntity :: FVisible ( CBaseEntity *pEntity )
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	Vector		vecTargetOrigin;
	
	if (FBitSet( pEntity->pev->flags, FL_NOTARGET ))
		return FALSE;

	// don't look through water
	if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) 
		|| (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
		return FALSE;

	vecLookerOrigin = pev->origin + pev->view_ofs;//look through the caller's 'eyes'
	vecTargetOrigin = pEntity->EyePosition();

	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target vector
//=========================================================
BOOL CBaseEntity :: FVisible ( const Vector &vecOrigin )
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	
	vecLookerOrigin = EyePosition();//look through the caller's 'eyes'

	UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}

/*
================
TraceAttack
================
*/
void CBaseEntity::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	if ( pev->takedamage )
	{
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );

		int blood = BloodColor();
		
		if ( blood != DONT_BLEED )
		{
			SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
			TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
		}
	}
}


/*
//=========================================================
// TraceAttack
//=========================================================
void CBaseMonster::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	ALERT ( at_console, "%d\n", ptr->iHitgroup );


	if ( pev->takedamage )
	{
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );

		int blood = BloodColor();
		
		if ( blood != DONT_BLEED )
		{
			SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
		}
	}
}
*/

//=========================================================
// TraceAttack
//=========================================================
void CBaseMonster :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
//	CBaseEntity *pPlayer = GetClassPtr((CBaseEntity *)pevAttacker);//ONLY TEST!!!
//	char	szText[ 512 ];

	if ( pev->takedamage )
	{
		m_LastHitGroup = ptr->iHitgroup;

		if (bitsDamageType & (DMG_SLASH | DMG_BLAST))//FIX: anybody can't make headshots using knifes or crowbars...
		ptr->iHitgroup = HITGROUP_GENERIC;

		switch ( ptr->iHitgroup )
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:

			flDamage *= gSkillData.monHead;
			break;
		case HITGROUP_CHEST:

			flDamage *= gSkillData.monChest;
			break;
		case HITGROUP_STOMACH:

			flDamage *= gSkillData.monStomach;
			break;
		case HITGROUP_LEFTARM:
			if ( phys_enable.value != 0  ) //no es cero esta _Activado_
			{
				if ( phys_movehit.value != 0  ) //no es cero esta _Activado_
				{
					if ( IsAlive())//[DNS] fix bug
					{
						pev->angles.y += 10;//this rotate the monster
					//	pev->avelocity.y += 100;//velocity
					}
				}
			}
			flDamage *= gSkillData.monArm;
			break;
		case HITGROUP_RIGHTARM:
			if ( phys_enable.value != 0  ) //no es cero esta _Activado_
			{
				if ( phys_movehit.value != 0  ) //no es cero esta _Activado_
				{
					if ( IsAlive())//[DNS] fix bug
					pev->angles.y -= 10;
				}
			}
			flDamage *= gSkillData.monArm;
			break;
		case HITGROUP_LEFTLEG:
			if ( phys_enable.value != 0  ) //no es cero esta _Activado_
			{
				if ( phys_movehit.value != 0  ) //no es cero esta _Activado_
				{
					if ( IsAlive())//[DNS] fix bug
					pev->angles.y += 10;
				}
			}
			flDamage *= gSkillData.monLeg;
			break;
		case HITGROUP_RIGHTLEG:
			if ( phys_enable.value != 0  ) //no es cero esta _Activado_
			{
				if ( phys_movehit.value != 0  ) //no es cero esta _Activado_
				{
					if ( IsAlive())//[DNS] fix bug
					pev->angles.y -= 10;
				}
			}
			flDamage *= gSkillData.monLeg;
			break;

		default:
			break;
		}

		if ( CVAR_GET_FLOAT("slowmo" ) != 0 )			
		CGib::SpawnStickyGibs(pev, pev->origin ,8);
		else
		{
			TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
			SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
		}
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
	}
}
//-----------------------------------
// Vector CBaseEntity::FireBulletsThroughWalls
// Funcion para disparar balas que atraviesan paredes
//
// Basado en un tutorial de ViRi- (http://www.planethalflife.com/hlprogramming/tutorial.asp?i=6)
// (C) Rodrigo 'r2d2rigo' Diaz 2005
// The Last Citizen Lead Programmer
//-----------------------------------

Vector CBaseEntity::FireBulletsThroughWalls ( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
// Variables que necesitaremos mas tarde
static int tracerCount;
TraceResult tr,beam_tr;
Vector vecRight = gpGlobals->v_right;
Vector vecUp = gpGlobals->v_up;
float x, y, z;

if ( pevAttacker == NULL )
 pevAttacker = pev;  // Si el atacante es nulo, pasa a ser esta entidad

ClearMultiDamage();
gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

for ( ULONG iShot = 1; iShot <= cShots; iShot++ ) // Repetimos el siguiente bucle de instrucciones para cada bala
{
 // Añadimos el vector gaussiano para desviar las balas
 x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
 y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
 z = x * x + y * y;

 Vector vecDir = vecDirShooting +
     x * vecSpread.x * vecRight +
     y * vecSpread.y * vecUp;
 Vector vecEnd;

 vecEnd = vecSrc + vecDir * flDistance;
 // Se interpone algo entre el origen y el destino del disparo?
 UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
 
 if (tr.flFraction != 1.0) // Hemos tocado algo
 {
  CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
		BOOL b_CanMakeParticles = TRUE;

		if ( pEntity->IsBSPModel() )
		{
			char chTextureType;
			char szbuffer[64];//64
			const char *pTextureName;
			float rgfl1[3];
			float rgfl2[3];
			float fattn = ATTN_NORM;

			chTextureType = 0;

			if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				// hit body
				chTextureType = CHAR_TEX_FLESH;
			else
			{
				vecSrc.CopyToArray(rgfl1);
				vecEnd.CopyToArray(rgfl2);

				if (pEntity)
					pTextureName = TRACE_TEXTURE( ENT(pEntity->pev), rgfl1, rgfl2 );
				else
					pTextureName = TRACE_TEXTURE( ENT(0), rgfl1, rgfl2 );
					
				if ( pTextureName )
				{
					if (*pTextureName == '-' || *pTextureName == '+')
						pTextureName += 2;

					if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
						pTextureName++;
					strcpy(szbuffer, pTextureName);
					szbuffer[CBTEXTURENAMEMAX - 1] = 0;
					chTextureType = TEXTURETYPE_Find(szbuffer);	
				
					if (*pTextureName == 'null')
					{
						b_CanMakeParticles = FALSE;
					}
				}
			}

			if ( FClassnameIs(pEntity->pev, "func_collision"))
			{
				b_CanMakeParticles = FALSE;
			}

			float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range

			if ( CVAR_GET_FLOAT( "r_paintball" ) == 0 )
			if ( CVAR_GET_FLOAT( "cl_wallpuff" ) >= 1 )
			{			
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					//the decal it's going to put in a solid. We only can see the decal only on
					//glass brushes because the decals cant be placed at a transparent texture
					//UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
					//EDIT: there is some problems, so put it down

					if (VARS(tr.pHit)->playerclass == 1)
					{//this is a really really glass, so make sounds
						UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
					}
				}
				else
				{
					if (chTextureType == CHAR_TEX_METAL)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
					
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_VENT)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
					
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_COMPUTER)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
	
						UTIL_Sparks( tr.vecEndPos );

						switch ( RANDOM_LONG(0,1) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark5.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark6.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
										
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_WOOD)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));						
					}
					else if (chTextureType == CHAR_TEX_BLUE)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_RED)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_YELLOW)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}				
					else if (chTextureType == CHAR_TEX_BLACK)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_SNOW)
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));							
					}
					else if (chTextureType == CHAR_TEX_DIRT)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));
					}
									
					else if (chTextureType == CHAR_TEX_TILE)		
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}	
					
					if (b_CanMakeParticles)
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
							if (chTextureType == CHAR_TEX_BLUE)		
							WRITE_SHORT(iDefaultHitBlue);
							else if (chTextureType == CHAR_TEX_RED)		
							WRITE_SHORT(iDefaultHitRed);
							else if (chTextureType == CHAR_TEX_YELLOW)		
							WRITE_SHORT(iDefaultHitYellow);
							else if (chTextureType == CHAR_TEX_BLACK)		
							WRITE_SHORT(iDefaultHitBlack);
							else if (chTextureType == CHAR_TEX_DIRT)		
							WRITE_SHORT(iDefaultHitGreen);
							else if (chTextureType == CHAR_TEX_WOOD)
							WRITE_SHORT(iDefaultHitWood1);
							else
							{	
								if ( RANDOM_LONG(0,99) < 40 )// 39.6 % of chance
								WRITE_SHORT(iDefaultWallSmokeLong);
								else
								WRITE_SHORT(iDefaultWallSmoke);
							}
						MESSAGE_END();
					}
				}//eo cvar check
			}//eo bsp check
		}//eo tr.fraction
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
  if ( iDamage )
  {
   // Si el daño a realizar es mayor que 0, hacemos daño a la entidad disparada
   pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | DMG_NEVERGIB );
   
   TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
   DecalGunshot( &tr, iBulletType );
  } 
  // Pintamos los agujeros de bala para cada tipo de bala disponible
  else switch(iBulletType)
  {
  default:

case BULLET_PLAYER_MP5:		
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, 
vecDir, &tr, DMG_BULLET); break;

case BULLET_PLAYER_M16:
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgM16, 
vecDir, &tr, DMG_BULLET); break;

case BULLET_PLAYER_AK74:
	
	/*
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos );
	WRITE_BYTE( TE_EXPLOSION );
	WRITE_COORD( tr.vecEndPos.x );
	WRITE_COORD( tr.vecEndPos.y );
	WRITE_COORD( tr.vecEndPos.z );
	WRITE_SHORT( g_sModelIndexFireball );
	WRITE_BYTE( 5 ); // scale
	WRITE_BYTE( 15 ); // framerate
	WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();

	::RadiusDamage( tr.vecEndPos, pev, pev, 50, 128, CLASS_NONE, DMG_BLAST );
	
	UTIL_DecalTrace( &tr, DECAL_SMALLSCORCH1 + RANDOM_LONG(0,2) );
	*/

pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgAK74, 
vecDir, &tr, DMG_BULLET); break;

case BULLET_PLAYER_DEAGLE:
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgDeagle, 
vecDir, &tr, DMG_BULLET); break;

case BULLET_PLAYER_SNIPER:
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgSniper, 
vecDir, &tr, DMG_BULLET); break;

case BULLET_PLAYER_M82:
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgM82, 
vecDir, &tr, DMG_BULLET); break;

case BULLET_PLAYER_SCOUT:
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgScout, 
vecDir, &tr, DMG_BULLET); break;

case BULLET_PLAYER_FAMAS:
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgFamas, 
vecDir, &tr, DMG_BULLET);  break;

case BULLET_PLAYER_M249:
pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgM249, 
vecDir, &tr, DMG_BULLET); break;
		
  case BULLET_PLAYER_9MM:  
   pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET); 
   break;

  case BULLET_PLAYER_BUCKSHOT:
   pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET | DMG_SHOTGUN); 
   break;
  
  case BULLET_PLAYER_357: 
   pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET); 
   break;

  case BULLET_NONE: // Golpe de Crowbar
   pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
   TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);

   if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
   {
    UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
   }

   break;
  }
 }
 // Hacemos otra comprobacion, en este caso 8 unidades hacia delante; la distancia maxima de pared que atraviesan los disparos
 UTIL_TraceLine( tr.vecEndPos + gpGlobals->v_forward * 8, vecEnd, dont_ignore_monsters, ENT(pev), &beam_tr);
 if (!beam_tr.fAllSolid)
 {
  // Otra comprobacion hacia atras, para pintar el agujero de salida de la bala
  UTIL_TraceLine( beam_tr.vecEndPos, tr.vecEndPos, dont_ignore_monsters, ENT(pev), &beam_tr);
  switch(iBulletType)
   {
   default:
	   //mp5 m16 ak uxi p90 ber92 glock usas deagle sniper famas m249
	case BULLET_PLAYER_MP5:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_MP5 );
    break;
	case BULLET_PLAYER_M16:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_M16 );
    break;
	case BULLET_PLAYER_AK74:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_AK74 );
    break;
	case BULLET_PLAYER_DEAGLE:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_DEAGLE );
    break;
	case BULLET_PLAYER_SNIPER:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_SNIPER );
    break;
	case BULLET_PLAYER_M82:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_M82 );
    break;
	case BULLET_PLAYER_SCOUT:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_SCOUT );
    break;

	case BULLET_PLAYER_FAMAS:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_FAMAS );
    break;
	case BULLET_PLAYER_M249:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_M249 );
    break;

   case BULLET_PLAYER_9MM:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_9MM );
    break;

   case BULLET_PLAYER_BUCKSHOT: 
    DecalGunshot( &beam_tr, BULLET_PLAYER_BUCKSHOT );
    break;
   
   case BULLET_PLAYER_357: 
    DecalGunshot( &beam_tr, BULLET_PLAYER_357);
    break;
    
   case BULLET_NONE: // Golpe de Crowbar
    DecalGunshot( &beam_tr, BULLET_PLAYER_CROWBAR );
    break;
   }
  // Ahora si, esta es la trayectoria final del disparo tras atravesar la pared
  UTIL_TraceLine( beam_tr.vecEndPos, vecEnd, dont_ignore_monsters, ENT(pev), &beam_tr);
  switch(iBulletType)
   // Volvemos a pintar el agujero de la bala
   {
   default:
	case BULLET_PLAYER_MP5:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_MP5 );
    break;
	case BULLET_PLAYER_M16:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_M16 );
    break;
	case BULLET_PLAYER_AK74:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_AK74 );
    break;
	case BULLET_PLAYER_DEAGLE:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_DEAGLE );
    break;
	case BULLET_PLAYER_SNIPER:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_SNIPER );
    break;
	case BULLET_PLAYER_M82:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_M82 );
    break;
		case BULLET_PLAYER_SCOUT:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_SCOUT );
    break;

	case BULLET_PLAYER_FAMAS:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_FAMAS );
    break;
	case BULLET_PLAYER_M249:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_M249 );
    break;

   case BULLET_PLAYER_9MM:  
    DecalGunshot( &beam_tr, BULLET_PLAYER_9MM );
    break;

   case BULLET_PLAYER_BUCKSHOT: 
    DecalGunshot( &beam_tr, BULLET_PLAYER_BUCKSHOT );
    break;
   
   case BULLET_PLAYER_357: 
    DecalGunshot( &beam_tr, BULLET_PLAYER_357);
    break;
    
   case BULLET_NONE: // Golpe de Crowbar
    DecalGunshot( &beam_tr, BULLET_PLAYER_CROWBAR );
    break;
   }

  CBaseEntity *pEnt = CBaseEntity::Instance(beam_tr.pHit);
  
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
		if ( pEnt->IsBSPModel() )
		{
			char chTextureType;
			char szbuffer[64];
			const char *pTextureName;
			float rgfl1[3];
			float rgfl2[3];
			float fattn = ATTN_NORM;

			chTextureType = 0;

			if (pEnt && pEnt->Classify() != CLASS_NONE && pEnt->Classify() != CLASS_MACHINE)
				// hit body
				chTextureType = CHAR_TEX_FLESH;
			else
			{
				vecSrc.CopyToArray(rgfl1);
				vecEnd.CopyToArray(rgfl2);

				if (pEnt)
					pTextureName = TRACE_TEXTURE( ENT(pEnt->pev), rgfl1, rgfl2 );
				else
					pTextureName = TRACE_TEXTURE( ENT(0), rgfl1, rgfl2 );
					
				if ( pTextureName )
				{
					if (*pTextureName == '-' || *pTextureName == '+')
						pTextureName += 2;

					if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
						pTextureName++;
					strcpy(szbuffer, pTextureName);
					szbuffer[CBTEXTURENAMEMAX - 1] = 0;
					chTextureType = TEXTURETYPE_Find(szbuffer);	
				}
			//	ALERT ( at_console, "texture hit: %s\n", szbuffer);
			}

			
			float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range
			BOOL	b_CanMakeParticles = TRUE;

			if ( CVAR_GET_FLOAT( "r_paintball" ) == 0 )
			if ( CVAR_GET_FLOAT( "cl_wallpuff" ) >= 1 )
			{			
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					//the decal it's going to put in a solid. We only can see the decal only on
					//glass brushes because the decals cant be placed at a transparent texture
					//UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
					//EDIT: there is some problems, so put it down

					if (VARS(tr.pHit)->playerclass == 1)
					{//this is a really really glass, so make sounds
						UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
					}
				}
				else
				{
					if (chTextureType == CHAR_TEX_METAL)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
					
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_VENT)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
					
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_COMPUTER)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
	
						UTIL_Sparks( tr.vecEndPos );

						switch ( RANDOM_LONG(0,1) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark5.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark6.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
										
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_WOOD)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));						
					}
					else if (chTextureType == CHAR_TEX_BLUE)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_RED)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_YELLOW)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}				
					else if (chTextureType == CHAR_TEX_BLACK)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_SNOW)
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));							
					}
					else if (chTextureType == CHAR_TEX_DIRT)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));
					}
									
					else if (chTextureType == CHAR_TEX_TILE)		
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}

					else if (chTextureType == CHAR_TEX_SLIME)		
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
					}

					else
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}	
					
					if (b_CanMakeParticles)
					{
						if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
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
								if (chTextureType == CHAR_TEX_BLUE)		
								WRITE_SHORT(iDefaultHitBlue);
								else if (chTextureType == CHAR_TEX_RED)		
								WRITE_SHORT(iDefaultHitRed);
								else if (chTextureType == CHAR_TEX_YELLOW)		
								WRITE_SHORT(iDefaultHitYellow);
								else if (chTextureType == CHAR_TEX_BLACK)		
								WRITE_SHORT(iDefaultHitBlack);
								else if (chTextureType == CHAR_TEX_DIRT)		
								WRITE_SHORT(iDefaultHitGreen);
								else if (chTextureType == CHAR_TEX_WOOD)
								WRITE_SHORT(iDefaultHitWood1);
								else if (chTextureType == CHAR_TEX_SLIME)
								WRITE_SHORT(iDefaultHitSlime);
								else
								{	
									if ( RANDOM_LONG(0,99) < 40 )// 39.6 % of chance
									WRITE_SHORT(iDefaultWallSmokeLong);
									else
									WRITE_SHORT(iDefaultWallSmoke);
								}
							MESSAGE_END();
						}
					}
				}//eo cvar check
			}//eo bsp check
		}//eo tr.fraction
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
  if ( iDamage )
  {
   // Si el daño a realizar es mayor que 0, hacemos daño a la entidad disparada
   pEnt->TraceAttack(pevAttacker, iDamage, vecDir, &beam_tr, DMG_BULLET | DMG_NEVERGIB );
   
   TEXTURETYPE_PlaySound(&beam_tr, vecSrc, vecEnd, iBulletType);
   DecalGunshot( &beam_tr, iBulletType );
  } 
  // Pintamos los agujeros de bala para cada tipo de bala disponible
  else switch(iBulletType)
  {
  default:
	  case BULLET_PLAYER_MP5:		
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, 
vecDir, &beam_tr, DMG_BULLET); break;

case BULLET_PLAYER_M16:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgM16, 
vecDir, &beam_tr, DMG_BULLET); break;

case BULLET_PLAYER_AK74:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgAK74, 
vecDir, &beam_tr, DMG_BULLET); break;

case BULLET_PLAYER_DEAGLE:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgDeagle, 
vecDir, &beam_tr, DMG_BULLET); break;

case BULLET_PLAYER_SNIPER:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgSniper, 
vecDir, &beam_tr, DMG_BULLET); break;
case BULLET_PLAYER_M82:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgM82, 
vecDir, &beam_tr, DMG_BULLET); break;

   case BULLET_PLAYER_SCOUT:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgScout, 
vecDir, &beam_tr, DMG_BULLET); break;

case BULLET_PLAYER_FAMAS:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgFamas, 
vecDir, &beam_tr, DMG_BULLET);  break;

case BULLET_PLAYER_M249:
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgM249, 
vecDir, &beam_tr, DMG_BULLET); break;

  case BULLET_PLAYER_9MM:  
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &beam_tr, DMG_BULLET); 
   break;

  case BULLET_PLAYER_BUCKSHOT://buckshot?
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &beam_tr, DMG_BULLET | DMG_SHOTGUN); 
   break;
  
  case BULLET_PLAYER_357: 
   pEnt->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &beam_tr, DMG_BULLET); 
   break;

  case BULLET_NONE: // Golpe de Crowbar
   pEnt->TraceAttack(pevAttacker, 50, vecDir, &beam_tr, DMG_CLUB);
   TEXTURETYPE_PlaySound(&beam_tr, vecSrc, vecEnd, iBulletType);

   if ( !FNullEnt(beam_tr.pHit) && VARS(beam_tr.pHit)->rendermode != 0)
   {
    UTIL_DecalTrace( &beam_tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
   }
   break;
  }
 }
 // Si estamos bajo el agua, hacer una estela de burbujas
 UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
}
// Resolvemos el daño final
ApplyMultiDamage(pev, pevAttacker);

// Devolvemos el vector trayectoria de la bala
return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}


class CCrossbowFollower : public CBaseEntity
{
    void Spawn( void );
    void Precache( void );
    int  Classify ( void );
    void EXPORT FollowThink( void );
    void EXPORT FollowTouch( CBaseEntity *pOther );

    int m_iKillTime;
public:
    static CCrossbowFollower *BoltCreate( void );
};

CCrossbowFollower *CCrossbowFollower::BoltCreate( void )
{
    //Hier wird ein neuer "Follower" erstellt.
    CCrossbowFollower *pFollower = GetClassPtr( (CCrossbowFollower *)NULL );
    pFollower->pev->classname = MAKE_STRING("boltfollower");
    pFollower->Spawn();

    return pFollower;
}
void CCrossbowFollower::Spawn( )
{
    Precache( );
    pev->movetype = MOVETYPE_FLY;
    pev->solid = SOLID_BBOX;    // Leider nötig. 
                    // Ich würde das lieber auf SOLID_NOT stellen,
                    // aber dann können wir keinen SetTouch mehr verwenden. :-(
    pev->gravity = 0.5;

    SET_MODEL(ENT(pev), "models/crossbow_bolt.mdl");     // Wir brauchen irgendein Model.
    pev->renderamt = 0;                     // Aber das wird hier gleich wieder
    pev->rendermode = kRenderTransTexture;            // unsichtbar gemacht.

    UTIL_SetOrigin( pev, pev->origin );
    UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

    SetTouch( FollowTouch );
    SetThink( FollowThink );
    pev->nextthink = gpGlobals->time + 0.2;

    m_iKillTime = gpGlobals->time + 2.0;            // Sozusagen die "Notbremse" falls wir
                    // aus irgendeinem Grund aus dem Level "rausfliegen" sollten.
}
void CCrossbowFollower::FollowThink( void )
{
    pev->nextthink = gpGlobals->time + 0.01;
    
    if ( gpGlobals->time >= m_iKillTime )  // Hier wird die vergangene Zeit überprüft
    {
        SET_VIEW( pev->owner, pev->owner );
        UTIL_Remove(this);
    }
}
void CCrossbowFollower::FollowTouch( CBaseEntity *pOther )
{
    // Super-Wichtig. Wenn wir mit unserem Verfolger irgendwo anstoßen, wird er gelöscht.
    SetTouch( NULL );
    SetThink( NULL );
    SET_VIEW( pev->owner, pev->owner );     // Das hier erkläre ich weiter unten.
    UTIL_Remove(this);

}
void CCrossbowFollower::Precache( )
{
    PRECACHE_MODEL ("models/crossbow_bolt.mdl");
}
int    CCrossbowFollower :: Classify ( void )
{
    return    CLASS_NONE;
}

CBullet *CBullet::BulletCreate( void )
{
    CBullet *pBullet = GetClassPtr( (CBullet *)NULL );
    pBullet->pev->classname = MAKE_STRING("bullet");
    pBullet->Spawn();

    return pBullet;
}

void CBullet:: Spawn( void )
{
	//precache resources before spawn()
	Precache();

	pev->movetype = MOVETYPE_FLY;
//	pev->classname = MAKE_STRING( "bullet" );
	
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/weapons/bullet.mdl");

	UTIL_SetSize( pev, Vector( 0, 0, 0), Vector(0, 0, 0) );

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(entindex());	// entity
		WRITE_SHORT(g_sModelIndexLaser );	// model
		WRITE_BYTE( 5 ); // life
		WRITE_BYTE( 1 );  // width
		WRITE_BYTE( 222 );   // r, g, b
		WRITE_BYTE( 222 );   // r, g, b
		WRITE_BYTE( 222 );   // r, g, b
		WRITE_BYTE( 100 );	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)
		  
	SetThink( PreThink );             // Das hier ist geändert
    pev->nextthink = gpGlobals->time + 0.2;

//	SetThink( Think );
//	pev->nextthink = gpGlobals->time + 0.1;
}
LINK_ENTITY_TO_CLASS( bullet, CBullet );

void CBullet::Precache( )
{
//	m_iTrail = PRECACHE_MODEL("sprites/streak.spr");
}

void CBullet::Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CBullet *pShoot = GetClassPtr( (CBullet *)NULL );
	pShoot->Spawn();

//	pShoot->BulletCreate();

	UTIL_SetOrigin( pShoot->pev, vecStart );
	pShoot->pev->velocity = vecVelocity;
	pShoot->pev->owner = ENT(pevOwner);
}

void CBullet :: Touch ( CBaseEntity *pOther )
{
	//dont hit itself
	if (FClassnameIs(pOther->pev, "bullet"))
	return;

	TraceResult tr;
	int		iPitch;

	// splat sound
	iPitch = RANDOM_FLOAT( 90, 110 );

	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "player/damage/ric_conc1.wav", 1, ATTN_NORM, 0, iPitch );	

	if ( !pOther->pev->takedamage )
	{
		// make a splat on the wall
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT( pev ), &tr );
/*		UTIL_DecalTrace(&tr, DECAL_GUNSHOT1 + RANDOM_LONG(0,3));
				
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos );
			WRITE_BYTE( TE_BREAKMODEL);	// position
			WRITE_COORD( tr.vecEndPos.x );
			WRITE_COORD( tr.vecEndPos.y );
			WRITE_COORD( tr.vecEndPos.z );		// size
			WRITE_COORD( 1 );
			WRITE_COORD( 1 );
			WRITE_COORD( 1 );
			WRITE_COORD( 55 );
			WRITE_COORD( 55 );
			WRITE_COORD( 55 );
			WRITE_BYTE( 10 ); 

			WRITE_SHORT( g_sConcreteGib );	//model id#

			WRITE_BYTE( 10 ); //5 gibs
			WRITE_BYTE( 2 );// 1 seconds

			WRITE_BYTE( 0 );//no sounds
		MESSAGE_END();
*/
	}
	else
	{
		pOther->TakeDamage ( pev, pev, gSkillData.hgruntShotgunPellets, DMG_BULLET );
	}

	SetTouch( NULL );
	SetThink( NULL );

	UTIL_Remove( this );
}

void CBullet::PreThink( void )
{
    pev->nextthink = gpGlobals->time + 0.01;
/*
 //   CBullet *pFollower = CBullet::BulletCreate();
	CBullet *pFollower = GetClassPtr( (CBullet *)NULL );

    pFollower->pev->origin = pev->origin - gpGlobals->v_forward * 10; //Etwas nach hinten verschoben
    pFollower->pev->angles = pev->angles;
    pFollower->pev->v_angle = UTIL_VecToAngles(pev->angles);
    pFollower->pev->owner = pev->owner;
    pFollower->pev->velocity = pev->velocity;
    pFollower->pev->speed = pev->speed;

    pFollower->pev->avelocity.z = -30; // Das bringt Action rein. Damit dreht sich die Kamera während des Fluges.
                        //Wer's nicht mag, macht es raus.

    SET_VIEW( pev->owner, pFollower->edict() ); //Das ist es endlich!
	*/
}

void CBullet :: Think ( )
{
	if ( CVAR_GET_FLOAT("slowmo" ) == 0 )			
	{
		SetTouch( NULL );

		UTIL_Remove( this );
	}
	else
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/bullet.wav", 1.0, ATTN_NORM);
	}
	
	pev->nextthink = gpGlobals->time + 0.1;
}



/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Monsters.
================
*/
void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker )
{
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

	if (pPlayer->m_fSlowMotionOn)
		return;

	static int tracerCount;
	int tracer;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		// get circular gaussian spread
		float x, y, z;
		do {
			x = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			y = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			z = x*x+y*y;
		} while (z > 1);

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);

		tracer = 0;
		if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
		{
			Vector vecTracerSrc;

			if ( IsPlayer() )
			{// adjust tracer position for player
				vecTracerSrc = vecSrc + Vector ( 0 , 0 , -4 ) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			}
			else
			{
				vecTracerSrc = vecSrc;
			}
			
			if ( iTracerFreq != 1 )		// guns that always trace also always decal
				tracer = 1;
			switch( iBulletType )
			{
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:
			default:
				MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecTracerSrc );
					WRITE_BYTE( TE_TRACER );
					WRITE_COORD( vecTracerSrc.x );
					WRITE_COORD( vecTracerSrc.y );
					WRITE_COORD( vecTracerSrc.z );
					WRITE_COORD( tr.vecEndPos.x );
					WRITE_COORD( tr.vecEndPos.y );
					WRITE_COORD( tr.vecEndPos.z );
				MESSAGE_END();
				break;
			}
		}

		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
		BOOL b_CanMakeParticles = TRUE;

		if ( pEntity->IsBSPModel() )
		{
			char chTextureType;
			char szbuffer[64];//64
			const char *pTextureName;
			float rgfl1[3];
			float rgfl2[3];
			float fattn = ATTN_NORM;

			chTextureType = 0;

			if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				// hit body
				chTextureType = CHAR_TEX_FLESH;
			else
			{
				vecSrc.CopyToArray(rgfl1);
				vecEnd.CopyToArray(rgfl2);

				if (pEntity)
					pTextureName = TRACE_TEXTURE( ENT(pEntity->pev), rgfl1, rgfl2 );
				else
					pTextureName = TRACE_TEXTURE( ENT(0), rgfl1, rgfl2 );
					
				if ( pTextureName )
				{
					if (*pTextureName == '-' || *pTextureName == '+')
						pTextureName += 2;

					if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
						pTextureName++;
					strcpy(szbuffer, pTextureName);
					szbuffer[CBTEXTURENAMEMAX - 1] = 0;
					chTextureType = TEXTURETYPE_Find(szbuffer);	
				
					if (*pTextureName == 'null')
					{
						b_CanMakeParticles = FALSE;
					}
				}
			}

			if ( FClassnameIs(pEntity->pev, "func_collision"))
			{
				b_CanMakeParticles = FALSE;
			}

			float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range

			if ( CVAR_GET_FLOAT( "r_paintball" ) == 0 )
			if ( CVAR_GET_FLOAT( "cl_wallpuff" ) >= 1 )
			{			
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					//the decal it's going to put in a solid. We only can see the decal only on
					//glass brushes because the decals cant be placed at a transparent texture
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				
					switch ( RANDOM_LONG(0,2) )
					{
						case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass1.wav", flVolume, ATTN_NORM, 0, 100); break;
						case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass2.wav", flVolume, ATTN_NORM, 0, 100); break;
						case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass3.wav", flVolume, ATTN_NORM, 0, 100); break;
					}

					if (VARS(tr.pHit)->playerclass == 1)
					{
						//EDIT: this is a brekeable glass
						/*
						UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
						*/
					}
				}
				else
				{
					if (chTextureType == CHAR_TEX_METAL)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
					
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_VENT)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
					
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_COMPUTER)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
	
						UTIL_Sparks( tr.vecEndPos );

						switch ( RANDOM_LONG(0,1) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark5.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark6.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
										
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_WOOD)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));						
					}
					else if (chTextureType == CHAR_TEX_BLUE)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_RED)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_YELLOW)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}				
					else if (chTextureType == CHAR_TEX_BLACK)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_SNOW)
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));							
					}
					else if (chTextureType == CHAR_TEX_DIRT)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));
					}
									
					else if (chTextureType == CHAR_TEX_TILE)		
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}

					else if (chTextureType == CHAR_TEX_SLIME)		
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
					}

					else
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}	
					
					if (b_CanMakeParticles)
					{
						if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
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
								if (chTextureType == CHAR_TEX_BLUE)		
								WRITE_SHORT(iDefaultHitBlue);
								else if (chTextureType == CHAR_TEX_RED)		
								WRITE_SHORT(iDefaultHitRed);
								else if (chTextureType == CHAR_TEX_YELLOW)		
								WRITE_SHORT(iDefaultHitYellow);
								else if (chTextureType == CHAR_TEX_BLACK)		
								WRITE_SHORT(iDefaultHitBlack);
								else if (chTextureType == CHAR_TEX_DIRT)		
								WRITE_SHORT(iDefaultHitGreen);
								else if (chTextureType == CHAR_TEX_WOOD)
								WRITE_SHORT(iDefaultHitWood1);
								else if (chTextureType == CHAR_TEX_SLIME)
								WRITE_SHORT(iDefaultHitSlime);
								else
								{	
									if ( RANDOM_LONG(0,99) < 40 )// 39.6 % of chance
									WRITE_SHORT(iDefaultWallSmokeLong);
									else
									WRITE_SHORT(iDefaultWallSmoke);
								}
							MESSAGE_END();
						}
					}
				}//eo cvar check
			}//eo bsp check
		}//eo tr.fraction
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

			if ( iDamage )
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
			} 
			else switch(iBulletType)
			{
			default:
			case BULLET_MONSTER_9MM:
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmg9MM, vecDir, &tr, DMG_BULLET);
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );

				break;

			case BULLET_MONSTER_MP5:
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmgMP5, vecDir, &tr, DMG_BULLET);
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
//				ALERT( at_console, "Monster is shooting! BULLET_MONSTER_MP5\n" );
				break;
			//	 ALERT( at_console, "Monster is shooting!\n" );
				
//SYS EDIT: agregado nuevamente. la funcion necesita saber sobre el daño

		   case BULLET_PLAYER_M16:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgM16, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_AK74:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgAK74, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_UZI:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgUZI, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_P90:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgP90, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_BER92F:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBer92F, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_GLOCK18:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgGlock18, 
                             vecDir, &tr, DMG_BULLET); 

			   break;
		   case BULLET_PLAYER_USAS:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgUsas, 
                             vecDir, &tr, DMG_BULLET | ((iDamage < 10) ? DMG_ALWAYSGIB : DMG_NEVERGIB) ); // DMG_BULLET 
			   break;

		   case BULLET_PLAYER_DEAGLE:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgDeagle, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_SNIPER:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgSniper, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_M82:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgM82, 
                             vecDir, &tr, DMG_BULLET); 
			   break;
			  case BULLET_PLAYER_SCOUT:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgScout, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_FAMAS:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgFamas, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		   case BULLET_PLAYER_M249:
			    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgM249, 
                             vecDir, &tr, DMG_BULLET); 
			   break;

		  case BULLET_PLAYER_IRGUN:
		  pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgIRGUN, vecDir, &tr, DMG_BULLET);
		  break;

			case BULLET_MONSTER_12MM:		
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmg12MM, vecDir, &tr, DMG_BULLET); 
				if ( !tracer )
				{
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot( &tr, iBulletType );
				}
				break;
			
			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				}

				break;
			}													
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
	}
	ApplyMultiDamage(pev, pevAttacker);
}


/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/
Vector CBaseEntity::FireBulletsPlayer ( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y, z;

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);
					
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
		BOOL b_CanMakeParticles = TRUE;

		if ( pEntity->IsBSPModel() )
		{
			char chTextureType;
			char szbuffer[64];//64
			const char *pTextureName;
			float rgfl1[3];
			float rgfl2[3];
			float fattn = ATTN_NORM;

			chTextureType = 0;

			if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				// hit body
				chTextureType = CHAR_TEX_FLESH;
			else
			{
				vecSrc.CopyToArray(rgfl1);
				vecEnd.CopyToArray(rgfl2);

				if (pEntity)
					pTextureName = TRACE_TEXTURE( ENT(pEntity->pev), rgfl1, rgfl2 );
				else
					pTextureName = TRACE_TEXTURE( ENT(0), rgfl1, rgfl2 );
					
				if ( pTextureName )
				{
					if (*pTextureName == '-' || *pTextureName == '+')
						pTextureName += 2;

					if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
						pTextureName++;
					strcpy(szbuffer, pTextureName);
					szbuffer[CBTEXTURENAMEMAX - 1] = 0;
					chTextureType = TEXTURETYPE_Find(szbuffer);	
				
					//this don't work
					if (*pTextureName == 'null')
					{
						b_CanMakeParticles = FALSE;
					}
				}
			}

			if ( FClassnameIs(pEntity->pev, "func_collision"))
			{
				b_CanMakeParticles = FALSE;
			}

			float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range

			if ( CVAR_GET_FLOAT( "r_paintball" ) == 0 )
			if ( CVAR_GET_FLOAT( "cl_wallpuff" ) >= 1 )
			{			
//				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode == kRenderTransAdd)
				{
					//the decal it's going to put in a solid. We only can see the decal only on
					//glass brushes because the decals cant be placed at a transparent texture
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				
					switch ( RANDOM_LONG(0,2) )
					{
						case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass1.wav", flVolume, ATTN_NORM, 0, 100); break;
						case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass2.wav", flVolume, ATTN_NORM, 0, 100); break;
						case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass3.wav", flVolume, ATTN_NORM, 0, 100); break;
					}

					if (VARS(tr.pHit)->playerclass == 1)
					{
						//EDIT: this is a brekeable glass
						/*
						UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/glass3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
						*/
					}
				}
				else
				{
					if (chTextureType == CHAR_TEX_METAL)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas				

						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_VENT)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
					
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_COMPUTER)		
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_METAL1 + RANDOM_LONG(0,2));
								
						UTIL_Ricochet( tr.vecEndPos, 0.5 );

						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/metal3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						if (RANDOM_LONG( 0, 99 ) < 40)
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 500 );//chispas

						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 9, 5, 5, 100 );//puntos
						UTIL_WhiteSparks( tr.vecEndPos, tr.vecPlaneNormal, 0, 5, 500, 20 );//chispas
	
						UTIL_Sparks( tr.vecEndPos );

						switch ( RANDOM_LONG(0,1) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark5.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "buttons/spark6.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
										
						b_CanMakeParticles = FALSE;
					}
					else if (chTextureType == CHAR_TEX_WOOD)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/wood3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));						
					}
					else if (chTextureType == CHAR_TEX_BLUE)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_RED)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_YELLOW)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}				
					else if (chTextureType == CHAR_TEX_BLACK)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}
					else if (chTextureType == CHAR_TEX_SNOW)
					{
						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));							
					}
					else if (chTextureType == CHAR_TEX_DIRT)
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_WOOD1 + RANDOM_LONG(0,2));
					}
									
					else if (chTextureType == CHAR_TEX_TILE)		
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/tile3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}

					else if (chTextureType == CHAR_TEX_SLIME)		
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/water3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
					}

					else
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}

						UTIL_DecalTrace( &tr, DECAL_HOLE_CONC1 + RANDOM_LONG(0,2));
					}	
					
					if (b_CanMakeParticles)
					{
						if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
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
								if (chTextureType == CHAR_TEX_BLUE)		
								WRITE_SHORT(iDefaultHitBlue);
								else if (chTextureType == CHAR_TEX_RED)		
								WRITE_SHORT(iDefaultHitRed);
								else if (chTextureType == CHAR_TEX_YELLOW)		
								WRITE_SHORT(iDefaultHitYellow);
								else if (chTextureType == CHAR_TEX_BLACK)		
								WRITE_SHORT(iDefaultHitBlack);
								else if (chTextureType == CHAR_TEX_DIRT)		
								WRITE_SHORT(iDefaultHitGreen);
								else if (chTextureType == CHAR_TEX_WOOD)
								WRITE_SHORT(iDefaultHitWood1);
								else if (chTextureType == CHAR_TEX_SLIME)
								WRITE_SHORT(iDefaultHitSlime);
								else
								{	
									if ( RANDOM_LONG(0,99) < 40 )// 39.6 % of chance
									WRITE_SHORT(iDefaultWallSmokeLong);
									else
									WRITE_SHORT(iDefaultWallSmoke);
								}
							MESSAGE_END();
						}
					}
				}//eo cvar check
			}//eo bsp check
		}//eo tr.fraction
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

			if ( iDamage )
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
			} 
			else switch(iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET); 
							//sys edit: need i to add this?
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);

				break;

			case BULLET_PLAYER_MP5:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET); 
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);

				break;

			case BULLET_PLAYER_BUCKSHOT:	
				 // make distance based!
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET | DMG_SHOTGUN); 
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
			
				break;
			
			case BULLET_PLAYER_357:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET); 
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
		
				break;
				
			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
		
		extern short g_sModelIndexLaser;
		
		if ( CVAR_GET_FLOAT("slowmo" ) != 0 )			
		{
			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_BEAMPOINTS );
				WRITE_COORD( vecSrc.x );
				WRITE_COORD( vecSrc.y );//asdasd
				WRITE_COORD( vecSrc.z );

				WRITE_COORD( tr.vecEndPos.x );
				WRITE_COORD( tr.vecEndPos.y );
				WRITE_COORD( tr.vecEndPos.z );
				WRITE_SHORT( g_sModelIndexLaser );
				WRITE_BYTE( 0 ); // framerate
				WRITE_BYTE( 0 ); // framerate

				WRITE_BYTE( 5 ); // life
				WRITE_BYTE( 1 );  // width

				WRITE_BYTE( 1 );   // noise
				WRITE_BYTE( 255 );   // r, g, b
				WRITE_BYTE( 255 );   // r, g, b
				WRITE_BYTE( 255 );   // r, g, b
				WRITE_BYTE( 100 );	// brightness
				WRITE_BYTE( 111 );		// speed
			MESSAGE_END();
		}
	}
	ApplyMultiDamage(pev, pevAttacker);

	
	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}

void CBaseEntity :: TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	if (BloodColor() == DONT_BLEED)
		return;
	
	if (flDamage == 0)
		return;

	if (! (bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR)))
		return;
	
	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir; 
	float flNoise;
	int cCount;
	int i;

/*
	if ( !IsAlive() )
	{
		// dealing with a dead monster. 
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return; 
		}
		else
		{
			pev->max_health--;
		}
	}
*/

	if (flDamage < 10)
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if (flDamage < 25)
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	for ( i = 0 ; i < cCount ; i++ )
	{
		vecTraceDir = vecDir * -1;// trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT(pev), &Bloodtr);

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}

//=========================================================
//=========================================================
void CBaseMonster :: MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir )
{
	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir; 
	int i;

	if ( !IsAlive() )
	{
		// dealing with a dead monster. 
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return; 
		}
		else
		{
			pev->max_health--;
		}
	}

	for ( i = 0 ; i < cCount ; i++ )
	{
		vecTraceDir = vecDir;

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, ignore_monsters, ENT(pev), &Bloodtr);

/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SHOWLINE);
			WRITE_COORD( ptr->vecEndPos.x );
			WRITE_COORD( ptr->vecEndPos.y );
			WRITE_COORD( ptr->vecEndPos.z );
			
			WRITE_COORD( Bloodtr.vecEndPos.x );
			WRITE_COORD( Bloodtr.vecEndPos.y );
			WRITE_COORD( Bloodtr.vecEndPos.z );
		MESSAGE_END();
*/

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}
void CGib :: HgruntArmGib( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( "models/gib_hgrunt_a.mdl" );// throw one head
	pGib->pev->body = 0;
	
	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
		
		edict_t		*pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );
		
		if ( RANDOM_LONG ( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t	*pevPlayer;

			pevPlayer = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
		}


		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
	
		if ( pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}
void CGib :: PitDroneGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/pit_drone_gibs.mdl" );
		pGib->pev->body = RANDOM_LONG(0,ALIEN_GIB_COUNT-1);
		
		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib :: ShockTrooperGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/strooper_gibs.mdl" );
		pGib->pev->body = RANDOM_LONG(0,ALIEN_GIB_COUNT-1);
		
		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib :: RobotGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/computergibs.mdl" );
		pGib->pev->body = RANDOM_LONG(0,ALIEN_GIB_COUNT-1);
		
		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib :: HoundeyeGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/houndeye_gibs.mdl" );
		pGib->pev->body = RANDOM_LONG(0,ALIEN_GIB_COUNT-1);
		
		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}
void CGib :: IslaveGibs( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( "models/islave_gibs.mdl" );

	pGib->pev->body = 0; //head ??
//	pGib->pev->body = 1; //pata ??
//	pGib->pev->body = 2; //mano??

	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
		
		edict_t		*pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );
		
		if ( RANDOM_LONG ( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t	*pevPlayer;

			pevPlayer = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
		}


		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
	
		if ( pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}
void CGib :: IslaveArmGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/islave_gibs.mdl" );
//		pGib->pev->body = 0; //head ??
//		pGib->pev->body = 1; //pata ??
		pGib->pev->body = 2; //mano??

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib :: IslaveLegGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/islave_gibs.mdl" );
//		pGib->pev->body = 0; //head ??
		pGib->pev->body = 1; //pata ??
//		pGib->pev->body = 2; //mano??

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
		//	pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}
void CGib :: RobotHeadGib( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( "models/robot_head.mdl" );

	pGib->pev->body = 0;

	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
		
		edict_t		*pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );
		
		if ( RANDOM_LONG ( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t	*pevPlayer;

			pevPlayer = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
		}


		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
	
		if ( pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}

void CGib :: RobotArmGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/robot_arm.mdl" );

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib :: RobotLegGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/robot_leg.mdl" );

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
		//	pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib :: ZoBodyGibs( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( "models/zo_body.mdl" );

	pGib->pev->body = 0; //head ??

	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
		
		edict_t		*pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );
		
		if ( RANDOM_LONG ( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t	*pevPlayer;

			pevPlayer = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
		}


		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
	
		if ( pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}
void CGib :: ZoLegsGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/zo_legs.mdl" );
		pGib->pev->body = 0; //head ??

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
		//	pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib :: HgruntHeadGibs( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( "models/hgibs.mdl" );

	switch (RANDOM_LONG(0,2)) 
	{
		case 0:	pGib->pev->body = 10; break;//chunl
		case 1:	pGib->pev->body = 8; break;//eye
		case 2:	pGib->pev->body = 10; break;
	}

	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
			
		switch (RANDOM_LONG(0,2)) 
		{
			case 0:	pGib->pev->origin.x += RANDOM_LONG ( -10, 10 ); break;
			case 1:	pGib->pev->origin.y += RANDOM_LONG ( -10, 10 ); break;
			case 2:	pGib->pev->origin.z += RANDOM_LONG ( -10, 10 ); break;
		}

		edict_t		*pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );
		
		if ( RANDOM_LONG ( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t	*pevPlayer;

			pevPlayer = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
		}


		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
	
		if ( pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}



/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Monsters.
================
*/
void CBaseEntity::FireIRGUNBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker )
{
	static int tracerCount;
	int tracer;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		// get circular gaussian spread
		float x, y, z;
		do {
			x = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			y = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			z = x*x+y*y;
		} while (z > 1);

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);

		tracer = 0;
		if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
		{
			Vector vecTracerSrc;

			if ( IsPlayer() )
			{// adjust tracer position for player
				vecTracerSrc = vecSrc + Vector ( 0 , 0 , -4 ) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			}
			else
			{
				vecTracerSrc = vecSrc;
			}
			
			if ( iTracerFreq != 1 )		// guns that always trace also always decal
				tracer = 1;
			switch( iBulletType )
			{
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:
			default:
				MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecTracerSrc );
					WRITE_BYTE( TE_TRACER );
					WRITE_COORD( vecTracerSrc.x );
					WRITE_COORD( vecTracerSrc.y );
					WRITE_COORD( vecTracerSrc.z );
					WRITE_COORD( tr.vecEndPos.x );
					WRITE_COORD( tr.vecEndPos.y );
					WRITE_COORD( tr.vecEndPos.z );
				MESSAGE_END();
				break;
			}
		}
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

			Vector vecForward = gpGlobals->v_forward;	
			Vector vecBlood = (tr.vecEndPos - vecSrc).Normalize( );

			Vector vecOrigen;

			vecOrigen = tr.vecEndPos + vecForward * -2;
			CSprite *pSprite0 = CSprite::SpriteCreate( "sprites/iRifle_hit.spr", vecOrigen, FALSE );	
			pSprite0->SetTransparency( kRenderTransAdd, 200, 200, 200, 200, kRenderFxNone );
			pSprite0->AnimateAndDie( 10 );
			pSprite0->SetScale( 0.05 );
			pSprite0->Expand( 0.1  , 150  );
		//	pSprite0->InpandScaled( 0.3 );

			//pSprite0->pev->velocity = Vector ( RANDOM_FLOAT( 2, 8 ), RANDOM_FLOAT( 2, 8 ), RANDOM_FLOAT( 2, 8 ) );
			
			if ( iDamage )
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
			//	TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
			} 
			else switch(iBulletType)
			{
			default:
			case BULLET_MONSTER_9MM:
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmg9MM, vecDir, &tr, DMG_BULLET);
				
			//	TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );

				break;

			case BULLET_MONSTER_MP5:
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmgMP5, vecDir, &tr, DMG_BULLET);
				
			//	TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
				break;

			case BULLET_PLAYER_IRGUN:
			  pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgIRGUN, vecDir, &tr, DMG_BULLET);
			  break;

			case BULLET_MONSTER_12MM:		
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmg12MM, vecDir, &tr, DMG_BULLET); 
				if ( !tracer )
				{
				//	TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot( &tr, iBulletType );
				}
				break;
			
			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				//TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				}

				break;
			}													
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
	}
	ApplyMultiDamage(pev, pevAttacker);
		
	if (RANDOM_LONG(0,1))
	{
		switch (RANDOM_LONG(0,4))
		{
			case 0:UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/irgun/nearmiss.wav", 1.5, ATTN_NORM, 0, 96 + RANDOM_LONG(0,0xf));break;
			case 1:UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/irgun/nearmiss.wav", 1.5, ATTN_NORM, 0, 96 + RANDOM_LONG(0,0xf));break;
			case 2:UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/irgun/nearmiss.wav", 1.5, ATTN_NORM, 0, 96 + RANDOM_LONG(0,0xf));break;
			case 3:UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/irgun/nearmiss.wav", 1.5, ATTN_NORM, 0, 96 + RANDOM_LONG(0,0xf));break;
			case 4:UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, "weapons/irgun/nearmiss.wav", 1.5, ATTN_NORM, 0, 96 + RANDOM_LONG(0,0xf));break;
		}
	}
}