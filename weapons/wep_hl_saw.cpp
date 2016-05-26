#include "extdll.h"
#include "util.h" 
#include "cbase.h"
#include "weapons.h"            
#include "player.h"
#include "gamerules.h"

enum m249_e 
{ 
	M249_IDLE = 0,
	M249_IDLE2,
	M249_DRAW,
	M249_HOLSTER,
	M249_SHOOT,
	M249_SHOOT2,
	M249_SHOOT3,
	M249_RELOAD,
	M249_RELOAD2
};

LINK_ENTITY_TO_CLASS( weapon_saw, CSaw );


void CSaw :: Spawn( )
{
	pev->classname = MAKE_STRING("weapon_saw"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_HL_SAW;
	SET_MODEL(ENT(pev), "models/weapons/saw/w_saw.mdl");

	pev->frame = 0;
	pev->sequence = 1;
	ResetSequenceInfo( );
	pev->framerate = 0;

	m_iDefaultAmmo = 50;

	FallInit();
}

void CSaw :: Precache( void )
{
	PRECACHE_MODEL("models/weapons/saw/v_saw.mdl");
	PRECACHE_MODEL("models/weapons/saw/w_saw.mdl");
	PRECACHE_MODEL("models/weapons/saw/p_saw.mdl");

	m_usSaw = PRECACHE_EVENT( 1, "scripts/events/saw.sc" );
}

int CSaw :: GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
    p->pszAmmo1  = "556";              // The type of ammo it uses
    p->iMaxAmmo1 = _556_MAX_AMMO;            // Max ammo the player can carry
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SAW_MAX_CLIP;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_HL_SAW;
	p->iWeight = 10;

	return 1;
}

BOOL CSaw :: Deploy ( ) 
{ 
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	return DefaultDeploy("models/weapons/saw/v_saw.mdl", "models/weapons/saw/p_saw.mdl", M249_DRAW, "xm4", 3.50f );
}

void CSaw :: Holster ( void )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	SendWeaponAnim( M249_HOLSTER );
}

void CSaw :: PrimaryAttack( void )
{
	Fire( 0.08);
}         
void CSaw :: SecondaryAttack( void )
{
	Fire( 0.8);
}
void CSaw :: Fire ( float nextattack )
{
	if (m_pPlayer->pev->waterlevel == 3 || m_iClip <= 0)
 	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	if ( m_iClip && m_pPlayer->pev->waterlevel != 3)//don't fire underwater
	{
		m_pPlayer->m_iWeaponVolume 	= LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash 	= BRIGHT_GUN_FLASH;
		
		m_iClip--;
                    
		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_pPlayer->pev->velocity 	= m_pPlayer->pev->velocity - gpGlobals->v_forward * 30;
            
		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
		Vector vecDir;

		//hide rounds on end tape
//		if(m_iClip < 9 && m_iClip) m_iBody++;
//		SendWeaponAnim( M249_SHOOT );
           
		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_357, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

		PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usSaw, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, pev->body, 0, 0, 0 );
	  
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + nextattack;
		
		if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + nextattack + 0.02;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT ( 10, 15 );
	}
	else
	{
		PlayEmptySound( );
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CSaw :: Reload( void )
{                      
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0) return;      
	DefaultReload( SAW_MAX_CLIP, M249_RELOAD, 1.4 );
	m_iOverloadLevel = 1;          
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT ( 10, 15 );
}                                                       

void CSaw :: WeaponIdle( void )
{
	if (m_iOverloadLevel)
	{
//		m_iBody = 0; //show rounds tape
		SendWeaponAnim( M249_RELOAD2 );
		m_iOverloadLevel = 0; //reloaded ok
	}

  	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ) return;
	if (m_iClip)
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.75)
		{
			iAnim = M249_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.3;
		}
		else
		{
			iAnim = M249_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.4;
		}
		SendWeaponAnim( iAnim );
	}
}

int CSaw::AddToPlayer( CBasePlayer *pPlayer )
{
    if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
    {
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Saw"); //digamos al cliente

		m_pPlayer->SetSuitUpdate("!HEV_WEAPON", FALSE, SUIT_NEXT_IN_1MIN);

          MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
          WRITE_BYTE( m_iId );
          MESSAGE_END();
          return TRUE;
    }
    return FALSE;
}

