


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
// monster template
//=========================================================
// UNDONE: Holster weapon?

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

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
#define		BARNEY_AE_DRAW		( 2 )
#define		BARNEY_AE_SHOOT		( 3 )
#define		BARNEY_AE_HOLSTER	( 4 )

#define	BARNEY_BODY_GUNHOLSTERED	0
#define	BARNEY_BODY_GUNDRAWN		1
#define BARNEY_BODY_GUNGONE			2




#define	HEVSCI_AE_SHOTGUN	( 21 )
#define	HEVSCI_AE_GAUSS		( 22 )
#define	HEVSCI_AE_MP5		( 23 )
#define	HEVSCI_AE_RPG		( 24 )
#define	HEVSCI_AE_PYTHON	( 25 )

#define HEVSCI_BODY_GROUP		0
#define HEVSCI_HEAD_GROUP		1
#define HEVSCI_GUN_GROUP		2

#define HEVSCI_GUN_NONE			0
#define HEVSCI_GUN_SHOTGUN		1
#define HEVSCI_GUN_GAUSS		2
#define HEVSCI_GUN_MP5			3
#define HEVSCI_GUN_RPG			4
#define HEVSCI_GUN_PYTHON		5

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_GRUNT_COVER_AND_RELOAD,
};


class CBarney : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	void BarneyFirePistol( void );
	void AlertSound( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	virtual int	ObjectCaps( void ) { return CTalkMonster :: ObjectCaps() | FCAP_IMPULSE_USE; }
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	
	void DeclineFollowing( void );

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

	int		m_iBaseBody; //LRC - for barneys with different bodies
	BOOL	m_fGunDrawn;
	float	m_painTime;
	float	m_checkAttackTime;
	BOOL	m_lastAttackCheck;

	// UNDONE: What is this for?  It isn't used?
	float	m_flPlayerDamage;// how much pain has the player inflicted on me?

	//for reload purposes
	void SetActivity ( Activity NewActivity );
	void CheckAmmo ( void );
	int	m_cClipSize;

	int		m_iBrassShell;
	int		m_iPistolShell;
	int		m_iShotgunShell;

	void FireWeapon( void );//for HEVSCI
	BOOL bHGruntPistol;
	BOOL	m_fFemaleCharacter;
	float m_flNextGrenadeCheck;

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_barney, CBarney );

TYPEDESCRIPTION	CBarney::m_SaveData[] = 
{
	DEFINE_FIELD( CBarney, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarney, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CBarney, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CBarney, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarney, m_flPlayerDamage, FIELD_FLOAT ),
	DEFINE_FIELD( CBarney, m_cClipSize, FIELD_INTEGER ),// FIX!!
	DEFINE_FIELD( CBarney, bHGruntPistol, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarney, m_fFemaleCharacter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarney, m_flNextGrenadeCheck, FIELD_TIME ),

};

IMPLEMENT_SAVERESTORE( CBarney, CTalkMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlBaFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slBaFollow[] =
{
	{
		tlBaFollow,
		ARRAYSIZE ( tlBaFollow ),
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
Task_t	tlBarneyHideReload[] =
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

Schedule_t slBarneyHideReload[] = 
{
	{
		tlBarneyHideReload,
		ARRAYSIZE ( tlBarneyHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntHideReload"
	}
};

//=========================================================
// BarneyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlBarneyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
};

Schedule_t slBarneyEnemyDraw[] = 
{
	{
		tlBarneyEnemyDraw,
		ARRAYSIZE ( tlBarneyEnemyDraw ),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t	tlBaFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slBaFaceTarget[] =
{
	{
		tlBaFaceTarget,
		ARRAYSIZE ( tlBaFaceTarget ),
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


Task_t	tlIdleBaStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleBaStand[] =
{
	{ 
		tlIdleBaStand,
		ARRAYSIZE ( tlIdleBaStand ), 
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
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

DEFINE_CUSTOM_SCHEDULES( CBarney )
{
	slBaFollow,
	slBarneyHideReload,
	slBarneyEnemyDraw,
	slBaFaceTarget,
	slIdleBaStand,
};


IMPLEMENT_CUSTOM_SCHEDULES( CBarney, CTalkMonster );

void CBarney :: StartTask( Task_t *pTask )
{
	CTalkMonster::StartTask( pTask );	
}

void CBarney :: RunTask( Task_t *pTask )
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
int CBarney :: ISoundMask ( void) 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CBarney :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_PLAYER_ALLY;
}

//=========================================================
// ALertSound - barney says "Freeze!"
//=========================================================
void CBarney :: AlertSound( void )
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
				PlaySentence( "BA_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
			}
		}
	}

}
//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CBarney :: SetYawSpeed ( void )
{
	int ys;

	ys = 0;

	switch ( m_Activity )
	{
	case ACT_IDLE:		
		ys = 70;
		break;
	case ACT_WALK:
		ys = 70;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	default:
		ys = 70;
		break;
	}

	pev->yaw_speed = ys;
}


//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CBarney :: CheckRangeAttack1 ( float flDot, float flDist )
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
		return m_lastAttackCheck;
	}
	return FALSE;
}


//=========================================================
// BarneyFirePistol - shoots one round from the pistol at
// the enemy barney is facing.
//=========================================================
void CBarney :: BarneyFirePistol ( void )
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
	pev->effects = EF_MUZZLEFLASH;

	if (pev->frags)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_PLAYER_357);
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/357_shot1.wav", 1, ATTN_NORM, 0, 100 );
		else
			EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/357_shot2.wav", 1, ATTN_NORM, 0, 100 );
	}
	else
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM );

		int pitchShift = RANDOM_LONG( 0, 20 );
	
		// Only shift about half the time
		if ( pitchShift > 10 )
			pitchShift = 0;
		else
			pitchShift -= 5;

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "barney/ba_attack2.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	}

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!
}
	

