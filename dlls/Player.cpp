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

//** This file uses code by Max (prontector@hotmail.de) 
//** YOU SHOULD ASK FOR PERMISSION BEFORE IT!!

***/

/**

  CHANGES ON THIS FILE:
  
+new save/restore values
+init of new messages for client side
+UTIL_EdictScreenFadeBuild (fade screen when your'e hit)
*damage system on TraceAttack
+when you are damage by explosion you can't hear anything for 3/4 seconds
+when you are hit your speed slows down by 3/4 seconds
*punch angle when you're hit
+added new reload animations (don't needed at all)
+insertsound() when you jump (so that the entities can hear you correctly)
+radar
+mp3 sound is played when you spawn (just place a mp3 file as the same name of the map)
+impulse cheats commands for weapons and so on...
+rain message -by G-cont-
+weapon drop system
+NVG
+functions for use cinebars and so on...
*other minor modifications

  Add-ons on Prethink()

  +stealth mode
  +texture detail
  +smoke detect
  +fog fix
  +flashlight improvements
  +hud help system
  +added b_PlayerIsNearOf (bad prefix b_ I know...) -used by terrorists to detect player's position-
  +XP Points
  +slow motion
  +Player Kick
  +semtex zone detect
  +override cheats for noclip and god mode (commented by now)
  +weapon weight system
  +dynamic crosshair
  +v_ weapon movement (SWAT 4 Style)
  +fire on player

***/
/*

===== player.cpp ========================================================

  functions dealing with the player

*/

#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "hltv.h"

#include "func_grass.h"
#include "particle_emitter.h"

// #define DUCKFIX

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL	BOOL	g_fDrawLines;
int gEvilImpulse101;
extern DLL_GLOBAL int		g_iSkillLevel, gDisplayTitle;

extern cvar_t mp_am_weaponweight;
extern cvar_t mp_am_truquitos;
extern cvar_t mp_am_warning_sound;

BOOL gInitHUD = TRUE;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

extern void CopyToBodyQue(entvars_t* pev);
extern void respawn(entvars_t *pev, BOOL fCopyCorpse);
extern Vector VecBModelOrigin(entvars_t *pevBModel );
extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

// the world node graph
extern CGraph	WorldGraph;

#define TRAIN_ACTIVE	0x80 
#define TRAIN_NEW		0xc0
#define TRAIN_OFF		0x00
#define TRAIN_NEUTRAL	0x01
#define TRAIN_SLOW		0x02
#define TRAIN_MEDIUM	0x03
#define TRAIN_FAST		0x04 
#define TRAIN_BACK		0x05

#define	FLASH_DRAIN_TIME	 1.2 //100 units/3 minutes
#define	FLASH_CHARGE_TIME	 0.1 // 100 units/10 seconds  (seconds per unit)

// Global Savedata for player
TYPEDESCRIPTION	CBasePlayer::m_playerSaveData[] = 
{
	DEFINE_FIELD( CBasePlayer, m_flFlashLightTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_iFlashBattery, FIELD_INTEGER ),

	DEFINE_FIELD( CBasePlayer, m_afButtonLast, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_afButtonPressed, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_afButtonReleased, FIELD_INTEGER ),

	DEFINE_ARRAY( CBasePlayer, m_rgItems, FIELD_INTEGER, MAX_ITEMS ),
	DEFINE_FIELD( CBasePlayer, m_afPhysicsFlags, FIELD_INTEGER ),

	DEFINE_FIELD( CBasePlayer, m_flTimeStepSound, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flTimeWeaponIdle, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flSwimTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flDuckTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flWallJumpTime, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_flSuitUpdate, FIELD_TIME ),
	DEFINE_ARRAY( CBasePlayer, m_rgSuitPlayList, FIELD_INTEGER, CSUITPLAYLIST ),
	DEFINE_FIELD( CBasePlayer, m_iSuitPlayNext, FIELD_INTEGER ),
	DEFINE_ARRAY( CBasePlayer, m_rgiSuitNoRepeat, FIELD_INTEGER, CSUITNOREPEAT ),
	DEFINE_ARRAY( CBasePlayer, m_rgflSuitNoRepeatTime, FIELD_TIME, CSUITNOREPEAT ),
	DEFINE_FIELD( CBasePlayer, m_lastDamageAmount, FIELD_INTEGER ),

	DEFINE_ARRAY( CBasePlayer, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES ),
	DEFINE_FIELD( CBasePlayer, m_pActiveItem, FIELD_CLASSPTR ),
	DEFINE_FIELD( CBasePlayer, m_pLastItem, FIELD_CLASSPTR ),
	
	DEFINE_ARRAY( CBasePlayer, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS ),
	DEFINE_FIELD( CBasePlayer, m_idrowndmg, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_idrownrestored, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_tSneaking, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_iTrain, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_bitsHUDDamage, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_flFallVelocity, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, m_iTargetVolume, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iWeaponVolume, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iExtraSoundTypes, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iWeaponFlash, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_fLongJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBasePlayer, m_fInitHUD, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBasePlayer, m_tbdPrev, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_pTank, FIELD_EHANDLE ),
	DEFINE_FIELD( CBasePlayer, m_iHideHUD, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iFOV, FIELD_INTEGER ),

	DEFINE_FIELD( CBasePlayer, m_fCanUseFlashlight, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBasePlayer, bRadarIsOn, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBasePlayer, m_bHEVPlayVoice, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBasePlayer, m_bHEVBroken, FIELD_BOOLEAN ),

	//don't save. recal'ed every frame
//	DEFINE_FIELD( CBasePlayer, m_bActivadorTieneFullTraje, FIELD_BOOLEAN ),
//	DEFINE_FIELD( CBasePlayer, m_bActivadorTieneFullVida, FIELD_BOOLEAN ),

	//G-Cont
	DEFINE_FIELD( CBasePlayer, Rain_dripsPerSecond, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, Rain_windX, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, Rain_windY, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, Rain_randX, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, Rain_randY, FIELD_FLOAT ),

	DEFINE_FIELD( CBasePlayer, Rain_ideal_dripsPerSecond, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, Rain_ideal_windX, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, Rain_ideal_windY, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, Rain_ideal_randX, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, Rain_ideal_randY, FIELD_FLOAT ),

	DEFINE_FIELD( CBasePlayer, Rain_endFade, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, Rain_nextFadeUpdate, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, flCarUseTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, flCarFixUseTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, fl_CarViewOffset, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, iCarFixAngle, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, pCar, FIELD_CLASSPTR ),  //this is the car the player uses
	DEFINE_FIELD( CBasePlayer, saved_flags, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, flCarHornTime, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, flCarDuckTime, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, iSprite, FIELD_INTEGER ),
};	

int gmsgAddShine = 0;   // LRC
int gmsgSetFog = 0; //LRC
int giPrecacheGrunt = 0;
int giPrecacheModelsForDev = 0;//for test
int gmsgShake = 0;
int gmsgFade = 0;
int gmsgSelAmmo = 0;
int gmsgFlashlight = 0;
int gmsgFlashBattery = 0;
int gmsgResetHUD = 0;
int gmsgInitHUD = 0;
int gmsgShowGameTitle = 0;
int gmsgCurWeapon = 0;
int gmsgHealth = 0;
int gmsgDamage = 0;
int gmsgBattery = 0;
int gmsgTrain = 0;
int gmsgLogo = 0;
int gmsgWeaponList = 0;
int gmsgAmmoX = 0;
int gmsgHudText = 0;
int gmsgDeathMsg = 0;
int gmsgScoreInfo = 0;
int gmsgTeamInfo = 0;
int gmsgTeamScore = 0;
int gmsgGameMode = 0;
int gmsgMOTD = 0;
int gmsgServerName = 0;
int gmsgAmmoPickup = 0;
int gmsgWeapPickup = 0;
int gmsgItemPickup = 0;
int gmsgHideWeapon = 0;
int gmsgSetCurWeap = 0;
int gmsgSayText = 0;
int gmsgTextMsg = 0;
int gmsgSetFOV = 0;
int gmsgShowMenu = 0;
int gmsgGeigerRange = 0;
int gmsgTeamNames = 0;
//int gmsgConcussion = 0;

int gmsgStatusText = 0;
int gmsgStatusValue = 0; 

int gmsgConcuss  = 0;
int gmsgPlayMP3 = 0; //Killar

int gmsgMenuOpen = 0;
int gmsgStatusIcon = 0; //LRC
int gmsgClcommand = 0;

// advanced NVG
//int gmsgNVG = 0;
int gmsgNVGActivate = 0;
int gmsgCine = 0;
int gmsgRicardo = 0;
int gmsgHudRadar = 0;

//int gmsgLuzBar = 0;
//int gmsgFlashBar = 0;

int gmsgRainData = 0;
int gmsgSetSky = 0;		//LRC
int	gmsgMoney = 0;
int gmsgAddRope = 0;//ryokeen
int gmsgSetBody = 0;//change body for view weapon model
int gmsgSetSkin = 0;//change skin for view weapon model
int gmsgTbutton = 0;
int gmsgParticles = 0; // particle system
int gmsgGrassParticles = 0; //Grass Entity
// FGW
int gmsgBumpLight = 0;
//int gmsgFmodPlaySound = 0; // fo0z

void LinkUserMessages( void )
{
	// Already taken care of?
	if ( gmsgSelAmmo )
	{
		return;
	}
//	gmsgFmodPlaySound = REG_USER_MSG("FPlaySound", -1);	// fo0z

	// FGW
	gmsgBumpLight = REG_USER_MSG("BumpLight", -1);

	//BP ParticleEmitter
	gmsgParticles = REG_USER_MSG( "Particles", -1 );

	//BP Grass Entity
	gmsgGrassParticles = REG_USER_MSG( "Grass", -1 );

	gmsgSetBody = REG_USER_MSG("SetBody", 1);
	gmsgSetSkin = REG_USER_MSG("SetSkin", 1);

	gmsgAddRope = REG_USER_MSG( "AddRope", -1 );//ryokeen

	gmsgMoney = REG_USER_MSG("Money", -1);

	gmsgSetSky = REG_USER_MSG( "SetSky", 7 );			//LRC //AJH changed size from 7 to 8 to support skybox scale

//	gmsgLuzBar = REG_USER_MSG("LuzBar", 1);
//	gmsgFlashBar = REG_USER_MSG("FlashBar", 2);///??

	gmsgHudRadar = REG_USER_MSG("HudRadar", -1);

	gmsgRicardo = REG_USER_MSG("RicardoBars", 1);//1 byte, si es ON
	gmsgNVGActivate = REG_USER_MSG("NVGActivate", 1);//1 byte, si es ON
	gmsgCine = REG_USER_MSG("CineBars", 1);//1 byte, si es ON

	gmsgAddShine = REG_USER_MSG( "AddShine", -1 );      //LRC
	gmsgSetFog = REG_USER_MSG("SetFog", 9 );			//LRC
	gmsgStatusIcon = REG_USER_MSG( "StatusIcon", -1 );//SP FIX !

	gmsgSelAmmo = REG_USER_MSG("SelAmmo", sizeof(SelAmmo));
	gmsgCurWeapon = REG_USER_MSG("CurWeapon", 3);
	gmsgGeigerRange = REG_USER_MSG("Geiger", 1);
	gmsgFlashlight = REG_USER_MSG("Flashlight", 2);
	gmsgFlashBattery = REG_USER_MSG("FlashBat", 1);
	gmsgHealth = REG_USER_MSG( "Health", 1 );
	gmsgDamage = REG_USER_MSG( "Damage", 12 );
	gmsgBattery = REG_USER_MSG( "Battery", 2);
	gmsgTrain = REG_USER_MSG( "Train", 1);
	gmsgHudText = REG_USER_MSG( "HudText", -1 );
	gmsgSayText = REG_USER_MSG( "SayText", -1 );
	gmsgTextMsg = REG_USER_MSG( "TextMsg", -1 );
	gmsgWeaponList = REG_USER_MSG("WeaponList", -1);
	gmsgResetHUD = REG_USER_MSG("ResetHUD", 1);		// called every respawn
	gmsgInitHUD = REG_USER_MSG("InitHUD", 0 );		// called every time a new player joins the server
	gmsgShowGameTitle = REG_USER_MSG("GameTitle", 1);
	gmsgDeathMsg = REG_USER_MSG( "DeathMsg", -1 );
	gmsgScoreInfo = REG_USER_MSG( "ScoreInfo", 9 );
	gmsgTeamInfo = REG_USER_MSG( "TeamInfo", -1 );  // sets the name of a player's team
	gmsgTeamScore = REG_USER_MSG( "TeamScore", -1 );  // sets the score of a team on the scoreboard
	gmsgGameMode = REG_USER_MSG( "GameMode", 1 );
//	gmsgConcussion = REG_USER_MSG( "Concuss", 1 );//SP show
	gmsgMOTD = REG_USER_MSG( "MOTD", -1 );
	gmsgServerName = REG_USER_MSG( "ServerName", -1 );
	gmsgAmmoPickup = REG_USER_MSG( "AmmoPickup", 2 );
	gmsgWeapPickup = REG_USER_MSG( "WeapPickup", 1 );
	gmsgItemPickup = REG_USER_MSG( "ItemPickup", -1 );
	gmsgHideWeapon = REG_USER_MSG( "HideWeapon", 1 );
	gmsgSetFOV = REG_USER_MSG( "SetFOV", 1 );
//	gmsgShowMenu = REG_USER_MSG( "ShowMenu", -1 );
	gmsgShake = REG_USER_MSG("ScreenShake", sizeof(ScreenShake));
	gmsgFade = REG_USER_MSG("ScreenFade", sizeof(ScreenFade));
	gmsgAmmoX = REG_USER_MSG("AmmoX", 2);
	gmsgTeamNames = REG_USER_MSG( "TeamNames", -1 );

	gmsgStatusText = REG_USER_MSG("StatusText", -1);
	gmsgStatusValue = REG_USER_MSG("StatusValue", 3); 
	gmsgPlayMP3 = REG_USER_MSG("PlayMP3", -1);	//Killar

	gmsgConcuss = REG_USER_MSG("Concuss",1);
	gmsgClcommand		= REG_USER_MSG( "Clcommand", -1);
	gmsgShowMenu		= REG_USER_MSG( "ShowMenu", -1 );
	gmsgRainData = REG_USER_MSG("RainData", 16);
	gmsgTbutton			= REG_USER_MSG( "Tbutton", 2 );
	gmsgLogo = REG_USER_MSG("Logo", 1);
}

LINK_ENTITY_TO_CLASS( player, CBasePlayer );

void CBasePlayer :: Pain( void )
{
	float	flRndSound;//sound randomizer

	flRndSound = RANDOM_FLOAT ( 0 , 1 ); 
	
	if ( flRndSound <= 0.33 )
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain5.wav", 1, ATTN_NORM);
	else if ( flRndSound <= 0.66 )	
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain6.wav", 1, ATTN_NORM);
	else
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain7.wav", 1, ATTN_NORM);
}

/* 
 *
 */
Vector VecVelocityForDamage(float flDamage)
{
	Vector vec(RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));

	if (flDamage > -50)
		vec = vec * 0.7;
	else if (flDamage > -200)
		vec = vec * 2;
	else
		vec = vec * 10;
	
	return vec;
}

#if 0 /*
static void ThrowGib(entvars_t *pev, char *szGibModel, float flDamage)
{
	edict_t *pentNew = CREATE_ENTITY();
	entvars_t *pevNew = VARS(pentNew);

	pevNew->origin = pev->origin;
	SET_MODEL(ENT(pevNew), szGibModel);
	UTIL_SetSize(pevNew, g_vecZero, g_vecZero);

	pevNew->velocity		= VecVelocityForDamage(flDamage);
	pevNew->movetype		= MOVETYPE_BOUNCE;
	pevNew->solid			= SOLID_NOT;
	pevNew->avelocity.x		= RANDOM_FLOAT(0,600);
	pevNew->avelocity.y		= RANDOM_FLOAT(0,600);
	pevNew->avelocity.z		= RANDOM_FLOAT(0,600);
	CHANGE_METHOD(ENT(pevNew), em_think, SUB_Remove);
	pevNew->ltime		= gpGlobals->time;
	pevNew->nextthink	= gpGlobals->time + RANDOM_FLOAT(10,20);
	pevNew->frame		= 0;
	pevNew->flags		= 0;
}
	
	
static void ThrowHead(entvars_t *pev, char *szGibModel, floatflDamage)
{
	SET_MODEL(ENT(pev), szGibModel);
	pev->frame			= 0;
	pev->nextthink		= -1;
	pev->movetype		= MOVETYPE_BOUNCE;
	pev->takedamage		= DAMAGE_NO;
	pev->solid			= SOLID_NOT;
	pev->view_ofs		= Vector(0,0,8);
	UTIL_SetSize(pev, Vector(-16,-16,0), Vector(16,16,56));
	pev->velocity		= VecVelocityForDamage(flDamage);
	pev->avelocity		= RANDOM_FLOAT(-1,1) * Vector(0,600,0);
	pev->origin.z -= 24;
	ClearBits(pev->flags, FL_ONGROUND);
}


*/ 
#endif

int TrainSpeed(int iSpeed, int iMax)
{
	float fSpeed, fMax;
	int iRet = 0;

	fMax = (float)iMax;
	fSpeed = iSpeed;

	fSpeed = fSpeed/fMax;

	if (iSpeed < 0)
		iRet = TRAIN_BACK;
	else if (iSpeed == 0)
		iRet = TRAIN_NEUTRAL;
	else if (fSpeed < 0.33)
		iRet = TRAIN_SLOW;
	else if (fSpeed < 0.66)
		iRet = TRAIN_MEDIUM;
	else
		iRet = TRAIN_FAST;

	return iRet;
}

void CBasePlayer :: DeathSound( void )
{
	// water death sounds
	
	if (pev->waterlevel == 3)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/die_water.wav", 1, ATTN_NORM);
		return;
	}
	

	// temporarily using pain sounds for death sounds
	switch (RANDOM_LONG(1,5)) 
	{
	case 1: 
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain5.wav", 1, ATTN_NORM);
		break;
	case 2: 
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain6.wav", 1, ATTN_NORM);
		break;
	case 3: 
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain7.wav", 1, ATTN_NORM);
		break;
	}

	// play one of the suit death alarms
	EMIT_GROUPNAME_SUIT(ENT(pev), "HEV_DEAD");
}

// override takehealth
// bitsDamageType indicates type of damage healed. 

int CBasePlayer :: TakeHealth( float flHealth, int bitsDamageType )
{
	return CBaseMonster :: TakeHealth (flHealth, bitsDamageType);

}

Vector CBasePlayer :: GetGunPosition( )
{
//	UTIL_MakeVectors(pev->v_angle);
//	m_HackedGunPos = pev->view_ofs;
	Vector origin;
	
	origin = pev->origin + pev->view_ofs;

	return origin;
}

static unsigned short FixedUnsigned16( float value, float scale ) //FX

{

int output;

output = value * scale;

if ( output < 0 )

output = 0;

if ( output > 0xFFFF )

output = 0xFFFF;

return (unsigned short)output;

}

void UTIL_EdictScreenFadeBuild( ScreenFade &fade, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags )

{

fade.duration = FixedUnsigned16( fadeTime, 1<<12 ); // 4.12 fixed

fade.holdTime = FixedUnsigned16( fadeHold, 1<<12 ); // 4.12 fixed

fade.r = (int)color.x;

fade.g = (int)color.y;

fade.b = (int)color.z;

fade.a = alpha;

fade.fadeFlags = flags;

}

void UTIL_EdictScreenFadeWrite( const ScreenFade &fade, edict_s *edict )

{

MESSAGE_BEGIN( MSG_ONE, gmsgFade, NULL, edict ); // use the magic #1 for "one client"


WRITE_SHORT( fade.duration ); // fade lasts this long

WRITE_SHORT( fade.holdTime ); // fade lasts this long

WRITE_SHORT( fade.fadeFlags ); // fade type (in / out)

WRITE_BYTE( fade.r ); // fade red

WRITE_BYTE( fade.g ); // fade green

WRITE_BYTE( fade.b ); // fade blue

WRITE_BYTE( fade.a ); // fade blue

MESSAGE_END();

}


void UTIL_EdictScreenFade( edict_s *edict, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags ) //FX

{

ScreenFade fade;

UTIL_EdictScreenFadeBuild( fade, color, fadeTime, fadeHold, alpha, flags );

UTIL_EdictScreenFadeWrite( fade, edict );

}


//=========================================================
// TraceAttack
//=========================================================
void CBasePlayer :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if ( pev->takedamage )
	{
		m_LastHitGroup = ptr->iHitgroup;

		switch ( ptr->iHitgroup )
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:
			flDamage *= gSkillData.plrHead;

		if (pev->armorvalue <= 0)
		{						  
			switch (RANDOM_LONG(0,2))
			{
				case 0:	pev->punchangle.x = RANDOM_LONG(0,20) - 3; pev->punchangle.z = RANDOM_LONG(0,10) - 20; pev->punchangle.y = RANDOM_LONG(0,19) - 15;
						UTIL_BloodStream( ptr->vecEndPos, gpGlobals->v_forward * -5 + gpGlobals->v_up * 2, (unsigned short)73, 100 );
					break;
				case 1:	pev->punchangle.x = RANDOM_LONG(0,20) - 15; pev->punchangle.z = RANDOM_LONG(0,29) - 15; pev->punchangle.y = RANDOM_LONG(0,49) - 25;
						UTIL_BloodStream( ptr->vecEndPos, gpGlobals->v_forward * -5 + gpGlobals->v_up * 2, (unsigned short)73, 100 );
					break;
				case 2:	pev->punchangle.x = RANDOM_LONG(0,7) - 5; pev->punchangle.z = RANDOM_LONG(0,7) - 5; pev->punchangle.y = RANDOM_LONG(0,7) - 5;
						UTIL_BloodStream( ptr->vecEndPos, gpGlobals->v_forward * -5 + gpGlobals->v_up * 2, (unsigned short)73, 50 );
					break;
			}
			
/*			switch (RANDOM_LONG(0,2))
			{
				case 0:EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/headshot1.wav", 0.9, ATTN_NORM); break;
				case 1:EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/headshot2.wav", 0.9, ATTN_NORM); break;
				case 2:EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/headshot3.wav", 0.9, ATTN_NORM); break;
			}*/
		}
		else
		{
			UTIL_WhiteSparks( ptr->vecEndPos, ptr->vecPlaneNormal, 0, 10, 200, 10 );

//			if((!m_fNvgOn) && (!m_InDREAM))//hold, fade
//			UTIL_EdictScreenFade( edict(), Vector(255,255,255), 0.1, 0.4, 222, FFADE_IN ); //FX

			switch (RANDOM_LONG(0,2))
			{
				case 0:	pev->punchangle.x = RANDOM_LONG(0,20) - 3; pev->punchangle.z = RANDOM_LONG(0,10) - 20; pev->punchangle.y = RANDOM_LONG(0,19) - 15;break;
				case 1:	pev->punchangle.x = RANDOM_LONG(0,20) - 15; pev->punchangle.z = RANDOM_LONG(0,29) - 15; pev->punchangle.y = RANDOM_LONG(0,49) - 25;break;
				case 2:	pev->punchangle.x = RANDOM_LONG(0,7) - 5; pev->punchangle.z = RANDOM_LONG(0,7) - 5; pev->punchangle.y = RANDOM_LONG(0,7) - 5;break;
			}

/*			switch (RANDOM_LONG(0,2)) 
			{
				case 0:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet1.wav", 0.9, ATTN_NORM); break;
				case 1:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet2.wav", 0.9, ATTN_NORM); break;
				case 2:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet3.wav", 0.9, ATTN_NORM); break;
			}*/
		}
			break;
// END HEADSHOT
		case HITGROUP_CHEST:
			flDamage *= gSkillData.plrChest;
		break;

		case HITGROUP_STOMACH:
		flDamage *= gSkillData.plrStomach;

		if (pev->armorvalue <= 0) // Solo si no tienes chaleco...
		{
/*			switch (RANDOM_LONG(0,1)) 
			{
				case 0:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/stomach1.wav", 1.0, ATTN_NORM);	break;
				case 1:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/stomach2.wav", 1.0, ATTN_NORM);	break;
			}*/
		}
		break;

		case HITGROUP_LEFTARM:
			flDamage *= gSkillData.plrArm;
		break;

		case HITGROUP_RIGHTARM:
			//jejeje pretty nasty :)
			flDamage *= gSkillData.plrArm;

//			if (RANDOM_LONG(0,1))
//			SERVER_COMMAND("drop\n");
		break;

		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
		flDamage *= gSkillData.plrLeg;
		break;
		
		default:
			break;
		}

		if ( pev->flags & FL_GODMODE )
			flDamage = 0;

//		if ( g_bGodMode )
//			flDamage = 0;

		if (pev->armorvalue <= 0)//no chaleco
		{
/*			switch (RANDOM_LONG(0,2)) 
			{
				case 0:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/hit_flesh-1.wav", 0.9, ATTN_NORM); break;
				case 1:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/hit_flesh-2.wav", 0.9, ATTN_NORM); break;
				case 2:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/hit_flesh-3.wav", 0.9, ATTN_NORM); break;
			}*/
				
			UTIL_EdictScreenFade( edict(), Vector(222,0,0), 0.5, 0.5, 100, FFADE_IN ); //FX

			SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
			TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
		}
		else
		{
/*			switch (RANDOM_LONG(0,2)) 
			{//chanel voice?
				case 0:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/hit_kevlar-1.wav", 0.9, ATTN_NORM); break;
				case 1:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/hit_kevlar-2.wav", 0.9, ATTN_NORM); break;
				case 2:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/hit_kevlar-3.wav", 0.9, ATTN_NORM); break;
			}*/
		}
				
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
	}// cierre del if
} // ciere del void


/*
	Take some damage.  
	NOTE: each call to TakeDamage with bitsDamageType set to a time-based damage
	type will cause the damage time countdown to be reset.  Thus the ongoing effects of poison, radiation
	etc are implemented with subsequent calls to TakeDamage using DMG_GENERIC.
*/

#define ARMOR_RATIO	 0.2	// Armor Takes 80% of the damage
#define ARMOR_BONUS  0.5	// Each Point of Armor is work 1/x points of health



