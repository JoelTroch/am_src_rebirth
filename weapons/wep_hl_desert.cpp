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

SET_MODEL(ENT(pev), "sprites/laserdot.spr");
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
PRECACHE_MODEL("sprites/laserdot.spr");
};

#endif

enum deagle_e 
{
	DEAGLE_IDLE1 = 0,
	DEAGLE_IDLE2,
	DEAGLE_IDLE3,
	DEAGLE_FIRE1,
	DEAGLE_FIRE_EMPTY,

	DEAGLE_RELOAD,	
	DEAGLE_RELOAD_EMPTY,

	DEAGLE_DEPLOY,
	DEAGLE_HOLSTER,
};

LINK_ENTITY_TO_CLASS( weapon_eagle, CDeagle );

void CDeagle::Spawn( )
{
	Precache( );
	m_iId = WEAPON_HL_DEAGLE;
	SET_MODEL(ENT(pev), "models/weapons/deagle/w_deagle.mdl");

	m_iDefaultAmmo = DEAGLE_DEFAULT_GIVE;

	pev->frame = 0;
	pev->sequence = 1;
	ResetSequenceInfo( );
	pev->framerate = 0;

	m_fSpotActive = 1;

	FallInit();// get ready to fall
}


void CDeagle::Precache( void )
{
	PRECACHE_MODEL("models/weapons/deagle/v_deagle.mdl");
	PRECACHE_MODEL("models/weapons/deagle/w_deagle.mdl");
	PRECACHE_MODEL("models/weapons/deagle/p_deagle.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND ("weapons/deagle/deagle_fire-1.wav");//deagle
	PRECACHE_SOUND ("weapons/deagle/deagle_fire-2.wav");//deagle
	
	PRECACHE_SOUND ("weapons/357_cock1.wav"); // gun empty sound

	PRECACHE_SOUND ("weapons/spot_on.wav"); // gun empty sound
	PRECACHE_SOUND ("weapons/spot_off.wav"); // gun empty sound

	//shell
	PRECACHE_MODEL("models/weapons/shell_50cal.mdl");

	m_usFireDeagle = PRECACHE_EVENT( 1, "scripts/events/eagle.sc" );
}

int CDeagle::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Deagle"); //digamos al cliente

		m_pPlayer->SetSuitUpdate("!HEV_PISTOL", FALSE, SUIT_NEXT_IN_5SEC);

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}


int CDeagle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "deagle";
	p->iMaxAmmo1 = DEAGLE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_HL_DEAGLE;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}



BOOL CDeagle::Deploy( )
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
	return DefaultDeploy( "models/weapons/deagle/v_deagle.mdl", "models/weapons/deagle/p_deagle.mdl", DEAGLE_DEPLOY, "xm4", UseDecrement(), pev->body );
}

void CDeagle::Holster( int skiplocal /* = 0 */ ) // we need this, because the laser-dot must be killed, else he
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

void CDeagle::PrimaryAttack()
{
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))
    return;

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

	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_357, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireDeagle, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (m_iClip != 0)
		m_flPumpTime = gpGlobals->time + 0.5;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.55; //() + 0.75;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.55;

	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	m_fInSpecialReload = 0;
	UpdateSpot( );
}

void CDeagle::SecondaryAttack()
{
	m_fSpotActive = ! m_fSpotActive;
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/spot_on.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NORMAL );
		m_pSpot = NULL;
		KillLaserSight();
		
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/spot_off.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
	}
#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}

void CDeagle::Reload( void )
{
	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 20, DEAGLE_RELOAD_EMPTY, 1.68 );
	else
		iResult = DefaultReload( 20, DEAGLE_RELOAD, 1.68 );

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

void CDeagle::WeaponIdle( void )
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
		iAnim = DEAGLE_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = DEAGLE_IDLE2;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

void CDeagle::UpdateSpot( void )
{

#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpotTEST::CreateSpotTEST();
		}

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAiming = gpGlobals->v_forward;
                    Vector vecDest = vecSrc + vecAiming * 8192;
                    
		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecDest, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		float flLength = (tr.vecEndPos - vecSrc).Length();

		m_pSpot->pev->scale = flLength / (DESERT_LASER_FOCUS + RANDOM_FLOAT(1, 100));
		int m_iSpotBright = (1 / log(flLength / 0.3))*1700;
		if (m_iSpotBright > 255 ) m_iSpotBright = 255;

		m_iSpotBright = m_iSpotBright + RANDOM_LONG (1, flLength / 200);
		m_pSpot->pev->renderamt = m_iSpotBright;	
		UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos + tr.vecPlaneNormal * 0.1);

		//ORIG CODE
/*
		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( );;
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		
		UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );*/
	}
#endif
}

void CDeagle::MakeLaserSight( void )
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

void CDeagle::KillLaserSight( void )
{
if ( m_pBeam )
{
UTIL_Remove( m_pBeam );
m_pBeam = NULL;
}
}
/*
class CDeagleAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/deagle/w_shotbox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/deagle/w_shotbox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_BUCKSHOTBOX_GIVE, "deagle", BUCKSHOT_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_deagleclip, CDeagleAmmo );*/


