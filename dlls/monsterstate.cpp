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
  
*Idle sounds of Zombies
+new effects on Runai() -that handles fire and so on-
+blood effects
+smoke detect code

***/

//=========================================================
// monsterstate.cpp - base class monster functions for 
// controlling core AI.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "monsters.h"
#include "animation.h"
#include "saverestore.h"
#include "soundent.h"

#include "weapons.h"//sprites
#include "decals.h" //para la sangre

extern cvar_t phys_normalize_angles;
extern cvar_t phys_globaltrace;
extern cvar_t phys_enable;
extern cvar_t phys_objects;

#include "particle_defs.h"//to use particle def

extern int gmsgParticles;//define external message

//=========================================================
// SetState
//=========================================================
void CBaseMonster :: SetState ( MONSTERSTATE State )
{
/*
	if ( State != m_MonsterState )
	{
		ALERT ( at_aiconsole, "State Changed to %d\n", State );
	}
*/
	
	switch( State )
	{
	
	// Drop enemy pointers when going to idle
	case MONSTERSTATE_IDLE:

		if ( m_hEnemy != NULL )
		{
			m_hEnemy = NULL;// not allowed to have an enemy anymore.
			ALERT ( at_aiconsole, "Stripped\n" );
		}
		break;
	}

	m_MonsterState = State;
	m_IdealMonsterState = State;
}
#define SF_ONSIGHT_STATECHECK	0x00004 //AJH
//SysOp- New
BOOL CBaseMonster :: VisionCheck( void )
{											
	CBaseEntity *pLooker;

	pLooker = UTIL_FindEntityByClassname(NULL, "player");

		if (!pLooker)
		{
			return FALSE;
		}

		CBaseEntity *pSeen;

		if (pev->message)
			pSeen = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
		else
			return CanSee(pLooker, this);

		if (!pSeen)
		{
		// must be a classname.
			pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
			while (pSeen != NULL){
				if (CanSee(pLooker, pSeen))
					return TRUE;
				pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
			}
			return FALSE;
		}
		else
		{
			while (pSeen != NULL){
				if (CanSee(pLooker, pSeen))
					return TRUE;
				pSeen = UTIL_FindEntityByTargetname(pSeen, STRING(pev->message));
			}
			return FALSE;
		}
}

BOOL CBaseMonster :: CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen)
{
	if (pev->frags && (pLooker->pev->origin - pSeen->pev->origin).Length() > pev->frags)
		return FALSE;

//	if (pev->max_health < 360)
//	{
		Vector2D	vec2LOS;
		float	flDot;
		float flComp = pev->health;
		UTIL_MakeVectors ( pLooker->pev->angles );
		vec2LOS = ( pSeen->pev->origin - pLooker->pev->origin ).Make2D();
		vec2LOS = vec2LOS.Normalize();
		flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

		if ( pev->max_health == -1 )
		{
			CBaseMonster *pMonst = pLooker->MyMonsterPointer();
			if (pMonst)
				flComp = pMonst->m_flFieldOfView;
			else
				return FALSE; // not a monster, can't use M-M-M-MonsterVision
		}

		if (flDot <= flComp)
			return FALSE;
//	}

	//si no tiene NO LINE OF SIGHT, entonces tiene

	/*
	TraceResult tr;
	UTIL_TraceLine( pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, dont_ignore_glass, pLooker->edict(), &tr );
	
	if (tr.flFraction < 1.0 && tr.pHit != pSeen->edict())
		return FALSE;
	*/

	return TRUE;
}
//************

/*
void CBaseMonster::SmokeTrail( void ) //the function itself
{
	pev->movetype = MOVETYPE_FLY; //sets the movetype of the trail
	//pev->effects |= EF_LIGHT; //plays a light effect

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY ); //here we start a message which draws a beam behind a player

	WRITE_BYTE( TE_BEAMFOLLOW ); //sets the beam to follow the entity(player)
	WRITE_SHORT( entindex() ); // entity
	WRITE_SHORT( m_iSmokeTrail ); //model/sprite
	WRITE_BYTE( 10 ); //life
	WRITE_BYTE( 10 ); //width
	WRITE_BYTE( 224 ); //red
	WRITE_BYTE( 224 ); //green
	WRITE_BYTE( 255 ); //blue
	WRITE_BYTE( 111 ); //brightness

	MESSAGE_END(); //ends the message we started above

	m_flPlayerSmokeTime = gpGlobals->time; //sets the time to always
	pev->nextthink = gpGlobals->time + 0.1; //the next time we can call a think function is in 0.1 sec
}
*/

//=========================================================
// RunAI
//=========================================================
void CBaseMonster :: RunAI ( void )
{		
	Vector		vecBlastSrc;
	vecBlastSrc = Center();
/*
	if (VisionCheck())
	{
		pev->effects &= ~EF_NODRAW;
		ALERT ( at_console, "Im seeing the monster, pev->effects &= ~EF_NODRAW\n" );
	}
	else
	{
		pev->effects |= EF_NODRAW;
		ALERT ( at_console, "Cannot see the monster, pev->effects |= EF_NODRAW\n" );
	}
*/
	if (pev->waterlevel == 2)
	{
		if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
		{
			MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
				WRITE_SHORT(0);
				WRITE_BYTE(0);
				WRITE_COORD( vecBlastSrc.x );
				WRITE_COORD( vecBlastSrc.y );
				WRITE_COORD( vecBlastSrc.z );
				WRITE_COORD( 0 );
				WRITE_COORD( 0 );
				WRITE_COORD( 0 );
				WRITE_SHORT(iDefaultWaves);
			MESSAGE_END();
		}
	}

	if(b_SlowedMonster)
		pev->framerate = 0.1;
	
	if (FreezeNow)
	{
		//estem... este codigo paraliza la entidad, a veces hay errores de no free edicts, pero bue, todo se rula
	
		//this code freezes the monster.
		pev->deadflag = DEAD_DYING;

		StopAnimation();
		pev->velocity = g_vecZero;
		pev->movetype = MOVETYPE_NONE;
		pev->avelocity = g_vecZero;
		pev->animtime = gpGlobals->time;
		pev->effects |= EF_NOINTERP;
		// Don't think
		pev->nextthink = 0;

		pev->rendermode = kRenderNormal;
		pev->renderamt = 50;
		pev->renderfx = 19;

		pev->rendercolor.x = 50;
		pev->rendercolor.y = 50;
		pev->rendercolor.z = 100;
	}

	if (ExpandNow)
	{
		pev->renderfx = 255;//dont draw shadows
		pev->renderfx = kRenderFxFatness;
		
		pev->renderamt += 5;

		if ( pev->renderamt == 100 )
		GibMonster();
	}

	if (SrhinkNow)
	{
		pev->renderfx = 255;//dont draw shadows

		pev->renderfx = kRenderFxThinness;
		
		pev->renderamt += 5;

		if ( pev->renderamt == 100 )
		GibMonster();
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//mover entidades debajo nuestro
	//BUGBUG!!: causa problemas con botones (se activan si la ent esta cerca)
	/*
	CBaseEntity *pEntidad = NULL;
	
	Vector VecSrc;
	VecSrc = pev->origin;

    while ((pEntidad = UTIL_FindEntityInSphere( pEntidad, VecSrc, 48 )) != NULL)//512
	{	
		if ( !pEntidad->IsAlive() )
		{
			//BUGBUG : el checkeo este sirve. La entidad actual no es afectada (esta viva)
		//	if (!pEntidad->pev)//mmmmm
		//	{
				pEntidad->TakeDamage( pev, pev, 1, DMG_BULLET | DMG_NEVERGIB );//20 y 20?

				//move it too ;)
				//if (pEntidad->pev->movetype == MOVETYPE_STEP)
				//pEntidad->pev->velocity.x = RANDOM_FLOAT( -100, 100 );
		//	}
		}
	}
	*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Smoke detect code
	//Purpose: Changes the entity's classify when are closer to smoke, so that the monster can't attack

		CBaseEntity *pEntidad = NULL;
	
		Vector VecSrc;
		VecSrc = pev->origin;

		//enviar "cercadehumo" 0... si detecta una SG el codigo que vale es el ultimo...
		f_bNearSmoke = FALSE;

		while ((pEntidad = UTIL_FindEntityInSphere( pEntidad, VecSrc, 256 )) != NULL)//512
		{	
			if ( FClassnameIs(pEntidad->pev, "smoke_grenade") )
			{		
				f_bNearSmoke = TRUE;
			}
		}

	Vector vecSrc = Center();

	if (StartBurning)
	{
		if ( f_NextBurnTime < gpGlobals->time )
		{
			if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
			{
				if ( FClassnameIs( pev, "prop_fire" ) )
				{
					MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
						WRITE_SHORT(0);
						WRITE_BYTE(0);
						WRITE_COORD( vecSrc.x );
						WRITE_COORD( vecSrc.y );
						WRITE_COORD( vecSrc.z );
						WRITE_COORD( 0 );
						WRITE_COORD( 0 );
						WRITE_COORD( 0 );
						WRITE_SHORT(iDefaultFire);//perhaps change this later
					MESSAGE_END();
				}
				else
				{
					MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
						WRITE_SHORT(0);
						WRITE_BYTE(0);
						WRITE_COORD( vecSrc.x );
						WRITE_COORD( vecSrc.y );
						WRITE_COORD( vecSrc.z );
						WRITE_COORD( 0 );
						WRITE_COORD( 0 );
						WRITE_COORD( 0 );
						WRITE_SHORT(iDefaultFire);
					MESSAGE_END();
				}
			}	
			
			//VecSrc, to make light depending of the flamme's position
			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE(TE_DLIGHT);
				WRITE_COORD(vecSrc.x);	// X
				WRITE_COORD(vecSrc.y);	// Y
				WRITE_COORD(vecSrc.z);	// Z
				WRITE_BYTE( RANDOM_FLOAT(10, 20) );		// radius * 0.1
				WRITE_BYTE( 255 );		// r
				WRITE_BYTE( 180 );		// g
				WRITE_BYTE( 100 );		// b
				WRITE_BYTE( 10 );		// time * 10
				WRITE_BYTE( 5 );		// decay * 0.1
			MESSAGE_END( );

			::RadiusDamage( pev->origin, pev, pev, 5, 64, CLASS_NONE, DMG_BURN );

			f_NextBurnTime = gpGlobals->time + 0.1;//0.1
		}

		if ( f_NextSoundBurnTime < gpGlobals->time )
		{
			if ( IsAlive() )
			{		
/*
				pev->rendermode = kRenderNormal;

				pev->renderfx = 19;
				pev->renderamt = 10;
				
				pev->rendercolor.x = 255;
				pev->rendercolor.y = 150;
				pev->rendercolor.z = 0;
*/
				TakeDamage( pev, pev, 10, DMG_GENERIC );

				EMIT_SOUND(ENT(pev), CHAN_ITEM, "ambience/burning1.wav", 0.5, ATTN_NORM );
			}
			else//este chequeo no se hace... moverlo en donde se inicia el fuego final (combat.cpp, becomedead() )
			{
				/*
				//Make black
				pev->renderfx = 21;
				pev->renderamt = 1;//+=1?

				//	just in case			
				pev->rendercolor.x = 0;
				pev->rendercolor.y = 0;
				pev->rendercolor.z = 0;

				//clear sound?
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/null.wav", 0.5, ATTN_NORM );
				STOP_SOUND( ENT(pev), CHAN_ITEM, "ambience/burning1.wav" );
				*/
			}
	
			f_NextSoundBurnTime = gpGlobals->time + 2;
		}
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////










