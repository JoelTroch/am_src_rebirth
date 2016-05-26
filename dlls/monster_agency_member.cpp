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

//====================
// Monster Terrorist
//====================

// This is a new ally that can fire 8 different weapons, drop secondary weapons, drop SG, HG and FB
// grenades, detect the enemy's position, look for enemy's sounds, try to surrender bad enemies and
// so on...
//
// I use temporaly a beta model from CS Source. I will change it, so don't mail me askin'me "hey, why do you
// use a CS S model?" (we are still looking for good modellers)

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"talkmonster.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"weapons.h"
#include	"soundent.h"

int g_fAmQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

//=========================================================
// monster-specific DEFINE's
//=========================================================
//#define GRUNT_ATTN					ATTN_NORM	// attenutation of grunt sentences

//UNDONE: make radius more realistic (for each voice type)
#define GRUNT_ATTN					ATTN_STATIC	// //medium radius

#define SENT_SMALLRADIUS			ATTN_IDLE
#define SENT_MEDIUMRADIUS			ATTN_STATIC
#define SENT_LARGERADIUS			ATTN_NORM

#define LIMP_HEALTH					20
#define	AMEMBER_SENTENCE_VOLUME		(float)0.35 // volume of grunt sentences

#define	AMEMBER_MINIMUN_LIGHT		40// el valor de luz minimo permitido (a partir de ese valor no puede ver)

#define HEAD_GROUP					1

#define GUN_GROUP					2

#define GUN_MP5						0
#define GUN_SHOTGUN					1 
#define GUN_LAW						2
#define GUN_AWP						3 
#define GUN_ASSAULT					4
#define GUN_PISTOL					5 
#define GUN_DESERT					6			
#define GUN_M249					7
#define BLANK2						8//smg before
#define GUN_NONE					9

//weapons
#define MP5						1
#define SHOTGUN					2
#define LAW						3
#define AWP						4
#define ASSAULT					5 
#define PISTOL					6
#define DESERT					7
#define M249					8			
//#define SMG						9


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		AMEMBER_AE_RELOAD		( 2 )
#define		AMEMBER_AE_KICK			( 3 )
#define		AMEMBER_AE_BURST1		( 4 )
#define		AMEMBER_AE_BURST2		( 5 ) 
#define		AMEMBER_AE_BURST3		( 6 ) 
#define		AMEMBER_AE_GREN_TOSS	( 7 )
#define		AMEMBER_AE_GREN_LAUNCH	( 8 )
#define		AMEMBER_AE_GREN_DROP	( 9 )
#define		AMEMBER_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		AMEMBER_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define		AMEMBER_AE_DIE_SHOOT	( 22 )
#define		AMEMBER_AE_DRAW			( 25 )

#define	ACTIVITY_NOT_AVAILABLE	-1

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_AMEMBER_COVER_AND_RELOAD,
};