int CBasePlayer :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// have suit diagnose the problem - ie: report damage type
	int bitsDamage = bitsDamageType;
	int ffound = TRUE;
	int fmajor;
	int fcritical;
	int fTookDamage;
	int ftrivial;
	float flRatio;
	float flBonus;
	float flHealthPrev = pev->health;

	flBonus = ARMOR_BONUS;
	flRatio = ARMOR_RATIO;

	if ( ( bitsDamageType & DMG_BLAST ) && g_pGameRules->IsMultiplayer() )
	{
		// blasts damage armor more.
		flBonus *= 2;
	}

	if (!m_fHurted)
	{
		if (!m_fSlowMotionOn)
		{
			if ( ( bitsDamageType & DMG_BLAST ) )
			{
				if (RANDOM_LONG( 0, 99 ) < 80)
				{
					CBaseEntity *pEntidadS = NULL;
					Vector VecSrc;
					VecSrc = pev->origin;

					//lets change the sound, ok?
					bChangeSound = TRUE;

					while ((pEntidadS = UTIL_FindEntityInSphere( pEntidadS, VecSrc, 9999 )) != NULL)
					{
						if ( FClassnameIs( pEntidadS->pev, "env_sound" ) )
						{
							//oh! shit! A env_sound is present on map... so, don't change...
							bChangeSound = FALSE;
						}
					}

					//if we can change sound, ok, go ahead
					if(bChangeSound)
					{	
						SERVER_COMMAND ("room_type 14");
					}

					ClientPrint(pev, HUD_PRINTCENTER, "#EarsDamaged");

					if ( pev->flags & FL_GODMODE )
					{
					}
					else
					{
						UTIL_EdictScreenFade( edict(), Vector(222,0,0), 1.5, 1.5, 200, FFADE_IN ); //FX
					}

					//esta herido tambien
					m_fHurted	= TRUE;
					
					SetThink( RestoreSpeedVol );
					
					if (RANDOM_LONG( 0, 99 ) < 77)
					{
						SetSuitUpdate("!HEV_MOVEOVER", FALSE, SUIT_NEXT_IN_5SEC);//se hizo mierda el movimeitn
						pev->nextthink = gpGlobals->time + 15;
					}
					else
					{
						pev->nextthink = gpGlobals->time + 4;
					}
				}
				else
				{
					ClientPrint(pev, HUD_PRINTCENTER, "#NoEarsDamaged");
				}
			}

			if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB /*| DMG_BLAST*/)) //already tested
			{
				m_fHurted	= TRUE;
				SetThink( RestoreSpeedVol );
							
				if (RANDOM_LONG( 0, 99 ) < 5)
				{
					if (pev->armorvalue <= 0)
					{
						EMIT_SOUND_SUIT(edict(), "!HEV_SHUTDOWN");
						m_bHEVBroken = TRUE;

						pev->nextthink = gpGlobals->time + 8;
					}
					else
					{
						pev->nextthink = gpGlobals->time + 1;
					}
				}
				else
				{
					pev->nextthink = gpGlobals->time + 0.1;//restore original speed now!
				}

				if	(RANDOM_LONG( 0, 99 ) < 10)
				{
					SetSuitUpdate("!HEV_MOVEOVER", FALSE, SUIT_NEXT_IN_5SEC);//se hizo mierda el movimeitn
					pev->nextthink = gpGlobals->time + 8;
				}
			}
		}
		else
		{

		}
	}
	else
	{
//		ALERT( at_console, "INFO: m_fHurted is TRUE\n");
	}

	// Already dead
	if ( !IsAlive() )
		return 0;
	// go take the damage first

	
	CBaseEntity *pAttacker = CBaseEntity::Instance(pevAttacker);

	if ( !g_pGameRules->FPlayerCanTakeDamage( this, pAttacker ) )
	{
		// Refuse the damage
		return 0;
	}

	// keep track of amount of damage last sustained
	m_lastDamageAmount = flDamage;

	// Armor. 
	if (pev->armorvalue && !(bitsDamageType & (DMG_FALL | DMG_DROWN)) )// armor doesn't protect against fall or drown damage!
	{
		float flNew = flDamage * flRatio;

		float flArmor;

		flArmor = (flDamage - flNew) * flBonus;

		// Does this use more armor than we have?
		if (flArmor > pev->armorvalue)
		{
			flArmor = pev->armorvalue;
			flArmor *= (1/flBonus);
			flNew = flDamage - flArmor;
			pev->armorvalue = 0;
		}
		else
			pev->armorvalue -= flArmor;
		
		flDamage = flNew;
	}

	// this cast to INT is critical!!! If a player ends up with 0.5 health, the engine will get that
	// as an int (zero) and think the player is dead! (this will incite a clientside screentilt, etc)
	fTookDamage = CBaseMonster::TakeDamage(pevInflictor, pevAttacker, (int)flDamage, bitsDamageType);

	// reset damage time countdown for each type of time based damage player just sustained

	{
		for (int i = 0; i < CDMG_TIMEBASED; i++)
			if (bitsDamageType & (DMG_PARALYZE << i))
				m_rgbTimeBasedDamage[i] = 0;
	}

	// tell director about it
	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// take damage event
		WRITE_SHORT( ENTINDEX(this->edict()) );	// index number of primary entity
		WRITE_SHORT( ENTINDEX(ENT(pevInflictor)) );	// index number of secondary entity
		WRITE_LONG( 5 );   // eventflags (priority and flags)
	MESSAGE_END();


	// how bad is it, doc?

	ftrivial = (pev->health > 75 || m_lastDamageAmount < 5);
	fmajor = (m_lastDamageAmount > 25);
	fcritical = (pev->health < 30);

	// handle all bits set in this damage message,
	// let the suit give player the diagnosis

	// UNDONE: add sounds for types of damage sustained (ie: burn, shock, slash )

	// UNDONE: still need to record damage and heal messages for the following types

		// DMG_BURN	
		// DMG_FREEZE
		// DMG_BLAST
		// DMG_SHOCK

	m_bitsDamageType |= bitsDamage; // Save this so we can report it to the client
	m_bitsHUDDamage = -1;  // make sure the damage bits get resent

	while (fTookDamage && (!ftrivial || (bitsDamage & DMG_TIMEBASED)) && ffound && bitsDamage)
	{
		ffound = FALSE;

		if (bitsDamage & DMG_CLUB)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);	// minor fracture
			bitsDamage &= ~DMG_CLUB;
			ffound = TRUE;
		}
		if (bitsDamage & (DMG_FALL | DMG_CRUSH))
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG5", FALSE, SUIT_NEXT_IN_30SEC);	// major fracture
			else
				SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);	// minor fracture
	
			bitsDamage &= ~(DMG_FALL | DMG_CRUSH);
			ffound = TRUE;
		}
		
		if (bitsDamage & DMG_BULLET)
		{
			if (m_lastDamageAmount > 5)
				SetSuitUpdate("!HEV_DMG6", FALSE, SUIT_NEXT_IN_30SEC);	// blood loss detected
			else
				SetSuitUpdate("!HEV_DMG0", FALSE, SUIT_NEXT_IN_30SEC);	// minor laceration
			
			bitsDamage &= ~DMG_BULLET;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_SLASH)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG1", FALSE, SUIT_NEXT_IN_30SEC);	// major laceration
			else
				SetSuitUpdate("!HEV_DMG0", FALSE, SUIT_NEXT_IN_30SEC);	// minor laceration

			bitsDamage &= ~DMG_SLASH;
			ffound = TRUE;
		}
		
		if (bitsDamage & DMG_SONIC)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG2", FALSE, SUIT_NEXT_IN_1MIN);	// internal bleeding
			bitsDamage &= ~DMG_SONIC;
			ffound = TRUE;
		}

		if (bitsDamage & (DMG_POISON | DMG_PARALYZE))
		{
			SetSuitUpdate("!HEV_DMG3", FALSE, SUIT_NEXT_IN_1MIN);	// blood toxins detected
			bitsDamage &= ~(DMG_POISON | DMG_PARALYZE);
			ffound = TRUE;
		}

		if (bitsDamage & DMG_ACID)
		{
			SetSuitUpdate("!HEV_DET1", FALSE, SUIT_NEXT_IN_1MIN);	// hazardous chemicals detected
			bitsDamage &= ~DMG_ACID;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_NERVEGAS)
		{
			SetSuitUpdate("!HEV_DET0", FALSE, SUIT_NEXT_IN_1MIN);	// biohazard detected
			bitsDamage &= ~DMG_NERVEGAS;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_RADIATION)
		{
			SetSuitUpdate("!HEV_DET2", FALSE, SUIT_NEXT_IN_1MIN);	// radiation detected
			bitsDamage &= ~DMG_RADIATION;
			ffound = TRUE;
		}
		if (bitsDamage & DMG_SHOCK)
		{
			SetSuitUpdate("!HEV_SHOCK", FALSE, SUIT_NEXT_IN_1MIN);//new
			bitsDamage &= ~DMG_SHOCK;
			ffound = TRUE;
		}

		//new
		if (bitsDamage & DMG_BURN)
		{
			SetSuitUpdate("!HEV_FIRE", FALSE, SUIT_NEXT_IN_1MIN);//new
			bitsDamage &= ~DMG_BURN;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_SLOWBURN)
		{
			SetSuitUpdate("!HEV_FIRE", FALSE, SUIT_NEXT_IN_1MIN);//new
			bitsDamage &= ~DMG_SLOWBURN;
			ffound = TRUE;
		}

	}
		
//	pev->punchangle.x = -2;
/*
	pev->punchangle.x = -flDamage /2;
	pev->punchangle.y = -flDamage /2;
	pev->punchangle.z = -flDamage /2;*/

	pev->punchangle.x = -flDamage /2.5;
	pev->punchangle.y = -flDamage /2.5;
	pev->punchangle.z = -flDamage /2.5;

	float alphachange;
	alphachange = flDamage * 3;
	if (alphachange > 255)
	alphachange = 255;

	if (fTookDamage && !ftrivial && fmajor && flHealthPrev >= 75) 
	{
		// first time we take major damage...
		// turn automedic on if not on
		SetSuitUpdate("!HEV_MED1", FALSE, SUIT_NEXT_IN_30MIN);	// automedic on

		// give morphine shot if not given recently
		SetSuitUpdate("!HEV_HEAL7", FALSE, SUIT_NEXT_IN_30MIN);	// morphine shot
	}
	
	if (fTookDamage && !ftrivial && fcritical && flHealthPrev < 75)
	{

		// already took major damage, now it's critical...
		if (pev->health < 6)
			SetSuitUpdate("!HEV_HLTH3", FALSE, SUIT_NEXT_IN_10MIN);	// near death
		else if (pev->health < 20)
			SetSuitUpdate("!HEV_HLTH2", FALSE, SUIT_NEXT_IN_10MIN);	// health critical
	
		// give critical health warnings
		if (!RANDOM_LONG(0,3) && flHealthPrev < 50)
			SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN); //seek medical attention
	}

	// if we're taking time based damage, warn about its continuing effects
	if (fTookDamage && (bitsDamageType & DMG_TIMEBASED) && flHealthPrev < 75)
		{
			if (flHealthPrev < 50)
			{
				if (!RANDOM_LONG(0,3))
					SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN); //seek medical attention
			}
			else
				SetSuitUpdate("!HEV_HLTH1", FALSE, SUIT_NEXT_IN_10MIN);	// health dropping
		}

	return fTookDamage;
}

//=========================================================
// PackDeadPlayerItems - call this when a player dies to
// pack up the appropriate weapons and ammo items, and to
// destroy anything that shouldn't be packed.
//
// This is pretty brute force :(
//=========================================================
void CBasePlayer::PackDeadPlayerItems( void )
{
	int iWeaponRules;
	int iAmmoRules;
	int i;
	CBasePlayerWeapon *rgpPackWeapons[ 20 ];// 20 hardcoded for now. How to determine exactly how many weapons we have?
	int iPackAmmo[ MAX_AMMO_SLOTS + 1];
	int iPW = 0;// index into packweapons array
	int iPA = 0;// index into packammo array

	memset(rgpPackWeapons, NULL, sizeof(rgpPackWeapons) );
	memset(iPackAmmo, -1, sizeof(iPackAmmo) );

	// get the game rules 
	iWeaponRules = g_pGameRules->DeadPlayerWeapons( this );
 	iAmmoRules = g_pGameRules->DeadPlayerAmmo( this );

	if ( iWeaponRules == GR_PLR_DROP_GUN_NO && iAmmoRules == GR_PLR_DROP_AMMO_NO )
	{
		// nothing to pack. Remove the weapons and return. Don't call create on the box!
		RemoveAllItems( TRUE );
		return;
	}

// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem *pPlayerItem = m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				switch( iWeaponRules )
				{
				case GR_PLR_DROP_GUN_ACTIVE:
					if ( m_pActiveItem && pPlayerItem == m_pActiveItem )
					{
						// this is the active item. Pack it.
						
						//Counter strike style? Fuck off! Am is not MP!
						//sys:tenemos el arma activa que debe ser arrojada
						//antes de esto elijamos la mejor arma y tirarla									
						g_pGameRules->GetNextBestWeapon( this, m_pActiveItem );

						rgpPackWeapons[ iPW++ ] = (CBasePlayerWeapon *)pPlayerItem;
					}
					break;

				case GR_PLR_DROP_GUN_ALL:
					rgpPackWeapons[ iPW++ ] = (CBasePlayerWeapon *)pPlayerItem;
					break;

				default:
					break;
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}

// now go through ammo and make a list of which types to pack.
	if ( iAmmoRules != GR_PLR_DROP_AMMO_NO )
	{
		for ( i = 0 ; i < MAX_AMMO_SLOTS ; i++ )
		{
			if ( m_rgAmmo[ i ] > 0 )
			{
				// player has some ammo of this type.
				switch ( iAmmoRules )
				{
				case GR_PLR_DROP_AMMO_ALL:
					iPackAmmo[ iPA++ ] = i;
					break;

				case GR_PLR_DROP_AMMO_ACTIVE:
					if ( m_pActiveItem && i == m_pActiveItem->PrimaryAmmoIndex() ) 
					{
						// this is the primary ammo type for the active weapon
						iPackAmmo[ iPA++ ] = i;
					}
					else if ( m_pActiveItem && i == m_pActiveItem->SecondaryAmmoIndex() ) 
					{
						// this is the secondary ammo type for the active weapon
						iPackAmmo[ iPA++ ] = i;
					}
					break;

				default:
					break;
				}
			}
		}
	}

// create a box to pack the stuff into.
	CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin, pev->angles, edict() );

	pWeaponBox->pev->angles.x = 0;// don't let weaponbox tilt.
	pWeaponBox->pev->angles.z = 0;

//	pWeaponBox->SetThink( CWeaponBox::Kill );
//	pWeaponBox->pev->nextthink = gpGlobals->time + 120;
	//dont kill?

// back these two lists up to their first elements
	iPA = 0;
	iPW = 0;

// pack the ammo
	while ( iPackAmmo[ iPA ] != -1 )
	{
		pWeaponBox->PackAmmo( MAKE_STRING( CBasePlayerItem::AmmoInfoArray[ iPackAmmo[ iPA ] ].pszName ), m_rgAmmo[ iPackAmmo[ iPA ] ] );
		iPA++;
	}

// now pack all of the items in the lists
	while ( rgpPackWeapons[ iPW ] )
	{
		// weapon unhooked from the player. Pack it into der box.
		pWeaponBox->PackWeapon( rgpPackWeapons[ iPW ] );

		iPW++;
	}
//1.2
	pWeaponBox->pev->velocity = pev->velocity * 0.8;// weaponbox has player's velocity, then some.

	RemoveAllItems( TRUE );// now strip off everything that wasn't handled by the code above.
}

void CBasePlayer::RemoveAllItems( BOOL removeSuit )
{
	if (m_pActiveItem)
	{
		ResetAutoaim( );
		m_pActiveItem->Holster( );
		m_pActiveItem = NULL;
	}

	m_pLastItem = NULL;

	int i;
	CBasePlayerItem *pPendingItem;
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		m_pActiveItem = m_rgpPlayerItems[i];
		while (m_pActiveItem)
		{
			pPendingItem = m_pActiveItem->m_pNext; 
			m_pActiveItem->Drop( );
			m_pActiveItem = pPendingItem;
		}
		m_rgpPlayerItems[i] = NULL;
	}
	m_pActiveItem = NULL;

	pev->viewmodel		= 0;
	pev->weaponmodel	= 0;
	
	if ( removeSuit )
		pev->weapons = 0;
	else
		pev->weapons &= ~WEAPON_ALLWEAPONS;

	for ( i = 0; i < MAX_AMMO_SLOTS;i++)
		m_rgAmmo[i] = 0;

	UpdateClientData();
	// send Selected Weapon Message to our client
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
	MESSAGE_END();
}

/*
 * GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
 *
 * ENTITY_METHOD(PlayerDie)
 */
entvars_t *g_pevLastInflictor;  // Set in combat.cpp.  Used to pass the damage inflictor for death messages.
								// Better solution:  Add as parameter to all Killed() functions.

void CBasePlayer::Killed( entvars_t *pevAttacker, int iGib )
{
	CSound *pSound;

	// Holster weapon immediately, to allow it to cleanup
	if ( m_pActiveItem )
		m_pActiveItem->Holster( );

//	m_flReleaseThrow = 1;//test

	if ( m_flStartThrow )
	{
		CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
	}

	g_pGameRules->PlayerKilled( this, pevAttacker, g_pevLastInflictor );

	if ( m_pTank != NULL )
	{
		m_pTank->Use( this, this, USE_OFF, 0 );
		m_pTank = NULL;
	}

	// this client isn't going to be thinking for a while, so reset the sound until they respawn
	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( edict() ) );
	{
		if ( pSound )
		{
			pSound->Reset();
		}
	}
	
	//dead note
	if ( !g_pGameRules->IsMultiplayer() )//this cause laggin in mp EDIT: Am it's not MP anymore, so don't take care
	UTIL_ShowMessageAll( STRING(ALLOC_STRING("GAMEOVER_CODE")));
	
	MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);
		WRITE_STRING("drop");
		WRITE_BYTE(1);
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_ONE, gmsgTbutton, NULL, pev );
		WRITE_SHORT( 0 );
	MESSAGE_END();

	// inform the client about the change
		MESSAGE_BEGIN(MSG_ONE, gmsgNVGActivate, NULL, pev);
			WRITE_BYTE( 0 );
		MESSAGE_END( );

	//	ClientPrint(pev, HUD_PRINTCENTER, "#NvgOff");
	//	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/nvg_off.wav", 0.9, ATTN_NORM); 

// advanced NVG
	
	SetAnimation( PLAYER_DIE );
	
	m_iRespawnFrames = 0;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes

	pev->deadflag		= DEAD_DYING;
	pev->movetype		= MOVETYPE_TOSS;
	ClearBits( pev->flags, FL_ONGROUND );
	if (pev->velocity.z < 10)
		pev->velocity.z += RANDOM_FLOAT(0,300);

	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(NULL, FALSE, 0);

	// send "health" update message to zero
	m_iClientHealth = 0;
	MESSAGE_BEGIN( MSG_ONE, gmsgHealth, NULL, pev );
		WRITE_BYTE( m_iClientHealth );
	MESSAGE_END();

	// Tell Ammo Hud that the player is dead
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_BYTE(0XFF);
		WRITE_BYTE(0xFF);
	MESSAGE_END();

	// reset FOV
	pev->fov = m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
		WRITE_BYTE(0);
	MESSAGE_END();

	UTIL_EdictScreenFade( edict(), Vector(0,0,0), 4, 10, 255, FFADE_OUT | FFADE_STAYOUT );

	if ( ( pev->health < -40 && iGib != GIB_NEVER ) || iGib == GIB_ALWAYS )
	{
		pev->solid			= SOLID_NOT;
		GibMonster();	// This clears pev->model
		pev->effects |= EF_NODRAW;
		return;
	}

	DeathSound();

//	if ( !g_pGameRules->IsMultiplayer() )
//	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/fredisdead.wav", 0.9, ATTN_NORM);

	pev->angles.x = 0;
	pev->angles.z = 0;

//	if (FrascosAdrenalina <= 1)
//	{
//		ClientPrint(pev, HUD_PRINTCENTER, "#YouCanRevive");
//	}
//	else
//	{
		SetThink(PlayerDeathThink);
		pev->nextthink = gpGlobals->time + 0.1;
//	}
}


// Set the activity based on an event or current state
void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
    int animDesired;
    float speed;
    char szAnim[64];

    speed = pev->velocity.Length2D();

	if (pCar)
		return;

    if (pev->flags & FL_FROZEN)
    {
        speed = 0;
        playerAnim = PLAYER_IDLE;
    }

    switch (playerAnim)
    {
    case PLAYER_JUMP:
        m_IdealActivity = ACT_HOP;
        break;
//*-
    case PLAYER_RELOAD://DA - new anims
        m_IdealActivity = ACT_RELOAD;
    break;

    case PLAYER_SUPERJUMP:
        m_IdealActivity = ACT_LEAP;
        break;
   
    case PLAYER_DIE:
        m_IdealActivity = ACT_DIESIMPLE;
        m_IdealActivity = GetDeathActivity( );
        break;

    case PLAYER_ATTACK1:   
        switch( m_Activity )
        {
        case ACT_HOVER:
        case ACT_SWIM:
        case ACT_HOP:
        case ACT_LEAP:
        case ACT_DIESIMPLE:
        case ACT_RELOAD:
            m_IdealActivity = m_Activity;
            break;
        default:
            m_IdealActivity = ACT_RANGE_ATTACK1;
            break;
        }
        break;
    case PLAYER_IDLE:
    case PLAYER_WALK:
        if ( !FBitSet( pev->flags, FL_ONGROUND ) && (m_Activity == ACT_HOP || m_Activity == ACT_LEAP) )    // Still jumping
        {
            m_IdealActivity = m_Activity;
        }
        else if ( pev->waterlevel > 1 )
        {
            if ( speed == 0 )
                m_IdealActivity = ACT_HOVER;
            else
                m_IdealActivity = ACT_SWIM;
        }
        else
        {
            m_IdealActivity = ACT_WALK;
        }
        break;
    }

    switch (m_IdealActivity)
    {
    case ACT_HOVER:
    case ACT_LEAP:
    case ACT_SWIM:
    case ACT_HOP:
    case ACT_DIESIMPLE:
    default:
        if ( m_Activity == m_IdealActivity)
            return;
        m_Activity = m_IdealActivity;

        animDesired = LookupActivity( m_Activity );
        // Already using the desired animation?
        if (pev->sequence == animDesired)
            return;

        pev->gaitsequence = 0;
        pev->sequence        = animDesired;
        pev->frame            = 0;
        ResetSequenceInfo( );
        return;

    case ACT_RANGE_ATTACK1:
        if ( FBitSet( pev->flags, FL_DUCKING ) )    // crouching
            strcpy( szAnim, "crouch_shoot_" );
        else
            strcpy( szAnim, "ref_shoot_" );
        strcat( szAnim, m_szAnimExtention );
        animDesired = LookupSequence( szAnim );
        if (animDesired == -1)
            animDesired = 0;

        if ( pev->sequence != animDesired || !m_fSequenceLoops )
        {
            pev->frame = 0;
        }

        if (!m_fSequenceLoops)
        {
            pev->effects |= EF_NOINTERP;
        }

        m_Activity = m_IdealActivity;

        pev->sequence        = animDesired;
        ResetSequenceInfo( );
        break;
case ACT_RELOAD:
 if( m_fProne )
  strcpy( szAnim, "prone_reload_" );
 else if ( FBitSet( pev->flags, FL_DUCKING ) ) // crouching
  strcpy( szAnim, "crouch_reload_" );
 else
  strcpy( szAnim, "ref_reload_" );

 strcat( szAnim, m_szAnimExtention );
 animDesired = LookupSequence( szAnim );
 if (animDesired == -1)
  animDesired = 0;

 if ( pev->sequence != animDesired || !m_fSequenceLoops )
 {
  pev->frame = 0;
 }

 if (!m_fSequenceLoops)
 {
  pev->effects |= EF_NOINTERP;
 }

 m_Activity = m_IdealActivity;

 pev->sequence  = animDesired;
 ResetSequenceInfo( );
 break;
    case ACT_WALK://sp
  if ((m_Activity != ACT_RANGE_ATTACK1 && m_Activity != ACT_RELOAD ) || m_fSequenceFinished )
 {
            if ( FBitSet( pev->flags, FL_DUCKING ) )    // crouching
                strcpy( szAnim, "crouch_aim_" );
            else
                strcpy( szAnim, "ref_aim_" );
            strcat( szAnim, m_szAnimExtention );
            animDesired = LookupSequence( szAnim );
            if (animDesired == -1)
                animDesired = 0;
            m_Activity = ACT_WALK;
        }
        else
        {
            animDesired = pev->sequence;
        }
    }

    if ( FBitSet( pev->flags, FL_DUCKING ) )
    {
        if ( speed == 0)
        {
            pev->gaitsequence    = LookupActivity( ACT_CROUCHIDLE );
            // pev->gaitsequence    = LookupActivity( ACT_CROUCH );
        }
        else
        {
            pev->gaitsequence    = LookupActivity( ACT_CROUCH );
        }
    }
    else if ( speed > 150 )//220? el player tiene animacion rara... que sea realmente cuando vaya despacio
    {
        pev->gaitsequence    = LookupActivity( ACT_RUN );
    }
    else if (speed > 0)
    {
        pev->gaitsequence    = LookupActivity( ACT_WALK );
    }
    else
    {
        // pev->gaitsequence    = LookupActivity( ACT_WALK );
        pev->gaitsequence    = LookupSequence( "deep_idle" );
    }


    // Already using the desired animation?
    if (pev->sequence == animDesired)
        return;

    //ALERT( at_console, "Set animation to %d\n", animDesired );
    // Reset to first frame of desired animation
    pev->sequence        = animDesired;
    pev->frame            = 0;
    ResetSequenceInfo( );
}
/*
===========
TabulateAmmo
This function is used to find and store 
all the ammo we have into the ammo vars.
============
*/
void CBasePlayer::TabulateAmmo()
{
	ammo_556 = AmmoInventory( GetAmmoIndex( "556" ) );
	ammo_9mm = AmmoInventory( GetAmmoIndex( "9mm" ) );
	ammo_357 = AmmoInventory( GetAmmoIndex( "357" ) );
	ammo_argrens = AmmoInventory( GetAmmoIndex( "ARgrenades" ) );
	ammo_bolts = AmmoInventory( GetAmmoIndex( "bolts" ) );
	ammo_buckshot = AmmoInventory( GetAmmoIndex( "buckshot" ) );
	ammo_rockets = AmmoInventory( GetAmmoIndex( "rockets" ) );
	ammo_uranium = AmmoInventory( GetAmmoIndex( "uranium" ) );
	ammo_hornets = AmmoInventory( GetAmmoIndex( "Hornets" ) );
}


/*
===========
WaterMove
============
*/
#define AIRTIME	12		// lung full of air lasts this many seconds

void CBasePlayer::WaterMove()
{
	int air;

	if (pev->movetype == MOVETYPE_NOCLIP)
		return;

	if (pev->health < 0)
		return;

	// waterlevel 0 - not in water
	// waterlevel 1 - feet in water
	// waterlevel 2 - waist in water
	// waterlevel 3 - head in water

	if (pev->waterlevel != 3) 
	{
		// not underwater
		
		// play 'up for air' sound
		if (pev->air_finished < gpGlobals->time)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade1.wav", 1, ATTN_NORM);
		else if (pev->air_finished < gpGlobals->time + 9)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade2.wav", 1, ATTN_NORM);

		pev->air_finished = gpGlobals->time + AIRTIME;
		pev->dmg = 2;

		// if we took drowning damage, give it back slowly
		if (m_idrowndmg > m_idrownrestored)
		{
			// set drowning damage bit.  hack - dmg_drownrecover actually
			// makes the time based damage code 'give back' health over time.
			// make sure counter is cleared so we start count correctly.
			
			// NOTE: this actually causes the count to continue restarting
			// until all drowning damage is healed.

			m_bitsDamageType |= DMG_DROWNRECOVER;
			m_bitsDamageType &= ~DMG_DROWN;
			m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
		}

	}
	else
	{	// fully under water
		// stop restoring damage while underwater
		m_bitsDamageType &= ~DMG_DROWNRECOVER;
		m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;

		if (pev->air_finished < gpGlobals->time)		// drown!
		{
			if (pev->pain_finished < gpGlobals->time)
			{
				// take drowning damage
				pev->dmg += 1;
				if (pev->dmg > 5)
					pev->dmg = 5;
				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->dmg, DMG_DROWN);
				pev->pain_finished = gpGlobals->time + 1;
				
				// track drowning damage, give it back when
				// player finally takes a breath

				m_idrowndmg += pev->dmg;
			} 
		}
		else
		{
			m_bitsDamageType &= ~DMG_DROWN;
		}
	}

	if (!pev->waterlevel)
	{
		if (FBitSet(pev->flags, FL_INWATER))
		{       
			ClearBits(pev->flags, FL_INWATER);
		}
		return;
	}
	
	// make bubbles

	air = (int)(pev->air_finished - gpGlobals->time);
	if (!RANDOM_LONG(0,0x1f) && RANDOM_LONG(0,AIRTIME-1) >= air)
	{
		switch (RANDOM_LONG(0,3))
			{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim1.wav", 0.8, ATTN_NORM); break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim2.wav", 0.8, ATTN_NORM); break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim3.wav", 0.8, ATTN_NORM); break;
			case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim4.wav", 0.8, ATTN_NORM); break;
		}
	}

	if (pev->watertype == CONTENT_LAVA)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time)
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 10 * pev->waterlevel, DMG_BURN);
	}
	else if (pev->watertype == CONTENT_SLIME)		// do damage
	{
		pev->dmgtime = gpGlobals->time + 1;
		TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 4 * pev->waterlevel, DMG_ACID);
	}
	
	if (!FBitSet(pev->flags, FL_INWATER))
	{
		SetBits(pev->flags, FL_INWATER);
		pev->dmgtime = 0;
	}
}


// TRUE if the player is attached to a ladder
BOOL CBasePlayer::IsOnLadder( void )
{ 
	return ( pev->movetype == MOVETYPE_FLY );
}

void CBasePlayer::PlayerDeathThink(void)
{
	float flForward;

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		flForward = pev->velocity.Length() - 20;
		if (flForward <= 0)
			pev->velocity = g_vecZero;
		else    
			pev->velocity = flForward * pev->velocity.Normalize();
	}

	if ( HasWeapons() )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}
		
	//CLIENT_COMMAND ( pev, "thirdperson \n");

	if (pev->modelindex && (!m_fSequenceFinished) && (pev->deadflag == DEAD_DYING))
	{
		StudioFrameAdvance( );

		m_iRespawnFrames++;				// Note, these aren't necessarily real "frames", so behavior is dependent on # of client movement commands
		if ( m_iRespawnFrames < 120 )   // Animations should be no longer than this
			return;
	}

	// once we're done animating our death and we're on the ground, we want to set movetype to None so our dead body won't do collisions and stuff anymore
	// this prevents a bug where the dead body would go to a player's head if he walked over it while the dead player was clicking their button to respawn
	if ( pev->movetype != MOVETYPE_NONE && FBitSet(pev->flags, FL_ONGROUND) )
		pev->movetype = MOVETYPE_NONE;

	if (pev->deadflag == DEAD_DYING)
		pev->deadflag = DEAD_DEAD;
	
	StopAnimation();

	pev->effects |= EF_NOINTERP;
	pev->framerate = 0.0;

	BOOL fAnyButtonDown = (pev->button & ~IN_SCORE );
	
	// wait for all buttons released
	if (pev->deadflag == DEAD_DEAD)
	{
		if (fAnyButtonDown)
			return;

		if ( g_pGameRules->FPlayerCanRespawn( this ) )
		{
			m_fDeadTime = gpGlobals->time;
			pev->deadflag = DEAD_RESPAWNABLE;
		}
		
		return;
	}

// if the player has been dead for one second longer than allowed by forcerespawn, 
// forcerespawn isn't on. Send the player off to an intermission camera until they 
// choose to respawn.
	
	if ( g_pGameRules->IsMultiplayer() && ( gpGlobals->time > (m_fDeadTime + 0.1) ) && !(m_afPhysicsFlags & PFLAG_OBSERVER) )
	{
		// go to dead camera. 
		StartDeathCam();
	}

// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
	if (!fAnyButtonDown 
		&& !( g_pGameRules->IsMultiplayer() && forcerespawn.value > 0 && (gpGlobals->time > (m_fDeadTime + 5))) )
		return;

	pev->button = 0;
	m_iRespawnFrames = 0;

		// clear all particlesystems with this hijacked message
		extern int gmsgParticles;
		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_SHORT(9999);
			WRITE_STRING("");
		MESSAGE_END();

	ALERT(at_console, "Resetting Particle Engine\n");
	
	SERVER_COMMAND( "hud_draw 1\n" );

	if ( !g_pGameRules->IsMultiplayer() )
	{
		SERVER_COMMAND( "volume 0.8 \n" );//sys
		SERVER_COMMAND( "host_framerate 0\n" );	
		SERVER_COMMAND( "firstperson\n" );//fix
		CVAR_SET_FLOAT( "slowmo", 0);

		//reset models
		SERVER_COMMAND("cl_showheadcrab 0\n");
		SERVER_COMMAND("cl_showparachute 0\n");
		//reset models
		
		StartBurning = FALSE;

		//reset the entities in the radar
		//called after by the postthink func.
		MESSAGE_BEGIN( MSG_ONE, gmsgHudRadar, NULL, pev ); // Set the message parameters - what type of msg + where to send 
			WRITE_BYTE( FALSE );
			WRITE_BYTE( 0 ); // total number of targets on the radar 
			WRITE_BYTE( 0 );
			WRITE_BYTE( 0 );
		MESSAGE_END();
		//reset the entities in the radar

//advanced NVG
		// inform the client about the change
		MESSAGE_BEGIN(MSG_ONE, gmsgNVGActivate, NULL, pev);
			WRITE_BYTE( 0 );
		MESSAGE_END( );
	}

	respawn(pev, !(m_afPhysicsFlags & PFLAG_OBSERVER) );// don't copy a corpse if we're in deathcam.
	pev->nextthink = -1;
}

