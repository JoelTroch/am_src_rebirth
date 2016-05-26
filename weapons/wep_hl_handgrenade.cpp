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
// NEW HL 2WEAPON FILE
//====================

// This weapon it's going to be used on C17's derivation. Please don't kill me Valve. There is not any
// leaked code!

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "gamerules.h"//SP: para la comprobacion del MP

#define	HANDGRENADE_PRIMARY_VOLUME		450

enum handgrenade_e {
	HANDGRENADE_IDLE = 0,
	HANDGRENADE_FIDGET,
	HANDGRENADE_PINPULL,
	HANDGRENADE_THROW1,	// toss
	HANDGRENADE_THROW2,	// medium
	HANDGRENADE_THROW3,	// hard
	HANDGRENADE_HOLSTER,
	HANDGRENADE_DRAW
};


LINK_ENTITY_TO_CLASS( weapon_handgrenade, CHL2HandGrenade );

void CHL2HandGrenade::Spawn( )
{
	Precache( );
	pev->classname = MAKE_STRING("weapon_handgrenade"); // hack to allow for old names
	m_iId = WEAPON_HL_HANDGRENADE;
	SET_MODEL(ENT(pev), "models/weapons/handgrenade/w_grenade.mdl");

#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif

	m_iDefaultAmmo = HANDGRENADE_DEFAULT_GIVE;
	m_bFamasBurst = FALSE;

	FallInit();// get ready to fall down.
}


int CHL2HandGrenade::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Grenades"); //digamos al cliente

		m_pPlayer->SetSuitUpdate("!HEV_GRENADE", FALSE, SUIT_NEXT_IN_1MIN);

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CHL2HandGrenade::Precache( void )
{
	PRECACHE_MODEL("models/weapons/handgrenade/w_grenade.mdl");
	PRECACHE_MODEL("models/weapons/handgrenade/v_grenade.mdl");
	PRECACHE_MODEL("models/weapons/handgrenade/p_grenade.mdl");
}

int CHL2HandGrenade::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Hand Grenade";
	p->iMaxAmmo1 = HANDGRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_HL_HANDGRENADE;
	p->iWeight = 10;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->weaponName = "HE";

	return 1;
}


BOOL CHL2HandGrenade::Deploy( )
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
	m_pPlayer->m_fCrosshairOff = TRUE;
	m_flReleaseThrow = -1;
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	return DefaultDeploy( "models/weapons/handgrenade/v_grenade.mdl", "models/weapons/handgrenade/p_grenade.mdl", 	HANDGRENADE_DRAW, "knife" ); //"crowbar"
}

BOOL CHL2HandGrenade::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}

void CHL2HandGrenade::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_fCrosshairOff = FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
	{
		SendWeaponAnim( HANDGRENADE_HOLSTER );
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_HL_HANDGRENADE);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void CHL2HandGrenade::PrimaryAttack()
{
	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim( HANDGRENADE_PINPULL );//test
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
				
		if (m_fGrenMode == 1) 
		if ( m_pPlayer->m_bFamasBurst == TRUE ) //es temporadizada
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#5Secs"); //digamos al cliente 5
		}

	}
}


void CHL2HandGrenade::WeaponIdle( void )
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
			CGrenade::ShootTimedPlayer( m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer) ;
		}
		else if (m_fGrenMode == 1) 
		{
			time = m_flStartThrow - gpGlobals->time + 3.0;//segundos 5
			CGrenade::ShootTimedPlayer( m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer);
		}
		else if (m_fGrenMode == 2) 
		{
			time = 23.0;
			CGrenade::ShootTimedPlayer( m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer);//fb default
		}
		
		if ( flVel < 500 )
		{
			SendWeaponAnim( HANDGRENADE_THROW1 );
		}
		else if ( flVel < 1000 )
		{
			SendWeaponAnim( HANDGRENADE_THROW2 );
		}
		else
		{
			SendWeaponAnim( HANDGRENADE_THROW3 );
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
			SendWeaponAnim( HANDGRENADE_DRAW );
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
			iAnim = HANDGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
		}
		else 
		{
			iAnim = HANDGRENADE_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim( iAnim );
	}
}

void CHL2HandGrenade::SecondaryAttack()
{

	if (m_fGrenMode == 0) 
	{
		m_fGrenMode = 1;

		m_pPlayer->m_bFamasBurst = TRUE;
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SecureOff");
		Deploy();
	}
	else if (m_fGrenMode == 1) 
	{
		m_pPlayer->m_bFamasBurst = FALSE;
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SecureOn");
		Deploy();
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7;    
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7;

	return; // return y no ejecutar más codigo.
}