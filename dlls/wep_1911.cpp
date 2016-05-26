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

enum glock_e 
{
	GLOCK_IDLE1 = 0,
	GLOCK_RELOAD,
	GLOCK_RELOAD_LAST,
	GLOCK_FIRE1,
	GLOCK_FIRE2,
	GLOCK_FIRE_LAST,
	GLOCK_DEPLOY,
	GLOCK_DEPLOY_VACIO,
	GLOCK_HOLSTER,
	GLOCK_HOLSTER_VACIO,
};

LINK_ENTITY_TO_CLASS( weapon_1911, CGlock );
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock );

void CGlock::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_GLOCK;
	SET_MODEL(ENT(pev), "models/weapons/1911/w_9mmhandgun.mdl");

	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

int CGlock::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_1911"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CGlock::Precache( void )
{
	PRECACHE_MODEL("models/weapons/1911/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/weapons/1911/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/weapons/1911/p_9mmhandgun.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND ("weapons/1911/1911_fire-1.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/1911/1911_fire-2.wav");//silenced handgun
//	PRECACHE_SOUND ("weapons/1911/pl_gun3.wav");//handgun

	m_usFireGlock1 = PRECACHE_EVENT( 1, "scripts/events/glock1.sc" );
	m_usFireGlock2 = PRECACHE_EVENT( 1, "scripts/events/glock2.sc" );
}

int CGlock::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;
	p->weaponName = "Colt 1911 A2";

	return 1;
}

BOOL CGlock::Deploy( )
{
	// pev->body = 1;
	return DefaultDeploy( "models/weapons/1911/v_9mmhandgun.mdl", "models/weapons/1911/p_9mmhandgun.mdl", GLOCK_DEPLOY, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CGlock::SecondaryAttack( void )
{
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))
    return;
	GlockFire( 0.9, 0.9, FALSE ); //( 0.1, 0.2, FALSE );
	return;
}

void CGlock::PrimaryAttack( void )
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
   	GlockFire( 0.02, 0.06, TRUE ); //GlockFire( 0.01, 0.3, TRUE );
	return;
}

void CGlock::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim ) 
{ 
// Aiming Mechanics 
float targetx=0.312; // these are the numbers we will use for the aiming vector (X Y Z) 
float targety=0.312; // these are the numbers the will be loward accordingly to adjust the aim 
float targetz=0.312; 
// Aiming Mechanics 
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
m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; 
m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH; 
} 

Vector vecSrc = m_pPlayer->GetGunPosition( ); 
Vector vecAiming; 

if ( fUseAutoAim ) 
{ 
vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES ); 
} 
else 
{ 
vecAiming = gpGlobals->v_forward; 
} 

// Aiming Mechanics 
if(!(m_pPlayer->pev->button & (IN_FORWARD|IN_BACK))) //test to see if you are moving forward or back 
{ 
targetx-=0.090; //if you are not moving forward or back then we lower these numbers 
targety-=0.132; 
targetz-=0.090; 
} 
else 
{ 
targetx-=0.058; //if you are moving forward or back then we lower these numbers 
targety-=0.018; //notice the diffrence in the values from the code above 
targetz-=0.058; 
} 

if(!(m_pPlayer->pev->button & (IN_MOVELEFT|IN_MOVERIGHT))) //test to see if you are moving left or right 
{ 
targetx-=0.132; //do not mistake the above test for looking left or right this test is for straifing not turning 
targety-=0.090; // these values are almost the same as the above only we alter the x more then y and z 
targetz-=0.090; 
} 
else 
{ 
targetx-=0.018; 
targety-=0.058; 
targetz-=0.058; 
} 
if((m_pPlayer->pev->button & (IN_DUCK))) //this test checks if you are crouched 
{ 
targetx-=0.090; //the values here are only slightly diffrent from the above here we alter the z more then anything 
targety-=0.090; 
targetz-=0.132; 
} 
else 
{ 
targetx-=0.020; 
targety-=0.020; 
targetz-=0.020; 
} 
// Aiming Mechanics 

Vector vecDir; 
vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( targetx, targety, targetz ), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed ); 

PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 ); 

m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime; 

m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); 
} 


void CGlock::Reload( void )
{
	if ( m_pPlayer->ammo_9mm <= 0 )
		 return;

	int iResult;

	if (m_iClip == 0)
	{
		iResult = DefaultReload( 8, GLOCK_RELOAD_LAST, 2.6 );//13
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
	}
	else
	{
		iResult = DefaultReload( 8, GLOCK_RELOAD, 2.4 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
	}

	// player "shoot" animation 


	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
			Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_pistol", vecSrc, pev->angles );		

	}
}




void CGlock::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = GLOCK_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = GLOCK_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}











/*
void CGlock::WeaponIdle( void )
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






class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/1911/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/1911/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
//LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo );
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );















