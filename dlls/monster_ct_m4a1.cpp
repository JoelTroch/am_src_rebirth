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

#define	LIMP_HEALTH			20
#define	ACTIVITY_NOT_AVAILABLE	-1
//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?

//Its this used???
#define	FBARNEY_AE_DRAW		( 21 )
#define	BARNEY_AE_SHOOT		( 3 )
#define	FBARNEY_AE_HOLSTER	( 9 )

#define	HGRUNT_AE_RELOAD		( 2 )
#define	HGRUNT_AE_KICK			( 3 )
#define	HGRUNT_AE_BURST1		( 4 )
#define	HGRUNT_AE_BURST2		( 5 ) 
#define	HGRUNT_AE_BURST3		( 6 ) 
#define	HGRUNT_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define	BARNEY_BODY_GUNHOLSTERED	0
#define	BARNEY_BODY_GUNDRAWN		1
#define BARNEY_BODY_GUNGONE			2
#define	BARNEY_BODY_GUNDEAGLE		3

//#define	BARNEY_BODY_GUNFIX			4

#define	BARNEY_BODY_GUNFIX			4

#define HEAD_GROUP					1

#define HEAD_GRUNT					0
#define HEAD_COMMANDER				1
#define HEAD_SHOTGUN				2
#define HEAD_M203					3
#define HEAD_OTHER					4

#define GUN_GROUP					2

#define GUN_MP5						0//FRAG 1 
#define GUN_SHOTGUN					1//FRAG 1 
#define GUN_LAW						2//FRAG 2
#define GUN_AWP						3//FRAG 3 
#define GUN_ASSAULT					4//FRAG 4 
#define GUN_PISTOL					5//FRAG 5 
#define GUN_M249					6//FRAG 6 				
#define GUN_SMG						7//FRAG 7 
#define GUN_NONE					8 
//#define GUN_KNIFE					6//FRAG 6 


//This is used for pev->frag (weapon)
//BUGBUG: The ally models doesn't have the machete
#define MP5						1
#define SHOTGUN					2
#define LAW						3
#define AWP						4
#define ASSAULT					5 
#define PISTOL					6
//#define KNIFE					7
#define M249					8			
#define SMG						9

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_GRUNT_COVER_AND_RELOAD,
};

class CMonsterCT : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	void FireWeapon( void );
	void AlertSound( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	int		m_cClipSize;
	void GibMonster( void );

	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	virtual int	ObjectCaps( void ) { return CTalkMonster :: ObjectCaps() | FCAP_IMPULSE_USE; }
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	
	void DeclineFollowing( void );
	void SetActivity ( Activity NewActivity );
	void CheckAmmo ( void );

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

	int		m_iBrassShell;
	int		m_iShotgunShell;

	//no grenades, no launcer...
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }

	// UNDONE: What is this for?  It isn't used?
	float	m_flPlayerDamage;// how much pain has the player inflicted on me?

	CUSTOM_SCHEDULES;
};

//LINK_ENTITY_TO_CLASS( monster_ct_m4a1_ally, CMonsterCT );//moved to another func

TYPEDESCRIPTION	CMonsterCT::m_SaveData[] = 
{
	DEFINE_FIELD( CMonsterCT, m_iBaseBody, FIELD_INTEGER ), //LRC
	DEFINE_FIELD( CMonsterCT, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterCT, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CMonsterCT, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CMonsterCT, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterCT, m_flPlayerDamage, FIELD_FLOAT ),
	DEFINE_FIELD( CMonsterCT, m_cClipSize, FIELD_INTEGER ),// FIX!!
};