class CMonsterAMember : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );

	void FireWeapon( void );

	void FireWeaponWhenDie( void );//the monster is diying and shooting his weapon

	void AlertSound( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	int	m_cClipSize;
	void GibMonster( void );

	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	virtual int	ObjectCaps( void ) { return CTalkMonster :: ObjectCaps() | FCAP_IMPULSE_USE; }
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	
	void DeclineFollowing( void );
	void SetActivity ( Activity NewActivity );
	void CheckAmmo ( void );
//	void IdleSound ( void );

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	MONSTERSTATE GetIdealState ( void );

	void DeathSound( void );
	void PainSound( void );
	
	void TalkInit( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void Killed( entvars_t *pevAttacker, int iGib );
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL	m_fGunDrawn;
	float	m_painTime;
	float	m_checkAttackTime;
	BOOL	m_lastAttackCheck;

	int		m_iBrassShell;
	int		m_iPistolShell;
	int		m_iShotgunShell;

	
	float m_flNextGrenadeCheck;

	Vector	m_vecTossVelocity;

	BOOL	m_fThrowGrenade;
	BOOL	m_fStanding;

	//new definitions

	Vector GetGunPosition( void );
	void RunAI( void );

	BOOL	m_fHasBeenHit;//the monster has been hit, slow it down
	BOOL	m_fStealthed;//the monster spawn as stealth
	BOOL	m_fHasDeagle;//check if the monster has desert e or not

	int		m_flHitCounter;

	int		m_iNumClips;//amount of clips
	int		m_iNumGrenades;//amount of grenades
	int		m_iFrustration;//what kind and amount of damage the player inflicted on me

	float	m_flLastLightLevel;

	int		m_flNextSuspCheckTime;

	BOOL CheckRangeAttack2 ( float flDot, float flDist );

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_agency_member, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_ct_m4a1, CMonsterAMember );
//fix
LINK_ENTITY_TO_CLASS( monster_ally_swat_mp5, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_ally_swat_shotgun, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_all_swat_law, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_ally_swat_sniper, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_ally_swat_assault, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_ally_swat_pistol, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_ally_swat_m249, CMonsterAMember );
LINK_ENTITY_TO_CLASS( monster_ally_swat_smg, CMonsterAMember );

TYPEDESCRIPTION	CMonsterAMember::m_SaveData[] = 
{
	DEFINE_FIELD( CMonsterAMember, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CMonsterAMember, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CMonsterAMember, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterAMember, m_cClipSize, FIELD_INTEGER ),// FIX!!
	DEFINE_FIELD( CMonsterAMember, m_vecTossVelocity, FIELD_VECTOR ),// FIX!!
	DEFINE_FIELD( CMonsterAMember, m_fThrowGrenade, FIELD_BOOLEAN ),// FIX!!
	
	DEFINE_FIELD( CMonsterAMember, m_fHasBeenHit, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterAMember, m_fStealthed, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterAMember, m_flHitCounter, FIELD_INTEGER ),
	DEFINE_FIELD( CMonsterAMember, m_iNumGrenades, FIELD_INTEGER ),
	DEFINE_FIELD( CMonsterAMember, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterAMember, m_iNumClips, FIELD_INTEGER ),
	DEFINE_FIELD( CMonsterAMember, m_fHasDeagle, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterAMember, m_fStanding, FIELD_BOOLEAN )
};

IMPLEMENT_SAVERESTORE( CMonsterAMember, CTalkMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlAmFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slAmFollow[] =
{
	{
		tlAmFollow,
		ARRAYSIZE ( tlAmFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlAmiendHideReload[] =
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

Schedule_t slAmiendHideReload[] = 
{
	{
		tlAmiendHideReload,
		ARRAYSIZE ( tlAmiendHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntHideReload"
	}
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlAmiendRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
};

Schedule_t	slAmiendRangeAttack2[] =
{
	{ 
		tlAmiendRangeAttack2,
		ARRAYSIZE ( tlAmiendRangeAttack2 ), 
		0,
		0,
		"RangeAttack2"
	},
};

//=========================================================
// BarneyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlAmrneyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },//ACT_SIGNAL1 isn't draw!!
};

Schedule_t slAmrneyEnemyDraw[] = 
{
	{
		tlAmrneyEnemyDraw,
		ARRAYSIZE ( tlAmrneyEnemyDraw ),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t	tlAmFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slAmFaceTarget[] =
{
	{
		tlAmFaceTarget,
		ARRAYSIZE ( tlAmFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};


Task_t	tlIdleAmStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleAmStand[] =
{
	{ 
		tlIdleAmStand,
		ARRAYSIZE ( tlIdleAmStand ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
//		bits_SOUND_COWARD		|//new
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

DEFINE_CUSTOM_SCHEDULES( CMonsterAMember )
{
	slAmFollow,
	slAmiendHideReload,
	slAmrneyEnemyDraw,
	slAmFaceTarget,
	slIdleAmStand,
	slAmiendRangeAttack2,
};


IMPLEMENT_CUSTOM_SCHEDULES( CMonsterAMember, CTalkMonster );

void CMonsterAMember :: StartTask( Task_t *pTask )
{
	CTalkMonster::StartTask( pTask );	
}

void CMonsterAMember :: RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_RANGE_ATTACK1:
		if (m_hEnemy != NULL && (m_hEnemy->IsPlayer()))
		{
			pev->framerate = 1.5;
		}
		CTalkMonster::RunTask( pTask );
		break;
	default:
		CTalkMonster::RunTask( pTask );
		break;
	}
}




//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CMonsterAMember :: ISoundMask ( void) 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_CARCASS	|
//			bits_SOUND_COWARD	|//new
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CMonsterAMember :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_PLAYER_ALLY;
}

//=========================================================
// ALertSound - barney says "Freeze!"
//=========================================================
void CMonsterAMember :: AlertSound( void )
{
	if ( m_hEnemy != NULL )
	{
		if ( FOkToSpeak() )
		{
			if (m_iszSpeakAs)
			{
				char szBuf[32];
				strcpy(szBuf,STRING(m_iszSpeakAs));
				strcat(szBuf,"_ATTACK");
				PlaySentence( szBuf, RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
			}
			else
			{
				PlaySentence( "FG_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
			}
		}
	}

}
//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMonsterAMember :: SetYawSpeed ( void )
{
	//copied from hgrunt
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


//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CMonsterAMember :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( flDist <= 1024 && flDot >= 0.5 )
	{
		if ( gpGlobals->time > m_checkAttackTime )
		{
			TraceResult tr;
			
			Vector shootOrigin = pev->origin + Vector( 0, 0, 55 );
			CBaseEntity *pEnemy = m_hEnemy;
			Vector shootTarget = ( (pEnemy->BodyTarget( shootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP );
			UTIL_TraceLine( shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr );
			m_checkAttackTime = gpGlobals->time + 1;

			if ( tr.flFraction == 1.0 || (tr.pHit != NULL && CBaseEntity::Instance(tr.pHit) == pEnemy) )
				m_lastAttackCheck = TRUE;
			else
				m_lastAttackCheck = FALSE;

			m_checkAttackTime = gpGlobals->time + 1.5;
		}

		CBaseEntity *pEntityToFrustrate = NULL;
		
		Vector VecSrc;
		VecSrc = pev->origin;
		
		if ( m_flNextSuspCheckTime < gpGlobals->time )
		{	
			while ((pEntityToFrustrate = UTIL_FindEntityInSphere( pEntityToFrustrate, VecSrc, 512 )) != NULL)//512
			{	
				if ( FClassnameIs( pEntityToFrustrate->pev, "monster_terrorist" ) || ( FClassnameIs( pEntityToFrustrate->pev,  "monster_human_ak" ) ) )
				{
					switch (RANDOM_LONG(0,4)) 
					{
						case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/1.wav", 1.0, ATTN_NORM); break;
						case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/2.wav", 1.0, ATTN_NORM); break;
						case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/3.wav", 1.0, ATTN_NORM); break;
						case 3:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/4.wav", 1.0, ATTN_NORM); break;
						case 4:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/5.wav", 1.0, ATTN_NORM); break;
					}

					pEntityToFrustrate->m_iFrustration += 30;
					CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 500, 0.3 );//so, it can be detected
					m_flNextSuspCheckTime = gpGlobals->time + 5;
				}
			}
		}

		return m_lastAttackCheck;
	}
	return FALSE;

	//old code
	/*
	if (pev->frags == LAW)
	{
		if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 2048 && flDot >= 0.5 )
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
		if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 2048 && flDot >= 0.5 )//can see enemy
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
		else//can't see it, but don't attack it anyway. say "stop men"!
		{
			CBaseEntity *pEntityToFrustrate = NULL;
		
			Vector VecSrc;
			VecSrc = pev->origin;
			
			if ( m_flNextSuspCheckTime < gpGlobals->time )
			{	
				while ((pEntityToFrustrate = UTIL_FindEntityInSphere( pEntityToFrustrate, VecSrc, 512 )) != NULL)//512
				{	
					if (FClassnameIs(pEntityToFrustrate->pev, "monster_terrorist"))
					{
						switch (RANDOM_LONG(0,4)) 
						{
							case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/1.wav", 1.0, ATTN_NORM); break;
							case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/2.wav", 1.0, ATTN_NORM); break;
							case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/3.wav", 1.0, ATTN_NORM); break;
							case 3:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/4.wav", 1.0, ATTN_NORM); break;
							case 4:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/5.wav", 1.0, ATTN_NORM); break;
						}

						pEntityToFrustrate->m_iFrustration += 30;
						CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 500, 0.3 );//so, it can be detected
						m_flNextSuspCheckTime = gpGlobals->time + 2;

					}
				}
			}
		}
	}

	return FALSE;
	*/
}

	
//=========================================================
// GetGunPosition	return the end of the barrel
//=========================================================

Vector CMonsterAMember :: GetGunPosition( )
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
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CMonsterAMember :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	switch( pEvent->event )
	{
		case AMEMBER_AE_DRAW:
			{
				if(m_fHasDeagle)
				SetBodygroup( GUN_GROUP, GUN_DESERT );
				else
				SetBodygroup( GUN_GROUP, GUN_PISTOL );
			
				EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/weapon_deploy.wav", 1, ATTN_NORM, 0, 100 );
				m_fGunDrawn = TRUE;
			}
		break;

//		case AMEMBER_AE_HOLSTER:
//			m_fGunDrawn = FALSE;
//		break;

		case AMEMBER_AE_DROP_GUN:
			{//UNDONE: keep track of shoots fired, and use the remaining shoots as ammo.
				if ( GetBodygroup( GUN_GROUP ) != GUN_NONE )
				{
					//SP: si tiene el flag de "no tirar arma" cerrar con break y no ejecutar
					//mas codigo
					if (pev->spawnflags & SF_MONSTER_NO_WPN_DROP) break;

					Vector	vecGunPos;
					Vector	vecGunAngles;

					GetAttachment( 3, vecGunPos, vecGunAngles );

					// switch to body group with no gun.
					SetBodygroup( GUN_GROUP, GUN_NONE );

					CBaseEntity *pItem;
					// now spawn a gun.

					if (pev->frags == MP5)
					{
						pItem = DropItem( "weapon_mp5", vecGunPos, vecGunAngles );
						pItem = DropItem( "item_mp5_silencer", vecGunPos, vecGunAngles );
					}
					else if (pev->frags == SHOTGUN){
						pItem = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );}
					else if (pev->frags == LAW){
						pItem = DropItem( "weapon_rpg", vecGunPos, vecGunAngles );}
					else if (pev->frags == AWP){
						pItem = DropItem( "weapon_sniper", vecGunPos, vecGunAngles );}
					else if (pev->frags == ASSAULT){
						pItem = DropItem( "weapon_m4a1", vecGunPos, vecGunAngles );}
					else if (pev->frags == PISTOL){
						pItem = DropItem( "weapon_9mmhandgun", vecGunPos, vecGunAngles );}
					else if (pev->frags == DESERT){
						pItem = DropItem( "weapon_deagle", vecGunPos, vecGunAngles );}
					else if (pev->frags == M249){
						pItem = DropItem( "weapon_M249", vecGunPos, vecGunAngles );}
					else
					{
						//CANT DROP ITEM!!!
					}

					if(m_iNumClips <=0)//if no ammo
					{		
						if (pev->frags == PISTOL)
						{
							//the pistol has infinite ammo
						}
						else if (pev->frags == DESERT)
						{
							//the pistol has infinite ammo
						}
						else
						{
							//when the monster doens't have any ammo deploy his secondary weapon. it could
							//be a desert eagle or a pistol (glock in T cases or colt 1911 in ct cases)
		/*					if ( RANDOM_LONG( 0, 1 ) == 0 )
							m_fHasDeagle = TRUE;
							else
							m_fHasDeagle = FALSE;
*/
							m_fHasDeagle = TRUE;

							if(m_fHasDeagle)
							{
								pev->frags = DESERT;//now we have a desert eagle
								m_iNumClips = 999;//at this point the monster doens't have any clips, give it a few of them
							
								//edit, change the clip size and give ammo
								m_cClipSize = 7;
								m_cAmmoLoaded = m_cClipSize;//give it ammo! (dont reload weapon when he draw it)
							}
							else
							{
								pev->frags = PISTOL;//now we have a pistol
								m_iNumClips = 999;//at this point the monster doens't have any clips, give it a few of them
							
								//edit, change the clip size and give ammo
								m_cClipSize = 20;
								m_cAmmoLoaded = m_cClipSize;//give it ammo! (dont reload weapon when he draw it)
							}
						}
					}

					//if -> crash, remove the following
					/*
					if(pItem)
					{
						pItem->pev->avelocity = Vector ( RANDOM_FLOAT( -222, 222 ), RANDOM_FLOAT( -222, 222 ),RANDOM_FLOAT( -222, 222 ) );
						pItem->pev->angles.x = 90;
						pItem->pev->angles.y =  90;
					}
					*/
				}
			}
			break;

		case AMEMBER_AE_RELOAD:
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "human_ak/reload1.wav", 1, ATTN_NORM );		

			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
			break;

		case AMEMBER_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );

			//shit! its too dark here!
			if (m_flLastLightLevel <= AMEMBER_MINIMUN_LIGHT)
			CGrenade::FlashShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 30.5 );
			else
			{
				switch ( RANDOM_LONG( 0, 2 ) )
				{
					case 0:	CGrenade::ShootFlashbang( pev, GetGunPosition(), m_vecTossVelocity, 4 );	break;
					case 1:	CGrenade::ShootTimedCz( pev, GetGunPosition(), m_vecTossVelocity, 4 );	break;
					case 2:	CGrenade::ShootTimedSmoke( pev, GetGunPosition(), m_vecTossVelocity, 23.0 );break;
				}
			}
			
			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		break;

		case AMEMBER_AE_GREN_LAUNCH:
		{
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM);
			CGrenade::ShootContact( pev, GetGunPosition(), m_vecTossVelocity );
			m_fThrowGrenade = FALSE;
			if (g_iSkillLevel == SKILL_HARD)
				m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT( 2, 5 );// wait a random amount of time before shooting again
			else
				m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		break;

		case AMEMBER_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
					
			if (m_flLastLightLevel <= AMEMBER_MINIMUN_LIGHT)
			CGrenade::FlashShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 30 );
			else
			{
				switch ( RANDOM_LONG( 0, 2 ) )
				{
					case 0:	CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4 );	break;
					case 1:	CGrenade::ShootTimedCz( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4 );break;
					case 2:	CGrenade::ShootTimedSmoke( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 23 );break;
				}
			}
		}
		break;

		case AMEMBER_AE_BURST1:
			{
				if (RANDOM_LONG( 0, 99 ) < 80)
				{
					m_fStanding = 1;
				}
				else
				{
					m_fStanding = 0;
				}

				if(m_fStealthed)//be carefully
				m_fStanding = 0;

				FireWeapon();
				CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
			}
			break;

		case AMEMBER_AE_BURST2:
		case AMEMBER_AE_BURST3:
			FireWeapon();
			break;

		case AMEMBER_AE_DIE_SHOOT:
			FireWeaponWhenDie();
/*
		case AMEMBER_AE_KICK:
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

		case AMEMBER_AE_CAUGHT_ENEMY:
		{
			if ( FOkToSpeak() )
			{
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_ALERT", AMEMBER_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				 JustSpoke();
			}

		}
*/
		default:
			CTalkMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CMonsterAMember :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/urban.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_HackedGunPos = Vector ( 0, 0, 55 );

	m_fGunDrawn			= FALSE;
	m_fHasBeenHit		= FALSE;
	m_iNumGrenades		= 3;

	if ( FStringNull( pev->frags ))
	{
		switch (RANDOM_LONG(0,7))
		{
			case 0: pev->frags = MP5; break;
			case 1: pev->frags = SHOTGUN; break;
			case 2: pev->frags = LAW; break;
			case 3: pev->frags = AWP; break;
			case 4: pev->frags = ASSAULT; break;
			case 5: pev->frags = PISTOL; break;
			case 6: pev->frags = M249; break;
			case 7: pev->frags = DESERT; break;//just I put it here
		}
	}

	if (pev->frags == MP5)
	{
		SetBodygroup( GUN_GROUP, GUN_MP5 );
		m_cClipSize		= 30;
		m_iNumClips		= 3;//120 bullets

		if (RANDOM_LONG( 0, 99 ) < 80)
		m_fStealthed = FALSE;
		else
		m_fStealthed = TRUE;
	}
	else if (pev->frags == SHOTGUN)
	{
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		m_cClipSize		= 8;
		m_iNumClips		= 4;//32 bullets
	}
	else if (pev->frags == LAW)
	{
		SetBodygroup( GUN_GROUP, GUN_LAW );
		m_cClipSize		= 1;
		m_iNumClips		= 1;//single shot
	}
	else if (pev->frags == AWP)
	{
		SetBodygroup( GUN_GROUP, GUN_AWP );
		m_cClipSize		= 1;
		m_iNumClips		= 10;//10 shots
	}
	else if (pev->frags == ASSAULT)
	{
		SetBodygroup( GUN_GROUP, GUN_ASSAULT );
		m_cClipSize		= 30;
		m_iNumClips		= 3;//120 bullets
	}
	else if (pev->frags == PISTOL)
	{
		SetBodygroup( GUN_GROUP, GUN_NONE );
		m_cClipSize		= 20;
		m_iNumClips		= 999;//infinite? bullets
	}	
	else if (pev->frags == DESERT)
	{
		SetBodygroup( GUN_GROUP, GUN_NONE );
		m_cClipSize		= 7;
		m_iNumClips		= 999;//infinite? bullets
		m_fHasDeagle	= TRUE;
	}
	else if (pev->frags == M249)
	{
		SetBodygroup( GUN_GROUP, GUN_M249 );
		m_cClipSize		= 70;
		m_iNumClips		= 2;//140 bullets
	}

	m_cAmmoLoaded		= m_cClipSize;

	//model test
	pev->skin = 0;

	MonsterInit();
	SetUse(&CMonsterAMember :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMonsterAMember :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/urban.mdl");

	PRECACHE_SOUND( "human_ak/reload1.wav" );
	PRECACHE_SOUND( "weapons/reload_shotgun.wav" );

	PRECACHE_SOUND( "npc_voices/friend/radio/SuspectCooperate.wav" );

	PRECACHE_MODEL ("models/weapons/w_czgrenade.mdl");
	
	UTIL_PrecacheOther( "hvr_rocket" );

	m_iBrassShell = PRECACHE_MODEL ("models/shell_762.mdl");
	m_iPistolShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");

	TalkInit();
	CTalkMonster::Precache();
}	

// Init talk data
void CMonsterAMember :: TalkInit()
{
	CTalkMonster::TalkInit();

	if (!m_iszSpeakAs)
	{
		m_szGrp[TLK_ANSWER]		=	"FG_ANSWER";
		m_szGrp[TLK_QUESTION]	=	"FG_QUESTION";
		m_szGrp[TLK_IDLE]		=	"FG_IDLE";
		m_szGrp[TLK_STARE]		=	"FG_STARE";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER) //LRC
			m_szGrp[TLK_USE]	=	"FG_PFOLLOW";
		else
			m_szGrp[TLK_USE] =	"FG_OK";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER)
			m_szGrp[TLK_UNUSE] = "FG_PWAIT";
		else
			m_szGrp[TLK_UNUSE] = "FG_WAIT";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER)
			m_szGrp[TLK_DECLINE] =	"FG_POK";
		else
			m_szGrp[TLK_DECLINE] =	"FG_NOTOK";
		m_szGrp[TLK_STOP] =		"FG_STOP";

		m_szGrp[TLK_NOSHOOT] =	"FG_SCARED";
		m_szGrp[TLK_HELLO] =	"FG_HELLO";

		m_szGrp[TLK_PLHURT1] =	"!FG_CUREA";
		m_szGrp[TLK_PLHURT2] =	"!FG_CUREB"; 
		m_szGrp[TLK_PLHURT3] =	"!FG_CUREC";

		m_szGrp[TLK_PHELLO] =	NULL;	//"FG_PHELLO";		// UNDONE
		m_szGrp[TLK_PIDLE] =	NULL;	//"FG_PIDLE";			// UNDONE
		m_szGrp[TLK_PQUESTION] = "FG_PQUEST";		// UNDONE

		m_szGrp[TLK_SMELL] =	"FG_SMELL";
	
		m_szGrp[TLK_WOUND] =	"FG_WOUND";
		m_szGrp[TLK_MORTAL] =	"FG_MORTAL";
	}

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}


static BOOL IsFacing( entvars_t *pevTest, const Vector &reference )
{
	Vector vecDir = (reference - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward, angle;
	angle = pevTest->v_angle;
	angle.x = 0;
	UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );
	// He's facing me, he meant it
	if ( DotProduct( forward, vecDir ) > 0.96 )	// +/- 15 degrees or so
	{
		return TRUE;
	}
	return FALSE;
}


int CMonsterAMember :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	if ( !IsAlive() || pev->deadflag == DEAD_DYING )
		return ret;

	// LRC - if my reaction to the player has been overridden, don't do this stuff
	if (m_iPlayerReact) return ret;

	if ( m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) )
	{
		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if ( m_hEnemy == NULL )
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if ( (m_afMemory & bits_MEMORY_SUSPICIOUS) || IsFacing( pevAttacker, pev->origin ) )
			{
				// Alright, now I'm pissed!
				if (m_iszSpeakAs)
				{
					char szBuf[32];
					strcpy(szBuf,STRING(m_iszSpeakAs));
					strcat(szBuf,"_MAD");
					PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
				}
				else
				{
					PlaySentence( "FG_MAD", 4, VOL_NORM, ATTN_NORM );
				}

				Remember( bits_MEMORY_PROVOKED );
				StopFollowing( TRUE );
			}
			else
			{
				// Hey, be careful with that
				if (m_iszSpeakAs)
				{
					char szBuf[32];
					strcpy(szBuf,STRING(m_iszSpeakAs));
					strcat(szBuf,"_SHOT");
					PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
				}
				else
				{
					PlaySentence( "FG_SHOT", 4, VOL_NORM, ATTN_NORM );
				}
				Remember( bits_MEMORY_SUSPICIOUS );
			}
		}
		else if ( !(m_hEnemy->IsPlayer()) && pev->deadflag == DEAD_NO )
		{
			if (m_iszSpeakAs)
			{
				char szBuf[32];
				strcpy(szBuf,STRING(m_iszSpeakAs));
				strcat(szBuf,"_SHOT");
				PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
			}
			else
			{
				PlaySentence( "FG_SHOT", 4, VOL_NORM, ATTN_NORM );
			}
		}
	}

	m_fHasBeenHit = TRUE;
	m_flHitCounter = 100;

	return ret;
}

	
//=========================================================
// PainSound
//=========================================================
void CMonsterAMember :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	switch (RANDOM_LONG(0,2))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CMonsterAMember :: DeathSound ( void )
{
	switch (RANDOM_LONG(0,2))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}