void CBarney :: FireWeapon ( void )
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

	if (pev->frags == HEVSCI_GUN_SHOTGUN)
	{
		ALERT ( at_console, "HEVSCI_GUN_SHOTGUN\n");
		FireBullets(8, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 1024, BULLET_PLAYER_BUCKSHOT);//357

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/shotgun/sbarrel1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 
	}
	else if (pev->frags == HEVSCI_GUN_RPG)
	{
		ALERT ( at_console, "HEVSCI_GUN_RPG\n");

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
	else if (pev->frags == HEVSCI_GUN_GAUSS)
	{
		ALERT ( at_console, "HEVSCI_GUN_GAUSS\n");

		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 99999, BULLET_PLAYER_SNIPER ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/sniper/sniper_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 
	}
	else if (pev->frags == HEVSCI_GUN_MP5)
	{
		ALERT ( at_console, "HEVSCI_GUN_MP5\n");

		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/m16/m16_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == HEVSCI_GUN_PYTHON)
	{
		ALERT ( at_console, "HEVSCI_GUN_PYTHON\n");

		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_3DEGREES, 99999, BULLET_PLAYER_DEAGLE ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/deagle/deagle_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iPistolShell, TE_BOUNCE_SHELL); 
	}
	else
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
}
	
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CBarney :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BARNEY_AE_SHOOT:
		BarneyFirePistol();
		break;

	//HEVSCI SCI!
	case HEVSCI_AE_SHOTGUN:
	case HEVSCI_AE_GAUSS:
	case HEVSCI_AE_MP5:
	case HEVSCI_AE_RPG:
	case HEVSCI_AE_PYTHON:
		FireWeapon();
		break;

	case BARNEY_AE_DRAW:
		{		
			if(m_fFemaleCharacter)
			{
				SetBodygroup( 2, 1 );//gun deployed
			}
			else
			{
				// barney's bodygroup switches here so he can pull gun from holster
				pev->body = m_iBaseBody + BARNEY_BODY_GUNDRAWN;
			}
						
			m_fGunDrawn = TRUE;
		}
		break;

	case BARNEY_AE_HOLSTER:
		{
			if(m_fFemaleCharacter)
			{
				SetBodygroup( 2, 0 );//gun deployed
			}
			else
			{
				// change bodygroup to replace gun in holster
				pev->body = m_iBaseBody + BARNEY_BODY_GUNHOLSTERED;
			}
					
			m_fGunDrawn = FALSE;
		}
		break;

	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CBarney :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	SET_MODEL( ENT(pev),"physics/models/barney.mdl");
	else
	SET_MODEL( ENT(pev),"models/barney.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_iBaseBody = pev->body; //LRC
	pev->body			= m_iBaseBody + BARNEY_BODY_GUNHOLSTERED; // gun in holster
	m_fGunDrawn			= FALSE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	if (pev->frags)//deagle
	{
		m_cClipSize		= 7;
	}
	else//glok
	{
		m_cClipSize		= 20;
	}

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse(&CBarney :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarney :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	PRECACHE_MODEL("physics/models/barney.mdl");
	else
	PRECACHE_MODEL("models/barney.mdl");

	PRECACHE_SOUND("barney/ba_attack1.wav" );
	PRECACHE_SOUND("barney/ba_attack2.wav" );

	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_SOUND("barney/ba_pain2.wav");
	PRECACHE_SOUND("barney/ba_pain3.wav");

	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");
	
	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}	

// Init talk data
void CBarney :: TalkInit()
{
	
	CTalkMonster::TalkInit();

	// barney speech group names (group names are in sentences.txt)

	if (!m_iszSpeakAs)
	{
		m_szGrp[TLK_ANSWER]		=	"BA_ANSWER";
		m_szGrp[TLK_QUESTION]	=	"BA_QUESTION";
		m_szGrp[TLK_IDLE]		=	"BA_IDLE";
		m_szGrp[TLK_STARE]		=	"BA_STARE";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER) //LRC
			m_szGrp[TLK_USE]	=	"BA_PFOLLOW";
		else
			m_szGrp[TLK_USE] =	"BA_OK";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER)
			m_szGrp[TLK_UNUSE] = "BA_PWAIT";
		else
			m_szGrp[TLK_UNUSE] = "BA_WAIT";
		if (pev->spawnflags & SF_MONSTER_PREDISASTER)
			m_szGrp[TLK_DECLINE] =	"BA_POK";
		else
			m_szGrp[TLK_DECLINE] =	"BA_NOTOK";
		m_szGrp[TLK_STOP] =		"BA_STOP";

		m_szGrp[TLK_NOSHOOT] =	"BA_SCARED";
		m_szGrp[TLK_HELLO] =	"BA_HELLO";

		m_szGrp[TLK_PLHURT1] =	"!BA_CUREA";
		m_szGrp[TLK_PLHURT2] =	"!BA_CUREB"; 
		m_szGrp[TLK_PLHURT3] =	"!BA_CUREC";

		m_szGrp[TLK_PHELLO] =	NULL;	//"BA_PHELLO";		// UNDONE
		m_szGrp[TLK_PIDLE] =	NULL;	//"BA_PIDLE";			// UNDONE
		m_szGrp[TLK_PQUESTION] = "BA_PQUEST";		// UNDONE

		m_szGrp[TLK_SMELL] =	"BA_SMELL";
	
		m_szGrp[TLK_WOUND] =	"BA_WOUND";
		m_szGrp[TLK_MORTAL] =	"BA_MORTAL";
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


int CBarney :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	if ( !IsAlive() || pev->deadflag == DEAD_DYING )
		return ret;

	// LRC - if my reaction to the player has been overridden, don't do this stuff
	if (m_iPlayerReact) return ret;

	if ( m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) )
	{
		m_flPlayerDamage += flDamage;

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
					PlaySentence( "BA_MAD", 4, VOL_NORM, ATTN_NORM );
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
					PlaySentence( "BA_SHOT", 4, VOL_NORM, ATTN_NORM );
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
				PlaySentence( "BA_SHOT", 4, VOL_NORM, ATTN_NORM );
			}
		}
	}

	return ret;
}

	
//=========================================================
// PainSound
//=========================================================
void CBarney :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	if (bHGruntPistol)
	{
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
	}
	else
	{
		if(m_fFemaleCharacter)
		{
			switch (RANDOM_LONG(0,8))
			{
				case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain01.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain02.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain03.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 3: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain04.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 4: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain05.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 5: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain06.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 6: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain07.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 7: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain08.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 8: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pain09.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			}
		}
		else
		{
			switch (RANDOM_LONG(0,2))
			{
				case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			}
		}
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CBarney :: DeathSound ( void )
{
	if(bHGruntPistol)
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
		if(m_fFemaleCharacter)
		{
			switch (RANDOM_LONG(0,1))
			{
				case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/die01.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/die02.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			}
		}
		else
		{
			switch (RANDOM_LONG(0,2))
			{
				case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "barney/ba_die3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
			}
		}
	}
}


