/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef WEAPONS_H
#define WEAPONS_H

#include "effects.h"

class CBasePlayer;
extern int gmsgWeapPickup;

void DeactivateSatchels( CBasePlayer *pOwner );


// Contact Grenade / Timed grenade / Satchel Charge / Cluster Grenade (SP)
class CGrenade : public CBaseMonster
{
public:
	void Spawn( void );

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;
//
	static CGrenade *ShootTimedCluster( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootFragment( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	void EXPORT ClusterTumbleThink( void );

	void EXPORT ClusterDetonate( CBaseEntity *pOther );

	static CGrenade *FlashShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	void EXPORT FlashTumbleThink( void );

	static CGrenade *ShootTimedPlayer( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, CBasePlayer *pPlayer );
	void RestoreVol( void );
//
	static CGrenade *ShootTimedSmoke( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootSmokeFragment( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	void EXPORT SmokeTumbleThink( void );
	void EXPORT SmokeDetonate( void );
	void SmokeGren( TraceResult *pTrace );

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	static CGrenade *ShootFlashbang( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootTimedCz( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	
	static CGrenade *ShootTimedCombine( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );

	void Explode( Vector vecSrc, Vector vecAim );
	void Explode( TraceResult *pTrace, int bitsDamageType );
	void EXPORT Smoke( void );
	void Flash( TraceResult *pTrace );
	static CGrenade *ShootC4( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	
	static CGrenade *ShootSfera( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	void SferaExplode( TraceResult *pTrace );

	int m_idFrag_gibs;
	void EXPORT FragGibs (void);

	void LegsGibs( void );
	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT DangerSoundThink( void );
	void EXPORT PreDetonate( void );
	void EXPORT Detonate( void );
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TumbleThink( void );

	virtual void BounceSound( void );
	virtual void SferaBounceSound( void );
	virtual int	BloodColor( void ) { return DONT_BLEED; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );

	BOOL m_fRegisteredSound;// whether or not this grenade has issued its DANGER sound to the world sound list yet.
	BOOL m_bIsFlashbang;

	BOOL m_bIsSfera;
		
	BOOL m_bIsSmoke;
		
	BOOL m_bIsCombineGrenade;
	int b_iNextPipSound;
};

// constant items
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4

#define WEAPON_NONE				0

#define	WEAPON_HL_CROSSBOW	1
#define	WEAPON_HL_DEAGLE	2
#define	WEAPON_HL_EGON		3
#define	WEAPON_HL_GAUSS		4
#define	WEAPON_HL_HORNETGUN	5
#define	WEAPON_HL_M16		6
#define	WEAPON_HL_SNIPER	7
#define	WEAPON_HL_MP5		8
#define	WEAPON_HL_PYTHON	9
#define	WEAPON_HL_RPG		10
#define	WEAPON_HL_SATCHEL	11
#define	WEAPON_HL_SAW		12
#define	WEAPON_HL_SNARK		13
#define	WEAPON_HL_TRIPMINE	14
#define	WEAPON_HL_MEDKIT	15
#define WEAPON_HL_HANDGRENADE	16
#define WEAPON_HL_CROWBAR		17
#define WEAPON_HL_GLOCK			18
#define WEAPON_HL_357			19
#define WEAPON_HL_SHOTGUN		21





#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				31	// ?????

#define MAX_WEAPONS			82

#define MAX_NORMAL_BATTERY	100


//=========================
// SETTINGS FOR EACH WEAPON
//=========================

//Desert eagle settings (weapon_eagle)
#define DESERT_MAX_CLIP			7
#define DESERT_WEIGHT			15
#define DESERT_DEFAULT_GIVE			7
#define DESERT_RANDOM_GIVE			RANDOM_LONG( 4, 7 )
#define DESERT_MAX_CARRY			21
#define DESERT_LASER_FOCUS			850

//saw settings (weapon_m249)
#define SAW_WEIGHT				25
#define SAW_MAX_CARRY			200
#define SAW_MAX_CLIP			100
#define SAW_DEFAULT_GIVE			100
#define SAW_RANDOM_GIVE			RANDOM_LONG( 20, 100 )


//m40a1 settings (weapon_m40a1)
#define M40A1_WEIGHT			25
#define M40A1_MAX_CARRY			10
#define M40A1_MAX_CLIP			5
#define M40A1_DEFAULT_GIVE			5
#define M40A1_RANDOM_GIVE			RANDOM_LONG( 1, 5 )
#define MAX_ZOOM				15

#define    IRGUN_WEIGHT    30
#define    IRGUN_MAX_CARRY 150
#define    IRGUN_MAX_CLIP  30
#define    IRGUN_MAX_DEFAULT_GIVE 30
#define    IRGUN_MAX_DEFAULT_AMMO 30
#define    AMMO_IRGUNCLIP_GIVE		IRGUN_MAX_CLIP

// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define CROWBAR_WEIGHT		0
#define GLOCK_WEIGHT		10
#define PYTHON_WEIGHT		15
#define MP5_WEIGHT			15
#define SHOTGUN_WEIGHT		15
#define CROSSBOW_WEIGHT		10
#define RPG_WEIGHT			20
#define GAUSS_WEIGHT		20
//#define EGON_WEIGHT			20
#define HORNETGUN_WEIGHT	10
#define HANDGRENADE_WEIGHT	5
#define SNARK_WEIGHT		5
#define SATCHEL_WEIGHT		-10
#define TRIPMINE_WEIGHT		-10
//SP: nuevas
#define SVD_WEIGHT			18

#define CLUSTERGRENADE_WEIGHT 8
#define M16_WEIGHT	        18
#define AK74_WEIGHT	        19
#define DEAGLE_WEIGHT	    15
#define SNIPER_WEIGHT	    20
#define FAMAS_WEIGHT	    17
#define M249_WEIGHT	        25
#define UZI_WEIGHT	        16
#define BER92F_WEIGHT	    11
#define GLOCK18_WEIGHT	    10
#define USAS_WEIGHT		    25
#define P90_WEIGHT	        16
#define LX_WEIGHT	        20
#define	M4A2_WEIGHT			20

#define	LX_DEFAULT_GIVE		25
#define	LX_MAX_AMMO			25

#define CLUSTERGRENADE_MAX_CARRY 5
#define CLUSTERGRENADE_MAX_CLIP WEAPON_NOCLIP
#define CLUSTERGRENADE_DEFAULT_GIVE 1

#define M16_DEFAULT_AMMO		30 
#define M16_MODEL_1STPERSON "models/weapons/m4a1/v_m4a1.mdl" 
#define M16_MODEL_3RDPERSON "models/weapons/m4a1/p_m4a1.mdl" 
#define M16_MODEL_WORLD     "models/weapons/m4a1/w_m4a1.mdl" 
#define M16_SOUND_FIRE1     "weapons/m4a1/m4a1_fire-1.wav" 
#define M16_SOUND_FIRE2     "weapons/m4a1/m4a1_fire-2.wav" 
#define M16_SOUND_VOLUME    0.85 
#define M16_FIRE_DELAY      0.10 // was: glock's is 0.2 
#define M16_RELOAD_TIME     2.8
//#define M16_SLOT            2
//#define M16_POSITION        3

#define	DEAGLE_MAX_CARRY		35
#define DEAGLE_MAX_CLIP			7
#define DEAGLE_DEFAULT_GIVE		7

//p11
// ENERGY
#define ENERGY_MAX_CARRY		50
#define ENERGY_DEFAULT_GIVE		50
#define ENERGY_WEIGHT			20

//#define	BER92F_MAX_CARRY		35
#define BER92F_MAX_CLIP			15
#define BER92F_DEFAULT_GIVE		15

#define GLOCK18_MAX_CLIP			20
#define GLOCK18_DEFAULT_GIVE		20

// USAS
#define USAS_MAX_CLIP			20
#define USAS_DEFAULT_GIVE		20

//USP
#define USP_DEFAULT_ANIM    0
#define USP_DEFAULT_GIVE    12
#define AMMO_USP_GIVE        USP_MAX_CLIP
#define USP_MAX_CLIP		12
#define USP_MAX_CARRY		90

#define ELITE_WEIGHT 10
//P90
#define P90_MAX_CLIP		50
#define P90_DEFAULT_GIVE	50
#define	P90_MAX_CARRY		150
//
#define OICW_MAX_CLIP		30
#define OICW_DEFAULT_GIVE	30
#define	OICW_MAX_CARRY		90

#define SNIPER_MODEL_1STPERSON "models/weapons/sniper/v_sniper.mdl" 
#define SNIPER_MODEL_3RDPERSON "models/weapons/sniper/p_sniper.mdl" 
#define SNIPER_MODEL_WORLD     "models/weapons/sniper/w_sniper.mdl" 
#define SNIPER_SOUND_FIRE1     "weapons/sniper/sniper_fire-1.wav" 
#define SNIPER_SOUND_FIRE2     "weapons/sniper/sniper_fire-2.wav" 
#define SNIPER_SOUND_VOLUME    0.80 
//#define SNIPER_FIRE_DELAY      1.95 // 1.5
//#define SNIPER_RELOAD_TIME     3.4 //2.9
#define SNIPER_DEFAULT_AMMO    10 
#define SNIPER_MAX_AMMO        10//30
#define SNIPER_SLOT            3
#define SNIPER_POSITION        0

#define FAMAS_MODEL_1STPERSON "models/weapons/famas/v_famas.mdl" 
#define FAMAS_MODEL_3RDPERSON "models/weapons/famas/p_famas.mdl" 
#define FAMAS_MODEL_WORLD     "models/weapons/famas/w_famas.mdl" 
#define FAMAS_SOUND_FIRE1     "weapons/famas/famas_fire-1.wav" 
#define FAMAS_SOUND_FIRE2     "weapons/famas/famas_fire-2.wav" 
#define FAMAS_SOUND_VOLUME    0.95 
#define FAMAS_FIRE_DELAY      0.05 // was: glock's is 0.2 
#define FAMAS_RELOAD_TIME     3.2
#define FAMAS_DEFAULT_AMMO    25 
//#define FAMAS_SLOT            2
//#define FAMAS_POSITION        5

// M249
#define M249_MODEL_1STPERSON "models/weapons/M249/v_M249.mdl" 
#define M249_MODEL_3RDPERSON "models/weapons/M249/p_M249.mdl" 
#define M249_MODEL_WORLD     "models/weapons/M249/w_M249.mdl" 
#define M249_SOUND_FIRE1     "weapons/M249/M249_fire-1.wav" 
#define M249_SOUND_FIRE2     "weapons/M249/M249_fire-2.wav" 
#define M249_SOUND_VOLUME    0.95 
#define M249_FIRE_DELAY      0.2 // was: glock's is 0.2 //
#define M249_RELOAD_TIME     4.8
#define M249_DEFAULT_AMMO    100 
#define M249_MAX_AMMO        200//250

//AK
#define AK74_MODEL_1STPERSON "models/weapons/ak-47/v_AK47.mdl" 
#define AK74_MODEL_3RDPERSON "models/weapons/ak-47/p_AK47.mdl" 
#define AK74_MODEL_WORLD     "models/weapons/ak-47/w_AK47.mdl" 
//#define AK74_SOUND_FIRE1     "weapons/ak-47/AK47_fire-1.wav" 
//#define AK74_SOUND_FIRE2     "weapons/ak-47/AK47_fire-2.wav" 
#define AK74_SOUND_VOLUME    0.95 
#define AK74_FIRE_DELAY      0.1 // was: glock's is 0.2 //
//#define AK74_RELOAD_TIME     3.8
#define AK74_DEFAULT_AMMO    30 
#define AK74_MAX_AMMO        90//150

//*****
#define M16_DEFAULT_GIVE	30
#define AMMO_M16CLIP_GIVE	30
#define _556_MAX_AMMO			120//150
// no redondear??
#define MP5_DEFAULT_ANIM    0

// weapon clip/carry ammo capacities
#define URANIUM_MAX_CARRY		100
#define	_9MM_MAX_CARRY			150//250
#define _357_MAX_CARRY			36
#define BUCKSHOT_MAX_CARRY		60//125
#define BOLT_MAX_CARRY			50
#define ROCKET_MAX_CARRY		2//5
#define HANDGRENADE_MAX_CARRY	10
#define SATCHEL_MAX_CARRY		5
#define TRIPMINE_MAX_CARRY		5
#define SNARK_MAX_CARRY			15
#define HORNET_MAX_CARRY		8
#define M203_GRENADE_MAX_CARRY	15//10

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

//#define CROWBAR_MAX_CLIP		WEAPON_NOCLIP
#define GLOCK_MAX_CLIP			8// colt 1911
#define PYTHON_MAX_CLIP			6
#define MP5_MAX_CLIP			30
#define MP5_DEFAULT_AMMO		30
#define SHOTGUN_MAX_CLIP		8
#define CROSSBOW_MAX_CLIP		5
#define RPG_MAX_CLIP			1
#define GAUSS_MAX_CLIP			WEAPON_NOCLIP
//#define EGON_MAX_CLIP			WEAPON_NOCLIP
#define HORNETGUN_MAX_CLIP		WEAPON_NOCLIP
#define HANDGRENADE_MAX_CLIP	WEAPON_NOCLIP
#define SATCHEL_MAX_CLIP		WEAPON_NOCLIP
#define TRIPMINE_MAX_CLIP		WEAPON_NOCLIP
#define SNARK_MAX_CLIP			WEAPON_NOCLIP


// the default amount of ammo that comes with each gun when it spawns
#define GLOCK_DEFAULT_GIVE			8 //1911
#define PYTHON_DEFAULT_GIVE			6
#define MP5_DEFAULT_GIVE			30
#define MP5_DEFAULT_AMMO			30
#define MP5_M203_DEFAULT_GIVE		0
#define SHOTGUN_DEFAULT_GIVE		12
#define CROSSBOW_DEFAULT_GIVE		5
#define RPG_DEFAULT_GIVE			1
#define GAUSS_DEFAULT_GIVE			20
//#define EGON_DEFAULT_GIVE			20
#define HANDGRENADE_DEFAULT_GIVE	5 //5?? Pero si es solo una!
#define SATCHEL_DEFAULT_GIVE		1
#define TRIPMINE_DEFAULT_GIVE		1
#define SNARK_DEFAULT_GIVE			5
#define HIVEHAND_DEFAULT_GIVE		8

// The amount of ammo given to a player by an ammo item.
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_GLOCKCLIP_GIVE		GLOCK_MAX_CLIP
#define AMMO_357BOX_GIVE		PYTHON_MAX_CLIP
#define AMMO_MP5CLIP_GIVE		MP5_MAX_CLIP
#define AMMO_CHAINBOX_GIVE		100//200
#define AMMO_M203BOX_GIVE		2
#define AMMO_BUCKSHOTBOX_GIVE	12
#define AMMO_CROSSBOWCLIP_GIVE	CROSSBOW_MAX_CLIP
#define AMMO_RPGCLIP_GIVE		RPG_MAX_CLIP
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_SNARKBOX_GIVE		5

#define    WEAPON_MP7    1
#define    MP7_WEIGHT    40
#define    MP7_MAX_CARRY 250
#define    MP7_MAX_CLIP  30
#define    MP7_MAX_DEFAULT_GIVE 250
#define    MP7_MAX__DEFAULT_AMMO 250
#define    AMMO_MP7CLIP_GIVE		MP7_MAX_CLIP

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
	BULLET_PLAYER_SCOUT,
	BULLET_PLAYER_FAMAS,
	BULLET_PLAYER_M249,
	BULLET_PLAYER_UZI,
	BULLET_PLAYER_BER92F,
	BULLET_PLAYER_GLOCK18,
	BULLET_PLAYER_USAS,
	BULLET_PLAYER_P90,
	BULLET_PLAYER_M82,

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
	BULLET_PLAYER_IRGUN
} Bullet;


#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char	*pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char	*pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char	*pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.

	const char *weaponName;    //***** NEW
} ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
} AmmoInfo;

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void );										//

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer );	// return TRUE if the item you want the item added to the player inventory
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	// return TRUE if you want your duplicate removed from world
	void EXPORT DestroyItem( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther );	// default weapon touch

	//sys add
	void	BounceCode( void );
	void NormalizeAngles( float *angles );


	void EXPORT FallThink ( void );// when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize( void );// make a weapon visible and tangible
	void EXPORT AttemptToMaterialize( void );  // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn ( void );// copy a weapon
	void FallInit( void );
	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; };	// returns 0 if struct not filled out
	virtual BOOL CanDeploy( void ) { return TRUE; };
	virtual BOOL Deploy( )								// returns is deploy was successful
		 { return TRUE; };

	virtual BOOL CanHolster( void ) { return TRUE; };// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; };

	virtual void ItemPreFrame( void )	{ return; }		// called each frame by the player PreThink
	virtual void ItemPostFrame( void ) { return; }		// called each frame by the player PostThink

	virtual void Drop( void );
	virtual void Kill( void );
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; };

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int		m_iId;												// WEAPON_???

