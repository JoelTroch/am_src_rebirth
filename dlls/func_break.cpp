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
  
+func_phy -BASE CODE BY F00z- (modified by SysOp -base class derived from CBrekeable and added
sounds & effects)
*Other minor modifications

***/
/*

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models

*/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "func_break.h"
#include "decals.h"
#include "explode.h"

#include "effects.h"//for sprite effects 
#include "particle_defs.h"
extern int gmsgParticles;

extern DLL_GLOBAL Vector		g_vecAttackDir;

// =================== FUNC_Breakable ==============================================

// Just add more items to the bottom of this array and they will automagically be supported
// This is done instead of just a classname in the FGD so we can control which entities can
// be spawned, and still remain fairly flexible
const char *CBreakable::pSpawnObjects[] =
{
	NULL,				// 0
	"item_battery",		// 1
	"item_healthkit",	// 2
	"weapon_9mmhandgun",// 3
	"ammo_9mmclip",		// 4
	"weapon_9mmAR",		// 5
	"ammo_9mmAR",		// 6
	"ammo_ARgrenades",	// 7
	"weapon_shotgun",	// 8
	"ammo_buckshot",	// 9
	"weapon_crossbow",	// 10
	"ammo_crossbow",	// 11
	"weapon_357",		// 12
	"ammo_357",			// 13
	"weapon_rpg",		// 14
	"ammo_rpgclip",		// 15
	"ammo_gaussclip",	// 16
	"weapon_handgrenade",// 17
	"weapon_tripmine",	// 18
	"weapon_satchel",	// 19
	"weapon_snark",		// 20
	"weapon_hornetgun",	// 21
	//Begin Here
	"xp_point_brown",	// 22
	"xp_point_plate",	// 23
	"xp_point_gold",	// 24
};