void CBarney::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	switch( ptr->iHitgroup)
	{
	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST))
		{
			flDamage = flDamage / 2;
		}
		break;
	case 10:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet( ptr->vecEndPos, 1.0 );
				flDamage = 0.01;
			}
		}
		// always a head shot
		ptr->iHitgroup = HITGROUP_HEAD;
		break;
	}

	CTalkMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


void CBarney::Killed( entvars_t *pevAttacker, int iGib )
{
	if ( pev->body < m_iBaseBody + BARNEY_BODY_GUNGONE && !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{// drop the gun!
		Vector vecGunPos;
		Vector vecGunAngles;

		if(m_fFemaleCharacter)
		SetBodygroup( 2, 2 );//gun none
		else
		pev->body = m_iBaseBody + BARNEY_BODY_GUNGONE;

		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
		if (pev->frags)
			pGun = DropItem( "weapon_357", vecGunPos, vecGunAngles );
		else
			pGun = DropItem( "weapon_9mmhandgun", vecGunPos, vecGunAngles );
	}

	SetUse( NULL );	
	CTalkMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

Schedule_t* CBarney :: GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != NULL )
		{
			// face enemy, then draw.
			return slBarneyEnemyDraw;
		}
		break;

	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slBaFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slBaFollow;

	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slBarneyHideReload[ 0 ];
		}

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
		{
			// just look straight ahead.
			return slIdleBaStand;
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
Schedule_t *CBarney :: GetSchedule ( void )
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
			PlaySentence( "BA_KILL", 4, VOL_NORM, ATTN_NORM );
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
				
			// wait for one schedule to draw gun
			if (!m_fGunDrawn )
				return GetScheduleOfType( SCHED_ARM_WEAPON );

			if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
					
			// no ammo
			if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				if(m_fFemaleCharacter)
				if ( RANDOM_LONG( 0, 1 ) == 0 )
				EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/coverwhilereload01.wav", 1, ATTN_NORM, 0, GetVoicePitch());
				else
				EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/gottareload01.wav", 1, ATTN_NORM, 0, GetVoicePitch());

				return GetScheduleOfType ( SCHED_GRUNT_COVER_AND_RELOAD );
			}

			//detect monster type
			if(m_fFemaleCharacter)
			{
				if ( m_hEnemy != NULL )
				{
						
					if (gpGlobals->time < m_flNextGrenadeCheck )
					{
						if ( FClassnameIs( m_hEnemy->pev, "monster_headcrab" ) )
						EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/headcrabs01.wav", 1, ATTN_NORM, 0, GetVoicePitch());
						else if ( FClassnameIs( m_hEnemy->pev, "npc_zombine" ) )
						EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/zombies01.wav", 1, ATTN_NORM, 0, GetVoicePitch());
						else if ( FClassnameIs( m_hEnemy->pev, "npc_zombie" ) )
						EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/zombies01.wav", 1, ATTN_NORM, 0, GetVoicePitch());
						else if ( FClassnameIs( m_hEnemy->pev, "monster_zombie" ) )
						EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/zombies01.wav", 1, ATTN_NORM, 0, GetVoicePitch());
						else if ( FClassnameIs( m_hEnemy->pev, "npc_zombie_fast" ) )
						EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/zombies01.wav", 1, ATTN_NORM, 0, GetVoicePitch());
						else if (FClassnameIs( m_hEnemy->pev, "npc_combine" ))
						ALERT(at_console, "Civil Protection!\n");
						else if (FClassnameIs( m_hEnemy->pev, "npc_combine_random" ))
						ALERT(at_console, "Civil Protection!\n");
						else if (FClassnameIs( m_hEnemy->pev, "npc_combine_metrocop" ))
						ALERT(at_console, "Civil Protection!\n");
						else if (FClassnameIs( m_hEnemy->pev, "npc_combine_supersoldier" ))
						ALERT(at_console, "Civil Protection!\n");
						else if (FClassnameIs( m_hEnemy->pev, "monster_terrorist" ))
						ALERT(at_console, "Terrorists!\n");
						else if (FClassnameIs( m_hEnemy->pev, "monster_hgrunt_opfor" ))
						ALERT(at_console, "Soldiers!\n");
						else if (FClassnameIs( m_hEnemy->pev, "monster_hgrunt_rpg" ))
						ALERT(at_console, "Heavy Soldiers!\n");
						else if (FClassnameIs( m_hEnemy->pev, "monster_human_grunt" ))

						m_flNextGrenadeCheck = gpGlobals->time + 10; // one full second.
					}

					ALERT(at_console, "Soldiers!\n");
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
					if(m_fFemaleCharacter)
					{
						switch (RANDOM_LONG(0,1))
						{
							case 0:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pardonme01.wav", 1, ATTN_NORM ); break;
							case 1:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/pardonme02.wav", 1, ATTN_NORM ); break;
						}
					}

					return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
				}
				return GetScheduleOfType( SCHED_TARGET_FACE );
			}
		}

		if ( HasConditions( bits_COND_CLIENT_PUSH ) )
		{
			if(m_fFemaleCharacter)
			{
				switch (RANDOM_LONG(0,1))
				{
					case 0:	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/sorry01.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
					case 1:	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "npc_voices/friend/Kate/sorry02.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
				}
			}

			return GetScheduleOfType( SCHED_MOVE_AWAY );
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}
	
	return CTalkMonster::GetSchedule();
}