//=========================================================
// StartDeathCam - find an intermission spot and send the
// player off into observer mode
//=========================================================
void CBasePlayer::StartDeathCam( void )
{
	edict_t *pSpot, *pNewSpot;
	int iRand;

	if ( pev->view_ofs == g_vecZero )
	{
		// don't accept subsequent attempts to StartDeathCam()
		return;
	}

	pSpot = FIND_ENTITY_BY_CLASSNAME( NULL, "info_intermission");	

	if ( !FNullEnt( pSpot ) )
	{
		// at least one intermission spot in the world.
		iRand = RANDOM_LONG( 0, 3 );

		while ( iRand > 0 )
		{
			pNewSpot = FIND_ENTITY_BY_CLASSNAME( pSpot, "info_intermission");
			
			if ( pNewSpot )
			{
				pSpot = pNewSpot;
			}

			iRand--;
		}

		CopyToBodyQue( pev );
		StartObserver( pSpot->v.origin, pSpot->v.v_angle );
	}
	else
	{
		// no intermission spot. Push them up in the air, looking down at their corpse
		TraceResult tr;
		CopyToBodyQue( pev );
		UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, 128 ), ignore_monsters, edict(), &tr );
//		UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, 512 ), ignore_monsters, edict(), &tr );
	
		StartObserver( tr.vecEndPos, UTIL_VecToAngles( tr.vecEndPos - pev->origin  ) );
	}
}

void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle )
{
	m_afPhysicsFlags |= PFLAG_OBSERVER;

	pev->view_ofs = g_vecZero;
	pev->angles = pev->v_angle = vecViewAngle;
	pev->fixangle = TRUE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;
	pev->modelindex = 0;
	UTIL_SetOrigin( pev, vecPosition );
}

//HL TOWN
//****** HL: TOWN
void CBasePlayer::PlayerMoveForward ( void )
{
	if (!(pev->button & IN_FORWARD) || !pCar)
		return;

	pCar->Use(this, this, USE_SET, 0);
}

void CBasePlayer::PlayerMoveBackward ( void )
{
	if (!(pev->button & IN_BACK) || !pCar)
		return;

	pCar->Use(this, this, USE_SET, 1);
}

void CBasePlayer::PlayerMoveLeft ( void )
{
	if (!pCar)
		return;

	if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_MOVELEFT))
	{
		if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_MOVERIGHT))
		{
			pCar->Use(this, this, USE_SET, 7);
		}
		return;
	}

	pCar->Use(this, this, USE_SET, 2);
}

void CBasePlayer::PlayerMoveRight ( void )
{
	if (!pCar)
		return;

	if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_MOVERIGHT))
	{
		if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_MOVELEFT))
		{
			pCar->Use(this, this, USE_SET, 8);
		}
		return;
	}

	pCar->Use(this, this, USE_SET, 3);
}

//car break
void CBasePlayer::PlayerCarJump ( void )
{
	if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_JUMP) || !pCar)
		return;

	pCar->Use(this, this, USE_SET, 4);
}

//fix camera angle
void CBasePlayer::PlayerCarHorn ( void )
{
	if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_RELOAD) || !pCar)
		return;

	if (flCarHornTime > gpGlobals->time)
		return;

	flCarHornTime = gpGlobals->time + 1.0;

	pCar->Use(this, this, USE_SET, 5);
}

void CBasePlayer::PlayerCarDuck ( void )
{
	if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_DUCK) || !pCar)
		return;

	if (flCarDuckTime > gpGlobals->time)
		return;

	flCarDuckTime = gpGlobals->time + 0.5;

	switch (iCarFixAngle)
	{
	case 0:
		iCarFixAngle = 1;
		break;
	case 1:
		iCarFixAngle = 2;
		break;
	case 2:
	default:
		iCarFixAngle = 0;
		break;
	}
}


//camera zoom in
void CBasePlayer::PlayerCarAttack ( void )
{
	if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_ATTACK) || !pCar)
		return;

	if (iCarFixAngle == 1)
		fl_CarViewOffset += 1;
	else
		fl_CarViewOffset += 3;

	if (fl_CarViewOffset <= 0)
		fl_CarViewOffset = 0;
	else if (fl_CarViewOffset >= 512)
		fl_CarViewOffset = 512;


	Vector vecFor;
	Vector vecUp;
	UTIL_MakeVectorsPrivate(pCar->pev->angles, vecFor, NULL, vecUp);

	float fl_viewvalue;
	fl_viewvalue = 1;

	if (pCar->pev->fuser4 < 0)
		fl_viewvalue = -1;

	if (iCarFixAngle == 1)
	{
		pev->origin = pCar->pev->origin - fl_viewvalue*vecFor.Normalize()*(64+2*fl_CarViewOffset) + vecUp.Normalize()*(32+fl_CarViewOffset);
	}
	else if (iCarFixAngle == 2)
	{
		pev->v_angle.x = pev->angles.x = 90;
		pev->v_angle.y = pev->angles.y = pCar->pev->angles.y;
		pev->v_angle.z = pev->angles.z = 0;

		pev->fixangle = TRUE;

		pev->origin = pCar->pev->origin + Vector(0,0,1)*(96+3*fl_CarViewOffset);
	}
	else
	{
		Vector vecViewFor;
		UTIL_MakeVectorsPrivate(pev->v_angle,vecViewFor,NULL,NULL);

		Vector vecLookAt = pCar->pev->origin;

		pev->origin = vecLookAt - vecViewFor.Normalize()*(96+fl_CarViewOffset*1.5);//fl_viewvalue*vecFor.Normalize()*(64+2*fl_CarViewOffset) + vecUp.Normalize()*(32+fl_CarViewOffset);
		if (pev->origin.z < vecLookAt.z)
			pev->origin.z = vecLookAt.z;
	}

		TraceResult tr;

		UTIL_TraceLine(pCar->pev->origin,pev->origin,ignore_monsters,dont_ignore_glass,ENT(pCar->pev),&tr);

		if (tr.flFraction != 1.0)
		{
			float value;
			value = 1;
			if ((pCar->pev->origin-pev->origin).Length() >= 64)
			{
				value = 32;
			}
			if (tr.vecPlaneNormal)
				pev->origin = tr.vecEndPos + tr.vecPlaneNormal.Normalize()*value;
			else
				pev->origin = tr.vecEndPos + (pCar->pev->origin-pev->origin).Normalize()*value;
		}
}

//camera zoom out

void CBasePlayer::PlayerCarAttack2 ( void )
{
	if ( !((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_ATTACK2) || !pCar)
		return;

	if (iCarFixAngle == 1)
		fl_CarViewOffset -= 1;
	else
		fl_CarViewOffset -= 3;


	if (fl_CarViewOffset <= 0)
		fl_CarViewOffset = 0;
	else if (fl_CarViewOffset >= 512)
		fl_CarViewOffset = 512;

	Vector vecFor;
	Vector vecUp;
	UTIL_MakeVectorsPrivate(pCar->pev->angles, vecFor, NULL, vecUp);

	float fl_viewvalue;
	fl_viewvalue = 1;

	if (pCar->pev->fuser4 < 0)
		fl_viewvalue = -1;

	if (iCarFixAngle == 1)
	{
		pev->origin = pCar->pev->origin - fl_viewvalue*vecFor.Normalize()*(64+2*fl_CarViewOffset) + vecUp.Normalize()*(32+fl_CarViewOffset);
	}
	else if (iCarFixAngle == 2)
	{
		pev->v_angle.x = pev->angles.x = 90;
		pev->v_angle.y = pev->angles.y = pCar->pev->angles.y;
		pev->v_angle.z = pev->angles.z = 0;

		pev->fixangle = TRUE;

		pev->origin = pCar->pev->origin + Vector(0,0,1)*(96+3*fl_CarViewOffset);
	}
	else
	{
		Vector vecViewFor;
		UTIL_MakeVectorsPrivate(pev->v_angle,vecViewFor,NULL,NULL);

		Vector vecLookAt = pCar->pev->origin;

		pev->origin = vecLookAt - vecViewFor.Normalize()*(96+fl_CarViewOffset*1.5);//fl_viewvalue*vecFor.Normalize()*(64+2*fl_CarViewOffset) + vecUp.Normalize()*(32+fl_CarViewOffset);
		if (pev->origin.z < vecLookAt.z)
			pev->origin.z = vecLookAt.z;
	}

		TraceResult tr;

		UTIL_TraceLine(pCar->pev->origin,pev->origin,ignore_monsters,dont_ignore_glass,ENT(pCar->pev),&tr);

		if (tr.flFraction != 1.0)
		{
			float value;
			value = 1;
			if ((pCar->pev->origin-pev->origin).Length() >= 64)
			{
				value = 32;
			}
			if (tr.vecPlaneNormal)
				pev->origin = tr.vecEndPos + tr.vecPlaneNormal.Normalize()*value;
			else
				pev->origin = tr.vecEndPos + (pCar->pev->origin-pev->origin).Normalize()*value;
		}
}

//car exit
void CBasePlayer::PlayerCarUse ( void )
{
	if ( !(pev->button & IN_USE) || !pCar)
		return;

		if (flCarUseTime > gpGlobals->time)
			return;

		//set next possible car using time to 1 seond
		flCarUseTime = gpGlobals->time + 1.0; //only use car every 1 second

		pCar->Use(this,this,USE_SET,6);

		//spawn player next to car position
		Vector vecRight;
		Vector vecCarAngles;
		Vector vecCarOrigin;

		vecCarAngles = pCar->pev->angles;
		vecCarOrigin = pCar->pev->origin;

		UTIL_MakeVectorsPrivate(vecCarAngles, NULL, vecRight, NULL);
		pev->origin = vecCarOrigin - (vecRight.Normalize()*64);

		pev->origin.z += 64; //dont stuck in floor
		DROP_TO_FLOOR ( ENT(pev) );

		pev->angles = vecCarAngles;
		pev->v_angle = vecCarAngles;
		pev->fixangle = TRUE;
		pev->velocity = Vector(0,0,0);
		pev->avelocity = Vector(0,0,0);

		//delete car information
		pev->view_ofs = VEC_VIEW;
		pev->effects &= ~EF_NODRAW;

		pev->takedamage		= DAMAGE_AIM;
		pev->solid			= SOLID_SLIDEBOX;
		pev->movetype		= MOVETYPE_WALK;

		pCar = NULL;

		if ( m_pActiveItem )
			m_pActiveItem->Deploy();

		ALERT( at_console, "stopped using car!\n");

		SERVER_COMMAND("cl_showplayer 1\n");	

		m_iHideHUD &= ~HIDEHUD_WEAPONS;

		EMIT_SOUND( ENT(pev), CHAN_ITEM, "motor_shut_off1.wav", 1, ATTN_NORM );

		pev->flags = saved_flags;
}


//****** HL: TOWN

// 
// PlayerUse - handles USE keypress
//
void CBasePlayer::PlayerUse ( void )
{
	//there is some problems whit this LRC's version of player use. Let's check if there is a car
	//near us. If so, so let's use the new code. If not we will use the normal version ('cause there
	//is some problems using some entities)
	BOOL bNearCar = FALSE;

	CBaseEntity *pEntidadToCheck = NULL;

    while ((pEntidadToCheck = UTIL_FindEntityInSphere( pEntidadToCheck, pev->origin, 128 )) != NULL)
	{	
		if (FClassnameIs( pEntidadToCheck->pev, "env_car") || FClassnameIs( pEntidadToCheck->pev, "env_car_brush") || FClassnameIs( pEntidadToCheck->pev, "func_car_brush") || FClassnameIs( pEntidadToCheck->pev, "func_car") || FClassnameIs( pEntidadToCheck->pev, "car_rally") || FClassnameIs( pEntidadToCheck->pev, "car_fork") || FClassnameIs( pEntidadToCheck->pev, "vehicle_tank"))
		{
			bNearCar = TRUE;		
		}
	}

	if (!bNearCar)//let's use the normal version
	{
		// Was use pressed or released?
		if ( ! ((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
			return;

		// Hit Use on a train?
		if ( m_afButtonPressed & IN_USE )
		{
			if ( m_pTank != NULL )
			{
				// Stop controlling the tank
				// TODO: Send HUD Update
				m_pTank->Use( this, this, USE_OFF, 0 );
				m_pTank = NULL;
				return;
			}
			else
			{
				if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
				{
					m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
					m_iTrain = TRAIN_NEW|TRAIN_OFF;
					return;
				}
				else
				{	// Start controlling the train!
					CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );

					if ( pTrain && !(pev->button & IN_JUMP) && FBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev) )
					{
						m_afPhysicsFlags |= PFLAG_ONTRAIN;
						m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
						m_iTrain |= TRAIN_NEW;
						EMIT_SOUND( ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM);
						return;
					}
				}
			}
		}

		CBaseEntity *pObject = NULL;
		CBaseEntity *pClosest = NULL;
		Vector		vecLOS;
		float flMaxDot = VIEW_FIELD_NARROW;
		float flDot;

		UTIL_MakeVectors ( pev->v_angle );// so we know which way we are facing
		
		while ((pObject = UTIL_FindEntityInSphere( pObject, pev->origin, PLAYER_SEARCH_RADIUS )) != NULL)
		{
			if (pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
			{
				// !!!PERFORMANCE- should this check be done on a per case basis AFTER we've determined that
				// this object is actually usable? This dot is being done for every object within PLAYER_SEARCH_RADIUS
				// when player hits the use key. How many objects can be in that area, anyway? (sjb)
				vecLOS = (VecBModelOrigin( pObject->pev ) - (pev->origin + pev->view_ofs));
				
				// This essentially moves the origin of the target to the corner nearest the player to test to see 
				// if it's "hull" is in the view cone
				vecLOS = UTIL_ClampVectorToBox( vecLOS, pObject->pev->size * 0.5 );
				
				flDot = DotProduct (vecLOS , gpGlobals->v_forward);
				if (flDot > flMaxDot )
				{// only if the item is in front of the user
					pClosest = pObject;
					flMaxDot = flDot;
	//				ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
				}
	//			ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
			}
		}
		pObject = pClosest;

		// Found an object
		if (pObject )
		{			
			//!!!UNDONE: traceline here to prevent USEing buttons through walls			
			int caps = pObject->ObjectCaps();

			if ( m_afButtonPressed & IN_USE )
				EMIT_SOUND( ENT(pev), CHAN_ITEM, "common/wpn_select.wav", 0.4, ATTN_NORM);

			if ( ( (pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE) ) ||
				 ( (m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE|FCAP_ONOFF_USE)) ) )
			{
				if ( caps & FCAP_CONTINUOUS_USE )
					m_afPhysicsFlags |= PFLAG_USING;

				pObject->Use( this, this, USE_SET, 1 );
			}
			// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
			else if ( (m_afButtonReleased & IN_USE) && (pObject->ObjectCaps() & FCAP_ONOFF_USE) )	// BUGBUG This is an "off" use
			{
				pObject->Use( this, this, USE_SET, 0 );
			}
		}
		else
		{
			if ( m_afButtonPressed & IN_USE )
				EMIT_SOUND( ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.4, ATTN_NORM);
		}
	}//eo bNearCar check
	else
	{
		if ( ! ((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
			return;

		if (pCar)
		{
			ALERT(at_notice, "Drueckt Use im Auto\n");
			return;
		}

			ALERT(at_notice, "Drueckt Use nicht im Auto\n");

		// Hit Use on a train?
		if ( m_afButtonPressed & IN_USE )
		{
			if ( m_pTank != NULL )
			{
				// Stop controlling the tank
				// TODO: Send HUD Update
				m_pTank->Use( this, this, USE_OFF, 0 );
				m_pTank = NULL;
				return;
			}
			else
			{
				if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
				{
					m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
					m_iTrain = TRAIN_NEW|TRAIN_OFF;
					return;
				}
				else
				{	// Start controlling the train!
					CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );

					if ( pTrain && !(pev->button & IN_JUMP) && FBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev) )
					{
						m_afPhysicsFlags |= PFLAG_ONTRAIN;
						m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
						m_iTrain |= TRAIN_NEW;
						EMIT_SOUND( ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM);
						return;
					}
				}
			}
		}

		CBaseEntity *pObject = NULL;
		CBaseEntity *pClosest = NULL;
		Vector		vecLOS;
		float flMaxDot = VIEW_FIELD_NARROW;
		float flDot;
		TraceResult tr;
		int caps;

		UTIL_MakeVectors ( pev->v_angle );// so we know which way we are facing

		//LRC- try to get an exact entity to use.
		// (is this causing "use-buttons-through-walls" problems? Surely not!)
		UTIL_TraceLine( pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + (gpGlobals->v_forward * PLAYER_SEARCH_RADIUS), dont_ignore_monsters, ENT(pev), &tr );
		if (tr.pHit)
		{
			pObject = CBaseEntity::Instance(tr.pHit);
			if (pObject)
			{
				if (FClassnameIs( pObject->pev, "env_car") || FClassnameIs( pObject->pev, "env_car_brush") || FClassnameIs( pObject->pev, "func_car_brush") || FClassnameIs( pObject->pev, "func_car")  || FClassnameIs( pObject->pev, "car_rally") || FClassnameIs( pObject->pev, "car_fork") || FClassnameIs( pObject->pev, "vehicle_tank"))
				{
					if (flCarUseTime > gpGlobals->time)
						return;

					//set next possible car using time to 1 seond
					flCarUseTime = gpGlobals->time + 1.0; //only use car every 1 second

					//set car information
					pCar = pObject;

					pev->effects |= EF_NODRAW;

					pev->takedamage		= NULL;
					pev->solid			= SOLID_NOT;
					pev->movetype		= MOVETYPE_NOCLIP;

					SERVER_COMMAND("cl_showplayer 0\n");

					ALERT( at_console, "using car!\n");

					if ( m_pActiveItem )
					{
						m_pActiveItem->Holster();
						pev->weaponmodel = 0;
						pev->viewmodel = 0; 
					}

					m_iHideHUD |= HIDEHUD_WEAPONS;

					EMIT_SOUND( ENT(pev), CHAN_ITEM, "motor_start1.wav", 1, ATTN_NORM );

					saved_flags = pev->flags;
					pev->flags = FL_SPECTATOR;

					Vector vecFor;
					Vector vecUp;
					UTIL_MakeVectorsPrivate(pCar->pev->angles, vecFor, NULL, vecUp);

					if (fl_CarViewOffset <= 0)
						fl_CarViewOffset = 0;
					else if (fl_CarViewOffset >= 512)
						fl_CarViewOffset = 512;

					//pev->origin = pCar->pev->origin - vecFor.Normalize()*64 + vecUp.Normalize()*32;
					//pev->view_ofs = -vecFor.Normalize()*fl_CarViewOffset*2 + vecUp.Normalize()*fl_CarViewOffset;

					pev->origin = pCar->pev->origin - vecFor.Normalize()*(64+2*fl_CarViewOffset) + vecUp.Normalize()*(32+fl_CarViewOffset);

					pev->angles = UTIL_VecToAngles(vecFor.Normalize()*64 - vecUp.Normalize()*32);
					pev->angles.x = -pev->angles.x;

					pev->v_angle = UTIL_VecToAngles(vecFor.Normalize()*64 - vecUp.Normalize()*32);
					pev->v_angle.x = -pev->v_angle.x;

					pev->fixangle = TRUE;

					return;
				}
					
				//this don't work
				if (FClassnameIs( pObject->pev, "func_tbutton"))
				{
					pObject = NULL;
				}

			}
			else if (/*!pObject || */!(pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE)))
			{
				pObject = NULL;
			}
		}

		if (!pObject) //LRC- couldn't find a direct solid object to use, try the normal method
		{
			while ((pObject = UTIL_FindEntityInSphere( pObject, pev->origin, PLAYER_SEARCH_RADIUS )) != NULL)
			{
				caps = pObject->ObjectCaps();
				if (caps & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE) && !(caps & FCAP_ONLYDIRECT_USE)) //LRC - we can't see 'direct use' entities in this section
				{
					// !!!PERFORMANCE- should this check be done on a per case basis AFTER we've determined that
					// this object is actually usable? This dot is being done for every object within PLAYER_SEARCH_RADIUS
					// when player hits the use key. How many objects can be in that area, anyway? (sjb)
					vecLOS = (VecBModelOrigin( pObject->pev ) - (pev->origin + pev->view_ofs));

	//				ALERT(at_console, "absmin %f %f %f, absmax %f %f %f, mins %f %f %f, maxs %f %f %f, size %f %f %f\n", pObject->pev->absmin.x, pObject->pev->absmin.y, pObject->pev->absmin.z, pObject->pev->absmax.x, pObject->pev->absmax.y, pObject->pev->absmax.z, pObject->pev->mins.x, pObject->pev->mins.y, pObject->pev->mins.z, pObject->pev->maxs.x, pObject->pev->maxs.y, pObject->pev->maxs.z, pObject->pev->size.x, pObject->pev->size.y, pObject->pev->size.z);//LRCTEMP
					// This essentially moves the origin of the target to the corner nearest the player to test to see
					// if it's "hull" is in the view cone
					vecLOS = UTIL_ClampVectorToBox( vecLOS, pObject->pev->size * 0.5 );

					flDot = DotProduct (vecLOS , gpGlobals->v_forward);
					if (flDot > flMaxDot || vecLOS == g_vecZero ) // LRC - if the player is standing inside this entity, it's also ok to use it.
					{// only if the item is in front of the user
						pClosest = pObject;
						flMaxDot = flDot;
	//					ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
					}
	//				ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
				}
			}
			pObject = pClosest;
		}

		// Found an object
		if (pObject )
		{			
			//!!!UNDONE: traceline here to prevent USEing buttons through walls			
			int caps = pObject->ObjectCaps();

			if ( m_afButtonPressed & IN_USE )
				EMIT_SOUND( ENT(pev), CHAN_ITEM, "common/wpn_select.wav", 0.4, ATTN_NORM);

			if ( ( (pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE) ) ||
				 ( (m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE|FCAP_ONOFF_USE)) ) )
			{
				if ( caps & FCAP_CONTINUOUS_USE )
					m_afPhysicsFlags |= PFLAG_USING;

				pObject->Use( this, this, USE_SET, 1 );
			}
			// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
			else if ( (m_afButtonReleased & IN_USE) && (pObject->ObjectCaps() & FCAP_ONOFF_USE) )	// BUGBUG This is an "off" use
			{
				pObject->Use( this, this, USE_SET, 0 );
			}
		}
		else
		{
			if ( m_afButtonPressed & IN_USE )
				EMIT_SOUND( ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.4, ATTN_NORM);
		}
	}
}



void CBasePlayer::Jump()
{
	Vector		vecWallCheckDir;// direction we're tracing a line to find a wall when walljumping
	Vector		vecAdjustedVelocity;
	Vector		vecSpot;
	TraceResult	tr;

	if (pCar)
	{
		pCar->Use(this, this, USE_SET, 4);

		return;
	}

	if (FBitSet(pev->flags, FL_WATERJUMP))
		return;
	
	if (pev->waterlevel >= 2)
	{
		return;
	}

	// jump velocity is sqrt( height * gravity * 2)

	// If this isn't the first frame pressing the jump button, break out.
	if ( !FBitSet( m_afButtonPressed, IN_JUMP ) )
		return;         // don't pogo stick

	if ( !(pev->flags & FL_ONGROUND) || !pev->groundentity )
	{
		return;
	}

// many features in this function use v_forward, so makevectors now.
	UTIL_MakeVectors (pev->angles);

	// ClearBits(pev->flags, FL_ONGROUND);		// don't stairwalk
	
	SetAnimation( PLAYER_JUMP );
						
//	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 600, 0.3 );
	
	int xp_points = CVAR_GET_FLOAT( "xp_points" );

//sys add
	if ( m_fLongJump && (pev->button & IN_DUCK) && ( pev->flDuckTime > 0 ) && pev->velocity.Length() > 50 )
	{
		SetAnimation( PLAYER_SUPERJUMP );
/*
		if (!m_fHurted)	
		{
	//		SetAnimation( PLAYER_SUPERJUMP );
			//to do: aumentar daño gradualmente segun experiencia
			if(xp_points >=100)//tiene experiencia como para hacer kung fu
			{
				SetAnimation( PLAYER_SUPERJUMP );
			}
			else
			{
				ClientPrint(pev, HUD_PRINTCENTER, "#NoXPForKungFU");
				SetAnimation( PLAYER_JUMP );
			}
		}
		else
		{
			ClientPrint(pev, HUD_PRINTCENTER, "#HasBeenHurtedKK");
		}
		*/
	}

	// If you're standing on a conveyor, add it's velocity to yours (for momentum)
	entvars_t *pevGround = VARS(pev->groundentity);
	if ( pevGround && (pevGround->flags & FL_CONVEYOR) )
	{
		pev->velocity = pev->velocity + pev->basevelocity;
	}
}



// This is a glorious hack to find free space when you've crouched into some solid space
// Our crouching collisions do not work correctly for some reason and this is easier
// than fixing the problem :(
void FixPlayerCrouchStuck( edict_t *pPlayer )
{
	TraceResult trace;

	// Move up as many as 18 pixels if the player is stuck.
	for ( int i = 0; i < 18; i++ )
	{
		UTIL_TraceHull( pPlayer->v.origin, pPlayer->v.origin, dont_ignore_monsters, head_hull, pPlayer, &trace );
		if ( trace.fStartSolid )
			pPlayer->v.origin.z ++;
		else
			break;
	}
}

void CBasePlayer::Duck( )
{
	if (pCar)
	return;

	if (pev->button & IN_DUCK) 
	{
		if ( m_IdealActivity != ACT_LEAP )
		{
			SetAnimation( PLAYER_WALK );
		}
	}
}

//
// ID's player as such.
//
int  CBasePlayer::Classify ( void )
{
	return CLASS_PLAYER;	
}


void CBasePlayer::AddPoints( int score, BOOL bAllowNegativeScore )
{
	// Positive score always adds
	if ( score < 0 )
	{
		if ( !bAllowNegativeScore )
		{
			if ( pev->frags < 0 )		// Can't go more negative
				return;
			
			if ( -score > pev->frags )	// Will this go negative?
			{
				score = -pev->frags;		// Sum will be 0
			}
		}
	}

	pev->frags += score;

	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( ENTINDEX(edict()) );
		WRITE_SHORT( pev->frags );
		WRITE_SHORT( m_iDeaths );
		WRITE_SHORT( 0 );
		WRITE_SHORT( g_pGameRules->GetTeamIndex( m_szTeamName ) + 1 );
	MESSAGE_END();
}


void CBasePlayer::AddPointsToTeam( int score, BOOL bAllowNegativeScore )
{
	int index = entindex();

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

		if ( pPlayer && i != index )
		{
			if ( g_pGameRules->PlayerRelationship( this, pPlayer ) == GR_TEAMMATE )
			{
				pPlayer->AddPoints( score, bAllowNegativeScore );
			}
		}
	}
}

//Player ID
void CBasePlayer::InitStatusBar()
{
	m_flStatusBarDisappearDelay = 0;
	m_SbarString1[0] = m_SbarString0[0] = 0; 
}

void CBasePlayer::UpdateStatusBar()
{
	int newSBarState[ SBAR_END ];
	char sbuf0[ SBAR_STRING_SIZE ];
	char sbuf1[ SBAR_STRING_SIZE ];

	memset( newSBarState, 0, sizeof(newSBarState) );
	strcpy( sbuf0, m_SbarString0 );
	strcpy( sbuf1, m_SbarString1 );

	// Find an ID Target
	TraceResult tr;
	UTIL_MakeVectors( pev->v_angle + pev->punchangle );
	Vector vecSrc = EyePosition();
	Vector vecEnd = vecSrc + (gpGlobals->v_forward * MAX_ID_RANGE);
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);

	if (tr.flFraction != 1.0)
	{
		if ( !FNullEnt( tr.pHit ) )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
//sys id
			if (pEntity->Classify() == CLASS_PLAYER_ALLY /*|| CLASS_HUMAN_MILITARY*/)
			{
				if (pEntity->IsAlive())
				{
					newSBarState[ SBAR_ID_TARGETNAME ] = ENTINDEX( pEntity->edict() );
					strcpy( sbuf1, "1\n2 Health: %i2%%\n3 Armor: %i3%%" );

					newSBarState[ SBAR_ID_TARGETHEALTH ] = 100 * (pEntity->pev->health / pEntity->pev->max_health);
					newSBarState[ SBAR_ID_TARGETARMOR ] = pEntity->pev->armorvalue; //No need to get it % based since 100 it's the max.

					m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
				}
			}
		}
		else if ( m_flStatusBarDisappearDelay > gpGlobals->time )
		{
			// hold the values for a short amount of time after viewing the object
			newSBarState[ SBAR_ID_TARGETNAME ] = m_izSBarState[ SBAR_ID_TARGETNAME ];
			newSBarState[ SBAR_ID_TARGETHEALTH ] = m_izSBarState[ SBAR_ID_TARGETHEALTH ];
			newSBarState[ SBAR_ID_TARGETARMOR ] = m_izSBarState[ SBAR_ID_TARGETARMOR ];
		}
	}

	BOOL bForceResend = FALSE;

	if ( strcmp( sbuf0, m_SbarString0 ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 0 );
			WRITE_STRING( sbuf0 );
		MESSAGE_END();

		strcpy( m_SbarString0, sbuf0 );

		// make sure everything's resent
		bForceResend = TRUE;
	}

	if ( strcmp( sbuf1, m_SbarString1 ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 1 );
			WRITE_STRING( sbuf1 );
		MESSAGE_END();

		strcpy( m_SbarString1, sbuf1 );

		// make sure everything's resent
		bForceResend = TRUE;
	}

	// Check values and send if they don't match
	for (int i = 1; i < SBAR_END; i++)
	{
		if ( newSBarState[i] != m_izSBarState[i] || bForceResend )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgStatusValue, NULL, pev );
				WRITE_BYTE( i );
				WRITE_SHORT( newSBarState[i] );
			MESSAGE_END();

			m_izSBarState[i] = newSBarState[i];
		}
	}
}/*
void CBasePlayer::UpdateStatusBar()
{
	int newSBarState[ SBAR_END ];
	char sbuf0[ SBAR_STRING_SIZE ];
	char sbuf1[ SBAR_STRING_SIZE ];

	memset( newSBarState, 0, sizeof(newSBarState) );
	strcpy( sbuf0, m_SbarString0 );
	strcpy( sbuf1, m_SbarString1 );

	// Find an ID Target
	TraceResult tr;
	UTIL_MakeVectors( pev->v_angle + pev->punchangle );
	Vector vecSrc = EyePosition();
	Vector vecEnd = vecSrc + (gpGlobals->v_forward * MAX_ID_RANGE);
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);

	if (tr.flFraction != 1.0)
	{
		if ( !FNullEnt( tr.pHit ) )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
//sys id
			if (pEntity->Classify() == CLASS_PLAYER_ALLY )
			{
				newSBarState[ SBAR_ID_TARGETNAME ] = ENTINDEX( pEntity->edict() );
				strcpy( sbuf1, "1 %p1\n2 Health: %i2%%\n3 Armor: %i3%%" );

				// allies and medics get to see the targets health
				if ( g_pGameRules->PlayerRelationship( this, pEntity ) == GR_TEAMMATE )
				{
					newSBarState[ SBAR_ID_TARGETHEALTH ] = 100 * (pEntity->pev->health / pEntity->pev->max_health);
					newSBarState[ SBAR_ID_TARGETARMOR ] = pEntity->pev->armorvalue; //No need to get it % based since 100 it's the max.
				}

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
		}
		else if ( m_flStatusBarDisappearDelay > gpGlobals->time )
		{
			// hold the values for a short amount of time after viewing the object
			newSBarState[ SBAR_ID_TARGETNAME ] = m_izSBarState[ SBAR_ID_TARGETNAME ];
			newSBarState[ SBAR_ID_TARGETHEALTH ] = m_izSBarState[ SBAR_ID_TARGETHEALTH ];
			newSBarState[ SBAR_ID_TARGETARMOR ] = m_izSBarState[ SBAR_ID_TARGETARMOR ];
		}
	}

	BOOL bForceResend = FALSE;

	if ( strcmp( sbuf0, m_SbarString0 ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 0 );
			WRITE_STRING( sbuf0 );
		MESSAGE_END();

		strcpy( m_SbarString0, sbuf0 );

		// make sure everything's resent
		bForceResend = TRUE;
	}

	if ( strcmp( sbuf1, m_SbarString1 ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 1 );
			WRITE_STRING( sbuf1 );
		MESSAGE_END();

		strcpy( m_SbarString1, sbuf1 );

		// make sure everything's resent
		bForceResend = TRUE;
	}

	// Check values and send if they don't match
	for (int i = 1; i < SBAR_END; i++)
	{
		if ( newSBarState[i] != m_izSBarState[i] || bForceResend )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgStatusValue, NULL, pev );
				WRITE_BYTE( i );
				WRITE_SHORT( newSBarState[i] );
			MESSAGE_END();

			m_izSBarState[i] = newSBarState[i];
		}
	}
}

*/
CBaseEntity *FindEntityForward( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}


