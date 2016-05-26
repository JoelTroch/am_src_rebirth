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

// MP5 convetible to MP5-SD2 (sightly bastard...)

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

enum mp5_e
{
	A2_IDLE1 = 0,
	A2_IDLE2,
	A2_FIRE1,
	A2_FIRE2,
	A2_RELOAD,
	A2_RELOADLAST,
	A2_DEPLOY,
	A2_HOLSTER,
	A2_SUPRESS,
	SD2_IDLE1,
	SD2_IDLE2,
	SD2_FIRE1,
	SD2_FIRE2,
	SD2_RELOAD,
	SD2_RELOADLAST,
	SD2_DEPLOY,
	SD2_HOLSTER,
	SD2_UNSUPRESS,
	SD2_FIRE3ND,
	SD2_FIRE2RND,
	SD2_FIRE3RND,
};
LINK_ENTITY_TO_CLASS( weapon_mp5, CMP5 );
LINK_ENTITY_TO_CLASS( weapon_mp5navy, CMP5 );
LINK_ENTITY_TO_CLASS( weapon_9mmAR, CMP5 );
//=========================================================
//=========================================================
/*
	BOOL				 m_bUspSilAdd;//usp
	BOOL				 m_bMp5SilAdd;//mp5
	BOOL				 m_bM16SilAdd;//m16
*/
void CMP5::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmAR"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/MP5/w_MP5.mdl");
	m_iId = WEAPON_MP5;

    m_fDefaultAnim = MP5_DEFAULT_ANIM; //to define on startup if the silencer added or not
	m_iDefaultAmmo = 30; //SP: MP5_DEFAULT_GIVE;

//	m_bMp5SilAdd = FALSE;     // Have we been initialised

	FallInit();// get ready to fall down.
}


void CMP5::Precache( void )
{

	PRECACHE_MODEL("models/weapons/MP5/v_MP5.mdl");
	PRECACHE_MODEL("models/weapons/MP5/w_MP5.mdl");
	PRECACHE_MODEL("models/weapons/MP5/p_MP5.mdl");

	PRECACHE_MODEL("models/weapons/MP5/p_MP5_2.mdl");
	m_iShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/weapons/MP5/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/weapons/MP5/w_MP5clip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              

//	PRECACHE_SOUND("items/clipinsert1.wav");
//	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND ("weapons/MP5/mp5_fire-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/MP5/mp5_fire-2.wav");// H to the K

//	PRECACHE_SOUND ("weapons/mp51.wav");// H to the K
//	PRECACHE_SOUND ("weapons/mp52.wav");// H to the K

	PRECACHE_SOUND ("weapons/MP5/mp5_firesil-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/MP5/mp5_firesil-2.wav");// H to the K
//	PRECACHE_SOUND ("weapons/weapons/MP5hks3.wav");// H to the K

//	PRECACHE_SOUND( "weapons/glauncher.wav" );
//	PRECACHE_SOUND( "weapons/glauncher2.wav" );

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_usFireMP51 = PRECACHE_EVENT( 1, "scripts/events/mp51.sc" );
	m_usFireMP52 = PRECACHE_EVENT( 1, "scripts/events/mp52.sc" );
}

int CMP5::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1; //M203_GRENADE_MAX_CARRY;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 0;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = MP5_WEIGHT;
	p->weaponName = "MP-5 Navy";

	return 1;
}

int CMP5::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_MP5"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CMP5::Deploy( )
{
//	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3; //delay before fire
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
    
	if(m_fDefaultAnim == 1)
	{
		return DefaultDeploy("models/weapons/MP5/v_MP5.mdl", "models/weapons/MP5/p_MP5_2.mdl", SD2_DEPLOY, "xm4", 3.50f );
	}
    else
	{
		return DefaultDeploy("models/weapons/MP5/v_MP5.mdl", "models/weapons/MP5/p_MP5.mdl", A2_DEPLOY, "xm4", 3.50f ); //"mp5a4", 3.50f );
	}

	 m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

//	return DefaultDeploy("models/weapons/MP5/v_MP5.mdl", "models/weapons/MP5/p_MP5.mdl", A2_DEPLOY, "mp5a4" );
}

