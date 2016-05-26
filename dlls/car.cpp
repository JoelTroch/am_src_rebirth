#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"nodes.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"customentity.h"
#include	"weapons.h"
#include	"effects.h"
#include	"soundent.h"
#include	"decals.h"
#include	"explode.h"
#include	"func_break.h"
#include	"squadmonster.h"
#include	"talkmonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	BLOATER_AE_ATTACK_MELEE1		0x01

#define GARG_GIB_MODEL				"models/computergibs.mdl"
int gGargGibModel3 = 0;
void SpawnExplosion3( Vector center, float randomRange, float time, int magnitude );

class CCar : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSnd( void );

	void EXPORT HuntThink( void );
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	void DeathEffect( void );

	// No range attacks
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) { return FALSE; }
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	float m_flDie;
	Vector m_vecTarget;
	float m_flNextHunt;
	float m_flNextHit;
	Vector m_posPrev;
	EHANDLE m_hOwner;
	int  m_iMyClass;

	virtual int		Save( CSave &save ); 
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
};

LINK_ENTITY_TO_CLASS( monster_car, CCar );

TYPEDESCRIPTION	CCar::m_SaveData[] = 
{
	DEFINE_FIELD( CCar, m_flNextHunt, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CCar, CBaseMonster );
//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CCar :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CCar :: SetYawSpeed ( void )
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

int CCar :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
//	PainSound();
	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CCar :: PainSound( void )
{

}

void CCar :: AlertSound( void )
{

}

void CCar :: IdleSound( void )
{

}

void CCar :: AttackSnd( void )
{

}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CCar :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	/*
	switch( pEvent->event )
	{
		case BLOATER_AE_ATTACK_MELEE1:
		{
			// do stuff for this event.
			AttackSnd();
		}
		break;

		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
	*/
}

//=========================================================
// Spawn
//=========================================================
void CCar :: Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/truck.mdl");
	UTIL_SetSize( pev, Vector( -32, -32, -32 ), Vector( 32, 32, 32 ) );
		
	UTIL_SetOrigin( pev, pev->origin );

//SOLID_BBOX
	pev->solid			= SOLID_BBOX;
	pev->movetype		= MOVETYPE_BOUNCE;
	m_bloodColor		= DONT_BLEED;
	pev->health			= 400;
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	
	pev->gravity		= 0.5;
	pev->friction		= 0.5;

	SetThink( HuntThink );
	pev->nextthink = gpGlobals->time + 0.1;
	m_flNextHunt = gpGlobals->time + 1E6;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CCar :: Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/truck.mdl");

	PRECACHE_MODEL( "models/computergibs.mdl" );
}	

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

void CCar :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	//*********************************************************
	case TASK_DIE:
		{
			if ( gpGlobals->time > m_flWaitFinished )
			{
				pev->renderfx = kRenderFxExplode;
				pev->rendercolor.x = 255;
				pev->rendercolor.y = 0;
				pev->rendercolor.z = 0;
				StopAnimation();
				pev->nextthink = gpGlobals->time + 0.15;
				SetThink( SUB_Remove );
				int i;
				int parts = MODEL_FRAMES( gGargGibModel3 );
				for ( i = 0; i < 10; i++ )
				{
					CGib *pGib = GetClassPtr( (CGib *)NULL );

					pGib->Spawn( GARG_GIB_MODEL );
					
					int bodyPart = 0;
					if ( parts > 1 )
						bodyPart = RANDOM_LONG( 0, pev->body-1 );

					pGib->pev->body = bodyPart;
					pGib->m_bloodColor = BLOOD_COLOR_YELLOW;
					pGib->m_material = matNone;
					pGib->pev->origin = pev->origin;
					pGib->pev->velocity = UTIL_RandomBloodVector() * RANDOM_FLOAT( 300, 500 );
					pGib->pev->nextthink = gpGlobals->time + 1.25;
					pGib->SetThink( SUB_FadeOut );
				}
				MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
					WRITE_BYTE( TE_BREAKMODEL);

					// position
					WRITE_COORD( pev->origin.x );
					WRITE_COORD( pev->origin.y );
					WRITE_COORD( pev->origin.z );

					// size
					WRITE_COORD( 200 );
					WRITE_COORD( 200 );
					WRITE_COORD( 128 );

					// velocity
					WRITE_COORD( 0 ); 
					WRITE_COORD( 0 );
					WRITE_COORD( 0 );

					// randomization
					WRITE_BYTE( 200 ); 

					// Model
					WRITE_SHORT( gGargGibModel3 );	//model id#

					// # of shards
					WRITE_BYTE( 50 );

					// duration
					WRITE_BYTE( 20 );// 3.0 seconds

					// flags

					WRITE_BYTE( BREAK_FLESH );
				MESSAGE_END();

				return;
			}
		}
	default:
		{
			CBaseMonster :: RunTask( pTask );
			break;
		}
	}
}


