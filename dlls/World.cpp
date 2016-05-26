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
  
+new decals
+weapons and so on to be precached
+loading system for mp3 music, brief system and snow,dust,rain effects


***/

/*

===== world.cpp ========================================================

  precaches and defs for entities and other data that must always be available.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "soundent.h"
#include "client.h"
#include "decals.h"
#include "skill.h"
#include "effects.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"

extern CGraph WorldGraph;
extern CSoundEnt *pSoundEnt;

extern CBaseEntity				*g_pLastSpawn;
DLL_GLOBAL edict_t				*g_pBodyQueueHead;
CGlobalState					gGlobalState;
extern DLL_GLOBAL	int			gDisplayTitle;

extern void W_Precache(void);

//
// This must match the list in util.h
//
DLL_DECALLIST gDecals[] = {
	{ "{shot1",	0 },		// DECAL_GUNSHOT1 
	{ "{shot2",	0 },		// DECAL_GUNSHOT2
	{ "{shot3",0 },			// DECAL_GUNSHOT3
	{ "{shot4",	0 },		// DECAL_GUNSHOT4
	{ "{shot5",	0 },		// DECAL_GUNSHOT5
	{ "{lambda01", 0 },		// DECAL_LAMBDA1
	{ "{lambda02", 0 },		// DECAL_LAMBDA2
	{ "{lambda03", 0 },		// DECAL_LAMBDA3
	{ "{lambda04", 0 },		// DECAL_LAMBDA4
	{ "{lambda05", 0 },		// DECAL_LAMBDA5
	{ "{lambda06", 0 },		// DECAL_LAMBDA6
	{ "{scorch1", 0 },		// DECAL_SCORCH1
	{ "{scorch2", 0 },		// DECAL_SCORCH2
	{ "{crack1", 0 },		// DECAL_crack1
	{ "{crack2", 0 },		// DECAL_crack2
	{ "{blood1", 0 },		// DECAL_BLOOD1
	{ "{blood2", 0 },		// DECAL_BLOOD2
	{ "{blood3", 0 },		// DECAL_BLOOD3
	{ "{blood4", 0 },		// DECAL_BLOOD4
	{ "{blood5", 0 },		// DECAL_BLOOD5
	{ "{blood6", 0 },		// DECAL_BLOOD6
	{ "{yblood1", 0 },		// DECAL_YBLOOD1
	{ "{yblood2", 0 },		// DECAL_YBLOOD2
	{ "{yblood3", 0 },		// DECAL_YBLOOD3
	{ "{yblood4", 0 },		// DECAL_YBLOOD4
	{ "{yblood5", 0 },		// DECAL_YBLOOD5
	{ "{yblood6", 0 },		// DECAL_YBLOOD6
	{ "{knife1", 0 },		// DECAL_KNIFE
	{ "{break1", 0 },		// DECAL_GLASSBREAK1
	{ "{break2", 0 },		// DECAL_GLASSBREAK2
	{ "{break3", 0 },		// DECAL_GLASSBREAK3
	{ "{bigshot1", 0 },		// DECAL_BIGSHOT1
	{ "{bigshot2", 0 },		// DECAL_BIGSHOT2
	{ "{bigshot3", 0 },		// DECAL_BIGSHOT3
	{ "{bigshot4", 0 },		// DECAL_BIGSHOT4
	{ "{bigshot5", 0 },		// DECAL_BIGSHOT5
	{ "{spit1", 0 },		// DECAL_SPIT1
	{ "{spit1", 0 },		// DECAL_SPIT1
	{ "{ofscorch1", 0 },	// DECAL_ST1
	{ "{ofscorch2", 0 },	// DECAL_ST2
	{ "{ofscorch3", 0 },	// DECAL_ST3
	{ "{ofscorch4", 0 },	// DECAL_ST4
	{ "{bproof1", 0 },		// DECAL_BPROOF1
	{ "{gargstomp", 0 },	// DECAL_GARGSTOMP1,	// Gargantua stomp crack
	{ "{smscorch1", 0 },	// DECAL_SMALLSCORCH1,	// Small scorch mark
	{ "{smscorch2", 0 },	// DECAL_SMALLSCORCH2,	// Small scorch mark
	{ "{smscorch3", 0 },	// DECAL_SMALLSCORCH3,	// Small scorch mark

	{ "{shot_pulse1", 0 },	// DECAL_SHOTPULSE	//a pulse decal

	{ "{mommablob", 0 },	// DECAL_MOMMABIRTH		// BM Birth spray
	{ "{mommablob", 0 },	// DECAL_MOMMASPLAT		// BM Mortar spray?? need decal

	{ "{blooddrip1", 0 },		// DECAL_BLOODDRIP1
	{ "{blooddrip2", 0 },		// DECAL_BLOODDRIP2
	{ "{blooddrip3", 0 },		// DECAL_BLOODDRIP3
	{ "{blooddrip4", 0 },		// DECAL_BLOODDRIP4
	{ "{blooddrip5", 0 },		// DECAL_BLOODDRIP5
	{ "{blooddrip6", 0 },		// DECAL_BLOODDRIP6

	{ "{hole_conc_1", 0 },//	DECAL_HOLE_CONC1, 
	{ "{hole_conc_2", 0 },//	DECAL_HOLE_CONC2, 
	{ "{hole_conc_3", 0 },//	DECAL_HOLE_CONC3, 
	
	{ "{hole_metal_1", 0 },//	DECAL_HOLE_METAL1, 
	{ "{hole_metal_2", 0 },//	DECAL_HOLE_METAL2, 
	{ "{hole_metal_3", 0 },//	DECAL_HOLE_METAL3, 

	{ "{hole_wood_1", 0 },//	DECAL_HOLE_WOOD1, 
	{ "{hole_wood_2", 0 },//	DECAL_HOLE_WOOD2, 
	{ "{hole_wood_3", 0 },//	DECAL_HOLE_WOOD3, 
	{ "{tire2", 0 },//	DECAL_TIRE, 
};

/*
==============================================================================

BODY QUE

==============================================================================
*/

