/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
// Zombie
//=========================================================

// UNDONE: Don't flinch every time you get hit

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"

#include	"animation.h"

#include "decals.h" //para la sangre
//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	ZOMBIE_AE_ATTACK_RIGHT		0x01
#define	ZOMBIE_AE_ATTACK_LEFT		0x02
#define	ZOMBIE_AE_ATTACK_BOTH		0x03

// headcrab jumps from zombie
#define     ZOMBIE_AE_CRAB1          0x04
#define     ZOMBIE_AE_CRAB2          0x05
#define     ZOMBIE_AE_CRAB3          0x06

#define     ZOMBIE_START_FALL        0x08
#define     ZOMBIE_END_FALL          0x09

#define     ZOMBIE_STARTJUMP         0x11

//#define     ZOMBIE_AE_SHOWGRENADE    0x22
#define     ZOMBIE_AE_SHOWGRENADE    22

		#define HEADCRAB_GROUP					1
#define HEADCRAB_ON				0
#define HEADCRAB_OFF			1

#define ZOMBIE_CRAB          "monster_headcrab" // headcrab jumps from zombie


#define ZOMBIE_FLINCH_DELAY			2		// at most one flinch every n secs

extern void ExplosionCreate( const Vector &center, const Vector &angles, edict_t *pOwner, int magnitude, BOOL doDamage );

class CZombie : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int IgnoreConditions ( void );
	void Killed( entvars_t *pevAttacker, int iGib );

	void SpawnCrab( void ); // headcrab jumps from zombie
	void DeathSound( void );
	int	m_voicePitch;

	float m_flNextFlinch;
// Teh_Freak: TraceAttack checks to see if it was shot in the head
	void TraceAttack( entvars_t *pevAttacker, 
	float flDamage, Vector vecDir, TraceResult *ptr, 
	int bitsDamageType);

	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );

	BOOL m_bHL2Zombie;
	BOOL m_bCanSpawnCrab;
	BOOL Isantlion;
	BOOL IsZombieFast;

	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];

	// No range attacks
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) { return FALSE; }
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	static const char *pDeathSounds[];

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	//for zombine
	BOOL b_Frustrated;
	BOOL b_GrenadePulled;
	
	float m_flNextGrenadeCheck;
};