void CBreakable::KeyValue( KeyValueData* pkvd )
{
	// UNDONE_WC: explicitly ignoring these fields, but they shouldn't be in the map file!
	if (FStrEq(pkvd->szKeyName, "explosion"))
	{
		if (!stricmp(pkvd->szValue, "directed"))
			m_Explosion = expDirected;
		else if (!stricmp(pkvd->szValue, "random"))
			m_Explosion = expRandom;
		else
			m_Explosion = expRandom;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "material"))
	{
		int i = atoi( pkvd->szValue);

		// 0:glass, 1:metal, 2:flesh, 3:wood

		if ((i < 0) || (i >= matLastMaterial))
			m_Material = matWood;
		else
			m_Material = (Materials)i;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deadmodel"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shards"))
	{
//			m_iShards = atof(pkvd->szValue);
			pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gibmodel") )
	{
		m_iszGibModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnobject") )
	{
		int object = atoi( pkvd->szValue );
		if ( object > 0 && object < ARRAYSIZE(pSpawnObjects) )
			m_iszSpawnObject = MAKE_STRING( pSpawnObjects[object] );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explodemagnitude") )
	{
		ExplosionSetMagnitude( atoi( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lip") )
		pkvd->fHandled = TRUE;
	else
		CBaseDelay::KeyValue( pkvd );
}


//
// func_breakable - bmodel that breaks into pieces after taking damage
//
LINK_ENTITY_TO_CLASS( func_breakable, CBreakable );
TYPEDESCRIPTION CBreakable::m_SaveData[] =
{
	DEFINE_FIELD( CBreakable, m_Material, FIELD_INTEGER ),
	DEFINE_FIELD( CBreakable, m_Explosion, FIELD_INTEGER ),

// Don't need to save/restore these because we precache after restore
//	DEFINE_FIELD( CBreakable, m_idShard, FIELD_INTEGER ),

	DEFINE_FIELD( CBreakable, m_angle, FIELD_FLOAT ),
	DEFINE_FIELD( CBreakable, m_iszGibModel, FIELD_STRING ),
	DEFINE_FIELD( CBreakable, m_iszSpawnObject, FIELD_STRING ),

	// Explosion magnitude is stored in pev->impulse
};

IMPLEMENT_SAVERESTORE( CBreakable, CBaseEntity );

void CBreakable::Spawn( void )
{
    Precache( );    

	if ( FBitSet( pev->spawnflags, SF_BREAK_TRIGGER_ONLY ) )
		pev->takedamage	= DAMAGE_NO;
	else
		pev->takedamage	= DAMAGE_YES;
  
	pev->solid		= SOLID_BSP;
    pev->movetype	= MOVETYPE_PUSH;
    m_angle			= pev->angles.y;
	pev->angles.y	= 0;

	// HACK:  matGlass can receive decals, we need the client to know about this
	//  so use class to store the material flag
	if ( m_Material == matGlass )
	{
		pev->playerclass = 1;
		//FIXME: all piece of glass mat has a additive render mode as default.
		/*
		pev->rendermode = kRenderNormal;			
		pGib->pev->rendermode = pev->rendermode;
		pGib->pev->renderamt = pev->renderamt;
		pGib->pev->rendercolor = pev->rendercolor;
		pGib->pev->renderfx = pev->renderfx;
		*/
	}

	SET_MODEL(ENT(pev), STRING(pev->model) );//set size and link into world.

	SetTouch( BreakTouch );
	if ( FBitSet( pev->spawnflags, SF_BREAK_TRIGGER_ONLY ) )		// Only break on trigger
		SetTouch( NULL );

	// Flag unbreakable glass as "worldbrush" so it will block ALL tracelines
	if ( !IsBreakable() && pev->rendermode != kRenderNormal )
		pev->flags |= FL_WORLDBRUSH;
}


const char *CBreakable::pSoundsWood[] = 
{
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
};

const char *CBreakable::pSoundsFlesh[] = 
{
	"debris/flesh1.wav",
	"debris/flesh2.wav",
	"debris/flesh3.wav",
	"debris/flesh5.wav",
	"debris/flesh6.wav",
	"debris/flesh7.wav",
};

const char *CBreakable::pSoundsMetal[] = 
{
	"debris/metal1.wav",
	"debris/metal2.wav",
	"debris/metal3.wav",
};

const char *CBreakable::pSoundsConcrete[] = 
{
	"debris/concrete1.wav",
	"debris/concrete2.wav",
	"debris/concrete3.wav",
};


const char *CBreakable::pSoundsGlass[] = 
{
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
};

const char **CBreakable::MaterialSoundList( Materials precacheMaterial, int &soundCount )
{
	const char	**pSoundList = NULL;

    switch ( precacheMaterial ) 
	{
	case matWood:
		pSoundList = pSoundsWood;
		soundCount = ARRAYSIZE(pSoundsWood);
		break;
	case matFlesh:
		pSoundList = pSoundsFlesh;
		soundCount = ARRAYSIZE(pSoundsFlesh);
		break;
	case matComputer:
	case matUnbreakableGlass:
	case matGlass:
		pSoundList = pSoundsGlass;
		soundCount = ARRAYSIZE(pSoundsGlass);
		break;

	case matMetal:
		pSoundList = pSoundsMetal;
		soundCount = ARRAYSIZE(pSoundsMetal);
		break;

	case matCinderBlock:
	case matRocks:
		pSoundList = pSoundsConcrete;
		soundCount = ARRAYSIZE(pSoundsConcrete);
		break;
	
	
	case matCeilingTile:
	case matNone:
	default:
		soundCount = 0;
		break;
	}

	return pSoundList;
}

void CBreakable::MaterialSoundPrecache( Materials precacheMaterial )
{
	const char	**pSoundList;
	int			i, soundCount = 0;

	pSoundList = MaterialSoundList( precacheMaterial, soundCount );

	for ( i = 0; i < soundCount; i++ )
	{
		PRECACHE_SOUND( (char *)pSoundList[i] );
	}
}

void CBreakable::MaterialSoundRandom( edict_t *pEdict, Materials soundMaterial, float volume )
{
	const char	**pSoundList;
	int			soundCount = 0;

	pSoundList = MaterialSoundList( soundMaterial, soundCount );

	if ( soundCount )
		EMIT_SOUND( pEdict, CHAN_BODY, pSoundList[ RANDOM_LONG(0,soundCount-1) ], volume, 1.0 );
}


void CBreakable::Precache( void )
{
	const char *pGibName;

    switch (m_Material) 
	{
	case matWood:
		pGibName = "models/woodgibs.mdl";
		
		PRECACHE_SOUND("debris/bustcrate1.wav");
		PRECACHE_SOUND("debris/bustcrate2.wav");
		break;
	case matFlesh:
		pGibName = "models/fleshgibs.mdl";
		
		PRECACHE_SOUND("debris/bustflesh1.wav");
		PRECACHE_SOUND("debris/bustflesh2.wav");
		break;
	case matComputer:
		PRECACHE_SOUND("buttons/spark5.wav");
		PRECACHE_SOUND("buttons/spark6.wav");
		pGibName = "models/computergibs.mdl";
		
		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;

	case matUnbreakableGlass:
	case matGlass:
		pGibName = "models/glassgibs.mdl";
	//	pGibName = "models/glassSmall.mdl";
		
		PRECACHE_SOUND("debris/bustglass1.wav");
		PRECACHE_SOUND("debris/bustglass2.wav");
		break;
	case matMetal:
		pGibName = "models/metalplategibs.mdl";
		
		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;
	case matCinderBlock:
		pGibName = "models/cindergibs.mdl";
		
		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matRocks:
		pGibName = "models/rockgibs.mdl";
		
		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matCeilingTile:
		pGibName = "models/ceilinggibs.mdl";
		
		PRECACHE_SOUND ("debris/bustceiling.wav");  
		break;
	}
	MaterialSoundPrecache( m_Material );
	if ( m_iszGibModel )
		pGibName = STRING(m_iszGibModel);

	m_idShard = PRECACHE_MODEL( (char *)pGibName );

	// Precache the spawn item's data
	if ( m_iszSpawnObject )
		UTIL_PrecacheOther( (char *)STRING( m_iszSpawnObject ) );
}

// play shard sound when func_breakable takes damage.
// the more damage, the louder the shard sound.


void CBreakable::DamageSound( void )
{
	int pitch;
	float fvol;
	char *rgpsz[6];
	int i;
	int material = m_Material;

//	if (RANDOM_LONG(0,1))
//		return;

	if (RANDOM_LONG(0,2))
		pitch = PITCH_NORM;
	else
		pitch = 95 + RANDOM_LONG(0,34);

	fvol = RANDOM_FLOAT(0.75, 1.0);

	if (material == matComputer && RANDOM_LONG(0,1))
		material = matMetal;

	switch (material)
	{
	case matComputer:
	case matGlass:
	case matUnbreakableGlass:
		rgpsz[0] = "debris/glass1.wav";
		rgpsz[1] = "debris/glass2.wav";
		rgpsz[2] = "debris/glass3.wav";
		i = 3;
		break;

	case matWood:
		rgpsz[0] = "debris/wood1.wav";
		rgpsz[1] = "debris/wood2.wav";
		rgpsz[2] = "debris/wood3.wav";
		i = 3;
		break;

	case matMetal:
		rgpsz[0] = "debris/metal1.wav";
		rgpsz[1] = "debris/metal3.wav";
		rgpsz[2] = "debris/metal2.wav";
		i = 2;
		break;

	case matFlesh:
		rgpsz[0] = "debris/flesh1.wav";
		rgpsz[1] = "debris/flesh2.wav";
		rgpsz[2] = "debris/flesh3.wav";
		rgpsz[3] = "debris/flesh5.wav";
		rgpsz[4] = "debris/flesh6.wav";
		rgpsz[5] = "debris/flesh7.wav";
		i = 6;
		break;

	case matRocks:
	case matCinderBlock:
		rgpsz[0] = "debris/concrete1.wav";
		rgpsz[1] = "debris/concrete2.wav";
		rgpsz[2] = "debris/concrete3.wav";
		i = 3;
		break;

	case matCeilingTile:
		// UNDONE: no ceiling tile shard sound yet
		i = 0;
		break;
	}

	if (i)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, rgpsz[RANDOM_LONG(0,i-1)], fvol, ATTN_NORM, 0, pitch);
}

void CBreakable::BreakTouch( CBaseEntity *pOther )
{
	float flDamage;
	entvars_t*	pevToucher = pOther->pev;
	
	// only players can break these right now
	if ( !pOther->IsPlayer() || !IsBreakable() )
	{
        return;
	}

	if ( FBitSet ( pev->spawnflags, SF_BREAK_TOUCH ) )
	{// can be broken when run into 
		flDamage = pevToucher->velocity.Length() * 0.01;

		if (flDamage >= pev->health)
		{
			SetTouch( NULL );
			TakeDamage(pevToucher, pevToucher, flDamage, DMG_CRUSH);

			// do a little damage to player if we broke glass or computer
			pOther->TakeDamage( pev, pev, flDamage/4, DMG_SLASH );
		}
	}

	if ( FBitSet ( pev->spawnflags, SF_BREAK_PRESSURE ) && pevToucher->absmin.z >= pev->maxs.z - 2 )
	{// can be broken when stood upon
		
		// play creaking sound here.
		DamageSound();

		SetThink ( Die );
		SetTouch( NULL );
		
		if ( m_flDelay == 0 )
		{// !!!BUGBUG - why doesn't zero delay work?
			m_flDelay = 0.1;
		}

		pev->nextthink = pev->ltime + m_flDelay;

	}

}


//
// Smash the our breakable object
//

// Break when triggered
void CBreakable::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( IsBreakable() )
	{
		pev->angles.y = m_angle;
		UTIL_MakeVectors(pev->angles);
		g_vecAttackDir = gpGlobals->v_forward;

		Die();
	}
}


void CBreakable::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	// random spark if this is a 'computer' object
	if (RANDOM_LONG(0,1) )
	{
		switch( m_Material )
		{
			case matComputer:
			{
				UTIL_Sparks( ptr->vecEndPos );

				float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range
				switch ( RANDOM_LONG(0,1) )
				{
					case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM);	break;
					case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM);	break;
				}
			}
			break;
			
			case matUnbreakableGlass:
				UTIL_Ricochet( ptr->vecEndPos, RANDOM_FLOAT(0.5,1.5) );
			break;
		}
	}

	CBaseDelay::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// Special takedamage for func_breakable. Allows us to make
// exceptions that are breakable-specific
// bitsDamageType indicates the type of damage sustained ie: DMG_CRUSH
//=========================================================
int CBreakable :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	Vector	vecTemp;

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if ( pevAttacker == pevInflictor )	
	{
		vecTemp = pevInflictor->origin - ( pev->absmin + ( pev->size * 0.5 ) );
				
		// if a client hit the breakable with a crowbar, and breakable is crowbar-sensitive, break it now.
			if ( FBitSet ( pevAttacker->flags, FL_CLIENT ) &&
					 FBitSet ( pev->spawnflags, SF_BREAK_CROWBAR ) && (bitsDamageType & DMG_CLUB))
				flDamage = pev->health;
	}
	else
	// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - ( pev->absmin + ( pev->size * 0.5 ) );
	}
	
	if (!IsBreakable())
		return 0;

	// si el breakeable tiene el flag de 'solo semtex' y es dañado por uno, romperlo
	//fix: el atacante es el cliente o el semtex mismo? sacarlo por las dudas
	if ( FBitSet ( pev->spawnflags, SF_BREAK_BYSEMTEX ))
	{
		ALERT( at_console, "SF_BREAK_BYSEMTEX spawnflag detected in -func_break-\n" );

		if ((bitsDamageType & DMG_CRUSH))
		{
			ALERT( at_console, "-func_break- killed by semtex!\n" );
			flDamage = pev->health;	
		}
		else
		{
			ALERT( at_console, "-func_break- can't be damaged!\n" );
			flDamage = 0;	
		}
	}
	else
	{
	//	ALERT( at_console, "-func_break- is not damaged by semtex!\n" );
//		ALERT( at_console, "-func_break- is damaged!\n" );
	}

	// Breakables take double damage from the crowbar
	if ( bitsDamageType & DMG_CLUB )
		flDamage *= 2;

	// Boxes / glass / etc. don't take much poison damage, just the impact of the dart - consider that 10%
	if ( bitsDamageType & DMG_POISON )
		flDamage *= 0.1;

// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();
		
// do the damage
	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		//movido a Die() func
	
		Killed( pevAttacker, GIB_NORMAL );
/*
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", pev->origin, TRUE );
		pSprite->AnimateAndDie( RANDOM_FLOAT( 8.0, 12.0 ) );//framerate
		pSprite->SetTransparency( kRenderTransAlpha, 50, 50, 50, 255, kRenderFxNone );
		pSprite->SetScale( 5.0 );
		pSprite->Expand( RANDOM_FLOAT( 10, 15 ), RANDOM_FLOAT( 18.0, 26.0 )  );//expand
		pSprite->pev->frame = 0;
*/
	//	SetThink ( Die );		
	//	pev->nextthink = gpGlobals->time + pev->speed;

	//	pev->takedamage = DAMAGE_NO;
	//sys test
		Die();
		return 0;
	}

	// Make a shard noise each time func breakable is hit.
	// Don't play shard noise if cbreakable actually died.

	DamageSound();

	return 1;
}


void CBreakable::Die( void )
{	
//	Killed( pevAttacker, GIB_NORMAL );

	//esto es lo mismo q la funcion Killed()

	//movido a TakeDamage() func, para decir que ya no puede recibir mas daño
//	pev->takedamage = DAMAGE_NO;

//	pev->deadflag = DEAD_DEAD;
//	UTIL_Remove( this );

	Vector vecSpot;// shard origin
	Vector vecVelocity;// shard velocity
	CBaseEntity *pEntity = NULL;
	char cFlag = 0;
	int pitch;
	float fvol;
	
	pitch = 95 + RANDOM_LONG(0,29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	// The more negative pev->health, the louder
	// the sound should be.

	fvol = RANDOM_FLOAT(0.85, 1.0) + (abs(pev->health) / 100.0);

	if (fvol > 1.0)
		fvol = 1.0;

	switch (m_Material)
	{
	case matGlass:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_GLASS;
		break;

	case matWood:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_WOOD;
		break;

	case matComputer:
	case matMetal:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_METAL;
		break;

	case matFlesh:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_FLESH;
		break;

	case matRocks:
	case matCinderBlock:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_CONCRETE;
		break;

	case matCeilingTile:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustceiling.wav", fvol, ATTN_NORM, 0, pitch);
		break;
	}
    
		
	if (m_Explosion == expDirected)
		vecVelocity = g_vecAttackDir * 200;
	else
	{
		vecVelocity.x = 0;
		vecVelocity.y = 0;
		vecVelocity.z = 0;
	}

	vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSpot );
		WRITE_BYTE( TE_BREAKMODEL);

		// position
		WRITE_COORD( vecSpot.x );
		WRITE_COORD( vecSpot.y );
		WRITE_COORD( vecSpot.z );

		// size
		WRITE_COORD( pev->size.x);
		WRITE_COORD( pev->size.y);
		WRITE_COORD( pev->size.z);

		// velocity
		WRITE_COORD( vecVelocity.x ); 
		WRITE_COORD( vecVelocity.y );
		WRITE_COORD( vecVelocity.z );

		// randomization
		WRITE_BYTE( 10 ); 

		// Model
		WRITE_SHORT( m_idShard );	//model id#

		// # of shards
		WRITE_BYTE( 0 );	// let client decide

		// duration
		WRITE_BYTE( 25 );// 2.5 seconds

		// flags
		WRITE_BYTE( cFlag );
	MESSAGE_END();

	float size = pev->size.x;
	if ( size < pev->size.y )
		size = pev->size.y;
	if ( size < pev->size.z )
		size = pev->size.z;

	// !!! HACK  This should work!
	// Build a box above the entity that looks like an 8 pixel high sheet
	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	// BUGBUG -- can only find 256 entities on a breakable -- should be enough
	CBaseEntity *pList[256];
	int count = UTIL_EntitiesInBox( pList, 256, mins, maxs, FL_ONGROUND );
	if ( count )
	{
		for ( int i = 0; i < count; i++ )
		{
			ClearBits( pList[i]->pev->flags, FL_ONGROUND );
			pList[i]->pev->groundentity = NULL;
		}
	}

	// Don't fire something that could fire myself
	pev->targetname = 0;
	pev->solid = SOLID_NOT;

	// Fire targets on break
	SUB_UseTargets( NULL, USE_TOGGLE, 0 );

	SetThink( SUB_Remove );
	pev->nextthink = pev->ltime + 0.1;
	if ( m_iszSpawnObject )
		CBaseEntity::Create( (char *)STRING(m_iszSpawnObject), VecBModelOrigin(pev), pev->angles, edict() );


	if ( Explodable() )
	{
		ExplosionCreate( Center(), pev->angles, edict(), ExplosionMagnitude(), TRUE );
	}
}



BOOL CBreakable :: IsBreakable( void ) 
{ 
	return m_Material != matUnbreakableGlass;
}


int	CBreakable :: DamageDecal( int bitsDamageType )
{
	if ( m_Material == matGlass  )
		return DECAL_GLASSBREAK1 + RANDOM_LONG(0,2);

	if ( m_Material == matUnbreakableGlass )
		return DECAL_BPROOF1;

	return CBaseEntity::DamageDecal( bitsDamageType );
}

class CPushable : public CBreakable
{
public:
	void	Spawn ( void );
	void	Precache( void );
	void	Touch ( CBaseEntity *pOther );
	void	Move( CBaseEntity *pMover, int push );
	void	KeyValue( KeyValueData *pkvd );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT StopSound( void );
//	virtual void	SetActivator( CBaseEntity *pActivator ) { m_pPusher = pActivator; }

	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	inline float MaxSpeed( void ) { return m_maxSpeed; }
	
	// breakables use an overridden takedamage
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	
	static	TYPEDESCRIPTION m_SaveData[];

	static char *m_soundNames[3];
	int		m_lastSound;	// no need to save/restore, just keeps the same sound from playing twice in a row
	float	m_maxSpeed;
	float	m_soundTime;
};

TYPEDESCRIPTION	CPushable::m_SaveData[] = 
{
	DEFINE_FIELD( CPushable, m_maxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPushable, m_soundTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CPushable, CBreakable );

LINK_ENTITY_TO_CLASS( func_pushable, CPushable );

char *CPushable :: m_soundNames[3] = { "debris/pushbox1.wav", "debris/pushbox2.wav", "debris/pushbox3.wav" };


void CPushable :: Spawn( void )
{
	if ( pev->spawnflags & SF_PUSH_BREAKABLE )
		CBreakable::Spawn();
	else
		Precache( );

	pev->movetype	= MOVETYPE_PUSHSTEP;
	pev->solid		= SOLID_BBOX;
	SET_MODEL( ENT(pev), STRING(pev->model) );

	if ( pev->friction > 399 )
		pev->friction = 399;

	m_maxSpeed = 400 - pev->friction;
	SetBits( pev->flags, FL_FLOAT );
	pev->friction = 0;
	
	pev->origin.z += 1;	// Pick up off of the floor
	UTIL_SetOrigin( pev, pev->origin );

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = ( pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y) ) * 0.0005;
	m_soundTime = 0;
}


