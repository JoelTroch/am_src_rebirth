/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"


#define	HANDGRENADE_PRIMARY_VOLUME		450

enum clustergrenade_e 
{
/*
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

LINK_ENTITY_TO_CLASS( weapon_clustergrenade, CClusterGrenade );



void CClusterGrenade::Spawn( )
{
	Precache( );
	pev->classname = MAKE_STRING("weapon_clustergrenade"); // hack to allow for old names
	m_iId = WEAPON_CLUSTERGRENADE;
	SET_MODEL(ENT(pev), "models/weapons/cluster/w_cluster.mdl");

#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif

	m_iDefaultAmmo = CLUSTERGRENADE_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CClusterGrenade::Precache( void )
{
	PRECACHE_MODEL("models/weapons/w_frag.mdl"); // the frag model
	PRECACHE_MODEL("models/weapons/w_cluster.mdl"); // FIX
	PRECACHE_MODEL("models/weapons/frag_gibs.mdl"); // FIX
	PRECACHE_MODEL("models/weapons/cluster/w_cluster.mdl");
	PRECACHE_MODEL("models/weapons/cluster/v_cluster.mdl");
	PRECACHE_MODEL("models/weapons/cluster/p_cluster.mdl");
}

int CClusterGrenade::GetItemInfo(ItemInfo *p)
{
p->pszName = STRING(pev->classname); //The name of the grenade
p->pszAmmo1 = "Cluster Grenade"; //The name of the ammo (?)
p->iMaxAmmo1 = CLUSTERGRENADE_MAX_CARRY; //How much can you carry?
p->pszAmmo2 = NULL; //No secondary ammo (?)
p->iMaxAmmo2 = -1; //Thus some sort of NULL value (?)
p->iMaxClip = WEAPON_NOCLIP; //This weapon doesn't use clips
p->iSlot = 7; //Weapon-slot index 0
p->iPosition = 1; //Weapon-slot position 1
p->iId = m_iId = WEAPON_CLUSTERGRENADE; //Identifier (what it is)
p->iWeight = CLUSTERGRENADE_WEIGHT; //How heavy is it
p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE; //Not sure, maybe it is limited to a certain ammount of instances (?)
p->weaponName = "Cluster Grenade";
return 1;
}



BOOL CClusterGrenade::Deploy( )
{
	m_pPlayer->m_fCrosshairOff = TRUE;
	m_flReleaseThrow = -1;
	return DefaultDeploy( "models/weapons/cluster/v_cluster.mdl", "models/weapons/cluster/p_cluster.mdl", HE_DEPLOY, "knife" ); //"crowbar"
}

BOOL CClusterGrenade::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}

void CClusterGrenade::Holster( int skiplocal /* = 0 */ )
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
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_CLUSTERGRENADE);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}

//	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void CClusterGrenade::PrimaryAttack()
{
	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim( HE_PIN_PULL );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}


void CClusterGrenade::WeaponIdle( void )
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

		// alway explode 3 seconds after the pin was pulled
//		float time = m_flStartThrow - gpGlobals->time + 15.0;
		float time = m_flStartThrow - gpGlobals->time + 3.0;
		if (time < 0)
			time = 0;
//SP - La traduccion seria que siempre explotara despues de tres segundos de sacar el
//pistillo, cambiemos esto:

		//Nah, me da fiaca, aumentemos solo el valor a 15 segs
		CGrenade::ShootTimedCluster( m_pPlayer->pev, vecSrc, vecThrow, time );

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




