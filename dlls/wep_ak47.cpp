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

#include "game.h"
#define	CROWBAR_BODYHIT_VOLUME 64
#define	CROWBAR_WALLHIT_VOLUME 256

enum AK74_e
{
	AK_IDLE1 = 0,
	AK_IDLE2,
	AK_FIRE1,
	AK_FIRE2,
	AK_STAB,
	AK_RELOAD,
	AK_RELOAD_VACIO,
	AK_DEPLOY,
	AK_HOLSTER,

	AK_NMC_IDLE1,
	AK_NMC_IDLE2,
	AK_NMC_FIRE1,
	AK_NMC_FIRE2,
	AK_NMC_STAB,
	AK_NMC_RELOAD_A,
	AK_NMC_RELOAD_A_VACIO,
	AK_NMC_RELOAD_B,
	AK_NMC_RELOAD_B_VACIO,
	AK_NMC_DEPLOY,
	AK_NMC_HOLSTER
};
LINK_ENTITY_TO_CLASS( weapon_AK47, CAK74 );
LINK_ENTITY_TO_CLASS( weapon_ak47, CAK74 );
//****************************************
void CAK74::KeyValue( KeyValueData *pkvd )//this sets for custom fields of weapon_generic. G-Cont.
{
	if (FStrEq(pkvd->szKeyName, "m_iszModel"))
	{
		char string[64];

		sprintf(string, "models/weapons/ak-47/v_%s.mdl", pkvd->szValue);
		v_model = ALLOC_STRING(string);

		sprintf(string, "models/weapons/ak-47/p_%s.mdl", pkvd->szValue);
		p_model = ALLOC_STRING(string);

		sprintf(string, "models/weapons/ak-47/w_%s.mdl", pkvd->szValue);
		w_model = ALLOC_STRING(string);

		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

//****************************************
//=========================================================
//=========================================================

void CAK74::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_AK47"); // hack to allow for old names
	Precache( );
//	SET_MODEL(ENT(pev), "models/weapons/ak-47/w_AK47.mdl");
	
	if (!FStringNull (v_model) )
	SET_MODEL( ENT(pev), STRING(w_model) );
	else
	SET_MODEL(ENT(pev), "models/weapons/ak-47/w_AK47.mdl");

	m_iId = WEAPON_AK74;

	m_iDefaultAmmo = AK74_DEFAULT_AMMO;

	FallInit();// get ready to fall down.
}


void CAK74::Precache( void )
{
	//movido aca adentro
	if (!FStringNull (v_model) )
	{
		PRECACHE_MODEL( (char *)STRING(v_model) );//HACKHACK. this required "UTIL_PrecacheOtherWeapon" for 
		PRECACHE_MODEL( (char *)STRING(p_model) );//right adding into HUD.ammo registry. don't worry about 
		PRECACHE_MODEL( (char *)STRING(w_model) );//this ;) G-Cont.
	}
	else
	{
		PRECACHE_MODEL("models/weapons/ak-47/v_AK47.mdl");
		PRECACHE_MODEL("models/weapons/ak-47/w_AK47.mdl");
		PRECACHE_MODEL("models/weapons/ak-47/p_AK47.mdl");
	}
	m_iShell = PRECACHE_MODEL ("models/weapons/shell_762.mdl");// brass shellTE_MODEL

	PRECACHE_SOUND ("weapons/ak-47/AK47_fire-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/ak-47/AK47_fire-2.wav");// H to the K

	PRECACHE_SOUND ( "weapons/ak-47/AK47_stab_hit-1.wav" );
    PRECACHE_SOUND ( "weapons/ak-47/AK47_stab_hitwall-1.wav" );
	PRECACHE_SOUND ( "weapons/ak-47/AK47_stab_hit-2.wav" );
    PRECACHE_SOUND ( "weapons/ak-47/AK47_stab_hitwall-2.wav" );

	m_usAK74 = PRECACHE_EVENT( 1, "scripts/events/AK47.sc" );
	m_usAk_stab = PRECACHE_EVENT( 1, "scripts/events/AK47_STAB.sc" );
}

int CAK74::GetItemInfo(ItemInfo *p)
{
    p->pszName   = STRING(pev->classname);
    p->pszAmmo1  = "ammo_AK47";              // The type of ammo it uses
    p->iMaxAmmo1 = AK74_MAX_AMMO;            // Max ammo the player can carry
    p->pszAmmo2  = NULL;                    // No secondary ammo
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = AK74_DEFAULT_AMMO;        // The clip size
    p->iSlot     = 3;						 // The number in the HUD
    p->iPosition = 0;						  // The position in a HUD slot
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
    p->iId       = m_iId = WEAPON_AK74;      // The weapon id
    p->iWeight   = AK74_WEIGHT; 
	p->weaponName = "AK-47";        //***** NEW

	return 1;
}

int CAK74::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Ak47"); //digamos al cliente
		return TRUE;
	}
	return FALSE;
}