	virtual int iItemSlot( void ) { return 0; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo
	int modelindexsave; 
	string_t modelsave;
	//SP FIX
};


// inventory items that 
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	BOOL CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen);
	//sys test
//	#ifndef CLIENT_DLL 								//AJH for lockable weapons
//	virtual void	KeyValue( KeyValueData* pkvd);	//
//	#endif	

	static	TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon ); //{ return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );// { return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up

	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };	// Return TRUE if you want to add yourself to the player

	// generic "shared" ammo handlers
	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

//	virtual void UpdateItemInfo( void ) {};	// updates HUD state
	virtual void UpdateItemInfo( void );

	int m_iPlayEmptySound;
	int m_fFireOnEmpty;		// True when the gun is empty and the player is still holding down the
							// attack key(s)
	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );  // skiplocal is 1 if client is predicting weapon animations

	virtual BOOL CanDeploy( void );
	virtual BOOL IsUseable( void );
	BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0, int body = 0 );
	int DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );

	virtual void ItemPostFrame( void );	// called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack( void ) { return; }				// do "+ATTACK"
	virtual void SecondaryAttack( void ) { return; }			// do "+ATTACK2"
	virtual void Reload( void ) { return; }						// do "+RELOAD"
	virtual void WeaponIdle( void ) { return; }					// called when no buttons pressed
	virtual int UpdateClientData( CBasePlayer *pPlayer );		// sends hud info to client dll, if things have changed
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) {return FALSE; };
	virtual void Holster( int skiplocal = 0 );
	virtual BOOL UseDecrement( void ) { return FALSE; };
	
	int	PrimaryAmmoIndex(); 
	int	SecondaryAmmoIndex(); 

	//LRC - used by weaponstrip
	void DrainClip(CBasePlayer* pPlayer, BOOL keep, int i9mm, int i357, int iBuck, int iBolt, int iARGren, int iRock, int iUranium, int iSatchel, int iSnark, int iTrip, int iGren );

	void PrintState( void );

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return (CBasePlayerItem *)this; };

	float m_flPumpTime;
	int		m_fInSpecialReload;									// Are we in the middle of a reload for the shotguns
	float	m_flNextPrimaryAttack;								// soonest time ItemPostFrame will call PrimaryAttack
	float	m_flNextSecondaryAttack;							// soonest time ItemPostFrame will call SecondaryAttack
	float	m_flTimeWeaponIdle;									// soonest time ItemPostFrame will call WeaponIdle
	int		m_iPrimaryAmmoType;									// "primary" ammo index into players m_rgAmmo[]
	int		m_iSecondaryAmmoType;								// "secondary" ammo index into players m_rgAmmo[]
	int		m_iClip;											// number of shots left in the primary weapon clip, -1 it not used
	int		m_iClientClip;										// the last version of m_iClip sent to hud dll
	int		m_iClientWeaponState;								// the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int		m_fInReload;										// Are we in the middle of a reload;

	int		m_iDefaultAmmo;// how much ammo you get when you pick up this weapon as placed by a level designer.
	
	int     m_fDefaultAnim; //SP
	int		m_fFireMode;
	BOOL	m_fCrosshairOff; //si es true, no crosshair, si es flaso, si crooshair
	BOOL	m_fCanUseFlashlight; //
	float	m_flTimeUpdate;									// special time for additional effects

	int 		m_iChargeLevel;//level of energy charge
	int 		m_iOverloadLevel;//level of overload weapon
};


