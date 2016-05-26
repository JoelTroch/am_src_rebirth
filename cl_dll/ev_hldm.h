//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( EV_HLDMH )
#define EV_HLDMH

// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, // glock
	BULLET_PLAYER_MP5, // mp5
	BULLET_PLAYER_357, // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR, // crowbar swipe

	BULLET_PLAYER_M16,
	BULLET_PLAYER_AK74,
	BULLET_PLAYER_DEAGLE,
	BULLET_PLAYER_SNIPER,
	BULLET_PLAYER_FAMAS,
	BULLET_PLAYER_M249,
	BULLET_PLAYER_UZI,
	BULLET_PLAYER_BER92F,
	BULLET_PLAYER_GLOCK18,
	BULLET_PLAYER_USAS,
	BULLET_PLAYER_P90,

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

	BULLET_MONSTER_AK,
	BULLET_MONSTER_M16,
	BULLET_MONSTER_DEAGLE,
	BULLET_MONSTER_SNIPER,
	BULLET_PLAYER_IRGUN

} Bullet;

enum glock_e 
{
	GLOCK_IDLE1 = 0,
	GLOCK_RELOAD,
	GLOCK_RELOAD_LAST,
	GLOCK_FIRE1,
	GLOCK_FIRE2,
	GLOCK_FIRE_LAST,
	GLOCK_DEPLOY,
	GLOCK_DEPLOY_VACIO,
	GLOCK_HOLSTER,
	GLOCK_HOLSTER_VACIO,
};
enum shotgun_e 
{
	SHOTGUN_IDLE1 = 0,
	SHOTGUN_IDLE2,

	SHOTGUN_RELOAD_START,
	SHOTGUN_RELOAD_INS_1,
	SHOTGUN_RELOAD_END_LAST,
	
	SHOTGUN_DEPLOY,

	SHOTGUN_FIRE1,
	SHOTGUN_FIRE2,

	SHOTGUN_PUMP1,
	SHOTGUN_PUMP2,

	SHOTGUN_HOLSTER,

	SHOTGUN_NEWANIMSBYME,

	SHOTGUN_NEWFIRE1,
	SHOTGUN_NEWFIRE2
};
/*
enum shotgun_e 
{
	SHOTGUN_IDLE1 = 0,
	SHOTGUN_IDLE2,
	SHOTGUN_FIRE1,
	SHOTGUN_FIRE2,
	SHOTGUN_MMS_1,
	SHOTGUN_MMS_2,
	SHOTGUN_RELOAD_START,
	SHOTGUN_RELOAD_INS_1,
	SHOTGUN_RELOAD_INS_2,
	SHOTGUN_RELOAD_INS_3,
	SHOTGUN_RELOAD_INS_NOM_1,
	SHOTGUN_RELOAD_INS_NOM_2,
	SHOTGUN_RELOAD_END,
	SHOTGUN_RELOAD_END_LAST,
	SHOTGUN_DEPLOY_FIRST,
	SHOTGUN_DEPLOY,
	SHOTGUN_HOLSTER,
};
*/
/*
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
*/

enum python_e {
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

enum irgun_e
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


#define	GAUSS_PRIMARY_CHARGE_VOLUME	256// how loud gauss is while charging
#define GAUSS_PRIMARY_FIRE_VOLUME	450// how loud gauss is when discharged
/*
enum gauss_e {
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};
*/
/////////
void EV_HLDM_MuzzleFlash( vec3_t pos, float amount );
/////////
void SpriteTracer( vec3_t pos, vec3_t oldpos, int modelindex, float r, float g, float b, float a );

//void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName );
void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, int special_weapon );

void EV_HLDM_t( pmtrace_t *pTrace, int iBulletType );
int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount );
//void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY );
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType,
    int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY, float recoil=0);

void	EV_HLDM_BulletFlyBySound( int idx, vec3_t start, vec3_t end );

#endif // EV_HLDMH