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

#define SILENCED_GUN_VOLUME		64
#define	SMALEST_GUN_FLASH		64

#define BER_NOSIL		0
#define BER_SEHSIL		1

enum Ber92f_e 
{
	BER_IDLE1_SIL = 0,
	BER_IDLE2_SIL,
	BER_IDLE1_SIL_VACIO,
	BER_IDLE2_SIL_VACIO,
	BER_RELOAD_SIL,
	BER_RELOAD_SIL_VACIO,
	BER_FIRE1_SIL,
	BER_FIRE2_SIL,
	BER_FIRE_SIL_LAST,
	BER_DEPLOY_SIL,
	BER_DEPLOY_SIL_VACIO,
	BER_HOLSTER_SIL,
	BER_HOLSTER_SIL_VACIO,
	
	BER_SILENCER_OFF,

	BER_IDLE1,
	BER_IDLE2,
	BER_IDLE1_VACIO,
	BER_IDLE2_VACIO,
	BER_RELOAD,
	BER_RELOAD_VACIO,
	BER_FIRE1,
	BER_FIRE2,
	BER_FIRE_LAST,
	BER_DEPLOY,
	BER_DEPLOY_VACIO,
	BER_HOLSTER,
	BER_HOLSTER_VACIO,

	BER_SILENCER_ON
};


///maybe is not the right place to put this:

void CFuckAr2::Spawn( )
{
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/glock18/w_glock18clip.mdl");

	FallInit();// get ready to fall down.

//	SetThink (Think);
//	pev->nextthink = gpGlobals->time + 3;
}
int CFuckAr2::AddToPlayer( CBasePlayer *pPlayer )
{
	return FALSE;
}
void CFuckAr2::Precache( void )
{
	PRECACHE_MODEL("models/weapons/glock18/w_glock18clip.mdl");
}
void CFuckAr2::Think( void )
{
	/*
	ALERT(at_console, "I am thinking!\n");

	CBaseEntity *pEffect = Create( "test_effectmini", pev->origin, pev->angles );
	pEffect->Use( this, this, USE_ON, 1 );

	UTIL_Sparks( pev->origin );

	UTIL_Remove( this );
	SetThink( NULL );
	*/
}
void CFuckAr2::Dissapear( void )
{
	/*
	ALERT(at_console, "I am thinking!\n");

	CBaseEntity *pEffect = Create( "test_effectmini", pev->origin, pev->angles );
	pEffect->Use( this, this, USE_ON, 1 );

	UTIL_Sparks( pev->origin );

	UTIL_Remove( this );
	SetThink( NULL );
	*/
}

LINK_ENTITY_TO_CLASS( item_clip_pistol, CFuckAr2 );

//

///maybe is not the right place to put this:

void CFuckMP7::Spawn( )
{
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/m16/w_m16Clip.mdl");

	FallInit();// get ready to fall down.
}
int CFuckMP7::AddToPlayer( CBasePlayer *pPlayer )
{
	return FALSE;
}
void CFuckMP7::Precache( void )
{
	PRECACHE_MODEL("models/weapons/m16/w_m16Clip.mdl");
}
void CFuckMP7::Think( void )
{

}
void CFuckMP7::Dissapear( void )
{

}

LINK_ENTITY_TO_CLASS( item_clip_rifle, CFuckMP7 );


LINK_ENTITY_TO_CLASS( weapon_ber92f, CBer92f );
LINK_ENTITY_TO_CLASS( weapon_beretta, CBer92f );


void CBer92f::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_ber92f"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_BER92F;
	SET_MODEL(ENT(pev), "models/weapons/ber92f/w_ber92f.mdl");

	m_iDefaultAmmo = BER92F_DEFAULT_GIVE;
	m_fDefaultAnim = MP5_DEFAULT_ANIM; //to define on startup if the silencer added or not
	m_bSilAdd = FALSE;     // Have we been initialised

	FallInit();// get ready to fall down.
}

