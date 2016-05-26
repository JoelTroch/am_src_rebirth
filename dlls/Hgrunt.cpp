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
// hgrunt
//=========================================================

#include	"extdll.h"
#include	"plane.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"animation.h"
#include	"squadmonster.h"
#include	"weapons.h"
#include	"talkmonster.h"
#include	"soundent.h"
#include	"effects.h"
#include	"customentity.h"

#include	"basemonster.h"
#include	"decals.h"

int g_fGruntQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int		g_iSkillLevel;
extern void ExplosionCreate( const Vector &center, const Vector &angles, edict_t *pOwner, int magnitude, BOOL doDamage );

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	GRUNT_CLIP_SIZE					30 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define HGRUNT_LIMP_HEALTH				20
#define HGRUNT_DMG_HEADSHOT				( DMG_BULLET | DMG_CLUB )	// damage types that can kill a grunt with a single headshot.
#define HGRUNT_NUM_HEADS				2 // how many grunt heads are there? 

#define	HGRUNT_SENTENCE_VOLUME			(float)0.35
#define GRUNT_ATTN						ATTN_NONE	// attenutation of grunt sentences

#define	PLAYER_MINIMUN_LIGHT			10// el valor de luz minimo permitido (a partir de ese valor para abajo no puede ver el player)

#define DOOR		1
#define LADDER		2
#define BREKABLE	3
#define	TRAIN		4		
#define	PLAT		5

#define HGRUNT_9MMAR				( 1 << 0)
#define HGRUNT_HANDGRENADE			( 1 << 1)
#define HGRUNT_GRENADELAUNCHER		( 1 << 2)
#define HGRUNT_SHOTGUN				( 1 << 3)
#define HGRUNT_M249					( 1 << 4)

#define		HEAD_GROUP					1
#define HEAD_GRUNT					0
#define HEAD_COMMANDER				1
#define HEAD_SHOTGUN				2
#define HEAD_M203					3
#define HEAD_NONE					4

	//used by opfor
#define HEAD_GLASSES				4
#define HEAD_MASK					5//medic
#define HEAD_HELMET					6
#define HEAD_FLASH					7

#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2

#define GUN_DEAGLE					0
#define GUN_LAW						1


// OPPOSING FORCE

		#define TORSO_GROUP					2
#define BODY_SAW				1
#define BODY_SHOTGUN			3

		#define GUN_GROUP_OPFOR				3
#define GUN_MP5_OPFOR				 	0
#define GUN_SHOTGUN_OPFOR			1
#define GUN_M249_OPFOR					2
#define GUN_NONE_OPFOR			      3 //FIX

/*
#define ARM_RIGHT_GROUP				3
#define ARM_RIGHT_NONE				1
#define ARM_LEFT_GROUP				4
#define ARM_LEFT_NONE				1
#define LEG_RIGHT_GROUP				5
#define LEG_RIGHT_NONE				1
#define LEG_LEFT_GROUP				6
#define LEG_LEFT_NONE				1
*/
		#define GRENADE_GROUP				3//3 in hgrunt normal mode
#define HIDE_GREN				 	0
#define SHOW_GREN			1

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		HGRUNT_AE_RELOAD		( 2 )
#define		HGRUNT_AE_KICK			( 3 )
#define		HGRUNT_AE_BURST1		( 4 )
#define		HGRUNT_AE_BURST2		( 5 ) 
#define		HGRUNT_AE_BURST3		( 6 ) 
#define		HGRUNT_AE_GREN_TOSS		( 7 )
#define		HGRUNT_AE_GREN_LAUNCH	( 8 )
#define		HGRUNT_AE_GREN_DROP		( 9 )
#define		HGRUNT_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		HGRUNT_AE_DROP_GUN		( 11) // grunt (probably dead, probably not) is dropping his mp5.

//NEW
#define		HGRUNT_AE_SHOW_GREN		( 18 )
#define		HGRUNT_AE_HIDE_GREN		( 19 )

//NEW | Show and hide the clip of m4 weapon
#define		HGRUNT_AE_SHOW_CLIP		( 15 )
#define		HGRUNT_AE_HIDE_CLIP		( 16 )

#define		HGRUNT_AE_DRAW			( 25 )

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_GRUNT_SUPPRESS = LAST_COMMON_SCHEDULE + 1,
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_GRUNT_COVER_AND_RELOAD,
	SCHED_GRUNT_SWEEP,
	SCHED_GRUNT_FOUND_ENEMY,
	SCHED_GRUNT_REPEL,
	SCHED_GRUNT_REPEL_ATTACK,
	SCHED_GRUNT_REPEL_LAND,
	SCHED_GRUNT_WAIT_FACE_ENEMY,
	SCHED_GRUNT_TAKECOVER_FAILED,// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_GRUNT_ELOF_FAIL,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_GRUNT_FACE_TOSS_DIR = LAST_COMMON_TASK + 1,
	TASK_GRUNT_SPEAK_SENTENCE,
	TASK_GRUNT_CHECK_FIRE,
};

//=========================================================
// monster-specific conditions
//=========================================================
#define bits_COND_GRUNT_NOFIRE	( bits_COND_SPECIAL1 )

class CHGrunt : public CSquadMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed ( void );
	int  Classify ( void );
	int ISoundMask ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL FCanCheckAttacks ( void );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	void CheckAmmo ( void );
	void SetActivity ( Activity NewActivity );
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	void DeathSound( void );
	void ShowDamage( void );
//	float m_flNextBloodTime;

	void PainSound( void );
	void IdleSound ( void );
	Vector GetGunPosition( void );
	void Shoot ( void );
	void Shotgun ( void );
	void ShootMachineGun ( void );//test
	void PrescheduleThink ( void );
	void GibMonster( void );
	void SpeakSentence( void );

//	void Killed( entvars_t *pevAttacker, int iGib );
////////
	int m_idGib_hgrunt;
	int m_idGib_hgrunt_a;
//	void CHGrunt ::	Gibs( void );

	void LegsGibs( void );
	void ArmsGibs( void );
///////
	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	CBaseEntity	*Kick( void );
	Schedule_t	*GetSchedule( void );
	Schedule_t  *GetScheduleOfType ( int Type );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	int IRelationship ( CBaseEntity *pTarget );

	BOOL FOkToSpeak( void );
	void JustSpoke( void );
	void RunAI( void );

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector	m_vecTossVelocity;

	BOOL	m_fThrowGrenade;
	BOOL	m_fStanding;
	BOOL	m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int		m_cClipSize;

	int m_voicePitch;

	int		m_iBrassShell;
	int		m_iShotgunShell;

	int		m_iSentence;
	static const char *pGruntSentences[];

//
	BOOL bPlayerVisible;//is the player hidden? can I see it?
	BOOL bGruntRPG;
	BOOL m_fGunDrawn;//check if the pistol has been deployed
	int	 m_iNumClips;//amount of clips
	int	 m_iPistolShell;

	BOOL b_mIsRobot;
	BOOL b_StartCount;
	float	 iNextExplodeTime;
	void EXPORT ExplodeThink();
};

LINK_ENTITY_TO_CLASS( monster_human_grunt, CHGrunt );

