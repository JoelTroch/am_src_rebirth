#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

#define WEAPON_TEST			12
#define TEST_DEFAULT_GIVE	30
#define TEST_MAX_CLIP		30
#define TEST_WEIGHT			20
#define AMMO_TESTCLIP_GIVE	30

enum TEST_e
{
TEST_LONGIDLE = 0,
TEST_IDLE1,
TEST_LAUNCH,
TEST_RELOAD,
TEST_DEPLOY,
TEST_FIRE1,
TEST_FIRE2,
TEST_FIRE3,
TEST_HOLSTER,
};
class CLaserSpotTEST : public CBaseEntity
{
void Spawn( void );
void Precache( void );

int ObjectCaps( void ) { return FCAP_DONT_SAVE; }

public:
void Suspend( float flSuspendTime );
void EXPORT Revive( void );

static CLaserSpotTEST *CreateSpotTEST( void );
};
LINK_ENTITY_TO_CLASS( laser_spot_TEST, CLaserSpotTEST );

class CTEST : public CBasePlayerWeapon
{
public:
int Save( CSave &save );
int Restore( CRestore &restore );
static TYPEDESCRIPTION m_SaveData[];

void Spawn( void );
void Precache( void );
int iItemSlot( void ) { return 3; }
int GetItemInfo(ItemInfo *p);
int AddToPlayer( CBasePlayer *pPlayer );

void PrimaryAttack( void );
void SecondaryAttack( void );
int SecondaryAmmoIndex( void );
BOOL Deploy( void );
void Reload( void );
void WeaponIdle( void );
void Holster ( int skiplocal );
float m_flNextAnimTime;
int m_iShell;

CLaserSpotTEST *m_pSpot;
int m_fSpotActive;

void UpdateSpot ( void );

private:
unsigned short m_usTEST;
};
LINK_ENTITY_TO_CLASS( weapon_test, CTEST );
//LINK_ENTITY_TO_CLASS( weapon_9mmAR, CTEST );

TYPEDESCRIPTION CTEST::m_SaveData[] =
{
DEFINE_FIELD( CTEST, m_fSpotActive, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( CTEST, CBasePlayerWeapon );
//=========================================================
//=========================================================
int CTEST::SecondaryAmmoIndex( void )
{
return m_iSecondaryAmmoType;
}

void CTEST::Spawn( )
{
pev->classname = MAKE_STRING("weapon_test"); // hack to allow for old names
Precache( );
SET_MODEL(ENT(pev), "models/w_9mmAR.mdl");
m_iId = WEAPON_TEST;
m_fSpotActive = 1;

m_iDefaultAmmo = TEST_DEFAULT_GIVE;

FallInit();// get ready to fall down.
}


void CTEST::Precache( void )
{
PRECACHE_MODEL("models/v_9mmAR.mdl");
PRECACHE_MODEL("models/w_9mmAR.mdl");
PRECACHE_MODEL("models/p_9mmAR.mdl");

m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

PRECACHE_MODEL("models/grenade.mdl"); // grenade

PRECACHE_MODEL("models/w_9mmARclip.mdl");
PRECACHE_SOUND("items/9mmclip1.wav");

PRECACHE_SOUND("items/clipinsert1.wav");
PRECACHE_SOUND("items/cliprelease1.wav");
// PRECACHE_SOUND("items/guncock1.wav");

PRECACHE_SOUND ("weapons/hks1.wav");// H to the K
PRECACHE_SOUND ("weapons/hks2.wav");// H to the K
PRECACHE_SOUND ("weapons/hks3.wav");// H to the K

PRECACHE_SOUND( "weapons/glauncher.wav" );
PRECACHE_SOUND( "weapons/glauncher2.wav" );

PRECACHE_SOUND ("weapons/357_cock1.wav");

m_usTEST = PRECACHE_EVENT( 1, "scripts/events/TEST.sc" );
}

int CTEST::GetItemInfo(ItemInfo *p)
{
p->pszName = STRING(pev->classname);
p->pszAmmo1 = "9mm";
p->iMaxAmmo1 = _9MM_MAX_CARRY;
p->pszAmmo2 = "ARgrenades";
p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
p->iMaxClip = TEST_MAX_CLIP;
p->iSlot = 2;
p->iPosition = 0;
p->iFlags = 0;
p->iId = m_iId = WEAPON_TEST;
p->iWeight = TEST_WEIGHT;

return 1;
}

int CTEST::AddToPlayer( CBasePlayer *pPlayer )
{
if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
{
MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
WRITE_BYTE( m_iId );
MESSAGE_END();
return TRUE;
}
return FALSE;
}

BOOL CTEST::Deploy( )
{
return DefaultDeploy( "models/v_9mmAR.mdl", "models/p_9mmAR.mdl", TEST_DEPLOY, "TEST" );
}


void CTEST::PrimaryAttack()
{
// don''t fire underwater
if (m_pPlayer->pev->waterlevel == 3)
{
PlayEmptySound( );
m_flNextPrimaryAttack = gpGlobals->time + 0.15;
return;
}

if (m_iClip <= 0)
{
PlayEmptySound();
m_flNextPrimaryAttack = gpGlobals->time + 0.15;
return;
}

PLAYBACK_EVENT( 0, m_pPlayer->edict(), m_usTEST );

m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

m_iClip--;

// player "shoot" animation
m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

Vector vecSrc = m_pPlayer->GetGunPosition( );
Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

if ( g_pGameRules->IsDeathmatch() )
{
// optimized multiplayer. Widened to make it easier to hit a moving player
m_pPlayer->FireBullets( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_MP5, 2 );
}
else
{
m_pPlayer->FireBullets( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 2 );
}

m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.1;
if (m_flNextPrimaryAttack < gpGlobals->time)
m_flNextPrimaryAttack = gpGlobals->time + 0.1;

m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );

UpdateSpot( );
}



void CTEST::SecondaryAttack( void )
{
// if the pointer is active, kill it, else create one:
m_fSpotActive = ! m_fSpotActive;

if (!m_fSpotActive && m_pSpot)
{
m_pSpot->Killed( NULL, GIB_NORMAL );
m_pSpot = NULL;
}

m_flNextSecondaryAttack = gpGlobals->time + 0.2;
}

void CTEST::Reload( void )
{
UpdateSpot( );
DefaultReload( TEST_MAX_CLIP, TEST_RELOAD, 1.5 );
}

void CTEST::Holster( int skiplocal /* = 0 */ ) // we need this, because the laser-dot must be killed, else he
{ // is in the map until we use it again
m_fInReload = FALSE;// cancel any reload in progress.
if (m_pSpot)
{
m_pSpot->Killed( NULL, GIB_NEVER );
m_pSpot = NULL;
}
}


void CTEST::WeaponIdle( void )
{
UpdateSpot( );

ResetEmptySound( );

m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

if (m_flTimeWeaponIdle > gpGlobals->time)
return;

int iAnim;
switch ( RANDOM_LONG( 0, 1 ) )
{
case 0:
iAnim = TEST_LONGIDLE;
break;

default:
case 1:
iAnim = TEST_IDLE1;
break;
}

SendWeaponAnim( iAnim );

m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );// how long till we do this again.
}



