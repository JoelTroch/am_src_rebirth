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
#include "soundent.h"
#include "gamerules.h"

enum m16_e
{
M16_SIL_IDLE1 = 0,
M16_SIL_IDLE2,
M16_SIL_FIRE1,
M16_SIL_FIRE2,
M16_SIL_RELOAD1,
M16_SIL_RELOAD2,
M16_SIL_RELOAD_VACIO,
M16_SIL_RELOAD_NMC1,
M16_SIL_RELOAD_NMC2,
M16_SIL_RELOAD_NMC_VACIO,
M16_SIL_DEPLOY_FIRST,
M16_SIL_DEPLOY,
M16_SIL_HOLSTER,

M16_SILENCER_ON,

M16_IDLE1,
M16_IDLE2,
M16_FIRE1,
M16_FIRE2,
M16_RELOAD1,
M16_RELOAD2,
M16_RELOAD_VACIO,
M16_RELOAD_NMC1,
M16_RELOAD_NMC2,
M16_RELOAD_NMC_VACIO,
M16_DEPLOY_FIRST,
M16_DEPLOY,
M16_HOLSTER,

M16_SILENCER_OFF,
};

//LINK_ENTITY_TO_CLASS( weapon_m16, CM16 );
LINK_ENTITY_TO_CLASS( weapon_m4a1, CM16 );

//=========================================================
//=========================================================

void CM16::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_m4a1"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/m4a1/w_m4a1.mdl");
	m_iId = WEAPON_M16;

	m_iDefaultAmmo = M16_DEFAULT_AMMO;
	m_fDefaultAnim = MP5_DEFAULT_ANIM; //to define on startup if the silencer added or not

//	m_bM16SilAdd = FALSE;     // Have we been initialised

	FallInit();// get ready to fall down.
}


void CM16::Precache( void )
{

	PRECACHE_MODEL("models/weapons/m4a1/v_m4a1.mdl");
	PRECACHE_MODEL("models/weapons/m4a1/w_m4a1.mdl");
	PRECACHE_MODEL("models/weapons/m4a1/p_m4a1.mdl");
	PRECACHE_MODEL("models/weapons/m4a1/p_m4a1_2.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_556.mdl");// brass shellTE_MODEL

	PRECACHE_SOUND ("weapons/m4a1/m4a1_fire-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/m4a1/m4a1_fire-2.wav");// H to the K

	PRECACHE_SOUND ("weapons/m4a1/m4a1_firesil-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/m4a1/m4a1_firesil-2.wav");// H to the K

	m_usFireM161 = PRECACHE_EVENT( 1, "scripts/events/m161.sc" );
	m_usFireM162 = PRECACHE_EVENT( 1, "scripts/events/m162.sc" );
}

int CM16::GetItemInfo(ItemInfo *p)
{
    p->pszName   = STRING(pev->classname);
    p->pszAmmo1  = "556";              // The type of ammo it uses
    p->iMaxAmmo1 = _556_MAX_AMMO;            // Max ammo the player can carry
    p->pszAmmo2  = NULL;                    // No secondary ammo
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = M16_DEFAULT_AMMO;        // The clip size
    p->iSlot     = 3;						 // The number in the HUD
    p->iPosition = 2;						  // The position in a HUD slot
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
    p->iId       = m_iId = WEAPON_M16;      // The weapon id
    p->iWeight   = M16_WEIGHT; 
	p->weaponName = "Colt M4A1";

	return 1;
}

int CM16::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_M4A1"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CM16::Deploy( )
{
//	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3; //delay before fire
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
    
	if(m_fDefaultAnim == 1)
	{
	return DefaultDeploy("models/weapons/m4a1/v_m4a1.mdl", "models/weapons/m4a1/p_m4a1_2.mdl", M16_SIL_DEPLOY, "m16", 2.50f );
	}
    else
	{
		return DefaultDeploy("models/weapons/m4a1/v_m4a1.mdl", "models/weapons/m4a1/p_m4a1.mdl", M16_DEPLOY, "m16", 2.50f );
	}

	 m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

//	return DefaultDeploy("models/weapons/MP5/v_MP5.mdl", "models/weapons/MP5/p_MP5.mdl", A2_DEPLOY, "mp5a4" );
}

void CM16::Holster( int skiplocal /* = 0 */ )
{
    m_fInReload = FALSE;// cancel any reload in progress.

 //  m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
    m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CM16::SecondaryAttack( void )
{
	// Acá comprobamos que el jugador tiene o no silenciador
	// y si no lo tiene, no lo ponemos...
	if (m_pPlayer->m_bM16SilAdd == FALSE )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#No_Silencer"); //digamos al cliente
		
		m_flNextSecondaryAttack = m_flNextSecondaryAttack + 2.0;//delays
		return; // return y no ejecutar más codigo.
	}
	else
	{

    m_flTimeWeaponIdle = gpGlobals->time + 5.50f; //5.50f;
    
    if(m_fDefaultAnim == 1){
    m_fDefaultAnim = 0; //Detach silencer
    SendWeaponAnim(M16_SILENCER_OFF);
    }
    else
    {
    m_fDefaultAnim = 1; //Add silencer
    SendWeaponAnim(M16_SILENCER_ON);
    }
    
    m_flNextSecondaryAttack = m_flNextSecondaryAttack + 4.15;//3.0
    if (m_flNextSecondaryAttack < gpGlobals->time)
    
    m_flNextPrimaryAttack = m_flNextPrimaryAttack + 4.15;//3.0 //8
    if (m_flNextPrimaryAttack < gpGlobals->time)
    return;
	}//cierre del sil
}

