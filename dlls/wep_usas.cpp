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
#include "gamerules.h"

// special deathmatch shotgun spreads
#define VECTOR_CONE_DM_SHOTGUN	Vector( 0.08716, 0.04362, 0.00  )// 10 degrees by 5 degrees
#define VECTOR_CONE_DM_DOUBLESHOTGUN Vector( 0.17365, 0.04362, 0.00 ) // 20 degrees by 5 degrees

#ifndef CLIENT_DLL

// === This is the laser-pointer-code:

CLaserSpotTEST *CLaserSpotTEST::CreateSpotTEST( void )
{
CLaserSpotTEST *pSpot = GetClassPtr( (CLaserSpotTEST *)NULL );
pSpot->Spawn();

pSpot->pev->classname = MAKE_STRING("laser_spot_TEST");

return pSpot;
}
//=========================================================
//=========================================================
void CLaserSpotTEST::Spawn( void )
{
TraceResult tr;

Precache( );
pev->movetype = MOVETYPE_NONE;
pev->solid = SOLID_NOT;

pev->rendermode = kRenderGlow;
pev->renderfx = kRenderFxNoDissipation;
pev->renderamt = 255;

SET_MODEL(ENT(pev), "sprites/laser_small.spr");
UTIL_SetOrigin( pev, pev->origin );
};
LINK_ENTITY_TO_CLASS( laser_spot_TEST, CLaserSpotTEST );

//=========================================================
// Suspend- make the laser sight invisible.
//=========================================================
void CLaserSpotTEST::Suspend( float flSuspendTime )
{
pev->effects |= EF_NODRAW;

SetThink( Revive );
pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpotTEST::Revive( void )
{
pev->effects &= ~EF_NODRAW;

SetThink( NULL );
}

void CLaserSpotTEST::Precache( void )
{
PRECACHE_MODEL("sprites/laser_small.spr");
};

#endif

enum usas_e 
{
	USAS_IDLE1 = 0,
	USAS_RELOAD,
	USAS_DEPLOY,
	USAS_FIRE1,
};

LINK_ENTITY_TO_CLASS( weapon_usas, CUsas );
//LINK_ENTITY_TO_CLASS( weapon_m3, CUsas );
void CUsas::Spawn( )
{
	Precache( );
	m_iId = WEAPON_USAS;
	SET_MODEL(ENT(pev), "models/weapons/usas/w_usas.mdl");

	m_iDefaultAmmo = USAS_DEFAULT_GIVE;

	m_fSpotActive = 1;

	FallInit();// get ready to fall
}


void CUsas::Precache( void )
{
	//test
//	m_usSpriteTracer = PRECACHE_EVENT( 1, "events/sprite_tracer.sc" );

//	PRECACHE_MODEL("sprites/laserbeam.spr");

	PRECACHE_MODEL("models/weapons/USAS/v_USAS.mdl");
	PRECACHE_MODEL("models/weapons/USAS/w_USAS.mdl");
	PRECACHE_MODEL("models/weapons/USAS/p_USAS.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_buckshot.mdl");// USAS shell

	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND ("weapons/USAS/USAS_fire-1.wav");//USAS
	PRECACHE_SOUND ("weapons/USAS/USAS_fire-2.wav");//USAS

//	PRECACHE_SOUND ("weapons/USAS/USAS_reload-1.wav");	// USAS reload
//	PRECACHE_SOUND ("weapons/USAS/USAS_reload-2.wav");	// USAS reload

//	PRECACHE_SOUND ("weapons/sshell1.wav");	// USAS reload - played on client
//	PRECACHE_SOUND ("weapons/sshell3.wav");	// USAS reload - played on client
	
	PRECACHE_SOUND ("weapons/357_cock1.wav"); // gun empty sound

	PRECACHE_SOUND ("weapons/USAS/scock1.wav");	// cock gun

	m_usFireUsas = PRECACHE_EVENT( 1, "scripts/events/usas.sc" );
//	m_usDoubleFire = PRECACHE_EVENT( 1, "events/USAS2.sc" );
}

int CUsas::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
						ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Usas"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}


int CUsas::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = USAS_MAX_CLIP;
	p->iSlot = 4; //2
	p->iPosition = 1; // KELLY: esta en la posicion del gauss. Esto causa errores en MP 
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_USAS;
	p->iWeight = USAS_WEIGHT;
	p->weaponName = "USAS-12";

	return 1;
}



BOOL CUsas::Deploy( )
{
#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		// enable laser sight geometry.
		pev->body = 3;
	}
	else
	{
		pev->body = 0;
	}

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1;
	return DefaultDeploy( "models/weapons/USAS/v_USAS.mdl", "models/weapons/USAS/p_USAS.mdl", USAS_DEPLOY, "xm4", UseDecrement(), pev->body );
}

void CUsas::Holster( int skiplocal /* = 0 */ ) // we need this, because the laser-dot must be killed, else he
{ // is in the map until we use it again
m_fInReload = FALSE;// cancel any reload in progress.
#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NEVER );
		m_pSpot = NULL;
	}
#endif
}

void CUsas::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload( );
		if (m_iClip == 0)
			PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	Vector vecDir;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		vecDir = m_pPlayer->FireBulletsPlayer( 4, vecSrc, vecAiming, VECTOR_CONE_DM_SHOTGUN, 2048, BULLET_PLAYER_USAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{
		// regular old, untouched spread. 
		vecDir = m_pPlayer->FireBulletsPlayer( 6, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_USAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireUsas, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (m_iClip != 0)
		m_flPumpTime = gpGlobals->time + 0.5;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25; //() + 0.75;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25;
	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
	m_fInSpecialReload = 0;
	UpdateSpot( );
}

void CUsas::SecondaryAttack()
{
	m_fSpotActive = ! m_fSpotActive;

#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NORMAL );
		m_pSpot = NULL;
		KillLaserSight();
	}
#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}

void CUsas::Reload( void )
{
	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 20, USAS_RELOAD, 3.10 );
	else
		iResult = DefaultReload( 20, USAS_RELOAD, 3.10 );

	if (iResult)
	{
#ifndef CLIENT_DLL
	if ( m_pSpot && m_fSpotActive )
	{
		m_pSpot->Suspend( 2.1 );
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
	}
}

void CUsas::WeaponIdle( void )
{
	UpdateSpot( );
	MakeLaserSight();

	ResetEmptySound( );

//	m_iFiring = FALSE;

//test
	
/*
	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usSpriteTracer );
	ALERT (at_console, "calling spr\n");
*/
	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = USAS_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = USAS_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

void CUsas::UpdateSpot( void )
{

#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpotTEST::CreateSpotTEST();
		}

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( );;
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		
		UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );
	}
#endif
}

void CUsas::MakeLaserSight( void )
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
		
			UTIL_TraceLine( m_vecSrc, m_vecSrc + m_vecDir * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );

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

void CUsas::KillLaserSight( void )
{
if ( m_pBeam )
{
UTIL_Remove( m_pBeam );
m_pBeam = NULL;
}
}

class CUsasAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/USAS/w_shotbox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/USAS/w_shotbox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_BUCKSHOTBOX_GIVE, "buckshot", BUCKSHOT_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_usasclip, CUsasAmmo );