BOOL CAK74::Deploy( )
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1;

	if (!FStringNull (v_model) )
	{
		m_pPlayer->pev->viewmodel = v_model;
		m_pPlayer->pev->weaponmodel = p_model;
	
		SendWeaponAnim( AK_NMC_DEPLOY, 1, 0 );	
		return TRUE;
	}
	else
	{
		return DefaultDeploy( AK74_MODEL_1STPERSON, AK74_MODEL_3RDPERSON, AK_NMC_DEPLOY, "ak47" ); //"mp5"
	}		
}

void CAK74::Holster( int skiplocal /* = 0 */ )
{
	m_fInReload = FALSE;// cancel any reload in progress.

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.2;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5;

	if (m_iClip)
		SendWeaponAnim( AK_NMC_HOLSTER );
	else
		SendWeaponAnim( AK_NMC_HOLSTER );
}

void CAK74::PrimaryAttack()
{
	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

/*
	if (m_iRicardo == 1)
	{
//		CBaseEntity *pRocket = CBaseEntity::Create( "rpg_rocket", vecSrc, pev->angles, edict() );
#ifndef CLIENT_DLL
		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		
//		CRpgRocket *pRocket = CRpgRocket::CreateRpgRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this );
		CBaseEntity *pRocket = CBaseEntity::Create( "rpg_rocket", vecSrc, m_pPlayer->pev->v_angle, edict() );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );// RpgRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
#endif
//		if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))
//		return;
	}
*/
// ### COD RECOIL START ###
	if ( m_pPlayer->pev->velocity.Length2D() <= 120 )
	{
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{	
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{	//  si esta en el suelo y agachado
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
			else // si no es porq esta parado
			{																//perfe 3d
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}																	//VECTOR_CONE_5DEGREES
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			if ( m_pPlayer->pev->flags & FL_DUCKING ) 
			{// y si esta agachado en el aire gana un punto
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}						// 15 puntos igual
			else
			{ // y si no esta agachado 15 puntos (still on air)
				vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
			}
		}
	}
	else
	{
		if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
		{																//perfe 3d
			vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_9DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
		}
		else // si no esta en el suelo es porque esta en el aire
		{
			vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
		}
	}
// ### COD RECOIL END ###


  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usAK74, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + AK74_FIRE_DELAY; // 0.1; delay
	
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
	
	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + AK74_FIRE_DELAY; // 0.1; delay

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CAK74::Reload( void )
{
	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 30, AK_NMC_RELOAD_B_VACIO, 2.54 );
	else
		iResult = DefaultReload( 30, AK_NMC_RELOAD_B_VACIO, 2.50 );
/*
	{

		switch (RANDOM_LONG(0,3))
		{
			case 0: iResult = DefaultReload( 28, AK_NMC_RELOAD_A, 2.06 ); break;
			case 1: iResult = DefaultReload( 19, AK_NMC_RELOAD_A, 2.06 ); break;
			case 2: iResult = DefaultReload( 30, AK_NMC_RELOAD_A, 2.06 ); break;
			case 3: iResult = DefaultReload( 30, AK_NMC_RELOAD_B, 2.06 ); break;
		}

	}
*/	

	if (iResult)
	{
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
			Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_rifle", vecSrc, pev->angles );		

	}
	//m_pPlayer->SetAnimation( PLAYER_RELOAD );
}


void CAK74::WeaponIdle( void )
{
/*
	if (!FStringNull (v_model) )
	{
		ALERT (at_console, "v_model is %s \n",STRING( v_model));
		ALERT (at_console, "p_model is %s \n",STRING( p_model));
		ALERT (at_console, "w_model is %s \n",STRING( w_model));
	}
*/
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = AK_NMC_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = AK_NMC_IDLE2;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

class CAK74AmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/ak-47/w_AK47clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/ak-47/w_AK47clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AK74_DEFAULT_AMMO, "ammo_AK47", AK74_MAX_AMMO) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_AK47, CAK74AmmoClip );

