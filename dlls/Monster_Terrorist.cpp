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

// This is a new bastard that can fire 8 different weapons, drop secondary weapons, drop SG, HG and FB
// grenades, detect the player's position, look for player sounds, go to kamikaze, surrender, say
// rude things so often, ermm... Well, a lot...
//
// I use temporaly a beta model from CS Source. I will change it, so don't mail me askin'me "hey, why do you
// use a CS S model?" (we are still looking for good modellers)

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

#include	"weapons.h"//m_iDefaultAmmo

int g_fHAkQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int		g_iSkillLevel;
extern void ExplosionCreate( const Vector &center, const Vector &angles, edict_t *pOwner, int magnitude, BOOL doDamage );

//=========================================================
// Terrorist -specific DEFINE's
//=========================================================
//UNDONE: make radius more realistic (for each voice type)

#define	PLAYER_MINIMUN_LIGHT			10// el valor de luz minimo permitido (a partir de ese valor para abajo no puede ver el player)

#define GRUNT_ATTN					ATTN_STATIC	// //medium radius

#define SENT_SMALLRADIUS			ATTN_IDLE
#define SENT_MEDIUMRADIUS			ATTN_STATIC
#define SENT_LARGERADIUS			ATTN_NORM

#define LIMP_HEALTH					20
#define	TERRORIST_MINIMUN_LIGHT		40// el valor de luz minimo permitido (a partir de ese valor no puede ver)

#define NO_RESISTENCE				0
#define LOW_RESISTENCE				50
#define MED_RESISTENCE				100
#define HIGH_RESISTENCE				250
#define SUPER_RESISTENCE			500

#define	TERRORIST_SENTENCE_VOLUME		(float)0.35 // volume of grunt sentences

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
#define KAMIKAZE				9//this is not a real weapon

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		TERRORIST_AE_RELOAD		( 2 )
#define		TERRORIST_AE_KICK			( 3 )
#define		TERRORIST_AE_BURST1		( 4 )
#define		TERRORIST_AE_BURST2		( 5 ) 
#define		TERRORIST_AE_BURST3		( 6 ) 
#define		TERRORIST_AE_GREN_TOSS		( 7 )
#define		TERRORIST_AE_GREN_LAUNCH	( 8 )
#define		TERRORIST_AE_GREN_DROP		( 9 )
#define		TERRORIST_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		TERRORIST_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define		TERRORIST_AE_DIE_SHOOT		( 22 )
#define		TERRORIST_AE_DRAW			( 25 )
#define		TERRORIST_AE_SURRENDER		( 29 )//the monster can play the animation, but not he won't be surrender at all
#define		TERRORIST_AE_KAMIKAZE		( 30 )//define when the monster explodes

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_TERRORIST_SUPPRESS = LAST_COMMON_SCHEDULE + 1,
	SCHED_TERRORIST_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_TERRORIST_COVER_AND_RELOAD,
	SCHED_TERRORIST_SWEEP,
	SCHED_TERRORIST_FOUND_ENEMY,
	SCHED_TERRORIST_REPEL,
	SCHED_TERRORIST_REPEL_ATTACK,
	SCHED_TERRORIST_REPEL_LAND,
	SCHED_TERRORIST_WAIT_FACE_ENEMY,
	SCHED_TERRORIST_TAKECOVER_FAILED,// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_TERRORIST_ELOF_FAIL,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_TERRORIST_FACE_TOSS_DIR = LAST_COMMON_TASK + 1,
	TASK_TERRORIST_SPEAK_SENTENCE,
	TASK_TERRORIST_CHECK_FIRE,
};

//=========================================================
// monster-specific conditions
//=========================================================
#define bits_COND_TERRORIST_NOFIRE	( bits_COND_SPECIAL1 )

class CHAk : public CSquadMonster
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

	void FireWeapon( void );

	void FireWeaponWhenDie( void );//the monster is diying and shooting his weapon

	void PrescheduleThink ( void );
	void GibMonster( void );
	void SpeakSentence( void );

	void RunAI( void );

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
	int		m_iPistolShell;
	int		m_iShotgunShell;

	int		m_iSentence;

	//SysOp: new definitions

	void TryToDeploySecondaryWeapon( Activity NewActivity );

	BOOL	m_fHasBeenHit;//the monster has been hit, slow it down
	BOOL	m_fStealthed;//the monster spawn as stealth
	BOOL	m_fGunDrawn;//check if the pistol has been deployed
	BOOL	m_fHasDeagle;//check if the monster has desert e or not
	BOOL	m_bChangeClassNone;//dont shoot at monster when he says "dont shoot me!"

	BOOL	m_bKamikaze;//the monster now doesn't have more weapons and it's prepared to suicide
	BOOL	m_bUseArmor;//saves headshots
	BOOL	m_bSurrenderStanding;//pick one surrender style animation (dont switch each other)

//	BOOL	m_fSurrender;//im out... i don't want to fight anymore
	//used cbase entity m_fSurrender

	int		m_flHitCounter;

	int		m_iNumClips;//amount of clips
	int		m_iNumGrenades;//amount of grenades

//	int		m_iFrustration;//what kind and amount of damage the player inflicted on me
	//used cbase entity m_iFrustration

	int		m_iBreakPoint;//some monsters can react so bad to player, others don't and they just surrender.

	BOOL bPlayerVisible;//is the player hidden? can I see it?

	static const char *pTerrorSentences[];
};

LINK_ENTITY_TO_CLASS( monster_terrorist, CHAk );
LINK_ENTITY_TO_CLASS( monster_human_ak, CHAk );

TYPEDESCRIPTION	CHAk::m_SaveData[] = 
{
	DEFINE_FIELD( CHAk, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHAk, m_flNextPainTime, FIELD_TIME ),
	DEFINE_FIELD( CHAk, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHAk, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_cClipSize, FIELD_INTEGER ),
	DEFINE_FIELD( CHAk, m_voicePitch, FIELD_INTEGER ),
	DEFINE_FIELD( CHAk, m_iSentence, FIELD_INTEGER ),
	
	DEFINE_FIELD( CHAk, m_fHasBeenHit, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_fStealthed, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_flHitCounter, FIELD_INTEGER ),
	DEFINE_FIELD( CHAk, m_iNumGrenades, FIELD_INTEGER ),
	DEFINE_FIELD( CHAk, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_iNumClips, FIELD_INTEGER ),
	DEFINE_FIELD( CHAk, m_iBreakPoint, FIELD_INTEGER ),
	DEFINE_FIELD( CHAk, m_fHasDeagle, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_iFrustration, FIELD_INTEGER ),
	DEFINE_FIELD( CHAk, m_bChangeClassNone, FIELD_BOOLEAN ),	
	DEFINE_FIELD( CHAk, m_fSurrender, FIELD_BOOLEAN ),//overwritted?
	DEFINE_FIELD( CHAk, m_bKamikaze, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHAk, m_bUseArmor, FIELD_BOOLEAN )
};

IMPLEMENT_SAVERESTORE( CHAk, CSquadMonster );

const char *CHAk::pTerrorSentences[] = 
{//zxczxc
	"SPA_GREN", // grenade scared grunt
	"SPA_ALERT", // sees player
	"SPA_MONSTER", // sees monster
	"SPA_COVER", // running to cover
	"SPA_THROW", // about to throw grenade
	"SPA_CHARGE",  // running out to get the enemy
	"SPA_TAUNT", // say rude things

	"SPA_LIGHT", // when the lights go on
	"SPA_GIVEUP", //won't fight anymore
	"SPA_CHECK" //fix: this is used but there isn't here... why?
	"SPA_QUEST",
	"SPA_IDLE",
	"SPA_CLEAR",
	"SPA_ANSWER"
};