class CTESTAmmoClip : public CBasePlayerAmmo
{
void Spawn( void )
{
Precache( );
SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
CBasePlayerAmmo::Spawn( );
}
void Precache( void )
{
PRECACHE_MODEL ("models/w_9mmARclip.mdl");
PRECACHE_SOUND("items/9mmclip1.wav");
}
BOOL AddAmmo( CBaseEntity *pOther )
{
int bResult = (pOther->GiveAmmo( AMMO_TESTCLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
if (bResult)
{
EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
}
return bResult;
}
};
LINK_ENTITY_TO_CLASS( ammo_testclip, CTESTAmmoClip );
//LINK_ENTITY_TO_CLASS( ammo_9mmAR, CTESTAmmoClip );

// === This is the laser-pointer-code:

CLaserSpotTEST *CLaserSpotTEST::CreateSpotTEST( void )
{
CLaserSpotTEST *pSpot = GetClassPtr( (CLaserSpotTEST *)NULL );
pSpot->Spawn();

pSpot->pev->classname = MAKE_STRING("laser_spot_TEST");

return pSpot;
}
//=========================================================
//=========================================================
void CLaserSpotTEST::Spawn( void )
{
TraceResult tr;

Precache( );
pev->movetype = MOVETYPE_NONE;
pev->solid = SOLID_NOT;

pev->rendermode = kRenderGlow;
pev->renderfx = kRenderFxNoDissipation;
pev->renderamt = 255;

SET_MODEL(ENT(pev), "sprites/laserdot.spr");
UTIL_SetOrigin( pev, pev->origin );
};

//=========================================================
// Suspend- make the laser sight invisible.
//=========================================================
void CLaserSpotTEST::Suspend( float flSuspendTime )
{
pev->effects |= EF_NODRAW;

SetThink( Revive );
pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpotTEST::Revive( void )
{
pev->effects &= ~EF_NODRAW;

SetThink( NULL );
}

void CLaserSpotTEST::Precache( void )
{
PRECACHE_MODEL("sprites/laserdot.spr");
};

void CTEST::UpdateSpot( void )
{
if (m_fSpotActive)
{
if (!m_pSpot)
{
m_pSpot = CLaserSpotTEST::CreateSpotTEST();
}

UTIL_MakeVectors( m_pPlayer->pev->v_angle );
Vector vecSrc = m_pPlayer->GetGunPosition( );;
Vector vecAiming = gpGlobals->v_forward;

TraceResult tr;
UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );

// ALERT( "%f %f\n", gpGlobals->v_forward.y, vecAiming.y );

/*
float a = gpGlobals->v_forward.y * vecAiming.y + gpGlobals->v_forward.x * vecAiming.x;
m_pPlayer->pev->punchangle.y = acos( a ) * (180 / M_PI);

ALERT( at_console, "%f\n", a );
*/

UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );
}
}