TYPEDESCRIPTION	CHGrunt::m_SaveData[] = 
{
	DEFINE_FIELD( CHGrunt, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHGrunt, m_flNextPainTime, FIELD_TIME ),
//	DEFINE_FIELD( CHGrunt, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD( CHGrunt, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHGrunt, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGrunt, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGrunt, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGrunt, m_cClipSize, FIELD_INTEGER ),
	DEFINE_FIELD( CHGrunt, m_voicePitch, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD( CHGrunt, m_iSentence, FIELD_INTEGER ),
//	DEFINE_FIELD( CHGrunt, bPlayerVisible, FIELD_BOOLEAN ),//don't save
	DEFINE_FIELD( CHGrunt, bGruntRPG, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGrunt, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGrunt, m_iNumClips, FIELD_INTEGER ),
	DEFINE_FIELD( CHGrunt, b_mIsRobot, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGrunt, b_StartCount, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGrunt, m_flNextGrenadeCheck, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CHGrunt, CSquadMonster );

const char *CHGrunt::pGruntSentences[] = 
{//speak as grunt
	"HG_GREN", // grenade scared grunt
	"HG_ALERT", // sees player
	"HG_MONSTER", // sees monster
	"HG_COVER", // running to cover
	"HG_THROW", // about to throw grenade
	"HG_CHARGE",  // running out to get the enemy
	"HG_TAUNT", // say rude things

	"HG_LIGHT", // when the lights go on
	"HG_CHECK" //fix: this is used but there isn't here... why?
	"HG_QUEST",
	"HG_IDLE",
	"HG_CLEAR",
	"HG_ANSWER"
};

enum
{
	HGRUNT_SENT_NONE = -1,
	HGRUNT_SENT_GREN = 0,
	HGRUNT_SENT_ALERT,
	HGRUNT_SENT_MONSTER,
	HGRUNT_SENT_COVER,
	HGRUNT_SENT_THROW,
	HGRUNT_SENT_CHARGE,
	HGRUNT_SENT_TAUNT,

	HGRUNT_SENT_LIGHT,
	HGRUNT_SENT_CHECK,
	HGRUNT_SENT_QUEST,
	HGRUNT_SENT_IDLE,
	HGRUNT_SENT_CLEAR,
	HGRUNT_SENT_ANSWER//no se si es necesario todo esto...
} HGRUNT_SENTENCE_TYPES;

//=========================================================
// Speak Sentence - say your cued up sentence.
//
// Some grunt sentences (take cover and charge) rely on actually
// being able to execute the intended action. It's really lame
// when a grunt says 'COVER ME' and then doesn't move. The problem
// is that the sentences were played when the decision to TRY
// to move to cover was made. Now the sentence is played after 
// we know for sure that there is a valid path. The schedule
// may still fail but in most cases, well after the grunt has 
// started moving.
//=========================================================
void CHGrunt :: SpeakSentence( void )
{
	if ( m_iSentence == HGRUNT_SENT_NONE )
	{
		// no sentence cued up.
		return; 
	}

	if (FOkToSpeak())
	{
		SENTENCEG_PlayRndSz( ENT(pev), pGruntSentences[ m_iSentence ], HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		JustSpoke();
	}
}

//=========================================================
// IRelationship - overridden because Alien Grunts are 
// Human Grunt's nemesis.
//=========================================================
int CHGrunt::IRelationship ( CBaseEntity *pTarget )
{
	if ( FClassnameIs( pTarget->pev, "monster_alien_grunt" ) || ( FClassnameIs( pTarget->pev,  "monster_gargantua" ) ) )
	{
		return R_NM;
	}

	return CSquadMonster::IRelationship( pTarget );
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CHGrunt :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

//	if ( GetBodygroup( 2 ) != 2 )
	// Si tiene arma ... y si NO tiene el flag de "no tirar arma"
	if ( GetBodygroup( 2 ) != 2 && !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
		if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
		{
			pGun = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
			pGun->pev->spawnflags |= SF_NORESPAWN;
		}
		else //sino tiene 9mmAR
		{
			pGun = DropItem( "weapon_m16", vecGunPos, vecGunAngles );

			pGun->pev->spawnflags |= SF_NORESPAWN;
		}
		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}
	
		if (FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ))
		{
			pGun = DropItem( "ammo_ARgrenades", vecGunPos, vecGunAngles );
			if ( pGun )
			{
				pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
				pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			}
		}
	}

	if(b_mIsRobot)
	{
		CGib::RobotGibs( pev, 8, 0 ); 
		CGib::RobotArmGibs( pev, 2, 1 );
		CGib::RobotHeadGib( pev );
		CGib::RobotLegGibs( pev, 2, 1 );

		return;
	}
	else
	{
		CGib::HgruntArmGib( pev );
		CGib::HgruntArmGib( pev );
		LegsGibs();
		LegsGibs();
	}

	CBaseMonster :: GibMonster();
}

//=========================================================
// ISoundMask - Overidden for human grunts because they 
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CHGrunt :: ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER;
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CHGrunt :: FOkToSpeak( void )
{
// if someone else is talking, don't speak
	if (gpGlobals->time <= CTalkMonster::g_talkWaitTime)
		return FALSE;

	if ( pev->spawnflags & SF_MONSTER_GAG )
	{
		if ( m_MonsterState != MONSTERSTATE_COMBAT )
		{
			// no talking outside of combat if gagged.
			return FALSE;
		}
	}

	// if player is not in pvs, don't speak
//	if (FNullEnt(FIND_CLIENT_IN_PVS(edict())))
//		return FALSE;
	
	return TRUE;
}

//=========================================================
//=========================================================
void CHGrunt :: JustSpoke( void )
{
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = HGRUNT_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CHGrunt :: PrescheduleThink ( void )
{
//	ALERT ( at_console, "PrescheduleThink \n" );
	if ( InSquad() && m_hEnemy != NULL )
	{
		if ( HasConditions ( bits_COND_SEE_ENEMY ) )
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if ( gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5 )
			{
				// been a while since we've seen the enemy
				MySquadLeader()->m_fEnemyEluded = TRUE;
			}
		}
	}
}

//=========================================================
// FCanCheckAttacks - this is overridden for human grunts
// because they can throw/shoot grenades when they can't see their
// target and the base class doesn't check attacks if the monster
// cannot see its enemy.
//
// !!!BUGBUG - this gets called before a 3-round burst is fired
// which means that a friendly can still be hit with up to 2 rounds. 
// ALSO, grenades will not be tossed if there is a friendly in front,
// this is a bad bug. Friendly machine gun fire avoidance
// will unecessarily prevent the throwing of a grenade as well.
//=========================================================
BOOL CHGrunt :: FCanCheckAttacks ( void )
{
	if ( !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CHGrunt :: CheckMeleeAttack1 ( float flDot, float flDist )
{
	CBaseMonster *pEnemy;

	if ( m_hEnemy != NULL )
	{
		pEnemy = m_hEnemy->MyMonsterPointer();

		if ( !pEnemy )
		{
			return FALSE;
		}
	}

	if ( flDist <= 64 && flDot >= 0.7	&& 
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack1 - overridden for HGrunt, cause 
// FCanCheckAttacks() doesn't disqualify all attacks based
// on whether or not the enemy is occluded because unlike
// the base class, the HGrunt can attack when the enemy is
// occluded (throw grenade over wall, etc). We must 
// disqualify the machine gun attack if the enemy is occluded.
//=========================================================
BOOL CHGrunt :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if(bGruntRPG)//only this class of monster by now
	{
		if (FBitSet( pev->weapons, HGRUNT_9MMAR))
		{
			if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 2048 && flDot >= 0.5 && NoFriendlyFire() )
			{
				TraceResult	tr;

				if ( flDist <= 256 )//dont shoot! 
				{
					return FALSE;
				}

				Vector vecSrc = GetGunPosition();

				// verify that a bullet fired from the gun will hit the enemy before the world.
				UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);

				if ( tr.flFraction == 1.0 )
				{
					return TRUE;
				}
			}
		}
		else
		{
			if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 2048 && flDot >= 0.5 && NoFriendlyFire() )
			{
				TraceResult	tr;

				if ( !m_hEnemy->IsPlayer() && flDist <= 64 )
				{
					// kick nonclients, but don't shoot at them.
					return FALSE;
				}

				Vector vecSrc = GetGunPosition();

				// verify that a bullet fired from the gun will hit the enemy before the world.
				UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);

				if ( tr.flFraction == 1.0 )
				{
					return TRUE;
				}
			}
		}
	}
	else
	{
		if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 2048 && flDot >= 0.5 && NoFriendlyFire() )
		{
			TraceResult	tr;

			if ( !m_hEnemy->IsPlayer() && flDist <= 64 )
			{
				// kick nonclients, but don't shoot at them.
				return FALSE;
			}

			Vector vecSrc = GetGunPosition();

			// verify that a bullet fired from the gun will hit the enemy before the world.
			UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);

			if ( tr.flFraction == 1.0 )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CHGrunt :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if (! FBitSet(pev->weapons, (HGRUNT_HANDGRENADE | HGRUNT_GRENADELAUNCHER)))
	{
		return FALSE;
	}
	
	// if the grunt isn't moving, it's ok to check.
	if ( m_flGroundSpeed != 0 )
	{
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextGrenadeCheck )
	{
		return m_fThrowGrenade;
	}

	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) && m_hEnemy->pev->waterlevel == 0 && m_vecEnemyLKP.z > pev->absmax.z  )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}
	
	Vector vecTarget;

	if (FBitSet( pev->weapons, HGRUNT_HANDGRENADE))
	{
		// find feet
		if (RANDOM_LONG(0,1))
		{
			// magically know where they are
			vecTarget = Vector( m_hEnemy->pev->origin.x, m_hEnemy->pev->origin.y, m_hEnemy->pev->absmin.z );
		}
		else
		{
			// toss it to where you last saw them
			vecTarget = m_vecEnemyLKP;
		}
		// vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
		// estimate position
		// vecTarget = vecTarget + m_hEnemy->pev->velocity * 2;
	}
	else
	{
		// find target
		// vecTarget = m_hEnemy->BodyTarget( pev->origin );
		vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
		// estimate position
		if (HasConditions( bits_COND_SEE_ENEMY))
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.hgruntGrenadeSpeed) * m_hEnemy->pev->velocity;
	}

	// are any of my squad members near the intended grenade impact area?
	if ( InSquad() )
	{
		if (SquadMemberInRange( vecTarget, 256 ))
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
			m_fThrowGrenade = FALSE;
		}
	}
	
	if ( ( vecTarget - pev->origin ).Length2D() <= 256 )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

		
	if (FBitSet( pev->weapons, HGRUNT_HANDGRENADE))
	{
		Vector vecToss = VecCheckToss( pev, GetGunPosition(), vecTarget, 0.5 );

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}
	else
	{
		Vector vecToss = VecCheckThrow( pev, GetGunPosition(), vecTarget, gSkillData.hgruntGrenadeSpeed, 0.5 );

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 0.3; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}

	

	return m_fThrowGrenade;
}


//=========================================================
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CHGrunt :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if(!b_mIsRobot)
	{
	// check for helmet shot	
	if (ptr->iHitgroup == HITGROUP_HEAD) // si el daño es en la cabeza
	{
		if (GetBodygroup( 1 ) == HEAD_MASK && (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB)))
		{
			// absorb damage
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet( ptr->vecEndPos, 1.0 );
				UTIL_Sparks( ptr->vecEndPos );
				// Sonidos (precache en world.cpp)
				switch (RANDOM_LONG(0,2)) 
				{
					case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/helmet1.wav", 0.9, ATTN_NORM); break;
					case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/helmet2.wav", 0.9, ATTN_NORM); break;
					case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/helmet3.wav", 0.9, ATTN_NORM); break;
				}
				flDamage = 0.01;
			}
		}
		else if (GetBodygroup( 1 ) == HEAD_GRUNT && (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB)))
		{
			// absorb damage
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet( ptr->vecEndPos, 1.0 );
				UTIL_Sparks( ptr->vecEndPos );
				// Sonidos (precache en world.cpp)
				switch (RANDOM_LONG(0,2)) 
				{
					case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/helmet1.wav", 0.9, ATTN_NORM); break;
					case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/helmet2.wav", 0.9, ATTN_NORM); break;
					case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/helmet3.wav", 0.9, ATTN_NORM); break;
				}
				flDamage = 0.01;
			}
		}
		else
		{
			ALERT( at_console, "Human Grunt Headshot!\n" );
			UTIL_BloodStream( ptr->vecEndPos,
						gpGlobals->v_forward * 10 +  // -5
						gpGlobals->v_up * 2, 
						(unsigned short)73, 100 );	
			//-35
			switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar
			{
			case 0: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot1.wav", 1, ATTN_NORM ); break;
			case 1: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot2.wav", 1, ATTN_NORM ); break;
			case 2: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot3.wav", 1, ATTN_NORM ); break;
			}
		}
	}
	else
	{
		switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar
		{
			case 0: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_hit1.wav", 1, ATTN_NORM ); break;
			case 1: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_hit2.wav", 1, ATTN_NORM ); break;
			case 2: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_hit3.wav", 1, ATTN_NORM ); break;
		}
	}
	}
	else
	{
		// absorb damage a bit
		flDamage -= 5;

		TraceResult trace = UTIL_GetGlobalTrace( );

		float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range

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
	}

	CSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}