#define SF_DECAL_NOTINDEATHMATCH		2048

class CDecal : public CBaseEntity
{
public:
	void	Spawn( void );
	void	KeyValue( KeyValueData *pkvd );
	void	EXPORT StaticDecal( void );
	void	EXPORT TriggerDecal( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( infodecal, CDecal );

// UNDONE:  These won't get sent to joining players in multi-player
void CDecal :: Spawn( void )
{
	if ( pev->skin < 0 || (gpGlobals->deathmatch && FBitSet( pev->spawnflags, SF_DECAL_NOTINDEATHMATCH )) )
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	if ( FStringNull ( pev->targetname ) )
	{
		SetThink( StaticDecal );
		// if there's no targetname, the decal will spray itself on as soon as the world is done spawning.
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		// if there IS a targetname, the decal sprays itself on when it is triggered.
		SetThink ( SUB_DoNothing );
		SetUse(TriggerDecal);
	}
}

void CDecal :: TriggerDecal ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// this is set up as a USE function for infodecals that have targetnames, so that the
	// decal doesn't get applied until it is fired. (usually by a scripted sequence)
	TraceResult trace;
	int			entityIndex;

	UTIL_TraceLine( pev->origin - Vector(5,5,5), pev->origin + Vector(5,5,5),  ignore_monsters, ENT(pev), &trace );

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE( TE_BSPDECAL );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_SHORT( (int)pev->skin );
		entityIndex = (short)ENTINDEX(trace.pHit);
		WRITE_SHORT( entityIndex );
		if ( entityIndex )
			WRITE_SHORT( (int)VARS(trace.pHit)->modelindex );
	MESSAGE_END();

	SetThink( SUB_Remove );
	pev->nextthink = gpGlobals->time + 0.1;
}


void CDecal :: StaticDecal( void )
{
	TraceResult trace;
	int			entityIndex, modelIndex;

	UTIL_TraceLine( pev->origin - Vector(5,5,5), pev->origin + Vector(5,5,5),  ignore_monsters, ENT(pev), &trace );

	entityIndex = (short)ENTINDEX(trace.pHit);
	if ( entityIndex )
		modelIndex = (int)VARS(trace.pHit)->modelindex;
	else
		modelIndex = 0;

	g_engfuncs.pfnStaticDecal( pev->origin, (int)pev->skin, entityIndex, modelIndex );

	SUB_Remove();
}


void CDecal :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "texture"))
	{
		pev->skin = DECAL_INDEX( pkvd->szValue );
		
		// Found
		if ( pev->skin >= 0 )
			return;
		ALERT( at_console, "Can't find decal %s\n", pkvd->szValue );
	}
	else
		CBaseEntity::KeyValue( pkvd );
}