TYPEDESCRIPTION	CZombie::m_SaveData[] = 
{
	DEFINE_FIELD( CZombie, m_bHL2Zombie, FIELD_BOOLEAN ),
	DEFINE_FIELD( CZombie, b_Frustrated, FIELD_BOOLEAN ),
	DEFINE_FIELD( CZombie, m_flNextGrenadeCheck, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CZombie, CBaseMonster );

LINK_ENTITY_TO_CLASS( monster_zombie, CZombie );

const char *CZombie::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CZombie::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CZombie::pAttackSounds[] = 
{
	"zombie/zo_attack1.wav",
	"zombie/zo_attack2.wav",
};

const char *CZombie::pIdleSounds[] = 
{
	"zombie/zo_idle1.wav",
	"zombie/zo_idle2.wav",
	"zombie/zo_idle3.wav",
	"zombie/zo_idle4.wav",
	"zombie/zo_idle5.wav",
	"zombie/zo_idle6.wav",
	"zombie/zo_idle7.wav",
	"zombie/zo_idle8.wav",
};

const char *CZombie::pAlertSounds[] = 
{
	"zombie/zo_alert10.wav",
	"zombie/zo_alert20.wav",
	"zombie/zo_alert30.wav",
};

const char *CZombie::pPainSounds[] = 
{
	"zombie/zo_pain1.wav",
	"zombie/zo_pain2.wav",
};

const char *CZombie::pDeathSounds[] = 
{
	"zombie/zo_die1.wav",
	"zombie/zo_die2.wav",
	"zombie/zo_die3.wav",
};

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CZombie :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CZombie :: SetYawSpeed ( void )
{
	int ys;

	ys = 120;

#if 0
	switch ( m_Activity )
	{
	}
#endif

	pev->yaw_speed = ys;
}

int CZombie :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
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

//	if ( bitsDamageType & DMG_BLAST )
//	{
	m_bCanSpawnCrab = TRUE;
//	}

	if (flDamage >= 220)//bueno, si el daño es mas que 220, reventar el cuerpo en dos partes
	{
		if (m_bHL2Zombie)//HL2 model compatibility
		{
			if ( IsAlive())//[DNS] fix bug
			{
				CBaseEntity *pTorso;

				pTorso = CBaseEntity::Create( "monster_zombie_torso", pev->origin, g_vecZero, edict() );
				pTorso->pev->velocity = pev->velocity;
				pTorso->pev->velocity.z += 111;
				pTorso->pev->angles = pev->angles;

				pTorso->pev->flags &= ~FL_ONGROUND;//no está en el suelo
				
				//SetBits( pTorso->pev->flags, FL_FLOAT );

				//pTorso->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND;

				CBaseEntity *pLegs;

				pLegs = CBaseEntity::Create( "monster_zombie_legs", pev->origin, g_vecZero, edict() );
				//pLegs->pev->velocity = pev->velocity;
				pLegs->pev->angles = pev->angles;
				
				pLegs->Killed( pev, GIB_NEVER );
				pLegs->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND;

					
				Vector VecSpot;

				VecSpot.y = pev->angles.y;

				CBaseEntity *pCrab;

				pCrab = CBaseEntity::Create( ZOMBIE_CRAB, EyePosition(), VecSpot, edict() ); // create the crab

				pCrab->pev->velocity = pev->velocity;
					
				//pCrab->pev->velocity.z += 100;
				pCrab->pev->angles = pev->angles;

				pCrab->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND; // make the crab fall

				CBaseMonster :: GibMonster();
			}
		}
	}

//	ALERT (at_console, "flDamage is %f", flDamage);

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CZombie :: PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	if (RANDOM_LONG(0,5) < 2)
		EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CZombie :: AlertSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	if(!Isantlion)
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CZombie :: IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	// Play a random idle sound
	if(!Isantlion)
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CZombie :: AttackSound( void )
{
	// Play a random attack sound
	
	if(!Isantlion)
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombie :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			// do stuff for this event.
	//		ALERT( at_console, "Slash right!\n" );
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.zombieDmgOneSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
				}
				// Play a random attack hit sound
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

			if (RANDOM_LONG(0,1))
				AttackSound();
		}
		break;

		case ZOMBIE_AE_ATTACK_LEFT:
		{
			// do stuff for this event.
	//		ALERT( at_console, "Slash left!\n" );
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.zombieDmgOneSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = 18;
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100;
				
					if ( pHurt->pev->flags & FL_CLIENT )
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

		case ZOMBIE_AE_ATTACK_BOTH:
		{
			// do stuff for this event.
			CBaseEntity *pHurt;

			if(Isantlion)
			pHurt = CheckTraceHullAttack( 70, gSkillData.zombieDmgBothSlash *2, DMG_SLASH );
			else
			pHurt = CheckTraceHullAttack( 70, gSkillData.zombieDmgBothSlash, DMG_SLASH );

			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
				
					if ( pHurt->pev->flags & FL_CLIENT )
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

// headcrab jumps from zombie
          case ZOMBIE_AE_CRAB1://event #3
          {
			  	if (m_bCanSpawnCrab)
				{
				   SetBodygroup( HEADCRAB_GROUP, HEADCRAB_OFF );
				   SpawnCrab(); // spawn a headcrab
				}
          }
          break;

		case ZOMBIE_AE_CRAB2://4
		{
			if (m_bCanSpawnCrab)
			{
				if ( FClassnameIs( pev, "monster_zombine" ) )
				{
					if(b_Frustrated)
					{
						if (RANDOM_LONG( 0, 99 ) < 40)
						{
							SetBodygroup( HEADCRAB_GROUP, HEADCRAB_OFF );
							SpawnCrab(); // spawn a headcrab
						}
					}
				}
				else
				{
					SetBodygroup( HEADCRAB_GROUP, HEADCRAB_OFF );
					SpawnCrab(); // spawn a headcrab
				}
			}
		}
		break;

          case ZOMBIE_AE_CRAB3://5
          {
			  	if (m_bCanSpawnCrab)
				{
				   SetBodygroup( HEADCRAB_GROUP, HEADCRAB_OFF );
				   SpawnCrab(); // spawn a headcrab
				}
          }
          break;
		           
		  case ZOMBIE_AE_SHOWGRENADE:
          {
				SetBodygroup( 2, 1 );
          }
          break;

		  case ZOMBIE_START_FALL:
          {

          }
          break;
		  		  
		  case ZOMBIE_END_FALL:
          {
			  	pev->solid = SOLID_SLIDEBOX;
				pev->flags |= FL_ONGROUND;
				DROP_TO_FLOOR( ENT(pev) );
		  }
          break;

		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CZombie :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/zombie.mdl");

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;

	if (pev->health == 0)
	{
		pev->health			= gSkillData.zombieHealth;
	}
	else
	{
		if ( FClassnameIs( pev, "npc_zombine" ) )
		pev->health			= gSkillData.zombieHealth * 2;
		else
		pev->health			= gSkillData.zombieHealth;
	}


	if (IsZombieFast)
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	else
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.

	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	m_voicePitch		= RANDOM_LONG( 85, 110 );

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombie :: Precache()
{
	int i;

	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/zombie.mdl");
	
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

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND((char *)pDeathSounds[i]);

	UTIL_PrecacheOther( ZOMBIE_CRAB ); // headcrab jumps from zombie
}	

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
int CZombie::IgnoreConditions ( void )
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
		//SYS
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
//=========================================================
// TraceAttack - checks to see if the zombie was shot in the head
//=========================================================
void CZombie :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
     // Teh_Freak: check to see if shot in the head
     if (ptr->iHitgroup == HITGROUP_HEAD)
     {
          m_bloodColor = BLOOD_COLOR_YELLOW; // Teh_Freak: is shot in the head, emit yellow blood
     }
     else
     {
          m_bloodColor = BLOOD_COLOR_RED; // Teh_Freak: if not, emit red blood
     }
// CODE RANDOM HEADSHOT SOUNDS START
	
	if (ptr->iHitgroup == HITGROUP_HEAD) // si el daño es en la cabeza
		{
		ALERT( at_console, "Zombie Headshot!\n" );
				UTIL_BloodStream( ptr->vecEndPos,
						gpGlobals->v_forward * -5 + 
						gpGlobals->v_up * 2, 
						(unsigned short)58, 100 ); //56 its ok
		switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar
		{
		case 0: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot1.wav", 1, ATTN_NORM ); break;
		case 1: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot2.wav", 1, ATTN_NORM ); break;
		case 2: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot3.wav", 1, ATTN_NORM ); break;
		}
	}

// CODE RANDOM HEADSHOT SOUNDS END
     CBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// Spawn Headcrab - headcrab jumps from zombie
//=========================================================
void CZombie :: SpawnCrab( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;
	GetAttachment( 0, vecGunPos, vecGunAngles );

	Vector VecSpot;

	VecSpot.y = pev->angles.y;

	CBaseEntity *pCrab;

	if (IsZombieFast)
	pCrab = CBaseEntity::Create( "npc_headcrab_fast", pev->view_ofs, VecSpot, edict() ); // create the crab
	else
	pCrab = CBaseEntity::Create( ZOMBIE_CRAB, EyePosition(), VecSpot, edict() ); // create the crab

	pCrab->pev->velocity = Vector(RANDOM_FLOAT(-50,50),RANDOM_FLOAT(-50,50),RANDOM_FLOAT(-50,50));

//	pCrab->pev->velocity = pev->velocity;

	pCrab->pev->angles = pev->angles;

	pCrab->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND; // make the crab fall
}

void CZombie :: DeathSound( void )
{
	EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pDeathSounds[ RANDOM_LONG(0,ARRAYSIZE(pDeathSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
}

void CZombie::Killed( entvars_t *pevAttacker, int iGib )
{
	if (IsZombieFast)
	{
		SetBodygroup( HEADCRAB_GROUP, HEADCRAB_OFF );

		CBaseEntity *pEntity = NULL;
		pEntity = CBaseEntity::Create( "npc_headcrab_fast", pev->origin, pev->angles );

		pEntity->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND; // make the crab fall
	}

	CBaseMonster::Killed( pevAttacker, iGib );
}

///////////////////////////////////////////////////////////////////////////
class CZombieHuman: public CZombie
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_zombie_human, CZombieHuman );

//=========================================================
// Spawn
//=========================================================
void CZombieHuman :: Spawn()
{
	Precache( );
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/zombie-human.mdl");
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	if (pev->health == 0)
	pev->health			= gSkillData.zombieHealth;
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;
	m_voicePitch		= RANDOM_LONG( 85, 110 );
	MonsterInit();
}
void CZombieHuman :: Precache()
{
	int i;
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/zombie-human.mdl");

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

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND((char *)pDeathSounds[i]);
}	
///////////////////////////////////////////////////////////////////////////
class CZombieBarney: public CZombie
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_zombie_barney, CZombieBarney );

//=========================================================
// Spawn
//=========================================================
void CZombieBarney :: Spawn()
{
	Precache( );
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/zombie_barney.mdl");
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	if (pev->health == 0)
	pev->health			= gSkillData.zombieHealth;//el doble del zombie
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;
	m_voicePitch		= RANDOM_LONG( 85, 110 );
	MonsterInit();
}
void CZombieBarney :: Precache()
{
	int i;
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/zombie_barney.mdl");

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

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND((char *)pDeathSounds[i]);
}	

///////////////////////////////////////////////////////////////////////////


class CZombieSoldier: public CZombie
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_zombie_soldier, CZombieSoldier );

//=========================================================
// Spawn
//=========================================================
void CZombieSoldier :: Spawn()
{
	Precache( );
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/zombie_soldier.mdl");
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	if (pev->health == 0)
	pev->health			= gSkillData.zombieHealth *2;//el doble del zombie
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;
	m_voicePitch		= RANDOM_LONG( 85, 110 );
	MonsterInit();
}
void CZombieSoldier :: Precache()
{
	int i;
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/zombie_soldier.mdl");

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

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND((char *)pDeathSounds[i]);
}



///////////////////////////////////////////////////////////////////////////


class CZombieLEGS: public CZombie
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_zombie_legs, CZombieLEGS );

//=========================================================
// Spawn
//=========================================================
void CZombieLEGS :: Spawn()
{
	Precache( );
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hl2/classic_legs.mdl");

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;//
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	if (pev->health == 0)
	pev->health			= gSkillData.zombieHealth;//el doble del zombie

	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
//	m_afCapability		= bits_CAP_DOORS_GROUP;
	m_voicePitch		= RANDOM_LONG( 85, 110 );

	m_bHL2Zombie		= TRUE;

	MonsterInit();
}
void CZombieLEGS :: Precache()
{
	int i;
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hl2/classic_legs.mdl");

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

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND((char *)pDeathSounds[i]);
}



///////////////////////////////////////////////////////////////////////////
class CZombieTORSO: public CZombie
{
public:
	void Spawn( );
	void Precache(void);
	void RunAI( void );

	void SetActivity ( Activity NewActivity );

	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
};

LINK_ENTITY_TO_CLASS( monster_zombie_torso, CZombieTORSO );

//=========================================================
// SetActivity 
//=========================================================
void CZombieTORSO :: SetActivity ( Activity NewActivity )
{
	int	iSequence = 0;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	Vector	vecGunPos;
	Vector	vecGunAngles;
	CBaseEntity *pEntidad = NULL;

	GetAttachment( 0, vecGunPos, vecGunAngles );

	switch ( NewActivity)
	{
	case ACT_IDLE:
		{
			if (!(pev->flags & FL_ONGROUND))
			{
				iSequence = LookupSequence( "fall" );
			}		
			else
			{
				//Esto es obsoloeto .... ¬¬
				if (RANDOM_LONG( 0, 99 ) < 60) //
				iSequence = LookupSequence( "idle01" );
				else
				iSequence = LookupSequence( "idle02" );
			}
		//	iSequence = LookupActivity ( NewActivity );
		}
		break;

	default:
		iSequence = LookupActivity ( NewActivity );
		break;
	}
	
	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > 0 )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo( );
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
	//	ALERT ( at_console, "%s no tiene una secuencia para el acto:%d, %c, %s \n", STRING(pev->classname), NewActivity, NewActivity, NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}
}

