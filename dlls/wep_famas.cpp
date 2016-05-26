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

enum famas_e
{
	FAMAS_IDLE1 = 0,
	FAMAS_IDLE2,
	FAMAS_FIRE1,
	FAMAS_FIRE2,
	FAMAS_RELOAD,
	FAMAS_RELOAD_LAST,
	FAMAS_DEPLOY,
	FAMAS_HOLSTER,
	FAMAS_FIRE1_RND,
	FAMAS_FIRE2_RND,

};
LINK_ENTITY_TO_CLASS( weapon_famas, CFamas );
LINK_ENTITY_TO_CLASS( weapon_aug, CFamas );
LINK_ENTITY_TO_CLASS( ammo_556, CFamasAmmoClip );


void CFamas::Spawn( )
{
    pev->classname = MAKE_STRING("weapon_famas"); // hack to allow for old names
    Precache( );
    SET_MODEL(ENT(pev), FAMAS_MODEL_WORLD);
    m_iId          = WEAPON_FAMAS;    
    m_iDefaultAmmo = FAMAS_DEFAULT_AMMO;					//FAMAS_DEFAULT_AMMO; 

	m_fDefaultAnim = MP5_DEFAULT_ANIM; // es 0

//	m_bFamasBurst = FALSE;
    FallInit();  // get ready to fall down. //se supone que fueron disparadas
}											//4 rafagas
//This is our spawn function

void CFamas::Precache( void )
{
    PRECACHE_MODEL(FAMAS_MODEL_1STPERSON);
    PRECACHE_MODEL(FAMAS_MODEL_3RDPERSON);
    PRECACHE_MODEL(FAMAS_MODEL_WORLD);
    
    m_iShell = PRECACHE_MODEL ("models/weapons/shell_556.mdl");// brass shell
           
    PRECACHE_SOUND (FAMAS_SOUND_FIRE1);
    PRECACHE_SOUND (FAMAS_SOUND_FIRE2);   
    
	PRECACHE_SOUND ("weapons/glock18/fire-1.wav");
	PRECACHE_SOUND ("weapons/glock18/fire-2.wav");

    m_event = PRECACHE_EVENT( 1, "scripts/events/famas.sc" );
	m_event2 = PRECACHE_EVENT( 1, "scripts/events/famas_burst.sc" );
}
//This is where we load any of the models or sounds that our weapon uses

int CFamas::GetItemInfo(ItemInfo *p)
{
    p->pszName   = STRING(pev->classname);
    p->pszAmmo1  = "556";              // The type of ammo it uses
    p->iMaxAmmo1 = _556_MAX_AMMO;            // Max ammo the player can carry
    p->pszAmmo2  = NULL;                    // No secondary ammo
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = FAMAS_DEFAULT_AMMO;        // The clip size
    p->iSlot     = 3;//	//FAMAS_SLOT - 1;     // The number in the HUD
    p->iPosition = 1;    //FAMAS_POSITION;            // The position in a HUD slot
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
    p->iId       = m_iId = WEAPON_FAMAS;      // The weapon id
    p->iWeight   = FAMAS_WEIGHT;              // for autoswitching
    p->weaponName = "FA-MAS";

    return 1;
}
//Here we set the information for the weapon

int CFamas::AddToPlayer( CBasePlayer *pPlayer )
{
    if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
    {
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Famas"); //digamos al cliente

          MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
          WRITE_BYTE( m_iId );
          MESSAGE_END();

//		  m_iFiring = FALSE;

          return TRUE;
    }
    return FALSE;
}

BOOL CFamas::Deploy( )
{
    return DefaultDeploy( FAMAS_MODEL_1STPERSON, FAMAS_MODEL_3RDPERSON, 
                          FAMAS_DEPLOY, "m16" );
}
//animation and "mp5" is the series of animations in the player model

void CFamas::PrimaryAttack( void )
{
#ifdef CLIENT_DLL
    if ( m_fDefaultAnim == MP5_DEFAULT_ANIM )
#else
    if ( m_fDefaultAnim == NULL )
#endif
    {
        m_fDefaultAnim = 0;//No Silencer
 //       if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) //only one shoot by mousclick
  //      return;
        FireSemi();
        return;
    }

    if ( m_fDefaultAnim != NULL )//Silencer added
    {
        m_fDefaultAnim = 1;
		FireBurst();
		return;
    }
}