void CMonsterAMember::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == HITGROUP_HEAD) // si el daño es en la cabeza
	{
		UTIL_BloodStream( ptr->vecEndPos,
					gpGlobals->v_forward * 10 +  // -5
					gpGlobals->v_up * 2, 
					(unsigned short)73, 100 );	

		switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar
		{
			case 0: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot1.wav", 1, ATTN_NORM ); break;
			case 1: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot2.wav", 1, ATTN_NORM ); break;
			case 2: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_headshot3.wav", 1, ATTN_NORM ); break;
		}
	}
	else
	{
		switch (RANDOM_LONG(0,2)) 
		{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/hit_kevlar-1.wav", 0.9, ATTN_NORM); break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/hit_kevlar-2.wav", 0.9, ATTN_NORM); break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/hit_kevlar-3.wav", 0.9, ATTN_NORM); break;
		}
	}

	CTalkMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


void CMonsterAMember::Killed( entvars_t *pevAttacker, int iGib )
{
//used a event

	SetUse( NULL );	
	CTalkMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Schedule_t* CMonsterAMember :: GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != NULL )
		{
			// face enemy, then draw.
			return slAmrneyEnemyDraw;
		}
		break;

	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slAmFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slAmFollow;

	case SCHED_AMEMBER_COVER_AND_RELOAD:
		{
			return &slAmiendHideReload[ 0 ];
		}	
	case SCHED_RANGE_ATTACK2:
		{
			return &slAmiendRangeAttack2[ 0 ];
		}

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
		{
			// just look straight ahead.
			return slIdleAmStand;
		}
		else
			return psched;	
	}

	return CTalkMonster::GetScheduleOfType( Type );
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CMonsterAMember :: GetSchedule ( void )
{
	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
	}
	if ( HasConditions( bits_COND_ENEMY_DEAD ) && FOkToSpeak() )
	{
		// Hey, be careful with that
		if (m_iszSpeakAs)
		{
			char szBuf[32];
			strcpy(szBuf,STRING(m_iszSpeakAs));
			strcat(szBuf,"_KILL");
			PlaySentence( szBuf, 4, VOL_NORM, ATTN_NORM );
		}
		else
		{
			PlaySentence( "FG_KILL", 4, VOL_NORM, ATTN_NORM );
		}
	}

	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

			// always act surprized with a new enemy
			if ( HasConditions( bits_COND_NEW_ENEMY ) && HasConditions( bits_COND_LIGHT_DAMAGE) )
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
				