// Body queue class here.... It's really just CBaseEntity
class CCorpse : public CBaseEntity
{
	virtual int ObjectCaps( void ) { return FCAP_DONT_SAVE; }	
};

LINK_ENTITY_TO_CLASS( bodyque, CCorpse );

static void InitBodyQue(void)
{
	string_t	istrClassname = MAKE_STRING("bodyque");

	g_pBodyQueueHead = CREATE_NAMED_ENTITY( istrClassname );
	entvars_t *pev = VARS(g_pBodyQueueHead);
	
	// Reserve 3 more slots for dead bodies
	for ( int i = 0; i < 3; i++ )
	{
		pev->owner = CREATE_NAMED_ENTITY( istrClassname );
		pev = VARS(pev->owner);
	}
	
	pev->owner = g_pBodyQueueHead;
}


//
// make a body que entry for the given ent so the ent can be respawned elsewhere
//
// GLOBALS ASSUMED SET:  g_eoBodyQueueHead
//
void CopyToBodyQue(entvars_t *pev) 
{
	if (pev->effects & EF_NODRAW)
		return;

	entvars_t *pevHead	= VARS(g_pBodyQueueHead);

	pevHead->angles		= pev->angles;
	pevHead->model		= pev->model;
	pevHead->modelindex	= pev->modelindex;
	pevHead->frame		= pev->frame;
	pevHead->colormap	= pev->colormap;
	pevHead->movetype	= MOVETYPE_TOSS;
	pevHead->velocity	= pev->velocity;
	pevHead->flags		= 0;
	pevHead->deadflag	= pev->deadflag;
	pevHead->renderfx	= kRenderFxDeadPlayer;
	pevHead->renderamt	= ENTINDEX( ENT( pev ) );

	pevHead->effects    = pev->effects | EF_NOINTERP;
	//pevHead->goalstarttime = pev->goalstarttime;
	//pevHead->goalframe	= pev->goalframe;
	//pevHead->goalendtime = pev->goalendtime ;
	
	pevHead->sequence = pev->sequence;
	pevHead->animtime = pev->animtime;

	UTIL_SetOrigin(pevHead, pev->origin);
	UTIL_SetSize(pevHead, pev->mins, pev->maxs);
	g_pBodyQueueHead = pevHead->owner;
}


CGlobalState::CGlobalState( void )
{
	Reset();
}

void CGlobalState::Reset( void )
{
	m_pList = NULL; 
	m_listCount = 0;
}

globalentity_t *CGlobalState :: Find( string_t globalname )
{
	if ( !globalname )
		return NULL;

	globalentity_t *pTest;
	const char *pEntityName = STRING(globalname);

	
	pTest = m_pList;
	while ( pTest )
	{
		if ( FStrEq( pEntityName, pTest->name ) )
			break;
	
		pTest = pTest->pNext;
	}

	return pTest;
}


// This is available all the time now on impulse 104, remove later
//#ifdef _DEBUG
void CGlobalState :: DumpGlobals( void )
{
	static char *estates[] = { "Off", "On", "Dead" };
	globalentity_t *pTest;

	ALERT( at_console, "-- Globals --\n" );
	pTest = m_pList;
	while ( pTest )
	{
		ALERT( at_console, "%s: %s (%s)\n", pTest->name, pTest->levelName, estates[pTest->state] );
		pTest = pTest->pNext;
	}
}
//#endif


void CGlobalState :: EntityAdd( string_t globalname, string_t mapName, GLOBALESTATE state )
{
	ASSERT( !Find(globalname) );

	globalentity_t *pNewEntity = (globalentity_t *)calloc( sizeof( globalentity_t ), 1 );
	ASSERT( pNewEntity != NULL );
	pNewEntity->pNext = m_pList;
	m_pList = pNewEntity;
	strcpy( pNewEntity->name, STRING( globalname ) );
	strcpy( pNewEntity->levelName, STRING(mapName) );
	pNewEntity->state = state;
	m_listCount++;
}