#define CLIMB_SHAKE_FREQUENCY	22	// how many frames in between screen shakes when climbing
#define	MAX_CLIMB_SPEED			200	// fastest vertical climbing speed possible
#define	CLIMB_SPEED_DEC			15	// climbing deceleration rate
#define	CLIMB_PUNCH_X			-7  // how far to 'punch' client X axis when climbing
#define CLIMB_PUNCH_Z			7	// how far to 'punch' client Z axis when climbing

#define DOOR		1
#define LADDER		2
#define BREKABLE	3
#define	TRAIN		4
#define	PLAT		5

#define SF_DOOR_USE_ONLY			256	// door must be opened by player's use button.

void CBasePlayer::PreThink(void)
{
	int buttonsChanged = (m_afButtonLast ^ pev->button);	// These buttons have changed this frame
	
	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed =  buttonsChanged & pev->button;		// The ones that changed and are now down are "pressed"
	m_afButtonReleased = buttonsChanged & (~pev->button);	// The ones that changed and aren't down are "released"

	g_pGameRules->PlayerThink( this );

	if ( g_fGameOver )
		return;         // intermission or finale

	UTIL_MakeVectors(pev->v_angle);             // is this still used?
	
	ItemPreFrame( );
	WaterMove();

	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();
	
	CheckTimeBasedDamage();

	CheckSuitUpdate();

	if (pev->deadflag >= DEAD_DYING)
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
		pev->flags |= FL_ONTRAIN;
	else 
		pev->flags &= ~FL_ONTRAIN;

	// Train speed control
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
	{
		CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );
		float vel;
		
		if ( !pTrain )
		{
			TraceResult trainTrace;
			// Maybe this is on the other side of a level transition
			UTIL_TraceLine( pev->origin, pev->origin + Vector(0,0,-38), ignore_monsters, ENT(pev), &trainTrace );

			// HACKHACK - Just look for the func_tracktrain classname
			if ( trainTrace.flFraction != 1.0 && trainTrace.pHit )
			pTrain = CBaseEntity::Instance( trainTrace.pHit );


			if ( !pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(pev) )
			{
				//ALERT( at_error, "In train mode with no train!\n" );
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
		}
		else if ( !FBitSet( pev->flags, FL_ONGROUND ) || FBitSet( pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL ) || (pev->button & (IN_MOVELEFT|IN_MOVERIGHT) ) )
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW|TRAIN_OFF;
			return;
		}

		pev->velocity = g_vecZero;
		vel = 0;
		if ( m_afButtonPressed & IN_FORWARD )
		{
			vel = 1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}
		else if ( m_afButtonPressed & IN_BACK )
		{
			vel = -1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}

		if (vel)
		{
			m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
			m_iTrain |= TRAIN_ACTIVE|TRAIN_NEW;
		}

	} else if (m_iTrain & TRAIN_ACTIVE)
		m_iTrain = TRAIN_NEW; // turn off train

	if (pev->button & IN_JUMP)
	{
		// If on a ladder, jump off the ladder
		// else Jump
		Jump();
	}


	// If trying to duck, already ducked, or in the process of ducking
	if ((pev->button & IN_DUCK) || FBitSet(pev->flags,FL_DUCKING) || (m_afPhysicsFlags & PFLAG_DUCKING) )
		Duck();

	if ( !FBitSet ( pev->flags, FL_ONGROUND ) )
	{
		m_flFallVelocity = -pev->velocity.z;
	}

	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Clear out ladder pointer
	m_hEnemy = NULL;

	if ( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		pev->velocity = g_vecZero;
	}

//******************************************************************************************
//******************************************************************************************

	if ( CVAR_GET_FLOAT("dev_override_prethink" ) != 0 )
	{
		ClientPrint(pev, HUD_PRINTCENTER, "PRETHINK OVERRIDE");
		return;
	}
#if CODE_CHK

	if ( CVAR_GET_FLOAT("commentary" ) == 0 )//only if commentary is disabled
	{
		if ( CVAR_GET_FLOAT("cl_tips" ) != 0 )	
		{
			if ( f_NextTipTime < gpGlobals->time )
			{
				if (i_TipCounter <= 0)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP00")));//this tip won't be showed
				else if (i_TipCounter == 1)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP01")));
				else if (i_TipCounter == 2)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP02")));
				else if (i_TipCounter == 3)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP03")));
				else if (i_TipCounter == 4)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP04")));
				else if (i_TipCounter == 5)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP05")));
				else if (i_TipCounter == 6)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP06")));
				else if (i_TipCounter == 7)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP07")));
				else if (i_TipCounter == 8)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP08")));
				else if (i_TipCounter == 9)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP09")));
				else if (i_TipCounter == 10)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP10")));
				else if (i_TipCounter == 11)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP11")));
				else if (i_TipCounter == 12)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP12")));
				else if (i_TipCounter == 13)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP13")));
				else if (i_TipCounter == 14)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP14")));
				else if (i_TipCounter == 15)
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP15")));
				else//well, too many tips, so disable for future
				{
					CVAR_SET_FLOAT( "cl_tips", 0 );
					i_TipCounter = 0;
				}

				i_TipCounter++;

				f_NextTipTime = gpGlobals->time + 30;//1/2 min
			}
		}
	}
	else//you can't use tips while commentary is activated
	{
		if ( f_NextTipTime < gpGlobals->time )
		{
			UTIL_ShowMessageAll( STRING(ALLOC_STRING("TIP_ERROR")));

			f_NextTipTime = gpGlobals->time + 99999;//27 hours (too much you think?)
			return;
		}
	}
#endif

	if ( !g_pGameRules->IsDeathmatch() )
	{
		if(m_bHEVBroken)
		{
			if (RANDOM_LONG(0,1))
				SERVER_COMMAND( "hud_draw 0\n" );
			else
				SERVER_COMMAND( "hud_draw 1\n" );
		}

		if ( pev->weapons & (1<<WEAPON_SUIT) )
		{	
			if (pev->armorvalue <= 0)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgNVGActivate, NULL, pev);
					WRITE_BYTE( 1 );
				MESSAGE_END( );
					
				if ( m_flNextNoiseValueUpdateTime < gpGlobals->time )
				{
					switch ( RANDOM_LONG(0,5) )
					{
						case 0: SetSuitUpdate("!HEV_E0", FALSE, SUIT_NEXT_IN_30SEC); break;
						case 1: SetSuitUpdate("!HEV_E1", FALSE, SUIT_NEXT_IN_30SEC); break;
						case 2: SetSuitUpdate("!HEV_E2", FALSE, SUIT_NEXT_IN_30SEC); break;
						case 3: SetSuitUpdate("!HEV_E3", FALSE, SUIT_NEXT_IN_30SEC); break;
						case 4: SetSuitUpdate("!HEV_E4", FALSE, SUIT_NEXT_IN_30SEC); break;
						case 5: SetSuitUpdate("!HEV_E5", FALSE, SUIT_NEXT_IN_30SEC); break;
					}
					m_flNextNoiseValueUpdateTime = gpGlobals->time + 30;
				}
			}
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgNVGActivate, NULL, pev);
					WRITE_BYTE( 0 );
				MESSAGE_END( );
			}
		}
	}

	if (pev->armorvalue == 100)
		m_bActivadorTieneFullTraje = TRUE;
	else
		m_bActivadorTieneFullTraje = FALSE;

	if (pev->health == 100)
		m_bActivadorTieneFullVida = TRUE;
	else
		m_bActivadorTieneFullVida = FALSE;

	if (pCar)
	return;

	Vector		vecBlastSrc;
	vecBlastSrc = Center();
	

	CBaseEntity *pCommentaryEntity;

	pCommentaryEntity = FindEntityForward( this );

	if ( pCommentaryEntity )
	{
		if ( FClassnameIs( pCommentaryEntity->pev, "info_commentary" ) )
		{
			g_bPlayerPointCommentary = TRUE;
		}
	}
	else
	{
		g_bPlayerPointCommentary = FALSE;
	}

	if ( pev->weapons & (1<<WEAPON_SUIT) )
	pev->body = 0;
	else
	pev->body = 1;
					
	int gl_texturedetail = CVAR_GET_FLOAT( "gl_texturedetail" );

	if ( !g_pGameRules->IsDeathmatch() )
	{
		if ( m_flNextTDetailUpdateTime < gpGlobals->time )
		{
			if (gl_texturedetail == 1)
			SERVER_COMMAND("gl_texturemode GL_NEAREST\n");
			if (gl_texturedetail == 2)
			SERVER_COMMAND("gl_texturemode GL_LINEAR\n");
			if (gl_texturedetail == 3)
			SERVER_COMMAND("gl_texturemode GL_NEAREST_MIPMAP_NEAREST\n");
			if (gl_texturedetail == 4)
			SERVER_COMMAND("gl_texturemode GL_LINEAR_MIPMAP_NEAREST\n");
			if (gl_texturedetail == 5)
			SERVER_COMMAND("gl_texturemode GL_NEAREST_MIPMAP_LINEAR\n");
			if (gl_texturedetail == 6)
			SERVER_COMMAND("gl_texturemode GL_LINEAR_MIPMAP_LINEAR\n");
		
			//small add here
			//if we have a hl2 weapon we have a suit, if so, lets allow flashlight property
			/*
			if(b_UsingHL2Weapon)
			m_fCanUseFlashlight = TRUE;*/

			m_flNextTDetailUpdateTime = gpGlobals->time + 2;
		}
	}

	CBaseEntity *pEntidadS = NULL;
	
	Vector VecSrc;
	VecSrc = pev->origin;

	while ((pEntidadS = UTIL_FindEntityInSphere( pEntidadS, VecSrc, 256 )) != NULL)//512
	{	
		//if ( FClassnameIs(pEntidadS->pev, "flash_grenade") )//ugly hack: the player cannot detect those lights as real lights...
		if ( FClassnameIs(pEntidadS->pev, "smoke_grenade") )
		{		

		}
	}

	//fog fix
	//send cl_fog 0 every time. if a env_fog entity is detected, send cl_fog 1 over (fix 
	//transparent textures
	//UNDONE: this must be done	at client side

	if ( m_flNextFogValueUpdateTime < gpGlobals->time )
	{
		SERVER_COMMAND("cl_fog 0\n");

		CBaseEntity *pFogEntity = NULL;
		while ((pFogEntity = UTIL_FindEntityInSphere( pFogEntity, pev->origin, 9999 )) != NULL)//500
		{
			if ( FClassnameIs(pFogEntity->pev, "env_fog"))
			{//aaa
				SERVER_COMMAND("cl_fog 1\n");
				//ALERT ( at_console, "Env_Fog entity detected\n" );
			}
		}
		m_flNextFogValueUpdateTime = gpGlobals->time + 2;
	}


	//FLASH LIGHT
	if (b_FlashlightIsOn)
	{/*
		if ( m_flNextBitsSoundsUpdateTime < gpGlobals->time )
		{
			//so, the enemies can "detect" the light
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 1000, 0.3 );

			m_flNextBitsSoundsUpdateTime = gpGlobals->time + 2;
		}*/
		// Teh_Freak: World Lighting!
		 MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			  WRITE_BYTE( TE_DLIGHT );
			  WRITE_COORD( pev->origin.x ); // origin
			  WRITE_COORD( pev->origin.y );
			  WRITE_COORD( pev->origin.z );
			  WRITE_BYTE( 20 );     // radius
			  WRITE_BYTE( 255 );     // R
			  WRITE_BYTE( 255 );     // G
			  WRITE_BYTE( 255 );     // B
			  WRITE_BYTE( 0 );     // life * 10
			  WRITE_BYTE( 0 ); // decay
		 MESSAGE_END();
		 	/*
		 MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_ELIGHT );
			WRITE_SHORT( entindex( ) + 0x3000 );		// entity, attachment
			WRITE_COORD( pev->origin.x );		// origin
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( 256 );	// radius
			WRITE_BYTE( 255 );	// R
			WRITE_BYTE( 255 );	// G
			WRITE_BYTE( 255 );	// B
			WRITE_BYTE( 0 );	// life * 10
			WRITE_COORD( 1 ); // decay
		MESSAGE_END();*/
	}

	if ( !g_pGameRules->IsDeathmatch() )
	{	
		//bueno, a ver: enviar el sprite en 0, si esta cerca, ponerlo en 1
		SERVER_COMMAND("neartalk 0\n");
		SERVER_COMMAND("nearbutton 0\n");
		SERVER_COMMAND("neardoor 0\n");
		SERVER_COMMAND("nearweapon 0\n");

		CBaseEntity *pEntidadTalk = NULL;

		while ((pEntidadTalk = UTIL_FindEntityInSphere( pEntidadTalk, pev->origin, 80 )) != NULL)
		{			
			if ( FClassnameIs(pEntidadTalk->pev, "trigger_dialogue"))
			{
				SERVER_COMMAND( "neartalk 1\n" );
			}
			else if ( FClassnameIs(pEntidadTalk->pev, "func_button")			
				|| FClassnameIs(pEntidadTalk->pev, "func_healthcharger")
				|| FClassnameIs(pEntidadTalk->pev, "func_tbutton")
				|| FClassnameIs(pEntidadTalk->pev, "func_tank")
				|| FClassnameIs(pEntidadTalk->pev, "func_rot_button")
				|| FClassnameIs(pEntidadTalk->pev, "func_traincontrols")
				|| FClassnameIs(pEntidadTalk->pev, "func_recharge")
				)
			{
				SERVER_COMMAND("nearbutton 1\n");
			}
			else if ( FClassnameIs(pEntidadTalk->pev, "func_door") || FClassnameIs(pEntidadTalk->pev, "func_door_rotating"))
			{
				if(pEntidadTalk->pev->targetname)
				{
	//				ALERT ( at_console, "La puerta tiene nombre para ser activada/desactivada\n");
				}
				else if ( FBitSet ( pEntidadTalk->pev->spawnflags, SF_DOOR_USE_ONLY ) )
				{
	//				ALERT ( at_console, "La puerta tiene spawnflags de -Solo Uso-\n");
				}
				else
				{
					SERVER_COMMAND("nearbutton 1\n");
				}
			}
			else
			{

			}
		}
	}
///////////////// grunt sentences code	   
	CBaseEntity *pEntidad = NULL;
    while ((pEntidad = UTIL_FindEntityInSphere( pEntidad, pev->origin, 150 )) != NULL)//500
	{
//        float flDist = (pEntidad->Center() - pev->origin).Length();
	
		//	ALERT ( at_console, "La puerta esta a unas %f unidades de distancia\n",flDist );

		if ( FClassnameIs(pEntidad->pev, "func_door") || FClassnameIs(pEntidad->pev, "func_door_rotating"))
		{
			b_PlayerIsNearOf = DOOR;
		//	ClientPrint(pev, HUD_PRINTCENTER, "Estas cerca de una puerta");
		}
		else if ( FClassnameIs(pEntidad->pev, "func_breakable"))
		{	
			b_PlayerIsNearOf = BREKABLE;

			if (pEntidad->pev->playerclass == 1)
			{
//				ClientPrint(pev, HUD_PRINTCENTER, "Estas cerca un vidrio rompible");
				//g_engfuncs.pfnSetPhysicsKeyValue( edict(), "glass", "1" );
			}
			else
			{
				//g_engfuncs.pfnSetPhysicsKeyValue( edict(), "glass", "0" );	
			}

		//	ClientPrint(pev, HUD_PRINTCENTER, "Estas cerca de algo rompible");
		}
		else if ( FClassnameIs(pEntidad->pev, "func_ladder"))
		{
			b_PlayerIsNearOf = LADDER;
		//	ClientPrint(pev, HUD_PRINTCENTER, "Estas cerca de una escalera");
		}
		else if ( FClassnameIs(pEntidad->pev, "func_train"))
		{
			b_PlayerIsNearOf = TRAIN;
		//	ClientPrint(pev, HUD_PRINTCENTER, "Estas cerca de un tren");
		}
		else if ( FClassnameIs(pEntidad->pev, "func_plat"))
		{
			b_PlayerIsNearOf = PLAT;
		//	ClientPrint(pev, HUD_PRINTCENTER, "Estas cerca de un ascensor");
		}
    }

	//change the player's XP points and save to cvar
	//Total points es xp_points, ej: iTotalExperiencePoints = 10
	
	int xp_points = CVAR_GET_FLOAT( "xp_points" );

				
	if ( IsAlive() && pev->weapons & (1<<WEAPON_SUIT) )
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMoney, NULL, pev);
			WRITE_SHORT( CVAR_GET_FLOAT( "xp_points") );
		MESSAGE_END();
	}

	if( b_InFuncSemtexZone && m_bSemtexInWorld == FALSE )//the player is in the place and there is-nt any c4 yet
	{
		ClientPrint(pev, HUD_PRINTCENTER, "#InSemtexZone");
	}
	//antes estaba abajo
	b_InFuncSemtexZone = FALSE;

	if ( !g_pGameRules->IsMultiplayer() )//this cause laggin in mp EDIT: Am it's not MP anymore, so don't take care
	{
		if ( IsAlive() && pev->health <= 40)//valor max.: 1.25 | valor min sin fix: 0.05 | valor min. con fix: 0.2 
		{
			//corazon
			if ( m_flNextTumbleTime < gpGlobals->time )
			{
				EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/damage/corazon.wav", 1.0, ATTN_NORM );
			
				if (pev->health <= 40)
				m_flNextTumbleTime = gpGlobals->time + 2;
				if (pev->health <= 30)
				m_flNextTumbleTime = gpGlobals->time + 1.5;
				if (pev->health <= 20)
				m_flNextTumbleTime = gpGlobals->time + 1;
				if (pev->health <= 10)
				m_flNextTumbleTime = gpGlobals->time + 0.5;
			}

			//TraceBlood
			if ( pev->velocity.Length2D() >= 10)
			{
				if ( m_flPlayerNextBloodTime < gpGlobals->time )
				{
					SetSuitUpdate("!HEV_DMG6", FALSE, SUIT_NEXT_IN_30SEC);	// blood loss detected

					//float health = pev->health * 0.1 / 2;
					//if (health <= 0.2) { health = 0.2; }//fix

					Vector		vecSpot;
					TraceResult	tr;
					vecSpot = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
					UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -64 ),  ignore_monsters, ENT(pev), & tr);			
					UTIL_DecalTrace( &tr, DECAL_BLOODDRIP1 +RANDOM_LONG(0,5) );//24

					m_flPlayerNextBloodTime = gpGlobals->time + 0.25;
				}
			}
		}
	}

#if SYS
	//OVERRIDE CHEATS.
	//Solo si es single play. Esto es medio estupido porque en MP no pueden hacerse cheats
	//pero es para evitar problemas con el modo spectador, que necesita dar el movimiento de MOVETYPE_NOCLIP
	if ( mp_am_truquitos.value == 0  )
	{
		if ( (pev->movetype == MOVETYPE_NOCLIP) || ( pev->flags & FL_GODMODE ) )
		{
			MessageBox(NULL, "SPA:\nLo siento pero no puedes utilizar trucos!\nTu partida ha sido guardada\nPresiona OK para cerrar ArrangeMode\n\nENG:\nSorry but you can not entry cheat codes!\nYour game has been saved\nPress Ok to exit Arrange Mode", "CHEAT DETECTED", MB_OK | MB_ICONERROR);
			SERVER_COMMAND("autosave\n");
			SERVER_COMMAND("quit\n");
		}
	}
#endif

	int speed;//320
	float iPesoArmas = 0;
	
	speed = 280;//320

	if ( m_fHurted )
		speed = 100;//180

	else
	{
		//I hate this cvar. This was used in MP games. Don't need it no more (anyway it's a good cheat)
		if ( mp_am_weaponweight.value != 0  ) //no es cero esta _Activado_
		{
			//Comprobar si posee X arma y restar a speed (320) X puntos de velocidad.
			/*
			if ( (pev->weapons & (1<<WEAPON_CROWBAR)) )//knife
			{
				speed -= 1;
				iPesoArmas += 1;
			}
			if ( (pev->weapons & (1<<WEAPON_DEAGLE)) )//1911
			{
				speed -= 5;
				iPesoArmas += 1.5;
			}
			if ( (pev->weapons & (1<<WEAPON_GLOCK)) )//mp5
			{
				speed -= 4;
				iPesoArmas += 1;
			}
			if ( (pev->weapons & (1<<WEAPON_GLOCK18)) )
			{
				speed -= 3;
				iPesoArmas += 0.8;
			}
			if ( (pev->weapons & (1<<WEAPON_BER92F)) )
			{
				speed -= 3;
				iPesoArmas += 0.9;
			}
			if ( (pev->weapons & (1<<WEAPON_MP5)) )
			{
				speed -= 11;
				iPesoArmas += 2;
			}
			if ( (pev->weapons & (1<<WEAPON_UZI)) )
			{
				speed -= 12;
				iPesoArmas += 1.8;
			}
			if ( (pev->weapons & (1<<WEAPON_AK74)) )
			{
				speed -= 15;
				iPesoArmas += 2.5;
			}
			if ( (pev->weapons & (1<<WEAPON_M16)) )
			{
				speed -= 18;
				iPesoArmas += 3;
			}
			if ( (pev->weapons & (1<<WEAPON_FAMAS)) )
			{
				speed -= 16;
				iPesoArmas += 2.8;
			}
			if ( (pev->weapons & (1<<WEAPON_SHOTGUN)) )
			{
				speed -= 20;
				iPesoArmas += 4;
			}
			if ( (pev->weapons & (1<<WEAPON_USAS)) )
			{
				speed -= 22;
				iPesoArmas += 5;
			}
			if ( (pev->weapons & (1<<WEAPON_SNIPER)) )
			{
				speed -= 20;
				iPesoArmas += 5;
			}
			if ( (pev->weapons & (1<<WEAPON_M249)) )
			{
				speed -= 55;//
				iPesoArmas += 10;
			}
			if ( (pev->weapons & (1<<WEAPON_HANDGRENADE)) )
			{
				speed -= 5;
				iPesoArmas += 1;
			}
			if ( (pev->weapons & (1<<WEAPON_RPG)) )
			{
				speed -= 22;
				iPesoArmas += 10;
			}
*/
			//TEST
			if(pev->button & (IN_MOVELEFT|IN_MOVERIGHT)) //test to see if you are moving left or right 
			speed -= 35;

			if(xp_points >=2000)
			speed += 100;

			//Posible fix a bug en caso de expandir el codigo
			if ( speed <= 20 )
			{
				speed = 20;
			}

			if ( speed <= 100 )//200
			{
				ClientPrint(pev, HUD_PRINTCENTER, "#Too_Slow" );
			}
		}//cierre cvar
	}//hurted

	if(xp_points >=8000)
	{
		g_engfuncs.pfnSetClientMaxspeed( ENT( pev ), 320 );
	}
	else
	{
		g_engfuncs.pfnSetClientMaxspeed( ENT( pev ), speed );
	}

	int pulso = 0;

	if ( pev->flags & FL_ONGROUND ) 
	{	
		if ( pev->flags & FL_DUCKING ) 
		{	//  si esta en el suelo y agachado
			if (m_fCrosshairOff == TRUE) { SERVER_COMMAND( "hud_newcross_size 0\n" ); }
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);

					if( m_iWeaponFlash >=256)//disparando
						WRITE_STRING("hud_newcross_size 3\n");
					else if ( m_iWeaponFlash >=128)//disparando
						WRITE_STRING("hud_newcross_size 2\n");
					else
						WRITE_STRING("hud_newcross_size 1\n");

					WRITE_BYTE(1);//say_team creo
				MESSAGE_END();

//					SERVER_COMMAND( "hud_newcross_size 1\n" );//MAP SMALL
			}				
			pulso = 0;//seamos más generosos
		}
		else // si no es porq esta parado
		{
			if ( (pev->velocity.Length2D() >= 100 && pev->velocity.Length2D() < 220 ) )//walk 220 //120-300
			{
				if (m_fCrosshairOff == TRUE) { SERVER_COMMAND( "hud_newcross_size 0\n" ); }
				else
				{
					MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);

					if ( m_iWeaponFlash >=128)//disparando
						WRITE_STRING("hud_newcross_size 4\n");
					else
						WRITE_STRING("hud_newcross_size 3\n");

						WRITE_BYTE(1);//say_team creo
					MESSAGE_END();

//						SERVER_COMMAND( "hud_newcross_size 3\n" );//MAP BIG
				}
			//	pulso = 3;
			}
			else if (pev->velocity.Length2D() >= 220 ) //RUN
			{			
				if (m_fCrosshairOff == TRUE) { SERVER_COMMAND( "hud_newcross_size 0\n" ); }
				else
				{
					MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);
						WRITE_STRING("hud_newcross_size 4\n");
						WRITE_BYTE(1);//say_team creo
					MESSAGE_END();

//						SERVER_COMMAND( "hud_newcross_size 4\n" );//MAP BIG
				}
			//	pulso = 4;
			}
			else//solo esta parado
			{
				if (m_fCrosshairOff == TRUE) { SERVER_COMMAND( "hud_newcross_size 0\n" ); }
				else
				{
					MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);

					if( m_iWeaponFlash >=256)//disparando
						WRITE_STRING("hud_newcross_size 4\n");
					else if ( m_iWeaponFlash >=128)//disparando
						WRITE_STRING("hud_newcross_size 3\n");
					else
						WRITE_STRING("hud_newcross_size 2\n");

						WRITE_BYTE(1);//say_team creo
					MESSAGE_END();


//						SERVER_COMMAND( "hud_newcross_size 2\n" );//MAP MEDIUM
				}
			}
			pulso = 1;//bueno, antes de que lo saquen
		}
	}
	else // si no esta en el suelo es porque esta en el aire
	{
		if (m_fCrosshairOff == TRUE) { SERVER_COMMAND( "hud_newcross_size 0\n" ); }
		else
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);
				WRITE_STRING("hud_newcross_size 5\n");
				WRITE_BYTE(1);//say_team creo
			MESSAGE_END();

//				SERVER_COMMAND( "hud_newcross_size 5\n" );//MAP TOO TOO BIG
		}
		
	//	pulso = 10;
	}

	//TO DO: let client decide? Of course not! Use the following in zoom states
	if ( CVAR_GET_FLOAT( "cl_pulso" ) == 0 )//sys
		pulso = 0;

		MESSAGE_BEGIN( MSG_ONE, gmsgConcuss, NULL, pev );
			WRITE_BYTE( pulso ); // This is the value of the effect
		MESSAGE_END();
		
