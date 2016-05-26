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

//extern cvar_t oldmodels;

enum deagle_e 
{
	DEAGLE_IDLE1 = 0,
	DEAGLE_IDLE2,
	DEAGLE_FIRE1,
	DEAGLE_FIRE2,
	DEAGLE_FIRE1_VACIO,
	DEAGLE_FIRE2_VACIO,
	DEAGLE_RELOAD,
	DEAGLE_RELOAD_VACIO,
	DEAGLE_DRAW,
	DEAGLE_HOLSTER,
	DEAGLE_DRAW_VACIO,
	DEAGLE_HOLSTER_VACIO,
	DEAGLE_IDLE1_VACIO,
	DEAGLE_IDLE2_VACIO,
};

LINK_ENTITY_TO_CLASS( weapon_deagle, CDeagle );
LINK_ENTITY_TO_CLASS( weapon_nigthhank, CDeagle );


void CDeagle::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_deagle"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/weapons/deagle/w_deagle.mdl");

	m_iDefaultAmmo = DEAGLE_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CDeagle::Precache( void )
{
	PRECACHE_MODEL("models/weapons/deagle/v_deagle.mdl");
//		PRECACHE_MODEL("models/weapons/deagle/v_deagle_2.mdl"); //sys test
	PRECACHE_MODEL("models/weapons/deagle/w_deagle.mdl");
	PRECACHE_MODEL("models/weapons/deagle/p_deagle.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_50cal.mdl");// brass shell

	PRECACHE_SOUND("weapons/deagle/deagle_fire-1.wav");

	m_usFireDeagle = PRECACHE_EVENT( 1, "scripts/events/deagle.sc" );
//	m_usFireDeagle2 = PRECACHE_EVENT( 1, "events/deagle2.sc" );
}

int CDeagle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ammo_deagle";
	p->iMaxAmmo1 = DEAGLE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iWeight = 14; //DEAGLE_WEIGHT;
	p->weaponName = "Desert Eagle";

	return 1;
}

int CDeagle::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Deagle"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CDeagle::Deploy( )
{
	// pev->body = 1;
	return DefaultDeploy( "models/weapons/deagle/v_deagle.mdl", "models/weapons/deagle/p_deagle.mdl", DEAGLE_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CDeagle::PrimaryAttack( void )
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))
    return;
	// ### COD RECOIL START ###
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				DeagleFire( 0, 0.2, TRUE ); //SPREAD CERO
			}
			else // si no es porq esta parado
			{																//perfe 3d
				DeagleFire( 0.01, 0.2, TRUE );
			}
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				DeagleFire( 0.1, 0.2, TRUE );
			}						// 14 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				DeagleFire( 0.1, 0.2, TRUE );
			}
		}
// ### COD RECOIL END ###
  // 	DeagleFire( 0, 0.2, TRUE ); //GlockFire( 0.01, 0.3, TRUE );
	return;
}

void CDeagle::DeagleFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	}

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	if ( fUseAutoAim )
	{
		vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	}
	else
	{
		vecAiming = gpGlobals->v_forward;
	}

	Vector vecDir;
	//vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_DEAGLE, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	
	vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_DEAGLE, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireDeagle  : m_usFireDeagle, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CDeagle::Reload( void )
{
//   DefaultReload( 7, DEAGLE_RELOAD, 2.6 );
//	if ( m_pPlayer->ammo_deagle <= 0 )
//		 return;
	//m_pPlayer->SetAnimation( PLAYER_RELOAD );

	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 7, DEAGLE_RELOAD_VACIO, 3.45 );
	else
		iResult = DefaultReload( 7, DEAGLE_RELOAD, 2.55 );

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
			
		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_pistol", vecSrc, pev->angles );		
	}
}


void CDeagle::WeaponIdle( void )
{
	/*
	//sys test	
	float fOldModels = CVAR_GET_FLOAT("mp_am_oldmodels");

	if (fOldModels) //no es cero esta _Activado_

	if ( CVAR_GET_FLOAT( "cl_oldmodels" ) == 1 )//sys
	{
		if ( b_Deagle2Sending == FALSE)
		{
			#ifndef CLIENT_DLL
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/deagle/v_deagle_2.mdl"); // pongamos el circulo negro
			#else
					LoadVModel ( "models/weapons/deagle/v_deagle_2.mdl", m_pPlayer ); //carguemos el model al jugador
			#endif							
				
			ALERT( at_console, "> Black Deagle Loaded !\n");
			b_Deagle2Sending = TRUE;
		}

	}
	else
	{
		if ( b_Deagle1Sending == FALSE)
		{
			#ifndef CLIENT_DLL
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/deagle/v_deagle.mdl"); // pongamos el circulo negro
			#else
					LoadVModel ( "models/weapons/deagle/v_deagle.mdl", m_pPlayer ); //carguemos el model al jugador
			#endif
					
			b_Deagle1Sending = TRUE;
		}

	}
	*/

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











/*
void CDeagle::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}

		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}

		else
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}

		SendWeaponAnim( iAnim, 1 );
		
	}
}
*/






class CDeagleAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/deagle/w_deagleclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/deagle/w_deagleclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( DEAGLE_DEFAULT_GIVE, "ammo_deagle", DEAGLE_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_deagle, CDeagleAmmo );
LINK_ENTITY_TO_CLASS( ammo_50cal, CDeagleAmmo );