MONSTERSTATE CBarney :: GetIdealState ( void )
{
	return CTalkMonster::GetIdealState();
}

void CBarney::DeclineFollowing( void )
{
	if ( IsAlive() )//stupid laurie :P :P ...
	PlaySentence( m_szGrp[TLK_DECLINE], 2, VOL_NORM, ATTN_NORM ); //LRC
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CBarney :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// SetActivity 
//=========================================================
void CBarney :: SetActivity ( Activity NewActivity )
{
	int	iSequence = 0;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 0, vecGunPos, vecGunAngles );

	switch ( NewActivity)
	{
	case ACT_RELOAD:
		{
			iSequence = LookupSequence( "reload" );
				
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "barney/ba_reload1.wav", 1, ATTN_NORM );		
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);

		//	DropItem( "item_clip_pistol", vecGunPos, vecGunAngles );//test
		}
		break;
	default:
		iSequence = LookupActivity ( NewActivity );
		break;
	}

	//codigo mejorado
	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > -1 )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			// don't reset frame between walk and run
			if ( !(m_Activity == ACT_WALK || m_Activity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN))
				pev->frame = 0;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo( );
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT ( at_aiconsole, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present
	
	// In case someone calls this with something other than the ideal activity
	m_IdealActivity = m_Activity;
}





//=========================================================
// DEAD BARNEY PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iFirstPose properly!
//
//=========================================================
class CDeadBarney : public CBaseMonster
{
public:
	void Spawn( void );
	int	Classify ( void ) { return	CLASS_PLAYER_ALLY; }

	void KeyValue( KeyValueData *pkvd );

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[3];
};

char *CDeadBarney::m_szPoses[] = { "lying_on_back", "lying_on_side", "lying_on_stomach" };

void CDeadBarney::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_barney_dead, CDeadBarney );