class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );
};


extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char *g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)
extern DLL_GLOBAL	short	g_sModelIndexNull;// holds the sprite index for blood spray (bigger)

//sys
extern DLL_GLOBAL	short	g_sModelIndexFireball2;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexFireball3;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexFireballEnd;// holds the index for the fireball
/*
extern DLL_GLOBAL int		g_sModelIndexErrorSprite;
extern DLL_GLOBAL int		g_sModelIndexErrorModel;
extern DLL_GLOBAL int		g_sModelIndexNullModel;
extern DLL_GLOBAL int		g_sModelIndexNullSprite;
*/
//combat test
extern DLL_GLOBAL	short	g_sGrenadeGib;// holds the index for the fireball

extern DLL_GLOBAL	short	g_sMaterialsGib;// holds the index for the fireball
/*extern DLL_GLOBAL	short	g_sConcreteGib;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sWoodGib;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sMetalGib;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sGlassGib;// holds the index for the fireball

extern DLL_GLOBAL	short	m_iSmokeTrail;// holds the index for the fireball
*/
extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

typedef struct 
{
	CBaseEntity		*pEntity;
	float			amount;
	int				type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;


#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128
#define UNDETECTABLE_GUN_FLASH    0

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )

#define VECTOR_CONE_PERFECT		Vector( 0.00001, 0.00001, 0.00001 )
#define VECTOR_CONE_BAD			Vector( 0.27365, 0.27365, 0.27365 )
#define VECTOR_CONE_TOOBAD		Vector( 0.47365, 0.47365, 0.47365 )
//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo. 
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache( void );
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	BOOL IsEmpty( void );
	int  GiveAmmo( int iCount, char *szName, int iMax, int *pIndex = NULL );
	void SetObjectCollisionBox( void );
	//test
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	void BounceCode( void );
	void EXPORT WeaponBoxThink( void );

	void NormalizeAngles( float *angles );

	BOOL bDoCrowbarSound;
	BOOL bDoHESound;

