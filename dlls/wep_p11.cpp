#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "effects.h"
#include "customentity.h"
#include "gamerules.h"
#include "squadmonster.h"
#include "shake.h"

#define ENERGY_PRIMARY_VOLUME 450
#define ENERGY_BEAM_SPRITE "sprites/xbeam4.spr"
#define ENERGY_FLARE_SPRITE "sprites/rjet1.spr"
#define ENERGY_SOUND_OFF "x/x_ballattack1.wav"
#define ENERGY_SOUND_RUN "debris/beamstart3.wav"
#define ENERGY_SOUND_STARTUP "weapons/mine_charge.wav"
#define GARG_BEAM_SPRITE_NAME "sprites/xbeam3.spr"
#define GARG_BEAM_SPRITE2 "sprites/xbeam3.spr"
#define GARG_FLAME_LENGTH 330

#define ENERGY_SWITCH_NARROW_TIME 0.75 // Time it takes to switch fire modes
#define ENERGY_SWITCH_WIDE_TIME 1.5
#define BALL_AIR_VELOCITY 2000
#define BALL_WATER_VELOCITY 1000
class CEnergyBall : public CBaseEntity
{
void Spawn( void );
void Precache( void );
int Classify ( void );
void EXPORT BubbleThink( void );
void EXPORT BallTouch( CBaseEntity *pOther );
void EXPORT ExplodeThink( void );

int m_iTrail;//CBaseEntity *pEnemy = m_hEnemy;

public:
static CEnergyBall *BallCreate( void );
};
LINK_ENTITY_TO_CLASS( energy_ball, CEnergyBall );


CEnergyBall *CEnergyBall::BallCreate( void )
{
// Create a new entity with CCrossbowBolt private data
CEnergyBall *pBall = GetClassPtr( (CEnergyBall *)NULL );
pBall->pev->classname = MAKE_STRING("energy_ball");
pBall->Spawn();

return pBall;
}

void CEnergyBall :: Spawn( void )
{
Precache( );
pev->movetype = MOVETYPE_FLY;
pev->solid = SOLID_BBOX;

pev->gravity = 0.5;

SET_MODEL(ENT(pev), "sprites/xspark4.spr");//xfireball3
pev->rendermode = kRenderTransAdd;
pev->rendercolor.x = 255;
pev->rendercolor.y = 255;
pev->rendercolor.z = 255;
pev->renderamt = 255;
pev->scale = 0.5;


UTIL_SetOrigin( pev, pev->origin );
UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));


SetTouch( BallTouch );
SetThink( BubbleThink );

pev->nextthink = gpGlobals->time + 0.2;

//m_hOwner = Instance( pev->owner );
//pev->dmgtime = gpGlobals->time;
// keep track of when ball spawned

}


void CEnergyBall :: Precache( void )
{
PRECACHE_MODEL("sprites/xspark4.spr");//playerflameloop
PRECACHE_SOUND("debris/zap4.wav");
PRECACHE_SOUND("weapons/electro4.wav");
}

int CEnergyBall :: Classify ( void )
{
return CLASS_NONE;
}



void CEnergyBall::BallTouch( CBaseEntity *pOther )
{
SetTouch( NULL );
SetThink( NULL );

if (pOther->pev->takedamage)
{
TraceResult tr = UTIL_GetGlobalTrace( );
entvars_t *pevOwner;

pevOwner = VARS( pev->owner );

// UNDONE: this needs to call TraceAttack instead
ClearMultiDamage( );

if ( pOther->IsPlayer() )
{
pOther->TraceAttack(pevOwner, gSkillData.plrDmgCrossbowClient, pev->velocity.Normalize(), &tr, DMG_ENERGYBEAM | DMG_ALWAYSGIB ); 
}
else
{
pOther->TraceAttack(pevOwner, gSkillData.plrDmgCrossbowMonster, pev->velocity.Normalize(), &tr, DMG_ENERGYBEAM | DMG_ALWAYSGIB ); 
}

ApplyMultiDamage( pev, pevOwner );

pev->velocity = Vector( 0, 0, 0 );
// play body "thwack" sound
switch( RANDOM_LONG(0,1) )
{
case 0:
EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/electro4.wav", 1, ATTN_NORM); break;
case 1:
EMIT_SOUND(ENT(pev), CHAN_WEAPON, "debris/zap4.wav", 1, ATTN_NORM); break;
}

if ( !g_pGameRules->IsMultiplayer() )
{
Killed( pev, GIB_NEVER );
}
}
else
{
EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/electro4.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 +
RANDOM_LONG(0,7));

SetThink( SUB_Remove );
pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
{
UTIL_Sparks( pev->origin );
}
}