int CBer92f::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Ber"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CBer92f::Precache( void )
{
	PRECACHE_MODEL("models/weapons/ber92f/v_ber92f.mdl");
	PRECACHE_MODEL("models/weapons/ber92f/w_ber92f.mdl");
	PRECACHE_MODEL("models/weapons/ber92f/p_ber92f.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_9mm.mdl");// brass shell

	PRECACHE_SOUND ("weapons/ber92f/ber92f_fire-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/ber92f/ber92f_fire-2.wav");// H to the K

	PRECACHE_SOUND ("weapons/ber92f/ber92f_firesil-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/ber92f/ber92f_firesil-2.wav");// H to the K

	m_usBerFire = PRECACHE_EVENT( 1, "scripts/events/92f.sc" );
	m_usBerFireSil = PRECACHE_EVENT( 1, "scripts/events/92f_sil.sc" );
}

int CBer92f::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = BER92F_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_BER92F;
	p->iWeight = BER92F_WEIGHT;
	p->weaponName = "Pietro Beretta 92 F";

	return 1;
}

BOOL CBer92f::Deploy( )
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
	if(m_fDefaultAnim == 1)
	{
		return DefaultDeploy( "models/weapons/ber92f/v_ber92f.mdl", "models/weapons/ber92f/p_ber92f.mdl", BER_DEPLOY_SIL, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
	}
    else
	{
		return DefaultDeploy( "models/weapons/ber92f/v_ber92f.mdl", "models/weapons/ber92f/p_ber92f.mdl", BER_DEPLOY, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
	}
//	return DefaultDeploy( "models/weapons/ber92f/v_ber92f.mdl", "models/weapons/ber92f/p_ber92f.mdl", BER_DEPLOY_SIL, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}
void CBer92f::Holster( int skiplocal /* = 0 */ )
{
    m_fInReload = FALSE;// cancel any reload in progress.

    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
    m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CBer92f::PrimaryAttack( void )
{
#ifdef CLIENT_DLL
    if ( m_fDefaultAnim == MP5_DEFAULT_ANIM )
#else
    if ( m_fDefaultAnim == NULL )
#endif
    {
        m_fDefaultAnim = 0;//No Silencer
        if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) //only one shoot by mousclick
        return;
  //    BerFire( 0.01, 0.03, TRUE, FALSE );
        BerFire( 0, 0.08, TRUE, FALSE );

        return;
    }

    if ( m_fDefaultAnim != NULL )//Silencer added
    {
        m_fDefaultAnim = 1;
        if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) //only one shoot by mousclick
        return;	
		BerFireSil( 0, 0.08, TRUE, TRUE );
//		BerFireSil( 0.02, 0.025, TRUE, TRUE );
		return;
    }
}

void CBer92f::BerFire( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
{
    // Aiming Mechanics 
	float targetx=0.312; // these are the numbers we will use for the aiming vector (X Y Z) 
	float targety=0.312; // these are the numbers the will be loward accordingly to adjust the aim 
	float targetz=0.312; 
// Aiming Mechanics 

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

    Vector vecDir, vecDir1;		

	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( targetx, targety, targetz ), 8192, BULLET_PLAYER_BER92F, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed ); 

    //vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 5096, BULLET_PLAYER_BER92F, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usBerFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 0:0, 0 );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CBer92f::BerFireSil( float flSpread , float flCycleTime, BOOL fUseAutoAim, BOOL Silenced )
{
    if (m_pPlayer->pev->waterlevel == 3)
    {
        PlayEmptySound( );
        m_flNextPrimaryAttack = 0.15;
        return;
    }

    m_fDefaultAnim = Silenced;//Nur zu sicherheit
/*
    if (m_iClip == 1)
    {
        SendWeaponAnim( BER_FIRE_SIL_LAST );
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
    }
*/
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

    Vector vecDir;																							
    vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 5096, BULLET_PLAYER_BER92F /2, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usBerFireSil, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 0:0, 0 );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

  //  m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
	  m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;//sys test

}

void CBer92f::SecondaryAttack( void )
{
	// Acá comprobamos que el jugador tiene o no silenciador
	// y si no lo tiene, no lo ponemos...
	if (m_pPlayer->m_bSilAdd== FALSE )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#No_Silencer"); //digamos al cliente
		
		m_flNextSecondaryAttack = m_flNextSecondaryAttack + 2.0;//delays
	//	SendWeaponAnim(	BER_IDLE1); // Set animation here... If you want
		return; // return y no ejecutar más codigo.
	}
	else
	{
    m_flTimeWeaponIdle = gpGlobals->time + 5.50f;
    
    if(m_fDefaultAnim == 1){
    m_fDefaultAnim = 0; //Detach silencer
    SendWeaponAnim(BER_SILENCER_OFF);
    }
    else
    {
    m_fDefaultAnim = 1; //Add silencer
    SendWeaponAnim(BER_SILENCER_ON);
    }
    
    m_flNextSecondaryAttack = m_flNextSecondaryAttack + 3.0;//delays
    if (m_flNextSecondaryAttack < gpGlobals->time)
    
    m_flNextPrimaryAttack = m_flNextPrimaryAttack + 3.0;
    if (m_flNextPrimaryAttack < gpGlobals->time)
    return;
	}
}