void CPushable :: Precache( void )
{
	for ( int i = 0; i < 3; i++ )
		PRECACHE_SOUND( m_soundNames[i] );

	if ( pev->spawnflags & SF_PUSH_BREAKABLE )
		CBreakable::Precache( );
}


void CPushable :: KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "size") )
	{
		int bbox = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch( bbox )
		{
		case 0:	// Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN*2, VEC_DUCK_HULL_MAX*2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}

	}
	else if ( FStrEq(pkvd->szKeyName, "buoyancy") )
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBreakable::KeyValue( pkvd );
}


// Pull the func_pushable
void CPushable :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
	{
		if ( pev->spawnflags & SF_PUSH_BREAKABLE )
			this->CBreakable::Use( pActivator, pCaller, useType, value );
		return;
	}

	if ( pActivator->pev->velocity != g_vecZero )
		Move( pActivator, 0 );
}


void CPushable :: Touch( CBaseEntity *pOther )
{
	if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		return;

	Move( pOther, 1 );
}


void CPushable :: Move( CBaseEntity *pOther, int push )
{
	entvars_t*	pevToucher = pOther->pev;
	int playerTouch = 0;

	// Is entity standing on this pushable ?
	if ( FBitSet(pevToucher->flags,FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev )
	{
		// Only push if floating
		if ( pev->waterlevel > 0 )
			pev->velocity.z += pevToucher->velocity.z * 0.1;

		return;
	}


	if ( pOther->IsPlayer() )
	{
		if ( push && !(pevToucher->button & (IN_FORWARD|IN_USE)) )	// Don't push unless the player is pushing forward and NOT use (pull)
			return;
		playerTouch = 1;
	}

	float factor;

	if ( playerTouch )
	{
		if ( !(pevToucher->flags & FL_ONGROUND) )	// Don't push away from jumping/falling players unless in water
		{
			if ( pev->waterlevel < 1 )
				return;
			else 
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else 
		factor = 0.25;

	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;

	float length = sqrt( pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y );
	if ( push && (length > MaxSpeed()) )
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length );
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length );
	}
	if ( playerTouch )
	{
		pevToucher->velocity.x = pev->velocity.x;
		pevToucher->velocity.y = pev->velocity.y;
		if ( (gpGlobals->time - m_soundTime) > 0.7 )
		{
			m_soundTime = gpGlobals->time;
			if ( length > 0 && FBitSet(pev->flags,FL_ONGROUND) )
			{
				m_lastSound = RANDOM_LONG(0,2);
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound], 0.5, ATTN_NORM);
	//			SetThink( StopSound );
	//			pev->nextthink = pev->ltime + 0.1;
			}
			else
				STOP_SOUND( ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound] );
		}
	}
}

#if 0
void CPushable::StopSound( void )
{
	Vector dist = pev->oldorigin - pev->origin;
	if ( dist.Length() <= 0 )
		STOP_SOUND( ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound] );
}
#endif

int CPushable::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if ( pev->spawnflags & SF_PUSH_BREAKABLE )
		return CBreakable::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );

	return 1;
}


/****************

  There is my new entity func_phy". 

  The original code is made by Max Fooz. It's the same code but it uses the func_pushable
  entity for that. The problem is if the pushable doesn't have any 'origin' brush, you will
  see a lot of weird things. 
  The code need a entity whit a origin, and you need to edit the map for that... so, why
  can't we do a new entity? There is... "func_phy", to simulate physic in half-life!

  I expanded the code and make this: 
  
	-New 5 materials
	-New .fgd entry to specific the material of the entity.
	-Specifics sounds that depends of the material.
	-Sparks, and simple effects when you hit the entity, and depends of the material too.
	-The materials has your own weight. A plaster cube is lighter than a metal barrel.

  UNDONE:
	[] Fix the bug whit the angles.
	[] Hurt the player or monsters, when the entity impacts them.For example: If a rock fall in a
	headcrab, it will smashed!
	[] Change the steps sounds, when a player is walking over the entity.
	[] Set a custom material sound. So... the mapper can change the sounds. It's could be good.
	[] Change the fgd, to make custom options:
		* can't hurt (the entity can't hurt anything)
		* can float (the entity can float in the water -i.e. a plastic barrel)
		* can't move (??? I don't kwon... if the mapper want to make this...)
		* can't push (the player can't pull it)
	[] Make a option to use models, not brush-based.
	[] Still expanding the code. I don't kwon... do a new things...

																						SysOp:
																						-----
****************/
class CPhy : public CBreakable
{
public:
	void	Spawn ( void );
	void	Precache( void );
	void	EXPORT PushableThink( void );
	void	Touch ( CBaseEntity *pOther );
	void	Move( CBaseEntity *pMover, int push );
	void	KeyValue( KeyValueData *pkvd );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT StopSound( void );
	int  Classify( void ) { return CLASS_FACTION_C; };//??????????

	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

		
	void EXPORT		Die( void );
	void TestBeam( Vector org, Vector end, Vector color, int life, int width );

	inline float MaxSpeed( void ) { return m_maxSpeed; }
	
	// breakables use an overridden takedamage
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	
	static	TYPEDESCRIPTION m_SaveData[];

	int		m_lastSound;	// no need to save/restore, just keeps the same sound from playing twice in a row
	float	m_maxSpeed;
	float	m_soundTime;
};

TYPEDESCRIPTION	CPhy::m_SaveData[] = 
{
	DEFINE_FIELD( CPhy, m_maxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPhy, m_soundTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CPhy, CBreakable );

LINK_ENTITY_TO_CLASS( func_phy, CPhy );

void CPhy :: Spawn( void )
{
	Vector vecMins = pev->mins;
	Vector vecMaxs = pev->maxs;

	CBreakable::Spawn();

	//sys movido aca
	if ( pev->speed == 0 )
	{// !!!BUGBUG - why doesn't zero delay work?
		pev->speed = 0.1;
	}

	if ( pev->speed == -1 )//random
	{				
		switch ( RANDOM_LONG(0,4) )
		{
			case 0:	pev->speed = 1;	break;
			case 1:	pev->speed = 1.5; break;
			case 2:	pev->speed = 2;	break;
			case 3:	pev->speed = 2.5; break;
			case 4:	pev->speed = 3;	break;
		}
		
	}

	if ( FStringNull( pev->speed ))
		pev->speed = 0.1;
	
	if ( FStringNull( pev->health ))
		pev->health = 999999;//Fix para mapas con anteriores entidades func_phy

	if ( FStringNull( pev->gravity ))
		pev->gravity = 1;//Fix para mapas con anteriores entidades func_phy

	if ( FStringNull( pev->friction ))
		pev->friction = 0.5;//Fix para mapas con anteriores entidades func_phy

	pev->movetype	= MOVETYPE_BOUNCE;//MOVETYPE_PUSHSTEP;
	pev->solid		= SOLID_BBOX;


	//The order must work?
	if (pev->spawnflags & SF_MOVETYPE_PUSHSTEP)
	pev->movetype	= MOVETYPE_NONE;

//	if (pev->spawnflags & SF_SOLID_BSP)
//	pev->solid		= SOLID_BSP;

	SET_MODEL( ENT(pev), STRING(pev->model) );

	if ( pev->friction > 399 )
		pev->friction = 399;

	m_maxSpeed = 400 - pev->friction;
	SetBits( pev->flags, FL_FLOAT );

//	pev->friction = 0.5;

	//SYS: Now this is FGD-able
//	pev->gravity = 0.1;

	pev->gravity = CVAR_GET_FLOAT("phys_gravity");
	pev->friction = CVAR_GET_FLOAT("phys_friction");
	
	pev->origin.z += 1;	// Pick up off of the floor

	if (pev->spawnflags & SF_DONT_PICKUPFLOOR)//FGd fix
	pev->origin.z = 0;	// Dont Pick up off of the floor

	UTIL_SetOrigin( pev, pev->origin );

	SetThink(&CPhy:: PushableThink );
	pev->nextthink = 0.1;

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = ( pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y) ) * 0.0005;
	m_soundTime = 0;
}