if ( g_pGameRules->IsMultiplayer() )
{
SetThink( ExplodeThink );
pev->nextthink = gpGlobals->time + 0.1;
}
}


void CEnergyBall::BubbleThink( void )
{
pev->nextthink = gpGlobals->time + 0.1;

if (pev->waterlevel == 0)
return;

UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 1 );
}


void CEnergyBall::ExplodeThink( void )
{
int iContents = UTIL_PointContents ( pev->origin );
int iScale;

pev->dmg = 40;
iScale = 10;

MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
WRITE_BYTE( TE_EXPLOSION);
WRITE_COORD( pev->origin.x );
WRITE_COORD( pev->origin.y );
WRITE_COORD( pev->origin.z );
if (iContents != CONTENTS_WATER)
{
WRITE_SHORT( g_sModelIndexFireball );
}
else
{
WRITE_SHORT( g_sModelIndexWExplosion );
}
WRITE_BYTE( iScale ); // scale * 10
WRITE_BYTE( 15 ); // framerate
WRITE_BYTE( TE_EXPLFLAG_NONE );
MESSAGE_END();

entvars_t *pevOwner;

if ( pev->owner )
pevOwner = VARS( pev->owner );
else
pevOwner = NULL;

pev->owner = NULL; // can''t traceline attack owner if this is set

::RadiusDamage( pev->origin, pev, pevOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB );

UTIL_Remove(this);
}

enum egon_e {
EGON_IDLE1 = 0,
EGON_IDLE2,
EGON_FIRE1,
EGON_FIRE2,
EGON_RELOAD,
EGON_DRAW,
EGON_HOLSTER
};


class CEnergy : public CBasePlayerWeapon
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
void FireBall( void );
BOOL Deploy( void );
void Holster( int skiplocal = 0 );
void SecondaryAttack( void );
void CreateEffect( void );
void UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend );
void DestroyEffect( void );

void EndAttack( void );
void Attack( void );
void PrimaryAttack( void );
void WeaponIdle( void );
static int g_fireAnims1[];
static int g_fireAnims2[];

float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

float GetPulseInterval( void );
float GetDischargeInterval( void );

void Fire( const Vector &vecOrigSrc, const Vector &vecDir );

BOOL HasAmmo( void )
{
if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
return FALSE;
return TRUE;
}

void UseAmmo( int count )
{
if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= count )
m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= count;
else
m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
}

enum ENERGY_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
enum ENERGY_FIREMODE { FIRE_NARROW, FIRE_WIDE};



private:
float m_shootTime;

CBeam *m_pBeam;
CBeam *m_pBeamQ;
CBeam *m_pBeamL;
CBeam *m_pFlame;
CBeam *m_pNoise;
CSprite *m_pSprite;
ENERGY_FIRESTATE m_fireState;
ENERGY_FIREMODE m_fireMode;
float m_shakeTime;
BOOL m_deployed;

};

LINK_ENTITY_TO_CLASS( weapon_p11, CEnergy );

int CEnergy::g_fireAnims1[] = { EGON_FIRE1, EGON_FIRE2 };
int CEnergy::g_fireAnims2[] = { EGON_FIRE1 };


TYPEDESCRIPTION CEnergy::m_SaveData[] = 
{
DEFINE_FIELD( CEnergy, m_pBeam, FIELD_CLASSPTR ),
DEFINE_FIELD( CEnergy, m_pBeamQ, FIELD_CLASSPTR ),
DEFINE_FIELD( CEnergy, m_pBeamL, FIELD_CLASSPTR ),
DEFINE_FIELD( CEnergy, m_pNoise, FIELD_CLASSPTR ),
DEFINE_FIELD( CEnergy, m_pSprite, FIELD_CLASSPTR ),
DEFINE_FIELD( CEnergy, m_shootTime, FIELD_TIME ),
DEFINE_FIELD( CEnergy, m_fireState, FIELD_INTEGER ),
DEFINE_FIELD( CEnergy, m_fireMode, FIELD_INTEGER ),
DEFINE_FIELD( CEnergy, m_shakeTime, FIELD_TIME ),
DEFINE_FIELD( CEnergy, m_flAmmoUseTime, FIELD_TIME ),
DEFINE_ARRAY( CEnergy, m_pFlame, FIELD_CLASSPTR, 4 ),
};
IMPLEMENT_SAVERESTORE( CEnergy, CBasePlayerWeapon );


