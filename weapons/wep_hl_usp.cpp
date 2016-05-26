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
#include "soundent.h"
#include "gamerules.h"

extern int gmsgClcommand;

enum hl2glock_e {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CHL2Pistol );

void CHL2Pistol::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_HL_GLOCK;
	SET_MODEL(ENT(pev), "models/weapons/glock/w_9mmhandgun.mdl");

	m_iDefaultAmmo = 15;

	FallInit();// get ready to fall down.
}

int CHL2Pistol::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Glock"); //digamos al cliente
			
		m_pPlayer->SetSuitUpdate("!HEV_PISTOL", FALSE, SUIT_NEXT_IN_5SEC);

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CHL2Pistol::Precache( void )
{
	PRECACHE_MODEL("models/weapons/glock/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/weapons/glock/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/weapons/glock/p_9mmhandgun.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND ("weapons/glock/glock_fire-1.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/glock/glock_fire-2.wav");//silenced handgun

	m_usFirePistol = PRECACHE_EVENT( 1, "scripts/events/glock.sc" );
}

int CHL2Pistol::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 15;
	p->iSlot = 1;
	p->iPosition = 0;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_HL_GLOCK;
	p->iWeight = 10;
	p->weaponName = "USP";

	return 1;
}

BOOL CHL2Pistol::Deploy( )
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	return DefaultDeploy( "models/weapons/glock/v_9mmhandgun.mdl", "models/weapons/glock/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CHL2Pistol::SecondaryAttack( void )
{
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))
    return;
	GlockFire( 0.9, 0.9, FALSE ); //( 0.1, 0.2, FALSE );
	return;
}

void CHL2Pistol::PrimaryAttack( void )
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

void CHL2Pistol::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim ) 
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

PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFirePistol : m_usFirePistol, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 ); 

m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime; 

if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0) 
// HEV suit - indicate out of ammo condition 
m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
/*
// HEV suit - indicate warning ammo condition 
if (m_iClip == 4)
m_pPlayer->SetSuitUpdate("!HEV_AMMO_WAR0", FALSE, 0); */

m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); 
} 


void CHL2Pistol::Reload( void )
{
	if ( m_pPlayer->ammo_9mm <= 0 )
		 return;

	int iResult;

	if (m_iClip == 0)
	{
		iResult = DefaultReload( 15, GLOCK_RELOAD, 2.6 );//13
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
	}
	else
	{
		iResult = DefaultReload( 15, GLOCK_RELOAD_NOT_EMPTY, 2.4 );
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

void CHL2Pistol::WeaponIdle( void )
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

class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/glock/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/glock/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY ) != -1)
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pOther;

			pPlayer->SetSuitUpdate("!HEV_9MM", FALSE, SUIT_NEXT_IN_5SEC);

			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );








