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

/**

  CHANGES ON THIS FILE:
  
Derivation of hornets. This is "soul" that flies to you. When you touch it you gain health.
Maybe you're wonder about this file. Me too.

***/
//=========================================================
// Souls
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"hornet.h"
#include	"gamerules.h"

//=========================================================
// Soul Defines
//=========================================================
#define Soul_TYPE_RED			0
#define Soul_TYPE_ORANGE		1

#define Soul_RED_SPEED			(float)300//600
#define Soul_ORANGE_SPEED		(float)400//800

#define	Soul_BUZZ_VOLUME		(float)0.8


int iSoulTrail;
int iSoulPuff;

LINK_ENTITY_TO_CLASS( soul, CSoul );

//=========================================================
// Save/Restore
//=========================================================
TYPEDESCRIPTION	CSoul::m_SaveData[] = 
{
	DEFINE_FIELD( CSoul, m_flStopAttack, FIELD_TIME ),
	DEFINE_FIELD( CSoul, m_iSoulType, FIELD_INTEGER ),
	DEFINE_FIELD( CSoul, m_flFlySpeed, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CSoul, CBaseMonster );

//=========================================================
// don't let Souls gib, ever.
//=========================================================
int CSoul :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// filter these bits a little.
	bitsDamageType &= ~ ( DMG_ALWAYSGIB );
	bitsDamageType |= DMG_NEVERGIB;

	return CBaseMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
//=========================================================
void CSoul :: Spawn( void )
{
	Precache();

	pev->movetype	= MOVETYPE_FLY;

//	pev->solid		= SOLID_BBOX;
	pev->solid = SOLID_NOT;

	pev->takedamage = DAMAGE_YES;
	pev->flags		|= FL_MONSTER;
	pev->health		= 1;// weak!
	
	if ( g_pGameRules->IsMultiplayer() )
	{
		// Souls don't live as long in multiplayer
		m_flStopAttack = gpGlobals->time + 3.5;
	}
	else
	{
		m_flStopAttack	= gpGlobals->time + 5.0;
	}

	m_flFieldOfView = 0.9; // +- 25 degrees

	if ( RANDOM_LONG ( 1, 5 ) <= 2 )
	{
		m_iSoulType = Soul_TYPE_RED;
		m_flFlySpeed = Soul_RED_SPEED;
	}
	else
	{
		m_iSoulType = Soul_TYPE_ORANGE;
		m_flFlySpeed = Soul_ORANGE_SPEED;
	}

	SET_MODEL(ENT( pev ), "models/combine_ball.mdl");
	UTIL_SetSize( pev, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );

	SetTouch( DieTouch );
	SetThink( StartTrack );

	edict_t *pSoundEnt = pev->owner;
	if ( !pSoundEnt )
		pSoundEnt = edict();

	/*
	CSprite *pSprite = CSprite::SpriteCreate( "sprites/alt_fire.spr", pev->origin, FALSE );
		pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 155, kRenderFxNone );
		pSprite->SetScale( 0.2 );		
		pSprite->SetAttachment( edict(), 0 );
//		pSprite->pev->frame = 0;
//		pSprite->pev->framerate = 0;
//	pSprite->Expand( 0.001, 333  );//150
*/
			
	pev->rendermode = kRenderNormal;
	pev->renderamt = 50;

	pev->renderfx = 19;

	pev->rendercolor.x = 50;
	pev->rendercolor.y = 50;
	pev->rendercolor.z = 50;

	pev->nextthink = gpGlobals->time + 0.1;
	ResetSequenceInfo( );
}


void CSoul :: Precache()
{
	PRECACHE_MODEL ("sprites/alt_fire.spr");
	PRECACHE_MODEL ("models/combine_ball.mdl");//weapons

	PRECACHE_SOUND( "agrunt/ag_fire1.wav" );
	PRECACHE_SOUND( "agrunt/ag_fire2.wav" );
	PRECACHE_SOUND( "agrunt/ag_fire3.wav" );

	PRECACHE_SOUND( "Soul/ag_buzz1.wav" );
	PRECACHE_SOUND( "Soul/ag_buzz2.wav" );
	PRECACHE_SOUND( "Soul/ag_buzz3.wav" );

	PRECACHE_SOUND( "Soul/ag_Soulhit1.wav" );
	PRECACHE_SOUND( "Soul/ag_Soulhit2.wav" );
	PRECACHE_SOUND( "Soul/ag_Soulhit3.wav" );

	iSoulPuff = PRECACHE_MODEL( "sprites/muz1.spr" );
	iSoulTrail = PRECACHE_MODEL("sprites/laserbeam.spr");
}	

//=========================================================
// Souls will never get mad at each other, no matter who the owner is.
//=========================================================
int CSoul::IRelationship ( CBaseEntity *pTarget )
{
	if ( pTarget->pev->modelindex == pev->modelindex )
	{
		return R_NO;
	}

	return CBaseMonster :: IRelationship( pTarget );
}

//=========================================================
// ID's Soul as their owner
//=========================================================
int CSoul::Classify ( void )
{
	if ( pev->owner && pev->owner->v.flags & FL_CLIENT)
	{
		return CLASS_PLAYER_BIOWEAPON;
	}

	return	CLASS_ALIEN_BIOWEAPON;
}

//=========================================================
// StartTrack - starts a Soul out tracking its target
//=========================================================
void CSoul :: StartTrack ( void )
{
	IgniteTrail();

	SetTouch( TrackTouch );
	SetThink( TrackTarget );

	pev->nextthink = gpGlobals->time + 0.1;
}

//=========================================================
// StartDart - starts a Soul out just flying straight.
//=========================================================
void CSoul :: StartDart ( void )
{
	IgniteTrail();

	SetTouch( DartTouch );

	SetThink( SUB_Remove );
	pev->nextthink = gpGlobals->time + 4;
}

void CSoul::IgniteTrail( void )
{
/*

  ted's suggested trail colors:

r161
g25
b97

r173
g39
b14

old colors
		case Soul_TYPE_RED:
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 128 );   // r, g, b
			WRITE_BYTE( 0 );   // r, g, b
			break;
		case Soul_TYPE_ORANGE:
			WRITE_BYTE( 0   );   // r, g, b
			WRITE_BYTE( 100 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			break;
	
*/

	// trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE(  TE_BEAMFOLLOW );
		WRITE_SHORT( entindex() );	// entity
		WRITE_SHORT( iSoulTrail );	// model
		WRITE_BYTE( 10 ); // life
		WRITE_BYTE( 10 );  // width //2
		
		switch ( m_iSoulType )
		{
		case Soul_TYPE_RED://white... always
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			break;
		case Soul_TYPE_ORANGE:
			WRITE_BYTE( 222 );   // r, g, b
			WRITE_BYTE( 222 );   // r, g, b
			WRITE_BYTE( 222 );   // r, g, b
			break;
		}

		WRITE_BYTE( 128 );	// brightness

	MESSAGE_END();
}

//=========================================================
// Soul is flying, gently tracking target
//=========================================================
void CSoul :: TrackTarget ( void )
{
	Vector	vecFlightDir;
	Vector	vecDirToEnemy;
	float	flDelta;

	StudioFrameAdvance( );

	if (gpGlobals->time > m_flStopAttack)
	{
		SetTouch( NULL );
		SetThink( SUB_Remove );
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	// UNDONE: The player pointer should come back after returning from another level
	if ( m_hEnemy == NULL )
	{// enemy is dead.
		Look( 512 );
		//m_hEnemy = BestVisibleEnemy( );
		m_hEnemy = UTIL_FindEntityByClassname(NULL, "player");
	}
	
	if ( m_hEnemy != NULL && FVisible( m_hEnemy ))
	{
		m_vecEnemyLKP = m_hEnemy->BodyTarget( pev->origin );
	}
	else
	{
		m_vecEnemyLKP = m_vecEnemyLKP + pev->velocity * m_flFlySpeed * 0.1;
	}

	vecDirToEnemy = ( m_vecEnemyLKP - pev->origin ).Normalize();

	if (pev->velocity.Length() < 0.1)
		vecFlightDir = vecDirToEnemy;
	else 
		vecFlightDir = pev->velocity.Normalize();

	// measure how far the turn is, the wider the turn, the slow we'll go this time.
	flDelta = DotProduct ( vecFlightDir, vecDirToEnemy );
	
	if ( flDelta < 0.5 )
	{// hafta turn wide again. play sound
		switch (RANDOM_LONG(0,2))
		{
		case 0:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_buzz1.wav", Soul_BUZZ_VOLUME, ATTN_NORM);	break;
		case 1:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_buzz2.wav", Soul_BUZZ_VOLUME, ATTN_NORM);	break;
		case 2:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_buzz3.wav", Soul_BUZZ_VOLUME, ATTN_NORM);	break;
		}
	}

	if ( flDelta <= 0 && m_iSoulType == Soul_TYPE_RED )
	{// no flying backwards, but we don't want to invert this, cause we'd go fast when we have to turn REAL far.
		flDelta = 0.25;
	}

	pev->velocity = ( vecFlightDir + vecDirToEnemy).Normalize();

	if ( pev->owner && (pev->owner->v.flags & FL_MONSTER) )
	{
		// random pattern only applies to Souls fired by monsters, not players. 

		pev->velocity.x += RANDOM_FLOAT ( -0.10, 0.10 );// scramble the flight dir a bit.
		pev->velocity.y += RANDOM_FLOAT ( -0.10, 0.10 );
		pev->velocity.z += RANDOM_FLOAT ( -0.10, 0.10 );
	}
	
	switch ( m_iSoulType )
	{
		case Soul_TYPE_RED:
			pev->velocity = pev->velocity * ( m_flFlySpeed * flDelta );// scale the dir by the ( speed * width of turn )
			pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.3 );
			break;
		case Soul_TYPE_ORANGE:
			pev->velocity = pev->velocity * m_flFlySpeed;// do not have to slow down to turn.
			pev->nextthink = gpGlobals->time + 0.1;// fixed think time
			break;
	}

	pev->angles = UTIL_VecToAngles (pev->velocity);

	pev->solid = SOLID_BBOX;

	// if Soul is close to the enemy, jet in a straight line for a half second.
	// (only in the single player game)
	if ( m_hEnemy != NULL && !g_pGameRules->IsMultiplayer() )
	{
		if ( flDelta >= 0.4 && ( pev->origin - m_vecEnemyLKP ).Length() <= 300 )
		{
/*
			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
				WRITE_BYTE( TE_SPRITE );
				WRITE_COORD( pev->origin.x);	// pos
				WRITE_COORD( pev->origin.y);
				WRITE_COORD( pev->origin.z);
				WRITE_SHORT( iSoulPuff );		// model
				// WRITE_BYTE( 0 );				// life * 10
				WRITE_BYTE( 2 );				// size * 10
				WRITE_BYTE( 128 );			// brightness
			MESSAGE_END();
*/
			switch (RANDOM_LONG(0,2))
			{
			case 0:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_buzz1.wav", Soul_BUZZ_VOLUME, ATTN_NORM);	break;
			case 1:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_buzz2.wav", Soul_BUZZ_VOLUME, ATTN_NORM);	break;
			case 2:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_buzz3.wav", Soul_BUZZ_VOLUME, ATTN_NORM);	break;
			}
			pev->velocity = pev->velocity * 2;
			pev->nextthink = gpGlobals->time + 1.0;
			// don't attack again
			m_flStopAttack = gpGlobals->time;
		}
	}
}

//=========================================================
// Tracking Soul hit something
//=========================================================
void CSoul :: TrackTouch ( CBaseEntity *pOther )
{
	if ( pOther->edict() == pev->owner || pOther->pev->modelindex == pev->modelindex )
	{// bumped into the guy that shot it.
		pev->solid = SOLID_NOT;
		return;
	}

	if ( !pOther->IsPlayer() )//if I touch a non-player, bounce and return
	{
		pev->velocity = pev->velocity.Normalize();

		pev->velocity.x *= -1;
		pev->velocity.y *= -1;

		pev->origin = pev->origin + pev->velocity * 4; // bounce the Soul off a bit.
		pev->velocity = pev->velocity * m_flFlySpeed;

		return;
	}

	if ( IRelationship( pOther ) <= R_NO )
	{
		// hit something we don't want to hurt, so turn around.

		pev->velocity = pev->velocity.Normalize();

		pev->velocity.x *= -1;
		pev->velocity.y *= -1;

		pev->origin = pev->origin + pev->velocity * 4; // bounce the Soul off a bit.
		pev->velocity = pev->velocity * m_flFlySpeed;

		return;
	}

	DieTouch( pOther );
}

void CSoul::DartTouch( CBaseEntity *pOther )
{
	DieTouch( pOther );
}

void CSoul::DieTouch ( CBaseEntity *pOther )
{
	if ( pOther && pOther->pev->takedamage )
	{// do the damage

		switch (RANDOM_LONG(0,2))
		{// buzz when you plug someone
			case 0:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_Soulhit1.wav", 1, ATTN_NORM);	break;
			case 1:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_Soulhit2.wav", 1, ATTN_NORM);	break;
			case 2:	EMIT_SOUND( ENT(pev), CHAN_VOICE, "Soul/ag_Soulhit3.wav", 1, ATTN_NORM);	break;
		}
			
//		pOther->TakeDamage( pev, VARS( pev->owner ), pev->dmg, DMG_BULLET );
			
//		pOther->TakeHealth (pev->dmg, DMG_GENERIC);

		pOther->pev->health += pev->dmg;//edit: el doble

		if (pOther->pev->health >200)			
		pOther->pev->health = 200;//FIX, para no tener 242 de vida

		pOther->pev->armorvalue += pev->dmg;//edit: el doble

		if (pOther->pev->armorvalue >200)
			pOther->pev->armorvalue = 200;//FIX, para no tener 242 de escudo
	}

	pev->modelindex = 0;// so will disappear for the 0.1 secs we wait until NEXTTHINK gets rid
	pev->solid = SOLID_NOT;

	SetThink ( SUB_Remove );
	pev->nextthink = gpGlobals->time + 1;// stick around long enough for the sound to finish!
}