enum
{
	TERRORIST_SENT_NONE = -1,
	TERRORIST_SENT_GREN = 0,
	TERRORIST_SENT_ALERT,
	TERRORIST_SENT_MONSTER,
	TERRORIST_SENT_COVER,
	TERRORIST_SENT_THROW,
	TERRORIST_SENT_CHARGE,
	TERRORIST_SENT_TAUNT,
} HAK_SENTENCE_TYPES;

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
void CHAk :: SpeakSentence( void )
{
	if ( m_iSentence == TERRORIST_SENT_NONE )
	{
		// no sentence cued up.
		return; 
	}

//	int iCloserToBreak;

//	iCloserToBreak = m_iBreakPoint - 50;//ej. break point = 200, closer to break = 150

	if (FOkToSpeak())
	{
		//if (iCloserToBreak >= m_iBreakPoint)					
		SENTENCEG_PlayRndSz( ENT(pev), "SPA_TAUNT", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		//else
	//	SENTENCEG_PlayRndSz( ENT(pev), pTerrorSentences[ m_iSentence ], TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		
		JustSpoke();
	}
}

//=========================================================
// IRelationship - overridden because Alien Grunts are 
// Human Grunt's nemesis.
//=========================================================
int CHAk::IRelationship ( CBaseEntity *pTarget )
{
		//LRC- only hate alien grunts if my behaviour hasn't been overridden
	if (!m_iClass && FClassnameIs( pTarget->pev, "monster_alien_grunt" ) || ( FClassnameIs( pTarget->pev,  "monster_gargantua" ) ) )
	{
		return R_NM;
	}
	/*
	if ( FClassnameIs( pTarget->pev, "monster_alien_grunt" ) || ( FClassnameIs( pTarget->pev,  "monster_gargantua" ) ) )
	{
		return R_NM;
	}
*/
	return CSquadMonster::IRelationship( pTarget );
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CHAk :: GibMonster ( void )
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
			pGun = DropItem( "weapon_ak47", vecGunPos, vecGunAngles );}
		else if (pev->frags == 6){
			pGun = DropItem( "weapon_glock18", vecGunPos, vecGunAngles );}
		else if (pev->frags == 7){}
		else if (pev->frags == 8){
			pGun = DropItem( "weapon_M249", vecGunPos, vecGunAngles );}
		else if (pev->frags == 9){
		pGun = DropItem( "weapon_mp7", vecGunPos, vecGunAngles );}
		else
		{
			//CANT DROP ITEM!!!
		}

		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			pGun->pev->angles.x = 90;
			pGun->pev->angles.y =  90;
		}
	}

	CBaseMonster :: GibMonster();
}

//=========================================================
// ISoundMask - Overidden for human grunts because they 
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CHAk :: ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER;
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CHAk :: FOkToSpeak( void )
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
void CHAk :: JustSpoke( void )
{
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = TERRORIST_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CHAk :: PrescheduleThink ( void )
{
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
BOOL CHAk :: FCanCheckAttacks ( void )
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
BOOL CHAk :: CheckMeleeAttack1 ( float flDot, float flDist )
{
	if(m_fSurrender)
	return FALSE;

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
BOOL CHAk :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if(m_fSurrender)
	return FALSE;

	if(m_bKamikaze)
	return FALSE;

	if (pev->frags == LAW)
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

	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CHAk :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if(m_fSurrender)
	return FALSE;

	if (m_iNumGrenades <= 0)
	return FALSE;
	
	if(m_bKamikaze)
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
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CHAk :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if(m_bUseArmor)
	{
		switch( ptr->iHitgroup)
		{
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
			if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST))
			{
				flDamage = flDamage / 2;

				switch (RANDOM_LONG(0,2)) 
				{
					case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/hit_kevlar-1.wav", 0.9, ATTN_NORM); break;
					case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/hit_kevlar-2.wav", 0.9, ATTN_NORM); break;
					case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/hit_kevlar-3.wav", 0.9, ATTN_NORM); break;
				}
			}
			break;
		case HITGROUP_HEAD:
			if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
			{
				flDamage -= 5;//20

				if (flDamage <= 0)
				{
					flDamage = 0.01;
				}
				
				switch (RANDOM_LONG(0,2)) 
				{
					case 0:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet1.wav", 0.9, ATTN_NORM); break;
					case 1:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet2.wav", 0.9, ATTN_NORM); break;
					case 2:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet3.wav", 0.9, ATTN_NORM); break;
				}

				if (RANDOM_LONG( 0, 99 ) < 40)
				UTIL_WhiteSparks( ptr->vecEndPos, ptr->vecPlaneNormal, 0, 5, 500, 500 );//chispas

				UTIL_WhiteSparks( ptr->vecEndPos, ptr->vecPlaneNormal, 9, 5, 5, 100 );//puntos
				UTIL_WhiteSparks( ptr->vecEndPos, ptr->vecPlaneNormal, 0, 5, 500, 20 );//chispas
			}
			// always a head shot
			ptr->iHitgroup = HITGROUP_HEAD;
			break;
		}
	}
	else
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
	}

	CSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CHAk :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	m_fHasBeenHit = TRUE;
	m_flHitCounter = 100;

	//only player can make the monster surrender
	if ( m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) )
	{
		if ( bitsDamageType & DMG_BULLET )
		m_iFrustration += flDamage * 2; //ie, 762 mm shoot takes 23 points, total 46. Whit four shots the monster can surrender.

		if ( bitsDamageType & DMG_BLAST )
		m_iFrustration += 50;

		if ( bitsDamageType & DMG_SLASH )//knife
		m_iFrustration += 30;

		if ( bitsDamageType & DMG_CLUB )//crowbar
		m_iFrustration += 80;
	}
	
//just remove for now
#if SYS	
	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 3, vecGunPos, vecGunAngles );//0

	if ( IsAlive() )//si esta vivo
	{
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))//daños reales
		{
			if ( flDamage >= 20 )//si es mayor o igual a 30
			{
				if ( m_LastHitGroup == HITGROUP_RIGHTARM)//
				{
					pev->weapons = -1; //sacar codigo de arma
					SetBodygroup( GUN_GROUP, GUN_NONE );//sacarla de vista
					
					GetScheduleOfType ( SCHED_TERRORIST_ELOF_FAIL );					

					if (FBitSet( pev->weapons, TERRORIST_SHOTGUN ))
						DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );//tirarla
					else
						DropItem( "weapon_ak47", vecGunPos, vecGunAngles );//tirarla
				}
			}
		}
	}
#endif

	if ( IsAlive() )//si esta vivo
	{
		if ( ( bitsDamageType & DMG_BULLET) || ( bitsDamageType & DMG_SHOTGUN) )
		{
			if ( flDamage >= 10 )
			{
				if ( m_LastHitGroup == HITGROUP_HEAD)
					pev->skin = 4; //head
				else if ( m_LastHitGroup == HITGROUP_LEFTARM)
					pev->skin = 1;//arms
				else if ( m_LastHitGroup == HITGROUP_RIGHTARM)
					pev->skin = 1;//arms
				else if ( m_LastHitGroup == HITGROUP_LEFTLEG)
					pev->skin = 2;//legs
				else if ( m_LastHitGroup == HITGROUP_RIGHTLEG)
					pev->skin = 2;//legs
				else
					pev->skin = 3; //chest
			}
		}
	}

	Forget( bits_MEMORY_INCOVER );

	return CSquadMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHAk :: SetYawSpeed ( void )
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