IMPLEMENT_SAVERESTORE( CMonsterCT, CTalkMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlCtFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slCtFollow[] =
{
	{
		tlCtFollow,
		ARRAYSIZE ( tlCtFollow ),
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
Task_t	tlCtiendHideReload[] =
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

Schedule_t slCtiendHideReload[] = 
{
	{
		tlCtiendHideReload,
		ARRAYSIZE ( tlCtiendHideReload ),
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
Task_t	tlCtrneyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	//sys
//	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
//	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_RANGE_ATTACK1 },
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_SIGNAL1 },
};

Schedule_t slCtrneyEnemyDraw[] = 
{
	{
		tlCtrneyEnemyDraw,
		ARRAYSIZE ( tlCtrneyEnemyDraw ),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t	tlCtFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slCtFaceTarget[] =
{
	{
		tlCtFaceTarget,
		ARRAYSIZE ( tlCtFaceTarget ),
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


Task_t	tlIdleCtStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleCtStand[] =
{
	{ 
		tlIdleCtStand,
		ARRAYSIZE ( tlIdleCtStand ), 
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

DEFINE_CUSTOM_SCHEDULES( CMonsterCT )
{
	slCtFollow,
	slCtiendHideReload,
	slCtrneyEnemyDraw,
	slCtFaceTarget,
	slIdleCtStand,
};


IMPLEMENT_CUSTOM_SCHEDULES( CMonsterCT, CTalkMonster );

void CMonsterCT :: StartTask( Task_t *pTask )
{
	CTalkMonster::StartTask( pTask );	
}

void CMonsterCT :: RunTask( Task_t *pTask )
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
int CMonsterCT :: ISoundMask ( void) 
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
int	CMonsterCT :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_PLAYER_ALLY;
}

//=========================================================
// ALertSound - barney says "Freeze!"
//=========================================================
void CMonsterCT :: AlertSound( void )
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
void CMonsterCT :: SetYawSpeed ( void )
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
BOOL CMonsterCT :: CheckRangeAttack1 ( float flDot, float flDist )
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
// FireWeapon - shoots one round from the pistol at
// the enemy barney is facing.
//=========================================================
void CMonsterCT :: FireWeapon ( void )
{
	/*
#define MP5						1
#define SHOTGUN					2
#define LAW						3
#define AWP						4
#define ASSAULT					5 
#define PISTOL					6
#define KNIFE					7
#define M249					8			
#define SMG						9
	*/
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
	pev->effects = EF_MUZZLEFLASH;

	if (pev->frags == MP5)
	{
		FireBulletsThroughWalls(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		int pitchShift = RANDOM_LONG( 0, 20 );
	
		// Only shift about half the time
		if ( pitchShift > 10 )
			pitchShift = 0;
		else
			pitchShift -= 5;

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == SHOTGUN)
	{
		//un solo perdigon?
		FireBullets(8, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 1024, BULLET_PLAYER_BUCKSHOT);//357

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/m_shotgun.wav", 1, ATTN_NORM, 0, 100 );
	
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
		FireBulletsThroughWalls(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_PLAYER_M16 ); // shoot +-5 degrees

		int pitchShift = RANDOM_LONG( 0, 20 );
	
		// Only shift about half the time
		if ( pitchShift > 10 )
			pitchShift = 0;
		else
			pitchShift -= 5;

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/m4a1/m4a1_firesil-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == PISTOL)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 99999, BULLET_PLAYER_9MM ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/ber92f/ber92f_firesil-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
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
	}
	else if (pev->frags == SMG)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 9999, BULLET_PLAYER_9MM ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/uzi/uzi_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
	}
	else
	{
		//CANT SHOOT THE WEAPON!
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
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CMonsterCT :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case HGRUNT_AE_BURST1:
		FireWeapon();
		CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		break;
//sys	
	case HGRUNT_AE_BURST2:
	case HGRUNT_AE_BURST3:
		FireWeapon();
		break;
/*
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

 #define GUN_MP5						0//FRAG 1 
#define GUN_SHOTGUN					1//FRAG 1 
#define GUN_LAW						2//FRAG 2
#define GUN_AWP						3//FRAG 3 
#define GUN_ASSAULT					4//FRAG 4 
#define GUN_PISTOL					5//FRAG 5 
#define GUN_KNIFE					6//FRAG 6 
#define GUN_M249					7//FRAG 7 				
#define GUN_SMG						8//FRAG 8 
*/
		case HGRUNT_AE_DROP_GUN:
			{
				//SP: si tiene el flag de "no tirar arma" cerrar con break y no ejecutar
				//mas codigo
				if (pev->spawnflags & SF_MONSTER_NO_WPN_DROP) break; //LRC

				Vector	vecGunPos;
				Vector	vecGunAngles;

				GetAttachment( 0, vecGunPos, vecGunAngles );

				SetBodygroup( GUN_GROUP, GUN_NONE );

				CBaseEntity *pItem;

				if (pev->frags == MP5){
					pItem = DropItem( "weapon_mp5", vecGunPos, vecGunAngles );}
				else if (pev->frags == SHOTGUN){
					pItem = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );}
				else if (pev->frags == LAW){
					pItem = DropItem( "weapon_rpg", vecGunPos, vecGunAngles );}
				else if (pev->frags == AWP){
					pItem = DropItem( "weapon_sniper", vecGunPos, vecGunAngles );}
				else if (pev->frags == ASSAULT){
					pItem = DropItem( "weapon_m4a1", vecGunPos, vecGunAngles );}
				else if (pev->frags == PISTOL){
					pItem = DropItem( "weapon_ber92f", vecGunPos, vecGunAngles );}
				else if (pev->frags == M249){
					pItem = DropItem( "weapon_M249", vecGunPos, vecGunAngles );}
				else if (pev->frags == SMG){
					pItem = DropItem( "weapon_uzi", vecGunPos, vecGunAngles );}			
				else
				{
					//CANT DROP ITEM!!!
				}
			
				pItem->pev->spawnflags |= SF_NORESPAWN; // No respawn
			}
		break;

	case HGRUNT_AE_RELOAD:
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "human_m4a1/reload1.wav", 1, ATTN_NORM );
		m_cAmmoLoaded = m_cClipSize;
		ClearConditions(bits_COND_NO_AMMO_LOADED);
		break;

	case FBARNEY_AE_DRAW:
		// barney's bodygroup switches here so he can pull gun from holster
		//sys
		if (pev->frags)//shotgun
	//	pev->body = m_iBaseBody + BARNEY_BODY_GUNDEAGLE;
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
			else
//		pev->body = m_iBaseBody + BARNEY_BODY_GUNDRAWN;
		SetBodygroup( GUN_GROUP, GUN_MP5 );

		m_fGunDrawn = TRUE;
		break;

	case FBARNEY_AE_HOLSTER:
		// change bodygroup to replace gun in holster
		pev->body = m_iBaseBody + BARNEY_BODY_GUNHOLSTERED;
		m_fGunDrawn = FALSE;
		break;

	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CMonsterCT :: Spawn()
{
	Precache( );
/*
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/human_m4a1.mdl");
*/
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
//	pev->body			= m_iBaseBody + BARNEY_BODY_GUNHOLSTERED; // gun in holster
//	m_fGunDrawn			= FALSE;

	pev->body			= m_iBaseBody + BARNEY_BODY_GUNDRAWN; // gun in holster
	m_fGunDrawn			= TRUE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	if (pev->frags == -1) //si no esta seteado la deagle
	{
		if (RANDOM_LONG( 0, 99 ) < 60) //
		{
			 pev->frags = 0;
			 ALERT (at_console, "n4a1\n");
		}
		else
		{
			pev->frags = 1;		
			ALERT (at_console, "shotgun\n");
		}	
	}
	
	if (pev->frags == MP5)
	{
		SetBodygroup( GUN_GROUP, GUN_MP5 );
		m_cClipSize		= 30;
	}
	else if (pev->frags == SHOTGUN)
	{
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		m_cClipSize		= 8;
	}
	else if (pev->frags == LAW)
	{
		SetBodygroup( GUN_GROUP, GUN_LAW );
		m_cClipSize		= 1;
	}
	else if (pev->frags == AWP)
	{
		SetBodygroup( GUN_GROUP, GUN_AWP );
		m_cClipSize		= 1;
	}
	else if (pev->frags == ASSAULT)
	{
		SetBodygroup( GUN_GROUP, GUN_ASSAULT );
		m_cClipSize		= 30;
	}
	else if (pev->frags == PISTOL)
	{
		SetBodygroup( GUN_GROUP, GUN_PISTOL );
		m_cClipSize		= 13;
	}
//	else if (pev->frags == KNIFE)
//	{
//		SetBodygroup( GUN_GROUP, GUN_KNIFE );
//		m_cClipSize		= 0;
//	}
	else if (pev->frags == M249)
	{
		SetBodygroup( GUN_GROUP, GUN_M249 );
		m_cClipSize		= 70;
	}
	else if (pev->frags == SMG)
	{
		SetBodygroup( GUN_GROUP, GUN_SMG );
		m_cClipSize		= 35;
	}

	if ( FStringNull( pev->skin ))
	{
		switch (RANDOM_LONG(0,3))
		{
			case 0: pev->skin = 0; break;
			case 1: pev->skin = 1; break;
			case 2: pev->skin = 2; break;
			case 3: pev->skin = 3; break;
		}
	}
	
	if (pev->skin == -1)//RANDOM (used in the fgd)
	{
		switch (RANDOM_LONG(0,3))
		{
			case 0: pev->skin = 0; break;
			case 1: pev->skin = 1; break;
			case 2: pev->skin = 2; break;
			case 3: pev->skin = 3; break;
		}
	}

	m_cAmmoLoaded		= m_cClipSize;
	
	switch (RANDOM_LONG(0,3))
	{
		case 0: SetBodygroup( HEAD_GROUP, HEAD_GRUNT ); break;
		case 1:	SetBodygroup( HEAD_GROUP, HEAD_COMMANDER ); break;
		case 2: SetBodygroup( HEAD_GROUP, HEAD_SHOTGUN ); break;
		case 3: SetBodygroup( HEAD_GROUP, HEAD_M203 ); break;
	}

	MonsterInit();
	SetUse(&CMonsterCT :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMonsterCT :: Precache()
{
	/*
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/human_m4a1.mdl");
*/
	UTIL_PrecacheOther( "hvr_rocket" );

//	PRECACHE_SOUND("weapons/m4a1/m4a1_fire-1.wav");//this is already precached

	PRECACHE_SOUND("weapons/m_shotgun.wav");//this is already precached

	PRECACHE_SOUND("human_m4a1/reload1.wav");

	PRECACHE_SOUND("human_m4a1/pain1.wav");
	PRECACHE_SOUND("human_m4a1/pain2.wav");
	PRECACHE_SOUND("human_m4a1/pain3.wav");
	PRECACHE_SOUND("human_m4a1/pain4.wav");
	PRECACHE_SOUND("human_m4a1/pain5.wav");

	PRECACHE_SOUND("human_m4a1/die1.wav");
	PRECACHE_SOUND("human_m4a1/die2.wav");
	PRECACHE_SOUND("human_m4a1/die3.wav");

	m_iBrassShell = PRECACHE_MODEL ("models/shell_762.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}	

// Init talk data
void CMonsterCT :: TalkInit()
{
	
	CTalkMonster::TalkInit();

	// barney speech group names (group names are in sentences.txt)

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


int CMonsterCT :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

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
					PlaySentence( "FG_MAD", 4, VOL_NORM, ATTN_NORM );
				}
			
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Hey, mas cuidado la proxima vez, idiota!" );

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
						
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Estoy de tu lado imbecil!" );

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
									
			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Tenemos un traidor! Fred nos esta atacando!" );
		}
	}

	return ret;
}

	
//=========================================================
// PainSound
//=========================================================
void CMonsterCT :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	switch (RANDOM_LONG(0,4))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/pain1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/pain2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/pain3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 3: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/pain4.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 4: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/pain5.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CMonsterCT :: DeathSound ( void )
{
	switch (RANDOM_LONG(0,2))
	{
	case 0: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/die1.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 1: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/die2.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	case 2: EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "human_m4a1/die3.wav", 1, ATTN_NORM, 0, GetVoicePitch()); break;
	}
}


void CMonsterCT::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
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


void CMonsterCT::Killed( entvars_t *pevAttacker, int iGib )
{
	//USED a EVENT
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): ..." );

	SetUse( NULL );	
	CTalkMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Schedule_t* CMonsterCT :: GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != NULL )
		{
			// face enemy, then draw.
			return slCtrneyEnemyDraw;
		}
		break;

	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slCtFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slCtFollow;

	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slCtiendHideReload[ 0 ];
		}

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
		{
			// just look straight ahead.
			return slIdleCtStand;
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
Schedule_t *CMonsterCT :: GetSchedule ( void )
{
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

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
			//esto causa problemas, creo, al encontrar enemigo y reemplazar el chequeo
		//	if ( HasConditions( bits_COND_NEW_ENEMY ))
		//	{
				/*
				if (RANDOM_LONG(0,1))
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Enemigo avistado!" );
				else
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Enemigo avistado2!" );
				*/
		//	}
		

			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Enemigo caido!" );
				PlaySentence( "FG_EDEAD", 4, VOL_NORM, ATTN_NORM );

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
			else if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				
				if (RANDOM_LONG(0,1))
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): No tengo municion!" );
				else
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Cubranme! Necesito recargar!" );
				
				PlaySentence( "FG_RELOAD", 4, VOL_NORM, ATTN_NORM );

				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_GRUNT_COVER_AND_RELOAD );
			}
		}
		break;

	case MONSTERSTATE_ALERT:	
	case MONSTERSTATE_IDLE:
		if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Estoy siendo atacado!" );
			
			PlaySentence( "FG_ATTACKED", 4, VOL_NORM, ATTN_NORM );

			// flinch if hurt
			return GetScheduleOfType( SCHED_SMALL_FLINCH );
		}

		if ( m_hEnemy == NULL && IsFollowing() )
		{
			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Te estoy siguiendo, Fred" );
		
			PlaySentence( "FG_FOLLOW", 2, VOL_NORM, ATTN_NONE );

			if ( !m_hTargetEnt->IsAlive() )
			{
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Maldicion! Fred ha caido!" );
				PlaySentence( "FG_PLDEAD", 4, VOL_NORM, ATTN_NORM );

				// UNDONE: Comment about the recently dead player here?
				StopFollowing( FALSE );
				break;
			}
			else
			{
				if ( HasConditions( bits_COND_CLIENT_PUSH ) )
				{
					if (RANDOM_LONG(0,1))
					ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Me estoy moviendo, Fred" );
					else
					ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Perdona, Fred" );

					PlaySentence( "FG_PLBLOK", 4, VOL_NORM, ATTN_NORM );
			
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

MONSTERSTATE CMonsterCT :: GetIdealState ( void )
{
	return CTalkMonster::GetIdealState();
}



void CMonsterCT::DeclineFollowing( void )
{
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	ClientPrint(pPlayer->pev, HUD_PRINTTALK, "Equipo aliado (RADIO): Perdon Fred, no puedo seguirte..." );

	PlaySentence( m_szGrp[TLK_DECLINE], 2, VOL_NORM, ATTN_NORM ); //LRC
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
class CDeadCt : public CBaseMonster
{
public:
	void Spawn( void );
	int	Classify ( void ) { return	CLASS_PLAYER_ALLY; }

	void KeyValue( KeyValueData *pkvd );

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[3];
};

char *CDeadCt::m_szPoses[] = { "lying_on_back", "lying_on_side", "lying_on_stomach" };

void CDeadCt::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_ct_m4a1_ally_dead, CDeadCt );

//=========================================================
// ********** DeadBarney SPAWN **********
//=========================================================
void CDeadCt :: Spawn( )
{
	PRECACHE_MODEL("models/human_m4a1.mdl");
	SET_MODEL(ENT(pev), "models/human_m4a1.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if (pev->sequence == -1)
	{
		ALERT ( at_error, "Dead barney with bad pose\n" );
	}
	// Corpses have less health
	pev->health			= 8;//gSkillData.barneyHealth;

	MonsterInitDead();
}

//=========================================================
// SetActivity 
//=========================================================
void CMonsterCT :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
/*
#define MP5						1
#define SHOTGUN					2
#define LAW						3
#define AWP						4
#define ASSAULT					5 
#define PISTOL					6
#define KNIFE					7
#define M249					8			
#define SMG						9
*/
	switch ( NewActivity)
	{
	case ACT_RANGE_ATTACK1:

		if (pev->frags == MP5)
		{
			iSequence = LookupSequence( "standing_mp5" );
		}
		else if (pev->frags == SHOTGUN)
		{
			iSequence = LookupSequence( "standing_shotgun" );
		}
		else if (pev->frags == LAW)
		{
			iSequence = LookupSequence( "LAW_fire" );//FIX- standing_mp5
		}
		else if (pev->frags == AWP)
		{
			iSequence = LookupSequence( "sniper-fire" );
		}
		else if (pev->frags == ASSAULT)
		{
			iSequence = LookupSequence( "standing_mp5" );
		}
		else if (pev->frags == PISTOL)
		{
			iSequence = LookupSequence( "pistol_standing" );
		}
//		else if (pev->frags == KNIFE)
//		{
//		}
		else if (pev->frags == M249)
		{
			iSequence = LookupSequence( "machinegun_fire" );
		}
		else if (pev->frags == SMG)
		{
			iSequence = LookupSequence( "shootcycle_tmp" );
		}	
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}
	break;

	case ACT_RUN:
		if ( pev->health <= LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity ( ACT_RUN_HURT );
		}
		else
		{
			if (pev->frags == LAW)//law		
			{
				iSequence = LookupSequence( "LAW_run" );
			}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "pistol_run" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_run" );
			}
//			else if (pev->frags == KNIFE)
//			{
//				iSequence = LookupSequence( "machete_run" );
//			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
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
			if (pev->frags == LAW)//law		
			{
				iSequence = LookupSequence( "LAW_run" );
			}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "machete_patrol_street" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
				switch (RANDOM_LONG(0,1))
				{
					case 0: iSequence = LookupSequence( "pistol_patrol_street" ); break;
					case 1: iSequence = LookupSequence( "machete_patrol_street" ); break;
				}
			}
//			else if (pev->frags == KNIFE)
//			{
//				iSequence = LookupSequence( "machete_patrol_street" );
//			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}	
		}
		break;

	case ACT_VICTORY_DANCE: iSequence = LookupSequence( "pistol_victorydance" ); break;

	case ACT_RELOAD:
		if (pev->frags == LAW)//law		
		{
			iSequence = LookupSequence( "LAW_reload" );
		}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "pistol_reload" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
			iSequence = LookupSequence( "pistol_reload" );
		}
		else if (pev->frags == SHOTGUN)
		{
			iSequence = LookupSequence( "reload_shotgun" );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}	
 break;
	case ACT_IDLE:
		if (pev->frags == LAW)//law		
		{
			iSequence = LookupSequence( "LAW_fire_idle" );
		}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "pistol_idle1" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
			iSequence = LookupSequence( "pistol_idle1" );
		}
//		else if (pev->frags == KNIFE)
//		{
//			iSequence = LookupSequence( "machete_idle1" );
//		}
		else
		{
			switch (RANDOM_LONG(0,2))
			{
				case 0: iSequence = LookupSequence( "idle1" );break;
				case 1: iSequence = LookupSequence( "idle02" ); break;
				case 2: iSequence = LookupSequence( "idle03" ); break;
			}
		//	iSequence = LookupActivity ( ACT_IDLE );
		}

		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		//	iSequence = LookupActivity ( ACT_IDLE_ANGRY );//si hay problemas puede ser esto
		}
	//	iSequence = LookupActivity ( NewActivity );
		break;

	case ACT_IDLE_ANGRY:
			if (pev->frags == LAW)//law		
			{
				iSequence = LookupSequence( "LAW_fire_idle" );
			}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "pistol_combat_idle" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_combat_idle" );
			}