void CEnergy::Spawn( )
{
Precache( );
m_iId = WEAPON_ENERGY;
SET_MODEL(ENT(pev), "models/weapons/p11/w_p11.mdl");

m_iDefaultAmmo = ENERGY_DEFAULT_GIVE;

FallInit();// get ready to fall down.
}


void CEnergy::Precache( void )
{
PRECACHE_MODEL("models/weapons/p11/w_p11.mdl");
PRECACHE_MODEL("models/weapons/p11/v_p11.mdl");
PRECACHE_MODEL("models/weapons/p11/p_p11.mdl");

PRECACHE_MODEL("sprites/xenobeam.spr");
PRECACHE_MODEL("sprites/plasma.spr");
PRECACHE_MODEL( GARG_BEAM_SPRITE_NAME );
PRECACHE_MODEL( GARG_BEAM_SPRITE2 );

PRECACHE_MODEL("models/w_9mmclip.mdl");
PRECACHE_SOUND("items/9mmclip1.wav");

PRECACHE_SOUND( ENERGY_SOUND_OFF );
PRECACHE_SOUND( ENERGY_SOUND_RUN );
PRECACHE_SOUND( ENERGY_SOUND_STARTUP );

PRECACHE_MODEL( ENERGY_BEAM_SPRITE );
PRECACHE_MODEL( ENERGY_FLARE_SPRITE );

PRECACHE_SOUND ("weapons/357_cock1.wav");


PRECACHE_SOUND("debris/zap4.wav");
PRECACHE_SOUND("weapons/electro4.wav");

UTIL_PrecacheOther( "energy_ball" );
}


BOOL CEnergy::Deploy( void )
{
m_deployed = FALSE;
return DefaultDeploy( "models/weapons/p11/v_p11.mdl", "models/weapons/p11/p_p11.mdl", EGON_DRAW, "deagle" );
}

int CEnergy::AddToPlayer( CBasePlayer *pPlayer )
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



void CEnergy::Holster( int skiplocal /* = 0 */ )
{
m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
// m_flTimeWeaponIdle = gpGlobals->time + UTIL_RandomFloat ( 10, 15 );
SendWeaponAnim( EGON_HOLSTER );

if ( m_fireState != FIRE_OFF )
EndAttack();
}

int CEnergy::GetItemInfo(ItemInfo *p)
{
p->pszName = STRING(pev->classname);
p->pszAmmo1 = "uranium";
p->iMaxAmmo1 = ENERGY_MAX_CARRY;
p->pszAmmo2 = NULL;
p->iMaxAmmo2 = -1;
p->iMaxClip = WEAPON_NOCLIP;
p->iSlot = 2;
p->iPosition = 3;
p->iId = m_iId = WEAPON_ENERGY;
p->iFlags = 0;
p->iWeight = ENERGY_WEIGHT;

return 1;
}



#define ENERGY_PULSE_INTERVAL 0.1
#define ENERGY_DISCHARGE_INTERVAL 0.1

float CEnergy::GetPulseInterval( void )
{
if ( g_pGameRules->IsMultiplayer() )
{
return 0.1;
}

return ENERGY_PULSE_INTERVAL;
}

float CEnergy::GetDischargeInterval( void )
{
if ( g_pGameRules->IsMultiplayer() )
{
return 0.1;
}

return ENERGY_DISCHARGE_INTERVAL;
}

void CEnergy::Attack( void )
{


UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
Vector vecAiming = gpGlobals->v_forward;
Vector vecSrc = m_pPlayer->GetGunPosition( );

switch( m_fireState )
{
case FIRE_OFF:
{
if ( !HasAmmo() )
{
m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.25;
PlayEmptySound( );
return;
}

m_flAmmoUseTime = gpGlobals->time;// start using ammo ASAP.

SendWeaponAnim( g_fireAnims1[ RANDOM_LONG(0,ARRAYSIZE(g_fireAnims1)-1) ] );
m_shakeTime = 0;

m_pPlayer->m_iWeaponVolume = ENERGY_PRIMARY_VOLUME;
m_flTimeWeaponIdle = gpGlobals->time + 0.1;
m_shootTime = gpGlobals->time + 2;

if ( m_fireMode == FIRE_WIDE )
{
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, ENERGY_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125 );
}
else
{
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, ENERGY_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100 );
}

pev->dmgtime = gpGlobals->time + GetPulseInterval();
m_fireState = FIRE_CHARGE;
}
break;