void CM16::PrimaryAttack( void )
{
#ifdef CLIENT_DLL
    if ( m_fDefaultAnim == MP5_DEFAULT_ANIM )
#else
    if ( m_fDefaultAnim == NULL )
#endif
    {
        m_fDefaultAnim = 0;//No Silencer 
   //     if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) //only one shoot by mousclick
    //    return;
// ### COD RECOIL START ###
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				M16Fire( 0.01, 0.09, TRUE, FALSE );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				M16Fire( 0.05, 0.09, TRUE, FALSE );
			}
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				M16Fire( 0.4, 0.09, TRUE, FALSE );
			}						// 14 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				M16Fire( 0.4, 0.09, TRUE, FALSE );
			}
		}
// ### COD RECOIL END ###
        //M16Fire( 0.01, 0.10, TRUE, FALSE );
       return;
    }

    if ( m_fDefaultAnim != NULL )//Silencer added
    {
        m_fDefaultAnim = 1;
// ### COD RECOIL START ###
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				M16FireSilenced( 0.001, 0.09, TRUE, TRUE );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				M16FireSilenced( 0.02, 0.09, TRUE, TRUE );
			}
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				M16FireSilenced( 0.2, 0.09, TRUE, TRUE );
			}						// 14 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				M16FireSilenced( 0.2, 0.09, TRUE, TRUE );
			}
		}
// ### COD RECOIL END ###
     //   M16FireSilenced( 0.01, 0.09, TRUE, TRUE );
		//return;
    }
        return;
}
// this function only gets called if silenced
void CM16::M16FireSilenced( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
{
    if (m_pPlayer->pev->waterlevel == 3)
    {
        PlayEmptySound( );
        m_flNextPrimaryAttack = 0.15;
        return;
    }

    m_fDefaultAnim = Silenced;//Nur zu sicherheit

    if (m_iClip == 1)
    {
        SendWeaponAnim( M16_SIL_FIRE2 );
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
    }

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
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
    
    Vector vecSrc     = m_pPlayer->GetGunPosition( );
    Vector vecAiming;
    
    if ( fUseAutoAim )
    {
        vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
    }
    else
    {
        vecAiming = gpGlobals->v_forward;
    }

    Vector vecDir;																							//BULLET_PLAYER_45
    vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 5096, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireM162, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 0:0, 0 );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

 	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CM16::M16Fire( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
{
    
    if (m_pPlayer->pev->waterlevel == 3)
    {
        PlayEmptySound( );
        m_flNextPrimaryAttack = 0.15;
        return;
    }

    m_fDefaultAnim = Silenced;
    
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

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

    Vector vecSrc     = m_pPlayer->GetGunPosition( );
    Vector vecAiming;
    
    if ( fUseAutoAim )
    {
        vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
    }
    else
    {
        vecAiming = gpGlobals->v_forward;
    }

    Vector vecDir, vecDir1;																				// BULLET_PLAYER_45
    vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 5096, BULLET_PLAYER_M16, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireM161, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 0:0, 0 );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CM16::Reload( void )
{
    if ( m_pPlayer->ammo_556 <= 0 )
    return;

    int iResult;

//if(m_iClip<=29)//?????????????
//{    
    if(m_fDefaultAnim != 0)
    {
	//	if (m_iClip == 0)
	//	{
			iResult = DefaultReload( 30, M16_SIL_RELOAD_NMC_VACIO, 2.70 );
	//	}
	//	else
	//	{
	//		switch (RANDOM_LONG(0,1))
	//		{
	//		case 0: iResult = DefaultReload( 30, M16_SIL_RELOAD_NMC1, 2.45 );
	//			break;
	//		case 1: iResult = DefaultReload( 30, M16_SIL_RELOAD_NMC2, 2.45 );
	//			break;
	//		}
	//	}
    }
    else
    {
	//	if (m_iClip == 0)
		iResult = DefaultReload( 30, M16_RELOAD_NMC_VACIO, 2.70 );
	//	else
	//	{
	//		switch (RANDOM_LONG(0,1))
	//		{
	//		case 0: iResult = DefaultReload( 30, M16_RELOAD_NMC1, 2.40 );
	//			break;
	//		case 1: iResult = DefaultReload( 30, M16_RELOAD_NMC2, 2.40 );
	//			break;
	//		}
	//	}
    }

//}

 	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
				
		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_rifle", vecSrc, pev->angles );
	}
}


void CM16::WeaponIdle( void )
{
	/*
    ResetEmptySound( );
    
    m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
    
    if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
          return;
    
    if (m_iClip != 0)    
    {
    if(m_fDefaultAnim == 1)
        SendWeaponAnim( M16_SIL_IDLE1 );
    else
    SendWeaponAnim( M16_IDLE1 );

    m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
    }
	*/
}


class CM16AmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/m4a1/w_m4a1clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/m4a1/w_m4a1clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( M16_DEFAULT_AMMO, "556", _556_MAX_AMMO) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_m4a1, CM16AmmoClip );
/*
class CM16SilAdd : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/m4a1/w_m4a1_extended.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/m4a1/w_m4a1_extended.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		ClientPrint(pOther->pev, HUD_PRINTCENTER, "#Pickup_m4a1Silencer"); //digamos al cliente
		ALERT (at_console, "KIT DE SILENCIADOR DE M4A1 ADQUIRIDO! \n" );	

		((CBasePlayer*)pOther)->m_bM16SilAdd = TRUE;
		m_bM16SilAdd = TRUE;
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_m4a1_sil, CM16SilAdd );
*/