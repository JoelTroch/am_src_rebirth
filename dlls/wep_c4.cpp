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

//---------Start
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "gamerules.h"

#define C4_PRIMARY_VOLUME 500
#define C4_MAX_CARRY 5

enum c4_e 
{
	C4_IDLE1,
	C4_DRAW,
	C4_DROP,
	C4_PRESSBUTTON,
};

LINK_ENTITY_TO_CLASS( weapon_c4, Cc4 );

void Cc4::Spawn( )
{
Precache( );
m_iId = WEAPON_C4;
SET_MODEL(ENT(pev), "models/weapons/semtex/w_c4.mdl");

#ifndef CLIENT_DLL
//pev->dmg = gSkillData.plrDmgC4;
pev->dmg = gSkillData.plrDmgSatchel;
#endif

m_iDefaultAmmo = 1;

FallInit();
}
int Cc4::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Semtex"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}
void Cc4::Precache( void )
{
	PRECACHE_MODEL("models/weapons/semtex/w_c4.mdl");
	PRECACHE_MODEL("models/weapons/semtex/v_c4.mdl");
	PRECACHE_MODEL("models/weapons/semtex/p_c4.mdl");

	PRECACHE_SOUND("weapons/semtex/bomb_armed.wav");
	PRECACHE_SOUND("weapons/semtex/bomb_armed_mp.wav");
	PRECACHE_SOUND("weapons/semtex/bomb_drop.wav");
}

int Cc4::GetItemInfo(ItemInfo *p)
{
p->pszName = STRING(pev->classname);
p->pszAmmo1 = "c4";
p->iMaxAmmo1 = C4_MAX_CARRY;
p->pszAmmo2 = NULL;
p->iMaxAmmo2 = -1;
p->iMaxClip = WEAPON_NOCLIP;
p->iSlot = 7;
p->iPosition = 3;
p->iId = m_iId = WEAPON_C4;
p->iWeight = 15; //40
p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
p->weaponName = "Explosivo C4";
return 1;
}


BOOL Cc4::Deploy( )
{
m_flReleaseThrow = -1;
return DefaultDeploy( "models/weapons/semtex/v_c4.mdl", "models/weapons/semtex/p_c4.mdl", C4_DRAW, "trip" );
}

BOOL Cc4::CanHolster( void )
{
return ( m_flStartThrow == 0 );
}

void Cc4::Holster( int skiplocal /* = 0 */ )
{
m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
{
SendWeaponAnim( C4_DRAW );
}
else
{
// no more grenades!
m_pPlayer->pev->weapons &= ~(1<<WEAPON_C4);
SetThink( DestroyItem );
pev->nextthink = gpGlobals->time + 0.1;
}

EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void Cc4::PrimaryAttack()
{
	if (m_pPlayer->b_InFuncSemtexZone == TRUE)
	{
		if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
		{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim( C4_PRESSBUTTON );
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#C4_Preparing");

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.5;
		}
	}
	else
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SemtexCanOnlyUsedInSemtexZone!");
	}
}

void Cc4::WeaponIdle( void )
{
if ( m_flReleaseThrow == 0 && m_flStartThrow )
m_flReleaseThrow = gpGlobals->time;

if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
return;

if ( m_flStartThrow )
{
Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
/*
if ( angThrow.x < 0 )
angThrow.x = -10 + angThrow.x * ( ( 20 - 10 ) / 20.0 );
else
angThrow.x = -10 + angThrow.x * ( ( 20 + 10 ) / 20.0 );

float flVel = ( 20 - angThrow.x ) * 4;
*/
		if ( angThrow.x < 0 )
			angThrow.x = -10 + angThrow.x * ( ( 10 - 10 ) / 20.0 );
		else
			angThrow.x = -10 + angThrow.x * ( ( 10 + 10 ) / 20.0 );

		float flVel = ( 10 - angThrow.x ) * 4;

if ( flVel > 5 )

flVel = 1;

UTIL_MakeVectors( angThrow );

Vector vecSrc = m_pPlayer->pev->origin;

Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		
//m_pPlayer->EnableControl(FALSE);//test

/*
flVel = 5;

UTIL_MakeVectors( angThrow );

Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
*/
#ifdef CLIENT_DLL
	if ( !bIsMultiplayer() )
#else
	if ( !g_pGameRules->IsMultiplayer() )
#endif
		/*
	{
		float time = m_flStartThrow - gpGlobals->time + 45.0; //45!!! yep
		if (time < 0)
		time = 0;
			
		EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/semtex/bomb_armed.wav", 0.99, ATTN_NORM);
		
		CGrenade::ShootC4( m_pPlayer->pev, vecSrc, vecThrow, time );
	}
	else//mp
	*/
	{ 
		float time = m_flStartThrow - gpGlobals->time + 10.0; //15!!! quickly
		if (time < 0)
		time = 0;

		EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/semtex/bomb_armed_mp.wav", 0.99, ATTN_NORM);

		CGrenade::ShootC4( m_pPlayer->pev, vecSrc, vecThrow, time );
	
	//	m_pPlayer->EnableControl(TRUE);//test
	}

//CGrenade::ShootTimed( m_pPlayer->pev, vecSrc, vecThrow, time );

if ( flVel < 1 ) //5
{
SendWeaponAnim( C4_DROP );
}
else if ( flVel < 1 ) //10
{
SendWeaponAnim( C4_DROP );
}
else
{
SendWeaponAnim( C4_DROP );
}
ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#C4_Planted!");
m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

m_flReleaseThrow = 0;
m_flStartThrow = 0;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/semtex/bomb_drop.wav", 0.99, ATTN_NORM);
//				channel	voice! doesn't ignore the bip sound!
m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;

if ( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
{
m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
}
return;
}
else if ( m_flReleaseThrow > 0 )
{
m_flStartThrow = 0;

if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
{
SendWeaponAnim( C4_DRAW );
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
iAnim = C4_IDLE1;
m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}
else 
{
iAnim = C4_IDLE1;
m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
}

SendWeaponAnim( iAnim );
}
}

//---------End