//found enemy and deploy weapon					
			if (!m_fGunDrawn )//test
			{
				if (pev->frags == PISTOL)
				{
					return GetScheduleOfType( SCHED_ARM_WEAPON );
				}
				else if (pev->frags == DESERT)
				{
					return GetScheduleOfType( SCHED_ARM_WEAPON );
				}
				else
				{
					//can't deploy this weapon
				}
			}

			if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );

			// no ammo
			else if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_AMEMBER_COVER_AND_RELOAD );
			}
					
			//new add
			//enemy is occluded
			else if ( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
			{
				if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) )
				{
					//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
					if (FOkToSpeak())
					{

					}
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
			}
		}
		break;

	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType( SCHED_SMALL_FLINCH );
		}

		if ( m_hEnemy == NULL && IsFollowing() )
		{
			if ( !m_hTargetEnt->IsAlive() )
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing( FALSE );
				break;
			}
			else
			{
				if ( HasConditions( bits_COND_CLIENT_PUSH ) )
				{
					return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
				}
				return GetScheduleOfType( SCHED_TARGET_FACE );
			}
		}

		if ( HasConditions( bits_COND_CLIENT_PUSH ) )
		{
			return GetScheduleOfType( SCHED_MOVE_AWAY );
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}
	
	return CTalkMonster::GetSchedule();
}

MONSTERSTATE CMonsterAMember :: GetIdealState ( void )
{
	/*
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_COMBAT:
		{
		//clear surrendered enemies			
			CBaseEntity *pEnemy = m_hEnemy;
				
			if ( pEnemy != NULL )
			{
				if ( pEnemy->m_fSurrender == TRUE)
				{
					m_hEnemy = NULL;
	
					ALERT ( at_console, "INFO: Trying to change enemy\n" );

					return m_IdealMonsterState;
				}
			}
		}
		break;
	}
*/
	return CTalkMonster::GetIdealState();
}



void CMonsterAMember::DeclineFollowing( void )
{
	if ( IsAlive() )
	PlaySentence( m_szGrp[TLK_DECLINE], 2, VOL_NORM, ATTN_NORM ); //LRC
}


//=========================================================
// SetActivity 
//=========================================================
void CMonsterAMember :: SetActivity ( Activity NewActivity )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 3, vecGunPos, vecGunAngles );

	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		{
			if (pev->frags == MP5)
			{
				if(m_cAmmoLoaded <= 3)//treated as empty
				{
					if(m_fStanding)
					iSequence = LookupSequence( "standing_empty" );
					else
					iSequence = LookupSequence( "crouching_empty" );
				}
				else
				{
					if(m_fStanding)
					iSequence = LookupSequence( "standing_mp5" );
					else
					iSequence = LookupSequence( "crouching_mp5" );
				}
			}
			else if (pev->frags == SHOTGUN)
			{
				if(m_fStanding)
				iSequence = LookupSequence( "standing_shotgun" );
				else
				iSequence = LookupSequence( "crouching_shotgun" );
			}
			else if (pev->frags == LAW)
			{
				iSequence = LookupSequence( "LAW_fire" );//FIX- standing_mp5
			}
			else if (pev->frags == AWP)
			{
				if(m_fStanding)
				iSequence = LookupSequence( "sniper-fire" );
				else
				iSequence = LookupSequence( "sniper_crouch-fire" );
			}
			else if (pev->frags == ASSAULT)
			{
				if(m_cAmmoLoaded <= 3)//treated as empty
				{
					if(m_fStanding)
					iSequence = LookupSequence( "standing_empty" );
					else
					iSequence = LookupSequence( "crouching_empty" );
				}
				else
				{
					if(m_fStanding)
					iSequence = LookupSequence( "standing_rifle" );
					else
					iSequence = LookupSequence( "crouching_rifle" );
				}
			}
			else if (pev->frags == PISTOL)
			{
				if(m_cAmmoLoaded <= 3)//treated as empty
				{
					if(m_fStanding)
					iSequence = LookupSequence( "pistol_standing_empty" );
					else
					iSequence = LookupSequence( "pistol_crouching_empty" );
				}
				else
				{
					if(m_fStanding)
					iSequence = LookupSequence( "pistol_standing" );
					else
					iSequence = LookupSequence( "pistol_crouching" );
				}
			}
			else if (pev->frags == DESERT)
			{
			//	if(m_fStanding)//shoot all remaining bullets
			//	iSequence = LookupSequence( "pistol_standing" );
			//	else
				iSequence = LookupSequence( "pistol_crouching" );
			}
			else if (pev->frags == M249)
			{
				if(m_cAmmoLoaded <= 3)//treated as empty
				{
					if(m_fStanding)
					iSequence = LookupSequence( "standing_empty" );
					else
					iSequence = LookupSequence( "crouching_empty" );
				}
				else
				{
					if(m_fStanding)
					iSequence = LookupSequence( "standing_rifle" );
					else
					iSequence = LookupSequence( "crouching_rifle" );
				}
			}
	//		else if (pev->frags == SMG)
	//		{
	//			iSequence = LookupSequence( "pistol_standing" );
	//		}	
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}
		}
	break;

	//this is not neccesary. called from qc file

	//EDIT: yes... this is neccesary because it calls drop_grenade animation sometimes, and it calls Dropgrenade Event
	//and the monster just drop a grenade at their feets. This is bad because he needs to run. There is
	//not a good schedule that can handle this, so just throw grenades by now.

	case ACT_RANGE_ATTACK2:
		iSequence = LookupSequence( "throwgrenade" );
		break;

	case ACT_RUN:
		if ( (pev->health <= LIMP_HEALTH) /*|| (m_fHasBeenHit = TRUE) */)//ouch!
		{
			// limp!
			iSequence = LookupActivity ( ACT_RUN_HURT );
		}
		else
		{
			if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_run" );
			}
			else if (pev->frags == DESERT )
			{
				iSequence = LookupSequence( "pistol_run" );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );//look for normal ACT_RUN
			}
		}
		break;

	case ACT_WALK:
		if ( pev->health <= LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity ( ACT_WALK_HURT );
		}
		else
		{
			if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_patrol_street" ); break;
			}
			else if (pev->frags == DESERT )//pistol
			{
				iSequence = LookupSequence( "pistol_patrol_street" ); break;
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );//look for normal ACT_WALK
			}	
		}
		break;

	case ACT_RELOAD:
		{														
			if (pev->frags == LAW)//law		
			{
				iSequence = LookupSequence( "LAW_reload" );
				m_iNumClips--;//less clip!
			}
			else if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_reload" );
				DropItem( "item_clip_pistol", vecGunPos, vecGunAngles );//test
				//i_NumClips--;//well... why take off a clip?
			}
			else if (pev->frags == DESERT )//pistol
			{
				iSequence = LookupSequence( "pistol_reload" );
				DropItem( "item_clip_pistol", vecGunPos, vecGunAngles );//test
				//i_NumClips--;//well... why take off a clip?
			}
			else if (pev->frags == SHOTGUN)
			{
				iSequence = LookupSequence( "reload_shotgun" );
				m_iNumClips--;//less clip!
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );

				if(m_iNumClips > 1)//only if we have clips!
				DropItem( "item_clip_rifle", vecGunPos, vecGunAngles );//test
				m_iNumClips--;//less clip!
			}
					
			if(m_iNumClips <=0)//no clips. overriden for pistol because the monster have 999 clips of that before drop primary weapon
			{		
				if (pev->frags == PISTOL){}
				else if (pev->frags == DESERT){}
				else
				{
					iSequence = LookupSequence( "drop_weapon" );
				}
			}
		}
	 break;