void CMP5::Holster( int skiplocal /* = 0 */ )
{
    m_fInReload = FALSE;// cancel any reload in progress.

 //   if(m_fDefaultAnim == 1)
 //       m_fDefaultAnim = 1;
 //   else
 //   m_fDefaultAnim = 0;

	/*
    m_fDefaultAnim = 0;
    pev->iuser3 = 0;
    */

 //  m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
    m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CMP5::SecondaryAttack( void )
{
	// Acá comprobamos que el jugador tiene o no silenciador
	// y si no lo tiene, no lo ponemos...
	if (m_pPlayer->m_bMp5SilAdd == FALSE )
	{											   //"########################################"
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Mp5_NoKit"); //digamos al cliente
		
		m_flNextSecondaryAttack = m_flNextSecondaryAttack + 2.0;//delays
		return; // return y no ejecutar más codigo.
	}
	else
	{
    m_flTimeWeaponIdle = gpGlobals->time + 9.50f; //5.50f;
    
    if(m_fDefaultAnim == 1){
    m_fDefaultAnim = 0; //Detach silencer
    SendWeaponAnim(SD2_UNSUPRESS);
    }
    else
    {
    m_fDefaultAnim = 1; //Add silencer
    SendWeaponAnim(A2_SUPRESS);
    }
    
    m_flNextSecondaryAttack = m_flNextSecondaryAttack + 9.05;//3.0
    if (m_flNextSecondaryAttack < gpGlobals->time)
    
    m_flNextPrimaryAttack = m_flNextPrimaryAttack + 9.05;//3.0 //8
    if (m_flNextPrimaryAttack < gpGlobals->time)
    return;
	}//cierre del sil
}

void CMP5::PrimaryAttack( void )
{
	  /*
	if(m_pPlayer->pev->flags & FL_DUCKING)
  {
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.y -= 1; break;
 	 case 1: m_pPlayer->pev->punchangle.y += 1; break;
    }
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.x -= 1; break;
 	 case 1: m_pPlayer->pev->punchangle.x += 1; break;
    }
  }
  else if (m_pPlayer->pev->velocity.Length() > .01)
  {
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.y -= 3; break;
 	 case 1: m_pPlayer->pev->punchangle.y += 3; break;
    }
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.x -= 3; break;
 	 case 1: m_pPlayer->pev->punchangle.x += 3; break;
    }
  }
else
  {
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.y -= 2; break;
 	 case 1: m_pPlayer->pev->punchangle.y += 2; break;
    }
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.x -= 2; break;
 	 case 1: m_pPlayer->pev->punchangle.x += 2; break;
    }
  } 
*/
#ifdef CLIENT_DLL
    if ( m_fDefaultAnim == MP5_DEFAULT_ANIM )
#else
    if ( m_fDefaultAnim == NULL )
#endif
    {
        m_fDefaultAnim = 0;//No Silencer 
        MP5Fire( 0.06, 0.08, TRUE, FALSE );
	//	 MP5Fire( 0.06, 0.1, TRUE, FALSE );
        return;
    }

    if ( m_fDefaultAnim != NULL )//Silencer added
    {
        m_fDefaultAnim = 1;
 //       MP5FireSilenced( 0.001, 0.06, TRUE, TRUE );
		 MP5FireSilenced( 0.01, 0.09, TRUE, TRUE );
		return;
    } 
}


void CMP5::Reload( void )
{
    if ( m_pPlayer->ammo_9mm <= 0 )
    return;

    int iResult;

//if(m_iClip<=29)
//{    
    if(m_fDefaultAnim != 0)
    {
	//	if (m_iClip == 0)
		iResult = DefaultReload( 30, SD2_RELOADLAST, 3.70 );
	//	else
	//	iResult = DefaultReload( 30, SD2_RELOAD, 2.45 );
    }
    else
    {
	//	if (m_iClip == 0)
		iResult = DefaultReload( 30, A2_RELOADLAST, 3.70 );
	//	else
	//	iResult = DefaultReload( 30, A2_RELOAD, 2.45 );
    }

//}
	m_pPlayer->SetAnimation( PLAYER_RELOAD ); 

 	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		
		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_rifle", vecSrc, pev->angles );
	}
}


void CMP5::WeaponIdle( void )
{
	//SysOp - UNTESTED
	
    ResetEmptySound( );
    
    m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
    
    if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
          return;
    
    if (m_iClip != 0)    
    {
    if(m_fDefaultAnim == 1)
        SendWeaponAnim( SD2_IDLE1 );
    else
    SendWeaponAnim( A2_IDLE1 );

    m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
    }
}

class CMP5AmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/MP5/w_MP5clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/MP5/w_MP5clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_MP5CLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_mp5clip, CMP5AmmoClip );
LINK_ENTITY_TO_CLASS( ammo_9mmAR, CMP5AmmoClip );



class CMP5Chainammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/MP5/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/MP5/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_CHAINBOX_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_9mmbox, CMP5Chainammo );


class CMP5AmmoGrenade : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/MP5/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/MP5/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_M203BOX_GIVE, "ARgrenades", M203_GRENADE_MAX_CARRY ) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_mp5grenades, CMP5AmmoGrenade );
LINK_ENTITY_TO_CLASS( ammo_ARgrenades, CMP5AmmoGrenade );







void CMP5::MP5Fire( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
{ 
// Aiming Mechanics
	/*
float targetx=0.312; // these are the numbers we will use for the aiming vector (X Y Z) 
float targety=0.312; // these are the numbers the will be loward accordingly to adjust the aim 
float targetz=0.312; 
// Aiming Mechanics */

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
/*
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
*/
Vector vecDir; 
//vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( targetx, targety, targetz ), 8192, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed ); 

// ### COD RECOIL START ###
	if ( m_pPlayer->pev->velocity.Length2D() <= 120 )
	{
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}																	//VECTOR_CONE_5DEGREES
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}						// 15 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
		}
	}
	else
	{
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{																//perfe 3d
			vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_9DEGREES, 2048, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_MP5, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
		}
	}
// ### COD RECOIL END ###

PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireMP51, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 ); 

m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime; 

m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); 
} 



void CMP5::MP5FireSilenced( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
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

m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME; 
m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH; 

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

//vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( targetx, targety, targetz ), 8192, BULLET_PLAYER_UZI, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed ); 
// ### COD RECOIL START ###
	if ( m_pPlayer->pev->velocity.Length2D() <= 120 )
	{
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}																	//VECTOR_CONE_5DEGREES
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}						// 15 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
		}
	}
	else
	{
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{																//perfe 3d
			vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_9DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
		}
	}
// ### COD RECOIL END ###

PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireMP52, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 ); 

m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime; 

m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); 
} 
/*
class CMp5SilAdd : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/MP5/w_mp5_extended.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/MP5/w_mp5_extended.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		ClientPrint(pOther->pev, HUD_PRINTCENTER, "#Mp5_PickupKit"); //digamos al cliente
		ALERT (at_console, "KIT DE CONVERSION DE MP5 ADQUIRIDO! \n" );	
		
		((CBasePlayer*)pOther)->m_bMp5SilAdd = TRUE;
		m_bMp5SilAdd = TRUE;
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_mp5_sil, CMp5SilAdd );
*/