////////////////////////////////////////////////////test
	if ( IsAlive() && pev->health <= pev->max_health * 0.25)//[DNS] fix bug
	{
		if ( m_bloodColor == BLOOD_COLOR_RED )
		{
			if ( f_NextBloodTime < gpGlobals->time )
			{
				Vector		vecSpot;
				TraceResult	tr;

				vecSpot = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
				UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -24 ),  ignore_monsters, ENT(pev), & tr);
			
				UTIL_DecalTrace( &tr, DECAL_BLOODDRIP1 +RANDOM_LONG(0,5) );
					
				f_NextBloodTime = gpGlobals->time + RANDOM_FLOAT( 0.2, 0.8 );
			}
		}
	}
////////////////////////////////////////////////////test

	if ( FClassnameIs( pev, "monster_zombie") && RANDOM_LONG(0,99) == 0 )
	{
		//si es zombie, idle sound () SIEMPRE
		if ( IsAlive() )//[DNS] fix bug
		{
			ALERT ( at_aiconsole, "IdleSound\n" );
			IdleSound();
		}
	}
	else //sino, es cualquier monster...
	{
		if ( ( m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT ) && RANDOM_LONG(0,99) == 0 && !(pev->flags & SF_MONSTER_GAG) )
		{
			IdleSound();//ejecutar codigo normal
		}
	}
	// to test model's eye height
	//UTIL_ParticleEffect ( pev->origin + pev->view_ofs, g_vecZero, 255, 10 );

	// IDLE sound permitted in ALERT state is because monsters were silent in ALERT state. Only play IDLE sound in IDLE state
	// once we have sounds for that state.