void CGlobalState :: EntitySetState( string_t globalname, GLOBALESTATE state )
{
	globalentity_t *pEnt = Find( globalname );

	if ( pEnt )
		pEnt->state = state;
}


const globalentity_t *CGlobalState :: EntityFromTable( string_t globalname )
{
	globalentity_t *pEnt = Find( globalname );

	return pEnt;
}


GLOBALESTATE CGlobalState :: EntityGetState( string_t globalname )
{
	globalentity_t *pEnt = Find( globalname );
	if ( pEnt )
		return pEnt->state;

	return GLOBAL_OFF;
}


// Global Savedata for Delay
TYPEDESCRIPTION	CGlobalState::m_SaveData[] = 
{
	DEFINE_FIELD( CGlobalState, m_listCount, FIELD_INTEGER ),
};

// Global Savedata for Delay
TYPEDESCRIPTION	gGlobalEntitySaveData[] = 
{
	DEFINE_ARRAY( globalentity_t, name, FIELD_CHARACTER, 64 ),
	DEFINE_ARRAY( globalentity_t, levelName, FIELD_CHARACTER, 32 ),
	DEFINE_FIELD( globalentity_t, state, FIELD_INTEGER ),
};


int CGlobalState::Save( CSave &save )
{
	int i;
	globalentity_t *pEntity;

	if ( !save.WriteFields( "GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData) ) )
		return 0;
	
	pEntity = m_pList;
	for ( i = 0; i < m_listCount && pEntity; i++ )
	{
		if ( !save.WriteFields( "GENT", pEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData) ) )
			return 0;

		pEntity = pEntity->pNext;
	}
	
	return 1;
}

int CGlobalState::Restore( CRestore &restore )
{
	int i, listCount;
	globalentity_t tmpEntity;


	ClearStates();
	if ( !restore.ReadFields( "GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData) ) )
		return 0;
	
	listCount = m_listCount;	// Get new list count
	m_listCount = 0;				// Clear loaded data

	for ( i = 0; i < listCount; i++ )
	{
		if ( !restore.ReadFields( "GENT", &tmpEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData) ) )
			return 0;
		EntityAdd( MAKE_STRING(tmpEntity.name), MAKE_STRING(tmpEntity.levelName), tmpEntity.state );
	}
	return 1;
}

void CGlobalState::EntityUpdate( string_t globalname, string_t mapname )
{
	globalentity_t *pEnt = Find( globalname );

	if ( pEnt )
		strcpy( pEnt->levelName, STRING(mapname) );
}


void CGlobalState::ClearStates( void )
{
	globalentity_t *pFree = m_pList;
	while ( pFree )
	{
		globalentity_t *pNext = pFree->pNext;
		free( pFree );
		pFree = pNext;
	}
	Reset();
}


void SaveGlobalState( SAVERESTOREDATA *pSaveData )
{
	CSave saveHelper( pSaveData );
	gGlobalState.Save( saveHelper );
}


void RestoreGlobalState( SAVERESTOREDATA *pSaveData )
{
	CRestore restoreHelper( pSaveData );
	gGlobalState.Restore( restoreHelper );
}


void ResetGlobalState( void )
{
	gGlobalState.ClearStates();
	gInitHUD = TRUE;	// Init the HUD on a new game / load game
}

// moved CWorld class definition to cbase.h
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================

LINK_ENTITY_TO_CLASS( worldspawn, CWorld );

#define SF_WORLD_DARK		0x0001		// Fade from black at startup
#define SF_WORLD_TITLE		0x0002		// Display game title at startup
#define SF_WORLD_FORCETEAM	0x0004		// Force teams

extern DLL_GLOBAL BOOL		g_fGameOver;
float g_flWeaponCheat; 

BOOL g_startSuit; //LRC

//BOOL g_startFlash; //LRC

void CWorld :: Spawn( void )
{
	g_fGameOver = FALSE;
	Precache( );
	g_flWeaponCheat = CVAR_GET_FLOAT( "sv_cheats" );  // Is the impulse 101 command allowed?
}

void CWorld :: Precache( void )
{
	g_pLastSpawn = NULL;
	
#if 1
	CVAR_SET_STRING("sv_gravity", "800"); // 67ft/sec
	CVAR_SET_STRING("sv_stepsize", "18");
#else
	CVAR_SET_STRING("sv_gravity", "384"); // 32ft/sec
	CVAR_SET_STRING("sv_stepsize", "24");
#endif

	CVAR_SET_STRING("room_type", "0");// clear DSP

	// Set up game rules
	if (g_pGameRules)
	{
		delete g_pGameRules;
	}

	g_pGameRules = InstallGameRules( );

	//!!!UNDONE why is there so much Spawn code in the Precache function? I'll just keep it here 

	///!!!LATER - do we want a sound ent in deathmatch? (sjb)
	//pSoundEnt = CBaseEntity::Create( "soundent", g_vecZero, g_vecZero, edict() );
	pSoundEnt = GetClassPtr( ( CSoundEnt *)NULL );
	pSoundEnt->Spawn();

	if ( !pSoundEnt )
	{
		ALERT ( at_console, "**COULD NOT CREATE SOUNDENT**\n" );
	}

	InitBodyQue();
	
// init sentence group playback stuff from sentences.txt.
// ok to call this multiple times, calls after first are ignored.

	SENTENCEG_Init();

// init texture type array from materials.txt

	TEXTURETYPE_Init();


// the area based ambient sounds MUST be the first precache_sounds

// player precaches     
	W_Precache ();									// get weapon precaches

	ClientPrecache();

	PRECACHE_SOUND( "motor_start1.wav" );
	PRECACHE_SOUND( "motor_idle_loop1.wav" );
	PRECACHE_SOUND( "motor_shut_off1.wav" );

// sounds used from C physics code
	PRECACHE_SOUND("common/null.wav");				// clears sound channels

	PRECACHE_SOUND( "items/suitchargeok1.wav" );//!!! temporary sound for respawning weapons.
	PRECACHE_SOUND( "items/gunpickup2.wav" );// player picks up a gun.

	PRECACHE_SOUND( "items/nvg_sw.wav" );// the nvg swhitc

	PRECACHE_SOUND( "common/bodydrop3.wav" );// dead bodies hitting the ground (animation events)
	PRECACHE_SOUND( "common/bodydrop4.wav" );

	PRECACHE_SOUND( "fvox/m_headshot1.wav" ); 
	PRECACHE_SOUND( "fvox/m_headshot2.wav" );// MonsterHeadshots
	PRECACHE_SOUND( "fvox/m_headshot3.wav" );

/*
	PRECACHE_SOUND("weapons/hks1.wav");
	PRECACHE_SOUND("weapons/hks2.wav");
	PRECACHE_SOUND("weapons/hks3.wav");
*/
	PRECACHE_SOUND( "fvox/cheater1.wav" ); 
	PRECACHE_SOUND( "fvox/cheater2.wav" );
	PRECACHE_SOUND( "fvox/cheater3.wav" );	

	PRECACHE_SOUND( "player/damage/suicide.wav" );

	PRECACHE_SOUND( "player/damage/hit_flesh-1.wav" );
	PRECACHE_SOUND( "player/damage/hit_flesh-2.wav" ); // si le pega en la carne
	PRECACHE_SOUND( "player/damage/hit_flesh-3.wav" );

	PRECACHE_SOUND( "fvox/m_hit1.wav" );
	PRECACHE_SOUND( "fvox/m_hit2.wav" ); // hit monster bullets
	PRECACHE_SOUND( "fvox/m_hit3.wav" );

	PRECACHE_MODEL ("sprites/llamas.spr");


//	PRECACHE_MODEL ("sprites/alt_fire.spr");
//	PRECACHE_MODEL ("models/combine_ball.mdl");//weapons

	PRECACHE_MODEL( "sprites/muzz1.spr" );
	PRECACHE_MODEL( "sprites/muzz2.spr" );
	PRECACHE_MODEL( "sprites/muzz3.spr" );

//	PRECACHE_MODEL( "models/muzz1.mdl" );

	//TO DO: using sprites instead of decals (like INVASION)
//	PRECACHE_MODEL ("sprites/bloodspot.spr");
//	PRECACHE_MODEL ("sprites/decal_concrete01.spr");

	PRECACHE_SOUND( "ambience/burning1.wav" );

	//The smoke sprite of guns
	PRECACHE_MODEL( "sprites/gunsmoke.spr" );

	PRECACHE_MODEL( "models/w_weaponbox.mdl" );

//	PRECACHE_MODEL( "models/weapons/bullet.mdl" );//for slow mo purposes
//	PRECACHE_SOUND( "weapons/bullet.wav" );
	
	// for the dropping in single play

	g_Language = (int)CVAR_GET_FLOAT( "sv_language" );
	if ( g_Language == LANGUAGE_GERMAN )
	{
		PRECACHE_MODEL( "models/germangibs.mdl" );
	}
	else
	{
		PRECACHE_MODEL( "models/hgibs.mdl" );
		PRECACHE_MODEL( "models/agibs.mdl" );
	}
	
	PRECACHE_SOUND ("weapons/ric1.wav");
	PRECACHE_SOUND ("weapons/ric2.wav");
	PRECACHE_SOUND ("weapons/ric3.wav");
	PRECACHE_SOUND ("weapons/ric4.wav");
	PRECACHE_SOUND ("weapons/ric5.wav");
	
//
// Setup light animation tables. 'a' is total darkness, 'z' is maxbright.
//

	// 0 normal
	LIGHT_STYLE(0, "m");
	
	// 1 FLICKER (first variety)
	LIGHT_STYLE(1, "mmnmmommommnonmmonqnmmo");
	
	// 2 SLOW STRONG PULSE
	LIGHT_STYLE(2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
	// 3 CANDLE (first variety)
	LIGHT_STYLE(3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
	// 4 FAST STROBE
	LIGHT_STYLE(4, "mamamamamama");
	
	// 5 GENTLE PULSE 1
	LIGHT_STYLE(5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
	// 6 FLICKER (second variety)
	LIGHT_STYLE(6, "nmonqnmomnmomomno");
	
	// 7 CANDLE (second variety)
	LIGHT_STYLE(7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	LIGHT_STYLE(8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	LIGHT_STYLE(9, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	LIGHT_STYLE(10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	LIGHT_STYLE(11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
	
	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	LIGHT_STYLE(12, "mmnnmmnnnmmnn");
	
	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	LIGHT_STYLE(63, "a");

	for ( int i = 0; i < ARRAYSIZE(gDecals); i++ )
		gDecals[i].index = DECAL_INDEX( gDecals[i].name );

// init the WorldGraph.
	WorldGraph.InitGraph();

////////////////////////////////
	int length= 0;

	byte *pMemRainFile;
		
	char szRainFilename1[100]="";

	const char *szRainMapName1 = (STRING(gpGlobals->mapname));
	strcpy(szRainFilename1, "maps/");
//	strcpy(szRainFilename1, "scripts/fx/");
	strcat ( szRainFilename1, szRainMapName1 );
	strcat( szRainFilename1, ".pcs" );

	pMemRainFile = g_engfuncs.pfnLoadFileForMe( szRainFilename1, &length );//chequemos si esta...

	if ( !pMemRainFile )//error!
	{
		ALERT ( at_console, "\n-----------------------------------------------------------\n");
		ALERT ( at_console, "ADVERTENCIA: Imposible encontrar el archivo de Efectos!\n", STRING(gpGlobals->mapname));
		ALERT ( at_console, "Imposible encontrar %s !\n", szRainFilename1);
		ALERT ( at_console, "-----------------------------------------------------------\n");
	}
	else
	{
		ALERT ( at_console, "\n-----------------------------------------------------------\n");
		ALERT ( at_console, "Efectos de %s encontrado con exito!\n", STRING(gpGlobals->mapname));
		ALERT ( at_console, "Iniciando FX...\n");
		ALERT ( at_console, "-----------------------------------------------------------\n");
	}
////////////////////////////////

	//CARGAR ARCHIVO PARA TABLA DE STATUS DE MISION
	ALERT ( at_console, "Buscando Reporte de Mision para mapa %s...\n", STRING(gpGlobals->mapname));

	byte *pMemFile;

	char szFilename1[100]="";
	char szFilename2[100]="";

	const char *szMapName1 = (STRING(gpGlobals->mapname));
//	strcpy(szFilename1, "maps/");
	strcpy(szFilename1, "scripts/brf/");
	strcat ( szFilename1, szMapName1 );
	strcat( szFilename1, ".brf" );//txt?
	strcpy( szFilename2, "mp_am_brief " );
	strcat( szFilename2, szFilename1 );
	strcat( szFilename2, "\n" );

	pMemFile = g_engfuncs.pfnLoadFileForMe( szFilename1, &length );//chequemos si esta...

	if ( !pMemFile )//error!
	{
		ALERT ( at_console, "-----------------------------------------------------------\n");
		ALERT ( at_console, "ADVERTENCIA: Informe de Mision de %s no encontrado!\n", STRING(gpGlobals->mapname));
		ALERT ( at_console, "*Imposible encontrar %s !\n", szFilename1);
		ALERT ( at_console, "-----------------------------------------------------------\n");
	}
	else
	{
		ALERT ( at_console, "\n-----------------------------------------------------------\n");
		ALERT ( at_console, "Informe de Mision de %s encontrado con exito!\n", STRING(gpGlobals->mapname));
		ALERT ( at_console, "-----------------------------------------------------------\n");
	}
	
	SERVER_COMMAND( szFilename2 );

	//CARGAR MUSICA SEGUN NOMBRE DE MAPA
	char szMusicFilename1[100]="";
	char szMusicFilename2[100]="";

	const char *szMusicMapName1 = (STRING(gpGlobals->mapname));
	//esta siguiente linea no es necesaria "sound/fmod/"
	strcpy(szMusicFilename1, "");//No setear directorio! Ej:
	strcat ( szMusicFilename1, szMusicMapName1 );//poniendo nombre de mapa Ej: MYMAP
	strcat( szMusicFilename1, ".mp3" );//agreagando extension Ej: mymap.MP3
	strcpy( szMusicFilename2, "mp_am_mapmusic " );//coloquemos estas rutas en el cvar mp_am_mapmusic
	strcat( szMusicFilename2, szMusicFilename1 );//juntemos todo como "mp_am_mapmusic mymap.MP3"
	strcat( szMusicFilename2, "\n" );//Agreguemos a eso el "\n" para salto de linea Ej: "mp_am_mapmusic mymap.MP3\n"

	if ( CVAR_GET_FLOAT("cl_playmusic") <= 0 )//1
	{
		SERVER_COMMAND( szMusicFilename2 );//Seteamos finalmente el cvar servidor

		ALERT (at_console, "Musica seteada segun mapa como %s \n", (char *)CVAR_GET_STRING( "mp_am_mapmusic" ) );
	
		ALERT ( at_console, "\n-----------------------------------------------------------\n");
		ALERT ( at_console, "Iniciando musica de mapa: %s \n", (char *)CVAR_GET_STRING( "mp_am_mapmusic" ));
		ALERT ( at_console, "-----------------------------------------------------------\n");
	}
	else
	{
		ALERT ( at_console, "\n-----------------------------------------------------------\n");
		ALERT ( at_console, "ADVERTENCIA: El reproductor MP3-OGG-WMA esta activado!\n");
		ALERT ( at_console, "*No es posible reproducir musica segun mapa!\n");
		ALERT ( at_console, "-----------------------------------------------------------\n");
	}
	//CARGAR MUSICA SEGUN NOMBRE DE MAPA
/*	
	//CARGAR MUSICA SEGUN NOMBRE DE MAPA (VERSION 0GG)
	char szMusicOggFilename1[100]="";
	char szMusicOggFilename2[100]="";

	const char *szMusicOggMapName1 = (STRING(gpGlobals->mapname));
	//esta siguiente linea no es necesaria "sound/fmod/"
	strcpy(szMusicOggFilename1, "");//No setear directorio! Ej:
	strcat ( szMusicOggFilename1, szMusicOggMapName1 );//poniendo nombre de mapa Ej: MYMAP
	strcat( szMusicOggFilename1, ".ogg" );//agreagando extension Ej: mymap.OGG
	strcpy( szMusicOggFilename2, "mp_am_mapmusic " );//coloquemos estas rutas en el cvar mp_am_mapmusic
	strcat( szMusicOggFilename2, szMusicOggFilename1 );//juntemos todo como "mp_am_mapmusic mymap.Ogg"
	strcat( szMusicOggFilename2, "\n" );//Agreguemos a eso el "\n" para salto de linea Ej: "mp_am_mapmusic mymap.Ogg\n"
	SERVER_COMMAND( szMusicOggFilename2 );//Seteamos finalmente el cvar servidor

	ALERT (at_console, "Musica seteada segun mapa como %s \n", (char *)CVAR_GET_STRING( "mp_am_mapmusic" ) );
*/
//
//
//	SERVER_COMMAND( "reinit_particles" );

// make sure the .NOD file is newer than the .BSP file.
	if ( !WorldGraph.CheckNODFile ( ( char * )STRING( gpGlobals->mapname ) ) )
	{// NOD file is not present, or is older than the BSP file.
		WorldGraph.AllocNodes ();
	}
	else
	{// Load the node graph for this level
		if ( !WorldGraph.FLoadGraph ( (char *)STRING( gpGlobals->mapname ) ) )
		{// couldn't load, so alloc and prepare to build a graph.
			ALERT ( at_console, "*Error opening .NOD file\n" );
			WorldGraph.AllocNodes ();
		}
		else
		{
			ALERT ( at_console, "\n*Graph Loaded!\n" );
		}
	}

	if ( pev->speed > 0 )
		CVAR_SET_FLOAT( "sv_zmax", pev->speed );
	else
		CVAR_SET_FLOAT( "sv_zmax", 99999 );//4096 por defecto??

	if ( pev->netname )
	{
		ALERT( at_aiconsole, "Chapter title: %s\n", STRING(pev->netname) );
		CBaseEntity *pEntity = CBaseEntity::Create( "env_message", g_vecZero, g_vecZero, NULL );
		if ( pEntity )
		{
			pEntity->SetThink( SUB_CallUseToggle );
			pEntity->pev->message = pev->netname;
			pev->netname = 0;
			pEntity->pev->nextthink = gpGlobals->time + 0.3;
			pEntity->pev->spawnflags = SF_MESSAGE_ONCE;
		}
	}
		
//	ALERT( at_console, "Save Game Title: %s\n", STRING(pev->netname) );

	if ( pev->spawnflags & SF_WORLD_DARK )
		CVAR_SET_FLOAT( "v_dark", 1.0 );
	else
		CVAR_SET_FLOAT( "v_dark", 0.0 );

	if ( pev->spawnflags & SF_WORLD_TITLE )
		gDisplayTitle = TRUE;		// display the game title if this key is set
	else
		gDisplayTitle = FALSE;

	if ( pev->spawnflags & SF_WORLD_FORCETEAM )
	{
		CVAR_SET_FLOAT( "mp_defaultteam", 1 );
	}
	else
	{
		CVAR_SET_FLOAT( "mp_defaultteam", 0 );
	}
}


//
// Just to ignore the "wad" field.
//
void CWorld :: KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "skyname") )
	{
		// Sent over net now.
		CVAR_SET_STRING( "sv_skyname", pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "sounds") )
	{
		gpGlobals->cdAudioTrack = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "WaveHeight") )
	{
		// Sent over net now.
		pev->scale = atof(pkvd->szValue) * (1.0/8.0);
		pkvd->fHandled = TRUE;
		CVAR_SET_FLOAT( "sv_wateramp", pev->scale );
	}
	else if ( FStrEq(pkvd->szKeyName, "MaxRange") )
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "chaptertitle") )
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "startdark") )
	{
		// UNDONE: This is a gross hack!!! The CVAR is NOT sent over the client/sever link
		// but it will work for single player
		int flag = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
		if ( flag )
			pev->spawnflags |= SF_WORLD_DARK;
	}
	else if ( FStrEq(pkvd->szKeyName, "newunit") )
	{
		// Single player only.  Clear save directory if set
		if ( atoi(pkvd->szValue) )
			CVAR_SET_FLOAT( "sv_newunit", 1 );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "gametitle") )
	{
		if ( atoi(pkvd->szValue) )
			pev->spawnflags |= SF_WORLD_TITLE;

		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "mapteams") )
	{
		pev->team = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "defaultteam") )
	{
		if ( atoi(pkvd->szValue) )
		{
			pev->spawnflags |= SF_WORLD_FORCETEAM;
		}
		pkvd->fHandled = TRUE;
	}
	//LRC- let map designers start the player with his suit already on
	else if ( FStrEq(pkvd->szKeyName, "startsuit") )
	{
		g_startSuit = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}/*
	else if ( FStrEq(pkvd->szKeyName, "startflash") )
	{
		g_startFlash = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}*/
	else
		CBaseEntity::KeyValue( pkvd );
}