public:
	void EXPORT Kill ( void );
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	HasWeapon( CBasePlayerItem *pCheckItem );
	BOOL PackWeapon( CBasePlayerItem *pWeapon );
	BOOL PackAmmo( int iszName, int iCount );
	
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each 

	int m_rgiszAmmo[MAX_AMMO_SLOTS];// ammo names
	int	m_rgAmmo[MAX_AMMO_SLOTS];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};

#ifdef CLIENT_DLL
bool bIsMultiplayer ( void );
void LoadVModel ( char *szViewModel, CBasePlayer *m_pPlayer );
#endif



class CCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	int Swing( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	TraceResult m_trHit;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usCrowbar;
};


class CPython : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flSoundDelay;

	int m_fInReload;
	float m_flNextReload;

	BOOL m_fInZoom;// don't save this. 

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFirePython;
};

class CM16 : public CBasePlayerWeapon
{
public:
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
	float m_flNextAnimTime;
	int m_iShell;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usM16;
	unsigned short m_usM162;
};

class CCrossbow : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( ) { return 3; }
	int GetItemInfo(ItemInfo *p);

	void FireBolt( void );
	void FireSniperBolt( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy( );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );

	int m_fInZoom; // don't save this

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usCrossbow;
};

class CHL2Shotgun : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif


	void Spawn( void );
	void Precache( void );
	int iItemSlot( ) { return 3; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	void Holster( int skiplocal = 0 );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( );
	void Reload( void );
	void WeaponIdle( void );
	int m_fInReload;
	float m_flNextReload;
	int m_iShell;

	BOOL NeedPump;//you're fired and need pump the gun?
	void EXPORT SpasShell( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usDoubleFire;
	unsigned short m_usSingleFire;
	unsigned short m_usRealDoubleFire;
};

class CShotgun : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif


	void Spawn( void );
	void Precache( void );
	int iItemSlot( ) { return 3; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	void Holster( int skiplocal = 0 );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( );
	void Reload( void );
	void WeaponIdle( void );
	int m_fInReload;
	float m_flNextReload;
	int m_iShell;

	BOOL NeedPump;//you're fired and need pump the gun?
	void EXPORT SpasShell( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usDoubleFire;
	unsigned short m_usSingleFire;
	unsigned short m_usSpasShell;
};

class CLaserSpot : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );

	int	ObjectCaps( void ) { return FCAP_DONT_SAVE; }

public:
	void Suspend( float flSuspendTime );
	void EXPORT Revive( void );
	
	static CLaserSpot *CreateSpot( void );
};

