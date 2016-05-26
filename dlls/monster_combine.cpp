//Lastest file
//COMBINE MONSTER FROM HL2.
//THIS FILE ITS RELATED TO ALL COMBINES

//EDIT: added "barney draw gun" style to metrocop combine (oh, amazing!)

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

int g_fCombineQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int		g_iSkillLevel;

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	GRUNT_CLIP_SIZE					30 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define HGRUNT_LIMP_HEALTH				20

#define	HGRUNT_SENTENCE_VOLUME			(float)0.8
#define GRUNT_ATTN						ATTN_NONE	// attenutation of grunt sentences
#define	HGRUNT_MINIMUN_LIGHT			100// el valor de luz minimo permitido (a partir de ese valor no puede ver)

#define COMBINE_MP7					( 1 << 0)
#define COMBINE_HANDGRENADE			( 1 << 1)
//#define COMBINE_GRENADELAUNCHER		( 1 << 2)
#define COMBINE_SHOTGUN				( 1 << 3)
#define COMBINE_IRGUN				( 1 << 4)//test

#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2
#define GUN_IRIFLE					3

#define DOOR		1
#define LADDER		2
#define BREKABLE	3
#define	TRAIN		4		
#define	PLAT		5

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
#define		HGRUNT_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define		COMBINE_AE_SHOOT_BALL	( 15) 
//New
#define		COMBINE_AE_DRAW		( 22 )
#define		COMBINE_AE_HOLSTER	( 23 )//Holster? is this used?

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

class CCombine : public CSquadMonster
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

	void PainSound( void );
	void IdleSound ( void );
	Vector GetGunPosition( void );
	void Shoot ( void );
	void Shotgun ( void );
	void PrescheduleThink ( void );
	void GibMonster( void );
	void SpeakSentence( void );

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

	BOOL	b_mIsCombineMetroCop;//for animations and such
	BOOL	m_fGunDrawn;//test
	BOOL	IsCombineSuperSoldier;

	int		m_cClipSize;

	int m_voicePitch;

	int		m_iBrassShell;
	int		m_iShotgunShell;

	int		m_iSentence;

	static const char *pGruntSentences[];
};

LINK_ENTITY_TO_CLASS( monster_combine_metrocop, CCombine );//im using npc_combine_metrocop, so there is not problems
LINK_ENTITY_TO_CLASS( monster_combine_supersoldier, CCombine );
LINK_ENTITY_TO_CLASS( npc_combine_supersoldier, CCombine );

TYPEDESCRIPTION	CCombine::m_SaveData[] = 
{
	DEFINE_FIELD( CCombine, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CCombine, m_flNextPainTime, FIELD_TIME ),
	DEFINE_FIELD( CCombine, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CCombine, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CCombine, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CCombine, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CCombine, m_cClipSize, FIELD_INTEGER ),
	DEFINE_FIELD( CCombine, m_voicePitch, FIELD_INTEGER ),
	DEFINE_FIELD( CCombine, m_iSentence, FIELD_INTEGER ),

	DEFINE_FIELD( CCombine, m_fGunDrawn, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CCombine, CSquadMonster );

const char *CCombine::pGruntSentences[] = 
{
	"COMBINE_GREN", // grenade scared grunt
	"COMBINE_ALERT", // sees player
	"COMBINE_MONSTER", // sees monster
	"COMBINE_COVER", // running to cover
	"COMBINE_THROW", // about to throw grenade
	"COMBINE_CHARGE",  // running out to get the enemy
	"COMBINE_TAUNT", // say rude things

	"COMBINE_LIGHT", // when the lights go on
	"COMBINE_CHECK" //fix: this is used but there isn't here... why?
	"COMBINE_QUEST",
	"COMBINE_IDLE",
	"COMBINE_CLEAR",
	"COMBINE_ANSWER"
	//TO DO:
	//add new sentences instead use wav files?
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
	HGRUNT_SENT_ANSWER
} COMBINE_SENTENCE_TYPES;

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
void CCombine :: SpeakSentence( void )
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
int CCombine::IRelationship ( CBaseEntity *pTarget )
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
void CCombine :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	if ( GetBodygroup( 2 ) != 2 && !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;

		if (FBitSet( pev->weapons, COMBINE_IRGUN))
		{
			pGun = DropItem( "weapon_hl2_irgun", vecGunPos, vecGunAngles );
		}
		else if (FBitSet( pev->weapons, COMBINE_SHOTGUN))
		{
			if(b_mIsCombineMetroCop)
			pGun = DropItem( "weapon_hl2_pistol", vecGunPos, vecGunAngles );
			else
			pGun = DropItem( "weapon_hl2_shotgun", vecGunPos, vecGunAngles );
		}
		else
		{
			pGun = DropItem( "weapon_hl2_mp7", vecGunPos, vecGunAngles );
		}
	
		pGun->pev->spawnflags |= SF_NORESPAWN;

		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}
	}

	CBaseMonster :: GibMonster();
}