case FIRE_CHARGE:
{
Fire( vecSrc, vecAiming );
m_pPlayer->m_iWeaponVolume = ENERGY_PRIMARY_VOLUME;

if ( m_shootTime != 0 && gpGlobals->time > m_shootTime )
{
if ( m_fireMode == FIRE_WIDE )
{
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_STATIC, ENERGY_SOUND_RUN, 0.98, ATTN_NORM, 0, 125 );
}
else
{
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_STATIC, ENERGY_SOUND_RUN, 0.9, ATTN_NORM, 0, 100 );
}

m_shootTime = 0;
}
if ( !HasAmmo() )
{
EndAttack();
m_fireState = FIRE_OFF;
m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 1.0;
}

}
break;
}
}
void CEnergy::PrimaryAttack( void )
{
FireBall();
}

void CEnergy::SecondaryAttack( void )
{
m_fireMode = FIRE_WIDE;
Attack();

}

void CEnergy::FireBall()
{
TraceResult tr;

if ( !HasAmmo() )
{
m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.25;
PlayEmptySound( );
return;
}

m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;


if ( gpGlobals->time >= m_flAmmoUseTime )
{
UseAmmo( 1 );
m_flAmmoUseTime = gpGlobals->time + 0.2;
}

EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "debris/zap4.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 +
RANDOM_LONG(0,0xF));

if (m_iClip)
{

EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/electro4.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 +
RANDOM_LONG(0,0xF));
SendWeaponAnim( EGON_FIRE1 );
}
else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
{
SendWeaponAnim( EGON_FIRE2 );
}

// player "shoot" animation
m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
UTIL_MakeVectors( anglesAim );

// Vector vecSrc = pev->origin + gpGlobals->v_up * 16 + gpGlobals->v_forward * 20 + gpGlobals->v_right * 4;
anglesAim.x = -anglesAim.x;
Vector vecSrc = m_pPlayer->GetGunPosition( ) - gpGlobals->v_up * 2;
Vector vecDir = gpGlobals->v_forward;

//CBaseEntity *pBolt = CBaseEntity::Create( "crossbow_bolt", vecSrc, anglesAim, m_pPlayer->edict() );
CEnergyBall *pBall = CEnergyBall::BallCreate();
pBall->pev->origin = vecSrc ;
pBall->pev->angles = anglesAim ;
pBall->pev->owner = m_pPlayer->edict();



if (m_pPlayer->pev->waterlevel == 3)
{
pBall->pev->velocity = vecDir * BALL_WATER_VELOCITY;
pBall->pev->speed = BALL_WATER_VELOCITY;
}
else
{
pBall->pev->velocity = vecDir * BALL_AIR_VELOCITY;
pBall->pev->speed = BALL_AIR_VELOCITY;
}
pBall->pev->avelocity.z = 10;

if ( !HasAmmo() )
{
m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 1.0;
}

m_flNextPrimaryAttack = gpGlobals->time + 0.2;

m_flNextSecondaryAttack = gpGlobals->time + 0.2;
if (m_iClip != 0)
m_flTimeWeaponIdle = gpGlobals->time + 3.0;
else
m_flTimeWeaponIdle = 0.1;

//m_pPlayer->pev->punchangle.x -= 2;
}