void CPhy :: Precache( void )
{
//UNDONE: do a specific precaches, that depends of the material. Don't precache aaaalll the sounds...

//PRECACHE STUFF
//0 - no sound
//1 - metal
//2 - plaster
//3 - glass
//4 - concrete
//5 - wood

		//wood
			PRECACHE_SOUND("fisica/madera/move1.wav");
			PRECACHE_SOUND("fisica/madera/move2.wav");
			PRECACHE_SOUND("fisica/madera/impact1.wav");
			PRECACHE_SOUND("fisica/madera/impact2.wav");

		//concrete sounds
		PRECACHE_SOUND("fisica/concreto/move1.wav");
		PRECACHE_SOUND("fisica/concreto/move2.wav");
		PRECACHE_SOUND("debris/concrete1.wav");
		PRECACHE_SOUND("debris/concrete3.wav");

		//metal
		PRECACHE_SOUND("fisica/metal/impact1.wav");
		PRECACHE_SOUND("fisica/metal/impact2.wav");
			PRECACHE_SOUND("fisica/metal/move1.wav");
			PRECACHE_SOUND("fisica/metal/move2.wav");
		PRECACHE_SOUND("fisica/metal/b_impact1.wav");
		PRECACHE_SOUND("fisica/metal/b_impact2.wav");
		PRECACHE_SOUND("fisica/metal/b_impact3.wav");

		//plastic sounds
		PRECACHE_SOUND("fisica/plastico/impact1.wav");
		PRECACHE_SOUND("fisica/plastico/impact2.wav");
		PRECACHE_SOUND("fisica/plastico/impact3.wav");
			PRECACHE_SOUND("fisica/plastico/move1.wav");
			PRECACHE_SOUND("fisica/plastico/move2.wav");

		CBreakable::Precache( );
}


void CPhy :: KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "size") )
	{
		int bbox = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch( bbox )
		{
		case 0:	// Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN*2, VEC_DUCK_HULL_MAX*2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}

	}
	else if ( FStrEq(pkvd->szKeyName, "buoyancy") )
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBreakable::KeyValue( pkvd );
}


// Pull the func_pushable
void CPhy :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
	{
			this->CBreakable::Use( pActivator, pCaller, useType, value );
		return;
	}

	if ( pActivator->pev->velocity != g_vecZero )
		Move( pActivator, 0 );
}

/*
===================
AngleBetweenVectors

===================
*/

float AngleBetweenVectors( const vec3_t v1, const vec3_t v2 )
{
	float angle;
	float l1 = v1.Length();
	float l2 = v2.Length();

	if ( !l1 || !l2 )
		return 0.0f;

	angle = acos( DotProduct( v1, v2 ) / (l1*l2) );
	angle = ( angle  * 180.0f ) / M_PI;

	return angle;
}

void NormalizeAngles( float *angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

void CPhy :: PushableThink( void )
{
	if (pev->health <= 0)
	{
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", pev->origin, TRUE );
					
		pSprite->SetTransparency( kRenderTransAlpha, 222, 222, 222, 88, kRenderFxNone );
		pSprite->SetScale( 5.0 );
		pSprite->Expand( 5 , RANDOM_FLOAT( 40.0, 60.0 )  );
		pSprite->pev->frame = 0;
	}

//	ALERT( at_debug, "Think !\n" );
//	pev->velocity = pev->velocity * 0.8;
//	pev->avelocity = pev->avelocity * 0.8;
//	ALERT( at_debug, "Angles yaw: %3.2f\n", pev->angles.y );

	//test
	/*
if( pev->angles.x >= 360 )
		pev->angles.x -= 360;
	if( pev->angles.x >= -360 )
		pev->angles.x += 360;
		*/


	if( pev->flags & FL_ONGROUND )
	{
		
		//pev->avelocity = pev->avelocity * 0.5;

	}

//	SetNextThink( 0.1 );
	pev->nextthink = 0.1;
}

void CPhy :: Touch( CBaseEntity *pOther )
{
	Vector savedangles = Vector( 0, 0, 0 );
	int negate = 0;
	TraceResult tr;
	// look down directly to know the surface we're lying.
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );

	//ALERT( at_debug, "Velocity Touch : %3.2f\n", pev->velocity.x );
	pev->velocity = pev->velocity * 0.8;
//	pev->avelocity = pev->avelocity * 0.5;

	if( !(pev->flags & FL_ONGROUND) )
	{
		//pev->avelocity.x = RANDOM_FLOAT( -400, 400 );//fooz
		if (pev->frags == 0)
		{
			//no sound
		}
/*****
Sys:
This code will do a impact sounds when the entity colides whit another entity or worldspawn, or it's damaged.
UNDONE: Use a switch func, to make a code more easy to see.
*****/
		if (pev->frags == 1) //metal
		{
			if( RANDOM_LONG( 0, 1 ))
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/metal/impact1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/metal/impact2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		}

		if (pev->frags == 2) //plaster
		{			
			switch (RANDOM_LONG(0,2)) 
			{
				case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/plastico/impact1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/plastico/impact2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 2:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/plastico/impact3.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
			}			
		}
		
		if (pev->frags == 3) //glass
		{
			if( RANDOM_LONG( 0, 1 ))
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/glass3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/glass1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		}

		if (pev->frags == 4) //concrete
		{
			if( RANDOM_LONG( 0, 1 ))
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/concrete1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/concrete3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		}

		if (pev->frags == 5) //wood
		{
			switch (RANDOM_LONG(0,1)) 
			{
				case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/madera/impact1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "fisica/madera/impact2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
			}
		}
	}
	else
/******
This code is nice, but has a bug: 
The angles of the entity it's normalized, but if you touch or move the entity, and if it have another initial
angle, the entity will move too quickly in the reverse angle.

Look that:
1- We have a stand barrel.

	------
	||||||
	|    |
	|    |
	|    |
	------

2- and you hit the barrel and the barrel fall of and it's lie on the ground

  -----------
  |	 	   ||
  |		   ||
  -----------

3- Well, do a simple touch to the barrel...

  -----------
  || 	    |
  ||		|
  -----------

If you have a barrel, don't make any diference, but if you have a more complex object (a monitor i suposed), 
you will see the bug.

It's too hard to explain, you need see it...
******/
	{	
		for( int i = 0; i<3; i++ )
		{
		if( pev->angles.x < 0 )
			negate = 1;

			if( fabs(pev->angles.x) < 45 )
				savedangles.x = 0;
			else if( fabs(pev->angles.x) >= 45 && fabs(pev->angles.x) <= 135 )
				savedangles.x = 90;
			else if( fabs(pev->angles.x) > 135 && fabs(pev->angles.x) <= 180 )
				savedangles.x = 180;
		}

		#ifndef M_PI
		#define M_PI 3.14159265358979
		#endif
		#define ang2rad (2 * M_PI / 360)

          if ( tr.flFraction < 1.0 )
          {
			Vector forward, right, angdir, angdiry;
			Vector Angles = pev->angles;
			//Fooz
			NormalizeAngles( Angles );
		
			UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
			angdir = forward;
			Vector left = -right;
			angdiry = left;

			pev->angles.x = -UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
		
			pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			
//			pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * right ).x;
          }

		#undef ang2rad
		  
		  if( negate )
			pev->angles.x -= savedangles.x;
		  else
			pev->angles.x += savedangles.x;
	}


	if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		return;

	Move( pOther, 1 );
}


