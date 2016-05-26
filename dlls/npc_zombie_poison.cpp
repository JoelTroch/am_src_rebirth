/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Zombie Poison
// Code by...
//=========================================================

// UNDONE: Don't flinch every time you get hit

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
//#include	"skill.h"


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	ZOMBIE_AE_MELEE		801
#define ZOMBIE_AE_THROW		802
#define ZOMBIE_AE_HC_ON_HAND	803
#define ZOMBIE_AE_RELEASE_HC	804
//default, zombie with 4 hc on back
/*bodys:
0 - zombie with 4 hc on back
1 - zombie with 3 hc on back
2 - zombie with 2 hc on back
3 - zombie with 1 hc on back
4 - zombie without hc on back
5 - 
*/

#define CRAB_ON_HAND_GROUP	1
#define CRAB_OFF			0
#define CRAB_ON				1

#define	PZ_BLACK_HEADCRAB          "npc_headcrab_poison"
#define ZOMBIE_FLINCH_DELAY			2		// at most one flinch every n secs

class CZombiePoison : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int IgnoreConditions ( void );

	float m_flNextFlinch;
	
	int iCrabCounter;
	void ThrowCrab ( void );
	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];

	BOOL CheckRangeAttack1 ( float flDot, float flDist );
//	BOOL CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
//private:
	float m_flNextThrowTime;
};

LINK_ENTITY_TO_CLASS( npc_zombie_poison, CZombiePoison );

TYPEDESCRIPTION	CZombiePoison::m_SaveData[] = 
{
	DEFINE_FIELD( CZombiePoison, m_flNextThrowTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CZombiePoison, CBaseMonster );


const char *CZombiePoison::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CZombiePoison::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CZombiePoison::pAttackSounds[] = 
{
	"zombie_poison/pz_warn1.wav",
	"zombie_poison/pz_warn2.wav",
};

const char *CZombiePoison::pIdleSounds[] = 
{
	"zombie_poison/pz_idle2.wav",
	"zombie_poison/pz_idle3.wav",
	"zombie_poison/pz_idle4.wav",
};

const char *CZombiePoison::pAlertSounds[] = 
{
	"zombie_poison/pz_alert1.wav",
	"zombie_poison/pz_alert2.wav",
};

const char *CZombiePoison::pPainSounds[] = 
{
	"zombie_poison/pz_pain1.wav",
	"zombie_poison/pz_pain2.wav",
	"zombie_poison/pz_pain3.wav",
};

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CZombiePoison :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CZombiePoison :: SetYawSpeed ( void )
{
	int ys;

	ys = 320;

#if 0
	switch ( m_Activity )
	{
	}
#endif

	pev->yaw_speed = ys;
}

int CZombiePoison :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// Take 30% damage from bullets
	if ( bitsDamageType == DMG_BULLET )
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce( flDamage );
		pev->velocity = pev->velocity + vecDir * flForce;
		flDamage *= 0.3;
	}

	// HACK HACK -- until we fix this.
	if ( IsAlive() )
		PainSound();
	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CZombiePoison :: PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);
		
	if (RANDOM_LONG(0,5) < 2)
		EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CZombiePoison :: AlertSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CZombiePoison :: IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	// Play a random idle sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CZombiePoison :: AttackSound( void )
{
	// Play a random attack sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombiePoison :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{

		case ZOMBIE_AE_MELEE:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.zombiepoisonMelee, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = 60;
					pHurt->pev->punchangle.x = 15;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 300;
								
					if (RANDOM_LONG(0,1))
					SERVER_COMMAND("drop\n");
				}
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}
			else
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

			if (RANDOM_LONG(0,1))
				AttackSound();
		}
		break;

		case ZOMBIE_AE_HC_ON_HAND:
		{
			if( pev->frags == 4 )
				pev->body = 1;
			else if( pev->frags == 3 )
				pev->body = 2;
			else if( pev->frags == 2 )
				pev->body = 3;
			else if( pev->frags == 1 )
				pev->body = 4;
			else
				ALERT( at_console, "ERROR, no crabs available\n" );

			SetBodygroup( CRAB_ON_HAND_GROUP, CRAB_ON );
		}
		break;

		case ZOMBIE_AE_RELEASE_HC:
		{
			SetBodygroup( CRAB_ON_HAND_GROUP, CRAB_OFF );
		}
		break;

		case ZOMBIE_AE_THROW:
		{
			ThrowCrab();
		}

		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CZombiePoison :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hl2/zombie_poison.mdl");
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	if (pev->health == 0)
		pev->health			= gSkillData.zombiepoisonHealth;
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;
	pev->body			= 0;
	pev->frags			= 4;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombiePoison :: Precache()
{
	int i;

	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hl2/zombie_poison.mdl");

	for ( i = 0; i < ARRAYSIZE( pAttackHitSounds ); i++ )
		PRECACHE_SOUND((char *)pAttackHitSounds[i]);

	for ( i = 0; i < ARRAYSIZE( pAttackMissSounds ); i++ )
		PRECACHE_SOUND((char *)pAttackMissSounds[i]);

	for ( i = 0; i < ARRAYSIZE( pAttackSounds ); i++ )
		PRECACHE_SOUND((char *)pAttackSounds[i]);

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND((char *)pIdleSounds[i]);

	for ( i = 0; i < ARRAYSIZE( pAlertSounds ); i++ )
		PRECACHE_SOUND((char *)pAlertSounds[i]);

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND((char *)pPainSounds[i]);

	//footsteps sound precahed in client.cpp around line 742
	PRECACHE_SOUND("zombie_poison/pz_throw2.wav");
	PRECACHE_SOUND("zombie_poison/pz_throw3.wav");

	UTIL_PrecacheOther( "npc_headcrab_poison" );
}
//=========================================================
// AI Schedules Specific to this monster
//=========================================================



