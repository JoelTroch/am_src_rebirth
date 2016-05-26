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

#include "gamerules.h"//SP: para la comprobacion del MP

#define	HANDGRENADE_PRIMARY_VOLUME		450

enum handgrenade_e 
{/*
	CE_IDLE1 = 0,
	CE_IDLE2,
	CE_PINPULL,
	CE_DEPLOY,
	CE_HOLSTER,
	CE_THROW,
	CE_THROW_UH,
	*/

	HE_DEPLOY = 0,
	HE_IDLE_PIN,
	HE_PIN_PULL,
	HE_IDLE,
	HE_PREPARE,
	HE_PRIME,
	HE_THROW,
	HE_HOLSTER,
	HE_HOLSTER_PIN,

};

LINK_ENTITY_TO_CLASS( weapon_hegrenade, CHandGrenade );
LINK_ENTITY_TO_CLASS( weapon_HEgrenade, CHandGrenade );
LINK_ENTITY_TO_CLASS( weapon_handgrenade, CHandGrenade );


void CHandGrenade::Spawn( )
{
	Precache( );
	pev->classname = MAKE_STRING("weapon_handgrenade"); // hack to allow for old names
	m_iId = WEAPON_HANDGRENADE;
	SET_MODEL(ENT(pev), "models/weapons/he/w_HEgrenade.mdl");

#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif

	m_iDefaultAmmo = HANDGRENADE_DEFAULT_GIVE;
	m_bFamasBurst = FALSE;

	FallInit();// get ready to fall down.
}


int CHandGrenade::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Grenades"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CHandGrenade::Precache( void )
{
	//temp fix
	PRECACHE_SOUND ("weapons/smoke_bang.wav");
	PRECACHE_SOUND ("weapons/bang.wav");
//	PRECACHE_MODEL ("models/weapons/w_flashbang.mdl");

	PRECACHE_MODEL("models/w_grenade.mdl"); //fix
//	PRECACHE_MODEL("models/grenade.mdl"); //fix

	PRECACHE_MODEL("models/weapons/he/w_HEgrenade.mdl");
	PRECACHE_MODEL("models/weapons/he/v_HEgrenade.mdl");
	PRECACHE_MODEL("models/weapons/he/p_HEgrenade.mdl");
}

int CHandGrenade::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Hand Grenade";
	p->iMaxAmmo1 = HANDGRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 7;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_HANDGRENADE;
	p->iWeight = HANDGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->weaponName = "Granada de mano";

	return 1;
}


BOOL CHandGrenade::Deploy( )
{
	m_pPlayer->m_fCrosshairOff = TRUE;
	m_flReleaseThrow = -1;
/*
	if (m_fGrenMode == 2) 
	{
#ifndef CLIENT_DLL
	m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/cluster/v_cluster.mdl");
#else
	//LoadVModel ( "models/model.mdl", m_pPlayer );
#endif
	}
	else
	{
#ifndef CLIENT_DLL
	m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/he/v_HEgrenade.mdl");
#else
	//LoadVModel ( "models/model.mdl", m_pPlayer );
#endif
	}
*/
	if (m_fGrenMode == 2) 
	return DefaultDeploy( "models/weapons/cluster/v_cluster.mdl", "models/weapons/he/p_HEgrenade.mdl", HE_DEPLOY, "knife" ); //"crowbar"
	else
	return DefaultDeploy( "models/weapons/he/v_HEgrenade.mdl", "models/weapons/he/p_HEgrenade.mdl", HE_DEPLOY, "knife" ); //"crowbar"
}

BOOL CHandGrenade::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}

void CHandGrenade::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_fCrosshairOff = FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
	{
		SendWeaponAnim( HE_HOLSTER );
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_HANDGRENADE);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void CHandGrenade::PrimaryAttack()
{
	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim( HE_PRIME );//test
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
				
		if (m_fGrenMode == 1) 
		if ( m_pPlayer->m_bFamasBurst == TRUE ) //es temporadizada
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#5Secs"); //digamos al cliente 5
		}

	}
}


