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

//===================
// NEW WEAPON FILE
//===================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

#include "effects.h"

#include "shake.h" //This is required for the screen fade

#include "decals.h"

#define	CROWBAR_BODYHIT_VOLUME 64
#define	CROWBAR_WALLHIT_VOLUME 256

enum flash_e
{
	FLASH_IDLEOFF_1 = 0,
	FLASH_DEPLOY,
	FLASH_IDLEON_1,
	FLASH_USE,
	FLASH_HOLSTER_OFF,
	FLASH_HOLSTER_ON,
	FLASH_USE_ENTITY,
	FLASH_ATTACK1,
	FLASH_ATTACK2,
	FLASH_ATTACK3
};

LINK_ENTITY_TO_CLASS( weapon_flash, CFlash );
LINK_ENTITY_TO_CLASS( weapon_flashlight, CFlash );

#ifndef CLIENT_DLL

// === This is the laser-pointer-code:

CFlashSpotTEST *CFlashSpotTEST::CreateFlashSpotTEST( void )
{
CFlashSpotTEST *pSpot = GetClassPtr( (CFlashSpotTEST *)NULL );
pSpot->Spawn();

pSpot->pev->classname = MAKE_STRING("laser_spot_flash");

return pSpot;
}
//=========================================================
//=========================================================
void CFlashSpotTEST::Spawn( void )
{
TraceResult tr;

Precache( );
pev->movetype = MOVETYPE_NONE;
pev->solid = SOLID_NOT;

pev->rendermode = kRenderGlow;
pev->renderfx = kRenderFxNoDissipation;
pev->renderamt = 155;//255?

SET_MODEL(ENT(pev), "sprites/flash_beam.spr");
UTIL_SetOrigin( pev, pev->origin );
};
LINK_ENTITY_TO_CLASS( laser_spot_flash, CFlashSpotTEST );

//=========================================================
// Suspend- make the laser sight invisible.
//=========================================================
void CFlashSpotTEST::Suspend( float flSuspendTime )
{
pev->effects |= EF_NODRAW;

SetThink( Revive );
pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CFlashSpotTEST::Revive( void )
{
pev->effects &= ~EF_NODRAW;

SetThink( NULL );
}

void CFlashSpotTEST::Precache( void )
{
PRECACHE_MODEL("sprites/flash_beam.spr");
};

#endif




//=========================================================
//=========================================================
void CFlash::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_flash"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/flash/w_flash.mdl");
	m_iId = WEAPON_FLASHLIGHT;

	m_iClip = -1;
//	m_fSpotActive = 1;

/*
	m_FlashGlow = CSprite::SpriteCreate( "sprites/glow04.spr", pev->origin, FALSE );
	m_FlashGlow->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
	m_FlashGlow->SetAttachment( edict(), 0 );
	m_FlashGlow->SetScale( 5.0 );
	m_iFlashBrightness = 0;
*/
	FallInit();// get ready to fall down.
}


void CFlash::Precache( void )
{
	PRECACHE_MODEL("models/weapons/flash/v_flash.mdl");
	PRECACHE_MODEL("models/weapons/flash/w_flash.mdl");
	PRECACHE_MODEL("models/weapons/flash/p_flash.mdl");

	PRECACHE_SOUND ("weapons/flash/flash_fire-1.wav");
	
//	PRECACHE_MODEL( "sprites/glow04.spr" );
}

int CFlash::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 3;//mostrar
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_FLASHLIGHT;
	p->iWeight = 10;
	p->weaponName = "Linterna";


	return 1;
}

int CFlash::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Flash"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();

		return TRUE;
	}
	return FALSE;
}

BOOL CFlash::Deploy( )
{
	m_pPlayer->m_fCrosshairOff = TRUE;

	m_fSpotActive = 0;//FIX?

	m_pPlayer->m_fCanUseFlashlight = TRUE;
	m_pPlayer->m_fCrosshairOff = TRUE;

#ifdef CLIENT_DLL
	if ( !bIsMultiplayer() )
#else
	if ( !g_pGameRules->IsMultiplayer() )
#endif
	{
			pev->body = 0;
	}

	pev->skin = 0;
#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgSetSkin, NULL, m_pPlayer->pev );
		WRITE_BYTE( pev->skin ); //weaponmodel skin.
	MESSAGE_END();
#endif	
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
	return DefaultDeploy("models/weapons/flash/v_flash.mdl", "models/weapons/flash/p_flash.mdl", FLASH_DEPLOY, "mp5a4" );
}