//=========================================================
// Spawn
//=========================================================
void CZombieTORSO :: Spawn()
{
	Precache( );
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hl2/classic_torso.mdl");

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	if (pev->health == 0)
	pev->health			= 20;
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
//	m_afCapability		= bits_CAP_DOORS_GROUP;
	m_voicePitch		= RANDOM_LONG( 85, 110 );

	m_bHL2Zombie		= TRUE;

	MonsterInit();
}
void CZombieTORSO :: Precache()
{
	int i;
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hl2/classic_torso.mdl");

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

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND((char *)pDeathSounds[i]);
}
int CZombieTORSO :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
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
//=========================================================
// RunAI
//=========================================================
void CZombieTORSO :: RunAI( void )
{


			if ( IsAlive())//[DNS] fix bug
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
				
			CBaseMonster :: RunAI();
}

class CZombieHL2: public CZombie
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( npc_zombie, CZombieHL2 );

void CZombieHL2 :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hl2/zombie.mdl");

	m_bHL2Zombie		= TRUE;

	CZombie :: Spawn();
}
void CZombieHL2 :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hl2/zombie.mdl");

	UTIL_PrecacheOther( "monster_zombie_torso" );
	UTIL_PrecacheOther( "monster_zombie_legs");

	CZombie :: Precache();
}	