//=========================================================
// ********** DeadBarney SPAWN **********
//=========================================================
void CDeadBarney :: Spawn( )
{
	PRECACHE_MODEL("models/barney.mdl");
	SET_MODEL(ENT(pev), "models/barney.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );

	// Corpses have less health
	pev->health			= 8;//gSkillData.barneyHealth;

	MonsterInitDead();
}




































class CHevSCI : public CBarney
{
public:
	void Spawn( );
	void Precache(void);
	void SetActivity ( Activity NewActivity );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	BOOL m_fStanding;
};

LINK_ENTITY_TO_CLASS( monster_hevsci, CHevSCI );

//=========================================================
// Spawn
//=========================================================
void CHevSCI :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hevsci.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_fGunDrawn			= TRUE;//TRUE!
	m_iszSpeakAs		= MAKE_STRING("SC");

	m_fStanding			= TRUE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	if ( FStringNull( pev->frags ) || (pev->frags == -1) )
	{
		switch (RANDOM_LONG(1,5))
		{
			case 1: pev->frags = HEVSCI_GUN_SHOTGUN; break;
			case 2: pev->frags = HEVSCI_GUN_GAUSS; break;
			case 3: pev->frags = HEVSCI_GUN_MP5; break;
			case 4: pev->frags = HEVSCI_GUN_RPG; break;
			case 5: pev->frags = HEVSCI_GUN_PYTHON; break;
		}
	}
		
	switch (RANDOM_LONG(1,4))
	{
		case 1: SetBodygroup( HEVSCI_HEAD_GROUP, 0 ); break;
		case 2: SetBodygroup( HEVSCI_HEAD_GROUP, 1 ); break;
		case 3: SetBodygroup( HEVSCI_HEAD_GROUP, 2 ); break;
		case 4: SetBodygroup( HEVSCI_HEAD_GROUP, 3 ); break;
	}

	if (pev->frags == HEVSCI_GUN_SHOTGUN)
	{
		SetBodygroup( HEVSCI_GUN_GROUP, HEVSCI_GUN_SHOTGUN );
		m_cClipSize		= 8;
	}
	else if (pev->frags == HEVSCI_GUN_RPG)
	{
		SetBodygroup( HEVSCI_GUN_GROUP, HEVSCI_GUN_RPG );
		m_cClipSize		= 1;
	}
	else if (pev->frags == HEVSCI_GUN_GAUSS)
	{
		SetBodygroup( HEVSCI_GUN_GROUP, HEVSCI_GUN_GAUSS );
		m_cClipSize		= 1;
	}
	else if (pev->frags == HEVSCI_GUN_MP5)
	{
		SetBodygroup( HEVSCI_GUN_GROUP, HEVSCI_GUN_MP5 );
		m_cClipSize		= 30;
	}
	else if (pev->frags == HEVSCI_GUN_PYTHON)
	{
		SetBodygroup( HEVSCI_GUN_GROUP, HEVSCI_GUN_PYTHON );
		m_cClipSize		= 7;
	}

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse(&CHevSCI :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHevSCI :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hevsci.mdl");

	UTIL_PrecacheOther( "hvr_rocket" );

	m_iBrassShell = PRECACHE_MODEL ("models/shell_762.mdl");
	m_iPistolShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");

	TalkInit();
	CTalkMonster::Precache();
}	

//=========================================================
// SetActivity 
//=========================================================
void CHevSCI :: SetActivity ( Activity NewActivity )
{
	int	iSequence = 0;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 0, vecGunPos, vecGunAngles );

	switch ( NewActivity)
	{
	case ACT_RELOAD:
		{
			iSequence = LookupSequence( "reload" );
				
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "barney/ba_reload1.wav", 1, ATTN_NORM );		
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
		}
		break;

	case ACT_RANGE_ATTACK1:
		{
			if (pev->frags == HEVSCI_GUN_SHOTGUN)
			{
				if(m_fStanding)
				iSequence = LookupSequence( "ref_shoot_shotgun" );
				else
				iSequence = LookupSequence( "crouch_shoot_shotgun" );
			}
			else if (pev->frags == HEVSCI_GUN_RPG)
			{
				if(m_fStanding)
				iSequence = LookupSequence( "ref_shoot_rpg" );
				else
				iSequence = LookupSequence( "crouch_shoot_rpg" );
			}
			else if (pev->frags == HEVSCI_GUN_GAUSS)
			{
				if(m_fStanding)
				iSequence = LookupSequence( "ref_shoot_gauss" );
				else
				iSequence = LookupSequence( "crouch_shoot_gauss" );
			}
			else if (pev->frags == HEVSCI_GUN_MP5)
			{
				if(m_fStanding)
				iSequence = LookupSequence( "ref_shoot_mp5" );
				else
				iSequence = LookupSequence( "crouch_shoot_mp5" );
			}
			else if (pev->frags == HEVSCI_GUN_PYTHON)
			{
				if(m_fStanding)
				iSequence = LookupSequence( "ref_shoot_python" );
				else
				iSequence = LookupSequence( "crouch_shoot_python" );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
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
		if (pev->frags == HEVSCI_GUN_SHOTGUN)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "ref_aim_shotgun" );
			else
			iSequence = LookupSequence( "crouch_aim_shotgun" );
		}
		else if (pev->frags == HEVSCI_GUN_GAUSS)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "ref_aim_gauss" );
			else
			iSequence = LookupSequence( "crouch_aim_gauss" );
		}
		else if (pev->frags == HEVSCI_GUN_MP5)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "ref_aim_mp5" );
			else
			iSequence = LookupSequence( "crouch_aim_mp5" );
		}
		else if (pev->frags == HEVSCI_GUN_RPG)
		{
			if(m_fStanding)
			iSequence = LookupSequence( "ref_aim_rpg" );
			else
			iSequence = LookupSequence( "crouch_aim_rpg" );
		}
		else if (pev->frags == HEVSCI_GUN_PYTHON)
		{
			iSequence = LookupSequence( "ref_aim_python" );
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

	//codigo mejorado
	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > -1 )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			// don't reset frame between walk and run
			if ( !(m_Activity == ACT_WALK || m_Activity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN))
				pev->frame = 0;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo( );
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT ( at_aiconsole, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present
	
	// In case someone calls this with something other than the ideal activity
	m_IdealActivity = m_Activity;
}

void CHevSCI::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == HITGROUP_HEAD) // si el daño es en la cabeza
	{
		m_bloodColor = BLOOD_COLOR_RED;
	}
	else
	{
		m_bloodColor = DONT_BLEED;

		// hit armor
		if ( pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0,10) < 1) )
		{
			UTIL_Ricochet( ptr->vecEndPos, RANDOM_FLOAT( 1, 2) );
			pev->dmgtime = gpGlobals->time;
		}

		if ( RANDOM_LONG( 0, 1 ) == 0 )
		{
			Vector vecTracerDir = vecDir;

			vecTracerDir.x += RANDOM_FLOAT( -0.3, 0.3 );
			vecTracerDir.y += RANDOM_FLOAT( -0.3, 0.3 );
			vecTracerDir.z += RANDOM_FLOAT( -0.3, 0.3 );

			vecTracerDir = vecTracerDir * -512;

			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, ptr->vecEndPos );
			WRITE_BYTE( TE_TRACER );
				WRITE_COORD( ptr->vecEndPos.x );
				WRITE_COORD( ptr->vecEndPos.y );
				WRITE_COORD( ptr->vecEndPos.z );

				WRITE_COORD( vecTracerDir.x );
				WRITE_COORD( vecTracerDir.y );
				WRITE_COORD( vecTracerDir.z );
			MESSAGE_END();
		}

		flDamage -= 20;
		if (flDamage <= 0)
			flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}
	CTalkMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}












