//------------------------------	
// MOVIMIENTO DE ARMA:
//------------------------------

	edict_t *pClient;
	pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );

	CBaseEntity *pEntity;

	pEntity = FindEntityForward( this );
	if ( pEntity )
	{
		if ( pEntity->IsAlive() )
		{
			if ( (pEntity->Classify() == CLASS_PLAYER_ALLY) || (pEntity->Classify() == CLASS_HUMAN_PASSIVE))
			{
				//ClientPrint(pev, HUD_PRINTCENTER, "[Friend]");

				//es amigo, bajar arma
				//abajo		
				if ( posiciondemanos_abajo <= 2.5 ) //3, mucho más
					posiciondemanos_abajo += 0.1;
				
				if(posiciondemanos_abajo >= 2.5)//fix
					posiciondemanos_abajo = 2.5;
			}
//			else if( pEntity->Classify() == (CLASS_MACHINE | CLASS_HUMAN_MILITARY | CLASS_ALIEN_MILITARY | CLASS_ALIEN_MONSTER | CLASS_ALIEN_PREY | CLASS_ALIEN_PREDATOR | CLASS_FACTION_A | CLASS_FACTION_B | CLASS_FACTION_C))
			else
			{
				//FIX: una luz puede estar "viva" segun el estado, comprobar solamente entidades del tipo MOVETYPE_STEP
				if (pEntity->pev->movetype == MOVETYPE_STEP)
				{
					//ClientPrint(pev, HUD_PRINTCENTER, "[Enemy]");
					//SetSuitUpdate("!HEV_AIM_ON", FALSE, SUIT_NEXT_IN_5SEC);

					//un enemigo?? apuntemos!!!		
					if ( posiciondemanos_medio <= 2 )//menos
						posiciondemanos_medio += 0.1;//un poco más rapido
					
					if(posiciondemanos_medio >= 2.0)
						posiciondemanos_medio = 2.0;
										
					//new add
							
					if ( posiciondemanos_adentro <= 3 )
						posiciondemanos_adentro += 0.1;
			
					if(posiciondemanos_adentro >= 3.0)
						posiciondemanos_adentro = 3.0;
						
					
					if ( posiciondezoom <= 115 )
						posiciondezoom += 1;
					
					if(posiciondezoom >= 115)//fix
						posiciondezoom = 115;

					SERVER_COMMAND( "cl_crosshair_ontarget 1\n" );//found enemy!!
				}
			}
		//	else
		//	{
			//	ALERT ( at_console, "worldspawn or unreconigzed entity!\n");
		//	}
			
		}
	}
	else// no debe haber entidad... entonces comprobemos si apuntamos a la pared...
	{
		SERVER_COMMAND( "cl_crosshair_ontarget 0\n" );//no entity, no friends, no target...

		UTIL_MakeVectors( pev->v_angle );
		TraceResult tr;
		Vector trace_origin;

		trace_origin = pev->origin;
		if ( pev->flags & FL_DUCKING )
		{
			trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
		}
		// find place to toss monster
		UTIL_TraceLine( trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 64, dont_ignore_monsters, NULL, &tr );
		

		if ( tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25 )	
		{		
			/*
			if ( (pev->velocity.Length2D() >= 120 && pev->velocity.Length2D() < 300 ) )//walk 220
			{
				if ( posiciondemanos_abajo <= 2.5 ) //3, mucho más
					posiciondemanos_abajo += 0.15;
				
				if(posiciondemanos_abajo >= 2.5)
					posiciondemanos_abajo = 2.5;
			}
			*/
			if (pev->velocity.Length2D() >= 200 ) //no se, un namber //300
			{
				//this could be work but... I don't wanna think anymore...
				/*
				if(pev->button & (IN_MOVELEFT))
				{
					if ( posiciondemanos_medio >= -1.5 )//si es menos ir restando
						posiciondemanos_medio -= 0.15;
					
					if(posiciondemanos_medio <= -1.5)//fix
						posiciondemanos_medio = -1.5;
				}
				*/
				if(pev->button & (IN_MOVERIGHT))
				{
					if ( posiciondemanos_medio <= 3 )//menos
						posiciondemanos_medio += 0.15;//un poco más rapido
					
					if(posiciondemanos_medio >= 3)
						posiciondemanos_medio = 3;
				}
				

				if ( posiciondemanos_abajo <= 3 ) //3, mucho más
					posiciondemanos_abajo += 0.15;//speed 0.2
				
				if(posiciondemanos_abajo >= 3)
					posiciondemanos_abajo = 3;
			}
			else//solo esta parado
			{
			//parado
	//abajo
				if ( posiciondemanos_abajo >=0 )
					posiciondemanos_abajo -=0.1;//0.05
				
				if(posiciondemanos_abajo <= 0)
					posiciondemanos_abajo = 0;
	//hacia adentro
				if ( posiciondemanos_adentro >=0 )
					posiciondemanos_adentro -=0.1;//0.05
				
				if(posiciondemanos_adentro <= 0)
					posiciondemanos_adentro = 0;
	//al medio
				if ( posiciondemanos_medio >=0 )
					posiciondemanos_medio -=0.1;//0.05
				
				if(posiciondemanos_medio <= 0)
					posiciondemanos_medio = 0;

								
				if ( posiciondezoom >= 90 )
					posiciondezoom -= 1;
					
				if(posiciondezoom <= 90)//fix
					posiciondezoom = 90;
			}
		}
		else//pared
		{	
			//abajo		
			if ( posiciondemanos_abajo <= 2 ) //3
				posiciondemanos_abajo += 0.1;
			
			if(posiciondemanos_abajo >= 2.0)
				posiciondemanos_abajo = 2.0;
			//hacia adentro		
			/*
			if ( posiciondemanos_adentro <= 3 )
				posiciondemanos_adentro += 0.1;
			
			if(posiciondemanos_adentro >= 3.0)
				posiciondemanos_adentro = 3.0;
			*/
		}

	}//no entity
		
	if (b_EstaEnZoom)//FIX: solo si no está con zoom
	{
		posiciondemanos_abajo =0;
		posiciondemanos_adentro =0;
		posiciondemanos_medio =0;
	}

	CLIENT_COMMAND(pClient, "scr_ofsz %f\n", posiciondemanos_abajo);//down
	CLIENT_COMMAND(pClient, "scr_ofsx %f\n", posiciondemanos_adentro);//back
	CLIENT_COMMAND(pClient, "scr_ofsy %f\n", posiciondemanos_medio);//back
			
	//Fire on player: This need to be particle based (I havent time to do it now)
///////////////////////////////////////////////////////////////////////////////////////////////////////
	Vector vecForward = gpGlobals->v_forward;	
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	Vector vecSrc;

	float Scale;
	float fTrans = RANDOM_FLOAT(80, 120);
	int RenderMode = kRenderTransAdd;
	int r,g,b;

	r = RANDOM_FLOAT(230, 255);
	g = RANDOM_FLOAT(200, 255);
	b = RANDOM_FLOAT(0, 255);
		
	if (StartBurning)
	{
		if ( f_NextBurnTime < gpGlobals->time )
		{
		if ( CVAR_GET_FLOAT( "cl_detailfire" ) != 0 )
		{
	vecSrc = pev->origin + vecForward * RANDOM_FLOAT( -8, 8 ) + vecRight * RANDOM_FLOAT( -8, 8 ) 
		+ vecUp * RANDOM_FLOAT( 55, 64 );//!!-32??
	Scale = RANDOM_FLOAT( 0.1, 0.2 );
				
	UTIL_Sparks( vecSrc );

		CSprite *pSpr = CSprite::SpriteCreate( "sprites/llamas.spr", vecSrc, TRUE );
			pSpr->SetScale( Scale );
			pSpr->AnimateAndDie( RANDOM_FLOAT(35, 45) );
			pSpr->SetTransparency( RenderMode, r, g, b, fTrans, kRenderFxNoDissipation );
			pSpr->pev->velocity.z = RANDOM_FLOAT(40, 80);
			pSpr->pev->velocity.x = RANDOM_FLOAT(-32, 32);
			pSpr->pev->velocity.y = RANDOM_FLOAT(-32, 32);

	vecSrc = pev->origin + vecForward * RANDOM_FLOAT( -8, 8 ) + vecRight * RANDOM_FLOAT( -8, 8 ) 
		+ vecUp * RANDOM_FLOAT( 35, 50 );//
	Scale = RANDOM_FLOAT( 0.2, 0.3 );

	UTIL_Sparks( vecSrc );

		CSprite *pSpr2 = CSprite::SpriteCreate( "sprites/llamas.spr", vecSrc, TRUE );
			pSpr2->SetScale( Scale );
			pSpr2->AnimateAndDie( RANDOM_FLOAT(35, 45) );
			pSpr2->SetTransparency( RenderMode, r, g, b, fTrans, kRenderFxNoDissipation );
			pSpr2->pev->velocity.z = RANDOM_FLOAT(40, 80);
			pSpr2->pev->velocity.x = RANDOM_FLOAT(-32, 32);
			pSpr2->pev->velocity.y = RANDOM_FLOAT(-32, 32);

		CSprite *pSpr3 = CSprite::SpriteCreate( "sprites/llamas.spr", vecSrc, TRUE );
			pSpr3->SetScale( Scale );
			pSpr3->AnimateAndDie( RANDOM_FLOAT(35, 45) );
			pSpr3->SetTransparency( RenderMode, r, g, b, fTrans, kRenderFxNoDissipation );
			pSpr3->pev->velocity.z = RANDOM_FLOAT(40, 60);
			pSpr3->pev->velocity.x = RANDOM_FLOAT(-32, 32);
			pSpr3->pev->velocity.y = RANDOM_FLOAT(-32, 32);
		}//	cl_detailfire

	vecSrc = pev->origin + vecForward * RANDOM_FLOAT( -8, 8 ) + vecRight * RANDOM_FLOAT( -8, 8 ) 
		+ vecUp * RANDOM_FLOAT( 25, 35 );//
	Scale = RANDOM_FLOAT( 0.4, 0.6 );

		CSprite *pSpr4 = CSprite::SpriteCreate( "sprites/llamas.spr", vecSrc, TRUE );
			pSpr4->SetScale( Scale );
			pSpr4->AnimateAndDie( RANDOM_FLOAT(35, 45) );
			pSpr4->SetTransparency( RenderMode, r, g, b, fTrans, kRenderFxNoDissipation );
			pSpr4->pev->velocity.z = RANDOM_FLOAT(40, 60);
			pSpr4->pev->velocity.x = RANDOM_FLOAT(-32, 32);
			pSpr4->pev->velocity.y = RANDOM_FLOAT(-32, 32);

		CSprite *pSpr5 = CSprite::SpriteCreate( "sprites/llamas.spr", vecSrc, TRUE );
			pSpr5->SetScale( Scale );
			pSpr5->AnimateAndDie( RANDOM_FLOAT(35, 45) );
			pSpr5->SetTransparency( RenderMode, r, g, b, fTrans, kRenderFxNoDissipation );
			pSpr5->pev->velocity.z = RANDOM_FLOAT(40, 60);
			pSpr5->pev->velocity.x = RANDOM_FLOAT(-32, 32);
			pSpr5->pev->velocity.y = RANDOM_FLOAT(-32, 32);

	//VecSrc, to make light depending of the flamme's position
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(vecSrc.x);	// X
			WRITE_COORD(vecSrc.y);	// Y
			WRITE_COORD(vecSrc.z);	// Z
			WRITE_BYTE( RANDOM_FLOAT(10, 20) );		// radius * 0.1
			WRITE_BYTE( 255 );		// r
			WRITE_BYTE( 180 );		// g
			WRITE_BYTE( 100 );		// b
			WRITE_BYTE( 10 );		// time * 10
			WRITE_BYTE( 5 );		// decay * 0.1
		MESSAGE_END( );

		vecSrc = pev->origin + vecForward * RANDOM_FLOAT( -8, 8 ) + vecRight * RANDOM_FLOAT( -8, 8 ) 
		+ vecUp * RANDOM_FLOAT( 64, 80 );//

		if ( CVAR_GET_FLOAT( "cl_detailfire" ) != 0 )
		{
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", vecSrc, TRUE );		
			pSprite->SetTransparency( kRenderTransAlpha, 222, 222, 222, 111, kRenderFxNone );
			pSprite->SetScale( 3.0 );
			pSprite->Expand( 10 , RANDOM_FLOAT( 120.0, 150.0 )  );
			pSprite->pev->velocity.z = RANDOM_FLOAT(10, 20);
		}
			::RadiusDamage( pev->origin, pev, pev, 5, 64, CLASS_NONE, DMG_BURN );

			f_NextBurnTime = gpGlobals->time + 0.1;
		}

		if ( f_NextSoundBurnTime < gpGlobals->time )
		{
			if ( IsAlive() )
			{
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "ambience/burning1.wav", 0.5, ATTN_NORM );
				TakeDamage( pev, pev, 10, DMG_GENERIC );
			}
			else
			{
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/null.wav", 0.5, ATTN_NORM );
				STOP_SOUND( ENT(pev), CHAN_ITEM, "ambience/burning1.wav" );
			}
	
			f_NextSoundBurnTime = gpGlobals->time + 2;

			if ( f_NextOffTime < gpGlobals->time )
			{
				StartBurning = FALSE;
				f_NextOffTime = gpGlobals->time + 10;
			}
		}//count
	}//startburn
}

//Test for make some blurring when some grenade impacts you. The r_blur don't work for now
/*
void CBasePlayer::BlurDamage() //the function itself
{
 	//	ALERT ( at_console, "velocity is %f\n", pev->velocity.Length2D());
//	b_Blur = 5;
	int b_Blur = 10;

	while ( b_Blur<=10 )
	{
		b_Blur--;
	}

	edict_t *pClient;
	pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CLIENT_COMMAND(pClient, "r_blur %i\n", b_Blur);//down
}
*/

//Some stuff here

void CBasePlayer::CheckTimeBasedDamage() 
{
	int i;
	BYTE bDuration = 0;

	static float gtbdPrev = 0.0;

	if (!(m_bitsDamageType & DMG_TIMEBASED))
		return;

	// only check for time based damage approx. every 2 seconds
	if (abs(gpGlobals->time - m_tbdPrev) < 2.0)
		return;
	
	m_tbdPrev = gpGlobals->time;

	for (i = 0; i < CDMG_TIMEBASED; i++)
	{
		// make sure bit is set for damage type
		if (m_bitsDamageType & (DMG_PARALYZE << i))
		{
			switch (i)
			{
			case itbd_Paralyze:
				// UNDONE - flag movement as half-speed
				bDuration = PARALYZE_DURATION;
				break;
			case itbd_NerveGas:
//				TakeDamage(pev, pev, NERVEGAS_DAMAGE, DMG_GENERIC);	
				bDuration = NERVEGAS_DURATION;
				break;
			case itbd_Poison:
				TakeDamage(pev, pev, POISON_DAMAGE, DMG_GENERIC);
				bDuration = POISON_DURATION;
				break;
			case itbd_Radiation:
//				TakeDamage(pev, pev, RADIATION_DAMAGE, DMG_GENERIC);
				bDuration = RADIATION_DURATION;
				break;
			case itbd_DrownRecover:
				// NOTE: this hack is actually used to RESTORE health
				// after the player has been drowning and finally takes a breath
				if (m_idrowndmg > m_idrownrestored)
				{
					int idif = min(m_idrowndmg - m_idrownrestored, 10);

					TakeHealth(idif, DMG_GENERIC);
					m_idrownrestored += idif;
				}
				bDuration = 4;	// get up to 5*10 = 50 points back
				break;
			case itbd_Acid:
//				TakeDamage(pev, pev, ACID_DAMAGE, DMG_GENERIC);
				bDuration = ACID_DURATION;
				break;
			case itbd_SlowBurn:
//				TakeDamage(pev, pev, SLOWBURN_DAMAGE, DMG_GENERIC);
				bDuration = SLOWBURN_DURATION;
				break;
			case itbd_SlowFreeze:
//				TakeDamage(pev, pev, SLOWFREEZE_DAMAGE, DMG_GENERIC);
				bDuration = SLOWFREEZE_DURATION;
				break;
			default:
				bDuration = 0;
			}

			if (m_rgbTimeBasedDamage[i])
			{
				// use up an antitoxin on poison or nervegas after a few seconds of damage					
				if (((i == itbd_NerveGas) && (m_rgbTimeBasedDamage[i] < NERVEGAS_DURATION)) ||
					((i == itbd_Poison)   && (m_rgbTimeBasedDamage[i] < POISON_DURATION)))
				{
					if (m_rgItems[ITEM_ANTIDOTE])
					{
						m_rgbTimeBasedDamage[i] = 0;
						m_rgItems[ITEM_ANTIDOTE]--;
						SetSuitUpdate("!HEV_HEAL4", FALSE, SUIT_REPEAT_OK);
					}
				}


				// decrement damage duration, detect when done.
				if (!m_rgbTimeBasedDamage[i] || --m_rgbTimeBasedDamage[i] == 0)
				{
					m_rgbTimeBasedDamage[i] = 0;
					// if we're done, clear damage bits
					m_bitsDamageType &= ~(DMG_PARALYZE << i);	
				}
			}
			else
				// first time taking this damage type - init damage duration
				m_rgbTimeBasedDamage[i] = bDuration;
		}
	}
}

/*
THE POWER SUIT

Some boring VALVe's guy stuff here ...

...
*/

// if in range of radiation source, ping geiger counter

#define GEIGERDELAY 0.25

void CBasePlayer :: UpdateGeigerCounter( void )
{
	BYTE range;

	// delay per update ie: don't flood net with these msgs
	if (gpGlobals->time < m_flgeigerDelay)
		return;

	m_flgeigerDelay = gpGlobals->time + GEIGERDELAY;
		
	// send range to radition source to client

	range = (BYTE) (m_flgeigerRange / 4);

	if (range != m_igeigerRangePrev)
	{
		m_igeigerRangePrev = range;

		MESSAGE_BEGIN( MSG_ONE, gmsgGeigerRange, NULL, pev );
			WRITE_BYTE( range );
		MESSAGE_END();
	}

	// reset counter and semaphore
	if (!RANDOM_LONG(0,3))
		m_flgeigerRange = 1000;

}

/*
================
CheckSuitUpdate

Play suit update if it's time
================
*/

#define SUITUPDATETIME	3.5
#define SUITFIRSTUPDATETIME 0.1

void CBasePlayer::CheckSuitUpdate()
{
	int i;
	int isentence = 0;
	int isearch = m_iSuitPlayNext;
	
	// Ignore suit updates if no suit
	if ( !(pev->weapons & (1<<WEAPON_SUIT)) )
		return;

	// if in range of radiation source, ping geiger counter
	UpdateGeigerCounter();

	if ( g_pGameRules->IsMultiplayer() )
	{
		// don't bother updating HEV voice in multiplayer.
		return;
	}

	if ( gpGlobals->time >= m_flSuitUpdate && m_flSuitUpdate > 0)
	{
		// play a sentence off of the end of the queue
		for (i = 0; i < CSUITPLAYLIST; i++)
			{
			if (isentence = m_rgSuitPlayList[isearch])
				break;
			
			if (++isearch == CSUITPLAYLIST)
				isearch = 0;
			}

		if (isentence)
		{
			m_rgSuitPlayList[isearch] = 0;
			if (isentence > 0)
			{
				// play sentence number

				char sentence[CBSENTENCENAME_MAX+1];
				strcpy(sentence, "!");
				strcat(sentence, gszallsentencenames[isentence]);
				EMIT_SOUND_SUIT(ENT(pev), sentence);
			}
			else
			{
				// play sentence group
				EMIT_GROUPID_SUIT(ENT(pev), -isentence);
			}
		m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME;
		}
		else
			// queue is empty, don't check 
			m_flSuitUpdate = 0;
	}
}
 
// add sentence to suit playlist queue. if fgroup is true, then
// name is a sentence group (HEV_AA), otherwise name is a specific
// sentence name ie: !HEV_AA0.  If iNoRepeat is specified in
// seconds, then we won't repeat playback of this word or sentence
// for at least that number of seconds.

void CBasePlayer::SetSuitUpdate(char *name, int fgroup, int iNoRepeatTime)
{
	int i;
	int isentence;
	int iempty = -1;
	
	// Ignore suit updates if no suit
	if ( !(pev->weapons & (1<<WEAPON_SUIT)) )
		return;

	// Ignore suit updates if we doesn't are using a HL weapon
	/*
	if (!b_UsingHL2Weapon)
		return;*/
	
	if ( m_bHEVBroken )
	{
		//don't send any info... (save this???)
		return;
	}

	if ( m_bHEVPlayVoice == FALSE )
	{
		//don't send any voice sound... (save this???)
		return;
	}

	if ( g_pGameRules->IsMultiplayer() )
	{
		// due to static channel design, etc. We don't play HEV sounds in multiplayer right now.
		return;
	}

	// if name == NULL, then clear out the queue

	if (!name)
	{
		for (i = 0; i < CSUITPLAYLIST; i++)
			m_rgSuitPlayList[i] = 0;
		return;
	}
	// get sentence or group number
	if (!fgroup)
	{
		isentence = SENTENCEG_Lookup(name, NULL);
		if (isentence < 0)
			return;
	}
	else
		// mark group number as negative
		isentence = -SENTENCEG_GetIndex(name);

	// check norepeat list - this list lets us cancel
	// the playback of words or sentences that have already
	// been played within a certain time.

	for (i = 0; i < CSUITNOREPEAT; i++)
	{
		if (isentence == m_rgiSuitNoRepeat[i])
			{
			// this sentence or group is already in 
			// the norepeat list

			if (m_rgflSuitNoRepeatTime[i] < gpGlobals->time)
				{
				// norepeat time has expired, clear it out
				m_rgiSuitNoRepeat[i] = 0;
				m_rgflSuitNoRepeatTime[i] = 0.0;
				iempty = i;
				break;
				}
			else
				{
				// don't play, still marked as norepeat
				return;
				}
			}
		// keep track of empty slot
		if (!m_rgiSuitNoRepeat[i])
			iempty = i;
	}

	// sentence is not in norepeat list, save if norepeat time was given

	if (iNoRepeatTime)
	{
		if (iempty < 0)
			iempty = RANDOM_LONG(0, CSUITNOREPEAT-1); // pick random slot to take over
		m_rgiSuitNoRepeat[iempty] = isentence;
		m_rgflSuitNoRepeatTime[iempty] = iNoRepeatTime + gpGlobals->time;
	}

	// find empty spot in queue, or overwrite last spot
	
	m_rgSuitPlayList[m_iSuitPlayNext++] = isentence;
	if (m_iSuitPlayNext == CSUITPLAYLIST)
		m_iSuitPlayNext = 0;

	if (m_flSuitUpdate <= gpGlobals->time)
	{
		if (m_flSuitUpdate == 0)
			// play queue is empty, don't delay too long before playback
			m_flSuitUpdate = gpGlobals->time + SUITFIRSTUPDATETIME;
		else 
			m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME; 
	}

}

/*
================
CheckPowerups

Check for turning off powerups

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
================
*/
	static void
CheckPowerups(entvars_t *pev)
{
	if (pev->health <= 0)
		return;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes
}


//=========================================================
// UpdatePlayerSound - updates the position of the player's
// reserved sound slot in the sound list.
//=========================================================
void CBasePlayer :: UpdatePlayerSound ( void )
{
	int iBodyVolume;
	int iVolume;
	CSound *pSound;

	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt :: ClientSoundIndex( edict() ) );

	if ( !pSound )
	{
		ALERT ( at_console, "Client lost reserved sound!\n" );
		return;
	}

	pSound->m_iType = bits_SOUND_NONE;

	// now calculate the best target volume for the sound. If the player's weapon
	// is louder than his body/movement, use the weapon volume, else, use the body volume.
	
	if ( FBitSet ( pev->flags, FL_ONGROUND ) )
	{	
		iBodyVolume = pev->velocity.Length(); 

		// clamp the noise that can be made by the body, in case a push trigger,
		// weapon recoil, or anything shoves the player abnormally fast. 
		if ( iBodyVolume > 512 )
		{
			iBodyVolume = 512;
		}
	}
	else
	{
		iBodyVolume = 0;
	}

	if ( pev->button & IN_JUMP )
	{
		iBodyVolume += 100;
	}

// convert player move speed and actions into sound audible by monsters.
	if ( m_iWeaponVolume > iBodyVolume )
	{
		m_iTargetVolume = m_iWeaponVolume;

		// OR in the bits for COMBAT sound if the weapon is being louder than the player. 
		pSound->m_iType |= bits_SOUND_COMBAT;
	}
	else
	{
		m_iTargetVolume = iBodyVolume;
	}

	// decay weapon volume over time so bits_SOUND_COMBAT stays set for a while
	m_iWeaponVolume -= 250 * gpGlobals->frametime;
	if ( m_iWeaponVolume < 0 )
	{
		iVolume = 0;
	}


	// if target volume is greater than the player sound's current volume, we paste the new volume in 
	// immediately. If target is less than the current volume, current volume is not set immediately to the
	// lower volume, rather works itself towards target volume over time. This gives monsters a much better chance
	// to hear a sound, especially if they don't listen every frame.
	iVolume = pSound->m_iVolume;

	if ( m_iTargetVolume > iVolume )
	{
		iVolume = m_iTargetVolume;
	}
	else if ( iVolume > m_iTargetVolume )
	{
		iVolume -= 250 * gpGlobals->frametime;

		if ( iVolume < m_iTargetVolume )
		{
			iVolume = 0;
		}
	}

	if ( m_fNoPlayerSound )
	{
		// debugging flag, lets players move around and shoot without monsters hearing.
		iVolume = 0;
	}

	if ( gpGlobals->time > m_flStopExtraSoundTime )
	{
		// since the extra sound that a weapon emits only lasts for one client frame, we keep that sound around for a server frame or two 
		// after actual emission to make sure it gets heard.
		m_iExtraSoundTypes = 0;
	}

	if ( pSound )
	{
		pSound->m_vecOrigin = pev->origin;
		pSound->m_iType |= ( bits_SOUND_PLAYER | m_iExtraSoundTypes );
		pSound->m_iVolume = iVolume;
	}

	// keep track of virtual muzzle flash
	m_iWeaponFlash -= 256 * gpGlobals->frametime;
	if (m_iWeaponFlash < 0)
		m_iWeaponFlash = 0;

	//UTIL_MakeVectors ( pev->angles );
	//gpGlobals->v_forward.z = 0;

	// Below are a couple of useful little bits that make it easier to determine just how much noise the 
	// player is making. 
	// UTIL_ParticleEffect ( pev->origin + gpGlobals->v_forward * iVolume, g_vecZero, 255, 25 );
	//ALERT ( at_console, "%d/%d\n", iVolume, m_iTargetVolume );
}


