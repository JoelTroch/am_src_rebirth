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
#define		FBARNEY_AE_DRAW		( 21 )
#define		BARNEY_AE_SHOOT		( 3 )

#define		HGRUNT_AE_RELOAD		( 2 )
#define		HGRUNT_AE_KICK			( 3 )
#define		HGRUNT_AE_BURST1		( 4 )
#define		HGRUNT_AE_BURST2		( 5 ) 
#define		HGRUNT_AE_BURST3		( 6 ) 
#define		HGRUNT_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define		HGRUNT_AE_GREN_TOSS		( 7 )
#define		HGRUNT_AE_GREN_DROP		( 9 )

#define	BARNEY_BODY_GUNHOLSTERED	0
#define	BARNEY_BODY_GUNDRAWN		1
#define BARNEY_BODY_GUNGONE			2
#define	BARNEY_BODY_GUNDEAGLE		3
//#define	BARNEY_BODY_GUNFIX			4

#define	BARNEY_BODY_GUNFIX			4

#define HEAD_GROUP					1
	//0	helmet
	//1	commander
	//2	mask
	//3	flaco gorrito
	//4 negro 1
	//5 medico
	//6	otro
	//7 negro 2
	//8 no head
#define HEAD_GRUNT					0
#define HEAD_COMMANDER				1
#define HEAD_SHOTGUN				2
#define HEAD_M203					3
#define HEAD_GLASSES				4
#define HEAD_MASK					5
#define HEAD_HELMET					6
#define HEAD_FLASH					7
#define HEAD_NONE					8

#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_GRUNT_COVER_AND_RELOAD,
};

class CFriend : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	void BarneyFirePistol( void );
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

	
	float m_flNextGrenadeCheck;

	Vector	m_vecTossVelocity;

	BOOL	m_fThrowGrenade;

	//no grenades, no launcer...
	//BOOL CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
	BOOL CheckRangeAttack2 ( float flDot, float flDist );

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_hgrunt_ally, CFriend );
LINK_ENTITY_TO_CLASS( monster_hgrunt_friendly, CFriend );

TYPEDESCRIPTION	CFriend::m_SaveData[] = 
{
	DEFINE_FIELD( CFriend, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFriend, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CFriend, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CFriend, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFriend, m_cClipSize, FIELD_INTEGER ),// FIX!!
	DEFINE_FIELD( CFriend, m_vecTossVelocity, FIELD_VECTOR ),// FIX!!
	DEFINE_FIELD( CFriend, m_fThrowGrenade, FIELD_BOOLEAN ),// FIX!!
};