void CHandGrenade::WeaponIdle( void )
{
	if ( m_flReleaseThrow == 0 && m_flStartThrow )
		 m_flReleaseThrow = gpGlobals->time;

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_flStartThrow )
	{
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if ( angThrow.x < 0 )
			angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
		else
			angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

		float flVel = ( 90 - angThrow.x ) * 4;
		if ( flVel > 500 )
			flVel = 500;

		UTIL_MakeVectors( angThrow );

		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		float time;

		if (m_fGrenMode == 0) 
		{
			if ( m_pPlayer->m_bFamasBurst == TRUE ) //es HE temporadizada
			{
				time = m_flStartThrow - gpGlobals->time + 3.0;//segundos 5
				
				if (time < 0)
				{
					time = 0;
				}
			}
			else//es normal
			{
				time = 3;//3

				if (time < 0)
				{
					time = 0;
				}
			}
			//TEST ONLY
		}

		if (m_fGrenMode == 0) 
		{
			CGrenade::ShootTimedPlayer( m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer );
		}
		else if (m_fGrenMode == 1) 
		{
			time = m_flStartThrow - gpGlobals->time + 3.0;//segundos 5
			CGrenade::ShootTimedPlayer( m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer );
			//CGrenade::FlashShootTimed(m_pPlayer->pev, vecSrc, vecThrow, 20 );
		}
		else if (m_fGrenMode == 2) 
		{
			time = 23.0;
			CGrenade::ShootTimedSmoke( m_pPlayer->pev, vecSrc, vecThrow, time );//fb default
		}
/*		
	}
	else
	{
		CGrenade::ShootTimedPlayer( m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer );
	}
	*/
//		CGrenade::ShootTimedPlayer( m_pPlayer->pev, vecSrc, vecThrow, time );
//		CGrenade::ShootFlashbang( m_pPlayer->pev, vecSrc, vecThrow, time );

		if ( flVel < 500 )
		{
			SendWeaponAnim( HE_THROW );
		}
		else if ( flVel < 1000 )
		{
			SendWeaponAnim( HE_THROW );
		}
		else
		{
			SendWeaponAnim( HE_THROW );
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_flReleaseThrow = 0;
		m_flStartThrow = 0;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

		m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;

		if ( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;// ensure that the animation can finish playing
		}
		return;
	}
	else if ( m_flReleaseThrow > 0 )
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0;

		if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
		{
			SendWeaponAnim( HE_DEPLOY );
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_flReleaseThrow = -1;
		return;
	}

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
		if (flRand <= 0.75)
		{
			iAnim = HE_IDLE_PIN;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
		}
		else 
		{
			iAnim = HE_IDLE_PIN;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim( iAnim );
	}
}

void CHandGrenade::SecondaryAttack()
{
	/*
#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{	*/
		if (m_fGrenMode == 0) 
		{
			m_fGrenMode = 1;

			m_pPlayer->m_bFamasBurst = TRUE;
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SecureOff");
			Deploy();
		}
		else if (m_fGrenMode == 1) 
		{
			m_fGrenMode = 2;
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SmokeMode"); //digamos al cliente
			Deploy();
		}
		else if (m_fGrenMode == 2) 
		{
			m_fGrenMode = 0;

			m_pPlayer->m_bFamasBurst = FALSE;
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SecureOn");
			Deploy();
		}/*
	}
	else
	{
		if ( m_pPlayer->m_bFamasBurst == FALSE ) //es temporadizada
		{
			m_pPlayer->m_bFamasBurst = TRUE; //fb ON
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SecureOff");
			Deploy();
		}
		else// es con seguro
		{
			m_pPlayer->m_bFamasBurst = FALSE;
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SecureOn");
			Deploy();
		}
	}
*/
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7;    
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7;

	return; // return y no ejecutar más codigo.
}