//*********************************************************************************
//*********************************************************************************
//*********************************************************************************
//*********************************************************************************

class COtis : public CBarney
{
public:
	void Spawn( );
	void Precache(void);
	void SetActivity ( Activity NewActivity );
	void Killed( entvars_t *pevAttacker, int iGib );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
};

LINK_ENTITY_TO_CLASS( monster_otis, COtis );


//=========================================================
// Spawn
//=========================================================

		#define OTIS_GUN_GROUP			1
#define OTIS_GUN_HOLSTERED		0
#define OTIS_GUN_DEAGLE_DRAW		1
#define OTIS_GUN_NONE			2

#define OTIS_HEAD_GROUP			2
#define OTIS_HEAD_OTHER			0
#define OTIS_HEAD_OTIS			1

void COtis :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	SET_MODEL( ENT(pev),"physics/models/otis.mdl");
	else
	SET_MODEL( ENT(pev),"models/otis.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.HgruntPiHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;
	
	SetBodygroup( OTIS_GUN_GROUP, OTIS_GUN_HOLSTERED );
	SetBodygroup( OTIS_HEAD_GROUP, OTIS_HEAD_OTIS );

	m_fGunDrawn			= FALSE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	//Vago FIX
	m_iszSpeakAs		= MAKE_STRING("HGPI");

	pev->frags		= 1;
	m_cClipSize		= 8;

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse(&COtis :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void COtis :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	PRECACHE_MODEL("physics/models/otis.mdl");
	else
	PRECACHE_MODEL("models/otis.mdl");

	PRECACHE_SOUND("barney/ba_attack1.wav" );
	PRECACHE_SOUND("barney/ba_attack2.wav" );

	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_SOUND("barney/ba_pain2.wav");
	PRECACHE_SOUND("barney/ba_pain3.wav");

	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");

	PRECACHE_SOUND( "barney/ba_reload1.wav" );

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}	

//=========================================================
// SetActivity 
//=========================================================
void COtis :: SetActivity ( Activity NewActivity )
{
	int	iSequence = 0;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RELOAD:
		{
			iSequence = LookupSequence( "reload" );
				
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "barney/ba_reload1.wav", 1, ATTN_NORM );		
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
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
		ALERT ( at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}
}
void COtis :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BARNEY_AE_SHOOT:
		BarneyFirePistol();
		break;

	case BARNEY_AE_DRAW:

		if (pev->frags)
		SetBodygroup( OTIS_GUN_GROUP, OTIS_GUN_DEAGLE_DRAW );
		else
		SetBodygroup( OTIS_GUN_GROUP, OTIS_GUN_DEAGLE_DRAW );

		m_fGunDrawn = TRUE;
		break;

	case BARNEY_AE_HOLSTER:
		SetBodygroup( OTIS_GUN_GROUP, OTIS_GUN_NONE );

		m_fGunDrawn = FALSE;
		break;

	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}
void COtis::Killed( entvars_t *pevAttacker, int iGib )
{
//same weapon. dont drop anything

	SetUse( NULL );	
	CTalkMonster::Killed( pevAttacker, iGib );
}







//**********************

class CBaMp5 : public CBarney
{
public:
	void Spawn( );
	void Precache(void);
	void SetActivity ( Activity NewActivity );
	void Killed( entvars_t *pevAttacker, int iGib );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void BarneyFireMp5( void );
};

LINK_ENTITY_TO_CLASS( monster_barney_mp5, CBaMp5 );
LINK_ENTITY_TO_CLASS( monster_barney_assault, CBaMp5 );


//=========================================================
// Spawn
//=========================================================

#define BAMP5_GUN_GROUP			1

#define GUN_NONE_FIX			0
#define GUN_MP5					1
#define GUN_M4A1				2
#define GUN_UZI					3


void CBaMp5 :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/barney_mp5.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.HgruntPiHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

//	SetBodygroup( GUN_GROUP, GUN_HOLSTERED );

	m_fGunDrawn			= FALSE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	if ( pev->frags == 0 )//no weapons
	{
		switch (RANDOM_LONG(0,2))
		{
			case 0: pev->frags = 1; break;	//uzi	
			case 1: pev->frags = 2; break; //mp5
			case 2: pev->frags = 3; break;//m4a1
		}
	}

	if ( pev->frags == 1 )//UZI
	{
		SetBodygroup( BAMP5_GUN_GROUP, GUN_UZI );
//		ALERT (at_console, "GUN_UZI\n");
	}
	else if ( pev->frags == 2 )//MP5
	{
		SetBodygroup( BAMP5_GUN_GROUP, GUN_MP5 );
//	    ALERT (at_console, "GUN_MP5\n");
	}
	else if ( pev->frags == 3 )//M4a1
	{
		SetBodygroup( BAMP5_GUN_GROUP, GUN_M4A1 );
//	    ALERT (at_console, "GUN_M4A1\n");
	}
	else
	{
		SetBodygroup( BAMP5_GUN_GROUP, GUN_NONE_FIX );
	    ALERT (at_console, "Spawn ERROR in monster_barney_mp5!!\n");
	}

	m_cClipSize		= 30;

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse(&CBaMp5 :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBaMp5 :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/barney_mp5.mdl");

	PRECACHE_SOUND("barney/ba_attack1.wav" );
	PRECACHE_SOUND("barney/ba_attack2.wav" );

	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_SOUND("barney/ba_pain2.wav");
	PRECACHE_SOUND("barney/ba_pain3.wav");

	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");

	PRECACHE_SOUND( "barney/ba_reload1.wav" );

	PRECACHE_SOUND("weapons/MP5/mp5_fire-1.wav");
	PRECACHE_SOUND("weapons/uzi/uzi_fire-1.wav");
	PRECACHE_SOUND("weapons/M4a1/m4a1_fire-1.wav");

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}	