void CFlash::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_fCanUseFlashlight = FALSE;

	if ( m_pPlayer->FlashlightIsOn() )//FIX: para no apagar lo q no está prendido.
	{
		m_pPlayer->FlashlightTurnOff();

#ifdef CLIENT_DLL
		if ( !bIsMultiplayer() )
#else
		if ( !g_pGameRules->IsMultiplayer() )
#endif
		{
			pev->body = 0;
		}
	}
		
	pev->skin = 0;

#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgSetSkin, NULL, m_pPlayer->pev );
		WRITE_BYTE( pev->skin ); //weaponmodel skin.
	MESSAGE_END();
#endif	
	m_pPlayer->m_fCrosshairOff = FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5; //0.5

	SendWeaponAnim( FLASH_HOLSTER_OFF );

#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NEVER );
		m_pSpot = NULL;
	}
#endif
}

void CFlash::PrimaryAttack()
{	
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))//FIX
    return;

	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	SendWeaponAnim( FLASH_USE );

	if ( m_pPlayer->FlashlightIsOn() )
	{
		m_pPlayer->FlashlightTurnOff();//OFF
		
//kill the beam
#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NEVER );
		m_pSpot = NULL;
	}
#endif
		m_fSpotActive = 0;
			
		pev->skin = 0;

	/*	
		if (m_FlashGlow)
		{
			if (m_iFlashBrightness > 0)
			{
				m_iFlashBrightness = 0;
			}
			m_FlashGlow->SetBrightness( m_iFlashBrightness );
		}
		*/

/*
//hide the flash beam
#ifdef CLIENT_DLL
		if ( !bIsMultiplayer() )
#else
		if ( !g_pGameRules->IsMultiplayer() )
#endif
		{
			pev->body = 1;
		}
*/				
	}
	else 		
	{	
		m_pPlayer->FlashlightTurnOn();	//ON
		m_fSpotActive = 1;

		pev->skin = 1;
/*
		if (m_FlashGlow)
		{
			if (m_iFlashBrightness != 255)
			{
				m_iFlashBrightness = 255;
			}
			m_FlashGlow->SetBrightness( m_iFlashBrightness );
		}
*/

/*
#ifdef CLIENT_DLL
		if ( !bIsMultiplayer() )
#else
		if ( !g_pGameRules->IsMultiplayer() )
#endif
		{
			pev->body = 0;
		}
		*/
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5; // delay

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5; // delay

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	
	//esto antes estaba comentado
	UpdateSpot( );

#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgSetSkin, NULL, m_pPlayer->pev );
		WRITE_BYTE( pev->skin ); //weaponmodel skin.
	MESSAGE_END();
#endif	
	/*
	if (m_bIsHolding==TRUE)
	{
		m_pEntity->pev->velocity = m_pEntity->pev->velocity + gpGlobals->v_forward * 1000;
		m_bIsHolding=0;
	}
	*/
}

void CFlash::SecondaryAttack()
{

}


void CFlash::WeaponIdle( void )
{
	UpdateSpot( );
	MakeLaserSight();

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = FLASH_IDLEOFF_1;
		break;
	
	default:
	case 1:
		iAnim = FLASH_IDLEOFF_1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 20, 25 ); // how long till we do this again. 10 15
}


void CFlash::UpdateSpot( void )
{
#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CFlashSpotTEST::CreateFlashSpotTEST();
		}

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( );
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
	
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 512, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );

//		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		
		UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );
	}
#endif
}