/*
	if ( ( m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT ) && RANDOM_LONG(0,99) == 0 && !(pev->flags & SF_MONSTER_GAG) )
	{
		IdleSound();
	}
*/
	if ( m_MonsterState != MONSTERSTATE_NONE	&& 
		 m_MonsterState != MONSTERSTATE_PRONE   && 
		 m_MonsterState != MONSTERSTATE_DEAD )// don't bother with this crap if monster is prone. 
	{	
		// collect some sensory Condition information.
		// don't let monsters outside of the player's PVS act up, or most of the interesting
		// things will happen before the player gets there!
		// UPDATE: We now let COMBAT state monsters think and act fully outside of player PVS. This allows the player to leave 
		// an area where monsters are fighting, and the fight will continue.
		if ( !FNullEnt( FIND_CLIENT_IN_PVS( edict() ) ) || ( m_MonsterState == MONSTERSTATE_COMBAT ) )
		{

			Look( m_flDistLook );
			Listen();// check for audible sounds. 

			// now filter conditions.
			ClearConditions( IgnoreConditions() );

			GetEnemy();
		}

		// do these calculations if monster has an enemy.
		if ( m_hEnemy != NULL )
		{
			CheckEnemy( m_hEnemy );
		}

		CheckAmmo();
	}

	FCheckAITrigger();

	PrescheduleThink();

	MaintainSchedule();

	// if the monster didn't use these conditions during the above call to MaintainSchedule() or CheckAITrigger()
	// we throw them out cause we don't want them sitting around through the lifespan of a schedule
	// that doesn't use them. 
	m_afConditions &= ~( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE );
}

//=========================================================
// GetIdealState - surveys the Conditions information available
// and finds the best new state for a monster.
//=========================================================
MONSTERSTATE CBaseMonster :: GetIdealState ( void )
{
	int	iConditions;

	iConditions = IScheduleFlags();
	
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
		
		/*
		IDLE goes to ALERT upon hearing a sound
		-IDLE goes to ALERT upon being injured
		IDLE goes to ALERT upon seeing food
		-IDLE goes to COMBAT upon sighting an enemy
		IDLE goes to HUNT upon smelling food
		*/
		{
			if ( iConditions & bits_COND_NEW_ENEMY )			
			{
				// new enemy! This means an idle monster has seen someone it dislikes, or 
				// that a monster in combat has found a more suitable target to attack
				m_IdealMonsterState = MONSTERSTATE_COMBAT;
			}
			else if ( iConditions & bits_COND_LIGHT_DAMAGE )
			{
				MakeIdealYaw ( m_vecEnemyLKP );
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			else if ( iConditions & bits_COND_HEAVY_DAMAGE )
			{
				MakeIdealYaw ( m_vecEnemyLKP );
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			else if ( iConditions & bits_COND_HEAR_SOUND )
			{
				CSound *pSound;
				
				pSound = PBestSound();
				ASSERT( pSound != NULL );
				if ( pSound )
				{
					MakeIdealYaw ( pSound->m_vecOrigin );
					if ( pSound->m_iType & (bits_SOUND_COMBAT|bits_SOUND_DANGER) )
						m_IdealMonsterState = MONSTERSTATE_ALERT;
				}
			}
			else if ( iConditions & (bits_COND_SMELL | bits_COND_SMELL_FOOD) )
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}

			break;
		}
	case MONSTERSTATE_ALERT:
		/*
		ALERT goes to IDLE upon becoming bored
		-ALERT goes to COMBAT upon sighting an enemy
		ALERT goes to HUNT upon hearing a noise
		*/
		{
			if ( iConditions & (bits_COND_NEW_ENEMY|bits_COND_SEE_ENEMY) )			
			{
				// see an enemy we MUST attack
				m_IdealMonsterState = MONSTERSTATE_COMBAT;
			}
			else if ( iConditions & bits_COND_HEAR_SOUND )
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				CSound *pSound = PBestSound();
				ASSERT( pSound != NULL );
				if ( pSound )
					MakeIdealYaw ( pSound->m_vecOrigin );
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		/*
		COMBAT goes to HUNT upon losing sight of enemy
		COMBAT goes to ALERT upon death of enemy
		*/
		{
			if ( m_hEnemy == NULL )
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				// pev->effects = EF_BRIGHTFIELD;
				ALERT ( at_aiconsole, "***Combat state with no enemy!\n" );
			}
			break;
		}
	case MONSTERSTATE_HUNT:
		/*
		HUNT goes to ALERT upon seeing food
		HUNT goes to ALERT upon being injured
		HUNT goes to IDLE if goal touched
		HUNT goes to COMBAT upon seeing enemy
		*/
		{
			break;
		}
	case MONSTERSTATE_SCRIPT:
		if ( iConditions & (bits_COND_TASK_FAILED|bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE) )
		{
			ExitScriptedSequence();	// This will set the ideal state
		}
		break;

	case MONSTERSTATE_DEAD:
		m_IdealMonsterState = MONSTERSTATE_DEAD;
		break;
	}

	return m_IdealMonsterState;
}