//=========================================================
// SetActivity 
//=========================================================
void CBaMp5 :: SetActivity ( Activity NewActivity )
{
	int	iSequence = 0;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RELOAD:
		{
			iSequence = LookupSequence( "reload" );
				
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "barney/ba_reload1.wav", 1, ATTN_NORM );		
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
		}
		break;

	case ACT_WALK:
		{
			if ( pev->frags == 1 )//UZI
				iSequence = LookupSequence( "walk_uzi" );
			else
				iSequence = LookupSequence( "walk" );
		}
		break;

	case ACT_RUN:
		{
			if ( pev->frags == 1 )//UZI
				iSequence = LookupSequence( "run_uzi" );
			else
				iSequence = LookupSequence( "run" );
		}
		break;
		
	case ACT_RANGE_ATTACK1:
		if ( pev->frags == 1 )//UZI
		{
			if ( RANDOM_LONG(0,1) )
				iSequence = LookupSequence( "shoot_uzi_1" );
			else
				iSequence = LookupSequence( "shoot_uzi_2" );
		}
		else if ( pev->frags == 2 )//MP5
		{
			if ( RANDOM_LONG(0,1) )
				iSequence = LookupSequence( "shoot_mp5_1" );
			else
				iSequence = LookupSequence( "shoot_mp5_2" );
		}
		else if ( pev->frags == 3 )//M4a1
		{
			if ( RANDOM_LONG(0,1) )
				iSequence = LookupSequence( "shoot_m4a1_1" );
			else
				iSequence = LookupSequence( "shoot_m4a1_2" );
		}
		else
		{
			iSequence = LookupSequence( "idle" );//error here!
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
		ALERT ( at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}
}
void CBaMp5 :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BARNEY_AE_SHOOT:
		pev->framerate = 2.5;
	
		BarneyFireMp5();
		break;

	case BARNEY_AE_DRAW:
		// barney's bodygroup switches here so he can pull gun from holster

	//	SetBodygroup( GUN_GROUP, GUN_DEAGLE_DRAW );

		m_fGunDrawn = TRUE;
		break;
/*
	case BARNEY_AE_RELOAD:
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "barney/ba_reload1.wav", 1, ATTN_NORM );		
		m_cAmmoLoaded = m_cClipSize;
		ClearConditions(bits_COND_NO_AMMO_LOADED);
		break;
*/
	case BARNEY_AE_HOLSTER:
	//	SetBodygroup( GUN_GROUP, GUN_NONE );
		// change bodygroup to replace gun in holster

		m_fGunDrawn = FALSE;
		break;

	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}
void CBaMp5::Killed( entvars_t *pevAttacker, int iGib )
{
//same weapon. dont drop anything

	SetUse( NULL );	
	CTalkMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// BarneyFirePistol - shoots one round from the pistol at
// the enemy barney is facing.
//=========================================================
void CBaMp5 :: BarneyFireMp5 ( void )
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
	pev->effects = EF_MUZZLEFLASH;

	int pitchShift = RANDOM_LONG( 0, 20 );
	if ( pitchShift > 10 )
		pitchShift = 0;
	else
		pitchShift -= 5;

	if (pev->frags == 1)//usi
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_3DEGREES, 1024, BULLET_MONSTER_9MM );	
	
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/uzi/uzi_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	}
	else if (pev->frags == 2)//mp5
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_MP5 );
	
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	}
	else if (pev->frags == 3)//m4a1
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 1024, BULLET_PLAYER_M16 );

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/m4a1/m4a1_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	}
	else
	{
		ALERT (at_console, "ERROR! NO WEAPON FOR monster_barney_mp5\n");
	}

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );

	// UNDONE: Reload?
	m_cAmmoLoaded--;// take away a bullet!

	// Teh_Freak: World Lighting!
     MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
          WRITE_BYTE( TE_DLIGHT );
          WRITE_COORD( vecShootOrigin.x ); // origin
          WRITE_COORD( vecShootOrigin.y );
          WRITE_COORD( vecShootOrigin.z );
          WRITE_BYTE( 11 );     // radius
          WRITE_BYTE( 255 );     // R
          WRITE_BYTE( 255 );     // G
          WRITE_BYTE( 128 );     // B
          WRITE_BYTE( 0 );     // life * 10
          WRITE_BYTE( 0 ); // decay
     MESSAGE_END();
	// Teh_Freak: World Lighting

	 	
	 CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

	if (pPlayer->m_fSlowMotionOn)
	CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
}


class CBarneyHEV : public CBarney
{
public:
	void Spawn( void );
	void Precache( void );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
};
LINK_ENTITY_TO_CLASS( monster_barney_hev, CBarneyHEV );