//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CHGrunt :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	Forget( bits_MEMORY_INCOVER );

	return CSquadMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHGrunt :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:	
		ys = 150;		
		break;
	case ACT_RUN:	
		ys = 150;	
		break;
	case ACT_WALK:	
		ys = 180;		
		break;
	case ACT_RANGE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_RANGE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:	
		ys = 180;
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		ys = 30;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}
/*
void CHGrunt :: ShowDamage( void )
{
	if ( IsAlive() )
	{
		Vector		vecSpot;
		TraceResult	tr;

		vecSpot = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
		UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -24 ),  ignore_monsters, ENT(pev), & tr);
	// This isn't really blood.  So you don't have to screen it out based on violence levels (UTIL_ShouldShowBlood())
	
		UTIL_DecalTrace( &tr, DECAL_BLOODDRIP1 +RANDOM_LONG(0,5) );
	}
	
///	else
//	{
//		SetThink( NULL );
//	}
	
}
*/
void CHGrunt :: IdleSound( void )
{
	if (FOkToSpeak() && (g_fGruntQuestion || RANDOM_LONG(0,1)))
	{
		if (!g_fGruntQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0,2))
			{
			case 0: // check in
				if (!bPlayerVisible)
					SENTENCEG_PlayRndSz(ENT(pev), "HG_LIGHT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CHECK", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);					

				g_fGruntQuestion = 1;
				break;
			case 1: // question
				if (!bPlayerVisible)
					SENTENCEG_PlayRndSz(ENT(pev), "HG_LIGHT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "HG_QUEST", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
			
				g_fGruntQuestion = 2;
				break;
			case 2: // statement
				if (!bPlayerVisible)
					SENTENCEG_PlayRndSz(ENT(pev), "HG_LIGHT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "HG_IDLE", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CLEAR", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "HG_ANSWER", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CHGrunt :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}
//=========================================================
// RunAI Tomado del cod de Assassin
//=========================================================
void CHGrunt :: RunAI( void )
{
	CBaseMonster :: RunAI();
		
	edict_t *pClient;
	pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );

	float iPlayerLuces = GETENTITYILLUM( ENT ( pClient ) );
	
	if ( CVAR_GET_FLOAT("dev_light" ) != 0 )
	{
		ALERT ( at_aiconsole, "La luz sobre el player, segun el soldado es %f\n", iPlayerLuces );
				
		pev->rendermode = kRenderNormal;

		int rendertestvalue = iPlayerLuces;

		if (rendertestvalue >= 255)
			rendertestvalue = 255;

		pev->renderamt = rendertestvalue;
		pev->renderfx = 19;

		if (iPlayerLuces <= PLAYER_MINIMUN_LIGHT)
		{								
			pev->rendercolor.x = 255;
			pev->rendercolor.y = 0;
			pev->rendercolor.z = 0;
		}
		else
		{
			pev->rendercolor.x = 0;
			pev->rendercolor.y = 0;
			pev->rendercolor.z = 255;
		}
	}

//CHANGED ON REBIRTH

//	if (iPlayerLuces <= PLAYER_MINIMUN_LIGHT)
//		bPlayerVisible = FALSE;
//	else
		bPlayerVisible = TRUE;

	if(b_mIsRobot)
	{
		if(b_StartCount)
		{
			iNextExplodeTime -= 0.1;

			if (iNextExplodeTime <= 0.1)//don't check zero, if so, the monster will explode as soon as he spawn
			{/*
				Killed( pev, GIB_NORMAL );

				ExplosionCreate( Center(), Center(), edict(), 100, TRUE );
						
				iNextExplodeTime = 9999999;//ugly hack
*/
				return;
			}
		}
	}
	
//	ALERT( at_console, "b_StartCount: %i, iNextExplodeTime: %f\n", b_StartCount, iNextExplodeTime );
}

void CHGrunt :: ExplodeThink ( void )
{
	Killed( pev, GIB_ALWAYS );
	ExplosionCreate( Center(), Center(), edict(), 100, TRUE );
			
//	pev->nextthink = gpGlobals->time + .01;
}
//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CHGrunt :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_HUMAN_MILITARY;
}

//=========================================================
//=========================================================
CBaseEntity *CHGrunt :: Kick( void )
{
	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 70);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		return pEntity;
	}

	return NULL;
}

//=========================================================
// GetGunPosition	return the end of the barrel
//=========================================================

Vector CHGrunt :: GetGunPosition( )
{
	if (m_fStanding )
	{
		return pev->origin + Vector( 0, 0, 60 );
	}
	else
	{
		return pev->origin + Vector( 0, 0, 48 );
	}
}

//=========================================================
// Shoot
//=========================================================
void CHGrunt :: Shoot ( void )
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 

	if ( FBitSet( pev->weapons, HGRUNT_M249 ))//test
	{
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/saw1.wav", 1, ATTN_NORM );
	//VECTOR_CONE_3DEGREES
		
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 4048, BULLET_PLAYER_M249 /2); // shoot +-5 degrees

		pev->effects |= EF_MUZZLEFLASH;
		
		m_cAmmoLoaded--;// take away a bullet!

		Vector angDir = UTIL_VecToAngles( vecShootDir );
		SetBlending( 0, angDir.x );

			// Teh_Freak: World Lighting!
		 MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			  WRITE_BYTE( TE_DLIGHT );
			  WRITE_COORD( vecShootOrigin.x ); // origin
			  WRITE_COORD( vecShootOrigin.y );
			  WRITE_COORD( vecShootOrigin.z );
			  WRITE_BYTE( 20 );     // radius
			  WRITE_BYTE( 255 );     // R
			  WRITE_BYTE( 255 );     // G
			  WRITE_BYTE( 128 );     // B
			  WRITE_BYTE( 0 );     // life * 10
			  WRITE_BYTE( 0 ); // decay
		 MESSAGE_END();
		// Teh_Freak: World Lighting!
	}
	else//////////////// MP5
	{
		if(bGruntRPG)//only this class of monster by now
		{
			Vector vecShootOrigin = GetGunPosition();
			Vector vecShootDir = ShootAtEnemy( vecShootOrigin );
		
			GetAttachment( 1, vecShootOrigin, vecShootDir );

			Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);

			pev->effects |= EF_MUZZLEFLASH;
			
			CBaseEntity *pRocket = CBaseEntity::Create( "hvr_rocket", vecShootOrigin, UTIL_VecToAngles( vecShootDir ), edict() );
			if (pRocket)
				pRocket->pev->velocity = pev->velocity + gpGlobals->v_forward * 100;
			
			UTIL_MakeVectors ( pRocket->pev->angles );	
		}
		else
		{		
			FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_6DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
		}

		pev->effects |= EF_MUZZLEFLASH;
		
		m_cAmmoLoaded--;// take away a bullet!

		Vector angDir = UTIL_VecToAngles( vecShootDir );
		SetBlending( 0, angDir.x );

			// Teh_Freak: World Lighting!
		 MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			  WRITE_BYTE( TE_DLIGHT );
			  WRITE_COORD( vecShootOrigin.x ); // origin
			  WRITE_COORD( vecShootOrigin.y );
			  WRITE_COORD( vecShootOrigin.z );
			  WRITE_BYTE( 10 );     // radius
			  WRITE_BYTE( 255 );     // R
			  WRITE_BYTE( 255 );     // G
			  WRITE_BYTE( 128 );     // B
			  WRITE_BYTE( 0 );     // life * 10
			  WRITE_BYTE( 0 ); // decay
		 MESSAGE_END();
		// Teh_Freak: World Lighting!
	}
					
	 
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

	if (pPlayer->m_fSlowMotionOn)
	CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
}

//=========================================================
// Shoot
//=========================================================
void CHGrunt :: Shotgun ( void )
{	
	if (m_hEnemy == NULL)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	if(bGruntRPG)
	{
		switch (RANDOM_LONG(0,1))
		{
			case 0:	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 2048, BULLET_PLAYER_DEAGLE /2 ); break;
			case 1:	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_6DEGREES, 2048, BULLET_PLAYER_DEAGLE /2); break;
		}

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/deagle/deagle_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iPistolShell, TE_BOUNCE_SHELL); 
	}
	else
	{
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 

		FireBullets(gSkillData.hgruntShotgunPellets, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0 ); // shoot +-7.5 degrees
	}

	pev->effects |= EF_MUZZLEFLASH;
	//SP - jejeje, dejar esto asi :O
//	CBaseEntity *pRocket = CBaseEntity::Create( "rpg_rocket", vecShootOrigin, pev->angles, edict() );

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
		// Teh_Freak: World Lighting!
     MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
          WRITE_BYTE( TE_DLIGHT );
          WRITE_COORD( vecShootOrigin.x ); // origin
          WRITE_COORD( vecShootOrigin.y );
          WRITE_COORD( vecShootOrigin.z );
          WRITE_BYTE( 12 );     // radius
          WRITE_BYTE( 255 );     // R
          WRITE_BYTE( 255 );     // G
          WRITE_BYTE( 128 );     // B
          WRITE_BYTE( 0 );     // life * 10
          WRITE_BYTE( 0 ); // decay
     MESSAGE_END();
	// Teh_Freak: World Lighting!


	 CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

	 if (pPlayer->m_fSlowMotionOn)
	 {
		 /*
		CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
		CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
		CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
		CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
		CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
		CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
*/
	//CBullet::Shoot( pev, vecShootOrigin, vecShootDir + Vector (RANDOM_FLOAT(-32, 32),RANDOM_FLOAT(-32, 32),RANDOM_FLOAT(-32, 32)) * 500 );
	
		 CBullet::Shoot( pev, vecShootOrigin, vecShootDir + Vector (RANDOM_FLOAT(-32, 32),0,0) * 500 );
		 CBullet::Shoot( pev, vecShootOrigin, vecShootDir + Vector (RANDOM_FLOAT(-32, 32),0,0 )* 500 );
		 CBullet::Shoot( pev, vecShootOrigin, vecShootDir + Vector (RANDOM_FLOAT(-32, 32),0,0) * 500 );
		 CBullet::Shoot( pev, vecShootOrigin, vecShootDir + Vector (RANDOM_FLOAT(-32, 32),0,0 )* 500 );
		 CBullet::Shoot( pev, vecShootOrigin, vecShootDir + Vector (RANDOM_FLOAT(-32, 32),0,0) * 500 );
	 }

}
//=========================================================
// Shoot
//=========================================================
void CHGrunt :: ShootMachineGun ( void )
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

	pev->effects |= EF_MUZZLEFLASH;
	
	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );

		// Teh_Freak: World Lighting!
     MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
          WRITE_BYTE( TE_DLIGHT );
          WRITE_COORD( vecShootOrigin.x ); // origin
          WRITE_COORD( vecShootOrigin.y );
          WRITE_COORD( vecShootOrigin.z );
          WRITE_BYTE( 10 );     // radius
          WRITE_BYTE( 255 );     // R
          WRITE_BYTE( 255 );     // G
          WRITE_BYTE( 128 );     // B
          WRITE_BYTE( 0 );     // life * 10
          WRITE_BYTE( 0 ); // decay
     MESSAGE_END();
	// Teh_Freak: World Lighting!

}
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CHGrunt :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 0, vecGunPos, vecGunAngles );

	switch( pEvent->event )
	{
		case HGRUNT_AE_DRAW:
			{
				SetBodygroup( GUN_GROUP, GUN_DEAGLE );
			
				EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/weapon_deploy.wav", 1, ATTN_NORM, 0, 100 );
				m_fGunDrawn = TRUE;
					
//				ALERT ( at_aiconsole, "INFO: m_fGunDrawn = TRUE\n" );
			}
		break;	

		case HGRUNT_AE_DROP_GUN:
				{
					//SP: si tiene el flag de "no tirar arma" cerrar con break y no ejecutar
					//mas codigo
					if (pev->spawnflags & SF_MONSTER_NO_WPN_DROP) break; //LRC

					Vector	vecGunPos;
					Vector	vecGunAngles;

					GetAttachment( 0, vecGunPos, vecGunAngles );

					// switch to body group with no gun.
					//SP: si tiene el model del Op4 el grupo esta desordenado, ordenarlo y sacar el weapon

					if ( FClassnameIs ( pev, "monster_hgrunt_opfor" ) )
					SetBodygroup( GUN_GROUP_OPFOR, GUN_NONE_OPFOR );
					else
					SetBodygroup( GUN_GROUP, GUN_NONE );

					CBaseEntity *pItem;

					// now spawn a gun.
					if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
					{
						pItem = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
						pItem->pev->spawnflags |= SF_NORESPAWN; // No respawn
					}
						
					else if (FBitSet( pev->weapons, HGRUNT_M249 ))
					{
						pItem = DropItem( "weapon_saw", vecGunPos, vecGunAngles );//test
					}
					else
					{
						if(bGruntRPG)//only this class of monster by now
						pItem = DropItem( "weapon_rpg", vecGunPos, vecGunAngles );
						else
						pItem = DropItem( "weapon_m16", vecGunPos, vecGunAngles );

						pItem->pev->spawnflags |= SF_NORESPAWN; // No respawn
					}

					if (FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ))
					{
						pItem = DropItem( "ammo_ARgrenades", BodyTarget( pev->origin ), vecGunAngles );
					}

					//CODE: if is a rpg grunt, has no ammo, and have law weapon -> give it a new weapon
					if(bGruntRPG)//only this class of monster by now
					{
						if(m_iNumClips <=0)//if no ammo
						{		
							if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
							{
								//the pistol has infinite ammo
							}
							else
							{
								//give it a new weapon
								switch (RANDOM_LONG(0,1))
								{
									case 0: pev->weapons = HGRUNT_SHOTGUN | HGRUNT_HANDGRENADE; break;
									case 1: pev->weapons = HGRUNT_SHOTGUN; break;
								}

								m_iNumClips = 999;//at this point the monster doens't have any clips, give it a few of them
							
								//edit, change the clip size and give ammo
								m_cClipSize = 7;
								m_cAmmoLoaded = m_cClipSize;//give it ammo! (dont reload weapon when he draw it)
												
								ClearConditions(bits_COND_NO_AMMO_LOADED);//this could be necessary
							}
						}
					}
				}
			break;

		case HGRUNT_AE_RELOAD:
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);

			//DropItem( "item_clip_rifle", vecGunPos, vecGunAngles );//test
			break;

		case HGRUNT_AE_GREN_TOSS:
		{
			/*
			if ( GETENTITYILLUM( ENT(pev) ) > 20 )//SYS TEST
			{
				ALERT ( at_console, "NO PUEDO VER!\n" );
			}
			else
			{
			*/
				UTIL_MakeVectors( pev->angles );
				// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
			
				//shit! its too dark here!
				if (!bPlayerVisible)
				CGrenade::FlashShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 30.5 );
				else
				{
				//	if (RANDOM_LONG(0,1))
					CGrenade::ShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 3.5 );
				//	else
				//	CGrenade::ShootTimedSmoke( pev, GetGunPosition(), m_vecTossVelocity, 23.5 );
				}

				m_fThrowGrenade = FALSE;
				m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
				// !!!LATER - when in a group, only try to throw grenade if ordered.
		//	}
		}
		break;

		case HGRUNT_AE_GREN_LAUNCH:
		{
			if (!bPlayerVisible)
			CGrenade::FlashShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 30.5 );
			else
			CGrenade::ShootContact( pev, GetGunPosition(), m_vecTossVelocity );
		
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM);
			m_fThrowGrenade = FALSE;
			
			if (g_iSkillLevel == SKILL_HARD)
				m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT( 2, 3 );// wait a random amount of time before shooting again
			else
				m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		break;

		case HGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
					
			if (!bPlayerVisible)
			CGrenade::FlashShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 30 );
			else
			{
			//	if (RANDOM_LONG(0,1))
				CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
			//	else
			//	CGrenade::ShootTimedSmoke( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 23 );
			}
		}
		break;

		//show grenade: the monster pick up a grenade