void CBasePlayer::PostThink()
{
	//TO DO: Make a new weapon (like a Palm-top) to display at center this fuck*** radar. The idea is to show
	//in the radar the actual objetive. I need to make a new trigger entity like: trigger_radarobj, to display
	// it in the radar

// Define Radar range, and maximum number of entities to display
/*#define RADAR_MAX_X 500
#define RADAR_MAX_Y 500
#define RADAR_MAX_Z 100 // just in case the monsters are just above/below the player
#define RADAR_MAX_ENTITIES 10 
*/
#define RADAR_MAX_X 900
#define RADAR_MAX_Y 900
#define RADAR_MAX_Z 100 // just in case the monsters are just above/below the player
#define RADAR_MAX_ENTITIES 15 

	// SGC Radar stuff
	// Stick radar code in here for now
	// Basically a copy of the barnacles code to check if it's licking people
	Vector vTarget;
	Vector2D vTarget2D;

	// Get the player origin, and create the max/min search area for the radar.
	// any entity within this box will get added to the radar display
	Vector delta = Vector( RADAR_MAX_X, RADAR_MAX_Y, RADAR_MAX_Z );
	Vector mins = pev->origin - delta;
	Vector maxs = pev->origin + delta; 

	// Create entity + coordinate lists for the first x entities
	CBaseEntity *pRadarList[RADAR_MAX_ENTITIES];
	byte iTargetForward[RADAR_MAX_ENTITIES];
	byte iTargetRight[RADAR_MAX_ENTITIES]; 

	byte iTargetNumber = 0; 

// This returns an array of all entities within the box. You could use the entitiesinsphere command too.
// FL_MONSTER just tells it to return all the monsters. You could send FL_CLIENT too - which would 
// return all the players on a multiplayer game.
	int count = UTIL_EntitiesInBox( pRadarList, RADAR_MAX_ENTITIES, mins, maxs, (FL_MONSTER) );

// If count > 0, then there's something to display.
	if ( count )
	{
		for ( int i = 0; i < count; i++ )
		{
			// Run through the pRadarList array, checking that the entity isnt dead, or the current
			// player. If it's something that we really want, translate it's coordinates to the x,y screen
			// coordinates that we want to display. We do this by calculating the vector dot product
			// between the players line of sight and the vector to the entity.
				
			//if( pRadarList[i] != this && pRadarList[i]->pev->deadflag != DEAD_DEAD) // ignore this player and the dead
		//	if (!FClassnameIs( pRadarList[i]->pev, "monster_scientist" ))	
		
//			if ( (pRadarList[i]->Classify() == CLASS_PLAYER_ALLY) || (pRadarList[i]->Classify() == CLASS_HUMAN_PASSIVE))
		//		
			if ( (FClassnameIs( pRadarList[i]->pev, "monster_hevsci" )) || (FClassnameIs( pRadarList[i]->pev, "monster_barney_hev")) )
			{
				if( pRadarList[i] != this && pRadarList[i]->pev->deadflag != DEAD_DYING) // ignore this player and the dead
				if( pRadarList[i] != this && pRadarList[i]->pev->deadflag != DEAD_DEAD) // ignore this player and the dead
				{
					// Found target possibly in range
					// Get Target vector + flatten (x, y only - ignoring height)
					vTarget = pRadarList[i]->pev->origin - (pev->origin + pev->view_ofs);
					vTarget2D = vTarget.Make2D(); 

					// set v_right/v_forward vectors to the current line of sight
					UTIL_MakeVectors( pev->angles ); 

					// DotProduct gives us the target vector in relation to the line of sight (go and look at a maths book...)
					// Divide it by the maximum range, so that altering the radar range doesnt start doing wierd things
					// to the display
					float fTargetRight = (DotProduct( vTarget2D, gpGlobals->v_right.Make2D() ) / RADAR_MAX_X);
					float fTargetForward = (DotProduct( vTarget2D, gpGlobals->v_forward.Make2D() ) / RADAR_MAX_Y);


					// Convert to bytes for sending to the HUD (i.e. convert it to an integer from 0->256, where
					// 128 means the centre of the display).
					// Check that the target isnt out of range too, since when
					// you bring line-of-sight into the equation, we can pick up entities
					// that are outside the screen coordinates
					if (fTargetRight > -1 && fTargetRight < 1 && fTargetForward > -1 && fTargetForward < 1)
					{
						// In range, so add it to array
						iTargetRight[iTargetNumber] = 128 * (1 + fTargetRight);
						iTargetForward[iTargetNumber] = 128 * (1 + fTargetForward);
						iTargetNumber ++;

						//si el jugador está vivo y tiene el radar encendido, hacer sonido de alerta
						//EDIT: now is client side sounds
						/*
						if ( IsAlive() && bRadarIsOn)
						{
							if ( m_flRadarNextSoundTime < gpGlobals->time )
							{
								EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/radar_warning.wav", 1.0, ATTN_NORM );
								m_flRadarNextSoundTime = gpGlobals->time + 1;
							}
						}*/
						
						if (count >= 10)
						{
							SetSuitUpdate("!HEV_BIOREAD", FALSE, 10);
						}
					//	SetSuitUpdate("!HEV_BIOREAD", FALSE, 15);
					//	ALERT(at_console, "count is :%i\n",count );
					} 
				}//cierre classname
			}//cierre classname
		}
	}


//ALERT( at_console, "Sending Target Info\n"); 

// Send coordinates to CHudRadar to draw
	MESSAGE_BEGIN( MSG_ONE, gmsgHudRadar, NULL, pev ); // Set the message parameters - what type of msg + where to send 
	
	WRITE_BYTE( bRadarIsOn );
	
	WRITE_BYTE( iTargetNumber ); // total number of targets on the radar 
	for( int j = 0; j < iTargetNumber; j++)
	{ 
		// send x, y coordinates
		// we could also send the target class type etc., if we wanted so that we could draw things in diff colours
		WRITE_BYTE( iTargetRight[j] );
		WRITE_BYTE( iTargetForward[j] );
	}
	MESSAGE_END();
// End

	if ( g_fGameOver )
		goto pt_end;         // intermission or finale

	if (!IsAlive())
		goto pt_end;

	//HL: TOWN, handle car
	if (pCar)
	{
		if (pCar->pev->health == 1)
		{
			pCar->Use(this,this,USE_SET,6);
			pCar->pev->health = 2;

			//spawn player next to car position
			Vector vecRight;
			Vector vecCarAngles;
			Vector vecCarOrigin;

			vecCarAngles = pCar->pev->angles;
			vecCarOrigin = pCar->pev->origin;

			UTIL_MakeVectorsPrivate(vecCarAngles, NULL, vecRight, NULL);
			pev->origin = vecCarOrigin - (vecRight.Normalize()*64);
			pev->origin.z += 64; //dont stuck in floor 16???
			pev->angles = vecCarAngles;
			pev->v_angle = vecCarAngles;
			pev->fixangle = TRUE;
			pev->velocity = Vector(0,0,0);
			pev->avelocity = Vector(0,0,0);

			//delete car information
			pev->view_ofs = VEC_VIEW;
			pev->effects &= ~EF_NODRAW;

			pev->takedamage		= DAMAGE_AIM;
			pev->solid			= SOLID_SLIDEBOX;
			pev->movetype		= MOVETYPE_WALK;

			//UTIL_Remove(pCar)
			pCar = NULL;
			
			pev->flags = saved_flags;
			//Restore( CRestore &restore )
			TakeDamage(pev,pev,200,DMG_GENERIC);

			#if defined( CLIENT_WEAPONS )
				// HACK:	This variable is saved/restored in CBaseMonster as a time variable, but we're using it
				//			as just a counter.  Ideally, this needs its own variable that's saved as a plain float.
				//			Barring that, we clear it out here instead of using the incorrect restored time value.
				m_flNextAttack = UTIL_WeaponTimeBase();
			#endif
			return;
		}
		pev->flags = FL_SPECTATOR;

		//pev->origin = pCar->pev->origin;
		pev->velocity = pCar->pev->velocity;
		//pev->avelocity = pCar->pev->avelocity;



		if (fl_CarViewOffset <= 0)
			fl_CarViewOffset = 0;
		else if (fl_CarViewOffset >= 512)
			fl_CarViewOffset = 512;




		float fl_viewvalue;
		fl_viewvalue = 1;

		if (pCar->pev->fuser4 < 0)
			fl_viewvalue = -1;


		if (iCarFixAngle == 1)
		{
			Vector vecFor;
			Vector vecUp;
			UTIL_MakeVectorsPrivate(pCar->pev->angles, vecFor, NULL, vecUp);

			pev->angles = UTIL_VecToAngles(fl_viewvalue*vecFor.Normalize()*64 - vecUp.Normalize()*32);
			pev->angles.x = -pev->angles.x;

			pev->v_angle = UTIL_VecToAngles(fl_viewvalue*vecFor.Normalize()*64 - vecUp.Normalize()*32);
			pev->v_angle.x = -pev->v_angle.x;

			pev->fixangle = TRUE;

			pev->origin = pCar->pev->origin - fl_viewvalue*vecFor.Normalize()*(64+2*fl_CarViewOffset) + vecUp.Normalize()*(32+fl_CarViewOffset);
		}
		else if (iCarFixAngle == 2)
		{
			pev->v_angle.x = pev->angles.x = 90;
			pev->v_angle.y = pev->angles.y = pCar->pev->angles.y;
			pev->v_angle.z = pev->angles.z = 0;

			pev->fixangle = TRUE;

			pev->origin = pCar->pev->origin + Vector(0,0,1)*(96+3*fl_CarViewOffset);
		}
		else
		{
			Vector vecViewFor;
			UTIL_MakeVectorsPrivate(pev->v_angle,vecViewFor,NULL,NULL);

			Vector vecLookAt = pCar->pev->origin;
			//vecLookAt.z += 32;

			pev->origin = vecLookAt - vecViewFor.Normalize()*(96+fl_CarViewOffset*1.5);//fl_viewvalue*vecFor.Normalize()*(64+2*fl_CarViewOffset) + vecUp.Normalize()*(32+fl_CarViewOffset);
			if (pev->origin.z < vecLookAt.z)
				pev->origin.z = vecLookAt.z;
		}

		TraceResult tr;

		UTIL_TraceLine(pCar->pev->origin,pev->origin+(pev->origin-pCar->pev->origin).Normalize()*4,ignore_monsters,dont_ignore_glass,ENT(pCar->pev),&tr);

		if (tr.flFraction != 1.0)
		{
			float value;
			value = 1;
			if ((pCar->pev->origin-pev->origin).Length() >= 64)
			{
				value = 32;
			}
			if (tr.vecPlaneNormal)
				pev->origin = tr.vecEndPos + tr.vecPlaneNormal.Normalize()*value;
			else
				pev->origin = tr.vecEndPos + (pCar->pev->origin-pev->origin).Normalize()*value;
		}

		//handle car commands
		PlayerMoveForward(); //movement
		PlayerMoveBackward();
		PlayerMoveLeft();
		PlayerMoveRight();

		PlayerCarJump(); //break
		PlayerCarDuck(); //horn
		PlayerCarHorn(); //horn

		PlayerCarAttack(); //zoom camera in
		PlayerCarAttack2(); //zoom camera out

		if (/*(pev->button & IN_DUCK) ||*/ FBitSet(pev->flags,FL_DUCKING) /*|| (m_afPhysicsFlags & PFLAG_DUCKING)*/ )
		{
			pev->origin.z += 8;
		}


		PlayerCarUse(); //exit

		return;
	}
//hl rown

	// Handle Tank controlling
	if ( m_pTank != NULL )
	{ // if they've moved too far from the gun,  or selected a weapon, unuse the gun
		if ( m_pTank->OnControls( pev ) && !pev->weaponmodel )
		{  
			m_pTank->Use( this, this, USE_SET, 2 );	// try fire the gun
		}
		else
		{  // they've moved off the platform
			m_pTank->Use( this, this, USE_OFF, 0 );
			m_pTank = NULL;
		}
	}

// do weapon stuff
	ItemPostFrame( );

// check to see if player landed hard enough to make a sound
// falling farther than half of the maximum safe distance, but not as far a max safe distance will
// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
// of maximum safe distance will make no sound. Falling farther than max safe distance will play a 
// fallpain sound, and damage will be inflicted based on how far the player fell

	if ( (FBitSet(pev->flags, FL_ONGROUND)) && (pev->health > 0) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		// ALERT ( at_console, "%f\n", m_flFallVelocity );//sys

		if (pev->watertype == CONTENT_WATER)
		{
			// Did he hit the world or a non-moving entity?
			// BUG - this happens all the time in water, especially when 
			// BUG - water has current force
			// if ( !pev->groundentity || VARS(pev->groundentity)->velocity.z == 0 )
				// EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM);
		}
		else if ( m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
		{// after this point, we start doing damage
				
			//je, pretty nasty
			
			//hay un crash cuando el jugador cae...
			//removamos por ahora
			//EDIT: el crash se resolvio, intentemos una nueva forma usando server_command
			 SERVER_COMMAND("drop\n");

			/*
			CBasePlayer *pWeaponCarrier = GetClassPtr((CBasePlayer *)pev);

			if ( pWeaponCarrier->m_pActiveItem->m_iId == WEAPON_CROWBAR )
			if ( pWeaponCarrier->m_pActiveItem->m_iId == WEAPON_HANDGRENADE )
			if ( pWeaponCarrier->m_pActiveItem->m_iId == WEAPON_HEAL )
			{

			}
			else
			{
				ALERT ( at_console, "Oh my weapon!\n");

				MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);
					WRITE_STRING("drop");
					WRITE_BYTE(1);
				MESSAGE_END();
			}

//			ALERT ( at_console, "Oh my weapon!\n");

			MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);
				WRITE_STRING("drop");
				WRITE_BYTE(1);
			MESSAGE_END();
*/
			float flFallDamage = g_pGameRules->FlPlayerFallDamage( this );

			if ( flFallDamage > pev->health )
			{//splat
				// note: play on item channel because we play footstep landing on body channel
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM);
			}

			if ( flFallDamage > 0 )
			{
				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), flFallDamage, DMG_FALL ); 
				pev->punchangle.x = 0;
			}
		}

		if ( IsAlive() )
		{
			SetAnimation( PLAYER_WALK );
		}
    }

	if (FBitSet(pev->flags, FL_ONGROUND))
	{		
		if (m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer())
		{
			CSoundEnt::InsertSound ( bits_SOUND_PLAYER, pev->origin, m_flFallVelocity, 0.2 );
			// ALERT( at_console, "fall %f\n", m_flFallVelocity );
		}
		m_flFallVelocity = 0;
	}

	// select the proper animation for the player character	
	if ( IsAlive() )
	{
		if (!pev->velocity.x && !pev->velocity.y)
			SetAnimation( PLAYER_IDLE );
		else if ((pev->velocity.x || pev->velocity.y) && (FBitSet(pev->flags, FL_ONGROUND)))
			SetAnimation( PLAYER_WALK );
		else if (pev->waterlevel > 1)
			SetAnimation( PLAYER_WALK );
	}

	StudioFrameAdvance( );
	CheckPowerups(pev);

	UpdatePlayerSound();

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;

pt_end:
	    //omega; fix the corpses.
    if (pev->deadflag == DEAD_NO)
        v_LastAngles = pev->angles;
    else
        pev->angles = v_LastAngles;

#if defined( CLIENT_WEAPONS )
		// Decay timers on weapons
	// go through all of the weapons and make a list of the ones to pack
	for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			CBasePlayerItem *pPlayerItem = m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				CBasePlayerWeapon *gun;

				gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
				
				if ( gun && gun->UseDecrement() )
				{
					gun->m_flNextPrimaryAttack		= max( gun->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0 );
					gun->m_flNextSecondaryAttack	= max( gun->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001 );

					if ( gun->m_flTimeWeaponIdle != 1000 )
					{
						gun->m_flTimeWeaponIdle		= max( gun->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001 );
					}

					if ( gun->pev->fuser1 != 1000 )
					{
						gun->pev->fuser1	= max( gun->pev->fuser1 - gpGlobals->frametime, -0.001 );
					}

					// Only decrement if not flagged as NO_DECREMENT
//					if ( gun->m_flPumpTime != 1000 )
				//	{
				//		gun->m_flPumpTime	= max( gun->m_flPumpTime - gpGlobals->frametime, -0.001 );
				//	}
					
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}

	m_flNextAttack -= gpGlobals->frametime;
	if ( m_flNextAttack < -0.001 )
		m_flNextAttack = -0.001;
	
	if ( m_flNextAmmoBurn != 1000 )
	{
		m_flNextAmmoBurn -= gpGlobals->frametime;
		
		if ( m_flNextAmmoBurn < -0.001 )
			m_flNextAmmoBurn = -0.001;
	}

	if ( m_flAmmoStartCharge != 1000 )
	{
		m_flAmmoStartCharge -= gpGlobals->frametime;
		
		if ( m_flAmmoStartCharge < -0.001 )
			m_flAmmoStartCharge = -0.001;
	}
	

#else
	return;
#endif
}


// checks if the spot is clear of players
BOOL IsSpawnPointValid( CBaseEntity *pPlayer, CBaseEntity *pSpot )
{
	CBaseEntity *ent = NULL;

	if ( !pSpot->IsTriggered( pPlayer ) )
	{
		return FALSE;
	}

	while ( (ent = UTIL_FindEntityInSphere( ent, pSpot->pev->origin, 128 )) != NULL )
	{
		// if ent is a client, don't spawn on 'em
		if ( ent->IsPlayer() && ent != pPlayer )
			return FALSE;
	}

	return TRUE;
}


DLL_GLOBAL CBaseEntity	*g_pLastSpawn;
inline int FNullEnt( CBaseEntity *ent ) { return (ent == NULL) || FNullEnt( ent->edict() ); }

/*
============
EntSelectSpawnPoint

Returns the entity to spawn at

USES AND SETS GLOBAL g_pLastSpawn
============
*/
edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer )
{
	CBaseEntity *pSpot;
	edict_t		*player;

	player = pPlayer->edict();

// choose a info_player_deathmatch point
	if (g_pGameRules->IsCoOp())
	{
		pSpot = UTIL_FindEntityByClassname( g_pLastSpawn, "info_player_coop");
		if ( !FNullEnt(pSpot) )
			goto ReturnSpot;
		pSpot = UTIL_FindEntityByClassname( g_pLastSpawn, "info_player_start");
		if ( !FNullEnt(pSpot) ) 
			goto ReturnSpot;
	}
	else if ( g_pGameRules->IsDeathmatch() )
	{
		pSpot = g_pLastSpawn;
		// Randomize the start spot
		for ( int i = RANDOM_LONG(1,5); i > 0; i-- )
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
		if ( FNullEnt( pSpot ) )  // skip over the null point
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );

		CBaseEntity *pFirstSpot = pSpot;

		do 
		{
			if ( pSpot )
			{
				// check if pSpot is valid
				if ( IsSpawnPointValid( pPlayer, pSpot ) )
				{
					if ( pSpot->pev->origin == Vector( 0, 0, 0 ) )
					{
						pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
						continue;
					}

					// if so, go to pSpot
					goto ReturnSpot;
				}
			}
			// increment pSpot
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
		} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

		// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
		if ( !FNullEnt( pSpot ) )
		{
			CBaseEntity *ent = NULL;
			while ( (ent = UTIL_FindEntityInSphere( ent, pSpot->pev->origin, 128 )) != NULL )
			{
				// if ent is a client, kill em (unless they are ourselves)
				if ( ent->IsPlayer() && !(ent->edict() == player) )
					ent->TakeDamage( VARS(INDEXENT(0)), VARS(INDEXENT(0)), 300, DMG_GENERIC );
			}
			goto ReturnSpot;
		}
	}

	// If startspot is set, (re)spawn there.
	if ( FStringNull( gpGlobals->startspot ) || !strlen(STRING(gpGlobals->startspot)))
	{
		pSpot = UTIL_FindEntityByClassname(NULL, "info_player_start");
		if ( !FNullEnt(pSpot) )
			goto ReturnSpot;
	}
	else
	{
		pSpot = UTIL_FindEntityByTargetname( NULL, STRING(gpGlobals->startspot) );
		if ( !FNullEnt(pSpot) )
			goto ReturnSpot;
	}

ReturnSpot:
	if ( FNullEnt( pSpot ) )
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	g_pLastSpawn = pSpot;
	return pSpot->edict();
}

void CBasePlayer::Spawn( void )
{
	pev->classname		= MAKE_STRING("player");
	pev->health			= 100;
	pev->armorvalue		= 0;
	pev->takedamage		= DAMAGE_AIM;
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_WALK;
	pev->max_health		= pev->health;
	pev->flags		   &= FL_PROXY;	// keep proxy flag sey by engine
	pev->flags		   |= FL_CLIENT;
	pev->air_finished	= gpGlobals->time + 12;
	pev->dmg			= 2;				// initial water damage
	pev->effects		= 0;
	pev->deadflag		= DEAD_NO;
	pev->dmg_take		= 0;
	pev->dmg_save		= 0;
	pev->friction		= 1.0;
	pev->gravity		= 1.0;
	m_bitsHUDDamage		= -1;
	m_bitsDamageType	= 0;
	m_afPhysicsFlags	= 0;
	m_fLongJump			= FALSE;// no longjump module. 

	m_fHurted			= FALSE;//no esta herido
	//new
	m_fCrosshairOff		= FALSE;
	m_fCineOn			= FALSE;
	m_fRicardoOn		= FALSE;

	m_fCanUseFlashlight	= TRUE;

	m_bHEVPlayVoice	= TRUE;

	pCar = NULL; //HL: Town, this is the car the player uses

//doesn't work... esta funcion se ejecuta solo una ves por juego :S
	
	SERVER_COMMAND( "hud_draw 1\n" );

	MESSAGE_BEGIN(MSG_ONE, gmsgClcommand, NULL, pev);
		WRITE_STRING("firstperson");
		WRITE_BYTE(1);//say_team creo
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_ONE, gmsgTbutton, NULL, pev );
		WRITE_SHORT( 0 );
	MESSAGE_END();

	//restore volume
	SERVER_COMMAND( "volume 0.8 \n" );//sys
	SERVER_COMMAND( "host_framerate 0\n" );
	SERVER_COMMAND( "firstperson\n" );
	CVAR_SET_FLOAT( "slowmo", 0);

	RicardoToggle(FALSE);

	// clear all particlesystems with this hijacked message
	extern int gmsgParticles;
	MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
		WRITE_SHORT(0);
		WRITE_BYTE(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_SHORT(9999);
		WRITE_STRING("");
	MESSAGE_END();

	ALERT(at_console, "Resetting Particle Engine\n");

	//reset models
	SERVER_COMMAND("cl_showheadcrab 0\n");
	SERVER_COMMAND("cl_showparachute 0\n");
	//reset models

	StartBurning = FALSE;

	//reset the entities in the radar
	MESSAGE_BEGIN( MSG_ONE, gmsgHudRadar, NULL, pev ); // Set the message parameters - what type of msg + where to send 
		WRITE_BYTE( FALSE );
		WRITE_BYTE( 0 ); // total number of targets on the radar 
		WRITE_BYTE( 0 );
		WRITE_BYTE( 0 );
	MESSAGE_END();
	//reset the entities in the radar

	SERVER_COMMAND( "con_color \"255 255 255\" \n" );
	ALERT ( at_console, "con_color is now 255 255 255\n" );
	
	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );//0
	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "hl", "1" );

	pev->fov = m_iFOV				= 0;// init field of view.
	m_iClientFOV		= -1; // make sure fov reset is sent

	m_flNextDecalTime	= 0;// let this player decal as soon as he spawns.

	m_flgeigerDelay = gpGlobals->time + 2.0;	// wait a few seconds until user-defined message registrations
												// are recieved by all clients
				
	f_NextTipTime = gpGlobals->time + 30;//1/2 min

	m_flTimeStepSound	= 0;
	m_iStepLeft = 0;
	m_flFieldOfView		= 0.5;// some monsters use this to determine whether or not the player is looking at them.

	m_bloodColor	= BLOOD_COLOR_RED;
	m_flNextAttack	= UTIL_WeaponTimeBase();
	StartSneaking();

	m_iFlashBattery = 99;
	m_flFlashLightTime = 1; // force first message

// dont let uninitialized value here hurt the player
	m_flFallVelocity = 0;

	g_pGameRules->SetDefaultPlayerTeam( this );
	g_pGameRules->GetPlayerSpawnSpot( this );

	if ( g_pGameRules->IsDeathmatch() )
	{
		 SET_MODEL(ENT(pev), "models/player.mdl");
//		 SERVER_COMMAND( "model sys\n" );//fix, set the model, but set the model...
	}
	else
	{
//		SET_MODEL(ENT(pev), "models/player.mdl");
//		SERVER_COMMAND( "model body\n" );
		SET_MODEL(ENT(pev), "models/player/body/body.mdl");
	}

	
	char *mp_am_mapmusic = (char *)CVAR_GET_STRING( "mp_am_mapmusic" );

	if ( mp_am_mapmusic && mp_am_mapmusic[0] )
	{
		
		if ( CVAR_GET_FLOAT("cl_playmusic") <= 0 )//1
		{
			ALERT ( at_console, "\n*Musica encontrada con exito! \n" );

			char szMusicCommand[256];
			sprintf( szMusicCommand, "playaudio %s\n", mp_am_mapmusic );

			if ( !g_pGameRules->IsMultiplayer() )
			SERVER_COMMAND( szMusicCommand );

			ALERT( at_console, "Reproduciendo musica del mapa...\n" );
		}
		else
		{
			ALERT ( at_console, "ERROR: Inesperado error al reproducir musica de Mapa, cl_playmusic es 1!\n" );
			ClientPrint(pev, HUD_PRINTCONSOLE, "English: cl_playmusic is 1, an automatic playlist is on!\n");
		}
	}
	else
	{
		ALERT ( at_console, "ERROR: Inesperado error al reproducir musica de Mapa\n" );
		ClientPrint(pev, HUD_PRINTCONSOLE, "Spanish: Asegurate de poner un archivo MP3 o OGG con el mismo nombre del mapa\n");
		ClientPrint(pev, HUD_PRINTCONSOLE, "dentro del directorio: -Sound/Fmod/- \n");
		ClientPrint(pev, HUD_PRINTCONSOLE, "\nEnglish: Be sure to place a MP3 or OGG whit the same name of the map\n");
		ClientPrint(pev, HUD_PRINTCONSOLE, "in the folder: -Sound/Fmod/- \n");

	//	SERVER_COMMAND( "stopaudio" );//Si hay musica detenerla...
		//esto causa error en MP
	}
//////***************************************************************************

    g_ulModelIndexPlayer = pev->modelindex;
	pev->sequence		= LookupActivity( ACT_IDLE );

	if ( FBitSet(pev->flags, FL_DUCKING) && !pCar)
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

    pev->view_ofs = VEC_VIEW;
	Precache();
	m_HackedGunPos		= Vector( 0, 32, 0 );

	if ( m_iPlayerSound == SOUNDLIST_EMPTY )
	{
		ALERT ( at_console, "Couldn't alloc player sound slot!\n" );
	}

	m_fNoPlayerSound = FALSE;// normal sound behavior.

	m_pLastItem = NULL;
	m_fInitHUD = TRUE;
	m_iClientHideHUD = -1;  // force this to be recalculated
	m_fWeapon = FALSE;
	m_pClientActiveItem = NULL;
	m_iClientBattery = -1;

//BOOleanos Stuff, movidos a Killed ();
	// reset all ammo values to 0
	for ( int i = 0; i < MAX_AMMO_SLOTS; i++ )
	{
		m_rgAmmo[i] = 0;
		m_rgAmmoLast[i] = 0;  // client ammo values also have to be reset  (the death hud clear messages does on the client side)
	}

	m_lastx = m_lasty = 0;
	
	m_flNextChatTime = gpGlobals->time;
		
	UTIL_EdictScreenFade( edict(), Vector(0,0,0), 0.1, 0, 0, FFADE_IN ); //FiX

	g_pGameRules->PlayerSpawn( this );
}


void CBasePlayer :: Precache( void )
{
	// in the event that the player JUST spawned, and the level node graph
	// was loaded, fix all of the node graph pointers before the game starts.
	
	// !!!BUGBUG - now that we have multiplayer, this needs to be moved!
	
	if ( WorldGraph.m_fGraphPresent && !WorldGraph.m_fGraphPointersSet )
	{
		if ( !WorldGraph.FSetGraphPointers() )
		{
			ALERT ( at_console, "**Graph pointers were not set!\n");
		}
		else
		{
			ALERT ( at_console, "**Graph Pointers Set!\n" );
		} 
	}

	// SOUNDS / MODELS ARE PRECACHED in ClientPrecache() (game specific)
	// because they need to precache before any clients have connected

	// init geiger counter vars during spawn and each time
	// we cross a level transition

	m_flgeigerRange = 1000;
	m_igeigerRangePrev = 1000;

	m_bitsDamageType = 0;
	m_bitsHUDDamage = -1;

	m_iClientBattery = -1;

	m_iTrain = TRAIN_NEW;

	pCar = NULL; //HL: Town, this is the car the player uses

	// Make sure any necessary user messages have been registered
	LinkUserMessages();

	m_iUpdateTime = 5;  // won't update for 1/2 a second

	if ( gInitHUD )
		m_fInitHUD = TRUE;
		
	Rain_needsUpdate = 1;
}


int CBasePlayer::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;

	return save.WriteFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );
}


//
// Marks everything as new so the player will resend this to the hud.
//
void CBasePlayer::RenewItems(void)
{

}


int CBasePlayer::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;

	int status = restore.ReadFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );

	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;
	// landmark isn't present.
	if ( !pSaveData->fUseLandmark )
	{
		ALERT( at_console, "No Landmark:%s\n", pSaveData->szLandmarkName );

		// default to normal spawn
		edict_t* pentSpawnSpot = EntSelectSpawnPoint( this );
		pev->origin = VARS(pentSpawnSpot)->origin + Vector(0,0,1);
		pev->angles = VARS(pentSpawnSpot)->angles;
	}
	pev->v_angle.z = 0;	// Clear out roll
	pev->angles = pev->v_angle;

	pev->fixangle = TRUE;           // turn this way immediately

// Copied from spawn() for now
	m_bloodColor	= BLOOD_COLOR_RED;

    g_ulModelIndexPlayer = pev->modelindex;

	if ( FBitSet(pev->flags, FL_DUCKING) && !pCar)
	{
		// Use the crouch HACK
		//FixPlayerCrouchStuck( edict() );
		// Don't need to do this with new player prediction code.
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	}
	else
	{
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
	}

	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "hl", "1" );

	if ( m_fLongJump )
	{
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "1" );
	}
	else
	{
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );//0
	}

	RenewItems();

#if defined( CLIENT_WEAPONS )
	// HACK:	This variable is saved/restored in CBaseMonster as a time variable, but we're using it
	//			as just a counter.  Ideally, this needs its own variable that's saved as a plain float.
	//			Barring that, we clear it out here instead of using the incorrect restored time value.
	m_flNextAttack = UTIL_WeaponTimeBase();
#endif

	return status;
}



void CBasePlayer::SelectNextItem( int iItem )
{
	CBasePlayerItem *pItem;

	pItem = m_rgpPlayerItems[ iItem ];
	
	if (!pItem)
		return;

	if (pItem == m_pActiveItem)
	{
		// select the next one in the chain
		pItem = m_pActiveItem->m_pNext; 
		if (! pItem)
		{
			return;
		}

		CBasePlayerItem *pLast;
		pLast = pItem;
		while (pLast->m_pNext)
			pLast = pLast->m_pNext;

		// relink chain
		pLast->m_pNext = m_pActiveItem;
		m_pActiveItem->m_pNext = NULL;
		m_rgpPlayerItems[ iItem ] = pItem;
	}

	ResetAutoaim( );

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
	{
		m_pActiveItem->Holster( );
	}
	
	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy( );
		m_pActiveItem->UpdateItemInfo( );
	}
}

void CBasePlayer::SelectItem(const char *pstr)
{
	if (!pstr)
		return;

	CBasePlayerItem *pItem = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			pItem = m_rgpPlayerItems[i];
	
			while (pItem)
			{
				if (FClassnameIs(pItem->pev, pstr))
					break;
				pItem = pItem->m_pNext;
			}
		}

		if (pItem)
			break;
	}

	if (!pItem)
		return;

	
	if (pItem == m_pActiveItem)
		return;

	ResetAutoaim( );

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
		m_pActiveItem->Holster( );
	
	m_pLastItem = m_pActiveItem;
	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy( );
		m_pActiveItem->UpdateItemInfo( );
	}
}


void CBasePlayer::SelectLastItem(void)
{
	if (!m_pLastItem)
	{
		return;
	}

	if ( m_pActiveItem && !m_pActiveItem->CanHolster() )
	{
		return;
	}

	ResetAutoaim( );

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
		m_pActiveItem->Holster( );
	
	CBasePlayerItem *pTemp = m_pActiveItem;
	m_pActiveItem = m_pLastItem;
	m_pLastItem = pTemp;
	m_pActiveItem->Deploy( );
	m_pActiveItem->UpdateItemInfo( );
}

