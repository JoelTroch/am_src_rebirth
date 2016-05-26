
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "shake.h" //This is required for the screen fade

#define SVD_MODEL_1STPERSON "models/weapons/m82/v_m82.mdl" 
#define SVD_MODEL_3RDPERSON "models/weapons/m82/p_m82.mdl" 
#define SVD_MODEL_WORLD     "models/weapons/m82/w_m82.mdl" 
#define SVD_SOUND_FIRE1     "weapons/m82/m82_fire-1.wav" 
#define SVD_SOUND_FIRE2     "weapons/m82/m82_fire-2.wav" 
#define SVD_SOUND_VOLUME    0.80 
#define SVD_MAX_AMMO		10

//#define SVD_WEIGHT		15 //SP: already defined in weapons.h

#define BODY_GROUP		0
#define GILL_GROUP		1

#define GILL_NONE		0
#define GILL_ADD		1
/*
	SVD_IDLE1 = 0,

	SVD_FIRE1,//1.10
	SVD_FIRE2,

	SVD_RELOAD,//4.05
	SVD_RELOAD_VACIO,//5.70

	SVD_DEPLOY,//1.36
	SVD_HOLSTER,
	SVD_DEPLOY_FIRST,//2.05
		
	SVD_RELOAD_NMC,//2.36
	SVD_RELOAD_VACIO_NMC//3.36
	*/
enum svd_e
{
	SVD_IDLE1 = 0,

//	SVD_FIRE1,//1.10
//	SVD_FIRE2,

	SVD_RELOAD,//4.05
//	SVD_RELOAD_VACIO,//5.70

	SVD_DEPLOY,//1.36

	SVD_FIRE1,//1.10
	SVD_FIRE2,

//	SVD_HOLSTER,
//	SVD_DEPLOY_FIRST,//2.05
		
//	SVD_RELOAD_NMC,//2.36
//	SVD_RELOAD_VACIO_NMC//3.36
};
LINK_ENTITY_TO_CLASS( weapon_m82, CSvd );

void CSvd::Spawn( )
{
    pev->classname = MAKE_STRING("weapon_m82"); // hack to allow for old names
    Precache( );
    SET_MODEL(ENT(pev), SVD_MODEL_WORLD);
    m_iId          = WEAPON_SVD;    
    m_iDefaultAmmo = 5; //10
    FallInit();  // get ready to fall down. //se supone que fueron disparadas
}											//4 rafagas
//This is our spawn function

void CSvd::Precache( void )
{
    PRECACHE_MODEL(SVD_MODEL_1STPERSON);
    PRECACHE_MODEL(SVD_MODEL_3RDPERSON);
    PRECACHE_MODEL(SVD_MODEL_WORLD);
    
	PRECACHE_MODEL("models/weapons/m82/v_m82_scope.mdl");

	PRECACHE_SOUND( "weapons/zoom.wav" );//zoom, used for some weapons

    m_iShell = PRECACHE_MODEL ("models/weapons/shell_sniper.mdl");// brass shell
           
    PRECACHE_SOUND (SVD_SOUND_FIRE1);
    PRECACHE_SOUND (SVD_SOUND_FIRE2);   
    
	PRECACHE_SOUND ("player/damage/breath_hearth.wav");

    m_usFireSvd = PRECACHE_EVENT( 1, "scripts/events/m82.sc" );
}
//This is where we load any of the models or sounds that our weapon uses

int CSvd::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ammo_m82";
	p->iMaxAmmo1 = 10;//SNIPER_MAX_AMMO
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1; //M203_GRENADE_MAX_CARRY;
	p->iMaxClip = 5;
	p->iSlot = 5;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SVD;
	p->iWeight = MP5_WEIGHT;
	p->weaponName = "Barret M82";
	/*
    p->pszName   = STRING(pev->classname);
    p->pszAmmo1  = "ammo_m82";              // The type of ammo it uses
    p->iMaxAmmo1 = SVD_MAX_AMMO;            // Max ammo the player can carry
    p->pszAmmo2  = NULL;                    // No secondary ammo
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = 30; //10//svd_DEFAULT_AMMO;        // The clip size
    p->iSlot     = 5;	//svd_SLOT - 1;     // The number in the HUD
    p->iPosition = 1;    //svd_POSITION;            // The position in a HUD slot
    p->iFlags    = 0;
    p->iId       = m_iId = WEAPON_SVD;      // The weapon id
    p->iWeight   = SVD_WEIGHT;              // for autoswitching
    p->weaponName = "m82";
	*/
    return 1;
}
//Here we set the information for the weapon

int CSvd::AddToPlayer( CBasePlayer *pPlayer )
{
    if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
    {
	//	ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "Recogida m82"); //digamos al cliente
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_m82"); //digamos al cliente

          MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
          WRITE_BYTE( m_iId );
          MESSAGE_END();
          return TRUE;
    }
    return FALSE;
}

BOOL CSvd::Deploy( )
{
//	m_pPlayer->m_fCrosshairOff = TRUE;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1;
	
	pev->body = 0;
	return DefaultDeploy( "models/weapons/m82/v_m82.mdl", "models/weapons/m82/p_m82.mdl", SVD_DEPLOY, "awn", UseDecrement(), pev->body );
}