//			else if (pev->frags == KNIFE)
//			{
//				iSequence = LookupSequence( "machete_combat_idle" );
//			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}	
		break;

		case ACT_TURN_LEFT:
			if (pev->frags == LAW)//law		
			{
				iSequence = LookupSequence( "LAW_180L" );
			}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "pistol_180L" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_180L" );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}		
		break;

		case ACT_TURN_RIGHT:
			if (pev->frags == LAW)//law		
			{
				iSequence = LookupSequence( "LAW_180R" );
			}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "pistol_180R" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_180R" );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}			
		break;

	case ACT_FLINCH_HEAD:
	case ACT_FLINCH_CHEST:
	case ACT_FLINCH_STOMACH:
	case ACT_FLINCH_LEFTARM:
	case ACT_FLINCH_RIGHTARM:
	case ACT_FLINCH_LEFTLEG:
	case ACT_FLINCH_RIGHTLEG:
			if (pev->frags == LAW)//law		
			{
				iSequence = LookupSequence( "LAW_fire_idle" );
			}
			else if (pev->frags == SMG )//pistol
			{
				iSequence = LookupSequence( "pistol_smflinch" );
			}
			else if (pev->frags == PISTOL )//pistol
			{
				iSequence = LookupSequence( "pistol_smflinch" );
			}