int CZombiePoison::IgnoreConditions ( void )
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
		else
#endif			
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;
	
}

//===================================================
//Check Range Attack 1
//===================================================
BOOL CZombiePoison::CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( IsMoving() && flDist >= 512 )
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return FALSE;
	}
	//has still crab
	if ( flDist > 75 && flDist <= 600 && flDot >= 0.5 && gpGlobals->time >= m_flNextThrowTime && pev->frags > 0 ) //&& (pev->body == 0 || 1 || 2 || 3 || 4) )
	{

		if ( IsMoving() )
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextThrowTime = gpGlobals->time + 6;
		}
		else
		{
			// not moving, so spit again pretty soon.
			m_flNextThrowTime = gpGlobals->time + 0.5;
		}

		return TRUE;
	}

	return FALSE;
}

//===================================================
//Throw Headcrab
//===================================================
void CZombiePoison::ThrowCrab ( void )
{
	switch (RANDOM_LONG(0,1))
	{
	case 0:
		EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "zombie_poison/pz_throw2.wav", 1, ATTN_NORM, 0, 100 );
		break;
	case 1:
		EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "zombie_poison/pz_throw3.wav", 1, ATTN_NORM, 0, 100 );
		break;
	}

	Vector vecHandPos;
	Vector vecThrowAngles;
	GetAttachment( 0, vecHandPos, vecThrowAngles );

	float fla;
	fla = fabs( pev->origin.z - m_hEnemy->pev->origin.z ) * 0.1;

    CBaseEntity *pCrab = CBaseEntity::Create( PZ_BLACK_HEADCRAB, vecHandPos, pev->angles, edict() ); // create the crab

	 pCrab->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND;	// headcrab fall to ground
	 pCrab->pev->spawnflags |= SF_MONSTER_FADECORPSE;		// if headcrab die, his corpse is fade

	pCrab->pev->velocity = pCrab->pev->velocity + gpGlobals->v_forward * 600;
	pCrab->pev->velocity = pCrab->pev->velocity + gpGlobals->v_up * 24 * fla;

	pev->frags -= 1;
}