/*
	case ACT_IDLE:
		if (pev->frags == PISTOL )//pistol
		{
			if ( RANDOM_LONG( 0, 1 ) == 0 )
			iSequence = LookupSequence( "pistol_idle1" );
			else
			iSequence = LookupSequence( "pistol_idle2" );
		}

		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		//	iSequence = LookupActivity ( ACT_IDLE_ANGRY );//si hay problemas puede ser esto
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}
		break;
*/

	case ACT_IDLE:
		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity ( NewActivity );
	break;

	case ACT_IDLE_ANGRY:
		if (pev->frags == MP5)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "combatidle_SMG" );
			else
			iSequence = LookupSequence( "crouching_idle" );
		}
		else if (pev->frags == SHOTGUN)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "standing_shotgun" );
			else
			iSequence = LookupSequence( "crouching_shotgun" );
		}
		else if (pev->frags == AWP)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "combatidle_SMG" );
			else
			iSequence = LookupSequence( "crouching_idle" );
		}
		else if (pev->frags == ASSAULT)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "combatidle" );
			else
			iSequence = LookupSequence( "crouching_idle" );
		}
		else if (pev->frags == PISTOL)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "pistol_combatidle" );
			else
			iSequence = LookupSequence( "pistolangryidle_low" );
		}	
		else if (pev->frags == DESERT)
		{
		//	if(m_fStanding)
		//	iSequence = LookupSequence( "pistol_combatidle" );
		//	else
			iSequence = LookupSequence( "pistolangryidle_low" );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}	
	break;
	
	case ACT_CROUCH:
		if (pev->frags == PISTOL )//pistol
		{
			iSequence = LookupSequence( "pistolangryidle_low" );//this is not a real crouch animation, but he don't make the standart crouch act, at least
		}	
		else if (pev->frags == DESERT )//pistol
		{
			iSequence = LookupSequence( "pistolangryidle_low" );
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

//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CMonsterAMember :: CheckMeleeAttack1 ( float flDot, float flDist )
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
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CMonsterAMember :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}
//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
//sys: esto es rarisimo, porque son raros los casos de gibbeo...Vamos a ponerlo
//igual de todas formas:
void CMonsterAMember :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	if ( GetBodygroup( GUN_GROUP ) != GUN_NONE && !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{// throw a gun if the grunt has one
		GetAttachment( 3, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
		if (pev->frags == 1){
		pGun = DropItem( "weapon_mp5", vecGunPos, vecGunAngles );}
		else if (pev->frags == 2){
		pGun = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );}
		else if (pev->frags == 3){
			pGun = DropItem( "weapon_rpg", vecGunPos, vecGunAngles );}
		else if (pev->frags == 4){
			pGun = DropItem( "weapon_sniper", vecGunPos, vecGunAngles );}
		else if (pev->frags == 5){
			pGun = DropItem( "weapon_m4a1", vecGunPos, vecGunAngles );}
		else if (pev->frags == 6){
			pGun = DropItem( "weapon_9mmhandgun", vecGunPos, vecGunAngles );}
		else if (pev->frags == 7){}
		else if (pev->frags == 8){
			pGun = DropItem( "weapon_M249", vecGunPos, vecGunAngles );}
		else if (pev->frags == 9){
		pGun = DropItem( "weapon_mp7", vecGunPos, vecGunAngles );}
		else
		{
			//CANT DROP ITEM!!!
		}
/*
		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			pGun->pev->angles.x = 90;
			pGun->pev->angles.y =  90;
		}
		*/
	}

	CBaseMonster :: GibMonster();
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CMonsterAMember :: CheckRangeAttack2 ( float flDot, float flDist )
{	
	if (m_iNumGrenades <= 0)
	return FALSE;
	
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
	
	if ( ( vecTarget - pev->origin ).Length2D() <= 256 )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

		
	Vector vecToss = VecCheckToss( pev, GetGunPosition(), vecTarget, 0.5 );

	if ( vecToss != g_vecZero )
	{
		m_vecTossVelocity = vecToss;

		// throw a hand grenade
		m_fThrowGrenade = TRUE;

		// take it down
		m_iNumGrenades -= 1;

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

	return m_fThrowGrenade;
}

//=========================================================
// FireWeapon 
//=========================================================
void CMonsterAMember :: FireWeapon ( void )
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );

	int pitchShift = RANDOM_LONG( 0, 20 );

	// Only shift about half the time
	if ( pitchShift > 10 )
		pitchShift = 0;
	else
		pitchShift -= 5;
	
	if(m_cAmmoLoaded > 4)//treated as empty
	pev->effects = EF_MUZZLEFLASH;

	if (pev->frags == MP5)
	{
		if(m_cAmmoLoaded <= 3)//treated as empty
		{
			m_cAmmoLoaded--;// take away a bullet!
			EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/357_cock1.wav", 1, ATTN_NORM );
			return;
		}

		if(m_fStealthed)//precise
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
		else
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		if(m_fStealthed)
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_firesil-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
		else
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );

		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iPistolShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == SHOTGUN)
	{
		FireBullets(8, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 1024, BULLET_PLAYER_BUCKSHOT);//357

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/spas12/spas12-fire.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 
	}
	else if (pev->frags == LAW)
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
	else if (pev->frags == AWP)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 99999, BULLET_PLAYER_SNIPER ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/sniper/sniper_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 
	}
	else if (pev->frags == ASSAULT)
	{
		if(m_cAmmoLoaded <= 3)//treated as empty
		{
			m_cAmmoLoaded--;// take away a bullet!
			EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/357_cock1.wav", 1, ATTN_NORM );
			return;
		}

		if(m_fStealthed)//precise
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
		else
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		if(m_fStealthed)
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/m4a1/m4a1_firesil-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
		else
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/m4a1/m4a1_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == PISTOL)
	{
		if(m_cAmmoLoaded <= 3)//treated as empty
		{
			m_cAmmoLoaded--;// take away a bullet!
			EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/357_cock1.wav", 1, ATTN_NORM );
			return;
		}

		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 99999, BULLET_PLAYER_9MM ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/1911/1911_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iPistolShell, TE_BOUNCE_SHELL); 
	}
	
	else if (pev->frags == DESERT)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_3DEGREES, 99999, BULLET_PLAYER_DEAGLE ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/deagle/deagle_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iPistolShell, TE_BOUNCE_SHELL); 
	}