//==============================================
// HasWeapons - do I have any weapons at all?
//==============================================
BOOL CBasePlayer::HasWeapons( void )
{
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CBasePlayer::SelectPrevItem( int iItem )
{
}


const char *CBasePlayer::TeamID( void )
{
	if ( pev == NULL )		// Not fully connected yet
		return "";

	// return their team name
	return m_szTeamName;
}


//==============================================
// !!!UNDONE:ultra temporary SprayCan entity to apply
// decal frame at a time. For PreAlpha CD
//==============================================
class CSprayCan : public CBaseEntity
{
public:
	void	Spawn ( entvars_t *pevOwner );
	void	Think( void );

	virtual int	ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};

void CSprayCan::Spawn ( entvars_t *pevOwner )
{
	pev->origin = pevOwner->origin + Vector ( 0 , 0 , 32 );
	pev->angles = pevOwner->v_angle;
	pev->owner = ENT(pevOwner);
	pev->frame = 0;

	pev->nextthink = gpGlobals->time + 0.1;
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/sprayer.wav", 1, ATTN_NORM);
}

void CSprayCan::Think( void )
{
	TraceResult	tr;	
	int playernum;
	int nFrames;
	CBasePlayer *pPlayer;
	
	pPlayer = (CBasePlayer *)GET_PRIVATE(pev->owner);

	if (pPlayer)
		nFrames = pPlayer->GetCustomDecalFrames();
	else
		nFrames = -1;

	playernum = ENTINDEX(pev->owner);
	
//	 ALERT(at_console, "Spray by player %i, %i of %i\n", playernum, (int)(pev->frame + 1), nFrames);

	UTIL_MakeVectors(pev->angles);
	UTIL_TraceLine ( pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, & tr);

	// No customization present.
	if (nFrames == -1)
	{
		UTIL_DecalTrace( &tr, DECAL_LAMBDA6 );
		UTIL_Remove( this );
	}
	else
	{
		UTIL_PlayerDecalTrace( &tr, playernum, pev->frame, TRUE );
		// Just painted last custom frame.
		if ( pev->frame++ >= (nFrames - 1))
			UTIL_Remove( this );
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

class	CBloodSplat : public CBaseEntity
{
public:
	void	Spawn ( entvars_t *pevOwner );
	void	Spray ( void );
};

void CBloodSplat::Spawn ( entvars_t *pevOwner )
{
	pev->origin = pevOwner->origin + Vector ( 0 , 0 , 32 );
	pev->angles = pevOwner->v_angle;
	pev->owner = ENT(pevOwner);

	SetThink ( Spray );
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBloodSplat::Spray ( void )
{
	TraceResult	tr;	
	
	if ( g_Language != LANGUAGE_GERMAN )
	{
		UTIL_MakeVectors(pev->angles);
		UTIL_TraceLine ( pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, & tr);

		UTIL_BloodDecalTrace( &tr, BLOOD_COLOR_RED );
	}
	SetThink ( SUB_Remove );
	pev->nextthink = gpGlobals->time + 0.1;
}

//==============================================



void CBasePlayer::GiveNamedItem( const char *pszName )
{
	edict_t	*pent;

	int istr = MAKE_STRING(pszName);

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in GiveNamedItem!\n" );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );
}


/*
CBaseEntity *FindEntityForward( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}
*/

BOOL CBasePlayer :: FlashlightIsOn( void )
{
	return FBitSet(pev->effects, EF_DIMLIGHT);
//	return FBitSet(pev->effects, 0);
}


void CBasePlayer :: FlashlightTurnOn( void )
{/*
	if ( !g_pGameRules->FAllowFlashlight() )
	{
		return;
	}
	*/

	if(!m_fCanUseFlashlight) //si no tenemos, nada
	return; //...return

	if ( (pev->weapons & (1<<WEAPON_SUIT)) )
	{
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_ON, 1.0, ATTN_NORM, 0, PITCH_NORM );

		SetBits(pev->effects, EF_DIMLIGHT);
//		SetBits(pev->effects, 0);

		MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
		WRITE_BYTE(1);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();

		b_FlashlightIsOn = TRUE;
		
		RicardoToggle(TRUE);

		m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
	}
}


void CBasePlayer :: FlashlightTurnOff( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_OFF, 1.0, ATTN_NORM, 0, PITCH_NORM );

    ClearBits(pev->effects, EF_DIMLIGHT);
//    ClearBits(pev->effects, 0);

	MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
	WRITE_BYTE(0);
	WRITE_BYTE(m_iFlashBattery);
	MESSAGE_END();
		
	b_FlashlightIsOn = FALSE;
	RicardoToggle(FALSE);

	m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
}

/*
===============
ForceClientDllUpdate

When recording a demo, we need to have the server tell us the entire client state
so that the client side .dll can behave correctly.
Reset stuff so that the state is transmitted.
===============
*/
void CBasePlayer :: ForceClientDllUpdate( void )
{
	m_iClientHealth  = -1;
	m_iClientBattery = -1;
	m_iTrain |= TRAIN_NEW;  // Force new train message.
	m_fWeapon = FALSE;          // Force weapon send
	m_fKnownItem = FALSE;    // Force weaponinit messages.
	m_fInitHUD = TRUE;		// Force HUD gmsgResetHUD message

	// Now force all the necessary messages
	//  to be sent.
	UpdateClientData();
}

/*
============
ImpulseCommands
============
*/
extern float g_flWeaponCheat;

void CBasePlayer::ImpulseCommands( )
{
	TraceResult	tr;// UNDONE: kill me! This is temporary for PreAlpha CDs

	// Handle use events
	PlayerUse();
			
	BOOL bOn = FALSE;

	int iImpulse = (int)pev->impulse;
	switch (iImpulse)
	{
	case 99:
		{	
		if (bOn)
		{
		//	MESSAGE_BEGIN( MSG_ONE, gmsgLogo, NULL, pev );
		//		WRITE_BYTE(0);
		//	MESSAGE_END();
						
			bOn = FALSE;
		}
		else
		{
		//	MESSAGE_BEGIN( MSG_ONE, gmsgLogo, NULL, pev );
		//		WRITE_BYTE(1);
		//	MESSAGE_END();
					
			bOn = TRUE;
		}
/*
		if (!gmsgLogo)
		{
			iOn = 1;
		//moved up
			//gmsgLogo = REG_USER_MSG("Logo", 1);
		} 
		else 
		{
			iOn = 0;
		}
		
		ASSERT( gmsgLogo > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgLogo, NULL, pev );
			WRITE_BYTE(iOn);
		MESSAGE_END();

		if(!iOn)
			gmsgLogo = 0;
			*/
		break;
		}
	case 100:
        // temporary flashlight for level designers
        if ( FlashlightIsOn() )
		{
			FlashlightTurnOff();
		}
        else 
		{
			FlashlightTurnOn();
		}
		break;

	case	201:// paint decal
			
		if(m_fStealth) //si esta en stealth, no decals
		{
			ClientPrint(pev, HUD_PRINTCENTER, "#StealthCantDecal");
			break;
		}

		if ( gpGlobals->time < m_flNextDecalTime )
		{
			// too early!
			break;
		}

		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine ( pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), & tr);

		if ( tr.flFraction != 1.0 )
		{// line hit something, so paint a decal
			m_flNextDecalTime = gpGlobals->time + decalfrequency.value;
			CSprayCan *pCan = GetClassPtr((CSprayCan *)NULL);
			pCan->Spawn( pev );
		}

		break;

	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands( iImpulse );
		break;
	}
	
	pev->impulse = 0;
}

//=========================================================
//=========================================================
void CBasePlayer::CheatImpulseCommands( int iImpulse )
{
	if ( g_flWeaponCheat == 0.0 )
	{
		return;
	}

	CBaseEntity *pEntity;
	TraceResult tr;

	switch ( iImpulse )
	{
	case 76: 
		{
			if (!giPrecacheGrunt)
			{
				giPrecacheGrunt = 1;
				ALERT(at_console, "You must now restart to use Grunt-o-matic.\n");
			}
			else
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				Create("monster_human_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			}
			break;
		}
	case 77: 
		{

			break;
		}
	case 101:
		{
			switch ( RANDOM_LONG(1,4) ) //al azar
			{
				case 1: ALERT(at_console, "Deberia darte verguenza...\n"); break;
				case 2: ALERT(at_console, "No puedes con lo que tienes?\n"); break;
				case 3: ALERT(at_console, "Demasiado para ti? Jajaja...\n"); break;			
				case 4: ALERT(at_console, "Pidele ayuda a James Bond!\n"); break;			
			}
		break;
		}

	case 111:
		{
			gEvilImpulse101 = TRUE;
			GiveNamedItem( "item_suit" );
			GiveNamedItem( "item_battery" );
			GiveNamedItem( "weapon_knife" );

			GiveNamedItem( "weapon_357" );
			GiveNamedItem( "ammo_357" );

			GiveNamedItem( "weapon_clustergrenade" );

			GiveNamedItem( "weapon_heal" );
			GiveNamedItem( "ammo_heal" );

			GiveNamedItem( "item_nvg" );
			ALERT(at_console, "Anaconda, NVG, Heal, Cluster and Stuff given!!\n");
		}
		break;

			
	case 122:
		{
			gEvilImpulse101 = TRUE;
			GiveNamedItem( "item_suit" );
			GiveNamedItem( "item_battery" );
			gEvilImpulse101 = FALSE;
		}
		break;

	case 007:
		gEvilImpulse101 = TRUE;
		GiveNamedItem( "item_suit" );
		GiveNamedItem( "item_battery" );
		GiveNamedItem( "weapon_crowbar" );
		GiveNamedItem( "weapon_9mmhandgun" );
		GiveNamedItem( "ammo_9mmclip" );
		GiveNamedItem( "weapon_shotgun" );
		GiveNamedItem( "ammo_buckshot" );
		GiveNamedItem( "weapon_9mmAR" );
		GiveNamedItem( "ammo_9mmAR" );
		GiveNamedItem( "ammo_ARgrenades" );
		GiveNamedItem( "weapon_handgrenade" );
		GiveNamedItem( "weapon_tripmine" );

		GiveNamedItem( "weapon_357" );
		GiveNamedItem( "ammo_357" );
		GiveNamedItem( "weapon_crossbow" );
		GiveNamedItem( "ammo_crossbow" );
		GiveNamedItem( "weapon_egon" );
		GiveNamedItem( "weapon_gauss" );
		GiveNamedItem( "ammo_gaussclip" );
		GiveNamedItem( "weapon_rpg" );
		GiveNamedItem( "ammo_rpgclip" );
		GiveNamedItem( "weapon_satchel" );
		GiveNamedItem( "weapon_snark" );
		GiveNamedItem( "weapon_hornetgun" );

		GiveNamedItem( "weapon_eagle" );
		GiveNamedItem( "weapon_m40a1" );
		GiveNamedItem( "weapon_saw" );
		GiveNamedItem( "weapon_m16" );

		GiveNamedItem( "weapon_medkit" );

		GiveNamedItem( "ammo_sniper" );

		// mensaje de cheto
		ALERT(at_console, "James Bond Rulezz!\n");
		// cheater voice
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fvox/cheater.wav", 1, ATTN_NORM);

		switch (RANDOM_LONG(0,2)) //ejecuta los siguientes sonidos al azar
		{
			case 0: 
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "fvox/cheater1.wav", 1, ATTN_NORM);
				ALERT(at_console, "YOU SUCKS!\n");
				UTIL_CenterPrintAll( "YOU SUCKS!" );
			break;

			case 1: 
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "fvox/cheater2.wav", 1, ATTN_NORM);
				ALERT(at_console, "LUCKY SON OF A BITCH!\n");
				UTIL_CenterPrintAll( "LUCKY SON OF A BITCH!" );
			break;

			case 2: 
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "fvox/cheater3.wav", 1, ATTN_NORM);
				ALERT(at_console, "DAMN YOU'RE UGLY!\n");
				UTIL_CenterPrintAll( "DAMN YOU'RE UGLY!" );
			break;
		}
		gEvilImpulse101 = FALSE;
		break;

	case 102:
		// Gibbage!!!
		CGib::SpawnRandomGibs( pev, 1, 1 );
		break;

	case 103:
		// What the hell are you doing?
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			CBaseMonster *pMonster = pEntity->MyMonsterPointer();
			if ( pMonster )
				pMonster->ReportAIState();
		}
		break;

	case 104:
		// Dump all of the global state varaibles (and global entity names)
		gGlobalState.DumpGlobals();
		break;

	case	105:// player makes no sound for monsters to hear.
		{
			if ( m_fNoPlayerSound )
			{
				ALERT ( at_console, "Player is audible\n" );
				m_fNoPlayerSound = FALSE;
			}
			else
			{
				ALERT ( at_console, "Player is silent\n" );
				m_fNoPlayerSound = TRUE;
			}
			break;
		}

	case 106:
		// Give me the classname and targetname of this entity.
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			ALERT ( at_console, "Classname: %s", STRING( pEntity->pev->classname ) );
			
			if ( !FStringNull ( pEntity->pev->targetname ) )
			{
				ALERT ( at_console, " - Targetname: %s\n", STRING( pEntity->pev->targetname ) );
			}
			else
			{
				ALERT ( at_console, " - TargetName: No Targetname\n" );
			}

			ALERT ( at_console, "Model: %s\n", STRING( pEntity->pev->model ) );
			if ( pEntity->pev->globalname )
				ALERT ( at_console, "Globalname: %s\n", STRING( pEntity->pev->globalname ) );
		}
		break;

	case 107:
		{
			TraceResult tr;

			edict_t		*pWorld = g_engfuncs.pfnPEntityOfEntIndex( 0 );

			Vector start = pev->origin + pev->view_ofs;
			Vector end = start + gpGlobals->v_forward * 1024;
			UTIL_TraceLine( start, end, ignore_monsters, edict(), &tr );
			if ( tr.pHit )
				pWorld = tr.pHit;
			const char *pTextureName = TRACE_TEXTURE( pWorld, start, end );
			if ( pTextureName )
				ALERT( at_console, "Texture: %s\n", pTextureName );
		}
		break;
		
	case 109:
		// What the hell are you doing?
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{

		}
		break;

	case	195:// show shortest paths for entire level to nearest node
		{
			Create("node_viewer_fly", pev->origin, pev->angles);
		}
		break;
	case	196:// show shortest paths for entire level to nearest node
		{
			Create("node_viewer_large", pev->origin, pev->angles);
		}
		break;
	case	197:// show shortest paths for entire level to nearest node
		{
			Create("node_viewer_human", pev->origin, pev->angles);
		}
		break;
	case	199:// show nearest node and all connections
		{
			ALERT ( at_console, "%d\n", WorldGraph.FindNearestNode ( pev->origin, bits_NODE_GROUP_REALM ) );
			WorldGraph.ShowNodeConnections ( WorldGraph.FindNearestNode ( pev->origin, bits_NODE_GROUP_REALM ) );
		}
		break;
	case	202:// Random blood splatter
		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine ( pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), & tr);

		if ( tr.flFraction != 1.0 )
		{// line hit something, so paint a decal
			CBloodSplat *pBlood = GetClassPtr((CBloodSplat *)NULL);
			pBlood->Spawn( pev );
		}
		break;
	case	203:// remove creature.
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			if ( pEntity->pev->takedamage )
				pEntity->SetThink(SUB_Remove);
		}
		break;
	}

}

//
// Add a weapon to the player (Item == Weapon == Selectable Object)
//
int CBasePlayer::AddPlayerItem( CBasePlayerItem *pItem )
{
	CBasePlayerItem *pInsert;
	
	pInsert = m_rgpPlayerItems[pItem->iItemSlot()];

	while (pInsert)
	{
		if (FClassnameIs( pInsert->pev, STRING( pItem->pev->classname) ))
		{
			if (pItem->AddDuplicate( pInsert ))
			{
				g_pGameRules->PlayerGotWeapon ( this, pItem );
				pItem->CheckRespawn();

				// ugly hack to update clip w/o an update clip message
				pInsert->UpdateItemInfo( );
				if (m_pActiveItem)
					m_pActiveItem->UpdateItemInfo( );

				pItem->Kill( );
			}
			else if (gEvilImpulse101)
			{
				// FIXME: remove anyway for deathmatch testing
				pItem->Kill( );
			}
			return FALSE;
		}
		pInsert = pInsert->m_pNext;
	}


	if (pItem->AddToPlayer( this ))
	{
		g_pGameRules->PlayerGotWeapon ( this, pItem );
		pItem->CheckRespawn();

		pItem->m_pNext = m_rgpPlayerItems[pItem->iItemSlot()];
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem;

		// should we switch to this item?
		if ( g_pGameRules->FShouldSwitchWeapon( this, pItem ) )
		{
			SwitchWeapon( pItem );
		}

		return TRUE;
	}
	else if (gEvilImpulse101)
	{
		// FIXME: remove anyway for deathmatch testing
		pItem->Kill( );
	}
	return FALSE;
}



int CBasePlayer::RemovePlayerItem( CBasePlayerItem *pItem )
{
	if (m_pActiveItem == pItem)
	{
		ResetAutoaim( );
		pItem->Holster( );
		pItem->pev->nextthink = 0;// crowbar may be trying to swing again, etc.
		pItem->SetThink( NULL );
		m_pActiveItem = NULL;
		pev->viewmodel = 0;
		pev->weaponmodel = 0;
	}
	else if ( m_pLastItem == pItem )
		m_pLastItem = NULL;

	CBasePlayerItem *pPrev = m_rgpPlayerItems[pItem->iItemSlot()];

	if (pPrev == pItem)
	{
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem->m_pNext;
		return TRUE;
	}
	else
	{
		while (pPrev && pPrev->m_pNext != pItem)
		{
			pPrev = pPrev->m_pNext;
		}
		if (pPrev)
		{
			pPrev->m_pNext = pItem->m_pNext;
			return TRUE;
		}
	}
	return FALSE;
}


//
// Returns the unique ID for the ammo, or -1 if error
//
int CBasePlayer :: GiveAmmo( int iCount, char *szName, int iMax )
{
	if ( !szName )
	{
		// no ammo.
		return -1;
	}

	if ( !g_pGameRules->CanHaveAmmo( this, szName, iMax ) )
	{
		// game rules say I can't have any more of this ammo type.
		return -1;
	}

	int i = 0;

	i = GetAmmoIndex( szName );

	if ( i < 0 || i >= MAX_AMMO_SLOTS )
		return -1;

	int iAdd = min( iCount, iMax - m_rgAmmo[i] );
	if ( iAdd < 1 )
		return i;

	m_rgAmmo[ i ] += iAdd;


	if ( gmsgAmmoPickup )  // make sure the ammo messages have been linked first
	{
		// Send the message that ammo has been picked up
		MESSAGE_BEGIN( MSG_ONE, gmsgAmmoPickup, NULL, pev );
			WRITE_BYTE( GetAmmoIndex(szName) );		// ammo ID
			WRITE_BYTE( iAdd );		// amount
		MESSAGE_END();
	}

	TabulateAmmo();

	return i;
}


/*
============
ItemPreFrame

Called every frame by the player PreThink
============
*/
void CBasePlayer::ItemPreFrame()
{
#if defined( CLIENT_WEAPONS )
    if ( m_flNextAttack > 0 )
#else
    if ( gpGlobals->time < m_flNextAttack )
#endif
	{
		return;
	}

	if (!m_pActiveItem)
		return;

	m_pActiveItem->ItemPreFrame( );
}


/*
============
ItemPostFrame

Called every frame by the player PostThink
============
*/
void CBasePlayer::ItemPostFrame()
{
	static int fInSelect = FALSE;

	// check if the player is using a tank
	if ( m_pTank != NULL )
		return;

#if defined( CLIENT_WEAPONS )
    if ( m_flNextAttack > 0 )
#else
    if ( gpGlobals->time < m_flNextAttack )
#endif
	{
		return;
	}

	ImpulseCommands();

	if (!m_pActiveItem)
		return;

	m_pActiveItem->ItemPostFrame( );
}

int CBasePlayer::AmmoInventory( int iAmmoIndex )
{
	if (iAmmoIndex == -1)
	{
		return -1;
	}

	return m_rgAmmo[ iAmmoIndex ];
}

int CBasePlayer::GetAmmoIndex(const char *psz)
{
	int i;

	if (!psz)
		return -1;

	for (i = 1; i < MAX_AMMO_SLOTS; i++)
	{
		if ( !CBasePlayerItem::AmmoInfoArray[i].pszName )
			continue;

		if (stricmp( psz, CBasePlayerItem::AmmoInfoArray[i].pszName ) == 0)
			return i;
	}

	return -1;
}

// Called from UpdateClientData
// makes sure the client has all the necessary ammo info,  if values have changed
void CBasePlayer::SendAmmoUpdate(void)
{
	for (int i=0; i < MAX_AMMO_SLOTS;i++)
	{
		if (m_rgAmmo[i] != m_rgAmmoLast[i])
		{
			m_rgAmmoLast[i] = m_rgAmmo[i];

			ASSERT( m_rgAmmo[i] >= 0 );
			ASSERT( m_rgAmmo[i] < 255 );

			// send "Ammo" update message
			MESSAGE_BEGIN( MSG_ONE, gmsgAmmoX, NULL, pev );
				WRITE_BYTE( i );
				WRITE_BYTE( max( min( m_rgAmmo[i], 254 ), 0 ) );  // clamp the value to one byte
			MESSAGE_END();
		}
	}
}

/*
=========================================================
	UpdateClientData

resends any changed player HUD info to the client.
Called every frame by PlayerPreThink
Also called at start of demo recording and playback by
ForceClientDllUpdate to ensure the demo gets messages
reflecting all of the HUD state info.
=========================================================
*/
void CBasePlayer :: UpdateClientData( void )
{
	if (m_fInitHUD)
	{
		m_fInitHUD = FALSE;
		gInitHUD = FALSE;

		MESSAGE_BEGIN( MSG_ONE, gmsgResetHUD, NULL, pev );
			WRITE_BYTE( 0 );
		MESSAGE_END();

		if ( !m_fGameHUDInitialized )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgInitHUD, NULL, pev );
			MESSAGE_END();

			g_pGameRules->InitHUD( this );
			m_fGameHUDInitialized = TRUE;
			if ( g_pGameRules->IsMultiplayer() )
			{
				FireTargets( "game_playerjoin", this, this, USE_TOGGLE, 0 );
			}
		}

		FireTargets( "game_playerspawn", this, this, USE_TOGGLE, 0 );

		InitStatusBar();
	}

	if ( m_iHideHUD != m_iClientHideHUD )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgHideWeapon, NULL, pev );
			WRITE_BYTE( m_iHideHUD );
		MESSAGE_END();

		m_iClientHideHUD = m_iHideHUD;
	}

	if ( m_iFOV != m_iClientFOV )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
			WRITE_BYTE( m_iFOV );
		MESSAGE_END();

		// cache FOV change at end of function, so weapon updates can see that FOV has changed
	}

	// HACKHACK -- send the message to display the game title
	if (gDisplayTitle)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgShowGameTitle, NULL, pev );
		WRITE_BYTE( 0 );
		MESSAGE_END();
		gDisplayTitle = 0;
	}

	if (pev->health != m_iClientHealth)
	{
		int iHealth = max( pev->health, 0 );  // make sure that no negative health values are sent

		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgHealth, NULL, pev );
			WRITE_BYTE( iHealth );
		MESSAGE_END();

		m_iClientHealth = pev->health;
	}


	if (pev->armorvalue != m_iClientBattery)
	{
		m_iClientBattery = pev->armorvalue;

		ASSERT( gmsgBattery > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgBattery, NULL, pev );
			WRITE_SHORT( (int)pev->armorvalue);
		MESSAGE_END();
	}

	if (pev->dmg_take || pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType)
	{
		// Comes from inside me if not set
		Vector damageOrigin = pev->origin;
		// send "damage" message
		// causes screen to flash, and pain compass to show direction of damage
		edict_t *other = pev->dmg_inflictor;
		if ( other )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(other);
			if ( pEntity )
				damageOrigin = pEntity->Center();
		}

		// only send down damage type that have hud art
		int visibleDamageBits = m_bitsDamageType & DMG_SHOWNHUD;

		MESSAGE_BEGIN( MSG_ONE, gmsgDamage, NULL, pev );
			WRITE_BYTE( pev->dmg_save );
			WRITE_BYTE( pev->dmg_take );
			WRITE_LONG( visibleDamageBits );
			WRITE_COORD( damageOrigin.x );
			WRITE_COORD( damageOrigin.y );
			WRITE_COORD( damageOrigin.z );
		MESSAGE_END();
	
		pev->dmg_take = 0;
		pev->dmg_save = 0;
		m_bitsHUDDamage = m_bitsDamageType;
		
		// Clear off non-time-based damage indicators
		m_bitsDamageType &= DMG_TIMEBASED;
	}
	// calculate and update rain fading
	if (Rain_endFade > 0)
	{
		if (gpGlobals->time < Rain_endFade)
		{ // we're in fading process
			if (Rain_nextFadeUpdate <= gpGlobals->time)
			{
				int secondsLeft = Rain_endFade - gpGlobals->time + 1;

				Rain_dripsPerSecond += (Rain_ideal_dripsPerSecond - Rain_dripsPerSecond) / secondsLeft;
				Rain_windX += (Rain_ideal_windX - Rain_windX) / (float)secondsLeft;
				Rain_windY += (Rain_ideal_windY - Rain_windY) / (float)secondsLeft;
				Rain_randX += (Rain_ideal_randX - Rain_randX) / (float)secondsLeft;
				Rain_randY += (Rain_ideal_randY - Rain_randY) / (float)secondsLeft;

				Rain_nextFadeUpdate = gpGlobals->time + 1; // update once per second
				Rain_needsUpdate = 1;

				ALERT(at_aiconsole, "Rain fading: curdrips: %i, idealdrips %i\n", Rain_dripsPerSecond, Rain_ideal_dripsPerSecond);
			}
		}
		else
		{ // finish fading process
			Rain_nextFadeUpdate = 0;
			Rain_endFade = 0;

			Rain_dripsPerSecond = Rain_ideal_dripsPerSecond;
			Rain_windX = Rain_ideal_windX;
			Rain_windY = Rain_ideal_windY;
			Rain_randX = Rain_ideal_randX;
			Rain_randY = Rain_ideal_randY;
			Rain_needsUpdate = 1;

			ALERT(at_aiconsole, "Rain fading finished at %i drips\n", Rain_dripsPerSecond);
		}		
	}

	// send rain message
	if (Rain_needsUpdate)
	{
	//search for rain_settings entity
		edict_t *pFind; 
		pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "rain_settings" );
		if (!FNullEnt( pFind ))
		{
		// rain allowed on this map
			CBaseEntity *pEnt = CBaseEntity::Instance( pFind );
			CRainSettings *pRainSettings = (CRainSettings *)pEnt;

			float raindistance = pRainSettings->Rain_Distance;
			float rainheight = pRainSettings->pev->origin[2];
			int rainmode = pRainSettings->Rain_Mode;

			// search for constant rain_modifies
			pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "rain_modify" );
			while ( !FNullEnt( pFind ) )
			{
				if (pFind->v.spawnflags & 1)
				{
					// copy settings to player's data and clear fading
					CBaseEntity *pEnt = CBaseEntity::Instance( pFind );
					CRainModify *pRainModify = (CRainModify *)pEnt;

					Rain_dripsPerSecond = pRainModify->Rain_Drips;
					Rain_windX = pRainModify->Rain_windX;
					Rain_windY = pRainModify->Rain_windY;
					Rain_randX = pRainModify->Rain_randX;
					Rain_randY = pRainModify->Rain_randY;

					Rain_endFade = 0;
					break;
				}
				pFind = FIND_ENTITY_BY_CLASSNAME( pFind, "rain_modify" );
			}

			MESSAGE_BEGIN(MSG_ONE, gmsgRainData, NULL, pev);
				WRITE_SHORT(Rain_dripsPerSecond);
				WRITE_COORD(raindistance);
				WRITE_COORD(Rain_windX);
				WRITE_COORD(Rain_windY);
				WRITE_COORD(Rain_randX);
				WRITE_COORD(Rain_randY);
				WRITE_SHORT(rainmode);
				WRITE_COORD(rainheight);
			MESSAGE_END();

			if (Rain_dripsPerSecond)
				ALERT(at_aiconsole, "Sending enabling rain message\n");
			else
				ALERT(at_aiconsole, "Sending disabling rain message\n");
		}
		else
		{ // no rain on this map
			Rain_dripsPerSecond = 0;
			Rain_windX = 0;
			Rain_windY = 0;
			Rain_randX = 0;
			Rain_randY = 0;
			Rain_ideal_dripsPerSecond = 0;
			Rain_ideal_windX = 0;
			Rain_ideal_windY = 0;
			Rain_ideal_randX = 0;
			Rain_ideal_randY = 0;
			Rain_endFade = 0;
			Rain_nextFadeUpdate = 0;

			ALERT(at_aiconsole, "Clearing rain data\n");
		}

		Rain_needsUpdate = 0;
	}
	// Update Flashlight
	if ((m_flFlashLightTime) && (m_flFlashLightTime <= gpGlobals->time))
	{
		if (FlashlightIsOn())
		{
			if (m_iFlashBattery)
			{
				m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
				m_iFlashBattery--;
				
				if (!m_iFlashBattery)
					FlashlightTurnOff();
			}
		}
		else
		{
			if (m_iFlashBattery < 100)
			{
				m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
				m_iFlashBattery++;
			}
			else
				m_flFlashLightTime = 0;
		}

		MESSAGE_BEGIN( MSG_ONE, gmsgFlashBattery, NULL, pev );
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();
	}


	if (m_iTrain & TRAIN_NEW)
	{
		ASSERT( gmsgTrain > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgTrain, NULL, pev );
			WRITE_BYTE(m_iTrain & 0xF);
		MESSAGE_END();

		m_iTrain &= ~TRAIN_NEW;
	}

	//
	// New Weapon?
	//
	if (!m_fKnownItem)
	{
		m_fKnownItem = TRUE;

	// WeaponInit Message
	// byte  = # of weapons
	//
	// for each weapon:
	// byte		name str length (not including null)
	// bytes... name
	// byte		Ammo Type
	// byte		Ammo2 Type
	// byte		bucket
	// byte		bucket pos
	// byte		flags	
	// ????		Icons
		
		// Send ALL the weapon info now
		int i;

		for (i = 0; i < MAX_WEAPONS; i++)
		{
			ItemInfo& II = CBasePlayerItem::ItemInfoArray[i];

			if ( !II.iId )
				continue;

			const char *pszName;
			if (!II.pszName)
				pszName = "Empty";
			else
				pszName = II.pszName;

			MESSAGE_BEGIN( MSG_ONE, gmsgWeaponList, NULL, pev );  
				WRITE_STRING(pszName);			// string	weapon name
				WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));	// byte		Ammo Type
				WRITE_BYTE(II.iMaxAmmo1);				// byte     Max Ammo 1
				WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));	// byte		Ammo2 Type
				WRITE_BYTE(II.iMaxAmmo2);				// byte     Max Ammo 2
				WRITE_BYTE(II.iSlot);					// byte		bucket
				WRITE_BYTE(II.iPosition);				// byte		bucket pos
				WRITE_BYTE(II.iId);						// byte		id (bit index into pev->weapons)
				WRITE_BYTE(II.iFlags);					// byte		Flags
			MESSAGE_END();
		}
	}


	SendAmmoUpdate();

	// Update all the items
	for ( int i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		if ( m_rgpPlayerItems[i] )  // each item updates it's successors
			m_rgpPlayerItems[i]->UpdateClientData( this );
	}

	// Cache and client weapon change
	m_pClientActiveItem = m_pActiveItem;
	m_iClientFOV = m_iFOV;

	// Update Status Bar
	if ( m_flNextSBarUpdateTime < gpGlobals->time )
	{
		UpdateStatusBar();
		m_flNextSBarUpdateTime = gpGlobals->time + 0.2;
	}
}


//=========================================================
// FBecomeProne - Overridden for the player to set the proper
// physics flags when a barnacle grabs player.
//=========================================================
BOOL CBasePlayer :: FBecomeProne ( void )
{
	m_afPhysicsFlags |= PFLAG_ONBARNACLE;
	return TRUE;
}

//=========================================================
// BarnacleVictimBitten - bad name for a function that is called
// by Barnacle victims when the barnacle pulls their head
// into its mouth. For the player, just die.
//=========================================================
void CBasePlayer :: BarnacleVictimBitten ( entvars_t *pevBarnacle )
{
	TakeDamage ( pevBarnacle, pevBarnacle, pev->health + pev->armorvalue, DMG_SLASH | DMG_ALWAYSGIB );
}

//=========================================================
// BarnacleVictimReleased - overridden for player who has
// physics flags concerns. 
//=========================================================
void CBasePlayer :: BarnacleVictimReleased ( void )
{
	m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
}


//=========================================================
// Illumination 
// return player light level plus virtual muzzle flash
//=========================================================
int CBasePlayer :: Illumination( void )
{
	int iIllum = CBaseEntity::Illumination( );

	iIllum += m_iWeaponFlash;
	if (iIllum > 255)
		return 255;
	return iIllum;
}


void CBasePlayer :: EnableControl(BOOL fControl)
{
	if (!fControl)
		pev->flags |= FL_FROZEN;
	else
		pev->flags &= ~FL_FROZEN;

}


#define DOT_1DEGREE   0.9998476951564
#define DOT_2DEGREE   0.9993908270191
#define DOT_3DEGREE   0.9986295347546
#define DOT_4DEGREE   0.9975640502598
#define DOT_5DEGREE   0.9961946980917
#define DOT_6DEGREE   0.9945218953683
#define DOT_7DEGREE   0.9925461516413
#define DOT_8DEGREE   0.9902680687416
#define DOT_9DEGREE   0.9876883405951
#define DOT_10DEGREE  0.9848077530122
#define DOT_15DEGREE  0.9659258262891
#define DOT_20DEGREE  0.9396926207859
#define DOT_25DEGREE  0.9063077870367