/*
		case HGRUNT_AE_SHOW_GREN:
		{	
			if (pev->frags != 1)
			SetBodygroup( GRENADE_GROUP, SHOW_GREN );
		}
		break;
		//hide grenade: finally, the monster throw the grenade
		case HGRUNT_AE_HIDE_GREN:
		{
			if (pev->frags != 1)
			SetBodygroup( GRENADE_GROUP, HIDE_GREN );
		}
		break;
*/
		case HGRUNT_AE_BURST1:
		{
			if ( FBitSet( pev->weapons, HGRUNT_9MMAR ))
			{
				//acelerar a los maricones
				pev->framerate = 1.5;//asi esta perfecto
			
				if(!bGruntRPG)
				{
					EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/m161.wav", 1, ATTN_NORM );

//					EMIT_FMOD_SOUND( entindex(), pev->origin, "../media/m161.wav", 170, 100 );//fo0z
				}

				Shoot();
			}
			else
			{
				Shotgun( );

				if(!bGruntRPG)
				EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/shotgun/sbarrel1.wav", 1, ATTN_NORM );
			}
		
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		}
		break;

		case HGRUNT_AE_BURST2:
		case HGRUNT_AE_BURST3:
			Shoot();
			break;

		case HGRUNT_AE_KICK:
		{
			CBaseEntity *pHurt = Kick();

			if ( pHurt )
			{
				// SOUND HERE!
				UTIL_MakeVectors( pev->angles );
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
				pHurt->TakeDamage( pev, pev, gSkillData.hgruntDmgKick, DMG_CLUB );
			}
		}
		break;

		case HGRUNT_AE_CAUGHT_ENEMY:
		{
			if ( FOkToSpeak() )
			{
				SENTENCEG_PlayRndSz(ENT(pev), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				 JustSpoke();
			}

		}

		default:
			CSquadMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CHGrunt :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	SET_MODEL( ENT(pev),"physics/models/hgrunt.mdl");
	else
	SET_MODEL( ENT(pev),"models/hgrunt.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //creo q algo de 100 o 70...
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP /*| bits_CAP_STRAFE*/ ; //jejeje

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.
	bGruntRPG			= FALSE;

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if (pev->weapons == 0)
	{
		switch (RANDOM_LONG(0,3))
		{
			case 0: pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE; break;		
			case 1: pev->weapons = HGRUNT_SHOTGUN | HGRUNT_HANDGRENADE; break;
			case 2: pev->weapons = HGRUNT_9MMAR; break;
			case 3: pev->weapons = HGRUNT_SHOTGUN; break;
		}
	}

	if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
	{
		m_cClipSize		= 8;
			
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
	}
	else//M16
	{
		m_cClipSize		= GRUNT_CLIP_SIZE;

		SetBodygroup( GUN_GROUP, GUN_MP5 );
	}

	//random heads
	switch (RANDOM_LONG(0,3))
	{
		case 0: SetBodygroup( HEAD_GROUP, HEAD_GRUNT ); break;
		case 1:	SetBodygroup( HEAD_GROUP, HEAD_COMMANDER ); break;
		case 2: SetBodygroup( HEAD_GROUP, HEAD_SHOTGUN ); break;
		case 3: SetBodygroup( HEAD_GROUP, HEAD_M203 ); break;	
	}

	//set random skins
	if (RANDOM_LONG( 0, 99 ) < 80)
	pev->skin = 0;	// light skin
	else
	pev->skin = 1;	// dark skin
	 
	//oh dear! commander is not a black guy!
	if ( GetBodygroup( HEAD_GROUP ) == HEAD_COMMANDER )
	pev->skin = 0;	// always light skin

	//right, the black guy must have dark arms
	if ( GetBodygroup( HEAD_GROUP ) == HEAD_M203 )
	pev->skin = 1;	// always dark skin

	/*
	if (FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ))//this is really really necessary?
	{
		SetBodygroup( HEAD_GROUP, HEAD_M203 );
	}
	*/

	m_cAmmoLoaded		= m_cClipSize;
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHGrunt :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	PRECACHE_MODEL("physics/models/hgrunt.mdl");
	else
	PRECACHE_MODEL("models/hgrunt.mdl");

//	PRECACHE_MODEL("models/stickygib.mdl");

	PRECACHE_SOUND ("hgrunt/saw1.wav");// test

	PRECACHE_SOUND( "hgrunt/m161.wav" );
	PRECACHE_SOUND( "hgrunt/mp51.wav" );

//	PRECACHE_SOUND( "hgrunt/gr_mgun1.wav" );
//	PRECACHE_SOUND( "hgrunt/gr_mgun2.wav" );

//	PRECACHE_SOUND( "hgrunt/M16_mgun1.wav" );
//	PRECACHE_SOUND( "hgrunt/M16_mgun2.wav" );

	PRECACHE_SOUND( "hgrunt/gr_die1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die3.wav" );

	PRECACHE_SOUND( "hgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain5.wav" );

	PRECACHE_SOUND( "hgrunt/gr_reload1.wav" );

	PRECACHE_SOUND( "weapons/glauncher.wav" );

//	PRECACHE_SOUND( "weapons/sbarrel1.wav" );
//	PRECACHE_SOUND( "hgrunt/usas_fire-1.wav" );

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	//SYS FIX: no sacar el pitch, sino disminuirlo. Un pitch elevado causa voz de maricon
	// get voice pitch
	if (RANDOM_LONG(0,1))//50% de posibilidad
		m_voicePitch = 95;//109 + 0 o 7, algo de eso, nah...
	else
		m_voicePitch = 100;
/*
	if (RANDOM_LONG(0,1))//50% de posibilidad
		m_voicePitch = 109 + RANDOM_LONG(0,7);//109 + 0 o 7, algo de eso, nah...
	else
		m_voicePitch = 100;
*/
		
	PRECACHE_SOUND("common/hg_step1.wav");		//SP: Soldiers walk on concrete
	PRECACHE_SOUND("common/hg_step2.wav");
	PRECACHE_SOUND("common/hg_step3.wav");
	PRECACHE_SOUND("common/hg_step4.wav");

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");

	PRECACHE_MODEL( "models/gib_hgrunt.mdl" );
	PRECACHE_MODEL( "models/gib_hgrunt_a.mdl" );

	m_idGib_hgrunt = PRECACHE_MODEL( "models/gib_hgrunt.mdl" );
	m_idGib_hgrunt_a = PRECACHE_MODEL( "models/gib_hgrunt_a.mdl" );
}	

//=========================================================
// start task
//=========================================================
void CHGrunt :: StartTask ( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
	case TASK_GRUNT_CHECK_FIRE:
		if ( !NoFriendlyFire() )
		{
			SetConditions( bits_COND_GRUNT_NOFIRE );
		}
		TaskComplete();
		break;

	case TASK_GRUNT_SPEAK_SENTENCE:
		SpeakSentence();
		TaskComplete();
		break;

	case TASK_STRAFE_PATH:
		{
			Vector2D	vec2DirToPoint; 
			Vector2D	vec2RightSide;

			// to start strafing, we have to first figure out if the target is on the left side or right side
			UTIL_MakeVectors ( pev->angles );

			vec2DirToPoint = ( m_Route[ 0 ].vecLocation - pev->origin ).Make2D().Normalize();
			vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

			if ( DotProduct ( vec2DirToPoint, vec2RightSide ) > 0 )
			{
				// strafe right
				m_movementActivity = ACT_STRAFE_RIGHT;
			}
			else
			{
				// strafe left
				m_movementActivity = ACT_STRAFE_LEFT;
			}
			TaskComplete();
			break;
		}

	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		// grunt no longer assumes he is covered if he moves
		Forget( bits_MEMORY_INCOVER );
		CSquadMonster ::StartTask( pTask );
		break;

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_GRUNT_FACE_TOSS_DIR:
		break;

	case TASK_FACE_IDEAL:
	case TASK_FACE_ENEMY:
		CSquadMonster :: StartTask( pTask );
		if (pev->movetype == MOVETYPE_FLY)
		{
			m_IdealActivity = ACT_GLIDE;
		}
		break;
	default: 
		CSquadMonster :: StartTask( pTask );
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CHGrunt :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_GRUNT_FACE_TOSS_DIR:
		{
			// project a point along the toss vector and turn to face that point.
			MakeIdealYaw( pev->origin + m_vecTossVelocity * 64 );
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				m_iTaskStatus = TASKSTATUS_COMPLETE;
			}
			break;
		}
	default:
		{
			CSquadMonster :: RunTask( pTask );
			break;
		}
	}
}