//EDIT: This is used for make the laser sprite visible. This fucking shit don't work and i don't wanna fix it.
void CFlash::MakeLaserSight( void )
{
#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			KillLaserSight();

			TraceResult tr;

			UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

			Vector m_vecSrc = m_pPlayer->GetGunPosition( );
			Vector m_vecDir = gpGlobals->v_forward;
			Vector m_vecEnd = pev->origin + m_vecDir * 2048;
			Vector m_vecOri = pev->origin;



		//	UTIL_TraceLine( m_vecSrc, m_vecEnd, dont_ignore_monsters, ENT(pev), &tr );
		
			UTIL_TraceLine( m_vecOri, m_vecSrc + m_vecDir * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );

//			m_flBeamLength = tr.flFraction;

			Vector vecTmpEnd = (pev->origin + Vector( 0, 0, 27 ) ) + m_vecDir * 2048 * m_flBeamLength;
			Vector m_vecFinalEnd = vecTmpEnd + Vector( 0, 0, 25); 

			m_pBeam = CBeam::BeamCreate( g_pModelNameLaser, 1 );
	
			 m_pBeam->PointEntInit( vecTmpEnd, entindex() ); 
			 m_pBeam->PointsInit(vecTmpEnd, (m_vecOri + Vector( 0, 0, 21)) );
			m_pBeam->PointsInit( vecTmpEnd, m_vecSrc );

			m_pBeam->SetColor( 255, 0, 0 );
			m_pBeam->SetScrollRate( 255 );
			m_pBeam->SetBrightness( 64 );
			m_pBeam->SetWidth( 25 );
		}
	}
#endif
}

void CFlash::KillLaserSight( void )
{
if ( m_pBeam )
{
UTIL_Remove( m_pBeam );
m_pBeam = NULL;
}
}

/* secondary attack code

  	/*
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecDir = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition( ); // + gpGlobals->v_up * -8 + gpGlobals->v_right * 8;
	Vector vecDest = vecSrc + vecDir * 4444;

	edict_t *pentIgnore;
	TraceResult tr;
	pentIgnore = m_pPlayer->edict();

	UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);
			
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

//		if (!(IsNear))
//		{
//			pEntity->pev->velocity = pEntity->pev->velocity + gpGlobals->v_forward * -50;
//		}
//		else
//		{
//			pEntity->pev->velocity = pEntity->pev->velocity + gpGlobals->v_forward * 999;
//		}

		if (pEntity)
		{
			m_bIsHolding = TRUE;
			m_pEntity = pEntity;
		}
	}

	//nada por el momento
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1; 
	*/

/* weapon idle code
void CFlash::WeaponIdle( void )
{
	UpdateSpot( );
	MakeLaserSight();

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecSrc = m_pPlayer->GetGunPosition( ); // + gpGlobals->v_up * -8 + gpGlobals->v_right * 8;
	Vector vecDir = gpGlobals->v_forward;
	Vector vecDest = vecSrc + vecDir * 4444;
/*
	edict_t *pentIgnore;
	TraceResult tr;
	pentIgnore = m_pPlayer->edict();

	UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);
	
	if ( tr.pHit )
	{		
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		float flDist = (pEntity->Center() - pev->origin).Length();

		if ( flDist <= 40)
		{
			ALERT ( at_console, "pEntity is near! Distance is: %f\n", flDist );
			IsNear = TRUE;
		}
		else
		{
			IsNear = FALSE;
		}
		ALERT ( at_console, "I'm looking %s -The Distance between this and me is: %f\n", STRING( tr.pHit->v.classname ), flDist );

//		ALERT( at_console, "hit %s, can't see %s\n", STRING( tr.pHit->v.classname ), STRING( pentIgnore->v.classname ) );
	}
*/
	/*
	if (m_bIsHolding==TRUE)
	{
		m_pEntity->pev->velocity = m_pEntity->pev->velocity + gpGlobals->v_forward * -50;

		if (pev->movetype == MOVETYPE_STEP)
		{
			//m_pEntity->pev->health -= 100;
			m_pEntity->Killed( pev, GIB_NEVER );
		}
	}

	extern short g_sModelIndexLaser;
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_BEAMPOINTS );
			WRITE_COORD( vecSrc.x );
			WRITE_COORD( vecSrc.y );
			WRITE_COORD( vecSrc.z );

			WRITE_COORD( vecDest.x );
			WRITE_COORD( vecDest.y );
			WRITE_COORD( vecDest.z );
			WRITE_SHORT( g_sModelIndexLaser );
			WRITE_BYTE( 0 ); // framerate
			WRITE_BYTE( 0 ); // framerate

			WRITE_BYTE( 1 ); // life
			WRITE_BYTE( 1 );  // width

			WRITE_BYTE( 0 );   // noise
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 255 );	// brightness
			WRITE_BYTE( 111 );		// speed
		MESSAGE_END();
	
}
*/