//=========================================================
// ISoundMask - Overidden for human grunts because they 
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CCombine :: ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER;
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CCombine :: FOkToSpeak( void )
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
void CCombine :: JustSpoke( void )
{
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = HGRUNT_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CCombine :: PrescheduleThink ( void )
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
BOOL CCombine :: FCanCheckAttacks ( void )
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
BOOL CCombine :: CheckMeleeAttack1 ( float flDot, float flDist )
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
BOOL CCombine :: CheckRangeAttack1 ( float flDot, float flDist )
{	
	//test
	//this could be work perfectly for metrocop monsters using stunbaton :)
//	if (flDist >= 256)
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

	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CCombine :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if (! FBitSet(pev->weapons, (COMBINE_HANDGRENADE | COMBINE_HANDGRENADE)))
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

	if (FBitSet( pev->weapons, COMBINE_HANDGRENADE))
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

		
	if (FBitSet( pev->weapons, COMBINE_HANDGRENADE))
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
void CCombine :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == HITGROUP_HEAD) // si el daño es en la cabeza
	{
		m_bloodColor = DONT_BLEED;

		switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar
		{
		case 0: EMIT_SOUND (ENT(pev), CHAN_BODY, "player/damage/helmet1.wav", 1, ATTN_NORM ); break;
		case 1: EMIT_SOUND (ENT(pev), CHAN_BODY, "player/damage/helmet2.wav", 1, ATTN_NORM ); break;
		case 2: EMIT_SOUND (ENT(pev), CHAN_BODY, "player/damage/helmet3.wav", 1, ATTN_NORM ); break;
		}

		UTIL_Ricochet( ptr->vecEndPos, 1.0 );
		UTIL_WhiteSparks( ptr->vecEndPos, vecDir, 0, 15, 500, 200 );//chispas
		UTIL_WhiteSparks( ptr->vecEndPos, vecDir, 9, 15, 5, 100 );//puntos
		UTIL_WhiteSparks( ptr->vecEndPos, vecDir, 0, 10, 500, 20 );//chispas
			
		if(IsCombineSuperSoldier)
			if ( bitsDamageType & DMG_SHOTGUN )
				pev->skin = 1;
	}
	else
	{
		m_bloodColor = BLOOD_COLOR_RED;

		switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar
		{
			case 0: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_hit1.wav", 1, ATTN_NORM ); break;
			case 1: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_hit2.wav", 1, ATTN_NORM ); break;
			case 2: EMIT_SOUND (ENT(pev), CHAN_BODY, "fvox/m_hit3.wav", 1, ATTN_NORM ); break;
		}
			
		if(IsCombineSuperSoldier)
			if ( bitsDamageType & DMG_SHOTGUN )
				pev->skin = 2;
	}

//	ALERT ( at_console, "Hitgroup is %i \n", ptr->iHitgroup );

	CSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CCombine :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	Forget( bits_MEMORY_INCOVER );

	return CSquadMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CCombine :: SetYawSpeed ( void )
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

void CCombine :: IdleSound( void )
{
	if (FOkToSpeak() && (g_fCombineQuestion || RANDOM_LONG(0,1)))
	{
		if (!g_fCombineQuestion)
		{
			CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

			if (pPlayer->b_PlayerIsNearOf == DOOR)
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "COMBINE/PUERTA.wav", 1.0, ATTN_NORM);
			else if (pPlayer->b_PlayerIsNearOf == LADDER)
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "COMBINE/ESCALERA.wav", 1.0, ATTN_NORM);
			else if (pPlayer->b_PlayerIsNearOf == BREKABLE)	
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "COMBINE/CAJA.wav", 1.0, ATTN_NORM);
			else if (pPlayer->b_PlayerIsNearOf == TRAIN)	
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "COMBINE/TREN.wav", 1.0, ATTN_NORM);
			else if (pPlayer->b_PlayerIsNearOf == PLAT)	
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "COMBINE/ELEVADOR.wav", 1.0, ATTN_NORM);
			else
			{
				// ask question or make statement
				switch (RANDOM_LONG(0,2))
				{
				case 0: // check in
					SENTENCEG_PlayRndSz(ENT(pev), "COMBINE_CHECK", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
					
					g_fCombineQuestion = 1;
					break;
				case 1: // question
					SENTENCEG_PlayRndSz(ENT(pev), "COMBINE_QUEST", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
					g_fCombineQuestion = 2;
					break;
				case 2: // statement
					SENTENCEG_PlayRndSz(ENT(pev), "COMBINE_IDLE", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
					break;
				}
			}
		}
		else
		{
			switch (g_fCombineQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "COMBINE_CLEAR", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "COMBINE_ANSWER", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fCombineQuestion = 0;
		}
		JustSpoke();
	}
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CCombine :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}
//=========================================================
// RunAI Tomado del cod de Assassin
//=========================================================
void CCombine :: RunAI( void )
{
	float m_flLastLightLevel = GETENTITYILLUM( ENT ( pev ) );// make this our new light level.

	if ( CVAR_GET_FLOAT("dev_light" ) != 0 )
	{
		pev->rendermode = kRenderNormal;

		int rendertestvalue = m_flLastLightLevel;

		if (rendertestvalue >= 255)
			rendertestvalue = 255;

		pev->renderamt = rendertestvalue;
		pev->renderfx = 19;

		if (m_flLastLightLevel <= 30)//40'
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

//#if CODE_TEST
	if ( IsAlive() )
	{			
		//low poly model
		//LOW DEF
		SetBodygroup( 0, 1 );//turn off left side
		SetBodygroup( 1, 2 );//change right side per low low low poly model
		SetBodygroup( 3, 0 );//turn off crome

		SetBodygroup( 4, 0 );//turn off lights

		CBaseEntity *pEntidadLODcheck2 = NULL;


		while ((pEntidadLODcheck2 = UTIL_FindEntityByClassname(pEntidadLODcheck2, "player")) != NULL) 
		{	
			float flDist = (pEntidadLODcheck2->Center() - pev->origin).Length();

			if ( flDist <= 512)
			{
				ALERT( at_console, "LOD -level 1-\n" );
				//MED DEF
				SetBodygroup( 1, 1 );//change right side per low poly model
			}
		}

		CBaseEntity *pEntidadLODcheck1 = NULL;
		
		while ((pEntidadLODcheck1 = UTIL_FindEntityByClassname(pEntidadLODcheck1, "player")) != NULL) 
		{	
			float flDist = (pEntidadLODcheck1->Center() - pev->origin).Length();

			if ( flDist <= 128)
			{
				//HIGH DEF
				ALERT( at_console, "No LOD -level 0-\n" );
					
				SetBodygroup( 0, 0 );//left side
				SetBodygroup( 1, 0 );//right side				
					
				SetBodygroup( 4, 1 );//turn on lights

				if (m_flLastLightLevel <= 40)//40'
					SetBodygroup( 3, 0 );
				else
					SetBodygroup( 3, 1 );//luz!
			}
		}
	}//is alive check
//#endif
	CBaseMonster :: RunAI();
}
//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CCombine :: Classify ( void )
{
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

//	ALERT( at_aiconsole, "pPlayer b_PlayerIsNearOf == %i\n", pPlayer->b_PlayerIsNearOf  );

	return m_iClass?m_iClass:CLASS_HUMAN_MILITARY;
}

//=========================================================
//=========================================================
CBaseEntity *CCombine :: Kick( void )
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

Vector CCombine :: GetGunPosition( )
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
void CCombine :: Shoot ( void )
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

	if (FBitSet( pev->weapons, COMBINE_MP7))
	{
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "combine/gun2.wav", 1, ATTN_NORM );

		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_6DEGREES, 4096, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
	}				
	else if (FBitSet( pev->weapons, COMBINE_IRGUN))
	{
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "combine/gun1.wav", 1, ATTN_NORM );

		FireIRGUNBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_3DEGREES, 9999, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
	}
	else
	{
		//well... what weapon?
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
		 
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	if (pPlayer->m_fSlowMotionOn)
	CBullet::Shoot( pev, vecShootOrigin, vecShootDir * 500 );
}