//=========================================================
// PainSound
//=========================================================
void CHGrunt :: PainSound ( void )
{
	if(b_mIsRobot)
		return;

	if ( gpGlobals->time > m_flNextPainTime )
	{
#if 0
		if ( RANDOM_LONG(0,99) < 5 )
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "HG_PAIN", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
				JustSpoke();
				return;
			}
		}
#endif 
		switch ( RANDOM_LONG(0,6) )
		{
		case 0:	
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_pain3.wav", 1, ATTN_NORM );	
			break;
		case 1:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_pain4.wav", 1, ATTN_NORM );	
			break;
		case 2:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_pain5.wav", 1, ATTN_NORM );	
			break;
		case 3:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_pain1.wav", 1, ATTN_NORM );	
			break;
		case 4:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_pain2.wav", 1, ATTN_NORM );	
			break;
		}

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CHGrunt :: DeathSound ( void )
{
	if(!b_mIsRobot)
	{
		switch ( RANDOM_LONG(0,2) )
		{
		case 0:	
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_die1.wav", 1, ATTN_IDLE );	
			break;
		case 1:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_die2.wav", 1, ATTN_IDLE );	
			break;
		case 2:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_die3.wav", 1, ATTN_IDLE );	
			break;
		}
	}
	else
	{	
		if ( m_LastHitGroup == HITGROUP_HEAD)
		{
			SetBodygroup( 1, 1 );	
			CGib::RobotHeadGib( pev );
		}			
		else if ( m_LastHitGroup == HITGROUP_LEFTARM)
		{
			SetBodygroup( 3, 1 );
			CGib::RobotArmGibs( pev, 1, 1 );
		}
		else if ( m_LastHitGroup == HITGROUP_RIGHTARM)
		{
			SetBodygroup( 4, 1 );
			CGib::RobotArmGibs( pev, 1, 1 );
		}
		else if (m_LastHitGroup == HITGROUP_LEFTLEG)
		{
			SetBodygroup( 5, 1 );
			CGib::RobotLegGibs( pev, 1, 1 );
		}
		else if( m_LastHitGroup == HITGROUP_RIGHTLEG)
		{
			SetBodygroup( 6, 1 );
			CGib::RobotLegGibs( pev, 1, 1 );
		}
	
		switch ( RANDOM_LONG(0,2) )
		{
		case 0:	
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "robot/robot_die1.wav", 1, ATTN_IDLE );	
			break;
		case 1:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "robot/robot_die2.wav", 1, ATTN_IDLE );	
			break;
		case 2:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "robot/robot_die3.wav", 1, ATTN_IDLE );	
			break;
		}

		UTIL_MakeVectors( pev->angles );
		CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );

		Vector vecSrc, vecAng;
		GetAttachment( 1, vecSrc, vecAng );

		if (pev->dmgtime + RANDOM_FLOAT( 0, 2 ) > gpGlobals->time)
		{
			// lots of smoke
			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_SMOKE );
				WRITE_COORD( vecSrc.x + RANDOM_FLOAT( -16, 16 ) );
				WRITE_COORD( vecSrc.y + RANDOM_FLOAT( -16, 16 ) );
				WRITE_COORD( vecSrc.z - 32 );
				WRITE_SHORT( g_sModelIndexSmoke );
				WRITE_BYTE( 15 ); // scale * 10
				WRITE_BYTE( 8 ); // framerate
			MESSAGE_END();
		}

		if(!b_StartCount)
		{
			b_StartCount = TRUE;
			iNextExplodeTime = + 4;
					
		//	SetThink ( ExplodeThink );
		//	pev->nextthink = gpGlobals->time + 5;
		}
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// GruntFail
//=========================================================
Task_t	tlGruntFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slGruntFail[] =
{
	{
		tlGruntFail,
		ARRAYSIZE ( tlGruntFail ),
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2,
		0,
		"Grunt Fail"
	},
};

