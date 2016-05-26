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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

#include "decals.h"

#define	CROWBAR_BODYHIT_VOLUME 64
#define	CROWBAR_WALLHIT_VOLUME 256

LINK_ENTITY_TO_CLASS( weapon_faknife, CFaknife );

enum faknife_e 
{
	FAKNIFE_IDLE = 0,
	FAKNIFE_SLASH1,
	FAKNIFE_MIDSLASH1,
	FAKNIFE_SLASH2,
	FAKNIFE_MIDSLASH2,

	FAKNIFE_STAB,
	FAKNIFE_STAB_HIT,

	FAKNIFE_DEPLOY,
	FAKNIFE_HOLSTER
};


void CFaknife::Spawn( )
{
	Precache( );
	m_iId = WEAPON_FAKNIFE; //WEAPON_NONE;
	SET_MODEL(ENT(pev), "models/weapons/FaKnife/w_FAknife.mdl");
	m_iClip = -1;

	FallInit();// get ready to fall down.
}


void CFaknife::Precache( void )
{
	PRECACHE_MODEL("models/weapons/FaKnife/v_FAknife.mdl");
	PRECACHE_MODEL("models/weapons/FaKnife/w_FAknife.mdl");
	PRECACHE_MODEL("models/weapons/FaKnife/p_FAknife.mdl");

	PRECACHE_SOUND("weapons/FaKnife/knife_stab_hit-1.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_stab_hit-2.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_stab_hitwall-1.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_stab_hitwall-2.wav");

	PRECACHE_SOUND("weapons/FaKnife/knife_hit1.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_hit2.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_hitbod1.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_hitbod2.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_hitbod3.wav");
	PRECACHE_SOUND("weapons/FaKnife/knife_miss1.wav");

	PRECACHE_SOUND("weapons/FaKnife/knife_deploy.wav");

	m_usFaknife = PRECACHE_EVENT ( 1, "scripts/events/faknife.sc" );
	m_usFaknife_stab = PRECACHE_EVENT ( 1, "scripts/events/faknife_stab.sc" );
}

int CFaknife::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_FAKNIFE; //WEAPON_NONE;
	p->iWeight = CROWBAR_WEIGHT;
	p->weaponName = "FA Knife";

	return 1;
}

int CFaknife::AddToPlayer( CBasePlayer *pPlayer )
{
    if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
    {
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_FAKnife"); //digamos al cliente

          MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
          WRITE_BYTE( m_iId );
          MESSAGE_END();
          return TRUE;
    }
    return FALSE;
}


BOOL CFaknife::Deploy( )
{
	m_pPlayer->m_fCrosshairOff = TRUE;
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/faknife/knife_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

//	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3; //10 15
	return DefaultDeploy( "models/weapons/FaKnife/v_FAknife.mdl", "models/weapons/FaKnife/p_FAknife.mdl", FAKNIFE_DEPLOY, "knife" ); //"crowbar"
}

void CFaknife::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_fCrosshairOff = FALSE;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5; ///UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( FAKNIFE_HOLSTER );
}


void FindHullIntersectionFA( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
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


void CFaknife::PrimaryAttack()
{
	if (! Swing( 1 ))
	{
		SetThink( SwingAgain );
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CFaknife::Smack( )
{
	//UTIL_DecalTrace( &m_trHit, DECAL_KNIFE1 );
//	DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}


void CFaknife::SwingAgain( void )
{
	Swing( 0 );
}

int CFaknife::Swing( int fFirst )
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
				FindHullIntersectionFA( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usFaknife, 
	0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0,
	0.0, 0, 0.0 );


	if ( tr.flFraction >= 1.0 )
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
			
			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		switch( ((m_iSwing++) % 2) + 1 )
		{
		case 0:
			SendWeaponAnim( FAKNIFE_SLASH2 ); break;
		case 1:
			SendWeaponAnim( FAKNIFE_SLASH1 ); break;
		case 2:
			SendWeaponAnim( FAKNIFE_SLASH2 ); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
#ifndef CLIENT_DLL

		// hit
		fDidHit = TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage( );

		if ( (m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase() ) || g_pGameRules->IsMultiplayer() )
		{
			// first swing does full damage
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnife, gpGlobals->v_forward, &tr, DMG_CLUB | DMG_NEVERGIB ); //DMG_CLUB );
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnife /*/ 2*/, gpGlobals->v_forward, &tr, DMG_CLUB | DMG_NEVERGIB ); //DMG_CLUB );
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
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_hitbod1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_hitbod2.wav", 1, ATTN_NORM); break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_hitbod3.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
			/*
				if ( !pEntity->IsAlive() )
					  return TRUE;
				else
				*/					
				if ( !pEntity->IsAlive() )
				{
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0; //LRC: corrected half-life bug
					m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
					return TRUE;
				}

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
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
#endif
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8; //UTIL_WeaponTimeBase() + 0.25;
		
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
		SetThink( Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 1.2; //0.2;

		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;

	}
	return fDidHit;
}

//************* SEC  ****************
void FindHullIntersection_FA2( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
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


void CFaknife::SecondaryAttack()
{
	if (! Swing2( 1 ))
	{
		SetThink( SwingAgain2 );
		pev->nextthink = gpGlobals->time + 0.1; //0.1
	}
	//ALERT ( at_console, "CK::SecondaryAttack\n" );
}

void CFaknife::Smack2( )
{
	//DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
	//ALERT ( at_console, "CAK74::Smack\n" );
}


void CFaknife::SwingAgain2( void )
{
	Swing2( 0 );
	//ALERT ( at_console, "CK::SwingAgain(\n" );
}


int CFaknife::Swing2( int fFirst )
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
				FindHullIntersection_FA2( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usFaknife_stab, 
	0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0,
	0.0, 0, 0.0 );


	if ( tr.flFraction >= 1.0 )
	{
		if (fFirst)
		{
			// miss
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3;//0.5
			
			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		switch( ((m_iSwing++) % 2) + 1 )// + 1
		{
		case 0:
			SendWeaponAnim( FAKNIFE_STAB_HIT ); break;
		case 1:
			SendWeaponAnim( FAKNIFE_STAB_HIT ); break;
		case 2:
			SendWeaponAnim( FAKNIFE_STAB_HIT ); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
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
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_stab_hit-1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_stab_hit-2.wav", 1, ATTN_NORM); break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_stab_hit-1.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				/*
				if ( !pEntity->IsAlive() )
					  return TRUE;
					  */
				if ( !pEntity->IsAlive() )
				{
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3; //LRC: corrected half-life bug
					m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3;
					return TRUE;
				}
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
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_stab_hitwall-1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/FaKnife/knife_stab_hitwall-2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
#endif
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3; //UTIL_WeaponTimeBase() + 0.25;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
		SetThink( Smack2 );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2; //0.2;


		//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
												//0.7
	}
	return fDidHit;
	//ALERT ( at_console, "CAK74::Swing\n" );
}
