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

// Super weapon of massive destruction 

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

enum M249_e
{
	M249_IDLE1 = 0,
	M249_IDLE_VACIO,
	M249_FIRE1,
	M249_FIRE2,
	M249_FIRE3,

	M249_FIRE_10,
	M249_FIRE_9,
	M249_FIRE_8,
	M249_FIRE_7,
	M249_FIRE_6,
	M249_FIRE_5,
	M249_FIRE_4,
	M249_FIRE_3,
	M249_FIRE_2,
	M249_FIRE_1,
	M249_FIRE_LAST,
	M249_RELOAD,
	M249_RELOAD_VACIO,
	M249_RELOAD_NMC,
	M249_RELOAD_NMC_VACIO,
	M249_DEPLOY,
	M249_HOLSTER,
	M249_DEPLOY_VACIO,
	M249_HOLSTER_VACIO,
	M249_DEPLOY_FIRST,
};

LINK_ENTITY_TO_CLASS( weapon_M249, CM249 );


//=========================================================
//=========================================================

void CM249::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_M249"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/m249/w_M249.mdl");
	m_iId = WEAPON_M249;

	m_iDefaultAmmo = M249_DEFAULT_AMMO;

	FallInit();// get ready to fall down.
}


void CM249::Precache( void )
{

	PRECACHE_MODEL("models/weapons/m249/v_M249.mdl");
	PRECACHE_MODEL("models/weapons/m249/w_M249.mdl");
	PRECACHE_MODEL("models/weapons/m249/p_M249.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_M249.mdl");// brass shellTE_MODEL

	PRECACHE_SOUND ("weapons/M249/M249_fire-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/M249/M249_fire-2.wav");// H to the K

	m_usM249 = PRECACHE_EVENT( 1, "scripts/events/m249.sc" );
}

int CM249::GetItemInfo(ItemInfo *p)
{
    p->pszName   = STRING(pev->classname);
    p->pszAmmo1  = "ammo_M249";              // The type of ammo it uses
    p->iMaxAmmo1 = M249_MAX_AMMO;            // Max ammo the player can carry
    p->pszAmmo2  = NULL;                    // No secondary ammo
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = M249_DEFAULT_AMMO;        // The clip size
    p->iSlot     = 6;						 // The number in the HUD
    p->iPosition = 0;						  // The position in a HUD slot
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
    p->iId       = m_iId = WEAPON_M249;      // The weapon id
    p->iWeight   = M249_WEIGHT; 
	p->weaponName = "M-249";

	return 1;
}

int CM249::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_m249"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CM249::Deploy( )
{	
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	//m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.1;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1;

	return DefaultDeploy( M249_MODEL_1STPERSON, M249_MODEL_3RDPERSON, M249_DEPLOY_FIRST, "xm4" ); //"mp5"
}

void CM249::Holster( int skiplocal /* = 0 */ )
{
	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_iClip)
		SendWeaponAnim( M249_HOLSTER );
	else
		SendWeaponAnim( M249_HOLSTER );
}

void CM249::PrimaryAttack()
{
//	if ( m_pPlayer->pev->velocity.Length2D() <= 220 )
//	{
//		return;				
//		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#M249_DuckForShoot"); //digamos al cliente
//	}

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

			m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
			m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

			m_iClip--;

			m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	#ifndef CLIENT_DLL
	// On fait un ScreenShake
									//AMPLITUDE//FREQUENCY//DURATION
	UTIL_ScreenShakeOne( m_pPlayer, 2, 4, 0.2 );
	#endif

			Vector vecSrc	 = m_pPlayer->GetGunPosition( );
			Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
			Vector vecDir;

		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_M249, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_15DEGREES, 8192, BULLET_PLAYER_M249, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
#if SYS			
				m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - gpGlobals->v_forward * 8 * 15; //*8 * 5
				//decir que se agache pa' disparar
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#M249_DuckForShoot"); //digamos al cliente
#endif		
			}
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_15DEGREES, 8192, BULLET_PLAYER_M249, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - gpGlobals->v_forward * 8 * 3; //*8 * 5
			ALERT ( at_console, "Firing whit 20 recoil # aire #  :( \n" );
		}
			// single player spread
	
	//	vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_M249, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

			//*8 * 5
			// punch
	//		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
	//		{	
		//		m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - gpGlobals->v_forward * 8 * 6;  //3
	//		}
	//		else // si no esta en el suelo es porque esta en el aire
	///		{
				//ALERT ( at_console, "FIX: in air, punch has no effect! \n" );
	//		}

		  int flags;
		#if defined( CLIENT_WEAPONS )
			flags = FEV_NOTHOST;
		#else
			flags = 0;
		#endif

			PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usM249, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1; // 0.1; delay

			if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1; // 0.1; delay

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
//		}
//	}
//	else
//	{
//		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#M249_CantShoot"); //digamos al cliente
//	}
}

void CM249::Reload( void )
{
	int iResult;

//	if (m_iClip == 0)
		iResult = DefaultReload( 100, M249_RELOAD_VACIO, 8.00 );
//	else
//		iResult = DefaultReload( 100, M249_RELOAD, 8.00 );

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
	}
	//m_pPlayer->SetAnimation( PLAYER_RELOAD );
}


void CM249::WeaponIdle( void )
{
	/*
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = M249_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = M249_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
*/
}


class CM249AmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/m249/w_M249clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/m249/w_M249clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( M249_DEFAULT_AMMO, "ammo_M249", M249_MAX_AMMO) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_M249, CM249AmmoClip );
//LINK_ENTITY_TO_CLASS( ammo_9mmAR, M249AmmoClip );