class CRpg : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	void Reload( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal = 0 );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle( void );

	void UpdateSpot( void );
	BOOL ShouldWeaponIdle( void ) { return TRUE; };

	CLaserSpot *m_pSpot;
	int m_fSpotActive;
	int m_cActiveRockets;// how many missiles in flight from this launcher right now?

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usRpg;

};

class CRpgRocket : public CGrenade
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	void Spawn( void );
	void Precache( void );
	void EXPORT FollowThink( void );
	void EXPORT IgniteThink( void );
	void EXPORT RocketTouch( CBaseEntity *pOther );
	static CRpgRocket *CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher );

	int m_iTrail;
	float m_flIgniteTime;
	CRpg *m_pLauncher;// pointer back to the launcher that fired me. 
};

class CGauss : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	BOOL Deploy( void );
	void Holster( int skiplocal = 0  );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle( void );
	
	void StartFire( void );
	void Fire( Vector vecOrigSrc, Vector vecDirShooting, float flDamage );
	float GetFullChargeTime( void );
	int m_iBalls;
	int m_iGlow;
	int m_iBeam;
	int m_iSoundState; // don't save this

	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects. 
	BOOL m_fPrimaryFire;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usGaussFire;
	unsigned short m_usGaussSpin;
};

class CEgon : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );

	void UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend );

	void CreateEffect ( void );
	void DestroyEffect ( void );

	void EndAttack( void );
	void Attack( void );
	void PrimaryAttack( void );
	void WeaponIdle( void );

	float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval( void );
	float GetDischargeInterval( void );

	void Fire( const Vector &vecOrigSrc, const Vector &vecDir );

	BOOL HasAmmo( void );

	void UseAmmo( int count );
	
	enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE};

	CBeam				*m_pBeam;
	CBeam				*m_pNoise;
	CSprite				*m_pSprite;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

	unsigned short m_usEgonStop;