//			else if (pev->frags == KNIFE)
//			{
//				iSequence = LookupSequence( "machete_smflinch" );
//			}
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
BOOL CMonsterCT :: CheckMeleeAttack1 ( float flDot, float flDist )
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
void CMonsterCT :: CheckAmmo ( void )
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
void CMonsterCT :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	// Si tiene arma ... y si NO tiene el flag de "no tirar arma"
	if ( GetBodygroup( 2 ) != 2 && !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
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
		
		pGun->pev->spawnflags |= SF_NORESPAWN;

		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}
/*	
		if (FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ))
		{
			pGun = DropItem( "ammo_ARgrenades", vecGunPos, vecGunAngles );
			if ( pGun )
			{
				pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
				pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			}
		}
*/
	}

	CBaseMonster :: GibMonster();
}

////////////////////////////////////
////////////////////////////////////
//		CUSTOM MONSTERS			  //
////////////////////////////////////
////////////////////////////////////

//-----------------------------------------------------------------------------
class CAllyMP5 : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_mp5, CAllyMP5 );//monster_ally_swat_mp5
// Spawn
void CAllyMP5 :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = MP5;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllyMP5 :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
class CAllyShotgun : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_shotgun, CAllyShotgun );//monster_ally_swat_mp5
// Spawn
void CAllyShotgun :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = SHOTGUN;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllyShotgun :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CAllyLAW : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_law, CAllyLAW );//monster_ally_swat_mp5
// Spawn
void CAllyLAW :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = LAW;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllyLAW :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CAllyAWP : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_sniper, CAllyAWP );//monster_ally_swat_mp5
// Spawn
void CAllyAWP :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = AWP;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllyAWP :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CAllyASSAULT : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_assault, CAllyASSAULT );//monster_ally_swat_mp5