//=========================================================
// Spawn
//=========================================================
void CBarneyHEV :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hev_barney.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_iBaseBody = pev->body; //LRC
	pev->body			= m_iBaseBody + BARNEY_BODY_GUNHOLSTERED; // gun in holster
	m_fGunDrawn			= FALSE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	if (pev->frags)//deagle
	{
		m_cClipSize		= 7;
	}
	else//glok
	{
		m_cClipSize		= 20;
	}

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse(&CBarneyHEV :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarneyHEV :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hev_barney.mdl");

	PRECACHE_SOUND("barney/ba_attack1.wav" );
	PRECACHE_SOUND("barney/ba_attack2.wav" );

	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_SOUND("barney/ba_pain2.wav");
	PRECACHE_SOUND("barney/ba_pain3.wav");

	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");
	
	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

void CBarneyHEV::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == HITGROUP_HEAD) // si el daño es en la cabeza
	{
		m_bloodColor = BLOOD_COLOR_RED;
	}
	else
	{
		m_bloodColor = DONT_BLEED;

		// hit armor
		if ( pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0,10) < 1) )
		{
			UTIL_Ricochet( ptr->vecEndPos, RANDOM_FLOAT( 1, 2) );
			pev->dmgtime = gpGlobals->time;
		}

		if ( RANDOM_LONG( 0, 1 ) == 0 )
		{
			Vector vecTracerDir = vecDir;

			vecTracerDir.x += RANDOM_FLOAT( -0.3, 0.3 );
			vecTracerDir.y += RANDOM_FLOAT( -0.3, 0.3 );
			vecTracerDir.z += RANDOM_FLOAT( -0.3, 0.3 );

			vecTracerDir = vecTracerDir * -512;

			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, ptr->vecEndPos );
			WRITE_BYTE( TE_TRACER );
				WRITE_COORD( ptr->vecEndPos.x );
				WRITE_COORD( ptr->vecEndPos.y );
				WRITE_COORD( ptr->vecEndPos.z );

				WRITE_COORD( vecTracerDir.x );
				WRITE_COORD( vecTracerDir.y );
				WRITE_COORD( vecTracerDir.z );
			MESSAGE_END();
		}

		flDamage -= 20;
		if (flDamage <= 0)
			flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}
	CTalkMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}









///////////

class CHGruntPistol : public CBarney
{
public:
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS( monster_hgrunt_pistol, CHGruntPistol );
LINK_ENTITY_TO_CLASS( monster_human_grunt_pistol, CHGruntPistol );


//=========================================================
// Spawn
//=========================================================
void CHGruntPistol :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	SET_MODEL( ENT(pev),"physics/models/hgrunt_pistol.mdl");
	else
	SET_MODEL( ENT(pev),"models/hgrunt_pistol.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_iBaseBody = pev->body; //LRC
	pev->body			= m_iBaseBody + BARNEY_BODY_GUNHOLSTERED; // gun in holster
	m_fGunDrawn			= FALSE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_cClipSize		= 20;

	m_cAmmoLoaded		= m_cClipSize;

	bHGruntPistol = TRUE;

	MonsterInit();
	SetUse(&CHGruntPistol :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHGruntPistol :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	PRECACHE_MODEL("physics/models/hgrunt_pistol.mdl");
	else
	PRECACHE_MODEL("models/hgrunt_pistol.mdl");

	PRECACHE_SOUND("barney/ba_attack1.wav" );
	PRECACHE_SOUND("barney/ba_attack2.wav" );

	PRECACHE_SOUND( "hgrunt/gr_die1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die3.wav" );

	PRECACHE_SOUND( "hgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain5.wav" );

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}












//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////




class CFemaleCharacter : public CBarney
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_kate, CFemaleCharacter );

//=========================================================
// Spawn
//=========================================================
void CFemaleCharacter :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/Kate_Romka/kate.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_fGunDrawn			= FALSE;
	m_fFemaleCharacter = TRUE;
	m_iszSpeakAs		= MAKE_STRING("FEM");
/*
	SetBodygroup( BODY_GROUP, 0 );
	SetBodygroup( HEAD_GROUP, 0 );
	SetBodygroup( GUN_GROUP, 0 );//gun at side
*/
	pev->body			= m_iBaseBody + BARNEY_BODY_GUNHOLSTERED; // gun in holster

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_cClipSize		= 20;

	pev->framerate = 1.5;

	m_cAmmoLoaded		= m_cClipSize;

	MonsterInit();
	SetUse(&CFemaleCharacter :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CFemaleCharacter :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/Kate_Romka/kate.mdl");

	PRECACHE_SOUND("npc_voices/friend/Kate/pain01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain02.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain03.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain04.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain05.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain06.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain07.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain08.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pain09.wav");

	//used from sentences.txt
/*
	PRECACHE_SOUND("npc_voices/friend/Kate/illstayhere01.wav");

	PRECACHE_SOUND("npc_voices/friend/Kate/leadtheway01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/leadtheway02.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/letsgo01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/letsgo02.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/ok01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/ok02.wav");
*/
	PRECACHE_SOUND("npc_voices/friend/Kate/pardonme01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/pardonme02.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/sorry01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/sorry02.wav");

	PRECACHE_SOUND("npc_voices/friend/Kate/die01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/die02.wav");

	PRECACHE_SOUND("npc_voices/friend/Kate/coverwhilereload01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/gottareload01.wav");

	PRECACHE_SOUND("npc_voices/friend/Kate/zombies01.wav");
	PRECACHE_SOUND("npc_voices/friend/Kate/headcrabs01.wav");

	PRECACHE_SOUND("barney/ba_attack1.wav" );
	PRECACHE_SOUND("barney/ba_attack2.wav" );

	TalkInit();
	CTalkMonster::Precache();
}	