private:
	float				m_shootTime;
	EGON_FIREMODE		m_fireMode;
	float				m_shakeTime;
	BOOL				m_deployed;

	unsigned short m_usEgonFire;
};

class CHgun : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL IsUseable( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;

	float m_flRechargeTime;
	
	int m_iFirePhase;// don't save me.

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usHornetFire;
};

class CHandGrenade : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );

	void SecondaryAttack( void );
//	void SmokeThink( void );

	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	
	int m_fGrenMode; // don't save this

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CSatchel : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int AddDuplicate( CBasePlayerItem *pOriginal );
	BOOL CanDeploy( void );
	BOOL Deploy( void );
	BOOL IsUseable( void );
	
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	void Throw( void );
	
	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CTripmine : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	void SetObjectCollisionBox( void )
	{
		//!!!BUGBUG - fix the model!
		pev->absmin = pev->origin + Vector(-16, -16, -5);
		pev->absmax = pev->origin + Vector(16, 16, 28); 
	}
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usTripFire;

};

class CSqueak : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	int m_fJustThrown;
	int AddToPlayer( CBasePlayer *pPlayer );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usSnarkFire;
};


class CSniper : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( ) { return 4; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
//	int SecondaryAmmoIndex( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy( );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	int m_iShell;

	int m_fInZoom; // This Will Set You That You Can Snipe
	//int m_fFireMode;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireSniper;
	unsigned short m_usSniperShell;
};