//=========================================================
// Shoot
//=========================================================
void CCombine :: Shotgun ( void )
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);

	if(b_mIsCombineMetroCop)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 2048, BULLET_PLAYER_GLOCK18 ); // shoot perfect

		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "combine/pistol.wav", 1, ATTN_NORM );

		pev->effects |= EF_MUZZLEFLASH;

		m_cAmmoLoaded--;// take away a bullet!

		Vector angDir = UTIL_VecToAngles( vecShootDir );
		SetBlending( 0, angDir.x );

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
	}
	else
	{
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 
		FireBullets(gSkillData.hgruntShotgunPellets * 2, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0 ); // shoot +-7.5 degrees
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/spas12/spas12-fire.wav", 1, ATTN_NORM );

		pev->effects |= EF_MUZZLEFLASH;

		m_cAmmoLoaded--;// take away a bullet!

		Vector angDir = UTIL_VecToAngles( vecShootDir );
		SetBlending( 0, angDir.x );

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			  WRITE_BYTE( TE_DLIGHT );
			  WRITE_COORD( vecShootOrigin.x ); // origin
			  WRITE_COORD( vecShootOrigin.y );
			  WRITE_COORD( vecShootOrigin.z );
			  WRITE_BYTE( 22 );     // radius
			  WRITE_BYTE( 255 );     // R
			  WRITE_BYTE( 255 );     // G
			  WRITE_BYTE( 128 );     // B
			  WRITE_BYTE( 0 );     // life * 10
			  WRITE_BYTE( 0 ); // decay
		 MESSAGE_END();
	}
}
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CCombine :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	switch( pEvent->event )
	{
		case COMBINE_AE_DRAW:
			SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
			m_fGunDrawn = TRUE;
		break;

		case COMBINE_AE_HOLSTER:
		//	pev->body = BARNEY_BODY_GUNHOLSTERED;
			m_fGunDrawn = FALSE;
		break;

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
			if (FBitSet( pev->weapons, COMBINE_MP7))
			{
				pItem = DropItem( "weapon_hl2_mp7", vecGunPos, vecGunAngles );
			}				
			else if (FBitSet( pev->weapons, COMBINE_IRGUN))
			{
				pItem = DropItem( "weapon_hl2_irgun", vecGunPos, vecGunAngles );
			}
			else
			{
				if(b_mIsCombineMetroCop)
				pItem = DropItem( "weapon_hl2_pistol", vecGunPos, vecGunAngles );
				else
				pItem = DropItem( "weapon_hl2_shotgun", vecGunPos, vecGunAngles );
			}

			pItem->pev->spawnflags |= SF_NORESPAWN; // No respawn

			//test		
			//pItem->pev->velocity = Vector (RANDOM_FLOAT(0,222), RANDOM_FLOAT(0,222), RANDOM_FLOAT(0,222));
			pItem->pev->avelocity = Vector ( RANDOM_FLOAT( -222, 222 ), RANDOM_FLOAT( -222, 222 ),RANDOM_FLOAT( -222, 222 ) );
			}
			break;

		case HGRUNT_AE_RELOAD:
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "combine/reload1.wav", 1, ATTN_NORM );
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
			break;

		case HGRUNT_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );

			CGrenade::ShootTimedCombine( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		break;

			
		case COMBINE_AE_SHOOT_BALL:
			UTIL_MakeVectors( pev->angles );

			//CGrenade::ShootSfera( pev, GetGunPosition()  + vecShootDir, m_vecTossVelocity, 10 );
			//CGrenade::ShootSfera( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 13.5 );

			CGrenade::ShootSfera( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), vecShootDir * 500 , 10 );

			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 1;// wait six seconds before even looking again to see if a grenade can be thrown.

			break;

		case HGRUNT_AE_GREN_LAUNCH:
		{
			/*
			CGrenade::ShootContact( pev, GetGunPosition(), m_vecTossVelocity );
		
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM);
			m_fThrowGrenade = FALSE;
			
			m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT( 2, 3 );// wait a random amount of time before shooting again
		*/
		}
		break;

		case HGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
					
			CGrenade::ShootTimedCombine( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );	
		}
		break;

		case HGRUNT_AE_BURST1:
		case HGRUNT_AE_BURST2:
		case HGRUNT_AE_BURST3:
		{
			if ( FBitSet( pev->weapons, COMBINE_MP7 ))
			{
				Shoot();
			}
			else if ( FBitSet( pev->weapons, COMBINE_IRGUN ))
			{
				Shoot();
			}
			else
			{
				Shotgun( );
			}
		
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		}
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
				SENTENCEG_PlayRndSz(ENT(pev), "COMBINE_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				JustSpoke();
			}

			if (b_mIsCombineMetroCop)//maybe this is not the right place
			{
				switch ( RANDOM_LONG(0,3) )
				{
				case 0:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-alert1.wav", 1, ATTN_IDLE );break;
				case 1:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-alert1.wav", 1, ATTN_IDLE );break;	
				case 2:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-alert1.wav", 1, ATTN_IDLE );break;	
				case 3:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-alert1.wav", 1, ATTN_IDLE );break;	
				}
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
void CCombine :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else
	SET_MODEL( ENT(pev),"models/hl2/combine_supersoldier.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	IsCombineSuperSoldier = TRUE;//fix

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP; //MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //200
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_STRAFE ; //jejeje

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if ( FStringNull( pev->weapons ))
	{
		switch (RANDOM_LONG(0,2))
		{
			case 0: pev->weapons = COMBINE_MP7 | COMBINE_HANDGRENADE;  break;
			case 1: pev->weapons = COMBINE_IRGUN | COMBINE_HANDGRENADE;  break;
			case 2: pev->weapons = COMBINE_SHOTGUN | COMBINE_HANDGRENADE;  break;
		}
	}

	if (FBitSet( pev->weapons, COMBINE_SHOTGUN ))
	{
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		m_cClipSize		= 8;
	}
	else if (FBitSet( pev->weapons, COMBINE_IRGUN ))
	{
		SetBodygroup( GUN_GROUP, GUN_IRIFLE );
		m_cClipSize	= 30;
	}
	else
	{
		SetBodygroup( GUN_GROUP, GUN_MP5 );
		m_cClipSize	= 30;	
	}

	m_cClipSize		= GRUNT_CLIP_SIZE;

	m_cAmmoLoaded		= m_cClipSize;
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CCombine :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else
	PRECACHE_MODEL("models/hl2/combine_supersoldier.mdl");

	PRECACHE_MODEL("models/weapons/w_ar2.mdl");
//	PRECACHE_MODEL("models/weapons/w_mp7.mdl");

	PRECACHE_MODEL("sprites/lgtning.spr");

	PRECACHE_MODEL("models/weapons/w_combine_he.mdl");

	PRECACHE_SOUND( "combine/gun1.wav" );

	PRECACHE_SOUND( "weapons/tick1.wav" );

	//TEST	
	
	PRECACHE_SOUND( "COMBINE/ESCALERA.wav" );
	PRECACHE_SOUND( "COMBINE/TREN.wav" );
	PRECACHE_SOUND( "COMBINE/CAJA.wav" );
	PRECACHE_SOUND( "COMBINE/ELEVADOR.wav" );

	PRECACHE_SOUND( "combine/gear1.wav" );
	PRECACHE_SOUND( "combine/gear2.wav" );
	PRECACHE_SOUND( "combine/gear3.wav" );

	PRECACHE_SOUND( "combine/die1.wav" );
	PRECACHE_SOUND( "combine/die2.wav" );
	PRECACHE_SOUND( "combine/die3.wav" );

	PRECACHE_SOUND( "combine/pain1.wav" );
	PRECACHE_SOUND( "combine/pain2.wav" );
	PRECACHE_SOUND( "combine/pain3.wav" );
	PRECACHE_SOUND( "combine/pain4.wav" );
	PRECACHE_SOUND( "combine/pain5.wav" );

	PRECACHE_SOUND( "combine/reload1.wav" );

	PRECACHE_SOUND( "weapons/glauncher.wav" );

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	//combine ball
	PRECACHE_SOUND ("weapons/energy_bounce1.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_bounce2.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_explode.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_shoot.wav");//grenade

	//SYS FIX: no sacar el pitch, sino disminuirlo. Un pitch elevado causa voz de maricon
	// get voice pitch
	if (RANDOM_LONG(0,1))//50% de posibilidad
		m_voicePitch = 95;//109 + 0 o 7, algo de eso, nah...
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");
}	

//=========================================================
// start task
//=========================================================
void CCombine :: StartTask ( Task_t *pTask )
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
//SP strafe test
/*
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
*/
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
void CCombine :: RunTask ( Task_t *pTask )
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
void CCombine :: PainSound ( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
		switch ( RANDOM_LONG(0,4) )
		{
		case 0:	
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/pain3.wav", 1, ATTN_NORM );	
			break;
		case 1:
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/pain4.wav", 1, ATTN_NORM );	
			break;
		case 2:
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/pain5.wav", 1, ATTN_NORM );	
			break;
		case 3:
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/pain1.wav", 1, ATTN_NORM );	
			break;
		case 4:
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/pain2.wav", 1, ATTN_NORM );	
			break;
		}

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CCombine :: DeathSound ( void )
{
	if(b_mIsCombineMetroCop)
	{
		switch ( RANDOM_LONG(0,3) )
		{
		case 0:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-die1.wav", 1, ATTN_IDLE );break;
		case 1:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-die2.wav", 1, ATTN_IDLE );break;	
		case 2:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-die3.wav", 1, ATTN_IDLE );break;	
		case 3:EMIT_SOUND( ENT(pev), CHAN_STATIC, "metropolice/sent-die4.wav", 1, ATTN_IDLE );break;	
		}
	}
	else
	{
		switch ( RANDOM_LONG(0,2) )
		{
		case 0:	
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/die1.wav", 1, ATTN_IDLE );	
			break;
		case 1:
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/die2.wav", 1, ATTN_IDLE );	
			break;
		case 2:
			EMIT_SOUND( ENT(pev), CHAN_STATIC, "combine/die3.wav", 1, ATTN_IDLE );	
			break;
		}
	}
	//hacer una funcion killed? nah
	SetBodygroup( 3, 0 );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// GruntFail
//=========================================================
Task_t	tlCombineFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slCombineFail[] =
{
	{
		tlCombineFail,
		ARRAYSIZE ( tlCombineFail ),
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
Task_t	tlCombineCombatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slCombineCombatFail[] =
{
	{
		tlCombineCombatFail,
		ARRAYSIZE ( tlCombineCombatFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Grunt Combat Fail"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t	tlCombineVictoryDance[] =
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

Schedule_t	slCombineVictoryDance[] =
{
	{ 
		tlCombineVictoryDance,
		ARRAYSIZE ( tlCombineVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"GruntVictoryDance"
	},
};

//=========================================================
// CombineEnemyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlCombineEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
};

Schedule_t slCombineEnemyDraw[] = 
{
	{
		tlCombineEnemyDraw,
		ARRAYSIZE ( tlCombineEnemyDraw ),
		0,
		0,
		"Combine Enemy Draw"
	}
};
//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlCombineEstablishLineOfFire[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_GRUNT_ELOF_FAIL	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0						},
	{ TASK_GRUNT_SPEAK_SENTENCE,(float)0						},
	{ TASK_RUN_PATH,			(float)0						},

//	{ TASK_STRAFE_PATH,			(float)0						}, //1
//SP test
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slCombineEstablishLineOfFire[] =
{
	{ 
		tlCombineEstablishLineOfFire,
		ARRAYSIZE ( tlCombineEstablishLineOfFire ),
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
Task_t	tlCombineFoundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL1			},
};

Schedule_t	slCombineFoundEnemy[] =
{
	{ 
		tlCombineFoundEnemy,
		ARRAYSIZE ( tlCombineFoundEnemy ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"GruntFoundEnemy"
	},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t	tlCombineCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1.5					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_SWEEP	},
};

Schedule_t	slCombineCombatFace[] =
{
	{ 
		tlCombineCombatFace1,
		ARRAYSIZE ( tlCombineCombatFace1 ), 
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
Task_t	tlCombineSignalSuppress[] =
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

Schedule_t	slCombineSignalSuppress[] =
{
	{ 
		tlCombineSignalSuppress,
		ARRAYSIZE ( tlCombineSignalSuppress ), 
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

Task_t	tlCombineSuppress[] =
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

Schedule_t	slCombineSuppress[] =
{
	{ 
		tlCombineSuppress,
		ARRAYSIZE ( tlCombineSuppress ), 
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
Task_t	tlCombineWaitInCover[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)1					},
};

Schedule_t	slCombineWaitInCover[] =
{
	{ 
		tlCombineWaitInCover,
		ARRAYSIZE ( tlCombineWaitInCover ), 
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
Task_t	tlCombineTakeCover1[] =
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

Schedule_t	slCombineTakeCover[] =
{
	{ 
		tlCombineTakeCover1,
		ARRAYSIZE ( tlCombineTakeCover1 ), 
		0,
		0,
		"TakeCover"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlCombineGrenadeCover1[] =
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

Schedule_t	slCombineGrenadeCover[] =
{
	{ 
		tlCombineGrenadeCover1,
		ARRAYSIZE ( tlCombineGrenadeCover1 ), 
		0,
		0,
		"GrenadeCover"
	},
};


//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlCombineTossGrenadeCover1[] =
{
	{ TASK_FACE_ENEMY,						(float)0							},
	{ TASK_RANGE_ATTACK2, 					(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t	slCombineTossGrenadeCover[] =
{
	{ 
		tlCombineTossGrenadeCover1,
		ARRAYSIZE ( tlCombineTossGrenadeCover1 ), 
		0,
		0,
		"TossGrenadeCover"
	},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t	tlCombineTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slCombineTakeCoverFromBestSound[] =
{
	{ 
		tlCombineTakeCoverFromBestSound,
		ARRAYSIZE ( tlCombineTakeCoverFromBestSound ), 
		0,
		0,
		"GruntTakeCoverFromBestSound"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlCombineHideReload[] =
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

Schedule_t slCombineHideReload[] = 
{
	{
		tlCombineHideReload,
		ARRAYSIZE ( tlCombineHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntHideReload"
	}
};

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t	tlCombineSweep[] =
{
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
};

Schedule_t	slCombineSweep[] =
{
	{ 
		tlCombineSweep,
		ARRAYSIZE ( tlCombineSweep ), 
		
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
Task_t	tlCombineRangeAttack1A[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_IDLE_ANGRY },//ACT_CROUCH//sea lo que sea no agacharse...
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

Schedule_t	slCombineRangeAttack1A[] =
{
	{ 
		tlCombineRangeAttack1A,
		ARRAYSIZE ( tlCombineRangeAttack1A ), 
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
Task_t	tlCombineRangeAttack1B[] =
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

Schedule_t	slCombineRangeAttack1B[] =
{
	{ 
		tlCombineRangeAttack1B,
		ARRAYSIZE ( tlCombineRangeAttack1B ), 
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
Task_t	tlCombineRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_GRUNT_FACE_TOSS_DIR,		(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_WAIT_FACE_ENEMY	},// don't run immediately after throwing grenade.
};

Schedule_t	slCombineRangeAttack2[] =
{
	{ 
		tlCombineRangeAttack2,
		ARRAYSIZE ( tlCombineRangeAttack2 ), 
		0,
		0,
		"RangeAttack2"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlCombineRepel[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_GLIDE 	},
};

Schedule_t	slCombineRepel[] =
{
	{ 
		tlCombineRepel,
		ARRAYSIZE ( tlCombineRepel ), 
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
Task_t	tlCombineRepelAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_FLY 	},
};

Schedule_t	slCombineRepelAttack[] =
{
	{ 
		tlCombineRepelAttack,
		ARRAYSIZE ( tlCombineRepelAttack ), 
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Repel Attack"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t	tlCombineRepelLand[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slCombineRepelLand[] =
{
	{ 
		tlCombineRepelLand,
		ARRAYSIZE ( tlCombineRepelLand ), 
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


DEFINE_CUSTOM_SCHEDULES( CCombine )
{
	slCombineFail,
	slCombineCombatFail,
	slCombineVictoryDance,
	slCombineEnemyDraw,
	slCombineEstablishLineOfFire,
	slCombineFoundEnemy,
	slCombineCombatFace,
	slCombineSignalSuppress,
	slCombineSuppress,
	slCombineWaitInCover,
	slCombineTakeCover,
	slCombineGrenadeCover,
	slCombineTossGrenadeCover,
	slCombineTakeCoverFromBestSound,
	slCombineHideReload,
	slCombineSweep,
	slCombineRangeAttack1A,
	slCombineRangeAttack1B,
	slCombineRangeAttack2,
	slCombineRepel,
	slCombineRepelAttack,
	slCombineRepelLand,
};

IMPLEMENT_CUSTOM_SCHEDULES( CCombine, CSquadMonster );

//=========================================================
// SetActivity 
//=========================================================
void CCombine :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );

	switch ( NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		if (FBitSet( pev->weapons, COMBINE_SHOTGUN))
		{
		//888
			if (b_mIsCombineMetroCop)
			{
				if ( m_fStanding )
				iSequence = LookupSequence( "standing_pistol" );
				else
				iSequence = LookupSequence( "lowcover_pistol" );
			}
			else
			{
				if ( m_fStanding )
				iSequence = LookupSequence( "standing_shotgun" );
				else
				iSequence = LookupSequence( "crouching_shotgun" );//crouching_shotgun
			}
		}
		else if (FBitSet( pev->weapons, COMBINE_IRGUN))
		{
			if ( m_fStanding )
			{
				// 90% chance of shoot bullets.
				// 10% chance of shoot ball.
				int iPercent = RANDOM_LONG(0,99);

				if ( iPercent <= 90 && m_hEnemy != NULL )
					iSequence = LookupSequence( "standing_irgun" );
				else
				{
					EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/energy_shoot.wav", 1, ATTN_NORM );
					iSequence = LookupSequence( "shootAR2alt" );
				}
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_irgun" );///crouching_mp5
			}
		}
		else//COMBINE_MP7
		{
			if (b_mIsCombineMetroCop)
			{
				if ( m_fStanding )
				iSequence = LookupSequence( "standing_smg" );
				else
				iSequence = LookupSequence( "lowcover_smg" );
			}
			else
			{
				if ( m_fStanding )
				iSequence = LookupSequence( "standing_mp7" );
				else
				iSequence = LookupSequence( "crouching_mp7" );///crouching_mp5
			}
		}
		break;
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown 
		// grenade or fired grenade, we must determine which and pick proper sequence
		if ( pev->weapons & COMBINE_HANDGRENADE )
		{
			// get toss anim
			iSequence = LookupSequence( "grenthrow" );//throwgrenade
		}
		else
		{
			// get launch anim
			iSequence = LookupSequence( "launchgrenade" );
		}
		break;
		
	case ACT_RELOAD:
		if (FBitSet( pev->weapons, COMBINE_SHOTGUN))
		{
			if (b_mIsCombineMetroCop)
			iSequence = LookupSequence( "reload_pistol" );
			else
			iSequence = LookupSequence( "reload_shotgun" );
		}
		else//COMBINE_MP7 || COMBINE_IRGUN
		{
			if (b_mIsCombineMetroCop)
			iSequence = LookupSequence( "reload_smg" );
			else
			iSequence = LookupSequence( "reload_mp5" );
		}
		break;

	case ACT_IDLE_ANGRY:
		if (FBitSet( pev->weapons, COMBINE_SHOTGUN))
		{
			if (b_mIsCombineMetroCop)
			{
				if ( m_fStanding )
				iSequence = LookupSequence( "pistolangryidle" );
				else
				iSequence = LookupSequence( "pistolangryidle_low" );
			}
			else
			{
				iSequence = LookupSequence( "CombatIdle_SG" );
			}
		}
		else if (FBitSet( pev->weapons, COMBINE_IRGUN))
		{
			iSequence = LookupSequence( "CombatIdle" );
		}
		else//COMBINE_MP7
		{
			if (b_mIsCombineMetroCop)
			{
				if ( m_fStanding )
				iSequence = LookupSequence( "smgangryidle" );
				else
				iSequence = LookupSequence( "smgangryidle_low" );
			}
			else
			{
				iSequence = LookupSequence( "CombatIdle_SMG1" );
			}
		}
		break;
/*
	case ACT_MELEE_ATTACK1:
		{
			if (pev->frags = 0)
			{
				switch ( RANDOM_LONG( 0, 1 ) )
				{
					case 0:	iSequence = LookupSequence( "punch" );	break;
					case 1:	iSequence = LookupSequence( "punch" );	break;
				}
			}
			iSequence = LookupActivity ( NewActivity );//ok, corrida normal
		}
		break;

	case ACT_RUN:
		if (pev->frags)//COMBINE
		{
			pev->framerate = 2;//mas rapido

			if ( pev->health <= HGRUNT_LIMP_HEALTH )
			{
				iSequence = LookupActivity ( ACT_RUN_HURT ); //codigo normal
			}
			else
			{
				iSequence = LookupSequence( "run" );
			}
		}
		else
		{
			if (FBitSet( pev->weapons, HGRUNT_9MMAR))
			{
				
			//	if (m_hEnemy == NULL)
			//	{
			//		iSequence = LookupActivity ( NewActivity );//ok, corrida normal
			//	}
			//	else
			//	{
			//		if ( m_MonsterState == MONSTERSTATE_COMBAT )
			//		{
			//			iSequence = LookupSequence( "plasma_run" );
			//		}
			//	}

				if (HasConditions ( bits_COND_SEE_ENEMY ))
					iSequence = LookupSequence( "plasma_run" );
				else
					iSequence = LookupActivity ( NewActivity );//ok, corrida normal
			}
			else
			{
				iSequence = LookupSequence( "run" );
			}
		}
		iSequence = LookupActivity ( NewActivity );//ok, corrida normal
		break;
	case ACT_WALK:
		if (pev->frags)
		{
			if ( pev->health <= HGRUNT_LIMP_HEALTH )
			{
				// limp!
				iSequence = LookupActivity ( ACT_WALK_HURT );
			}
		}
		else
		{
			iSequence = LookupSequence( "run" );
		}
		iSequence = LookupActivity ( NewActivity );

		break;
*/
	case ACT_IDLE:
		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}

		if (StartBurning)
		NewActivity = ACT_SNIFF;

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
// Get Schedule!
//=========================================================
Schedule_t *CCombine :: GetSchedule( void )
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
					SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_GREN", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
			
		//asa code
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

			//SysOp: This code make the monster will look for sounds. It's great.
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

	case MONSTERSTATE_COMBAT:
		{				
			if (!m_fGunDrawn )//test
			{
				if (b_mIsCombineMetroCop)
				{
					if (FBitSet( pev->weapons, COMBINE_SHOTGUN ))//pistol
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
								SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
							else if ((m_hEnemy != NULL) &&
									(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) && 
									(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) && 
									(m_hEnemy->Classify() != CLASS_MACHINE))
								// monster
								SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_MONSTER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);

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
				// 10% chance of taking cover
				// 90% chance of flinch.
				int iPercent = RANDOM_LONG(0,99);

				if ( iPercent <= 10 && m_hEnemy != NULL )//90
				{
					// only try to take cover if we actually have an enemy!

					//!!!KELLY - this grunt was hit and is going to run to cover.
					if (FOkToSpeak()) // && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_COVER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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

				if ( OccupySlot ( bits_SLOTS_COMBINE_ENGAGE ) )
				{
					// try to take an available ENGAGE slot
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_COMBINE_GRENADE ) )
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
				if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_COMBINE_GRENADE ) )
				{
					//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else if ( OccupySlot( bits_SLOTS_COMBINE_ENGAGE ) )
				{
					//!!!KELLY - grunt cannot see the enemy and has just decided to 
					// charge the enemy's position. 
					if (FOkToSpeak())// && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_CHARGE", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
					{
						SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_TAUNT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					
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
Schedule_t* CCombine :: GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{
		case SCHED_ARM_WEAPON://test
			if ( m_hEnemy != NULL )
			{
				// face enemy, then draw.
				return slCombineEnemyDraw;
			}

	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if ( InSquad() )
			{
				if ( g_iSkillLevel == SKILL_HARD && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_COMBINE_GRENADE ) )
				{
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz( ENT(pev), "COMBINE_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return slCombineTossGrenadeCover;
				}
				else
				{
					return &slCombineTakeCover[ 0 ];
				}
			}
			else
			{
				if ( RANDOM_LONG(0,1) )
				{
					return &slCombineTakeCover[ 0 ];
				}
				else
				{
					return &slCombineGrenadeCover[ 0 ];
				}
			}
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slCombineTakeCoverFromBestSound[ 0 ];
		}
	case SCHED_GRUNT_TAKECOVER_FAILED:
		{
			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_COMBINE_ENGAGE ) )
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
			return &slCombineEstablishLineOfFire[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK1:
		{
			// randomly stand or crouch
			
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
				return &slCombineRangeAttack1B[ 0 ];
		//	else
		//		return &slCombineRangeAttack1A[ 0 ];
		}
	case SCHED_RANGE_ATTACK2:
		{
			return &slCombineRangeAttack2[ 0 ];
		}
	case SCHED_COMBAT_FACE:
		{
			return &slCombineCombatFace[ 0 ];
		}
	case SCHED_GRUNT_WAIT_FACE_ENEMY:
		{
			return &slCombineWaitInCover[ 0 ];
		}
	case SCHED_GRUNT_SWEEP:
		{
			return &slCombineSweep[ 0 ];
		}
	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slCombineHideReload[ 0 ];
		}
	case SCHED_GRUNT_FOUND_ENEMY:
		{
			return &slCombineFoundEnemy[ 0 ];
		}
	case SCHED_VICTORY_DANCE:
		{
			if ( InSquad() )
			{
				if ( !IsLeader() )
				{
					return &slCombineFail[ 0 ];
				}
			}

			return &slCombineVictoryDance[ 0 ];
		}
	case SCHED_GRUNT_SUPPRESS:
		{
			if ( m_hEnemy->IsPlayer() && m_fFirstEncounter )
			{
				m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slCombineSignalSuppress[ 0 ];
			}
			else
			{
				return &slCombineSuppress[ 0 ];
			}
		}
	case SCHED_FAIL:
		{
			if ( m_hEnemy != NULL )
			{
				// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
				return &slCombineCombatFail[ 0 ];
			}

			return &slCombineFail[ 0 ];
		}
	case SCHED_GRUNT_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slCombineRepel[ 0 ];
		}
	case SCHED_GRUNT_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slCombineRepelAttack[ 0 ];
		}
	case SCHED_GRUNT_REPEL_LAND:
		{
			return &slCombineRepelLand[ 0 ];
		}
	default:
		{
			return CSquadMonster :: GetScheduleOfType ( Type );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCombineCustom : public CCombine
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_combine, CCombineCustom );
LINK_ENTITY_TO_CLASS( npc_combine, CCombineCustom );

void CCombineCustom :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );

	else
	PRECACHE_MODEL("models/hl2/combine.mdl");

	PRECACHE_MODEL("models/weapons/w_ar2.mdl");
//	PRECACHE_MODEL("models/weapons/w_mp7.mdl");

	PRECACHE_SOUND( "combine/gun1.wav" );
	PRECACHE_SOUND( "combine/gun2.wav" );

	PRECACHE_SOUND( "weapons/tick1.wav" );

	PRECACHE_SOUND( "combine/gear1.wav" );
	PRECACHE_SOUND( "combine/gear2.wav" );
	PRECACHE_SOUND( "combine/gear3.wav" );

	PRECACHE_MODEL("sprites/lgtning.spr");

	PRECACHE_MODEL("models/weapons/w_combine_he.mdl");

	PRECACHE_SOUND( "combine/die1.wav" );
	PRECACHE_SOUND( "combine/die2.wav" );
	PRECACHE_SOUND( "combine/die3.wav" );

	PRECACHE_SOUND( "combine/pain1.wav" );
	PRECACHE_SOUND( "combine/pain2.wav" );
	PRECACHE_SOUND( "combine/pain3.wav" );
	PRECACHE_SOUND( "combine/pain4.wav" );
	PRECACHE_SOUND( "combine/pain5.wav" );

	PRECACHE_SOUND( "COMBINE/ESCALERA.wav" );
	PRECACHE_SOUND( "COMBINE/TREN.wav" );
	PRECACHE_SOUND( "COMBINE/CAJA.wav" );
	PRECACHE_SOUND( "COMBINE/ELEVADOR.wav" );

	PRECACHE_SOUND( "combine/reload1.wav" );

	PRECACHE_SOUND( "weapons/glauncher.wav" );

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	//combine ball
	PRECACHE_SOUND ("weapons/energy_bounce1.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_bounce2.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_explode.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_shoot.wav");//grenade

	//SYS FIX: no sacar el pitch, sino disminuirlo. Un pitch elevado causa voz de maricon
	// get voice pitch
	if (RANDOM_LONG(0,1))//50% de posibilidad
		m_voicePitch = 95;//109 + 0 o 7, algo de eso, nah...
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");
}	

void CCombineCustom :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else
	SET_MODEL( ENT(pev),"models/hl2/combine.mdl");

	pev->frags = 1;

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP; //MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //200
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_STRAFE ; //jejeje

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if ( FStringNull( pev->weapons ))
	{
		switch (RANDOM_LONG(0,2))
		{
			case 0: pev->weapons = COMBINE_MP7 | COMBINE_HANDGRENADE;  break;
			case 1: pev->weapons = COMBINE_IRGUN | COMBINE_HANDGRENADE;  break;
			case 2: pev->weapons = COMBINE_SHOTGUN | COMBINE_HANDGRENADE;  break;
		}
	}

	if (FBitSet( pev->weapons, COMBINE_SHOTGUN ))
	{
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		m_cClipSize		= 8;
	}
	else if (FBitSet( pev->weapons, COMBINE_IRGUN ))
	{
		SetBodygroup( GUN_GROUP, GUN_IRIFLE );
		m_cClipSize	= 30;
	}
	else
	{
		SetBodygroup( GUN_GROUP, GUN_MP5 );
		m_cClipSize	= 30;	
	}


	m_cAmmoLoaded		= m_cClipSize;
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCombineRandom : public CCombine
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( monster_combine_random, CCombineRandom );
LINK_ENTITY_TO_CLASS( npc_combine_random, CCombineRandom );

void CCombineRandom :: Precache()
{
	PRECACHE_MODEL("models/hl2/combine.mdl");
	PRECACHE_MODEL("models/hl2/combine_supersoldier.mdl");
	PRECACHE_MODEL("models/hl2/combine_metrocop.mdl");

	PRECACHE_MODEL("models/weapons/w_ar2.mdl");
//	PRECACHE_MODEL("models/weapons/w_mp7.mdl");

	PRECACHE_MODEL("sprites/lgtning.spr");

	PRECACHE_MODEL("models/weapons/w_combine_he.mdl");

	PRECACHE_SOUND( "combine/gun1.wav" );
	PRECACHE_SOUND( "combine/gun2.wav" );

	//metrocop custom
	PRECACHE_SOUND( "combine/pistol.wav" );
	PRECACHE_SOUND( "metropolice/sent-die1.wav" );
	PRECACHE_SOUND( "metropolice/sent-die2.wav" );
	PRECACHE_SOUND( "metropolice/sent-die3.wav" );
	PRECACHE_SOUND( "metropolice/sent-die4.wav" );

	PRECACHE_SOUND( "metropolice/sent-alert1.wav" );
	PRECACHE_SOUND( "metropolice/sent-alert2.wav" );
	PRECACHE_SOUND( "metropolice/sent-alert3.wav" );

	PRECACHE_SOUND( "weapons/tick1.wav" );

	PRECACHE_SOUND( "combine/gear1.wav" );
	PRECACHE_SOUND( "combine/gear2.wav" );
	PRECACHE_SOUND( "combine/gear3.wav" );

	PRECACHE_SOUND( "combine/die1.wav" );
	PRECACHE_SOUND( "combine/die2.wav" );
	PRECACHE_SOUND( "combine/die3.wav" );

	PRECACHE_SOUND( "combine/pain1.wav" );
	PRECACHE_SOUND( "combine/pain2.wav" );
	PRECACHE_SOUND( "combine/pain3.wav" );
	PRECACHE_SOUND( "combine/pain4.wav" );
	PRECACHE_SOUND( "combine/pain5.wav" );

	PRECACHE_SOUND( "combine/reload1.wav" );

	PRECACHE_SOUND( "COMBINE/ESCALERA.wav" );
	PRECACHE_SOUND( "COMBINE/TREN.wav" );
	PRECACHE_SOUND( "COMBINE/CAJA.wav" );
	PRECACHE_SOUND( "COMBINE/ELEVADOR.wav" );

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event
	
	//combine ball
	PRECACHE_SOUND ("weapons/energy_bounce1.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_bounce2.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_explode.wav");//grenade
	PRECACHE_SOUND ("weapons/energy_shoot.wav");//grenade

	if (RANDOM_LONG(0,1))//50% de posibilidad
		m_voicePitch = 95;//109 + 0 o 7, algo de eso, nah...
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");
}	

void CCombineRandom :: Spawn()
{
	Precache( );

	switch( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	SET_MODEL( ENT(pev),"models/hl2/combine.mdl"); break;
		case 1:	SET_MODEL( ENT(pev),"models/hl2/combine_supersoldier.mdl");
				IsCombineSuperSoldier = TRUE; break;
		case 2:	SET_MODEL( ENT(pev),"models/hl2/combine_metrocop.mdl");
				b_mIsCombineMetroCop= TRUE; break;
	}

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP; //MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //200
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_STRAFE ; //jejeje

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if (b_mIsCombineMetroCop)
	{
		if ( FStringNull( pev->weapons ))
		{
			switch (RANDOM_LONG(0,1))
			{
				case 0: pev->weapons = COMBINE_MP7 | COMBINE_HANDGRENADE;  break;
				case 1: pev->weapons = COMBINE_SHOTGUN;  break;
			}
		}
		//sys: Si tiene pistola sacar el bodygroup para despues activarlo desde el evento
		//este es el ultimo pase de codigo
		if (FBitSet( pev->weapons, COMBINE_SHOTGUN ))//pistol
		{
		//	SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
			SetBodygroup( GUN_GROUP, GUN_NONE );
			m_cClipSize		= 20;
		}
		else
		{
			SetBodygroup( GUN_GROUP, GUN_MP5 );//mp7
			m_cClipSize	= 30;	
		}
	}
	else
	{
		if ( FStringNull( pev->weapons ))
		{
			switch (RANDOM_LONG(0,2))
			{
				case 0: pev->weapons = COMBINE_MP7 | COMBINE_HANDGRENADE;  break;
				case 1: pev->weapons = COMBINE_IRGUN | COMBINE_HANDGRENADE;  break;
				case 2: pev->weapons = COMBINE_SHOTGUN | COMBINE_HANDGRENADE;  break;
			}
		}
		if (FBitSet( pev->weapons, COMBINE_SHOTGUN ))
		{
			SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
			m_cClipSize		= 8;
		}
		else if (FBitSet( pev->weapons, COMBINE_IRGUN ))
		{
			SetBodygroup( GUN_GROUP, GUN_IRIFLE );
			m_cClipSize	= 30;
		}
		else
		{
			SetBodygroup( GUN_GROUP, GUN_MP5 );
			m_cClipSize	= 30;	
		}
	}

	m_cAmmoLoaded		= m_cClipSize;
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class CCombineMetroCop: public CCombine
{
public:
	void Spawn( );
	void Precache(void);
};

LINK_ENTITY_TO_CLASS( npc_combine_metrocop, CCombineMetroCop );

//=========================================================
// Spawn
//=========================================================
void CCombineMetroCop :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else
	SET_MODEL( ENT(pev),"models/hl2/combine_metrocop.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	if (pev->health == 0)
		pev->health			= gSkillData.hgruntHealth; //200
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP /*| bits_CAP_STRAFE */; //jejeje

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	b_mIsCombineMetroCop= TRUE;// this monster is a combine Metrocop

	m_fGunDrawn			= FALSE;//by defalut this boolean value is FALSE

	m_HackedGunPos = Vector ( 0, 0, 55 );

	if ( FStringNull( pev->weapons ))
	{
		switch (RANDOM_LONG(0,1))
		{
			case 0: pev->weapons = COMBINE_MP7 | COMBINE_HANDGRENADE;  break;
			case 1: pev->weapons = COMBINE_SHOTGUN | COMBINE_HANDGRENADE;  break;
		}
	}
//sys: Si tiene pistola sacar el bodygroup para despues activarlo desde el evento
	//este es el ultimo pase de codigo
	if (FBitSet( pev->weapons, COMBINE_SHOTGUN ))//pistol
	{
	//	SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		SetBodygroup( GUN_GROUP, GUN_NONE );
		m_cClipSize		= 20;
	}
	else
	{
		SetBodygroup( GUN_GROUP, GUN_MP5 );//mp7
		m_cClipSize	= 30;	
	}

	m_cClipSize		= GRUNT_CLIP_SIZE;

	m_cAmmoLoaded		= m_cClipSize;
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CCombineMetroCop :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else
	PRECACHE_MODEL("models/hl2/combine_metrocop.mdl");

	PRECACHE_MODEL("models/weapons/w_combine_he.mdl");
	PRECACHE_SOUND( "combine/gun1.wav" );

	PRECACHE_SOUND( "combine/pistol.wav" );
	PRECACHE_SOUND( "metropolice/sent-die1.wav" );
	PRECACHE_SOUND( "metropolice/sent-die2.wav" );
	PRECACHE_SOUND( "metropolice/sent-die3.wav" );
	PRECACHE_SOUND( "metropolice/sent-die4.wav" );

	PRECACHE_SOUND( "metropolice/sent-alert1.wav" );
	PRECACHE_SOUND( "metropolice/sent-alert2.wav" );
	PRECACHE_SOUND( "metropolice/sent-alert3.wav" );

//	PRECACHE_SOUND( "COMBINE/ESCALERA.wav" );
//	PRECACHE_SOUND( "COMBINE/TREN.wav" );
//	PRECACHE_SOUND( "COMBINE/CAJA.wav" );
//	PRECACHE_SOUND( "COMBINE/ELEVADOR.wav" );

	PRECACHE_SOUND( "weapons/tick1.wav" );
	PRECACHE_SOUND( "combine/gear1.wav" );
	PRECACHE_SOUND( "combine/gear2.wav" );
	PRECACHE_SOUND( "combine/gear3.wav" );

	PRECACHE_SOUND( "combine/pain1.wav" );
	PRECACHE_SOUND( "combine/pain2.wav" );
	PRECACHE_SOUND( "combine/pain3.wav" );
	PRECACHE_SOUND( "combine/pain4.wav" );
	PRECACHE_SOUND( "combine/pain5.wav" );
	PRECACHE_SOUND( "combine/reload1.wav" );
	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event
	if (RANDOM_LONG(0,1))//50% de posibilidad
		m_voicePitch = 95;//109 + 0 o 7, algo de eso, nah...
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
}	