void CFamas::SecondaryAttack()
{
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK2 || IN_ATTACK)) 
	return; // SP: Fix to allow multichange pressed

	if(m_fDefaultAnim == 1)
	{
    m_fDefaultAnim = 0; //Detach silencer
//	m_pPlayer->m_bFamasBurst = FALSE;
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/glock18/fire-1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
	ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Semi_Mode");
	
//    SendWeaponAnim(BER_SILENCER_OFF);
    }
    else
    {
    m_fDefaultAnim = 1; //Add silencer
//	m_pPlayer->m_bFamasBurst = TRUE; //burst ON
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/glock18/fire-2.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
	ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Burst_Mode");
//    SendWeaponAnim(BER_SILENCER_ON);
    }

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;    
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;
	return; // return y no ejecutar más codigo.
}
void CFamas::FireSemi()
{
//	m_iFiring =FALSE;

	//KELLY: La gracia del ak es que dispara bajo el agua

    /*// don't fire underwater
    if (m_pPlayer->pev->waterlevel == 3)
    {
          PlayEmptySound( );
          m_flNextPrimaryAttack = 0.15;
          return;
    }
    */

    // don't fire if empty
    if (m_iClip <= 0)
    {
          PlayEmptySound();
          m_flNextPrimaryAttack = 0.15;
          return;
    }

    // Weapon sound
    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    // one less round in the clip
    m_iClip--;

    // add a muzzle flash
    m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
    
    // player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

// ### COD RECOIL START ###
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_2DEGREES, 8192, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}																	//VECTOR_CONE_5DEGREES
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}						// 14 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
		}
// ### COD RECOIL END ###
	
  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_event, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

    // Add a delay before the player can fire the next shot
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.09;    
    
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25; // tata
	
	m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 
                            UTIL_SharedRandomFloat(m_pPlayer->random_seed, 
                                        FAMAS_FIRE_DELAY + 1, FAMAS_FIRE_DELAY + 2);
}
//This is the meat of the weapon.

void CFamas::Reload( void )
{
	int iResult;

//	if (m_iClip == 0)
		iResult = DefaultReload( 25, FAMAS_RELOAD_LAST, 3.2 );
//	else
//		iResult = DefaultReload( 25, FAMAS_RELOAD, 2.8 );

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
			
		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_rifle", vecSrc, pev->angles );		

	}
}

void CFamas::WeaponIdle( void )
{
	ResetEmptySound( );

//	m_iFiring = FALSE;

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = FAMAS_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = FAMAS_IDLE2;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

void CFamas::FireBurst(void)
{
	
    // don't fire if empty
    if (m_iClip <= 0)
    {
          PlayEmptySound();
          m_flNextPrimaryAttack = 0.15;
		  Reload();
          return;
    }

    // Weapon sound
    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    // one less round in the clip
	int iDisparos;

	if (m_iClip >= 3)//si tiene mas de 3 tiros, _3_ tiros
		iDisparos = 3;
	if (m_iClip == 2)//si tiene 2 tiros, _2_ tiros
		iDisparos = 2;
	if (m_iClip == 1)//si tiene 1 tiro, _1_ tiro
		iDisparos = 1;

	m_iClip -= iDisparos;//restar al cargador # tiros ( 1, 2 o 3 )
//   m_iClip--;
//	m_iClip--;
//	m_iClip--;

    // add a muzzle flash
    m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
    
    // player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

#ifdef CLIENT_DLL
	if ( !bIsMultiplayer() )
#else
	if ( !g_pGameRules->IsMultiplayer() )
#endif
	{//mp
	vecDir = m_pPlayer->FireBulletsThroughWalls( iDisparos, vecSrc, vecAiming, VECTOR_CONE_4DEGREES, 8192, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	//vecDir = m_pPlayer->FireBulletsPlayer( 3, vecSrc, vecAiming, VECTOR_CONE_4DEGREES, 8192, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{//sp
	vecDir = m_pPlayer->FireBulletsThroughWalls( iDisparos, vecSrc, vecAiming, VECTOR_CONE_4DEGREES, 8192, BULLET_PLAYER_FAMAS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_event2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	// disparando
//	m_iFiring =TRUE;

    // Add a delay before the player can fire the next shot
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;    
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;	
	
	m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 
                            UTIL_SharedRandomFloat(m_pPlayer->random_seed, 
                                        FAMAS_FIRE_DELAY + 1, FAMAS_FIRE_DELAY + 2);
}

void CFamasAmmoClip::Spawn( void )
{ 
    Precache( );
    SET_MODEL(ENT(pev), "models/weapons/famas/w_556ammo.mdl");
    CBasePlayerAmmo::Spawn( );
}

void CFamasAmmoClip::Precache( void )
{
    PRECACHE_MODEL ("models/weapons/famas/w_556ammo.mdl");
    PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CFamasAmmoClip::AddAmmo( CBaseEntity *pOther ) 
{ 
    int bResult = (pOther->GiveAmmo(FAMAS_DEFAULT_AMMO, "556", 
                                    _556_MAX_AMMO) != -1);
    if (bResult)
    {
        EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
    }
    return bResult;
}