//************* SECONDARY ATTACK ***************** FUNCs A

void FindHullIntersection_AK( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int			i, j, k;
	float		distance;
	float		*minmaxs[2] = {mins, maxs};
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );
	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}


void CAK74::SecondaryAttack()
{
	if (! Swing( 1 ))
	{
		SetThink( SwingAgain );
		pev->nextthink = gpGlobals->time + 0.5; //0.1
	}
/*
	m_pPlayer->m_afButtonPressed |= IN_USE;
	m_pPlayer->PlayerUse();
	m_pPlayer->m_afButtonPressed &= ~IN_USE;
*/
}

void CAK74::Smack( )
{
	DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}


void CAK74::SwingAgain( void )
{
	Swing( 0 );
}


int CAK74::Swing( int fFirst )
{
	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors (m_pPlayer->pev->v_angle);
	Vector vecSrc	= m_pPlayer->GetGunPosition( );
	Vector vecEnd	= vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

#ifndef CLIENT_DLL
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection_AK( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usAk_stab, 
	0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0,
	0.0, 0, 0.0 );


	if ( tr.flFraction >= 1.0 )
	{
		if (fFirst)
		{
			// miss
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5;//0.5
			
			// player "shoot" animation
			//m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		switch( ((m_iSwing++) % 2) + 1 )// + 1
		{
		case 0:
			SendWeaponAnim( AK_NMC_STAB ); break;
		case 1:
			SendWeaponAnim( AK_NMC_STAB ); break;
		case 2:
			SendWeaponAnim( AK_NMC_STAB ); break;
		}

		// player "shoot" animation
	//	m_pPlayer->SetAnimation( PLAYER_ATTACK1 ); // Naaaa
		
#ifndef CLIENT_DLL

		// hit
		fDidHit = TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage( );

		if ( (m_flNextSecondaryAttack + 1 < UTIL_WeaponTimeBase() ) || g_pGameRules->IsMultiplayer() )
		{
			// first swing does full damage
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnifeStab, gpGlobals->v_forward, &tr, DMG_CLUB | DMG_NEVERGIB ); //DMG_CLUB );
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnifeStab /*/ 2*/, gpGlobals->v_forward, &tr, DMG_CLUB | DMG_NEVERGIB ); //DMG_CLUB );
		}	
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;

		if (pEntity)
		{
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// play thwack or smack sound
				switch( RANDOM_LONG(0,2) )
				{
				case 0:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/ak-47/AK47_stab_hit-1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/ak-47/AK47_stab_hit-2.wav", 1, ATTN_NORM); break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/ak-47/AK47_stab_hit-1.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if ( !pEntity->IsAlive() )
					  return TRUE;
				else
					  flVol = 0.1;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd-vecSrc)*2, BULLET_PLAYER_CROWBAR);

			if ( g_pGameRules->IsMultiplayer() )
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch( RANDOM_LONG(0,1) )
			{
			case 0:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/ak-47/AK47_stab_hitwall-1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/ak-47/AK47_stab_hitwall-2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
#endif
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7; //UTIL_WeaponTimeBase() + 0.25;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.9;

		SetThink( Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.8; //0.2;

															//0.7
	}
	return fDidHit;
}