//=========================================================
// Autoaim
// set crosshair position to point to enemey
//=========================================================
Vector CBasePlayer :: GetAutoaimVector( float flDelta )
{
	if (g_iSkillLevel == SKILL_HARD)
	{
		UTIL_MakeVectors( pev->v_angle + pev->punchangle );
		return gpGlobals->v_forward;
	}

	Vector vecSrc = GetGunPosition( );
	float flDist = 8192;

	// always use non-sticky autoaim
	// UNDONE: use sever variable to chose!
	if (1 || g_iSkillLevel == SKILL_MEDIUM)
	{
		m_vecAutoAim = Vector( 0, 0, 0 );
		// flDelta *= 0.5;
	}

	BOOL m_fOldTargeting = m_fOnTarget;
	Vector angles = AutoaimDeflection(vecSrc, flDist, flDelta );

	// update ontarget if changed
	if ( !g_pGameRules->AllowAutoTargetCrosshair() )
		m_fOnTarget = 0;
	else if (m_fOldTargeting != m_fOnTarget)
	{
		m_pActiveItem->UpdateItemInfo( );
	}

	if (angles.x > 180)
		angles.x -= 360;
	if (angles.x < -180)
		angles.x += 360;
	if (angles.y > 180)
		angles.y -= 360;
	if (angles.y < -180)
		angles.y += 360;

	if (angles.x > 25)
		angles.x = 25;
	if (angles.x < -25)
		angles.x = -25;
	if (angles.y > 12)
		angles.y = 12;
	if (angles.y < -12)
		angles.y = -12;


	// always use non-sticky autoaim
	// UNDONE: use sever variable to chose!
	if (0 || g_iSkillLevel == SKILL_EASY)
	{
		m_vecAutoAim = m_vecAutoAim * 0.67 + angles * 0.33;
	}
	else
	{
		m_vecAutoAim = angles * 0.9;
	}

	// m_vecAutoAim = m_vecAutoAim * 0.99;

	// Don't send across network if sv_aim is 0
	if ( g_psv_aim->value != 0 )
	{
		if ( m_vecAutoAim.x != m_lastx ||
			 m_vecAutoAim.y != m_lasty )
		{
			SET_CROSSHAIRANGLE( edict(), -m_vecAutoAim.x, m_vecAutoAim.y );
			
			m_lastx = m_vecAutoAim.x;
			m_lasty = m_vecAutoAim.y;
		}
	}

	 //ALERT( at_console, "%f %f\n", angles.x, angles.y );

	UTIL_MakeVectors( pev->v_angle + pev->punchangle + m_vecAutoAim );
	return gpGlobals->v_forward;
}


Vector CBasePlayer :: AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  )
{
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity	*pEntity;
	float		bestdot;
	Vector		bestdir;
	edict_t		*bestent;
	TraceResult tr;

	if ( g_psv_aim->value == 0 )
	{
		m_fOnTarget = FALSE;
		return g_vecZero;
	}

	UTIL_MakeVectors( pev->v_angle + pev->punchangle + m_vecAutoAim );

	// try all possible entities
	bestdir = gpGlobals->v_forward;
	bestdot = flDelta; // +- 10 degrees
	bestent = NULL;

	m_fOnTarget = FALSE;

	UTIL_TraceLine( vecSrc, vecSrc + bestdir * flDist, dont_ignore_monsters, edict(), &tr );


	if ( tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
	{
		// don't look through water
		if (!((pev->waterlevel != 3 && tr.pHit->v.waterlevel == 3) 
			|| (pev->waterlevel == 3 && tr.pHit->v.waterlevel == 0)))
		{
			if (tr.pHit->v.takedamage == DAMAGE_AIM)
				m_fOnTarget = TRUE;

			return m_vecAutoAim;
		}
	}

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		Vector center;
		Vector dir;
		float dot;

		if ( pEdict->free )	// Not in use
			continue;
		
		if (pEdict->v.takedamage != DAMAGE_AIM)
			continue;
		if (pEdict == edict())
			continue;
//		if (pev->team > 0 && pEdict->v.team == pev->team)
//			continue;	// don't aim at teammate
		if ( !g_pGameRules->ShouldAutoAim( this, pEdict ) )
			continue;

		pEntity = Instance( pEdict );
		if (pEntity == NULL)
			continue;

		if (!pEntity->IsAlive())
			continue;

		// don't look through water
		if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) 
			|| (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
			continue;

		center = pEntity->BodyTarget( vecSrc );

		dir = (center - vecSrc).Normalize( );

		// make sure it's in front of the player
		if (DotProduct (dir, gpGlobals->v_forward ) < 0)
			continue;

		dot = fabs( DotProduct (dir, gpGlobals->v_right ) ) 
			+ fabs( DotProduct (dir, gpGlobals->v_up ) ) * 0.5;

		// tweek for distance
		dot *= 1.0 + 0.2 * ((center - vecSrc).Length() / flDist);

		if (dot > bestdot)
			continue;	// to far to turn

		UTIL_TraceLine( vecSrc, center, dont_ignore_monsters, edict(), &tr );
		if (tr.flFraction != 1.0 && tr.pHit != pEdict)
		{
		//	 ALERT( at_console, "hit %s, can't see %s\n", STRING( tr.pHit->v.classname ), STRING( pEdict->v.classname ) );
			continue;
		}

		// don't shoot at friends
		if (IRelationship( pEntity ) < 0)
		{
			if ( !pEntity->IsPlayer() && !g_pGameRules->IsDeathmatch())
				// ALERT( at_console, "friend\n");
				continue;
		}

		// can shoot at this one
		bestdot = dot;
		bestent = pEdict;
		bestdir = dir;
	}

	if (bestent)
	{
		bestdir = UTIL_VecToAngles (bestdir);
		bestdir.x = -bestdir.x;
		bestdir = bestdir - pev->v_angle - pev->punchangle;

		if (bestent->v.takedamage == DAMAGE_AIM)
			m_fOnTarget = TRUE;

		return bestdir;
	}

	return Vector( 0, 0, 0 );
}


void CBasePlayer :: ResetAutoaim( )
{
	if (m_vecAutoAim.x != 0 || m_vecAutoAim.y != 0)
	{
		m_vecAutoAim = Vector( 0, 0, 0 );
		SET_CROSSHAIRANGLE( edict(), 0, 0 );
	}
	m_fOnTarget = FALSE;
}

/*
=============
SetCustomDecalFrames

  UNDONE:  Determine real frame limit, 8 is a placeholder.
  Note:  -1 means no custom frames present.
=============
*/
void CBasePlayer :: SetCustomDecalFrames( int nFrames )
{
	if (nFrames > 0 &&
		nFrames < 8)
		m_nCustomSprayFrames = nFrames;
	else
		m_nCustomSprayFrames = -1;
}

/*
=============
GetCustomDecalFrames

  Returns the # of custom frames this player's custom clan logo contains.
=============
*/
int CBasePlayer :: GetCustomDecalFrames( void )
{
	return m_nCustomSprayFrames;
}


//=========================================================
// DropPlayerItem - drop the named item, or if no name,
// the active item. 
//=========================================================
void CBasePlayer::DropPlayerItem ( char *pszItemName )
{
	//SP: que sea posible
//	if ( !g_pGameRules->IsMultiplayer() || (weaponstay.value > 0) )
//	{
		// no dropping in single player.
//		return;
//	}

	//DONE
	//the problem was when the player tried to drop something. The client print func was bad used.
	//CBasePlayer *pWeaponCarrier = GetClassPtr((CBasePlayer *)pev);
		
	//BUG BUG: when the player have a suit and doesn't have weapons, the system crash. I was tring to
	//make the following fix, but it doesn't seems to work. Talking about C++ the code appears to be Ok, but
	//the engine can't handle it...
	/*
	CBasePlayer *pWeaponCarrier = ( CBasePlayer *) CBasePlayer::Instance( pev );

	if ( ( pWeaponCarrier->pev->weapons & (1<<WEAPON_SUIT) ) && (pWeaponCarrier->m_pActiveItem->m_iId != NULL) )//FIX
	{
		if ( pWeaponCarrier->m_pActiveItem->m_iId == WEAPON_CROWBAR )
		{
			ClientPrint(pWeaponCarrier->pev, HUD_PRINTCENTER, "#CantDrop");
			return;
		}
		else if ( pWeaponCarrier->m_pActiveItem->m_iId == WEAPON_HANDGRENADE )
		{
			ClientPrint(pWeaponCarrier->pev, HUD_PRINTCENTER, "#CantDrop");
			return;
		}
		else if ( pWeaponCarrier->m_pActiveItem->m_iId == WEAPON_HEAL )
		{
			ClientPrint(pWeaponCarrier->pev, HUD_PRINTCENTER, "#CantDrop");
			return;
		}
		else
		{
			return;
		}
	}
*/
	if ( !strlen( pszItemName ) )
	{
		// if this string has no length, the client didn't type a name!
		// assume player wants to drop the active item.
		// make the string null to make future operations in this function easier
		pszItemName = NULL;
	} 

	CBasePlayerItem *pWeapon;
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pWeapon = m_rgpPlayerItems[ i ];

		while ( pWeapon )
		{
			if ( pszItemName )
			{
				// try to match by name. 
				if ( !strcmp( pszItemName, STRING( pWeapon->pev->classname ) ) )
				{
					// match! 
					break;
				}
			}
			else
			{
				// trying to drop active item
				if ( pWeapon == m_pActiveItem )
				{
					// active item!
					break;
				}
			}

			pWeapon = pWeapon->m_pNext; 
		}

//copied and pasted from 
		Vector angThrow = pev->v_angle + pev->punchangle;

		if ( angThrow.x < 0 )
			angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
		else
			angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

		float flVel = ( 90 - angThrow.x ) * 4;
		if ( flVel > 500 )
			flVel = 500;

		UTIL_MakeVectors( angThrow );

		Vector vecSrc = pev->origin + pev->view_ofs + gpGlobals->v_forward * 16;//16

		Vector vecThrow = gpGlobals->v_forward * flVel + pev->velocity;
//copied and pasted from 
		
		// if we land here with a valid pWeapon pointer, that's because we found the 
		// item we want to drop and hit a BREAK;  pWeapon is the item.
		if ( pWeapon )
		{
			g_pGameRules->GetNextBestWeapon( this, pWeapon );

		//	UTIL_MakeVectors ( pev->angles ); 

			pev->weapons &= ~(1<<pWeapon->m_iId);// take item off hud
		
			CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::
				Create( "weaponbox", vecSrc, vecThrow, edict() );

		//	CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, edict() );
		
			//this is the problem?
		//	pWeaponBox->pev->angles.x = 0;
		//	pWeaponBox->pev->angles.z = 0;

		//	pev->angles.x = RANDOM_FLOAT( 25, -25 );
		//	pev->angles.y =  RANDOM_FLOAT( 25, -25 );
		//	pev->angles.z =  RANDOM_FLOAT( 25, -25 );

			pev->angles.x = 90;
			pev->angles.y =  90;
		//	pev->angles.z =  RANDOM_FLOAT( 25, -25 );

			//pack weapon? sure!
			pWeaponBox->PackWeapon( pWeapon );
		
			//velocity? erm... nope
			//pWeaponBox->pev->velocity = gpGlobals->v_forward * 300 + gpGlobals->v_forward * 100;

			pWeaponBox->pev->velocity = gpGlobals->v_forward * 200 + gpGlobals->v_forward * 75;
			
			//sys test		
	//		pWeaponBox->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 20, 100 ), 0 );
			
			// drop half of the ammo for this weapon.
			int	iAmmoIndex;

			iAmmoIndex = GetAmmoIndex ( pWeapon->pszAmmo1() ); // ???

			/*
			if ( iAmmoIndex != -1 )
			{
				// this weapon weapon uses ammo, so pack an appropriate amount.
				if ( pWeapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE )
				{
					// pack up all the ammo, this weapon is its own ammo type
					pWeaponBox->PackAmmo( MAKE_STRING(pWeapon->pszAmmo1()), m_rgAmmo[ iAmmoIndex ] );
					m_rgAmmo[ iAmmoIndex ] = 0; 

				}
				else
				{
					// pack half of the ammo
					pWeaponBox->PackAmmo( MAKE_STRING(pWeapon->pszAmmo1()), m_rgAmmo[ iAmmoIndex ] / 2 );
					m_rgAmmo[ iAmmoIndex ] /= 2; 
				}
			}
			*/

			return;// we're done, so stop searching with the FOR loop.
		}
	}
}

//=========================================================
// HasPlayerItem Does the player already have this item?
//=========================================================
BOOL CBasePlayer::HasPlayerItem( CBasePlayerItem *pCheckItem )
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem)
	{
		if (FClassnameIs( pItem->pev, STRING( pCheckItem->pev->classname) ))
		{
			return TRUE;
		}
		pItem = pItem->m_pNext;
	}

	return FALSE;
}

//=========================================================
// HasNamedPlayerItem Does the player already have this item?
//=========================================================
BOOL CBasePlayer::HasNamedPlayerItem( const char *pszItemName )
{
	CBasePlayerItem *pItem;
	int i;
 
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pItem = m_rgpPlayerItems[ i ];
		
		while (pItem)
		{
			if ( !strcmp( pszItemName, STRING( pItem->pev->classname ) ) )
			{
				return TRUE;
			}
			pItem = pItem->m_pNext;
		}
	}

	return FALSE;
}

//=========================================================
// 
//=========================================================
BOOL CBasePlayer :: SwitchWeapon( CBasePlayerItem *pWeapon ) 
{
	if ( !pWeapon->CanDeploy() )
	{
		return FALSE;
	}

	// Si le joueur n'a pas d'arme on lui en donne une quand
	// même
//	if (autoswitch == 0 && m_pActiveItem)
//		return FALSE;

	ResetAutoaim( );
	
	if (m_pActiveItem)
	{
		m_pActiveItem->Holster( );
	}

	m_pActiveItem = pWeapon;
	pWeapon->Deploy( );

	return TRUE;
}

//=========================================================
// Dead HEV suit prop
//=========================================================
class CDeadHEV : public CBaseMonster
{
public:
	void Spawn( void );
	int	Classify ( void ) { return	CLASS_HUMAN_MILITARY; }

	void KeyValue( KeyValueData *pkvd );

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[4];
};

char *CDeadHEV::m_szPoses[] = { "deadback", "deadsitting", "deadstomach", "deadtable" };

void CDeadHEV::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_hevsuit_dead, CDeadHEV );

//=========================================================
// ********** DeadHEV SPAWN **********
//=========================================================
void CDeadHEV :: Spawn( void )
{
	PRECACHE_MODEL("models/player.mdl");
	SET_MODEL(ENT(pev), "models/player.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	pev->body			= 1;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );

	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead hevsuit with bad pose\n" );
		pev->sequence = 0;
		pev->effects = EF_BRIGHTFIELD;
	}

	// Corpses have less health
	pev->health			= 8;

	MonsterInitDead();
}

/*
class CStripWeapons : public CPointEntity
{
public:
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:
};

LINK_ENTITY_TO_CLASS( player_weaponstrip, CStripWeapons );

void CStripWeapons :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = NULL;

	if ( pActivator && pActivator->IsPlayer() )
	{
		pPlayer = (CBasePlayer *)pActivator;
	}
	else if ( !g_pGameRules->IsDeathmatch() )
	{
		pPlayer = (CBasePlayer *)CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );
	}

	if ( pPlayer )
		pPlayer->RemoveAllItems( FALSE );
}
*/
class CStripWeapons : public CPointEntity
{
public:
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue( KeyValueData *pkvd );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

private:
//	int m_iAmmo[MAX_WEAPONS];
	int m_i9mm;
	int m_i357;
	int m_iBuck;
	int m_iBolt;
	int m_iARGren;
	int m_iRock;
	int m_iUranium;
	int m_iSatchel;
	int m_iSnark;
	int m_iTrip;
	int m_iGren;
	int m_iHornet;
};

LINK_ENTITY_TO_CLASS( player_weaponstrip, CStripWeapons );

TYPEDESCRIPTION	CStripWeapons::m_SaveData[] =
{
	DEFINE_FIELD( CStripWeapons, m_i9mm, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_i357, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iBuck, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iBolt, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iARGren, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iRock, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iUranium, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iSatchel, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iSnark, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iTrip, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iGren, FIELD_INTEGER ),
	DEFINE_FIELD( CStripWeapons, m_iHornet, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CStripWeapons, CPointEntity );

void CStripWeapons :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "bullets"))
	{
		m_i9mm= atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "magnum"))
	{
		m_i357 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shotgun"))
	{
		m_iBuck = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "crossbow"))
	{
		m_iBolt = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "argrenades"))
	{
		m_iARGren = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "rockets"))
	{
		m_iRock = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "uranium"))
	{
		m_iUranium = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "satchels"))
	{
		m_iSatchel = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "snarks"))
	{
		m_iSnark = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "tripmines"))
	{
		m_iTrip = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "handgrenades"))
	{
		m_iGren = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "hornetgun"))
	{
		m_iHornet = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CStripWeapons :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = NULL;

	if ( pActivator && pActivator->IsPlayer() )
	{
		pPlayer = (CBasePlayer *)pActivator;
	}
	else if ( !g_pGameRules->IsDeathmatch() )
	{
		pPlayer = (CBasePlayer *)CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );
	}

	if ( pPlayer )
	{
		pPlayer->RemoveItems( pev->spawnflags, m_i9mm, m_i357, m_iBuck, m_iBolt,
				m_iARGren, m_iRock, m_iUranium, m_iSatchel, m_iSnark, m_iTrip, m_iGren, m_iHornet);
//		pPlayer->RemoveAllItems( FALSE );
	}
}

class CRevertSaved : public CPointEntity
{
public:
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT MessageThink( void );
	void	EXPORT LoadThink( void );
	void	KeyValue( KeyValueData *pkvd );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	inline	float	Duration( void ) { return pev->dmg_take; }
	inline	float	HoldTime( void ) { return pev->dmg_save; }
	inline	float	MessageTime( void ) { return m_messageTime; }
	inline	float	LoadTime( void ) { return m_loadTime; }

	inline	void	SetDuration( float duration ) { pev->dmg_take = duration; }
	inline	void	SetHoldTime( float hold ) { pev->dmg_save = hold; }
	inline	void	SetMessageTime( float time ) { m_messageTime = time; }
	inline	void	SetLoadTime( float time ) { m_loadTime = time; }

private:
	float	m_messageTime;
	float	m_loadTime;
};

LINK_ENTITY_TO_CLASS( player_loadsaved, CRevertSaved );

TYPEDESCRIPTION	CRevertSaved::m_SaveData[] = 
{
	DEFINE_FIELD( CRevertSaved, m_messageTime, FIELD_FLOAT ),	// These are not actual times, but durations, so save as floats
	DEFINE_FIELD( CRevertSaved, m_loadTime, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CRevertSaved, CPointEntity );

void CRevertSaved :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "messagetime"))
	{
		SetMessageTime( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "loadtime"))
	{
		SetLoadTime( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else 
		CPointEntity::KeyValue( pkvd );
}

void CRevertSaved :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	UTIL_ScreenFadeAll( pev->rendercolor, Duration(), HoldTime(), pev->renderamt, FFADE_OUT );
	pev->nextthink = gpGlobals->time + MessageTime();
	SetThink( MessageThink );
}


void CRevertSaved :: MessageThink( void )
{
	UTIL_ShowMessageAll( STRING(pev->message) );
	float nextThink = LoadTime() - MessageTime();
	if ( nextThink > 0 ) 
	{
		pev->nextthink = gpGlobals->time + nextThink;
		SetThink( LoadThink );
	}
	else
		LoadThink();
}


void CRevertSaved :: LoadThink( void )
{
	if ( !gpGlobals->deathmatch )
	{
		SERVER_COMMAND("reload\n");
	}
}


//=========================================================
// Multiplayer intermission spots.
//=========================================================
class CInfoIntermission:public CPointEntity
{
	void Spawn( void );
	void Think( void );
};

void CInfoIntermission::Spawn( void )
{
	UTIL_SetOrigin( pev, pev->origin );
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->v_angle = g_vecZero;

	pev->nextthink = gpGlobals->time + 2;// let targets spawn!

}

void CInfoIntermission::Think ( void )
{
	edict_t *pTarget;

	// find my target
	pTarget = FIND_ENTITY_BY_TARGETNAME( NULL, STRING(pev->target) );

	if ( !FNullEnt(pTarget) )
	{
		pev->v_angle = UTIL_VecToAngles( (pTarget->v.origin - pev->origin).Normalize() );
		pev->v_angle.x = -pev->v_angle.x;
	}
}

LINK_ENTITY_TO_CLASS( info_intermission, CInfoIntermission );

//LRC
void CBasePlayer::RemoveAmmo( const char* szName, int iAmount )
{
//	ALERT(at_console, "RemoveAmmo(\"%s\", %d): \n", szName, iAmount);

	if (iAmount == -3 || iAmount == -1)
	{
		return;
	}

	int x = GetAmmoIndex(szName);

	if (iAmount > 0)
	{
		m_rgAmmo[x] -= iAmount;
		if (m_rgAmmo[x] < 0)
		{
			m_rgAmmo[x] = 0;
//			ALERT(at_console, "Reduce to 0\n");
		}
//		else
//			ALERT(at_console, "Reduce\n");
	}
	else
	{
//		ALERT(at_console, "All\n");
		m_rgAmmo[x] = 0;
	}
}

//LRC
void CBasePlayer::RemoveItems( int iWeaponMask, int i9mm, int i357, int iBuck, int iBolt, int iARGren, int iRock, int iUranium, int iSatchel, int iSnark, int iTrip, int iGren, int iHornet )
{
	int i;
	CBasePlayerItem *pCurrentItem;

	// hornetgun is outside the spawnflags Worldcraft can set - handle it seperately.
	if (iHornet)
		iWeaponMask |= 1<<WEAPON_NONE; //WEAPON_HORNETGUN;

	RemoveAmmo("9mm", i9mm);
	RemoveAmmo("357", i357);
	RemoveAmmo("buckshot", iBuck);
	RemoveAmmo("bolts", iBolt);
	RemoveAmmo("ARgrenades", iARGren);
	RemoveAmmo("uranium", iUranium);
	RemoveAmmo("rockets", iRock);
	RemoveAmmo("Satchel Charge", iSatchel);
	RemoveAmmo("Snarks", iSnark);
	RemoveAmmo("Trip Mine", iTrip);
	RemoveAmmo("Hand Grenade", iGren);
	RemoveAmmo("Hornets", iHornet);

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i] == NULL)
			continue;
		pCurrentItem = m_rgpPlayerItems[i];
		while (pCurrentItem->m_pNext)
		{
			if (!(1<<pCurrentItem->m_pNext->m_iId & iWeaponMask))
			{
				((CBasePlayerWeapon*)pCurrentItem)->DrainClip(this, FALSE, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
				//remove pCurrentItem->m_pNext from the list
//				ALERT(at_console, "Removing %s. (id = %d)\n", pCurrentItem->m_pNext->pszName(), pCurrentItem->m_pNext->m_iId);
				pCurrentItem->m_pNext->Drop( );
				if (m_pLastItem == pCurrentItem->m_pNext)
					m_pLastItem = NULL;
				pCurrentItem->m_pNext = pCurrentItem->m_pNext->m_pNext;
			}
			else
			{
				//we're keeping this, so we need to empty the clip
				((CBasePlayerWeapon*)pCurrentItem)->DrainClip(this, TRUE, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
				//now, leave pCurrentItem->m_pNext in the list and go on to the next
//				ALERT(at_console, "Keeping %s. (id = %d)\n", pCurrentItem->m_pNext->pszName(), pCurrentItem->m_pNext->m_iId);
				pCurrentItem = pCurrentItem->m_pNext;
			}
		}
		// we've gone through items 2+, now we finish off by checking item 1.
		if (!(1<<m_rgpPlayerItems[i]->m_iId & iWeaponMask))
		{
			((CBasePlayerWeapon*)pCurrentItem)->DrainClip(this, FALSE, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
//			ALERT(at_console, "Removing %s. (id = %d)\n", m_rgpPlayerItems[i]->pszName(), m_rgpPlayerItems[i]->m_iId);
			m_rgpPlayerItems[i]->Drop( );
			if (m_pLastItem == m_rgpPlayerItems[i])
				m_pLastItem = NULL;
			m_rgpPlayerItems[i] = m_rgpPlayerItems[i]->m_pNext;
		}
		else
		{
			((CBasePlayerWeapon*)pCurrentItem)->DrainClip(this, TRUE, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
//			ALERT(at_console, "Keeping %s. (id = %d)\n", m_rgpPlayerItems[i]->pszName(), m_rgpPlayerItems[i]->m_iId);
		}
	}

	int suit = pev->weapons & 1<<WEAPON_SUIT;
	pev->weapons &= ~(1<<WEAPON_SUIT);
//	ALERT(at_console, "weapons was %d; ", pev->weapons);
	pev->weapons &= iWeaponMask;
//	ALERT(at_console, "now %d\n(Mask is %d)", pev->weapons, iWeaponMask);
	if (suit && !(iWeaponMask & 1))
		pev->weapons |= 1<<WEAPON_SUIT;

	// are we dropping the active item?
	if (m_pActiveItem && !(1<<m_pActiveItem->m_iId & iWeaponMask))
	{
		ResetAutoaim( );
		m_pActiveItem->Holster( );
		pev->viewmodel = 0;
		pev->weaponmodel = 0;
		m_pActiveItem = NULL;

		UpdateClientData();
		// send Selected Weapon Message to our client
		MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_BYTE(0);
		MESSAGE_END();
	}
	else
	{
		if (m_pActiveItem && !((CBasePlayerWeapon*)m_pActiveItem)->IsUseable())
		{
			//lower the gun if it's out of ammo
			((CBasePlayerWeapon*)m_pActiveItem)->m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
		}
		UpdateClientData();
	}
}

//==============================================================
// Hud sprite displayer
//==============================================================
#define SF_HUDSPR_ACTIVE 1

class CHudSprite:public CBaseEntity
{
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	STATE GetState( void ) { return pev->spawnflags & SF_HUDSPR_ACTIVE? STATE_ON:STATE_OFF; }
	void Think( void );
//	void	Precache( void );//sys
};
/*
void CHudSprite::Precache( void )//sys
{
	PRECACHE_MODEL( (char *)STRING(pev->model) );
}
*/
void CHudSprite::Spawn( void )
{
//	Precache();//sys
//	SET_MODEL( ENT(pev), STRING(pev->model) );

	if (FStringNull(pev->targetname))
	{
		pev->spawnflags |= SF_HUDSPR_ACTIVE;
	}

	if (pev->spawnflags & SF_HUDSPR_ACTIVE)
	{
	//	SetNextThink(2);
		pev->nextthink = 2;
	}
}

void CHudSprite::Think( void )
{
	Use(this, this, USE_ON, 0);
}

void CHudSprite::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
	{
		pActivator = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex( 1 ));
	}

	if (ShouldToggle(useType))
	{
		if (pev->spawnflags & SF_HUDSPR_ACTIVE)
			pev->spawnflags &= ~SF_HUDSPR_ACTIVE;
		else
			pev->spawnflags |= SF_HUDSPR_ACTIVE;
	}

//		byte   : TRUE = ENABLE icon, FALSE = DISABLE icon
//		string : the sprite name to display
//		byte   : red
//		byte   : green
//		byte   : blue
	MESSAGE_BEGIN( MSG_ONE, gmsgStatusIcon, NULL, pActivator->pev );
		WRITE_BYTE(pev->spawnflags & SF_HUDSPR_ACTIVE);
		WRITE_STRING(STRING(pev->model));
		WRITE_BYTE(pev->rendercolor.x);
		WRITE_BYTE(pev->rendercolor.y);
		WRITE_BYTE(pev->rendercolor.z);
	MESSAGE_END();
}

LINK_ENTITY_TO_CLASS( hud_sprite, CHudSprite );

//Jeje, test, just for fun
/*
void CBasePlayer::Jetpack( void ) //the function itself
{
	//old code
	if ( FBitSet( m_afButtonPressed, IN_DUCK ) )
	{
		pev->velocity.z = -50;
	}
	if ( FBitSet( m_afButtonPressed, IN_JUMP ) )
	{
		pev->velocity.z = 100;
	}
}
*/
void CBasePlayer::RestoreSpeedVol(void)
{
	SERVER_COMMAND("volume 0.8\n");
			
	if(bChangeSound)
	SERVER_COMMAND ("room_type 0");

	g_engfuncs.pfnSetClientMaxspeed( ENT( pev ), 200 ); //200 y despues el prethink.

//	ALERT( at_console, "# Volume and Speed Restored\n");

	m_fHurted	= FALSE;//yeah
}

//////////////////////////////////////////////////////////////
// Show Menu /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void CBasePlayer::ShowMenu (CBasePlayer *pPlayer, int bitsValidSlots, int nDisplayTime, BOOL fNeedMore, char *pszText)
{
    MESSAGE_BEGIN( MSG_ONE, gmsgShowMenu, NULL, pPlayer ->pev);
        WRITE_SHORT( bitsValidSlots);
        WRITE_CHAR( nDisplayTime );
        WRITE_BYTE( fNeedMore );
        WRITE_STRING (pszText);
    MESSAGE_END();
}

void CBasePlayer::CineToggle(BOOL activate)
{
	if (activate && !m_fCineOn) 
	{
		m_fCineOn = TRUE; // activate the NVG

		// inform the client about the change
		MESSAGE_BEGIN(MSG_ONE, gmsgCine, NULL, pev);
			WRITE_BYTE( 1 );
		MESSAGE_END( );
	} 
	else if (!activate && m_fCineOn) 
	{	
		m_fCineOn = FALSE; // deactivate the NVG

		// inform the client about the change
		MESSAGE_BEGIN(MSG_ONE, gmsgCine, NULL, pev);
			WRITE_BYTE( 0 );
		MESSAGE_END( );
	}
}

void CBasePlayer::RicardoToggle(BOOL activate)
{
	if (activate && !m_fRicardoOn) 
	{
		m_fRicardoOn = TRUE; // activate the NVG

		// inform the client about the change
		MESSAGE_BEGIN(MSG_ONE, gmsgRicardo, NULL, pev);
			WRITE_BYTE( 1 );
		MESSAGE_END( );
	} 
	else if (!activate && m_fRicardoOn) 
	{	
		m_fRicardoOn = FALSE; // deactivate the NVG

		// inform the client about the change
		MESSAGE_BEGIN(MSG_ONE, gmsgRicardo, NULL, pev);
			WRITE_BYTE( 0 );
		MESSAGE_END( );
	}
}
