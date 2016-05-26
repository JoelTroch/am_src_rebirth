//irgun.cpp
//created with Gott`s weapon creator by Cid Highwind www.warineuropemod.com

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

enum IRGUN_e
{
  IRGUN_LONGIDLE = 0,
  IRGUN_IDLE1,
  IRGUN_LAUNCH,
  IRGUN_RELOAD,
  IRGUN_DEPLOY,
  IRGUN_FIRE1,
  IRGUN_FIRE2,
  IRGUN_FIRE3,
};

LINK_ENTITY_TO_CLASS( weapon_irgun, CIRGUN );

void CIRGUN::Spawn( )
{
  pev->classname = MAKE_STRING("weapon_irgun");
  Precache( );
  m_iId = WEAPON_HL2_IRGUN;
  SET_MODEL(ENT(pev), "models/weapons/irgun/w_irgun.mdl");
  m_iDefaultAmmo = IRGUN_MAX_DEFAULT_GIVE; 
  FallInit();
}

void CIRGUN::Precache( void )
{
  PRECACHE_MODEL("models/weapons/irgun/v_irgun.mdl");
  PRECACHE_MODEL("models/weapons/irgun/w_irgun.mdl");
  PRECACHE_MODEL("models/weapons/irgun/p_irgun.mdl");

//  m_iShell = PRECACHE_MODEL ("models/shell.mdl");
	
  iMuzzleFlash = PRECACHE_MODEL( "sprites/muz4.spr" );

  PRECACHE_SOUND("items/irgunclip.wav");
  PRECACHE_SOUND("weapons/irgun/irgun_fire-1.wav");
  PRECACHE_SOUND("weapons/irgun/irgun_fire-2.wav");
  PRECACHE_SOUND("weapons/irgun/irgun_fire-3.wav");

  m_usFireIRGUN = PRECACHE_EVENT( 1, "scripts/events/irgun.sc" );
}
int CIRGUN::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_IRifle"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}
int CIRGUN::GetItemInfo(ItemInfo *p)
{
  p->pszName = STRING(pev->classname);
  p->pszAmmo1 = "Pulse";
  p->iMaxAmmo1 = IRGUN_MAX_CARRY;

	p->pszAmmo2 = "IRgrenades";
	p->iMaxAmmo2 = 10;//M203_GRENADE_MAX_CARRY

//  p->pszAmmo2 = "Nothing";
//  p->iMaxAmmo2 = 0;
  p->iMaxClip = IRGUN_MAX_CLIP;
  p->iSlot = 2;
  p->iPosition = 3;
  p->iFlags = 0;
  p->iId = m_iId = WEAPON_HL2_IRGUN;
  p->iWeight = ELITE_WEIGHT;
  p->weaponName = "HL2";

  return 1;
}

BOOL CIRGUN::Deploy( )
{
  return DefaultDeploy( "models/weapons/irgun/v_irgun.mdl", "models/weapons/irgun/p_irgun.mdl", IRGUN_DEPLOY, "irgun",  0 );
}

void CIRGUN::SecondaryAttack( void )
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound( );
		return;
	}
}

void CIRGUN::PrimaryAttack( void )
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}
	if (m_iClip <= 0)
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_2DEGREES, 8192, BULLET_PLAYER_IRGUN, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireIRGUN, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		//m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.09; // 0.1; delay

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.09; // 0.1; delay

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 20, 30 );

  if(m_pPlayer->pev->flags & FL_DUCKING)//ducking
  {
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.y -= 0.6; break;
 	 case 1: m_pPlayer->pev->punchangle.y += 0.6; break;
    }
    switch (RANDOM_LONG(0,1))
    {
 	 case 0: m_pPlayer->pev->punchangle.x -= 0.6; break;
 	 case 1: m_pPlayer->pev->punchangle.x += 0.6; break;
    }
  }
  else if (m_pPlayer->pev->velocity.Length() > .01)//moving
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
else//stand
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
}

int CIRGUN::SecondaryAmmoIndex( void )
{
	return m_iSecondaryAmmoType;
}

void CIRGUN::IRGUNFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{/*
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

  PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireIRGUN : m_usFireIRGUN, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

  m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
  Vector vecSrc	 = m_pPlayer->GetGunPosition( );
  Vector vecAiming;

  if ( fUseAutoAim )
  {
    vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
  }
  else
  {
    vecAiming = gpGlobals->v_forward;
  }

  m_pPlayer->FireBullets( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_IRGUN, 2 );
 m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
      
 // m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;

  if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
  m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

  m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

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
}

void CIRGUN::Reload( void )
{
  int iResult;

  if (m_iClip == IRGUN_MAX_CLIP)
  return;

  if (m_iClip == 0)
  {
    iResult = DefaultReload( IRGUN_MAX_CLIP, IRGUN_RELOAD, 1.5);
  }
  else
  {
    iResult = DefaultReload( IRGUN_MAX_CLIP, IRGUN_RELOAD, 1.5);
  }
  if (iResult)
  {
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
  }
}

void CIRGUN::WeaponIdle( void )
{
  ResetEmptySound( );
  m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

  if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    return;

  if (m_iClip != 0)
  {
    int iAnim;
    float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

    if (flRand <= 0.3 + 0 * 0.75)
    {
      iAnim = IRGUN_IDLE1;
      m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10 / 16;
    }
    else if (flRand <= 0.6 + 0 * 0.875)
    {
      iAnim = IRGUN_LONGIDLE;
      m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 15 / 16;
    }
    else
    {
      iAnim = IRGUN_IDLE1;
      m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 15 / 16;
    }
  SendWeaponAnim( iAnim, 1 );
  }
}

class CIRGUNAmmoClip : public CBasePlayerAmmo
{
  void Spawn( void )
  {
    Precache( );
    SET_MODEL(ENT(pev), "models/weapons/irgun/w_irgunclip.mdl");
    CBasePlayerAmmo::Spawn( );
  }
  void Precache( void )
  {
    PRECACHE_MODEL ("models/weapons/irgun/w_irgunclip.mdl");
    PRECACHE_SOUND("items/irgunclip.wav");
  }
  BOOL AddAmmo( CBaseEntity *pOther ) 
  {
    int bResult = (pOther->GiveAmmo( AMMO_IRGUNCLIP_GIVE, "Pulse", IRGUN_MAX_CARRY) != -1);
    if(bResult)
    {
      EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/irgunclip.wav", 1, ATTN_NORM);
    }
  return bResult;
  }
};
LINK_ENTITY_TO_CLASS( ammo_irgun, CIRGUNAmmoClip );