//LINK_ENTITY_TO_CLASS(monster_ct_m4a1_ally, CAllyASSAULT );								//FIXFIX!!!!!!!!
//edit used a better class
// Spawn
void CAllyASSAULT :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = ASSAULT;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllyASSAULT :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CAllyPISTOL : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_pistol, CAllyPISTOL );//monster_ally_swat_mp5
// Spawn
void CAllyPISTOL :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = PISTOL;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllyPISTOL :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CAllyFIX : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_machete, CAllyFIX );//monster_ally_swat_mp5
// Spawn
void CAllyFIX :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

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
			case 7: pev->frags = SMG; break;
		}
	}
	ALERT (at_console, "Monster CT Ally Spawn as f%\n", pev->frags);

	CMonsterCT::Spawn();
}
//precache
void CAllyFIX :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
LINK_ENTITY_TO_CLASS(monster_ct_m4a1_ally, CAllyFIX );								//FIXFIX!!!!!!!!

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CAllyM249 : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_m249, CAllyM249 );//monster_ally_swat_mp5
// Spawn
void CAllyM249 :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = M249;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllyM249 :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CAllySMG : public CMonsterCT
{
public:
	void Spawn( );
	void Precache(void);
};
LINK_ENTITY_TO_CLASS( monster_ally_swat_smg, CAllySMG );//monster_ally_swat_mp5
// Spawn
void CAllySMG :: Spawn()
{
	//UNDONE: Use a CBASEmonster
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/swat.mdl");

	pev->frags = SMG;//set weapons

	CMonsterCT::Spawn();
}
//precache
void CAllySMG :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/swat.mdl");
	
	CMonsterCT :: Precache();
}
//-----------------------------------------------------------------------------
/*#define MP5						1
#define SHOTGUN					2
#define LAW						3
#define AWP						4
#define ASSAULT					5 
#define PISTOL					6
#define KNIFE					7
#define M249					8			
#define SMG						9
*/