///////////////////

class CZombieFastHL2: public CZombie
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( npc_zombie_fast, CZombieFastHL2 );

void CZombieFastHL2 :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hl2/zombie_fast.mdl");

	IsZombieFast = TRUE;

	CZombie :: Spawn();
}
void CZombieFastHL2 :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hl2/zombie_fast.mdl");

	UTIL_PrecacheOther( "npc_headcrab_fast" );

	CZombie :: Precache();
}	


//////////////////////////////////////////////////////////
class CAntLion : public CZombie
{
public:
	void Spawn( void );
	void Precache( void );

	void DeathSound( void );

	void TraceAttack( entvars_t *pevAttacker, 
	float flDamage, Vector vecDir, TraceResult *ptr, 
	int bitsDamageType);

	void PainSound( void );

	static const char *pPainSounds[];
	static const char *pDeathSounds[];
};

LINK_ENTITY_TO_CLASS( npc_antlion, CAntLion );

const char *CAntLion::pPainSounds[] = 
{
	"antlion/pain1.wav",
	"antlion/pain2.wav",
};

const char *CAntLion::pDeathSounds[] = //UNDONE!: estos sonidos no se encuentran
{
	"antlion/die1.wav",
	"antlion/die2.wav",
	"antlion/die3.wav",
};