void CSvd::Holster( int skiplocal /* = 0 */ )
{
//	m_pPlayer->m_fCrosshairOff = FALSE;
	m_fInReload = FALSE;// cancel any reload in progress.
		m_pPlayer->b_EstaEnZoom = FALSE;
if (m_fInZoom == 0) 
{
	#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/m82/v_m82.mdl");
#else
	//	LoadVModel ( "models/v_scope.mdl", m_pPlayer );
#endif
m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
m_fInZoom = 0;
}
else if (m_fInZoom == 1) 
{
	#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/m82/v_m82.mdl");
#else
	//	LoadVModel ( "models/v_scope.mdl", m_pPlayer );
#endif
m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
m_fInZoom = 0;
}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0; //fix
	if (m_iClip)
		SendWeaponAnim( SVD_IDLE1 );
	else
		SendWeaponAnim( SVD_IDLE1 );
//	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
}

void CSvd::PrimaryAttack()
{
		if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))
    return;

//	pev->body = 0;
//	SetBodygroup( GILL_GROUP, GILL_NONE );

    if (m_pPlayer->pev->waterlevel == 3)
    {
          PlayEmptySound( );
          m_flNextPrimaryAttack = 0.15;
          return;
    }
    
    // don't fire if empty
    if (m_iClip <= 0)
    {
          PlayEmptySound();
          m_flNextPrimaryAttack = 0.15;
          return;
    }

    // Weapon sound
    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

    // one less round in the clip
    m_iClip--;

    // add a muzzle flash
    m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
    
    // player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
    
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;
		
// ### COD RECOIL END ###
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_PERFECT, 8192, BULLET_PLAYER_SCOUT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_SCOUT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_SCOUT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}						// 14 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_SCOUT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
		}
// ### COD RECOIL END ###
  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireSvd, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
    
    // Add a delay before the player can fire the next shot

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;  // 1.15;  //SVD_FIRE_DELAY 1.95
    m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 21.53 + 21, 21.53 + 20); //20 1
	
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75; //zoom
}
//This is the meat of the weapon.


void CSvd::SecondaryAttack()
{


if (m_fInZoom == 0) 
{
		m_pPlayer->b_EstaEnZoom = TRUE;
m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45; // 0 means reset to default fov
m_fInZoom = 1;	//valor de zoom 1
ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "1x"); //digamos al cliente que tiene 1x de zoom
#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/m82/v_m82_scope.mdl"); // pongamos el circulo negro
#else
		LoadVModel ( "models/weapons/m82/v_m82_scope.mdl", m_pPlayer ); //carguemos el model al jugador
#endif        
		UTIL_ScreenFade( m_pPlayer, Vector(0,0,0), 1, 0.1, 255, FFADE_IN); //fade to red 
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "player/damage/breath_hearth.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
}
else if (m_fInZoom == 1)
{
m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/m82/v_m82.mdl");//model default
#else
	//	LoadVModel ( "models/v_scope.mdl", m_pPlayer );
#endif
m_fInZoom = 0; //zoom ninguno (=ZERO)
		m_pPlayer->b_EstaEnZoom = FALSE;
ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "0x");// el jugador ya no tiene zoom
// limpiemos canal de sonido
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
}
	pev->nextthink = UTIL_WeaponTimeBase() + 0.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5; 
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
}


void CSvd::Reload( void )
{
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
		m_pPlayer->b_EstaEnZoom = FALSE;
//m_pPlayer->m_iHideHUD &= ~HIDEHUD_WEAPONS;
	//ya no es necesario, al cargar el v_scope el otro model desaparece
m_fInReload = FALSE;// cancel any reload in progress.

if (m_fInZoom == 0) 
{

	#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/m82/v_m82.mdl");
#else
	//	LoadVModel ( "models/v_scope.mdl", m_pPlayer );
#endif
//UTIL_ScreenFade( m_pPlayer, Vector(128,0,0), 0.001, 0.001, 16, FFADE_OUT); //fade desde rojo y sacamos

m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
m_fInZoom = 0;
}
else if (m_fInZoom == 1) 
{
	#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/m82/v_m82.mdl");
#else
	//	LoadVModel ( "models/v_scope.mdl", m_pPlayer );
#endif
//UTIL_ScreenFade( m_pPlayer, Vector(128,0,0), 0.001, 0.001, 16, FFADE_OUT); //fade desde rojo y sacamos
m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
m_fInZoom = 0;
}

	int iResult;

	iResult = DefaultReload( 5, SVD_RELOAD, 2 );

/*
if(m_iClip<=9)
{    
	if (m_iClip == 0)
		iResult = DefaultReload( 10, SVD_RELOAD_VACIO, 5.70 );
	else
	{
		switch (RANDOM_LONG(0,1))
		{
		case 0: iResult = DefaultReload( 10, SVD_RELOAD, 4.05 );
			break;
		case 1: iResult = DefaultReload( 10, SVD_RELOAD_NMC, 2.35 );
			break;
		}
	}
}
*/
	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
}				

void CSvd::WeaponIdle( void )
{

//	pev->body = 0;

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = SVD_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = SVD_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 20, 25 ); // how long till we do this again.

}

void CSvdAmmoClip::Spawn( void )
{ 
    Precache( );
    SET_MODEL(ENT(pev), "models/weapons/m82/w_m82clip.mdl");
    CBasePlayerAmmo::Spawn( );
}

void CSvdAmmoClip::Precache( void )
{
    PRECACHE_MODEL ("models/weapons/m82/w_m82clip.mdl");
    PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CSvdAmmoClip::AddAmmo( CBaseEntity *pOther ) 
{								
    int bResult = (pOther->GiveAmmo(8, "ammo_m82", 
                                   SVD_MAX_AMMO) != -1);
    if (bResult)
    {
        EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
    }
    return bResult;
}
LINK_ENTITY_TO_CLASS( ammo_m82, CSvdAmmoClip );