void CEnergy::Fire( const Vector &vecOrigSrc, const Vector &vecDir )
{
Vector vecDest = vecOrigSrc + vecDir * 2048;
edict_t *pentIgnore;
TraceResult tr;

pentIgnore = m_pPlayer->edict();
Vector tmpSrc = vecOrigSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

// ALERT( at_console, "." );

UTIL_TraceLine( vecOrigSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr );

if (tr.fAllSolid)
return;

CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

if (pEntity == NULL)
return;

if ( g_pGameRules->IsMultiplayer() )
{
if ( m_pSprite && pEntity->pev->takedamage )
{
m_pSprite->pev->effects &= ~EF_NODRAW;
}
else if ( m_pSprite )
{
m_pSprite->pev->effects |= EF_NODRAW;
}
}

float timedist;

switch ( m_fireMode )
{
case FIRE_NARROW:
if ( pev->dmgtime < gpGlobals->time )
{
// Narrow mode only does damage to the entity it hits
ClearMultiDamage();
if (pEntity->pev->takedamage)
{
pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgEgonNarrow, vecDir, &tr, DMG_ENERGYBEAM );
}
ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

if ( g_pGameRules->IsMultiplayer() )
{
// multiplayer uses 1 ammo every 1/10th second
if ( gpGlobals->time >= m_flAmmoUseTime )
{
UseAmmo( 1 );
m_flAmmoUseTime = gpGlobals->time + 0.1;
}
}
else
{
// single player, use 3 ammo/second
if ( gpGlobals->time >= m_flAmmoUseTime )
{
UseAmmo( 1 );
m_flAmmoUseTime = gpGlobals->time + 0.166;
}
}

pev->dmgtime = gpGlobals->time + GetPulseInterval();
}
timedist = ( pev->dmgtime - gpGlobals->time ) / GetPulseInterval();
break;

case FIRE_WIDE:
if ( pev->dmgtime < gpGlobals->time )
{
// wide mode does damage to the ent, and radius damage
ClearMultiDamage();
if (pEntity->pev->takedamage)
{
pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgEgonWide, vecDir, &tr, DMG_ENERGYBEAM | DMG_ALWAYSGIB);
}
ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

if ( g_pGameRules->IsMultiplayer() )
{
// radius damage a little more potent in multiplayer.
::RadiusDamage( tr.vecEndPos, pev, m_pPlayer->pev, gSkillData.plrDmgEgonWide/4, 128, CLASS_NONE, DMG_ENERGYBEAM | DMG_BLAST |
DMG_ALWAYSGIB );
}

if ( !m_pPlayer->IsAlive() )
return;

if ( g_pGameRules->IsMultiplayer() )
{
//multiplayer uses 5 ammo/second
if ( gpGlobals->time >= m_flAmmoUseTime )
{
UseAmmo( 1 );
m_flAmmoUseTime = gpGlobals->time + 0.2;
}
}
else
{
// Wide mode uses 10 charges per second in single player
if ( gpGlobals->time >= m_flAmmoUseTime )
{
UseAmmo( 1 );
m_flAmmoUseTime = gpGlobals->time + 0.1;
}
}

pev->dmgtime = gpGlobals->time + GetDischargeInterval();
if ( m_shakeTime < gpGlobals->time )
{
UTIL_ScreenShake( tr.vecEndPos, 5.0, 150.0, 0.75, 250.0 );
m_shakeTime = gpGlobals->time + 1.5;
}
}
timedist = ( pev->dmgtime - gpGlobals->time ) / GetDischargeInterval();
break;
}

if ( timedist < 0 )
timedist = 0;
else if ( timedist > 1 )
timedist = 1;
timedist = 1-timedist;

UpdateEffect( tmpSrc, tr.vecEndPos, timedist );
}


void CEnergy::UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend )
{
if ( !m_pBeam )
{
CreateEffect();
}

m_pBeam->SetStartPos( endPoint );
m_pBeam->SetBrightness( 255 - (timeBlend*180) );
m_pBeam->SetWidth( 40 - (timeBlend*20) );

m_pFlame->SetStartPos( endPoint );
m_pFlame->SetBrightness( 255 - (timeBlend*180) );
m_pFlame->SetWidth( 40 - (timeBlend*20) );

// m_pBeamL->SetStartPos( endPoint );

if ( m_fireMode == FIRE_WIDE )
m_pBeam->SetColor( 30 + (25*timeBlend), 30 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );
else
m_pBeam->SetColor( 60 + (25*timeBlend), 120 + (30*timeBlend), 64 + 80*fabs(sin(gpGlobals->time*10)) );


UTIL_SetOrigin( m_pSprite->pev, endPoint );
m_pSprite->pev->frame += 8 * gpGlobals->frametime;
if ( m_pSprite->pev->frame > m_pSprite->Frames() )
m_pSprite->pev->frame = 0;

m_pNoise->SetStartPos( endPoint );
}