void CHAk :: IdleSound( void )
{
	if (FOkToSpeak() && (g_fHAkQuestion || RANDOM_LONG(0,1)))
	{
		if (!g_fHAkQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0,2))
			{
			case 0: // check in
				if (!bPlayerVisible)
					SENTENCEG_PlayRndSz(ENT(pev), "SPA_LIGHT", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_CHECK", TERRORIST_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);					

				g_fHAkQuestion = 1;
				break;
			case 1: // question
				if (!bPlayerVisible)
					SENTENCEG_PlayRndSz(ENT(pev), "SPA_LIGHT", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_QUEST", TERRORIST_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
			
				g_fHAkQuestion = 2;
				break;
			case 2: // statement
				if (!bPlayerVisible)
					SENTENCEG_PlayRndSz(ENT(pev), "SPA_LIGHT", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				else
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_IDLE", TERRORIST_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fHAkQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_CLEAR", TERRORIST_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_ANSWER", TERRORIST_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fHAkQuestion = 0;
		}
		JustSpoke();
	}
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CHAk :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CHAk :: Classify ( void )
{
	if(m_bChangeClassNone)
	{
		return CLASS_NONE;//i can't attack, and they can't attack me
//		ALERT ( at_aiconsole, "INFO: Classify is now CLASS_NONE\n" );
	}
	else
	{
		return m_iClass?m_iClass:CLASS_FACTION_C; //Terroristas odian a todos TODOS !
	}
}

//=========================================================
//=========================================================
CBaseEntity *CHAk :: Kick( void )
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

Vector CHAk :: GetGunPosition( )
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
//=========================================================
void CHAk :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	switch( pEvent->event )
	{
		case TERRORIST_AE_DRAW:
			{
				if(m_fHasDeagle)
				SetBodygroup( GUN_GROUP, GUN_DESERT );
				else
				SetBodygroup( GUN_GROUP, GUN_PISTOL );
			
				EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/weapon_deploy.wav", 1, ATTN_NORM, 0, 100 );
				m_fGunDrawn = TRUE;
					
//				ALERT ( at_aiconsole, "INFO: m_fGunDrawn = TRUE\n" );
			}
		break;	
		
		case TERRORIST_AE_KAMIKAZE:
			{
				ExplosionCreate( Center(), pev->angles, edict(), 150, TRUE );
				GibMonster();
			}
		break;

//		case TERRORIST_AE_HOLSTER:
//			m_fGunDrawn = FALSE;
//		break;

		case TERRORIST_AE_DROP_GUN:
			{	//UNDONE: keep track of shoots fired, and use the remaining shoots as ammo.
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
						pItem = DropItem( "weapon_AK47", vecGunPos, vecGunAngles );}
					else if (pev->frags == PISTOL){
						pItem = DropItem( "weapon_glock18", vecGunPos, vecGunAngles );}
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
//						ALERT ( at_aiconsole, "INFO: AE_DROPGUN - Dropping Gun whit no ammo\n" );

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
/*							if ( RANDOM_LONG( 0, 1 ) == 0 )
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
												
								ClearConditions(bits_COND_NO_AMMO_LOADED);//this could be necessary
							}
							else
							{
								pev->frags = PISTOL;//now we have a pistol
								m_iNumClips = 999;//at this point the monster doens't have any clips, give it a few of them
							
								//edit, change the clip size and give ammo
								m_cClipSize = 20;
								m_cAmmoLoaded = m_cClipSize;//give it ammo! (dont reload weapon when he draw it)
									
								ClearConditions(bits_COND_NO_AMMO_LOADED);
							}
						}
					}
								
					//give it a few clips (so that the monster doesn't try to perform a new reload animation)
					//clear enemy but dont set surrender to true, the code will check this later
					//and set again all this
					if (!m_fSurrender)
					{
//						ALERT ( at_aiconsole, "INFO: AE_DROP_GUN - Clearing enemy and monsterstate, m_fSurrender = TRUE\n" );

						//if the monster wants to reload

						m_iNumClips = 1;//this is not really necessary
						m_cClipSize = 1;
						m_cAmmoLoaded = m_cClipSize;

						ClearConditions(bits_COND_NO_AMMO_LOADED);
					
						pev->frags = 0;	//this prevent a bug, when the monster tries to attack
						//m_fSurrender = TRUE;
						//look for the event
					}

					//if -> crash, remove the following
					/*
					if(pItem)
					{
						pItem->pev->avelocity = Vector ( RANDOM_FLOAT( -222, 222 ), RANDOM_FLOAT( -222, 222 ),RANDOM_FLOAT( -222, 222 ) );
						pItem->pev->angles.x = 90;
						pItem->pev->angles.y =  90;
					}*/
				}
			}
			break;

		case TERRORIST_AE_SURRENDER:
			{
				//give it a few clips (so that the monster doesn't try to perform a new reload animation)
				if (!m_fSurrender)
				{
//					ALERT ( at_aiconsole, "INFO: AE_SURRENDER - Clearing enemy and monsterstate, m_fSurrender = TRUE\n" );
		
					//moved to run AI
					m_MonsterState	= MONSTERSTATE_IDLE;
					m_hEnemy = NULL;//clear enemy

/*
					//why again?
					//why again?
					//moved to run AI
						m_hEnemy = NULL;//clear enemy

						m_iNumClips = 1;//this is not really necessary
						m_cClipSize = 1;
						m_cAmmoLoaded = m_cClipSize;

						SetConditions(bits_COND_NO_AMMO_LOADED);
									
						pev->frags = 0;	//this prevent a bug, when the monster tries to attack
											//take out weapons.
									//why again?
									//why again?
*/

					CBaseEntity *pEntity = NULL;

					Vector VecSrc = pev->origin;
										
					//play it everywhere? (like a radio)
					while ((pEntity = UTIL_FindEntityInSphere( pEntity, VecSrc, 512 )) != NULL)//512
					if (FClassnameIs(pEntity->pev, "monster_agency_member"))
						EMIT_SOUND_DYN( ENT(0), CHAN_STATIC, "npc_voices/friend/radio/SuspectCooperate.wav", 1, ATTN_NONE, 0, PITCH_NORM );

					CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

					//gain points
					int iXPPoints = CVAR_GET_FLOAT( "xp_points" );

					UTIL_ShowMessageAll( STRING(ALLOC_STRING("BONUS_SURR")));
					//ClientPrint(pPlayer->pev, HUD_PRINTTALK, "#BONUS_SURR" );

					CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 50) );
									
					m_fSurrender = TRUE;

					if(!m_bSurrenderStanding)
					pev->solid = SOLID_NOT;	
					
					FireTargets( "surrendered", this, this, USE_TOGGLE, 0 );
				}
			}
			break;


		case TERRORIST_AE_RELOAD:
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "human_ak/reload1.wav", 1, ATTN_NORM );		

			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
			break;

		case TERRORIST_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );

			//shit! its too dark here!
			if (!bPlayerVisible)
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

		case TERRORIST_AE_GREN_LAUNCH:
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

		case TERRORIST_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
					
			if (!bPlayerVisible)
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

		case TERRORIST_AE_BURST1:
			FireWeapon();
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
			break;

		case TERRORIST_AE_BURST2:
		case TERRORIST_AE_BURST3:
			FireWeapon();
			break;

		case TERRORIST_AE_DIE_SHOOT:
			FireWeaponWhenDie();

		case TERRORIST_AE_KICK:
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

		case TERRORIST_AE_CAUGHT_ENEMY:
		{
			if ( FOkToSpeak() )
			{
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_ALERT", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
void CHAk :: Spawn()
{
	Precache( );

	if (pev->model)
	SET_MODEL( ENT(pev), STRING(pev->model) );
	else
	SET_MODEL(ENT(pev), "models/human_ak.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;

	if (pev->health == 0)
	pev->health			= gSkillData.hGrunt_akHealth; //No es demasiado???

	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence			= TERRORIST_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector ( 0, 0, 55 );

	m_fGunDrawn			= FALSE;
	m_fHasBeenHit		= FALSE;
	m_iNumGrenades		= 3;

	if (RANDOM_LONG(0,1))
	m_bUseArmor			= TRUE;
	else
	m_bUseArmor			= FALSE;

	//the following determines if the terrorist will use one of the two differetns animations
	//(standing or on knees)
	if (RANDOM_LONG(0,1))
	m_bSurrenderStanding	= TRUE;
	else
	m_bSurrenderStanding	= FALSE;

	//if pev->frags is not present (can't find any value, not even zero)
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

	//random option from FGD file
	if ( pev->frags == -1)
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

	//new code: Let's give random frustration	
	//EDIT: this, at hammer, works so wrong
//	if ( FStringNull( pev->dmg ))
//	{
		switch (RANDOM_LONG(0,4))
		{
			case 0: m_iBreakPoint = NO_RESISTENCE; break;
			case 1: m_iBreakPoint = LOW_RESISTENCE; break;
			case 2: m_iBreakPoint = MED_RESISTENCE; break;
			case 3: m_iBreakPoint = HIGH_RESISTENCE; break;
			case 4: m_iBreakPoint = SUPER_RESISTENCE; break;
		}
//	}

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
/*
		TraceResult tr;
		CBeam *m_pBeam;

		Vector vecShootOrigin;

		UTIL_MakeVectors(pev->angles);
		vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
		Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

		Vector angDir = UTIL_VecToAngles( vecShootDir );

		UTIL_TraceLine( vecShootOrigin, vecShootDir, dont_ignore_monsters, ENT( pev ), &tr );		

		m_pBeam = CBeam::BeamCreate( g_pModelNameLaser, 10 );
		m_pBeam->PointEntInit( vecShootDir, entindex() );
		m_pBeam->SetColor( 255, 0, 0 );
		m_pBeam->SetEndAttachment( 1 );
		m_pBeam->SetScrollRate( 255 );
		m_pBeam->SetBrightness( 64 );
*/
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
	else if (pev->frags == KAMIKAZE)
	{
		SetBodygroup( GUN_GROUP, GUN_NONE );
		m_cClipSize		= 999;
		m_iNumClips		= 999;//140 bullets
	}

	m_cAmmoLoaded		= m_cClipSize;

	//model test
	pev->skin = 0;

	/*
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

	if (pev->skin == -1)//azar
	{
		switch (RANDOM_LONG(0,3))
		{
			case 0: pev->skin = 0; break;//BUG BUG, the skin 0 its 1
			case 1: pev->skin = 1; break;
			case 2: pev->skin = 2; break;
			case 3: pev->skin = 3; break;
		}
	}
*/

	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHAk :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else
	PRECACHE_MODEL("models/human_ak.mdl");
	
	UTIL_PrecacheOther( "hvr_rocket" );

	PRECACHE_SOUND( "human_ak/die1.wav" );
	PRECACHE_SOUND( "human_ak/die2.wav" );
	PRECACHE_SOUND( "human_ak/die3.wav" );

	PRECACHE_SOUND( "human_ak/pain1.wav" );
	PRECACHE_SOUND( "human_ak/pain2.wav" );
	PRECACHE_SOUND( "human_ak/pain3.wav" );
	PRECACHE_SOUND( "human_ak/pain4.wav" );
	PRECACHE_SOUND( "human_ak/pain5.wav" );

	PRECACHE_SOUND( "human_ak/myarm01.wav" );
	PRECACHE_SOUND( "human_ak/myarm02.wav" );
	PRECACHE_SOUND( "human_ak/myleg01.wav" );
	PRECACHE_SOUND( "human_ak/myleg02.wav" );
	
	PRECACHE_SOUND( "npc_voices/spanish/surrender_1_voiceC.wav" );
	PRECACHE_SOUND( "npc_voices/spanish/surrender_2_voiceC.wav" );

	PRECACHE_SOUND( "human_ak/reload1.wav" );
	PRECACHE_SOUND( "weapons/reload_shotgun.wav" );

	PRECACHE_MODEL ("models/weapons/w_czgrenade.mdl");

	// get voice pitch
	if (RANDOM_LONG(0,1))
		m_voicePitch = 109 + RANDOM_LONG(0,7);
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL ("models/shell_762.mdl");// brass shell
	m_iPistolShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl");
}	

//=========================================================
// start task
//=========================================================
void CHAk :: StartTask ( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
	case TASK_TERRORIST_CHECK_FIRE:
		if ( !NoFriendlyFire() )
		{
			SetConditions( bits_COND_TERRORIST_NOFIRE );
		}
		TaskComplete();
		break;

	case TASK_TERRORIST_SPEAK_SENTENCE:
		SpeakSentence();
		TaskComplete();
		break;
	
	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		// grunt no longer assumes he is covered if he moves
		Forget( bits_MEMORY_INCOVER );
		CSquadMonster ::StartTask( pTask );
		break;

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_TERRORIST_FACE_TOSS_DIR:
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
void CHAk :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_TERRORIST_FACE_TOSS_DIR:
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
/*		
	case TASK_RANGE_ATTACK1:
		if (m_hEnemy != NULL && (m_hEnemy->IsPlayer()))
		{
			pev->framerate = 2;
		}
		CSquadMonster::RunTask( pTask );
		break;
*/
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
void CHAk :: PainSound ( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
#if 0
		if ( RANDOM_LONG(0,99) < 5 )
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "SPA_PAIN", TERRORIST_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
				JustSpoke();
				return;
			}
		}
#endif 
		//fix CHAN_VOICE
		//if the monster doenst have any mouth controller?
			
		if ( m_LastHitGroup == HITGROUP_LEFTARM)
		EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/myarm01.wav", 1, ATTN_NORM );	
		else if ( m_LastHitGroup == HITGROUP_RIGHTARM)
		EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/myarm02.wav", 1, ATTN_NORM );	
		else if ( m_LastHitGroup == HITGROUP_LEFTLEG)
		EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/myleg01.wav", 1, ATTN_NORM );	
		else if ( m_LastHitGroup == HITGROUP_RIGHTLEG)
		EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/myleg02.wav", 1, ATTN_NORM );	
		else
		{		
			switch ( RANDOM_LONG(0,4) )
			{
				case 0:	
					EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/pain3.wav", 1, ATTN_NORM );	
					break;
				case 1:
					EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/pain4.wav", 1, ATTN_NORM );	
					break;
				case 2:
					EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/pain5.wav", 1, ATTN_NORM );	
					break;
				case 3:
					EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/pain1.wav", 1, ATTN_NORM );	
					break;
				case 4:
					EMIT_SOUND( ENT(pev), CHAN_ITEM, "human_ak/pain2.wav", 1, ATTN_NORM );	
					break;
			}
		}

		/*
		switch ( RANDOM_LONG(0,4) )
		{
		case 0:	
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/pain3.wav", 1, ATTN_NORM );	
			break;
		case 1:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/pain4.wav", 1, ATTN_NORM );	
			break;
		case 2:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/pain5.wav", 1, ATTN_NORM );	
			break;
		case 3:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/pain1.wav", 1, ATTN_NORM );	
			break;
		case 4:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/pain2.wav", 1, ATTN_NORM );	
			break;
		}
		*/

		m_flNextPainTime = gpGlobals->time + 2;//let finish what he are saying
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CHAk :: DeathSound ( void )
{
	switch ( RANDOM_LONG(0,2) )
	{
	case 0:	
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/die1.wav", 1, ATTN_IDLE );	
		break;
	case 1:
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/die2.wav", 1, ATTN_IDLE );	
		break;
	case 2:
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "human_ak/die3.wav", 1, ATTN_IDLE );	
		break;
	}
	/*		
	Vector	vecGunPos;

	Vector	vecGunAngles;
						
	GetAttachment( 3, vecGunPos, vecGunAngles );

	CBaseEntity *pItem1 = DropItem( "xp_point_plate", vecGunPos + Vector (-5,15,5), vecGunAngles );
	CBaseEntity *pItem2 = DropItem( "xp_point_plate", vecGunPos + Vector (-4,6,5), vecGunAngles );
	CBaseEntity *pItem3 = DropItem( "xp_point_plate", vecGunPos + Vector (-7,3,-5), vecGunAngles );
	CBaseEntity *pItem4 = DropItem( "xp_point_plate", vecGunPos + Vector (-2,-1,11), vecGunAngles );
			
	pItem1->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
	pItem1->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
	
	pItem2->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
	pItem2->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );

	pItem3->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
	pItem3->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );

	pItem4->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
	pItem4->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
*/
	//i need to add a killed func, but im quite lazy to do it
	
	if (m_bKamikaze)
	{
		Vector	vecGunPos;
		Vector	vecGunAngles;

		GetAttachment( 3, vecGunPos, vecGunAngles );
		
		CGrenade::ShootTimedCz( pev, vecGunPos + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4 );
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// GruntFail
//=========================================================
Task_t	tlHAkFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slHAkFail[] =
{
	{
		tlHAkFail,
		ARRAYSIZE ( tlHAkFail ),
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
Task_t	tlHAkCombatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slHAkCombatFail[] =
{
	{
		tlHAkCombatFail,
		ARRAYSIZE ( tlHAkCombatFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Grunt Combat Fail"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t	tlHAkVictoryDance[] =
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

Schedule_t	slHAkVictoryDance[] =
{
	{ 
		tlHAkVictoryDance,
		ARRAYSIZE ( tlHAkVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"GruntVictoryDance"
	},
};
//=========================================================
// HAkEnemyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t	tlHAkEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
};

Schedule_t slHAkEnemyDraw[] = 
{
	{
		tlHAkEnemyDraw,
		ARRAYSIZE ( tlHAkEnemyDraw ),
		0,
		0,
		"HAk Enemy Draw"
	}
};
//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlHAkEstablishLineOfFire[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_TERRORIST_ELOF_FAIL	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0						},
	{ TASK_TERRORIST_SPEAK_SENTENCE,(float)0						},
	{ TASK_RUN_PATH,			(float)0						},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slHAkEstablishLineOfFire[] =
{
	{ 
		tlHAkEstablishLineOfFire,
		ARRAYSIZE ( tlHAkEstablishLineOfFire ),
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
Task_t	tlHAkFoundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL1			},
};

Schedule_t	slHAkFoundEnemy[] =
{
	{ 
		tlHAkFoundEnemy,
		ARRAYSIZE ( tlHAkFoundEnemy ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"GruntFoundEnemy"
	},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t	tlHAkCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1.5					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TERRORIST_SWEEP	},
};

Schedule_t	slHAkCombatFace[] =
{
	{ 
		tlHAkCombatFace1,
		ARRAYSIZE ( tlHAkCombatFace1 ), 
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
Task_t	tlHAkSignalSuppress[] =
{
	{ TASK_STOP_MOVING,					0						},
	{ TASK_FACE_IDEAL,					(float)0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_SIGNAL2		},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_TERRORIST_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_TERRORIST_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_TERRORIST_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_TERRORIST_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_TERRORIST_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
};

Schedule_t	slHAkSignalSuppress[] =
{
	{ 
		tlHAkSignalSuppress,
		ARRAYSIZE ( tlHAkSignalSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_TERRORIST_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SignalSuppress"
	},
};

Task_t	tlHAkSuppress[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
};

Schedule_t	slHAkSuppress[] =
{
	{ 
		tlHAkSuppress,
		ARRAYSIZE ( tlHAkSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_TERRORIST_NOFIRE		|
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
Task_t	tlHAkWaitInCover[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)1					},
};

Schedule_t	slHAkWaitInCover[] =
{
	{ 
		tlHAkWaitInCover,
		ARRAYSIZE ( tlHAkWaitInCover ), 
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
Task_t	tlHAkTakeCover1[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_TERRORIST_TAKECOVER_FAILED	},
	{ TASK_WAIT,					(float)0.2							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_TERRORIST_SPEAK_SENTENCE,	(float)0							},
	{ TASK_RUN_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TERRORIST_WAIT_FACE_ENEMY	},
};

Schedule_t	slHAkTakeCover[] =
{
	{ 
		tlHAkTakeCover1,
		ARRAYSIZE ( tlHAkTakeCover1 ), 
		0,
		0,
		"TakeCover"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlHAkGrenadeCover1[] =
{
	{ TASK_STOP_MOVING,						(float)0							},
	{ TASK_FIND_COVER_FROM_ENEMY,			(float)99							},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384							},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SPECIAL_ATTACK1			},
	{ TASK_CLEAR_MOVE_WAIT,					(float)0							},
	{ TASK_RUN_PATH,						(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TERRORIST_WAIT_FACE_ENEMY	},
};

Schedule_t	slHAkGrenadeCover[] =
{
	{ 
		tlHAkGrenadeCover1,
		ARRAYSIZE ( tlHAkGrenadeCover1 ), 
		0,
		0,
		"GrenadeCover"
	},
};


//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlHAkTossGrenadeCover1[] =
{
	{ TASK_FACE_ENEMY,						(float)0							},
	{ TASK_RANGE_ATTACK2, 					(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t	slHAkTossGrenadeCover[] =
{
	{ 
		tlHAkTossGrenadeCover1,
		ARRAYSIZE ( tlHAkTossGrenadeCover1 ), 
		0,
		0,
		"TossGrenadeCover"
	},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t	tlHAkTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slHAkTakeCoverFromBestSound[] =
{
	{ 
		tlHAkTakeCoverFromBestSound,
		ARRAYSIZE ( tlHAkTakeCoverFromBestSound ), 
		0,
		0,
		"GruntTakeCoverFromBestSound"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlHAkHideReload[] =
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

Schedule_t slHAkHideReload[] = 
{
	{
		tlHAkHideReload,
		ARRAYSIZE ( tlHAkHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntHideReload"
	}
};

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t	tlHAkSweep[] =
{
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
};

Schedule_t	slHAkSweep[] =
{
	{ 
		tlHAkSweep,
		ARRAYSIZE ( tlHAkSweep ), 
		
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
Task_t	tlHAkRangeAttack1A[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	//$sequence crouching_wait "crouching_wait" fps 30 ACT_CROUCH 0 
	//delay de 0.23 segundos... joya

	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_CROUCH },
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slHAkRangeAttack1A[] =
{
	{ 
		tlHAkRangeAttack1A,
		ARRAYSIZE ( tlHAkRangeAttack1A ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_TERRORIST_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"Range Attack1A"
	},
};


//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlHAkRangeAttack1B[] =
{
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_IDLE_ANGRY },
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_TERRORIST_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slHAkRangeAttack1B[] =
{
	{ 
		tlHAkRangeAttack1B,
		ARRAYSIZE ( tlHAkRangeAttack1B ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_TERRORIST_NOFIRE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Range Attack1B"
	},
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlHAkRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_TERRORIST_FACE_TOSS_DIR,		(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TERRORIST_WAIT_FACE_ENEMY	},// don't run immediately after throwing grenade.
};

Schedule_t	slHAkRangeAttack2[] =
{
	{ 
		tlHAkRangeAttack2,
		ARRAYSIZE ( tlHAkRangeAttack2 ), 
		0,
		0,
		"RangeAttack2"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlHAkRepel[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_GLIDE 	},
};

Schedule_t	slHAkRepel[] =
{
	{ 
		tlHAkRepel,
		ARRAYSIZE ( tlHAkRepel ), 
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
Task_t	tlHAkRepelAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_FLY 	},
};

Schedule_t	slHAkRepelAttack[] =
{
	{ 
		tlHAkRepelAttack,
		ARRAYSIZE ( tlHAkRepelAttack ), 
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Repel Attack"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t	tlHAkRepelLand[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slHAkRepelLand[] =
{
	{ 
		tlHAkRepelLand,
		ARRAYSIZE ( tlHAkRepelLand ), 
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


DEFINE_CUSTOM_SCHEDULES( CHAk )
{
	slHAkFail,
	slHAkCombatFail,
	slHAkVictoryDance,
	slHAkEnemyDraw,
	slHAkEstablishLineOfFire,
	slHAkFoundEnemy,
	slHAkCombatFace,
	slHAkSignalSuppress,
	slHAkSuppress,
	slHAkWaitInCover,
	slHAkTakeCover,
	slHAkGrenadeCover,
	slHAkTossGrenadeCover,
	slHAkTakeCoverFromBestSound,
	slHAkHideReload,
	slHAkSweep,
	slHAkRangeAttack1A,
	slHAkRangeAttack1B,
	slHAkRangeAttack2,
	slHAkRepel,
	slHAkRepelAttack,
	slHAkRepelLand,
};

IMPLEMENT_CUSTOM_SCHEDULES( CHAk, CSquadMonster );

//=========================================================
// SetActivity 
//=========================================================
void CHAk :: SetActivity ( Activity NewActivity )
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
//					ALERT ( at_aiconsole, "INFO: SetActivity - Weapon Empty!\n" );

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
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}	
				
			//the monster is going to attack. check if we don't fight anymore
			//override numclips check, only call "surrender_smg/pistol" sequence
			//the monster will gain clips and secondary weapons, but it doens't matter
			//because he won't fight

			//dont check it at runai() func, check here now, when the monster attack
			//unless the monster start to give up when it's shooted
					
			if (!m_fSurrender)//start to change some values :)
			{
				if (m_iFrustration >= m_iBreakPoint)//I can't take this no more!
				{
					if (m_bSurrenderStanding)
					{
						if (pev->frags == PISTOL || DESERT )//pistols
						{
							if (RANDOM_LONG(0,1))
							iSequence = LookupSequence( "surrender_pistol" );
							else
							iSequence = LookupSequence( "surrender_pistol2" );
						}
						else
						{
							iSequence = LookupSequence( "surrender_smg" );
						}
					}
					else
					{
						iSequence = LookupSequence( "surrender_all" );
					}
							
					//speak something like: 'oh please! don't kill me superman!'
					switch ( RANDOM_LONG(0,1) )
					{
						case 0:	
							EMIT_SOUND( ENT(pev), CHAN_BODY, "npc_voices/spanish/surrender_1_voiceC.wav", 1, ATTN_NORM );	
							break;
						case 1:
							EMIT_SOUND( ENT(pev), CHAN_BODY, "npc_voices/spanish/surrender_2_voiceC.wav", 1, ATTN_NORM );	
							break;
					}
//					ALERT ( at_aiconsole, "INFO: SetActivity ACT_RANGE_ATTACK1 - Sending Animation and changing to CLASS_NONE\n" );
					
										
					m_hEnemy = NULL;//clear enemy

					m_bChangeClassNone = TRUE;//change his class now!!
				}
			}
		}
	break;

	//this is not neccesary. called from qc file
//	case ACT_RANGE_ATTACK2://they always have grenades, dont check "weapon TERRORIST_HANDGRENADE"
//		iSequence = LookupSequence( "throwgrenade" );
//		break;

	case ACT_RUN:	
		{
			if (m_fSurrender)
			{
				if (m_bSurrenderStanding)
				iSequence = LookupSequence( "surrender_run" );
				else
				iSequence = LookupActivity ( ACT_INSPECT_FLOOR );
			}
			else if (m_bKamikaze)
			iSequence = LookupSequence( "run_he" );
			else
			{
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
			}
		}
		break;

	case ACT_WALK:
		{
			if (m_fSurrender)
			{
				if (m_bSurrenderStanding)
				iSequence = LookupSequence( "surrender_walk" );
				else
				iSequence = LookupActivity ( ACT_INSPECT_FLOOR );
			}
			else if (m_bKamikaze)
			iSequence = LookupSequence( "run_he" );
			else
			{
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
			}
		}
		break;

	case ACT_RELOAD:
		{		
			if (m_fSurrender)
			{
				if (m_bSurrenderStanding)
				iSequence = LookupActivity ( ACT_GUARD );
				else
				iSequence = LookupActivity ( ACT_INSPECT_FLOOR );
			}
			else
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
//					DropItem( "item_clip_rifle", vecGunPos, vecGunAngles );//test
					m_iNumClips--;//less clip!
				}
			
				//the monster will try to reload... if it don't have clips, drop the weapon
				if(m_iNumClips <=0)//no clips. overriden for pistol because the monster have 999 clips of that before drop primary weapon
				{		
					if (pev->frags == PISTOL){}
					else if (pev->frags == DESERT){}
					else
					{
						iSequence = LookupSequence( "drop_weapon" );
					}
				}

				m_iFrustration += 10;//oohhhh shiiit! I doens't have ammo!
		
				//check it here too???
				//if the monster is not surrendered and the frustration breach the maximun breakpoint value
				//drop the weapon
				//change this at random, drop the weapon and deploy his secondary weapon or go to kamikaze
				if (!m_fSurrender)//start to change some values :)
				{	
					//if (RANDOM_LONG(0,1))//its is just yes or no... //fix
					if ( RANDOM_LONG(0,99) < 5 )
					{
						if(!m_bKamikaze)
						{
							m_bKamikaze = TRUE;
									
							pev->frags = KAMIKAZE;//change the weapon, no matter why
									
							//well, the monster reload the weapon and clear the bit COND_NO_AMMO_LOADED, only
							//in HandleAnimFunc. It needs the monster make the reload animation and I don't
							//want it. Just clear the bit COND_NO_AMMO_LOADED and "reload" the weapon here and now.
							//after this, the monster will try to attack, but only in melee mode.

							m_cAmmoLoaded = m_cClipSize;
							ClearConditions(bits_COND_NO_AMMO_LOADED);
						}
					}
					else
					{
						if (m_iFrustration >= m_iBreakPoint)//I can't take this no more!
						{
							if (m_bSurrenderStanding)
							{
								if (pev->frags == PISTOL || DESERT )//pistols
								{
									if (RANDOM_LONG(0,1))
									iSequence = LookupSequence( "surrender_pistol" );
									else
									iSequence = LookupSequence( "surrender_pistol2" );
								}
								else
								{
									iSequence = LookupSequence( "surrender_smg" );
								}
							}
							else
							{
								iSequence = LookupSequence( "surrender_all" );
							}
									
							//speak something like: 'oh please! don't kill me superman!'
							switch ( RANDOM_LONG(0,1) )
							{
								case 0:	
									EMIT_SOUND( ENT(pev), CHAN_BODY, "npc_voices/spanish/surrender_1_voiceC.wav", 1, ATTN_NORM );	
									break;
								case 1:
									EMIT_SOUND( ENT(pev), CHAN_BODY, "npc_voices/spanish/surrender_2_voiceC.wav", 1, ATTN_NORM );	
									break;
							}
						
											
							m_hEnemy = NULL;//clear enemy

//							ALERT ( at_aiconsole, "INFO: SetActivity ACT_RELOAD - Sending Animation and changing to CLASS_NONE\n" );

							m_bChangeClassNone = TRUE;//change his class now!!
						}
					}
				}
			}
		}
	 break;

	case ACT_IDLE:
			if (StartBurning)
			{
				NewActivity = ACT_SNIFF;
			}
			else
			{
				if (m_fSurrender)
				{
					if (m_bSurrenderStanding)
					NewActivity = ACT_GUARD;
					else
					NewActivity = ACT_INSPECT_FLOOR;
				}

				if ( m_MonsterState == MONSTERSTATE_COMBAT )
				{
					NewActivity = ACT_IDLE_ANGRY;
				}
			}

			iSequence = LookupActivity ( NewActivity );//right... go look foor NewActivity (could be ACT_GUARD, ACT_INSPECT_FLOOR or ACT_IDLE_ANGRY)
		break;

	/*
	case ACT_IDLE:
		if (m_fSurrender)
		{
			iSequence = LookupSequence( "Surrender_Idle" );
		}
		else
		{
			iSequence = LookupSequence( "idle1" );
		}

		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		break;
*/
	case ACT_IDLE_ANGRY:
		{				
			if (StartBurning)
			{
				iSequence = LookupActivity ( ACT_SNIFF );
			}
			else
			{
				if (m_fSurrender)
				{
					if (m_bSurrenderStanding)
					iSequence = LookupActivity ( ACT_GUARD );
					else
					iSequence = LookupActivity ( ACT_INSPECT_FLOOR );
				}
				else
				{
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
				}
			}
		}
	break;
	
	case ACT_CROUCH:
		{
			if (m_fSurrender)
			{
				if (m_bSurrenderStanding)
				iSequence = LookupActivity ( ACT_GUARD );
				else
				iSequence = LookupActivity ( ACT_INSPECT_FLOOR );
			}
			else
			{
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
			}
		}
	 break;

	case ACT_MELEE_ATTACK1:	
		if (pev->frags == KAMIKAZE)
		{
			iSequence = LookupSequence( "fire_he" );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}	
	 break;


	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		{
			if (m_fSurrender)
			{
				if (m_bSurrenderStanding)
				iSequence = LookupActivity ( ACT_GUARD );
				else
				iSequence = LookupActivity ( ACT_INSPECT_FLOOR );
			}
			else
			{
				iSequence = LookupActivity ( NewActivity );
			}	
		}
	break;

	case ACT_SMALL_FLINCH://new
	case ACT_BIG_FLINCH://new
	case ACT_FLINCH_HEAD:
	case ACT_FLINCH_CHEST:
	case ACT_FLINCH_STOMACH:
	case ACT_FLINCH_LEFTARM:
	case ACT_FLINCH_RIGHTARM:
	case ACT_FLINCH_LEFTLEG:
	case ACT_FLINCH_RIGHTLEG:
		{
			if (StartBurning)
			{
				iSequence = LookupActivity ( ACT_SNIFF );
			}
			else
			{
				if (m_fSurrender)
				{
					if (m_bSurrenderStanding)
					iSequence = LookupActivity ( ACT_GUARD );
					else
					iSequence = LookupActivity ( ACT_INSPECT_FLOOR );
				}
				else
				{
					iSequence = LookupActivity ( NewActivity );
				}	
			}
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
Schedule_t *CHAk :: GetSchedule( void )
{
	// clear old sentence
	m_iSentence = TERRORIST_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if ( pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE )
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType ( SCHED_TERRORIST_REPEL_LAND );
		}
		else
		{
			// repel down a rope, 
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
				return GetScheduleOfType ( SCHED_TERRORIST_REPEL_ATTACK );
			else
				return GetScheduleOfType ( SCHED_TERRORIST_REPEL );
		}
	}

	// grunts place HIGH priority on running away from danger sounds.
	//SysOp: if the monster is surrendered dont get involved, go away from enemy
	/*
	if (m_fSurrender)
	{
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
						SENTENCEG_PlayRndSz( ENT(pev), "SPA_GREN", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_NONE );
				}
			}
		}
	}
*/
	switch	( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		{
			//SysOp: This code make the monster will look for sounds. It's great.
			//edit: creo que se contradice con lo de arriba
			//edit2: no, esta bien, el chequeo es danger y luego combat
			if (!m_fSurrender)
			{
				if ( HasConditions ( bits_COND_HEAR_SOUND ))
				{
					CSound *pSound;
					pSound = PBestSound();

					ASSERT( pSound != NULL );
					if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
					{
						if (FOkToSpeak())
						{
							SENTENCEG_PlayRndSz( ENT(pev), "SPA_GREN", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
							JustSpoke();
						}

						return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
					}
					if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
					{
						return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
					}
				}
			}
		}
		break;


	case MONSTERSTATE_COMBAT:
		{
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
								SENTENCEG_PlayRndSz( ENT(pev), "SPA_ALERT", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
							else if ((m_hEnemy != NULL) &&
									(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) && 
									(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) && 
									(m_hEnemy->Classify() != CLASS_MACHINE))
								// monster
								SENTENCEG_PlayRndSz( ENT(pev), "SPA_MONST", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);

							JustSpoke();
						}
						
						if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
						{
							return GetScheduleOfType ( SCHED_TERRORIST_SUPPRESS );
						}
						else
						{
							return GetScheduleOfType ( SCHED_TERRORIST_ESTABLISH_LINE_OF_FIRE );
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
				return GetScheduleOfType ( SCHED_TERRORIST_COVER_AND_RELOAD );
			}
			
// damaged just a little
			else if ( HasConditions( bits_COND_LIGHT_DAMAGE ) )
			{
				// if hurt:
				// 10% chance of taking cover
				// 90% chance of flinch.
				int iPercent = RANDOM_LONG(0,99);

				if ( iPercent <= 10 && m_hEnemy != NULL )
				{
					// only try to take cover if we actually have an enemy!

					//!!!KELLY - this grunt was hit and is going to run to cover.
					if (FOkToSpeak()) // && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "SPA_COVER", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						m_iSentence = TERRORIST_SENT_COVER;
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
						return GetScheduleOfType ( SCHED_TERRORIST_FOUND_ENEMY );
					}
				}

				if ( OccupySlot ( bits_SLOTS_TERRORIST_ENGAGE ) )
				{
					// try to take an available ENGAGE slot
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_TERRORIST_GRENADE ) )
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
				if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_TERRORIST_GRENADE ) )
				{
					//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz( ENT(pev), "SPA_THROW", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else if ( OccupySlot( bits_SLOTS_TERRORIST_ENGAGE ) )
				{
					//!!!KELLY - grunt cannot see the enemy and has just decided to 
					// charge the enemy's position. 
					if (FOkToSpeak())// && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "SPA_CHARGE", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						m_iSentence = TERRORIST_SENT_CHARGE;
						//JustSpoke();
					}

					return GetScheduleOfType( SCHED_TERRORIST_ESTABLISH_LINE_OF_FIRE );
				}
				else
				{
					//!!!KELLY - grunt is going to stay put for a couple seconds to see if
					// the enemy wanders back out into the open, or approaches the
					// grunt's covered position. Good place for a taunt, I guess?
					if (FOkToSpeak() && RANDOM_LONG(0,1))
					{
						SENTENCEG_PlayRndSz( ENT(pev), "SPA_TAUNT", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_STANDOFF );
				}
			}
			
			if ( HasConditions( bits_COND_SEE_ENEMY ) && !HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
			//	m_iFrustration++;//this is devil

				return GetScheduleOfType ( SCHED_TERRORIST_ESTABLISH_LINE_OF_FIRE );
			}
		}
	}
	
	// no special cases here, call the base class
	return CSquadMonster :: GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CHAk :: GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{	
	case SCHED_ARM_WEAPON://test
		if ( m_hEnemy != NULL )
		{
			// face enemy, then draw.
			return slHAkEnemyDraw;
		}

	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if ( InSquad() )
			{
				if ( g_iSkillLevel == SKILL_HARD && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_TERRORIST_GRENADE ) )
				{
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz( ENT(pev), "SPA_THROW", TERRORIST_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return slHAkTossGrenadeCover;
				}
				else
				{
					return &slHAkTakeCover[ 0 ];
				}
			}
			else
			{
				if ( RANDOM_LONG(0,1) )
				{
					return &slHAkTakeCover[ 0 ];
				}
				else
				{
					return &slHAkGrenadeCover[ 0 ];
				}
			}
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slHAkTakeCoverFromBestSound[ 0 ];
		}
	case SCHED_TERRORIST_TAKECOVER_FAILED:
		{
			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_TERRORIST_ENGAGE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}

			return GetScheduleOfType ( SCHED_FAIL );
		}
		break;
	case SCHED_TERRORIST_ELOF_FAIL:
		{
			// human grunt is unable to move to a position that allows him to attack the enemy.
			return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
		}
		break;
	case SCHED_TERRORIST_ESTABLISH_LINE_OF_FIRE:
		{
			return &slHAkEstablishLineOfFire[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK1:
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

			if (m_fStanding)
				return &slHAkRangeAttack1B[ 0 ];//parado?
			else
				return &slHAkRangeAttack1A[ 0 ];
		}
	case SCHED_RANGE_ATTACK2:
		{
			return &slHAkRangeAttack2[ 0 ];
		}
	case SCHED_COMBAT_FACE:
		{
			return &slHAkCombatFace[ 0 ];
		}
	case SCHED_TERRORIST_WAIT_FACE_ENEMY:
		{
			return &slHAkWaitInCover[ 0 ];
		}
	case SCHED_TERRORIST_SWEEP:
		{
			return &slHAkSweep[ 0 ];
		}
	case SCHED_TERRORIST_COVER_AND_RELOAD:
		{
			return &slHAkHideReload[ 0 ];
		}
	case SCHED_TERRORIST_FOUND_ENEMY:
		{
			return &slHAkFoundEnemy[ 0 ];
		}
	case SCHED_VICTORY_DANCE:
		{
			if ( InSquad() )
			{
				if ( !IsLeader() )
				{
					return &slHAkFail[ 0 ];
				}
			}

			return &slHAkVictoryDance[ 0 ];
		}
	case SCHED_TERRORIST_SUPPRESS:
		{
			if ( m_hEnemy->IsPlayer() && m_fFirstEncounter )
			{
				m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slHAkSignalSuppress[ 0 ];
			}
			else
			{
				return &slHAkSuppress[ 0 ];
			}
		}
	case SCHED_FAIL:
		{
			m_iFrustration += 20;//oh!!! I failed! The things doesn't work as I expected! The world is against me!

			if ( m_hEnemy != NULL )
			{
				// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
				return &slHAkCombatFail[ 0 ];
			}

			return &slHAkFail[ 0 ];
		}
	case SCHED_TERRORIST_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slHAkRepel[ 0 ];
		}
	case SCHED_TERRORIST_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slHAkRepelAttack[ 0 ];
		}
	case SCHED_TERRORIST_REPEL_LAND:
		{
			return &slHAkRepelLand[ 0 ];
		}
	default:
		{
			return CSquadMonster :: GetScheduleOfType ( Type );
		}
	}
}


//=========================================================
// CHAkRepel - when triggered, spawns a monster_human_grunt
// repelling down a line.
//=========================================================

class CHAkRepel : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT RepelUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int m_iSpriteTexture;	// Don't save, precache
};

LINK_ENTITY_TO_CLASS( monster_terrorist_repel, CHAkRepel );

void CHAkRepel::Spawn( void )
{
	Precache( );
	pev->solid = SOLID_NOT;

	SetUse( RepelUse );
}

void CHAkRepel::Precache( void )
{
	UTIL_PrecacheOther( "monster_terrorist" );
	m_iSpriteTexture = PRECACHE_MODEL( "sprites/rope.spr" );
}

void CHAkRepel::RepelUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, -4096.0), dont_ignore_monsters, ENT(pev), &tr);
	/*
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP) 
		return NULL;
	*/

	CBaseEntity *pEntity = Create( "monster_terrorist", pev->origin, pev->angles );
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
// RunAI Tomado del cod de Assassin
//=========================================================
void CHAk :: RunAI( void )
{
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

	if (iPlayerLuces <= PLAYER_MINIMUN_LIGHT)
		bPlayerVisible = FALSE;
	else
		bPlayerVisible = TRUE;
		
	if(m_flHitCounter >= 1)
	m_flHitCounter -= 1;

	if(m_flHitCounter <= 0)
	m_fHasBeenHit = FALSE;

//	if(m_fHasBeenHit)
//	ALERT ( at_aiconsole, "Monster is Hurted!\n" );
	
	if(m_bKamikaze)
	{
		m_iFrustration = 0;//dont get frustrated
						
		Vector	vecGunPos;
		Vector	vecGunAngles;

		GetAttachment( 3, vecGunPos, vecGunAngles );

		// switch to body group with no gun.
		SetBodygroup( GUN_GROUP, GUN_NONE );
	
		//start smoke the HE
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", vecGunPos, TRUE );
					
		pSprite->SetTransparency( kRenderTransAlpha, 150, 150, 150, 255, kRenderFxNone );
		pSprite->SetScale( RANDOM_FLOAT( 0.5, 1 ) );
				
		pSprite->Expand( 1, RANDOM_FLOAT( 125.0, 135.0 )  );//????????????
	}
		
	if (pev->spawnflags & SF_MONSTER_DONT_REACT)
	m_iFrustration = 0;//dont get frustrated

	CBaseMonster :: RunAI();
}

//=========================================================
// FireWeapon 
//=========================================================

#define	CONE_1DEGREES 0.00873
#define	CONE_2DEGREES 0.01745
#define	CONE_3DEGREES 0.02618
#define	CONE_4DEGREES 0.03490
#define	CONE_5DEGREES 0.04362
#define	CONE_6DEGREES 0.05234
#define	CONE_7DEGREES 0.06105
#define	CONE_8DEGREES 0.06976
#define	CONE_9DEGREES 0.07846
#define	CONE_10DEGREES 0.08716
#define	CONE_15DEGREES  0.13053
#define	CONE_20DEGREES 0.17365

void CHAk :: FireWeapon ( void )
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
	
	if(m_cAmmoLoaded > 4)//treated as empty
	pev->effects = EF_MUZZLEFLASH;
		
	float targetx, targety, targetz, totalrecoil; 
	
	if (pev->frags == MP5)
		totalrecoil=CONE_4DEGREES;
	else if (pev->frags == SHOTGUN)
		totalrecoil=CONE_15DEGREES;
	else if (pev->frags == AWP)
		totalrecoil=CONE_1DEGREES;
	else if (pev->frags == ASSAULT)
		totalrecoil=CONE_15DEGREES;
	else if (pev->frags == PISTOL)
		totalrecoil=CONE_8DEGREES;
	else if (pev->frags == DESERT)
		totalrecoil=CONE_5DEGREES;
	else if (pev->frags == M249)
		totalrecoil=CONE_20DEGREES;
	else
		totalrecoil=0;

	if ( !m_fStanding )
		totalrecoil-=CONE_2DEGREES;

	if (m_iFrustration >= 200)
		totalrecoil+=CONE_20DEGREES;
	else if (m_iFrustration >= 150)
		totalrecoil+=CONE_10DEGREES;
	else if (m_iFrustration >= 100)
		totalrecoil+=CONE_6DEGREES;
	else if (m_iFrustration >= 50)
		totalrecoil+=CONE_3DEGREES;
	else if (m_iFrustration >= 25)
		totalrecoil+=CONE_1DEGREES;
	else {}

	targetx = targety = targetz = totalrecoil;//let's make this fix (I don't want recode all again :P)

	if (pev->frags == MP5)
	{
		if(m_cAmmoLoaded <= 3)//treated as empty
		{
			m_cAmmoLoaded--;// take away a bullet!
			EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/357_cock1.wav", 1, ATTN_NORM );
			return;
		}

		if(m_fStealthed)//precise
		FireBullets(1, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
		else
		FireBullets(1, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		int pitchShift = RANDOM_LONG( 0, 20 );
	
		// Only shift about half the time
		if ( pitchShift > 10 )
			pitchShift = 0;
		else
			pitchShift -= 5;

		if(m_fStealthed)
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_firesil-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
		else
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/mp5/mp5_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );

		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iPistolShell, TE_BOUNCE_SHELL); 
	}
	else if (pev->frags == SHOTGUN)
	{
		FireBullets(8, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 1024, BULLET_PLAYER_BUCKSHOT);//357

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
		FireBullets(1, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 99999, BULLET_PLAYER_SNIPER ); // shoot perfect

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

		FireBullets(1, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 2048, BULLET_PLAYER_FAMAS	 ); // shoot +-5 degrees

		int pitchShift = RANDOM_LONG( 0, 20 );
	
		// Only shift about half the time
		if ( pitchShift > 10 )
			pitchShift = 0;
		else
			pitchShift -= 5;

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/ak-47/ak47_fire-1.wav", 1, ATTN_NORM, 0, 100 + pitchShift );
	
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

		FireBullets(1, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 99999, BULLET_PLAYER_9MM ); // shoot perfect

		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/glock18/glock18_fire-1.wav", 1, ATTN_NORM, 0, 100 );
	
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iPistolShell, TE_BOUNCE_SHELL); 
	}
	
	else if (pev->frags == DESERT)
	{
		FireBullets(1, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 99999, BULLET_PLAYER_DEAGLE ); // shoot perfect

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
	
		FireBullets(1, vecShootOrigin, vecShootDir, Vector( targetx, targety, targetz ), 9999, BULLET_MONSTER_12MM ); // shoot +-5 degrees
	
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
void CHAk :: FireWeaponWhenDie ( void )
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );

	Vector vecShootDir = vecShootOrigin + gpGlobals->v_forward * 5;

	Vector angDir = UTIL_VecToAngles( vecShootDir );

	SetBlending( 0, angDir.x );

	pev->effects = EF_MUZZLEFLASH;

	//only shoot automatic weapons
	if (pev->frags == MP5)
	{
		if(m_fStealthed)//precise
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
		else
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		int pitchShift = RANDOM_LONG( 0, 20 );
	
		// Only shift about half the time
		if ( pitchShift > 10 )
			pitchShift = 0;
		else
			pitchShift -= 5;

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

		int pitchShift = RANDOM_LONG( 0, 20 );
	
		// Only shift about half the time
		if ( pitchShift > 10 )
			pitchShift = 0;
		else
			pitchShift -= 5;

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
// TryToDeploySecondaryWeapon - the monster doens't have any
// primary ammo, so drop the gun and deploy his pistol
//=========================================================
void CHAk :: TryToDeploySecondaryWeapon( Activity NewActivity )
{
	//code removed
}