//	else if (pev->frags == KNIFE)
//	{
		//MACHETE??
//	}
	else if (pev->frags == M249)
	{
		if(m_cAmmoLoaded <= 3)//treated as empty
		{
			m_cAmmoLoaded--;// take away a bullet!
			EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/357_cock1.wav", 1, ATTN_NORM );
			return;
		}

		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/M249/M249_fire-1.wav", 1, ATTN_NORM );
	
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 9999, BULLET_MONSTER_12MM ); // shoot +-5 degrees
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}/*
	else if (pev->frags == SMG)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 9999, BULLET_PLAYER_9MM ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/uzi/uzi_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
*/	else
	{
		//CANT SHOOT THE WEAPON!
	}

	m_cAmmoLoaded--;// take away a bullet!

	// Teh_Freak: World Lighting!
     MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
          WRITE_BYTE( TE_DLIGHT );
          WRITE_COORD( vecShootOrigin.x ); // origin
          WRITE_COORD( vecShootOrigin.y );
          WRITE_COORD( vecShootOrigin.z );
          WRITE_BYTE( 16 );     // radius
          WRITE_BYTE( 255 );     // R
          WRITE_BYTE( 255 );     // G
          WRITE_BYTE( 128 );     // B
          WRITE_BYTE( 0 );     // life * 10
          WRITE_BYTE( 0 ); // decay
     MESSAGE_END();
	// Teh_Freak: World Lighting!

	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	if (pPlayer->m_fSlowMotionOn)
	CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
}
		