void CEnergy::CreateEffect( void )
{
DestroyEffect();



m_pBeam = CBeam::BeamCreate( "sprites/xenobeam.spr", 30 );
m_pBeam->PointEntInit( pev->origin, m_pPlayer->entindex() );
m_pBeam->SetScrollRate( 25 );
m_pBeam->SetBrightness( 100 );
m_pBeam->SetEndAttachment( 1 );
m_pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY; // Flag these to be destroyed on save/restore or level transition

m_pNoise = CBeam::BeamCreate( "sprites/plasma.spr", 55 );
m_pNoise->PointEntInit( pev->origin, m_pPlayer->entindex() );
m_pNoise->SetScrollRate( 25 );
m_pNoise->SetBrightness( 100 );
m_pNoise->SetEndAttachment( 1 );
m_pNoise->pev->spawnflags |= SF_BEAM_TEMPORARY;

m_pSprite = CSprite::SpriteCreate( ENERGY_FLARE_SPRITE, pev->origin, FALSE );
m_pSprite->pev->scale = 1.0;
m_pSprite->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
m_pSprite->pev->spawnflags |= SF_SPRITE_TEMPORARY;

m_pFlame = CBeam::BeamCreate( GARG_BEAM_SPRITE_NAME, 240 );
m_pFlame->PointEntInit( pev->origin, m_pPlayer->entindex());
m_pFlame->SetColor( 255, 130, 90 );
m_pFlame->SetBrightness( 190 );
m_pFlame->SetFlags( BEAM_FSHADEIN );
m_pFlame->SetScrollRate( 20 );
// attachment is 1 based in SetEndAttachment
m_pFlame->SetEndAttachment( 1 );
m_pFlame->pev->spawnflags |= SF_BEAM_TEMPORARY;

//m_pBeam->SetScrollRate( 50 );
m_pBeam->SetNoise( 35 );
// m_pNoise->SetScrollRate( 50 );
m_pNoise->SetNoise( 35 );

//m_pBeamL->SetScrollRate( 60 );
// m_pBeamL->SetNoise( 25 );

// m_pBeamQ->SetScrollRate( 70 );
// m_pBeamQ->SetNoise( 30 );

//m_pFlame->SetScrollRate( 100 );
m_pFlame->SetNoise( 35 );





}


void CEnergy::DestroyEffect( void )
{
if ( m_pBeam )
{
UTIL_Remove( m_pBeam );
m_pBeam = NULL;
}
if ( m_pFlame )
{
UTIL_Remove( m_pFlame );
m_pFlame = NULL;
}

if ( m_pNoise )
{
UTIL_Remove( m_pNoise );
m_pNoise = NULL;
}
if ( m_pSprite )
{
if ( m_fireMode == FIRE_WIDE )
m_pSprite->Expand( 10, 500 );
else
UTIL_Remove( m_pSprite );
m_pSprite = NULL;
}
}


void CEnergy::WeaponIdle( void )
{
ResetEmptySound( );

if ( m_flTimeWeaponIdle > gpGlobals->time )
return;

if ( m_fireState != FIRE_OFF )
EndAttack();


int iAnim;

float flRand = RANDOM_FLOAT(0,1);

if ( flRand <= 0.5 )
{
iAnim = EGON_IDLE1;
m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10,15);
}
else 
{
iAnim = EGON_IDLE2;
m_flTimeWeaponIdle = gpGlobals->time + 3;
}

SendWeaponAnim( iAnim );
m_deployed = TRUE;
}



void CEnergy::EndAttack( void )
{
STOP_SOUND( ENT(m_pPlayer->pev), CHAN_STATIC, ENERGY_SOUND_RUN );
EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, ENERGY_SOUND_OFF, 0.98, ATTN_NORM, 0, 100); 
m_fireState = FIRE_OFF;
m_flTimeWeaponIdle = gpGlobals->time + 2.0;
m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.5;
DestroyEffect();
}



class CEnergyAmmo : public CBasePlayerAmmo
{
void Spawn( void )
{ 
Precache( );
SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
CBasePlayerAmmo::Spawn( );
}
void Precache( void )
{
PRECACHE_MODEL ("models/w_chainammo.mdl");
PRECACHE_SOUND("items/9mmclip1.wav");
}
BOOL AddAmmo( CBaseEntity *pOther ) 
{ 
if (pOther->GiveAmmo( AMMO_URANIUMBOX_GIVE, "uranium", URANIUM_MAX_CARRY ) != -1)
{
EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
return TRUE;
}
return FALSE;
}
};
LINK_ENTITY_TO_CLASS( ammo_p11, CEnergyAmmo );