void CPhy :: Move( CBaseEntity *pOther, int push )
{
	entvars_t*	pevToucher = pOther->pev;
	int playerTouch = 0;

	// Is entity standing on this pushable ?
	if ( FBitSet(pevToucher->flags,FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev )
	{
		// Only push if floating
		if ( pev->waterlevel > 0 && pev->watertype > CONTENT_FLYFIELD)
			pev->velocity.z += pevToucher->velocity.z * 0.1;

		return;
	}

	if ( pOther->IsPlayer() )
	{
		if ( push && !(pevToucher->button & (IN_FORWARD|IN_USE)) )	// Don't push unless the player is pushing forward and NOT use (pull)
			return;
		playerTouch = 1;
	}

	float factor;

	if ( playerTouch )
	{
		if ( !(pevToucher->flags & FL_ONGROUND) )	// Don't push away from jumping/falling players unless in water
		{
			if ( pev->waterlevel < 1 || pev->watertype <= CONTENT_FLYFIELD)
				return;
			else 
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else 
		factor = 0.25;

	if (!push)
		factor = factor*0.5;

	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;

	float length = sqrt( pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y );
	if ( push && (length > MaxSpeed()) )
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length );
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length );
	}
	if ( playerTouch )
	{
		pevToucher->velocity.x = pev->velocity.x;
		pevToucher->velocity.y = pev->velocity.y;
		if ( (gpGlobals->time - m_soundTime) > 0.7 )
		{
			m_soundTime = gpGlobals->time;
			if ( length > 0 && FBitSet(pev->flags,FL_ONGROUND) )
			{
/******			
When you move the entity, it will make a sound. 

There are 6 materials type:	 
	-No defined (no material) The entity doesn't make any sound.
	-Metal (metal rough sound)
	-Plaster (a plastic tube sound)
	-Glass (UNDONE: glass move sound??)
	-Cement (concrete sound)
	-Wood (wood sound)
*******/
				if (pev->frags == 0)
				{
					//no sound
				}

				if (pev->frags == 1) //metal
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/metal/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/metal/move2.wav", 0.5, ATTN_NORM);
					
				}

				if (pev->frags == 2) //plaster
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/plastico/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/plastico/move2.wav", 0.5, ATTN_NORM);
				}
				
				if (pev->frags == 3) { }//glass

				if (pev->frags == 4) //cemento
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/concreto/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/concreto/move2.wav", 0.5, ATTN_NORM);
				}

				if (pev->frags == 5) //wood
				{
					if (RANDOM_LONG(0,1))
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/madera/move1.wav", 0.5, ATTN_NORM);
					else
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "fisica/madera/move2.wav", 0.5, ATTN_NORM);
				}
		//		SetThink( StopSound );
		//		pev->nextthink = 0.1;
			}
		}
	}
}

#if 0
void CPhy::StopSound( void )
{
	Vector dist = pev->oldorigin - pev->origin;
	if ( dist.Length() <= 0 )
		STOP_SOUND( ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound] );
}
#endif
//A Fooz's Testbeam func expanded by me. Why? I needed a realistic sparks. Anyways, don't like me the lasers.
//I don't use this function. But, you can test your entity whit that...
void CPhy :: TestBeam( Vector org, Vector end, Vector color, int life, int width )
{
		extern short g_sModelIndexLaser;
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_BEAMPOINTS );
			WRITE_COORD( org.x );
			WRITE_COORD( org.y );
			WRITE_COORD( org.z );

			WRITE_COORD( end.x );
			WRITE_COORD( end.y );
			WRITE_COORD( end.z );
			WRITE_SHORT( g_sModelIndexLaser );
			WRITE_BYTE( 0 ); // framerate
			WRITE_BYTE( 0 ); // framerate

			WRITE_BYTE( life ); // life
			WRITE_BYTE( width );  // width

			WRITE_BYTE( 0 );   // noise
			WRITE_BYTE( color.x );   // r, g, b
			WRITE_BYTE( color.y );   // r, g, b
			WRITE_BYTE( color.z );   // r, g, b
			WRITE_BYTE( 160 );	// brightness
			WRITE_BYTE( 0 );		// speed
		MESSAGE_END();
}

int CPhy :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if (pev->spawnflags & SF_TAKEDAMAGE_NO)
		return 0;//no damage

//	if (!(pev->spawnflags & SF_SOLID_BSP))
	pev->movetype	= MOVETYPE_BOUNCE;//This should work?

	Vector			vecDir, r, anorm, rforward, rup, rright;
//	float a;
	float force = flDamage * 10;
	TraceResult trace = UTIL_GetGlobalTrace( );
	//UTIL_MakeVectors( pev->angles );

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = r = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{			
		if ( FClassnameIs( pevInflictor, "projectile" ) )
			pevInflictor = VARS( pevInflictor->owner );

		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{	
			vecDir = g_vecAttackDir = ( trace.vecEndPos - pInflictor->Center() ).Normalize();
			r = ( trace.vecEndPos - Center() ).Normalize();
		}
	}

	anorm = UTIL_VecToAngles( r );
	anorm.x = -anorm.x;
	UTIL_MakeVectorsPrivate( anorm, rforward, rright, rup );

/*****
Let's do a material sound when you hit the entity and big sparks when you hit the material 'metal'
This only work whit certain damage types, like a crowbar-knife hit, o a bullet impact.
******/

			float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range
			BOOL	b_CanMakeParticles;

			if ( CVAR_GET_FLOAT( "r_paintball" ) == 0 )
			if ( CVAR_GET_FLOAT( "cl_wallpuff" ) >= 1 )
			{			
					if (pev->frags == 1) //metal
					{								
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
															
						b_CanMakeParticles = FALSE;
					}
					if (pev->frags == 5) //wood
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/wood1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/wood2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/wood3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
					}
					else
					{
						switch ( RANDOM_LONG(0,2) )
						{
							case 0: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/concrete1.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 1: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/concrete2.wav", flVolume, ATTN_NORM, 0, 100); break;
							case 2: UTIL_EmitAmbientSound(ENT(0), trace.vecEndPos, "weapons/bulletimpact/concrete3.wav", flVolume, ATTN_NORM, 0, 100); break;
						}
					}	
					
					if (b_CanMakeParticles)
					{
						if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
						{
							MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
								WRITE_SHORT(0);
								WRITE_BYTE(0);
								WRITE_COORD( trace.vecEndPos.x );
								WRITE_COORD( trace.vecEndPos.y );
								WRITE_COORD( trace.vecEndPos.z );
								WRITE_COORD( 0 );
								WRITE_COORD( 0 );
								WRITE_COORD( 0 );
								if (pev->frags == 5) //wood
								WRITE_SHORT(iDefaultHitWood1);
								else
								WRITE_SHORT(iDefaultWallSmoke);
							MESSAGE_END();
						}
					}
				}//eo cvar check