IMPLEMENT_SAVERESTORE( CFriend, CTalkMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t	tlFrFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slFrFollow[] =
{
	{
		tlFrFollow,
		ARRAYSIZE ( tlFrFollow ),
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
Task_t	tlFriendHideReload[] =
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

Schedule_t slFriendHideReload[] = 
{
	{
		tlFriendHideReload,
		ARRAYSIZE ( tlFriendHideReload ),
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
Task_t	tlFriendRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
};

Schedule_t	slFriendRangeAttack2[] =
{
	{ 
		tlFriendRangeAttack2,
		ARRAYSIZE ( tlFriendRangeAttack2 ), 
		0,
		0,
		"RangeAttack2"
	},
};

//=========================================================
// BarneyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlFrrneyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_SIGNAL1 },
};

Schedule_t slFrrneyEnemyDraw[] = 
{
	{
		tlFrrneyEnemyDraw,
		ARRAYSIZE ( tlFrrneyEnemyDraw ),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t	tlFrFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slFrFaceTarget[] =
{
	{
		tlFrFaceTarget,
		ARRAYSIZE ( tlFrFaceTarget ),
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


Task_t	tlIdleFrStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // reset head position
};

Schedule_t	slIdleFrStand[] =
{
	{ 
		tlIdleFrStand,
		ARRAYSIZE ( tlIdleFrStand ), 
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

DEFINE_CUSTOM_SCHEDULES( CFriend )
{
	slFrFollow,
	slFriendHideReload,
	slFrrneyEnemyDraw,
	slFrFaceTarget,
	slIdleFrStand,
	slFriendRangeAttack2,
};


IMPLEMENT_CUSTOM_SCHEDULES( CFriend, CTalkMonster );

void CFriend :: StartTask( Task_t *pTask )
{
	CTalkMonster::StartTask( pTask );	
}

void CFriend :: RunTask( Task_t *pTask )
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
int CFriend :: ISoundMask ( void) 
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
int	CFriend :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_PLAYER_ALLY;
}

//=========================================================
// ALertSound - barney says "Freeze!"
//=========================================================
void CFriend :: AlertSound( void )
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
void CFriend :: SetYawSpeed ( void )
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
BOOL CFriend :: CheckRangeAttack1 ( float flDot, float flDist )
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
void CFriend :: BarneyFirePistol ( void )
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
		FireBullets(8, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 1024, BULLET_PLAYER_BUCKSHOT);//357

		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/shotgun/sbarrel1.wav", 1, ATTN_NORM );
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

		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/m16/m16_fire-1.wav", 1, ATTN_NORM );
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
void CFriend :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 0, vecGunPos, vecGunAngles );

	switch( pEvent->event )
	{
	case HGRUNT_AE_BURST1:
		BarneyFirePistol();
		break;
//sys	
	case HGRUNT_AE_BURST2:
	case HGRUNT_AE_BURST3:
		BarneyFirePistol();
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

				// now spawn a gun.
				//if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
				if (pev->frags)
				{
					pItem = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
					pItem->pev->spawnflags |= SF_NORESPAWN; // No respawn
				}
				else
				{
					pItem = DropItem( "weapon_m16", vecGunPos, vecGunAngles );
					pItem->pev->spawnflags |= SF_NORESPAWN; // No respawn
				}
				
			//if -> crash, remove the following	
			//	pItem->pev->avelocity = Vector ( RANDOM_FLOAT( -222, 222 ), RANDOM_FLOAT( -222, 222 ),RANDOM_FLOAT( -222, 222 ) );
			}
		break;

	case HGRUNT_AE_RELOAD:
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );
		m_cAmmoLoaded = m_cClipSize;
		ClearConditions(bits_COND_NO_AMMO_LOADED);
				
//		DropItem( "item_clip_rifle", vecGunPos, vecGunAngles );//test
		break;

	case FBARNEY_AE_DRAW:
		if (pev->frags)//shotgun
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		else
		SetBodygroup( GUN_GROUP, GUN_MP5 );

		m_fGunDrawn = TRUE;
		break;
	
	case HGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );		
		
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
		break;
	
	case HGRUNT_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );
			// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
			
			CGrenade::ShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 3.5 );

			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		break;

	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CFriend :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
	SET_MODEL( ENT(pev),"physics/models/hgrunt_ally.mdl");
	else
	SET_MODEL( ENT(pev),"models/hgrunt_ally.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if (pev->health == 0) //LRC
		pev->health			= gSkillData.barneyHealth;
	pev->view_ofs		= Vector ( 0, 0, 50 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;

	m_fGunDrawn			= TRUE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	if (pev->frags == -1) //si no esta seteado la deagle
	{
		if (RANDOM_LONG( 0, 99 ) < 60) //
		{
			 pev->frags = 0;
//			 ALERT (at_console, "mp5\n");
		}
		else
		{
			pev->frags = 1;		
//			ALERT (at_console, "shotgun\n");
		}	
	}

	if (pev->frags)//shotgun
	{
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		m_cClipSize		= 8;
	}
	else
	{
		SetBodygroup( GUN_GROUP, GUN_MP5 );
		m_cClipSize		= 30;
	}
	m_cAmmoLoaded		= m_cClipSize;
	
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

	MonsterInit();
	SetUse(&CFriend :: FollowerUse );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CFriend :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else if ( CVAR_GET_FLOAT("cl_ragdoll" ) != 0 )	
		PRECACHE_MODEL("physics/models/hgrunt_ally.mdl");
		else
		PRECACHE_MODEL("models/hgrunt_ally.mdl");

	PRECACHE_SOUND( "hgrunt/gr_die1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die3.wav" );

	PRECACHE_SOUND( "hgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain5.wav" );

	PRECACHE_SOUND("hgrunt/gr_reload1.wav");
	
	TalkInit();
	CTalkMonster::Precache();
}	

// Init talk data
void CFriend :: TalkInit()
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


int CFriend :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
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

	return ret;
}

	
//=========================================================
// PainSound
//=========================================================
void CFriend :: PainSound ( void )
{
	if (gpGlobals->time < m_painTime)
		return;
	
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

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

//=========================================================
// DeathSound 
//=========================================================
void CFriend :: DeathSound ( void )
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


void CFriend::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
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

	CTalkMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


void CFriend::Killed( entvars_t *pevAttacker, int iGib )
{
//used a event

	SetUse( NULL );	
	CTalkMonster::Killed( pevAttacker, iGib );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Schedule_t* CFriend :: GetScheduleOfType ( int Type )
{
	Schedule_t *psched;

	switch( Type )
	{
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != NULL )
		{
			// face enemy, then draw.
			return slFrrneyEnemyDraw;
		}
		break;

	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney will talk
		// when 'used' 
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slFrFaceTarget;	// override this for different target face behavior
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slFrFollow;

	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slFriendHideReload[ 0 ];
		}
	case SCHED_RANGE_ATTACK2:
		{
			return &slFriendRangeAttack2[ 0 ];
		}

	case SCHED_IDLE_STAND:
		// call base class default so that scientist will talk
		// when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
		{
			// just look straight ahead.
			return slIdleFrStand;
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
Schedule_t *CFriend :: GetSchedule ( void )
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
				
			// wait for one schedule to draw gun
			if (!m_fGunDrawn )
				return GetScheduleOfType( SCHED_ARM_WEAPON );

			if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );

			// no ammo
			else if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_GRUNT_COVER_AND_RELOAD );
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

MONSTERSTATE CFriend :: GetIdealState ( void )
{
	return CTalkMonster::GetIdealState();
}



void CFriend::DeclineFollowing( void )
{
	if ( IsAlive() )
	PlaySentence( m_szGrp[TLK_DECLINE], 2, VOL_NORM, ATTN_NORM ); //LRC
}


//=========================================================
// SetActivity 
//=========================================================
void CFriend :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RANGE_ATTACK1:
					
//		if (RANDOM_LONG(0,1))
//		{
//			iSequence = LookupSequence( "throwgrenade" );
//		}
//		else
//		{
			if (pev->frags)
			{
				iSequence = LookupSequence( "standing_shotgun" );
			}
			else
			{
				iSequence = LookupSequence( "standing_mp5" );
			}
//		}
		break;

	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown 
		// grenade or fired grenade, we must determine which and pick proper sequence
		// get toss anim
		iSequence = LookupSequence( "throwgrenade" );
		break;

	case ACT_RUN:
		if ( pev->health <= LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity ( ACT_RUN_HURT );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
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
			iSequence = LookupActivity ( NewActivity );
		}
		break;
	case ACT_IDLE:
		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity ( NewActivity );
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
BOOL CFriend :: CheckMeleeAttack1 ( float flDot, float flDist )
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
void CFriend :: CheckAmmo ( void )
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
void CFriend :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	// Si tiene arma ... y si NO tiene el flag de "no tirar arma"
	if ( GetBodygroup( 2 ) != 2 && !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
	//	if (FBitSet( pev->weapons, HGRUNT_SHOTGUN ))
		if (pev->frags)
		{
			pGun = DropItem( "weapon_spas12", vecGunPos, vecGunAngles );
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

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CFriend :: CheckRangeAttack2 ( float flDot, float flDist )
{	
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