//=========================================================
// FireWeapon 
//=========================================================
void CMonsterAMember :: FireWeaponWhenDie ( void )
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );

	Vector vecShootDir = vecShootOrigin + gpGlobals->v_forward * 5;

	Vector angDir = UTIL_VecToAngles( vecShootDir );

	SetBlending( 0, angDir.x );

	pev->effects = EF_MUZZLEFLASH;

	int pitchShift = RANDOM_LONG( 0, 20 );

	// Only shift about half the time
	if ( pitchShift > 10 )
		pitchShift = 0;
	else
		pitchShift -= 5;

	//only shoot automatic weapons
	if (pev->frags == MP5)
	{
		if(m_fStealthed)//precise
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
		else
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		if(m_fStealthed)
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_firesil-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
		else
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );

		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == SHOTGUN)
	{

	}
	else if (pev->frags == LAW)
	{

	}
	else if (pev->frags == AWP)
	{

	}
	else if (pev->frags == ASSAULT)
	{
		FireBulletsThroughWalls(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_PLAYER_M16 ); // shoot +-5 degrees

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/ak-47/ak47_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == PISTOL)
	{

	}	
	else if (pev->frags == DESERT)
	{

	}
//	else if (pev->frags == KNIFE)
//	{
		//MACHETE??
//	}
	else if (pev->frags == M249)
	{
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/M249/M249_fire-1.wav", 1, ATTN_NORM );
	
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 9999, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}/*
	else if (pev->frags == SMG)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 9999, BULLET_PLAYER_9MM ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/uzi/uzi_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
*/	else
	{
		//CANT SHOOT THE WEAPON!
	}

	// Teh_Freak: World Lighting!
     MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
          WRITE_BYTE( TE_DLIGHT );
          WRITE_COORD( vecShootOrigin.x ); // origin
          WRITE_COORD( vecShootOrigin.y );
          WRITE_COORD( vecShootOrigin.z );
          WRITE_BYTE( 16 );     // radius
          WRITE_BYTE( 255 );     // R
          WRITE_BYTE( 255 );     // G
          WRITE_BYTE( 128 );     // B
          WRITE_BYTE( 0 );     // life * 10
          WRITE_BYTE( 0 ); // decay
     MESSAGE_END();
	// Teh_Freak: World Lighting!

	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	if (pPlayer->m_fSlowMotionOn)
	CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
}

//=========================================================
// RunAI Tomado del cod de Assassin
//=========================================================
void CMonsterAMember :: RunAI( void )
{
	m_flLastLightLevel = GETENTITYILLUM( ENT ( pev ) );// make this our new light level.

	if ( CVAR_GET_FLOAT("dev_light" ) != 0 )
	{
		pev->rendermode = kRenderNormal;

		int rendertestvalue = m_flLastLightLevel;

		if (rendertestvalue >= 255)
			rendertestvalue = 255;

		pev->renderamt = rendertestvalue;
		pev->renderfx = 19;

		if (m_flLastLightLevel <= AMEMBER_MINIMUN_LIGHT)
		{	
	//		ALERT(at_aiconsole, "Monster Grunt detected low lights at %.2f %.2f %.2f\n", pev->origin.x, pev->origin.y, pev->origin.z);
			ALERT ( at_aiconsole, "La luz sobre el soldado es %f \n",m_flLastLightLevel );
							
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
		
	if(m_flHitCounter >= 1)
	m_flHitCounter -= 1;

	if(m_flHitCounter <= 0)
	m_fHasBeenHit = FALSE;

	CBaseMonster :: RunAI();
}
/*
void CMonsterAMember :: IdleSound( void )
{
	CBaseEntity *pEntity = NULL;

	Vector VecSrc = pev->origin;
		
	if ( m_flNextSuspCheckTime < gpGlobals->time )
	{
		while ((pEntity = UTIL_FindEntityInSphere( pEntity, VecSrc, 512 )) != NULL)//512
		{	
			if(	pEntity->m_fSurrender)
			EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "npc_voices/friend/SuspectCooperate.wav", 1, ATTN_NORM, 0, 100 );
		}
		
		m_flNextSuspCheckTime = gpGlobals->time + 10;
	}

	if (FOkToSpeak() && (g_fAmQuestion || RANDOM_LONG(0,1)))
	{		
		if (!g_fAmQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0,2))
			{
			case 0: // check in
				if (m_flLastLightLevel <= AMEMBER_MINIMUN_LIGHT)
					SENTENCEG_PlayRndSz(ENT(pev), "SPA_LIGHT", AMEMBER_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_CHECK", AMEMBER_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);					

				g_fAmQuestion = 1;
				break;
			case 1: // question
					if (m_flLastLightLevel <= AMEMBER_MINIMUN_LIGHT)
					SENTENCEG_PlayRndSz(ENT(pev), "SPA_LIGHT", AMEMBER_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_QUEST", AMEMBER_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
			
				g_fAmQuestion = 2;
				break;
			case 2: // statement
					if (m_flLastLightLevel <= AMEMBER_MINIMUN_LIGHT)
					SENTENCEG_PlayRndSz(ENT(pev), "SPA_LIGHT", AMEMBER_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_IDLE", AMEMBER_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fAmQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_CLEAR", AMEMBER_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_ANSWER", AMEMBER_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fAmQuestion = 0;
		}
	//	JustSpoke();
	}
}
*/