/*
	if (pev->frags == 1) //metal
	{
		//if ( bitsDamageType & (DMG_BULLET | DMG_CLUB) )
		if ( ( bitsDamageType & DMG_BULLET)|| ( bitsDamageType & DMG_CLUB) )
		{
			UTIL_Ricochet( trace.vecEndPos, 0.5 );
			
			switch (RANDOM_LONG(0,2)) 
			{
				case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "fisica/metal/b_impact1.wav", 0.9, ATTN_NORM); break;
				case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "fisica/metal/b_impact2.wav", 0.9, ATTN_NORM); break;
				case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "fisica/metal/b_impact3.wav", 0.9, ATTN_NORM); break;			
			}

		//int color, int count, int speed, int velocityRange//PARAMs

//The big white sparks... I use my own function, but you can use the StreakSplash() to do a similar sparks
//Just copy the func in gargantua.cpp, and paste in this file. Remember! You must declare the func too! 

//	StreakSplash( trace.vecEndPos, trace.vecPlaneNormal, 0, 5, 500, 500 );
			if (RANDOM_LONG( 0, 99 ) < 40)
			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 0, 5, 500, 500 );//chispas

			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 9, 5, 5, 100 );//puntos
			UTIL_WhiteSparks( trace.vecEndPos, trace.vecPlaneNormal, 0, 5, 500, 20 );//chispas

		}
	}
	
	if (pev->frags == 3) //glass
		UTIL_Ricochet( trace.vecEndPos, 0.5 );

	if (pev->frags == 4) //cement
		UTIL_Sparks( trace.vecEndPos );
	
	if (pev->frags == 5) //wood
	{
		switch (RANDOM_LONG(0,2)) 
		{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "debris/wood1.wav", 0.9, ATTN_NORM); break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "debris/wood2.wav", 0.9, ATTN_NORM); break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "debris/wood3.wav", 0.9, ATTN_NORM); break;			
		}
	}
*/
//	if ( bitsDamageType & DMG_BULLET )
/*******
Sys: What the ....

If the material is "plaster" let's do more movement.
*******/
	if (pev->frags == 2) //plastico
	{
		force *= 1;//0.8
		pev->avelocity.z = cos( AngleBetweenVectors( vecDir, rup ) ) * force * 1;//fooz	(yeah! that work)
	}
	else// Isn't plaster
	{
		if ( ( bitsDamageType & DMG_BULLET)|| ( bitsDamageType & DMG_CLUB) )
			force *= 0.5;
	}
	
/*
	CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", trace.vecEndPos, TRUE );
					
	pSprite->SetTransparency( kRenderTransAlpha, 222, 222, 222, 111, kRenderFxNone );
	pSprite->SetScale( 5.0 );
	pSprite->Expand( 5 , RANDOM_FLOAT( 140.0, 160.0 )  );
*/
	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 1;

	pev->avelocity.x = cos( AngleBetweenVectors( vecDir, rup ) ) * 100;
	pev->avelocity.y = cos( AngleBetweenVectors( vecDir, -rright ) ) * 200;

	ALERT( at_console, "X : %3.1f %3.1f° Y: %3.1f %3.1f°\n", pev->avelocity.x, AngleBetweenVectors( vecDir, rup ), pev->avelocity.y, AngleBetweenVectors( vecDir, -rright ) );

//	pev->velocity = pev->velocity /*+ gpGlobals->v_up * force * RANDOM_FLOAT( 0, 0.5 )*/ + vecDir * force * RANDOM_FLOAT( 0.5, 1.0 );

	pev->velocity = pev->velocity + gpGlobals->v_up * force * RANDOM_FLOAT( 0, 0.5 ) + vecDir * force * RANDOM_FLOAT( 0.5, 1.0 );

	//sys test:
	//return 1:damage was done

	//**************************************************************************************
		//if ( pev->spawnflags & SF_PUSH_BREAKABLE )
//	CBreakable::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
		
	Vector	vecTemp;

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if ( pevAttacker == pevInflictor )	
	{
		vecTemp = pevInflictor->origin - ( pev->absmin + ( pev->size * 0.5 ) );
				
		// if a client hit the breakable with a crowbar, and breakable is crowbar-sensitive, break it now.
			if ( FBitSet ( pevAttacker->flags, FL_CLIENT ) &&
					 FBitSet ( pev->spawnflags, SF_BREAK_CROWBAR ) && (bitsDamageType & DMG_CLUB))
				flDamage = pev->health;
	}
	else
	// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - ( pev->absmin + ( pev->size * 0.5 ) );
	}
	
	if (!IsBreakable())
		return 0;

	// Breakables take double damage from the crowbar
	if ( bitsDamageType & DMG_CLUB )
		flDamage *= 2;

	// Boxes / glass / etc. don't take much poison damage, just the impact of the dart - consider that 10%
	if ( bitsDamageType & DMG_POISON )
		flDamage *= 0.1;

	g_vecAttackDir = vecTemp.Normalize();
		
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		SetThink ( Die );		
		pev->nextthink = gpGlobals->time + pev->speed;

		pev->takedamage = DAMAGE_NO;
		return 0;
	}

	DamageSound();

	return 1;
}

void CPhy::Die( void )
{	
//	Killed( pevAttacker, GIB_NORMAL );

	//esto es lo mismo q la funcion Killed()

	//movido a TakeDamage() func, para decir que ya no puede recibir mas daño
//	pev->takedamage = DAMAGE_NO;

	pev->deadflag = DEAD_DEAD;
	UTIL_Remove( this );

	Vector vecSpot;// shard origin
	Vector vecVelocity;// shard velocity
	CBaseEntity *pEntity = NULL;
	char cFlag = 0;
	int pitch;
	float fvol;
	
	pitch = 95 + RANDOM_LONG(0,29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	// The more negative pev->health, the louder
	// the sound should be.

	fvol = RANDOM_FLOAT(0.85, 1.0) + (abs(pev->health) / 100.0);

	if (fvol > 1.0)
		fvol = 1.0;

	switch (m_Material)
	{
	case matGlass:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_GLASS;
		break;

	case matWood:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_WOOD;
		break;

	case matComputer:
	case matMetal:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_METAL;
		break;

	case matFlesh:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_FLESH;
		break;

	case matRocks:
	case matCinderBlock:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete1.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete2.wav", fvol, ATTN_NORM, 0, pitch);	
			break;
		}
		cFlag = BREAK_CONCRETE;
		break;

	case matCeilingTile:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustceiling.wav", fvol, ATTN_NORM, 0, pitch);
		break;
	}
    
		
	if (m_Explosion == expDirected)
		vecVelocity = g_vecAttackDir * 200;
	else
	{
		vecVelocity.x = 0;
		vecVelocity.y = 0;
		vecVelocity.z = 0;
	}

	//Sys0p: that's why the BounceGibTouch() is not called... :(
	vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSpot );
		WRITE_BYTE( TE_BREAKMODEL);

		// position
		WRITE_COORD( vecSpot.x );
		WRITE_COORD( vecSpot.y );
		WRITE_COORD( vecSpot.z );

		// size
		WRITE_COORD( pev->size.x);
		WRITE_COORD( pev->size.y);
		WRITE_COORD( pev->size.z);

		// velocity
		WRITE_COORD( vecVelocity.x ); 
		WRITE_COORD( vecVelocity.y );
		WRITE_COORD( vecVelocity.z );

		// randomization
		WRITE_BYTE( 10 ); 

		// Model
		WRITE_SHORT( m_idShard );	//model id#

		// # of shards
		WRITE_BYTE( 0 );	// let client decide

		// duration
		WRITE_BYTE( 25 );// 2.5 seconds

		// flags
		WRITE_BYTE( cFlag );
	MESSAGE_END();

	float size = pev->size.x;
	if ( size < pev->size.y )
		size = pev->size.y;
	if ( size < pev->size.z )
		size = pev->size.z;

	// !!! HACK  This should work!
	// Build a box above the entity that looks like an 8 pixel high sheet
	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	// BUGBUG -- can only find 256 entities on a breakable -- should be enough
	CBaseEntity *pList[256];
	int count = UTIL_EntitiesInBox( pList, 256, mins, maxs, FL_ONGROUND );
	if ( count )
	{
		for ( int i = 0; i < count; i++ )
		{
			ClearBits( pList[i]->pev->flags, FL_ONGROUND );
			pList[i]->pev->groundentity = NULL;
		}
	}

	// Don't fire something that could fire myself
	pev->targetname = 0;
	pev->solid = SOLID_NOT;

	// Fire targets on break
	SUB_UseTargets( NULL, USE_TOGGLE, 0 );

	SetThink( SUB_Remove );
	pev->nextthink = pev->ltime + 0.1;
	if ( m_iszSpawnObject )
		CBaseEntity::Create( (char *)STRING(m_iszSpawnObject), VecBModelOrigin(pev), pev->angles, edict() );


	if ( Explodable() )
	{
		ExplosionCreate( Center(), pev->angles, edict(), ExplosionMagnitude(), TRUE );
	}
}