class CSniperAmmoClip : public CBasePlayerAmmo
{
    virtual void Spawn( void );    
	virtual void Precache( void );
    virtual BOOL AddAmmo( CBaseEntity *pOther ) ;
};

// CLASE FAMAS | ARMA Y MUNICION

class CFamas : public CBasePlayerWeapon
{
public:    

	void Spawn( void );
    void Precache( void );
    int  iItemSlot( void ) { return 2; }
    int  GetItemInfo(ItemInfo *p);
    int  AddToPlayer( CBasePlayer *pPlayer );    
	void PrimaryAttack( void );
    void SecondaryAttack( void );
	BOOL Deploy( void );
    void Reload( void );    
	void WeaponIdle( void );  
	int  m_iShell;

	void FireSemi( void );
	void FireBurst( void );
	//Add a true/false for firing state
//	BOOL m_iFiring;

	virtual BOOL UseDecrement( void )
{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
}

private:    
    unsigned short m_event;
	unsigned short m_event2;
};
	

class CFamasAmmoClip : public CBasePlayerAmmo
{
    /*virtual*/ void Spawn( void );    
	/*virtual*/ void Precache( void );
    /*virtual*/ BOOL AddAmmo( CBaseEntity *pOther ) ;
};

class CHand : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 9; }
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
//	void SecondaryAttack( void );
	int SwingH( int fFirst );
//	int Swing2( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwingH;

	TraceResult m_trHit;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usHand;

};

class CM249 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	//void SecondaryAttack( void );
	//int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usM249;
};

// NEW uzi CLASS (ammo class stored in cpp file)
class CUZI : public CBasePlayerWeapon
{
public:
	#ifndef CLIENT_DLL
    int    Save( CSave &save );
    int    Restore( CRestore &restore );
    static    TYPEDESCRIPTION m_SaveData[];
    #endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	//void SecondaryAttack( void );
	//int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	void SecondaryAttack( void );

	void UZIFire( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL Silenced );
    void UZIFireSilenced( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL Silenced );


	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireUZI;
	unsigned short m_usFireUZI_SIL;
};

class CFuckAr2 : public CBasePlayerItem
{
public:
    void Spawn( void );
    void Precache( void );
    int AddToPlayer( CBasePlayer *pPlayer );

	void EXPORT Think( void );
	void EXPORT Dissapear( void );
};
class CFuckMP7 : public CBasePlayerItem
{
public:
    void Spawn( void );
    void Precache( void );
    int AddToPlayer( CBasePlayer *pPlayer );
	void EXPORT Think( void );
	void EXPORT Dissapear( void );
};

class CBer92f : public CBasePlayerWeapon
{
public:
    //Hier wird das ganze für den Server gespeichert, da sonst nach einem Mapreload die Animationen nicht stimmen,
    //das Problem besteht aber im Client noch immer näheres weiter unten
    #ifndef CLIENT_DLL
    int    Save( CSave &save );
    int    Restore( CRestore &restore );
    static    TYPEDESCRIPTION m_SaveData[];
    #endif

    void Spawn( void );
    void Precache( void );
    int iItemSlot( void ) { return 2; }
    int GetItemInfo(ItemInfo *p);
    int AddToPlayer( CBasePlayer *pPlayer );

    void PrimaryAttack( void );
    void SecondaryAttack( void );
    void BerFire( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL Silenced );
    void BerFireSil( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL Silenced );
    BOOL Deploy( void );
    void Reload( void );
    void Holster( int skiplocal = 0 );
    void WeaponIdle( void );

	BOOL m_bSilAdd; //ok de todas maneras...

    virtual BOOL UseDecrement( void )
    {
#if defined( CLIENT_WEAPONS )
        return TRUE;
#else
        return FALSE;
#endif
    }
private:
    int m_iShell;
    unsigned short m_usBerFire;
    unsigned short m_usBerFireSil;
};

class CGlock18 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void Glock18Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

	void FireSemi( void );
	void FireBurst( void );

//	BOOL m_bGlock40shots; // no creo q esto valla aqui, aunque no tenga problemas
	int m_fGlockBurst; // This Will Set You That You Can Snipe

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	int m_iShell;

	unsigned short m_usFireGlock18;
	unsigned short m_usFireGlock18_B;
	unsigned short m_usSpriteTracer;
};


class CMP5 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );

	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usMP5;
};

class CHL2Pistol : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void GlockFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	int m_iShell;

    unsigned short m_usFirePistol;
};
// NEW heal  CLASS (ammo class stored in cpp file)
class CHeal : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	void EXPORT ChargeOk( void );
	void EXPORT BatteryOk( void );
	void EXPORT SuicideOk( void );
	void EXPORT SuicideNow( void );

	int m_fMode; // don't save this
	int m_fSick; // don't save this

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usHeal;
};