void CBer92f::Reload( void )
{
   if ( m_pPlayer->ammo_9mm <= 0 )
   return;

    int iResult;

//if(m_iClip<=14)
//{    
    if(m_fDefaultAnim != 0)
    {
        iResult = DefaultReload( 15, BER_RELOAD_SIL, 2.55 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
    }
    else
    {
        iResult = DefaultReload( 15, BER_RELOAD, 2.45 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
    }

//}

    if (iResult)
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
				Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_pistol", vecSrc, pev->angles );		

    }
}

void CBer92f::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

    if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
          return;
    
    if (m_iClip != 0)    //si tiene bala
    {
    if(m_fDefaultAnim == 1) //si tiene sil
        SendWeaponAnim( BER_IDLE2_SIL );
    else
		SendWeaponAnim( BER_IDLE1 );//si no

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 20, 25 ); // how long till we do this again.

	}

}

class CBer92fAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/ber92f/w_ber92fclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/ber92f/w_ber92fclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( BER92F_DEFAULT_GIVE, "9mm", _9MM_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_ber92f, CBer92fAmmo );
/*
class CBer92fSilAdd : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/ber92f/w_ber92f_extended.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/ber92f/w_ber92f_extended.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		ClientPrint(pOther->pev, HUD_PRINTCENTER, "#Pickup_BerSilencer"); //digamos al cliente

		ALERT (at_console, "KIT DE SILENCIADOR DE BERETTA ADQUIRIDO! \n" );	

		((CBasePlayer*)pOther)->m_bSilAdd = TRUE;
		m_bSilAdd = TRUE;
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_ber_sil, CBer92fSilAdd );

class CAllSilAdd : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/w_all_extended.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/w_all_extended.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		ClientPrint(pOther->pev, HUD_PRINTCENTER, "#Pickup_MasterKit"); //digamos al cliente
		
		((CBasePlayer*)pOther)->m_bSilAdd = TRUE; //Ber92f
		m_bSilAdd = TRUE;

		((CBasePlayer*)pOther)-> m_bUspSilAdd = TRUE; //Usp
		m_bUspSilAdd = TRUE;

		((CBasePlayer*)pOther)->m_bMp5SilAdd = TRUE; //Mp5
		m_bMp5SilAdd = TRUE;

		((CBasePlayer*)pOther)->m_bM16SilAdd = TRUE; //m4a1
		m_bM16SilAdd = TRUE;

		((CBasePlayer*)pOther)->m_bUZISilAdd = TRUE; //m4a1
		m_bUZISilAdd = TRUE;

		ALERT (at_console, "##TODOS LOS KIT DE CONVERSION ADQUIRIDOS!## \n" );	

		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_all_sil, CAllSilAdd );
*/