void CCar :: StartTask ( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
		
	case TASK_DIE:
		m_flWaitFinished = gpGlobals->time + 1.6;
		DeathEffect();
	break;

	default: 
		CBaseMonster :: StartTask( pTask );
		break;
	}
}
// HACKHACK Cut and pasted from explode.cpp
void SpawnExplosion3( Vector center, float randomRange, float time, int magnitude )
{
	KeyValueData	kvd;
	char			buf[128];

	center.x += RANDOM_FLOAT( -randomRange, randomRange );
	center.y += RANDOM_FLOAT( -randomRange, randomRange );

	CBaseEntity *pExplosion = CBaseEntity::Create( "env_explosion", center, g_vecZero, NULL );
	sprintf( buf, "%3d", magnitude );
	kvd.szKeyName = "iMagnitude";
	kvd.szValue = buf;
	pExplosion->KeyValue( &kvd );
	pExplosion->pev->spawnflags |= SF_ENVEXPLOSION_NODAMAGE;

	pExplosion->Spawn();
	pExplosion->SetThink( CBaseEntity::SUB_CallUseToggle );
	pExplosion->pev->nextthink = gpGlobals->time + time;
}
void CCar::DeathEffect( void )
{
	int i;
	UTIL_MakeVectors(pev->angles);
	Vector deathPos = pev->origin + gpGlobals->v_forward * 100;

	// Create a spiral of streaks
//	CSpiral::Create( deathPos, (pev->absmax.z - pev->absmin.z) * 0.6, 125, 1.5 );

	Vector position = pev->origin;
	position.z += 32;
	for ( i = 0; i < 7; i+=2 )
	{
		SpawnExplosion3( position, 70, (i * 0.3), 60 + (i*20) );
		position.z += 15;
	}

	CBaseEntity *pSmoker = CBaseEntity::Create( "env_smoker", pev->origin, g_vecZero, NULL );
	pSmoker->pev->health = 1;	// 1 smoke balls
	pSmoker->pev->scale = 46;	// 4.6X normal size
	pSmoker->pev->dmg = 0;		// 0 radial distribution
	pSmoker->pev->nextthink = gpGlobals->time + 2.5;	// Start in 2.5 seconds
}

void CCar::HuntThink( void )
{
	// ALERT( at_console, "think\n" );

	if (!IsInWorld())
	{
		SetTouch( NULL );
		UTIL_Remove( this );
		return;
	}
	
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	// explode when ready
	if (gpGlobals->time >= m_flDie)
	{
		g_vecAttackDir = pev->velocity.Normalize( );
		pev->health = -1;
		Killed( pev, 0 );
		return;
	}

	// float
	if (pev->waterlevel != 0)
	{
		if (pev->movetype == MOVETYPE_BOUNCE)
		{
			pev->movetype = MOVETYPE_FLY;
		}
		pev->velocity = pev->velocity * 0.9;
		pev->velocity.z += 8.0;
	}
	else if (pev->movetype = MOVETYPE_FLY)
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}

	// return if not time to hunt
	if (m_flNextHunt > gpGlobals->time)
		return;

	m_flNextHunt = gpGlobals->time + 2.0;
	
	CBaseEntity *pOther = NULL;
	Vector vecDir;
	TraceResult tr;

	Vector vecFlat = pev->velocity;
	vecFlat.z = 0;
	vecFlat = vecFlat.Normalize( );

	UTIL_MakeVectors( pev->angles );

	if (m_hEnemy == NULL || !m_hEnemy->IsAlive())
	{
		// find target, bounce a bit towards it.
		Look( 512 );
		m_hEnemy = BestVisibleEnemy( );
	}

	if (m_hEnemy != NULL)
	{
		if (FVisible( m_hEnemy ))
		{
			vecDir = m_hEnemy->EyePosition() - pev->origin;
			m_vecTarget = vecDir.Normalize( );
		}

		float flVel = pev->velocity.Length();
		float flAdj = 50.0 / (flVel + 10.0);

		if (flAdj > 1.2)
			flAdj = 1.2;
		
		// ALERT( at_console, "think : enemy\n");

		// ALERT( at_console, "%.0f %.2f %.2f %.2f\n", flVel, m_vecTarget.x, m_vecTarget.y, m_vecTarget.z );

		pev->velocity = pev->velocity * flAdj + m_vecTarget * 300;
	}

	if (pev->flags & FL_ONGROUND)
	{
		pev->avelocity = Vector( 0, 0, 0 );
	}
	else
	{
		if (pev->avelocity == Vector( 0, 0, 0))
		{
			pev->avelocity.x = RANDOM_FLOAT( -100, 100 );
			pev->avelocity.z = RANDOM_FLOAT( -100, 100 );
		}
	}

	if ((pev->origin - m_posPrev).Length() < 1.0)
	{
		pev->velocity.x = RANDOM_FLOAT( -100, 100 );
		pev->velocity.y = RANDOM_FLOAT( -100, 100 );
	}
	m_posPrev = pev->origin;

	pev->angles = UTIL_VecToAngles( pev->velocity );
	pev->angles.z = 0;
	pev->angles.x = 0;
}