// NEW M4A2 CLASS (ammo class stored in cpp file)
class CM4A2 : public CBasePlayerWeapon
{
public:
	#ifndef CLIENT_DLL
    int    Save( CSave &save );
    int    Restore( CRestore &restore );
    static    TYPEDESCRIPTION m_SaveData[];
    #endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	int SecondaryAmmoIndex( void );

	void PrimaryAttack( void );
	void SecondaryAttack( void );

	void M4A2Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL Silenced );
    void M4A2FireGl( );
 //  void M4A2FireGlAnim( );

	BOOL NeedReloadGl;//you're fired the gun?

	//int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	//int m_fInZoom; // don't save this

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireM4A2;
	unsigned short m_usFireM4A2Gl;
};

class COICW : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	BOOL m_fInZoom;// don't save this. 
	void GlAttack( void );
	int m_iGlmode;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usOICW;
	unsigned short m_usOICWgl;
};

class CFlashSpotTEST : public CBaseEntity
{
void Spawn( void );
void Precache( void );

int ObjectCaps( void ) { return FCAP_DONT_SAVE; }

public:
void Suspend( float flSuspendTime );
void EXPORT Revive( void );

static CFlashSpotTEST *CreateFlashSpotTEST( void );
};
/*	void Precache( void );
	int iItemSlot( void ) { return 1; }
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int Swing( int fFirst );
	int Swing2( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	int m_iSwing2;
	void EXPORT SwingAgain2( void );
	void EXPORT Smack2( void );

	TraceResult m_trHit;
//
//	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
*/
// NEW flash  CLASS (doesn0t have any ammo)
class CFlash : public CBasePlayerWeapon
{
public:
	#ifndef CLIENT_DLL
    int    Save( CSave &save );
    int    Restore( CRestore &restore );
    static    TYPEDESCRIPTION m_SaveData[];
    #endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 8; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );

	CFlashSpotTEST *m_pSpot;
	int m_fSpotActive;

	void UpdateSpot ( void );

	void MakeLaserSight( void );
	void KillLaserSight( void );
	float m_flBeamLength;
	CBeam *m_pBeam;
};

class CIRGUN : public CBasePlayerWeapon
{
  public:
  void Spawn( void );
  void Precache( void );
  int iItemSlot( void ) { return 2; }
  int GetItemInfo(ItemInfo *p);

  void PrimaryAttack( void );
  void SecondaryAttack( void );
  void IRGUNFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
  BOOL Deploy( void );
  void Reload( void );
  void WeaponIdle( void );
  int AddToPlayer( CBasePlayer *pPlayer );

  int SecondaryAmmoIndex( void );
	
  int iMuzzleFlash;

  virtual BOOL UseDecrement( void )
  {
#if defined( CLIENT_WEAPONS )
  return TRUE;
#else
  return FALSE;
#endif
  }

private:

  int m_iShell;
  unsigned short m_usFireIRGUN;
};

class CFinalWeapon : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen);
	BOOL VisionCheck( void );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );

	void EXPORT PrimFire( void );
	void EXPORT SecFire( void );
};

class CHL2HandGrenade : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );

	void SecondaryAttack( void );
//	void SmokeThink( void );

	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	
	int m_fGrenMode; // don't save this

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CHL2Crowbar : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	int Swing( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	TraceResult m_trHit;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usCrowbar;
};


//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************
//*******************************


class CSaw : public CBasePlayerWeapon
{
public:
	void  Spawn( void );
	void  Precache( void );
	void  PrimaryAttack( void );
	void  SecondaryAttack( void );
	void  Fire (float nextattack);
	void  Reload( void );
	void  WeaponIdle( void );
	void  Holster( void );            
	BOOL  Deploy( void );
	int  m_iShell;
	int AddToPlayer( CBasePlayer *pPlayer );

	int  GetItemInfo(ItemInfo *p);
	int AddDuplicate( CBasePlayerItem *pOriginal ) { return FALSE; };//don't give second launcher!
	int  m_iModeAmmo;
private:	  
	unsigned short m_usSaw;
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

class CDeagle : public CBasePlayerWeapon
{
public:
	#ifndef CLIENT_DLL
    int    Save( CSave &save );
    int    Restore( CRestore &restore );
    static    TYPEDESCRIPTION m_SaveData[];
    #endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	//BOOL			FlashlightIsOn( void );
//	void			FlashlightTurnOn( void );
//	void			FlashlightTurnOff( void );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );
	int m_iShell;

	void Holster( int skiplocal = 0 );
	CLaserSpotTEST *m_pSpot;
	int m_fSpotActive;

	void UpdateSpot ( void );
//NEW
	//MP5 LASER SPOT - shadow
	void MakeLaserSight( void );
	void KillLaserSight( void );
	float m_flBeamLength;
	CBeam *m_pBeam;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFireDeagle;
};





#endif // WEAPONS_H
