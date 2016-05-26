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

#define SILENCED_GUN_VOLUME		64
#define	SMALEST_GUN_FLASH		64

enum UZI_e
{
	UZI_SIL_IDLE1 = 0,
	UZI_SIL_IDLE2,
	UZI_SIL_FIRE1,
	UZI_SIL_FIRE2,
	UZI_SIL_DEPLOY,
	UZI_SIL_HOLSTER,
	UZI_SIL_RELOAD,
	UZI_SIL_RELOAD_VACIO,

	UZI_SIL_ON,
	UZI_SIL_OFF,

	UZI_IDLE1,
	UZI_IDLE2,
	UZI_FIRE1,
	UZI_FIRE2,
	UZI_DEPLOY,
	UZI_HOLSTER,
	UZI_RELOAD,
	UZI_RELOAD_VACIO,
};

LINK_ENTITY_TO_CLASS( weapon_uzi, CUZI );

//=========================================================
//=========================================================
void CUZI::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_uzi"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/uzi/w_uzi.mdl");
	m_iId = WEAPON_UZI;

	m_iDefaultAmmo = MP5_DEFAULT_GIVE; // 250 ammo

//	m_bUZISilAdd = FALSE;     // Have we been initialised
	FallInit();// get ready to fall down.
}


void CUZI::Precache( void )
{
	PRECACHE_MODEL("models/weapons/uzi/v_uzi.mdl");
	PRECACHE_MODEL("models/weapons/uzi/w_uzi.mdl");
	PRECACHE_MODEL("models/weapons/uzi/p_uzi.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");// brass shellTE_MODEL

	PRECACHE_SOUND ("weapons/UZI/UZI_fire-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/UZI/UZI_fire-2.wav");// H to the K

	PRECACHE_SOUND ("weapons/UZI/UZI_fireSIL-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/UZI/UZI_fireSIL-2.wav");// H to the K

	PRECACHE_SOUND ("weapons/pistol_reload.wav");

	m_usFireUZI = PRECACHE_EVENT( 1, "scripts/events/UZI.sc" );
	m_usFireUZI_SIL = PRECACHE_EVENT( 1, "scripts/events/UZI_sil.sc" );
}

int CUZI::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1; //M203_GRENADE_MAX_CARRY;
	p->iMaxClip = 30; //CLIP
	p->iSlot = 2;
	p->iPosition = 2;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_UZI;
	p->iWeight = UZI_WEIGHT;
	p->weaponName = "IMI UZI";

	return 1;
}

int CUZI::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Uzi"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();

		return TRUE;
	}
	return FALSE;
}

BOOL CUZI::Deploy( )
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
    
	if(m_fDefaultAnim == 1)
	{
		return DefaultDeploy("models/weapons/uzi/v_uzi.mdl", "models/weapons/uzi/p_uzi.mdl", UZI_SIL_DEPLOY, "uzi", 3.50f );
	}
    else
	{
		return DefaultDeploy("models/weapons/uzi/v_uzi.mdl", "models/weapons/uzi/p_uzi.mdl", UZI_DEPLOY, "uzi", 3.50f );
	}

	 m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	return DefaultDeploy("models/weapons/uzi/v_UZI.mdl", "models/weapons/uzi/p_UZI.mdl", UZI_SIL_DEPLOY, "uzi" );
}

void CUZI::Holster( int skiplocal /* = 0 */ )
{
	m_fInReload = FALSE;// cancel any reload in progress.
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5; //0.5
	if (m_iClip)
		SendWeaponAnim( UZI_SIL_HOLSTER );
	else
		SendWeaponAnim( UZI_SIL_HOLSTER );
}
void CUZI::SecondaryAttack( void )
{
	// Acá comprobamos que el jugador tiene o no silenciador
	// y si no lo tiene, no lo ponemos...
	if (m_pPlayer->m_bUZISilAdd == FALSE )
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
    SendWeaponAnim(UZI_SIL_ON);
    }
    else
    {
    m_fDefaultAnim = 1; //Add silencer
    SendWeaponAnim(UZI_SIL_OFF);
    }
    
    m_flNextSecondaryAttack = m_flNextSecondaryAttack + 3.15;//3.0
    if (m_flNextSecondaryAttack < gpGlobals->time)
    
    m_flNextPrimaryAttack = m_flNextPrimaryAttack + 3.15;//3.0 //8
    if (m_flNextPrimaryAttack < gpGlobals->time)
    return;
	}//cierre del sil
}

void CUZI::PrimaryAttack( void )
{
#ifdef CLIENT_DLL
    if ( m_fDefaultAnim == MP5_DEFAULT_ANIM )
#else
    if ( m_fDefaultAnim == NULL )
#endif
    {
        m_fDefaultAnim = 0;//No Silencer 
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				UZIFire( 0.01, 0.09, TRUE, FALSE );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				UZIFire( 0.05, 0.09, TRUE, FALSE );
			}
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				UZIFire( 0.4, 0.09, TRUE, FALSE );
			}						// 14 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				UZIFire( 0.4, 0.09, TRUE, FALSE );
			}
		}
// ### COD RECOIL END ###
//		SERVER_COMMAND( "hud_newcross_size 5\n" );//MAP TOO BIG
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
				UZIFireSilenced( 0.001, 0.05, TRUE, TRUE );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				UZIFireSilenced( 0.02, 0.05, TRUE, TRUE );
			}
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				UZIFireSilenced( 0.2, 0.05, TRUE, TRUE );
			}						// 14 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				UZIFireSilenced( 0.2, 0.05, TRUE, TRUE );
			}
		}
// ### COD RECOIL END ###
//		SERVER_COMMAND( "hud_newcross_size 5\n" );//MAP TOO BIG
    }	
//test
	return;
}
// this function only gets called if silenced
void CUZI::UZIFireSilenced( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
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
        SendWeaponAnim( UZI_SIL_FIRE2 );
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
        m_pPlayer->m_iWeaponFlash = UNDETECTABLE_GUN_FLASH;
    
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
    vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 5096, BULLET_PLAYER_UZI, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireUZI_SIL, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 0:0, 0 );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

 	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CUZI::UZIFire( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
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
    vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 5096, BULLET_PLAYER_UZI, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireUZI, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 0:0, 0 );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CUZI::Reload( void )
{
    if ( m_pPlayer->ammo_9mm <= 0 )
    return;

    int iResult;

//if(m_iClip<=29)
//{    
    if(m_fDefaultAnim != 0)
    {
	//	if (m_iClip == 0)
			iResult = DefaultReload( 30, UZI_SIL_RELOAD_VACIO, 2.80 );
	//	else
	//		iResult = DefaultReload( 30, UZI_SIL_RELOAD, 2.10 );
    }
    else
    {
	//	if (m_iClip == 0)
			iResult = DefaultReload( 30, UZI_RELOAD_VACIO, 2.80 );
	//	else
	//		iResult = DefaultReload( 30, UZI_RELOAD, 2.10 );
    }

//}
 	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD );
			
		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_pistol", vecSrc, pev->angles );
	}
}


void CUZI::WeaponIdle( void )
{
	
    ResetEmptySound( );
    
    m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
    
    if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
          return;
    
    if (m_iClip != 0)    
    {
    if(m_fDefaultAnim == 1)
        SendWeaponAnim( UZI_SIL_IDLE1 );
    else
    SendWeaponAnim( UZI_IDLE1 );

    m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
    }
	
}


class CUZIAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/uzi/w_UZIclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/uzi/w_UZIclip.mdl");
//		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 30, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_uziclip, CUZIAmmoClip );