//=========================================================
// Grunt Combat Fail
//=========================================================
Task_t	tlGruntCombatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slGruntCombatFail[] =
{
	{
		tlGruntCombatFail,
		ARRAYSIZE ( tlGruntCombatFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Grunt Combat Fail"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t	tlGruntVictoryDance[] =
{
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_FACE_ENEMY,						(float)0					},
	{ TASK_WAIT,							(float)1.5					},
	{ TASK_GET_PATH_TO_ENEMY_CORPSE,		(float)0					},
	{ TASK_WALK_PATH,						(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0					},
	{ TASK_FACE_ENEMY,						(float)0					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
};

Schedule_t	slGruntVictoryDance[] =
{
	{ 
		tlGruntVictoryDance,
		ARRAYSIZE ( tlGruntVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"GruntVictoryDance"
	},
};
//=========================================================
// GruntEnemyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlGruntEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
};

Schedule_t slGruntEnemyDraw[] = 
{
	{
		tlGruntEnemyDraw,
		ARRAYSIZE ( tlGruntEnemyDraw ),
		0,
		0,
		"Grunt Enemy Draw"
	}
};

//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlGruntEstablishLineOfFire[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_GRUNT_ELOF_FAIL	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0						},
	{ TASK_GRUNT_SPEAK_SENTENCE,(float)0						},
	{ TASK_RUN_PATH,			(float)0						},

//	{ TASK_STRAFE_PATH,			(float)0						}, //1
//SP test
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slGruntEstablishLineOfFire[] =
{
	{ 
		tlGruntEstablishLineOfFire,
		ARRAYSIZE ( tlGruntEstablishLineOfFire ),
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"GruntEstablishLineOfFire"
	},
};

//=========================================================
// GruntFoundEnemy - grunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
Task_t	tlGruntFoundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL1			},
};

Schedule_t	slGruntFoundEnemy[] =
{
	{ 
		tlGruntFoundEnemy,
		ARRAYSIZE ( tlGruntFoundEnemy ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"GruntFoundEnemy"
	},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t	tlGruntCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1.5					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_SWEEP	},
};

Schedule_t	slGruntCombatFace[] =
{
	{ 
		tlGruntCombatFace1,
		ARRAYSIZE ( tlGruntCombatFace1 ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_ENEMY_DEAD			|
		bits_COND_CAN_RANGE_ATTACK1		|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Combat Face"
	},
};

//=========================================================
// Suppressing fire - don't stop shooting until the clip is
// empty or grunt gets hurt.
//=========================================================
Task_t	tlGruntSignalSuppress[] =
{
	{ TASK_STOP_MOVING,					0						},
	{ TASK_FACE_IDEAL,					(float)0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_SIGNAL2		},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
};

Schedule_t	slGruntSignalSuppress[] =
{
	{ 
		tlGruntSignalSuppress,
		ARRAYSIZE ( tlGruntSignalSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SignalSuppress"
	},
};

Task_t	tlGruntSuppress[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
};

Schedule_t	slGruntSuppress[] =
{
	{ 
		tlGruntSuppress,
		ARRAYSIZE ( tlGruntSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"Suppress"
	},
};


//=========================================================
// grunt wait in cover - we don't allow danger or the ability
// to attack to break a grunt's run to cover schedule, but
// when a grunt is in cover, we do want them to attack if they can.
//=========================================================
Task_t	tlGruntWaitInCover[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)1					},
};

Schedule_t	slGruntWaitInCover[] =
{
	{ 
		tlGruntWaitInCover,
		ARRAYSIZE ( tlGruntWaitInCover ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_HEAR_SOUND		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2,

		bits_SOUND_DANGER,
		"GruntWaitInCover"
	},
};

//=========================================================
// run to cover.
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
Task_t	tlGruntTakeCover1[] =
{
//SP test
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_GRUNT_TAKECOVER_FAILED	},
	{ TASK_WAIT,					(float)0.2							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_GRUNT_SPEAK_SENTENCE,	(float)0							},	
	{ TASK_RUN_PATH,				(float)0							},

//	{ TASK_STRAFE_PATH,				(float)0							}, //2

	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_WAIT_FACE_ENEMY	},
};

Schedule_t	slGruntTakeCover[] =
{
	{ 
		tlGruntTakeCover1,
		ARRAYSIZE ( tlGruntTakeCover1 ), 
		0,
		0,
		"TakeCover"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlGruntGrenadeCover1[] =
{
	{ TASK_STOP_MOVING,						(float)0							},
	{ TASK_FIND_COVER_FROM_ENEMY,			(float)99							},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384							},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SPECIAL_ATTACK1			},
	{ TASK_CLEAR_MOVE_WAIT,					(float)0							},
	{ TASK_RUN_PATH,						(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_GRUNT_WAIT_FACE_ENEMY	},
};

Schedule_t	slGruntGrenadeCover[] =
{
	{ 
		tlGruntGrenadeCover1,
		ARRAYSIZE ( tlGruntGrenadeCover1 ), 
		0,
		0,
		"GrenadeCover"
	},
};


//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlGruntTossGrenadeCover1[] =
{
	{ TASK_FACE_ENEMY,						(float)0							},
	{ TASK_RANGE_ATTACK2, 					(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t	slGruntTossGrenadeCover[] =
{
	{ 
		tlGruntTossGrenadeCover1,
		ARRAYSIZE ( tlGruntTossGrenadeCover1 ), 
		0,
		0,
		"TossGrenadeCover"
	},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t	tlGruntTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slGruntTakeCoverFromBestSound[] =
{
	{ 
		tlGruntTakeCoverFromBestSound,
		ARRAYSIZE ( tlGruntTakeCoverFromBestSound ), 
		0,
		0,
		"GruntTakeCoverFromBestSound"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlGruntHideReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slGruntHideReload[] = 
{
	{
		tlGruntHideReload,
		ARRAYSIZE ( tlGruntHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntHideReload"
	}
};

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t	tlGruntSweep[] =
{
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
};

Schedule_t	slGruntSweep[] =
{
	{ 
		tlGruntSweep,
		ARRAYSIZE ( tlGruntSweep ), 
		
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER		|
		bits_SOUND_PLAYER,

		"Grunt Sweep"
	},
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlGruntRangeAttack1A[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_CROUCH },
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slGruntRangeAttack1A[] =
{
	{ 
		tlGruntRangeAttack1A,
		ARRAYSIZE ( tlGruntRangeAttack1A ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"Range Attack1A"
	},
};


//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlGruntRangeAttack1B[] =
{
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_IDLE_ANGRY  },
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slGruntRangeAttack1B[] =
{
	{ 
		tlGruntRangeAttack1B,
		ARRAYSIZE ( tlGruntRangeAttack1B ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Range Attack1B"
	},
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlGruntRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_GRUNT_FACE_TOSS_DIR,		(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_WAIT_FACE_ENEMY	},// don't run immediately after throwing grenade.
};

Schedule_t	slGruntRangeAttack2[] =
{
	{ 
		tlGruntRangeAttack2,
		ARRAYSIZE ( tlGruntRangeAttack2 ), 
		0,
		0,
		"RangeAttack2"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlGruntRepel[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_GLIDE 	},
};

Schedule_t	slGruntRepel[] =
{
	{ 
		tlGruntRepel,
		ARRAYSIZE ( tlGruntRepel ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"Repel"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlGruntRepelAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_FLY 	},
};

Schedule_t	slGruntRepelAttack[] =
{
	{ 
		tlGruntRepelAttack,
		ARRAYSIZE ( tlGruntRepelAttack ), 
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Repel Attack"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t	tlGruntRepelLand[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slGruntRepelLand[] =
{
	{ 
		tlGruntRepelLand,
		ARRAYSIZE ( tlGruntRepelLand ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"Repel Land"
	},
};


DEFINE_CUSTOM_SCHEDULES( CHGrunt )
{
	slGruntFail,
	slGruntCombatFail,
	slGruntVictoryDance,
	slGruntEnemyDraw,
	slGruntEstablishLineOfFire,
	slGruntFoundEnemy,
	slGruntCombatFace,
	slGruntSignalSuppress,
	slGruntSuppress,
	slGruntWaitInCover,
	slGruntTakeCover,
	slGruntGrenadeCover,
	slGruntTossGrenadeCover,
	slGruntTakeCoverFromBestSound,
	slGruntHideReload,
	slGruntSweep,
	slGruntRangeAttack1A,
	slGruntRangeAttack1B,
	slGruntRangeAttack2,
	slGruntRepel,
	slGruntRepelAttack,
	slGruntRepelLand,
};

IMPLEMENT_CUSTOM_SCHEDULES( CHGrunt, CSquadMonster );

//=========================================================
// SetActivity 
//=========================================================
void CHGrunt :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RELOAD:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet( pev->weapons, HGRUNT_9MMAR))				
		{	
			//the monster will try to reload... if it don't have clips, drop the weapon
			if(bGruntRPG)//only this class of monster by now
			{
				iSequence = LookupSequence( "Drop_Weapon" );
				m_iNumClips--;//less clip!
			}
			else
			{
				iSequence = LookupSequence( "reload_mp5" );
			}
		}
		else
		{
			if(bGruntRPG)//only this class of monster by now
			iSequence = LookupSequence( "reload_pistol" );
			else
			iSequence = LookupSequence( "reload_shotgun" );
		}
		break;

	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet( pev->weapons, HGRUNT_9MMAR))
		{
			if(bGruntRPG)//only this class of monster by now
			{
				if ( m_fStanding )
					iSequence = LookupSequence( "standing_law" );
				else
					iSequence = LookupSequence( "crouching_law" );
			}
			else
			{
				if ( m_fStanding )
				{
					// get aimable sequence
					/*
					if (RANDOM_LONG(0,1))
					iSequence = LookupSequence( "standing_mp5" );
					else
					iSequence = LookupSequence( "frenetic_mp5" );
	*/
					iSequence = LookupSequence( "standing_mp5" );
				}
				else
				{
					// get crouching shoot
					iSequence = LookupSequence( "crouching_mp5" );///crouching_mp5
				}
			}
		}
		else if (FBitSet( pev->weapons, HGRUNT_M249))//test
		{
			if ( m_fStanding )
			{
				iSequence = LookupSequence( "standing_saw" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_saw" );//"crouching_saw" );
			}
		}
		else//shotgun
		{
			if(bGruntRPG)//only this class of monster by now
			{
				if ( m_fStanding )
					iSequence = LookupSequence( "standing_pistol" );
				else
					iSequence = LookupSequence( "crouching_pistol" );
			}
			else
			{
				if ( m_fStanding )
					iSequence = LookupSequence( "standing_shotgun" );
				else
					iSequence = LookupSequence( "crouching_shotgun" );//crouching_shotgun
			}
		}
		break;
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown 
		// grenade or fired grenade, we must determine which and pick proper sequence
		if ( pev->weapons & HGRUNT_HANDGRENADE )
		{
			// get toss anim
			iSequence = LookupSequence( "throwgrenade" );
		}
		else
		{
			// get launch anim
			iSequence = LookupSequence( "launchgrenade" );
		}
		break;




	case ACT_RUN:	
		{
			if ( (pev->health <= HGRUNT_LIMP_HEALTH) /*|| (m_fHasBeenHit = TRUE) */)//ouch!
			{
				// limp!
				iSequence = LookupActivity ( ACT_RUN_HURT );
			}
			else
			{
				if(bGruntRPG)//only this class of monster by now
				{
					iSequence = LookupSequence( "run_pistol" );
				}
				else
				{
					iSequence = LookupActivity ( NewActivity );//look for normal ACT_RUN
				}
			}
		}
		break;

	case ACT_WALK:
		{
			if ( pev->health <= HGRUNT_LIMP_HEALTH )
			{
				// limp!
				iSequence = LookupActivity ( ACT_WALK_HURT );
			}
			else
			{
				if(bGruntRPG)//only this class of monster by now
				{
					iSequence = LookupSequence( "walk1_pistol" );
				}
				else
				{
					iSequence = LookupActivity ( NewActivity );//look for normal ACT_WALK
				}
			}
		}
		break;


	case ACT_IDLE:
		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity ( NewActivity );
		break;

	case ACT_IDLE_ANGRY:
		{				
			if(bGruntRPG)//only this class of monster by now
				iSequence = LookupSequence( "combatidle_pistol" );
			else
				iSequence = LookupActivity ( NewActivity );
		}
	break;

	case ACT_TURN_LEFT:
		{
			if(bGruntRPG)//only this class of monster by now
				iSequence = LookupSequence( "180L_pistol" );
			else
				iSequence = LookupActivity ( NewActivity );
		}
	break;

	case ACT_TURN_RIGHT:
		{
			if(bGruntRPG)//only this class of monster by now
				iSequence = LookupSequence( "180R_pistol" );
			else
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

//=========================================================
// Get Schedule!
//=========================================================
Schedule_t *CHGrunt :: GetSchedule( void )
{

	// clear old sentence
	m_iSentence = HGRUNT_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if ( pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE )
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType ( SCHED_GRUNT_REPEL_LAND );
		}
		else
		{
			// repel down a rope, 
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
				return GetScheduleOfType ( SCHED_GRUNT_REPEL_ATTACK );
			else
				return GetScheduleOfType ( SCHED_GRUNT_REPEL );
		}
	}

	// grunts place HIGH priority on running away from danger sounds.
	if ( HasConditions(bits_COND_HEAR_SOUND) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound)
		{
			if (pSound->m_iType & bits_SOUND_DANGER)
			{
				// dangerous sound nearby!
				
				//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
				// and the grunt should find cover from the blast
				// good place for "SHIT!" or some other colorful verbal indicator of dismay.
				// It's not safe to play a verbal order here "Scatter", etc cause 
				// this may only affect a single individual in a squad. 
				
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz( ENT(pev), "HG_GREN", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
			}
			/*
			if (!HasConditions( bits_COND_SEE_ENEMY ) && ( pSound->m_iType & (bits_SOUND_PLAYER | bits_SOUND_COMBAT) ))
			{
				MakeIdealYaw( pSound->m_vecOrigin );
			}
			*/
		}
	}
	switch	( m_MonsterState )
	{
		
			//assassin COD
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		{
			/*
			if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else if ( FRouteClear() )
			{
				// no valid route!
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}
			else
			{
				// valid route. Get moving
				return GetScheduleOfType( SCHED_IDLE_WALK );
			}
			break;
			*/
			//ASSASIN CODE
			if ( HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
				if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
				{
					return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
				}
			}
			//ASSASIN CODE
		}
		break;
			//assassin COD
			//assassin COD

	case MONSTERSTATE_COMBAT:
		{
//found enemy and deploy weapon					
			if (!m_fGunDrawn )//test
			{
				if(bGruntRPG)//only this class of monster by now
				{
					if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))//DEAGLE
					{
						return GetScheduleOfType( SCHED_ARM_WEAPON );
					}
				}
			}
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

// new enemy
			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				if ( InSquad() )
				{
					MySquadLeader()->m_fEnemyEluded = FALSE;

					if ( !IsLeader() )
					{
						return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
					}
					else 
					{
						//!!!KELLY - the leader of a squad of grunts has just seen the player or a 
						// monster and has made it the squad's enemy. You
						// can check pev->flags for FL_CLIENT to determine whether this is the player
						// or a monster. He's going to immediately start
						// firing, though. If you'd like, we can make an alternate "first sight" 
						// schedule where the leader plays a handsign anim
						// that gives us enough time to hear a short sentence or spoken command
						// before he starts pluggin away.
						if (FOkToSpeak())// && RANDOM_LONG(0,1))
						{
							if ((m_hEnemy != NULL) && m_hEnemy->IsPlayer())
								// player
								SENTENCEG_PlayRndSz( ENT(pev), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
							else if ((m_hEnemy != NULL) &&
									(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) && 
									(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) && 
									(m_hEnemy->Classify() != CLASS_MACHINE))
								// monster
								SENTENCEG_PlayRndSz( ENT(pev), "HG_MONSTER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);

							JustSpoke();
						}
						
						if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
						{
							return GetScheduleOfType ( SCHED_GRUNT_SUPPRESS );
						}
						else
						{
							return GetScheduleOfType ( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
						}
					}
				}
			}
// no ammo
			else if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_GRUNT_COVER_AND_RELOAD );
			}
			
// damaged just a little
			else if ( HasConditions( bits_COND_LIGHT_DAMAGE ) )
			{
				// if hurt:
				// 90% chance of taking cover
				// 10% chance of flinch.
				int iPercent = RANDOM_LONG(0,99);

				if ( iPercent <= 90 && m_hEnemy != NULL )
				{
					// only try to take cover if we actually have an enemy!

					//!!!KELLY - this grunt was hit and is going to run to cover.
					if (FOkToSpeak()) // && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "HG_COVER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						m_iSentence = HGRUNT_SENT_COVER;
						//JustSpoke();
					}
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				else
				{
					return GetScheduleOfType( SCHED_SMALL_FLINCH );
				}
			}
// can kick
			else if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}
// can grenade launch

			else if ( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER) && HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
			{
				// shoot a grenade if you can
				return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
			}
// can shoot
			else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				if ( InSquad() )
				{
					// if the enemy has eluded the squad and a squad member has just located the enemy
					// and the enemy does not see the squad member, issue a call to the squad to waste a 
					// little time and give the player a chance to turn.
					if ( MySquadLeader()->m_fEnemyEluded && !HasConditions ( bits_COND_ENEMY_FACING_ME ) )
					{
						MySquadLeader()->m_fEnemyEluded = FALSE;
						return GetScheduleOfType ( SCHED_GRUNT_FOUND_ENEMY );
					}
				}

				if ( OccupySlot ( bits_SLOTS_HGRUNT_ENGAGE ) )
				{
					// try to take an available ENGAGE slot
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					// throw a grenade if can and no engage slots are available
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else
				{
					// hide!
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
			}
// can't see enemy
			else if ( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
			{
				if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz( ENT(pev), "HG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else if ( OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
				{
					//!!!KELLY - grunt cannot see the enemy and has just decided to 
					// charge the enemy's position. 
					if (FOkToSpeak())// && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "HG_CHARGE", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);				
						/*
						CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

						if (pPlayer->b_PlayerIsNearOf == DOOR)
							ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Soldado Marine(RADIO): Esta cerca de la Puerta!" );
						else if (pPlayer->b_PlayerIsNearOf == LADDER)
							ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Soldado Marine(RADIO): Esta cerca de la Escalera!" );
						else if (pPlayer->b_PlayerIsNearOf == BREKABLE)	
							ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Soldado Marine(RADIO): Esta cerca de la Caja!" );
						else if (pPlayer->b_PlayerIsNearOf == TRAIN)	
							ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Soldado Marine(RADIO): Esta cerca del Tren!" );
						else if (pPlayer->b_PlayerIsNearOf == PLAT)	
							ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Soldado Marine(RADIO): Esta cerca del Ascensor!" );
					*/
						m_iSentence = HGRUNT_SENT_CHARGE;
						//JustSpoke();
					}

					return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
				}
				else
				{
					//!!!KELLY - grunt is going to stay put for a couple seconds to see if
					// the enemy wanders back out into the open, or approaches the
					// grunt's covered position. Good place for a taunt, I guess?
					if (FOkToSpeak() && RANDOM_LONG(0,1))
					{/*
						CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
						ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Soldado Marine(RADIO): Eres Picadillo!" );
*/
						SENTENCEG_PlayRndSz( ENT(pev), "HG_TAUNT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);		
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_STANDOFF );
				}
			}
			
			if ( HasConditions( bits_COND_SEE_ENEMY ) && !HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				return GetScheduleOfType ( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
			}
		}
	}
	
	// no special cases here, call the base class
	return CSquadMonster :: GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CHGrunt :: GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{
	case SCHED_ARM_WEAPON://test
		if ( m_hEnemy != NULL )
		{
			// face enemy, then draw.
			return slGruntEnemyDraw;
		}

	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if ( InSquad() )
			{
				//sys test
				if (/* g_iSkillLevel == SKILL_HARD && */HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					if (FOkToSpeak())
					{/*
						CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
						ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Soldado Marine(RADIO): Fuego en el hoyo!" );
*/
						SENTENCEG_PlayRndSz( ENT(pev), "HG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return slGruntTossGrenadeCover;
				}
				else
				{
					return &slGruntTakeCover[ 0 ];
				}
			}
			else
			{
				if ( RANDOM_LONG(0,1) )
				{
					return &slGruntTakeCover[ 0 ];
				}
				else
				{
					return &slGruntGrenadeCover[ 0 ];
				}
			}
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slGruntTakeCoverFromBestSound[ 0 ];
		}
	case SCHED_GRUNT_TAKECOVER_FAILED:
		{
			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}

			return GetScheduleOfType ( SCHED_FAIL );
		}
		break;
	case SCHED_GRUNT_ELOF_FAIL:
		{
			// human grunt is unable to move to a position that allows him to attack the enemy.
			return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
		}
		break;
	case SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE:
		{
			return &slGruntEstablishLineOfFire[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK1:
		{
	//		if (RANDOM_LONG(0,5) == 0)//09
	//			m_fStanding = RANDOM_LONG(0,1);

			switch (RANDOM_LONG(0,2))
			{
				case 0: m_fStanding = TRUE;  break;
				case 1: m_fStanding = TRUE;  break;
				case 2: m_fStanding = FALSE;  break;
			}	
			
			//cambiado para evitar mandar el ACT_CROUCH que usa el rangeattack1A
			//de todas maneras se siguen utilizando animaciones de parado o sentado al azar
			//esto es para que si esta parado, no se agache y luego se pare nuevamente.
		//	if (m_fStanding)
				return &slGruntRangeAttack1B[ 0 ];
		//	else
		//		return &slGruntRangeAttack1a[ 0 ];
		}
	case SCHED_RANGE_ATTACK2:
		{
			return &slGruntRangeAttack2[ 0 ];
		}
	case SCHED_COMBAT_FACE:
		{
			return &slGruntCombatFace[ 0 ];
		}
	case SCHED_GRUNT_WAIT_FACE_ENEMY:
		{
			return &slGruntWaitInCover[ 0 ];
		}
	case SCHED_GRUNT_SWEEP:
		{
			return &slGruntSweep[ 0 ];
		}
	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slGruntHideReload[ 0 ];
		}
	case SCHED_GRUNT_FOUND_ENEMY:
		{
			return &slGruntFoundEnemy[ 0 ];
		}
	case SCHED_VICTORY_DANCE:
		{
			if ( InSquad() )
			{
				if ( !IsLeader() )
				{
					return &slGruntFail[ 0 ];
				}
			}

			return &slGruntVictoryDance[ 0 ];
		}
	case SCHED_GRUNT_SUPPRESS:
		{
			if ( m_hEnemy->IsPlayer() && m_fFirstEncounter )
			{
				m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slGruntSignalSuppress[ 0 ];
			}
			else
			{
				return &slGruntSuppress[ 0 ];
			}
		}
	case SCHED_FAIL:
		{
			if ( m_hEnemy != NULL )
			{
				// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
				return &slGruntCombatFail[ 0 ];
			}

			return &slGruntFail[ 0 ];
		}
	case SCHED_GRUNT_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepel[ 0 ];
		}
	case SCHED_GRUNT_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepelAttack[ 0 ];
		}
	case SCHED_GRUNT_REPEL_LAND:
		{
			return &slGruntRepelLand[ 0 ];
		}
	default:
		{
			return CSquadMonster :: GetScheduleOfType ( Type );
		}
	}
}


//=========================================================
// CHGruntRepel - when triggered, spawns a monster_human_grunt
// repelling down a line.
//=========================================================

class CHGruntRepel : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT RepelUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int m_iSpriteTexture;	// Don't save, precache
};

LINK_ENTITY_TO_CLASS( monster_grunt_repel, CHGruntRepel );

void CHGruntRepel::Spawn( void )
{
	Precache( );
	pev->solid = SOLID_NOT;

	SetUse( RepelUse );
}

void CHGruntRepel::Precache( void )
{
	UTIL_PrecacheOther( "monster_human_grunt" );
	m_iSpriteTexture = PRECACHE_MODEL( "sprites/rope.spr" );
}

void CHGruntRepel::RepelUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, -4096.0), dont_ignore_monsters, ENT(pev), &tr);
	/*
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP) 
		return NULL;
	*/

	CBaseEntity *pEntity = Create( "monster_human_grunt", pev->origin, pev->angles );
	CBaseMonster *pGrunt = pEntity->MyMonsterPointer( );
	pGrunt->pev->movetype = MOVETYPE_FLY;
	pGrunt->pev->velocity = Vector( 0, 0, RANDOM_FLOAT( -196, -128 ) );
	pGrunt->SetActivity( ACT_GLIDE );
	// UNDONE: position?
	pGrunt->m_vecLastPosition = tr.vecEndPos;

	CBeam *pBeam = CBeam::BeamCreate( "sprites/rope.spr", 10 );
	pBeam->PointEntInit( pev->origin + Vector(0,0,112), pGrunt->entindex() );
	pBeam->SetFlags( BEAM_FSOLID );
	pBeam->SetColor( 255, 255, 255 );
	pBeam->SetThink( SUB_Remove );
	pBeam->pev->nextthink = gpGlobals->time + -4096.0 * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

	UTIL_Remove( this );
}



//=========================================================
// DEAD HGRUNT PROP
//=========================================================
class CDeadHGrunt : public CBaseMonster
{
public:
	void Spawn( void );
	int	Classify ( void ) { return	CLASS_HUMAN_MILITARY; }

	void KeyValue( KeyValueData *pkvd );

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[3];
};

char *CDeadHGrunt::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

void CDeadHGrunt::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_hgrunt_dead, CDeadHGrunt );

//=========================================================
// ********** DeadHGrunt SPAWN **********
//=========================================================
void CDeadHGrunt :: Spawn( void )
{
	PRECACHE_MODEL("models/hgrunt.mdl");
	SET_MODEL(ENT(pev), "models/hgrunt.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );

	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead hgrunt with bad pose\n" );
	}

	// Corpses have less health
	pev->health			= 8;

	// map old bodies onto new bodies
	switch( pev->body )
	{
	case 0: // Grunt with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( HEAD_GROUP, HEAD_GRUNT );
		SetBodygroup( GUN_GROUP, GUN_MP5 );
		break;
	case 1: // Commander with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( HEAD_GROUP, HEAD_COMMANDER );
		SetBodygroup( GUN_GROUP, GUN_MP5 );
		break;
	case 2: // Grunt no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( HEAD_GROUP, HEAD_GRUNT );
		SetBodygroup( GUN_GROUP, GUN_NONE );
		break;
	case 3: // Commander no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( HEAD_GROUP, HEAD_COMMANDER );
		SetBodygroup( GUN_GROUP, GUN_NONE );
		break;
	}

	MonsterInitDead();
}

void CHGrunt:: LegsGibs( void )
{
	Vector vecSpot;// shard origin
	Vector vecVelocity;// shard velocity
//	CBaseEntity *pEntity = NULL;
	//char cFlag = 0;
	//int pitch;
	//float fvol;	

// Calcular el punto de 1 gib y hacer decals de sangre en el suelo
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( "models/gib_hgrunt.mdl" );// Es necesario?? KELLY: Yep
	pGib->m_bloodColor = BLOOD_COLOR_RED;
	pGib->pev->origin = pev->origin;
	pGib->pev->velocity = UTIL_RandomBloodVector() * RANDOM_FLOAT( 300, 500 );
// End	
}

void CHGrunt :: ArmsGibs( void )
{	
	Vector vecSpot;// shard origin
	Vector vecVelocity;// shard velocity
	CBaseEntity *pEntity = NULL;

// Calculate the point of the gib and splat blood at the surface	
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( "models/gib_hgrunt_a.mdl" ); // Es necesario?? KELLY: Yep
	pGib->m_bloodColor = BLOOD_COLOR_RED;
	pGib->pev->origin = pev->origin;
	pGib->pev->velocity = UTIL_RandomBloodVector() * RANDOM_FLOAT( 300, 500 );
}



///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//////////////// CUSTOM GRUNTS ///////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class CHgruntCustom2 : public CHGrunt
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_hgrunt_opfor, CHgruntCustom2 );

void CHgruntCustom2 :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	SET_MODEL( ENT(pev),"physics/models/hgrunt_opfor.mdl");
	else
	SET_MODEL( ENT(pev),"models/hgrunt_opfor.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP; //MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //creo q algo de 100 o 70...
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;
	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP /*| bits_CAP_STRAFE*/ ; //jejeje
	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.
	bGruntRPG			= FALSE;

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if (pev->weapons == 0) //si no tiene armas (monstermaker se escuchó?)
	{
		switch (RANDOM_LONG(0,5)) //comenzar con ak o escopeta
		{
			case 0: pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE; break;		
			case 1: pev->weapons = HGRUNT_SHOTGUN | HGRUNT_HANDGRENADE; break;
			case 2: pev->weapons = HGRUNT_9MMAR; break;
			case 3: pev->weapons = HGRUNT_SHOTGUN; break;
			case 4: pev->weapons = HGRUNT_M249 | HGRUNT_HANDGRENADE; break;//test
			case 5: pev->weapons = HGRUNT_M249; break;
		}
	}
	if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
	{
		m_cClipSize		= 8;
	}
	else if (FBitSet( pev->weapons, HGRUNT_M249 ))//test
	{
		m_cClipSize		= 70;
	}
	else
	{
		m_cClipSize		= GRUNT_CLIP_SIZE;
	}
	m_cAmmoLoaded		= m_cClipSize;

	//set random skins
	if (RANDOM_LONG( 0, 99 ) < 80)
	pev->skin = 0;	// light skin
	else
	pev->skin = 1;	// dark skin
	 
	//oh dear! commander is not a black guy!
	if ( GetBodygroup( HEAD_GROUP ) == HEAD_COMMANDER )
	pev->skin = 0;	// always light skin

	//right, the black guy must have dark arms
	if ( GetBodygroup( HEAD_GROUP ) == HEAD_M203 )
	pev->skin = 1;	// always dark skin

	//random heads
	switch (RANDOM_LONG(0,6))
	{
		case 0: SetBodygroup( HEAD_GROUP, HEAD_GRUNT ); break;
		case 1:	SetBodygroup( HEAD_GROUP, HEAD_COMMANDER ); break;
		case 2: SetBodygroup( HEAD_GROUP, HEAD_SHOTGUN ); break;
		case 3: SetBodygroup( HEAD_GROUP, HEAD_M203 ); break;	
		case 4: SetBodygroup( HEAD_GROUP, HEAD_GLASSES ); break;
		case 5:	SetBodygroup( HEAD_GROUP, HEAD_MASK ); break;
		case 6: SetBodygroup( HEAD_GROUP, HEAD_HELMET ); break;
	}

	if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
	{
		SetBodygroup( GUN_GROUP_OPFOR, GUN_SHOTGUN_OPFOR );
	}
	else if (FBitSet( pev->weapons, HGRUNT_M249 ))
	{
		SetBodygroup( TORSO_GROUP, BODY_SAW );
		SetBodygroup( GUN_GROUP_OPFOR, GUN_M249_OPFOR );
	}
	else
	{
		SetBodygroup( GUN_GROUP_OPFOR, GUN_MP5_OPFOR );
	}

	CTalkMonster::g_talkWaitTime = 0;
	MonsterInit();
}

void CHgruntCustom2 :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	PRECACHE_MODEL("physics/models/hgrunt_opfor.mdl");
	else
	PRECACHE_MODEL("models/hgrunt_opfor.mdl");

	PRECACHE_SOUND ("hgrunt/saw1.wav");// test

	PRECACHE_SOUND( "hgrunt/M161.wav" );

	PRECACHE_SOUND( "hgrunt/gr_die1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain5.wav" );
	PRECACHE_SOUND( "hgrunt/gr_reload1.wav" );
	PRECACHE_SOUND( "weapons/glauncher.wav" );
	PRECACHE_SOUND( "hgrunt/usas_fire-1.wav" );
	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	// get voice pitch
	if (RANDOM_LONG(0,1))
		m_voicePitch = 90 + RANDOM_LONG(0,7);
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");
	PRECACHE_MODEL( "models/gib_hgrunt.mdl" );
	PRECACHE_MODEL( "models/gib_hgrunt_a.mdl" );
	m_idGib_hgrunt = PRECACHE_MODEL( "models/gib_hgrunt.mdl" );
	m_idGib_hgrunt_a = PRECACHE_MODEL( "models/gib_hgrunt_a.mdl" );
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//////////////// CUSTOM GRUNTS ///////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class CHgruntRPG : public CHGrunt
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_hgrunt_rpg, CHgruntRPG );
LINK_ENTITY_TO_CLASS( monster_human_grunt_rpg, CHgruntRPG );

void CHgruntRPG :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	SET_MODEL( ENT(pev),"physics/models/hgrunt_rpg.mdl");
	else
	SET_MODEL( ENT(pev),"physics/models/hgrunt_rpg.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //creo q algo de 100 o 70...
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP /*| bits_CAP_STRAFE*/ ; //jejeje

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.
	bGruntRPG			= TRUE;//using RPG

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))//DEAGLE
	{
		m_cClipSize		= 7;
		m_iNumClips		= 999;
		m_fGunDrawn = TRUE;

		SetBodygroup( GUN_GROUP, GUN_DEAGLE );
	}
	else//LAW
	{
		m_cClipSize		= 1;
		m_iNumClips		= 1;//single shot
		m_fGunDrawn = FALSE;

		SetBodygroup( GUN_GROUP, GUN_LAW );
	}

	//random heads
	switch (RANDOM_LONG(0,3))
	{
		case 0: SetBodygroup( HEAD_GROUP, HEAD_GRUNT ); break;
		case 1:	SetBodygroup( HEAD_GROUP, HEAD_COMMANDER ); break;
		case 2: SetBodygroup( HEAD_GROUP, HEAD_SHOTGUN ); break;
		case 3: SetBodygroup( HEAD_GROUP, HEAD_M203 ); break;	
	}

	//set random skins
	if (RANDOM_LONG( 0, 99 ) < 80)
	pev->skin = 0;	// light skin
	else
	pev->skin = 1;	// dark skin
	 
	//oh dear! commander is not a black guy!
	if ( GetBodygroup( HEAD_GROUP ) == HEAD_COMMANDER )
	pev->skin = 0;	// always light skin

	//right, the black guy must have dark arms
	if ( GetBodygroup( HEAD_GROUP ) == HEAD_M203 )
	pev->skin = 1;	// always dark skin

	/*
	if (FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ))//this is really really necessary?
	{
		SetBodygroup( HEAD_GROUP, HEAD_M203 );
	}
	*/

	m_cAmmoLoaded		= m_cClipSize;
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

void CHgruntRPG :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	PRECACHE_MODEL("physics/models/hgrunt_rpg.mdl");
	else
	PRECACHE_MODEL("physics/models/hgrunt_rpg.mdl");
	
	UTIL_PrecacheOther( "hvr_rocket" );

	PRECACHE_SOUND( "hgrunt/gr_die1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die3.wav" );

	PRECACHE_SOUND( "hgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain5.wav" );

	PRECACHE_SOUND( "hgrunt/gr_reload1.wav" );

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	// get voice pitch
	if (RANDOM_LONG(0,1))
		m_voicePitch = 90 + RANDOM_LONG(0,7);
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");

	PRECACHE_MODEL( "models/gib_hgrunt.mdl" );
	PRECACHE_MODEL( "models/gib_hgrunt_a.mdl" );
	m_idGib_hgrunt = PRECACHE_MODEL( "models/gib_hgrunt.mdl" );
	m_idGib_hgrunt_a = PRECACHE_MODEL( "models/gib_hgrunt_a.mdl" );

	m_iPistolShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");// brass shell
}

















































class CRoboGrunt : public CHGrunt
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_robot, CRoboGrunt );

//=========================================================
// Spawn
//=========================================================
void CRoboGrunt :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else
	SET_MODEL( ENT(pev),"models/robot.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;

	m_bloodColor		= DONT_BLEED;

	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //creo q algo de 100 o 70...
	
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP /*| bits_CAP_STRAFE*/ ; //jejeje

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.
	bGruntRPG			= FALSE;
	b_mIsRobot			= TRUE;

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if (pev->weapons == 0)
	{
		switch (RANDOM_LONG(0,3))
		{
			case 0: pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE; break;		
			case 1: pev->weapons = HGRUNT_SHOTGUN | HGRUNT_HANDGRENADE; break;
			case 2: pev->weapons = HGRUNT_9MMAR; break;
			case 3: pev->weapons = HGRUNT_SHOTGUN; break;
		}
	}

	if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
	{
		m_cClipSize		= 8;
			
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
	}
	else//M16
	{
		m_cClipSize		= GRUNT_CLIP_SIZE;

		SetBodygroup( GUN_GROUP, GUN_MP5 );
	}

	m_cAmmoLoaded		= m_cClipSize;
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CRoboGrunt :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else
	PRECACHE_MODEL("models/robot.mdl");

	PRECACHE_SOUND( "hgrunt/m161.wav" );

	PRECACHE_MODEL("models/robot_head.mdl");
	PRECACHE_MODEL("models/robot_arm.mdl");
	PRECACHE_MODEL("models/robot_leg.mdl");

	PRECACHE_SOUND( "robot/robot_die1.wav" );
	PRECACHE_SOUND( "robot/robot_die2.wav" );
	PRECACHE_SOUND( "robot/robot_die3.wav" );
/*
	PRECACHE_SOUND( "hgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain5.wav" );
*/
	PRECACHE_SOUND( "hgrunt/gr_reload1.wav" );

	PRECACHE_SOUND( "weapons/glauncher.wav" );

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	if (RANDOM_LONG(0,1))
		m_voicePitch = 20 + RANDOM_LONG(0,7);
	else
		m_voicePitch = 40;

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");
		
	PRECACHE_MODEL( "models/computergibs.mdl" );
}	