//=======================================================================
// func_lamp - switchable brush light. 
//=======================================================================
//I can't make this work (maybe I should leave this code and start to mapping something...)
#define SF_START_ON				0x1
#define SF_NOTSOLID				0x2

class CFuncLamp : public CBreakable
{
public:
	void Spawn( void );
	void Precache( void ){ CBreakable::Precache(); }
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	void EXPORT Flicker( void );
	void Die( void );
	
	STATE	m_iState;
	virtual STATE GetState( void ) { return m_iState; };
	int				m_iStyle; // LRC - almost anything can have a lightstyle these days...
};
LINK_ENTITY_TO_CLASS( func_lamp, CFuncLamp );

void CFuncLamp :: Spawn( void )
{	
	CBreakable::Spawn();
         
	pev->movetype = MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid    = SOLID_BSP;
	SET_MODEL( ENT(pev), STRING(pev->model) );
	
//	if(!(pev->spawnflags & SF_START_ON)) 
		Use( this, this, USE_OFF, 0 );
}

void CFuncLamp :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{	
	if(m_iState == STATE_OFF)
		return;//lamp is broken

	if (useType == USE_TOGGLE)
	{
		if(m_iState == STATE_OFF) useType = USE_ON;
		else useType = USE_OFF;
	}
	if (useType == USE_ON)
	{
		if(m_flDelay)//make flickering delay
		{
			pev->frame = 0;//light texture is on
			m_iState = STATE_TURN_ON;
			LIGHT_STYLE(m_iStyle, "mmamammmmammamamaaamammma");
			SetThink( Flicker );
			pev->nextthink = gpGlobals->time + m_flDelay;
		}
		else
		{         //instantly enable
			m_iState = STATE_ON;
			pev->frame = 0;//light texture is on
			LIGHT_STYLE(m_iStyle, "k");
			//FireTargets( pev->target, this, this, USE_ON, 0 );//lamp enable
			//FireTargets( "pev->target", this, this, USE_ON, 0 );//LOW
		}
	}
	else if (useType == USE_OFF)
	{
		pev->frame = 1;//light texture is off
		LIGHT_STYLE(m_iStyle, "a");
		//FireTargets( "pev->target", this, this, USE_OFF, 0 );//LOW
		m_iState = STATE_OFF;
	}
	else if (useType == USE_SET)
		Die();//broke lamp with sfx
}

void CFuncLamp::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	if(m_iState == STATE_OFF) return;
	const char *pTextureName;
	Vector start = pevAttacker->origin + pevAttacker->view_ofs;
	Vector end = start + vecDir * 1024;
	edict_t *pWorld = ptr->pHit;
	if ( pWorld )pTextureName = TRACE_TEXTURE( pWorld, start, end );
	if(strstr(pTextureName, "+0~") || strstr(pTextureName, "~") )//take damage only at light texture
	{
		UTIL_Sparks( ptr->vecEndPos );
		pev->oldorigin = ptr->vecEndPos;//save last point of damage
		pev->takedamage = DAMAGE_YES;//inflict damage only at light texture
	}
	else	pev->takedamage = DAMAGE_NO;
	CBreakable::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}
int CFuncLamp::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if ( bitsDamageType & DMG_BLAST ) flDamage *= 3;
	else if ( bitsDamageType & DMG_CLUB) flDamage *= 2;
	else if ( bitsDamageType & DMG_SONIC ) flDamage *= 1.2;
	else if ( bitsDamageType & DMG_SHOCK ) flDamage *= 10;//!!!! over voltage
	else if ( bitsDamageType & DMG_BULLET) flDamage *= 0.5;//half damage at bullet
	pev->health -= flDamage;//calculate health

	if (pev->health <= 0)
	{
		Die();
		return 0;
	}
	CBreakable::DamageSound();

	return 1;
}

void CFuncLamp::Die( void )
{
	//lamp is random choose die style
	if(m_iState == STATE_OFF)
	{
		pev->frame = 1;//light texture is off
		LIGHT_STYLE(m_iStyle, "a");
		DontThink();
	}
	else
	{         //simple randomization
		pev->impulse = RANDOM_LONG(1, 2);
		SetThink( Flicker );
		pev->nextthink = 0.1 + (RANDOM_LONG(1, 2) * 0.1);
	}

	m_iState = STATE_OFF;//lamp is die
	pev->health = 0;//set health to NULL
	pev->takedamage = DAMAGE_NO;
	//FireTargets( "pev->target", this, this, USE_OFF, 0 );//LOW

	switch ( RANDOM_LONG(0,1))
	{
		case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "materials/glass/bustglass1.wav", 0.7, ATTN_IDLE); break;
		case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "materials/glass/bustglass2.wav", 0.8, ATTN_IDLE); break;
	}
	Vector vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
}

void CFuncLamp :: Flicker( void )
{
	if(m_iState == STATE_TURN_ON)//flickering on enable
	{
		LIGHT_STYLE(m_iStyle, "k");
		//FireTargets( "pev->target", this, this, USE_OFF, 0 );//LOW
		m_iState = STATE_ON;
		DontThink();
		return;
	}
	if(pev->impulse == 1)//fadeout on break
	{
		pev->frame = 1;//light texture is off
		LIGHT_STYLE(m_iStyle, "a");
		SetThink( NULL );
		return;
	}
	if(pev->impulse == 2)//broken flickering
	{
		//make different flickering
		switch ( RANDOM_LONG(0,3))
		{
			case 0: LIGHT_STYLE(m_iStyle, "abcaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); break;
			case 1: LIGHT_STYLE(m_iStyle, "acaaabaaaaaaaaaaaaaaaaaaaaaaaaaaa"); break;
			case 2: LIGHT_STYLE(m_iStyle, "aaafbaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); break;
			case 3: LIGHT_STYLE(m_iStyle, "aaaaaaaaaaaaagaaaaaaaaacaaaacaaaa"); break;
		}		
		pev->frags = RANDOM_FLOAT(0.5, 10);
                   	UTIL_Sparks( pev->oldorigin );

		switch ( RANDOM_LONG(0, 2) )
		{
			case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "materials/spark1.wav", 0.4, ATTN_IDLE); break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "materials/spark2.wav", 0.3, ATTN_IDLE); break;
			case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "materials/spark3.wav", 0.35, ATTN_IDLE); break;
		}
		if(pev->frags > 6.5) pev->impulse = 1;//stop sparking obsolete
	}
	pev->nextthink = gpGlobals->time + pev->frags;
}