void CAntLion :: PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	if (RANDOM_LONG(0,5) < 2)
		EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CAntLion :: DeathSound( void )
{
	EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pDeathSounds[ RANDOM_LONG(0,ARRAYSIZE(pDeathSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
}

void CAntLion :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	m_bloodColor = BLOOD_COLOR_YELLOW; // Teh_Freak: is shot in the head, emit yellow blood
		
	switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar		
	{		
		case 0: EMIT_SOUND (ENT(pev), CHAN_BODY, "antlion/impact1.wav", 1, ATTN_NORM ); break;	
		case 1: EMIT_SOUND (ENT(pev), CHAN_BODY, "antlion/impact2.wav", 1, ATTN_NORM ); break;	
		case 2: EMIT_SOUND (ENT(pev), CHAN_BODY, "antlion/impact3.wav", 1, ATTN_NORM ); break;
	}

    if (ptr->iHitgroup == HITGROUP_HEAD)
    {
		if ( bitsDamageType & DMG_SHOTGUN )
		pev->skin = 1;
	}

	CBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void CAntLion :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hl2/antlion.mdl");

	Isantlion = TRUE;
	pev->framerate = 1.5;

	CZombie :: Spawn();
}
void CAntLion :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hl2/antlion.mdl");

	PRECACHE_SOUND("antlion/impact1.wav" );
	PRECACHE_SOUND("antlion/impact2.wav" );
	PRECACHE_SOUND("antlion/impact3.wav" );

	PRECACHE_SOUND("antlion/pain1.wav" );
	PRECACHE_SOUND("antlion/pain2.wav" );

	//event sounds
	PRECACHE_SOUND("antlion/idle1.wav" );
	PRECACHE_SOUND("antlion/idle2.wav" );
	PRECACHE_SOUND("antlion/idle3.wav" );
	PRECACHE_SOUND("antlion/idle4.wav" );

	PRECACHE_SOUND("antlion/foot1.wav" );
	PRECACHE_SOUND("antlion/foot2.wav" );
	PRECACHE_SOUND("antlion/foot3.wav" );

	PRECACHE_SOUND("antlion/single1.wav" );
	PRECACHE_SOUND("antlion/single2.wav" );
	PRECACHE_SOUND("antlion/single3.wav" );

	PRECACHE_SOUND("antlion/double1.wav" );
	PRECACHE_SOUND("antlion/double2.wav" );

	CZombie :: Precache();
}	


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

class CZombine : public CZombie
{
public:
	void Spawn( void );
	void Precache( void );

	void DeathSound( void );

	void TraceAttack( entvars_t *pevAttacker, 
	float flDamage, Vector vecDir, TraceResult *ptr, 
	int bitsDamageType);

	void PainSound( void );
	void  SetActivity ( Activity NewActivity );

	static const char *pPainSounds[];
	static const char *pDeathSounds[];
	static const char *pAlertSounds[];
	static const char *pIdleSounds[];

	void RunAI( void );
};

LINK_ENTITY_TO_CLASS( npc_zombine, CZombine );

const char *CZombine::pPainSounds[] = 
{
	"zombine/zombine_pain1.wav",
	"zombine/zombine_pain2.wav",
	"zombine/zombine_pain3.wav",
	"zombine/zombine_pain4.wav",
};

const char *CZombine::pDeathSounds[] =
{
	"zombine/zombine_die1.wav",
	"zombine/zombine_die2.wav",
};

const char *CZombine::pAlertSounds[] = 
{
	"zombine/zombine_alert1.wav",
	"zombine/zombine_alert2.wav",
	"zombine/zombine_alert3.wav",
	"zombine/zombine_alert4.wav",
	"zombine/zombine_alert5.wav",
	"zombine/zombine_alert6.wav",
	"zombine/zombine_alert7.wav"
};

const char *CZombine::pIdleSounds[] = 
{
	"zombine/zombine_idle1.wav",
	"zombine/zombine_idle2.wav",
	"zombine/zombine_idle3.wav",
	"zombine/zombine_idle4.wav"
};

void CZombine :: PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	if (RANDOM_LONG(0,5) < 2)
		EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CZombine :: DeathSound( void )
{
	EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pDeathSounds[ RANDOM_LONG(0,ARRAYSIZE(pDeathSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
}

void CZombine :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == HITGROUP_HEAD)
		m_bloodColor = BLOOD_COLOR_YELLOW; // Teh_Freak: is shot in the head, emit yellow blood
	else
		m_bloodColor = BLOOD_COLOR_RED; // Teh_Freak: if not, emit red blood


    if (ptr->iHitgroup == HITGROUP_HEAD)
    {
		if ( flDamage >= 20)
			bViolentHeadDamaged = TRUE;

		ALERT (at_aiconsole, "flDamage: %f\n", flDamage);
	}
		
	if ( pev->health <= 40)
	{
		b_Frustrated = TRUE;
	}

	CBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void CZombine :: RunAI( void )
{
	//low poly model
	SetBodygroup( 0, 2 );

	CBaseEntity *pEntidadLODcheck2 = NULL;

	while ((pEntidadLODcheck2 = UTIL_FindEntityByClassname(pEntidadLODcheck2, "player")) != NULL) 
	{	
		float flDist = (pEntidadLODcheck2->Center() - pev->origin).Length();

		if ( flDist <= 512)
			SetBodygroup( 0, 1 );
	}

	CBaseEntity *pEntidadLODcheck1 = NULL;
	
	while ((pEntidadLODcheck1 = UTIL_FindEntityByClassname(pEntidadLODcheck1, "player")) != NULL) 
	{	
		float flDist = (pEntidadLODcheck1->Center() - pev->origin).Length();

		if ( flDist <= 256)
			SetBodygroup( 0, 0 );
	}

	Vector vecHand;
	Vector vecAngle;

	GetAttachment( 1, vecHand, vecAngle );

	if (b_GrenadePulled)
	{
		m_flNextGrenadeCheck -= 0.1;

		if (m_flNextGrenadeCheck <= 0.1)//don't check zero, if so, the monster will explode as soon as he spawn
		{
			Killed( pev, GIB_NORMAL );

			ExplosionCreate( vecHand, vecAngle, edict(), 50, TRUE );
					
			SetBodygroup( 2, 0 );//hide gren

			m_flNextGrenadeCheck = 9999999;//ugly hack

			return;
		}
	}

//	ALERT( at_console, "b_GrenadePulled: %i, m_flNextGrenadeCheck: %f\n", b_GrenadePulled, m_flNextGrenadeCheck );

	CBaseMonster :: RunAI();
}

void CZombine :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hl2/zombine.mdl");

	CZombie :: Spawn();
}
void CZombine :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hl2/zombine.mdl");

		
	PRECACHE_SOUND("zombine/zombine_pain1.wav" );
	PRECACHE_SOUND("zombine/zombine_pain2.wav" );
	PRECACHE_SOUND("zombine/zombine_pain3.wav" );
	PRECACHE_SOUND("zombine/zombine_pain4.wav" );

	PRECACHE_SOUND("zombine/zombine_die1.wav" );
	PRECACHE_SOUND("zombine/zombine_die2.wav" );

	PRECACHE_SOUND("zombine/zombine_alert1.wav" );
	PRECACHE_SOUND("zombine/zombine_alert2.wav" );
	PRECACHE_SOUND("zombine/zombine_alert3.wav" );
	PRECACHE_SOUND("zombine/zombine_alert4.wav" );
	PRECACHE_SOUND("zombine/zombine_alert5.wav" );
	PRECACHE_SOUND("zombine/zombine_alert6.wav" );
	PRECACHE_SOUND("zombine/zombine_alert7.wav" );

	PRECACHE_SOUND("zombine/zombine_idle1.wav" );
	PRECACHE_SOUND("zombine/zombine_idle2.wav" );
	PRECACHE_SOUND("zombine/zombine_idle3.wav" );
	PRECACHE_SOUND("zombine/zombine_idle4.wav" );

	//event sounds
	PRECACHE_SOUND("zombine/gear1.wav" );
	PRECACHE_SOUND("zombine/gear2.wav" );
	PRECACHE_SOUND("zombine/gear3.wav" );

	CZombie :: Precache();
}

//=========================================================
// SetActivity 
//=========================================================
void CZombine :: SetActivity ( Activity NewActivity )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 3, vecGunPos, vecGunAngles );

	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{

	case ACT_RUN:	
		{
			if (!b_Frustrated)//i'm so relax, I don't need to run...
			{
				iSequence = LookupActivity ( ACT_WALK );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );//look for normal ACT_RUN
			}
		}
		break;
/*
	case ACT_WALK:
		{
			if ( pev->health <= LIMP_HEALTH )
			{
				// limp!
				iSequence = LookupActivity ( ACT_WALK_HURT );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );//look for normal ACT_WALK
			}	
		}
		break;
*/
	case ACT_MELEE_ATTACK1:	

		if (b_Frustrated)
		{
			if(!b_GrenadePulled)
			{
				iSequence = LookupSequence( "pullGrenade" );

				m_flNextGrenadeCheck = + 4;

				b_GrenadePulled = TRUE;
			}
			else//well... doesnt matter
			iSequence = LookupActivity ( NewActivity );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}	
	 break;


	default:
		iSequence = LookupActivity ( NewActivity );
		break;
	}
		
	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo( );
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT ( at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}
}