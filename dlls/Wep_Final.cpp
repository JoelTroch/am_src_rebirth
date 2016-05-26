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

// This weapon is used on final stage when the player is in dream. Try to keep it secret, ok? :)

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

#include "shake.h" //This is required for the screen fade

#define FINAL_DEFAULT_GIVE		999
#define FINAL_MAX_CARRY			999
#define FINAL_WEIGHT			0
#define SF_ONSIGHT_STATECHECK	0x00004 //AJH

//HARDCODED: Make a decent tracehull

enum final_e
{
	FINAL_IDLE1 = 0,

	FINAL_FIRE1,
	FINAL_FIRE2,

	FINAL_RELOAD,

	FINAL_DEPLOY,
	FINAL_HOLSTER
};

LINK_ENTITY_TO_CLASS( weapon_hands, CFinalWeapon );

//=========================================================
//=========================================================
void CFinalWeapon::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_hands"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/finalweapon/w_hands.mdl");
	m_iId = WEAPON_FINAL;

	m_iClip = -1;

	FallInit();// get ready to fall down.
}


void CFinalWeapon::Precache( void )
{
	PRECACHE_MODEL("models/weapons/finalweapon/v_hands.mdl");
	PRECACHE_MODEL("models/weapons/finalweapon/w_hands.mdl");
	PRECACHE_MODEL("models/weapons/finalweapon/p_hands.mdl");

	PRECACHE_SOUND ("weapons/finalweapon/heal_fire-1.wav");// H to the K

	UTIL_PrecacheOther( "soul" );
}

int CFinalWeapon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 4;//mostrar
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_FINAL;
	p->iWeight = 1;
	p->weaponName = "Final Weapon";

	return 1;
}

int CFinalWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Hands"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();

		return TRUE;
	}
	return FALSE;
}

BOOL CFinalWeapon::Deploy( )
{
	m_pPlayer->m_InDREAM = TRUE;//automatic
	m_pPlayer->CineToggle(TRUE);
	SERVER_COMMAND("sv_gravity 400\n");

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.7;

	return DefaultDeploy("models/weapons/finalweapon/v_hands.mdl", "models/weapons/finalweapon/p_hands.mdl", FINAL_DEPLOY, "mp5a4" );
}

void CFinalWeapon::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_InDREAM = FALSE;//automatic
	m_pPlayer->CineToggle(FALSE);
	SERVER_COMMAND("sv_gravity 800\n");//Im a stupid... I should use a new trigger_gravity?

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5; //0.5
	if (m_iClip)
		SendWeaponAnim( FINAL_HOLSTER ); //KELLY:? HEAL_HOLSTER
	else
		SendWeaponAnim( FINAL_HOLSTER );
}

void CFinalWeapon::PrimaryAttack()
{	
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK))//FIX
    return;

	SendWeaponAnim( FINAL_FIRE1 );	
	SetThink( PrimFire );
	pev->nextthink = gpGlobals->time + 0.5;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}
void CFinalWeapon::SecondaryAttack()
{
	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK2))//FIX
    return;

	SendWeaponAnim( FINAL_FIRE2 );	
	SetThink( SecFire );
	pev->nextthink = gpGlobals->time + 1;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 2;
}

void CFinalWeapon::PrimFire()
{		
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecDir = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition( ); // + gpGlobals->v_up * -8 + gpGlobals->v_right * 8;
	Vector vecDest = vecSrc + vecDir * 9999;

	edict_t *pentIgnore;
	TraceResult tr;
	pentIgnore = m_pPlayer->edict();

	UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);
			
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		if ( pEntity && !pEntity->IsBSPModel() && pEntity->IsAlive() && pEntity->pev->movetype == MOVETYPE_STEP)//only monster ents
		{		
			switch (RANDOM_LONG(0,1)) 
			{
				case 0:	pEntity->Killed( pev, GIB_NEVER ); break;//GIB_NEVER
				case 1:	pEntity->Killed( pev, GIB_NEVER ); break;//GIB_ALWAYS
			}
		//	m_pPlayer->TakeHealth (pEntity->pev->max_health, DMG_GENERIC);
					
			CBaseEntity *pSoul = CBaseEntity::Create( "soul", pEntity->Center(), pEntity->pev->angles, edict() );
			pSoul->pev->dmg = pEntity->pev->max_health;
		}
		else
		{

		}
	}

	return;
} 

void CFinalWeapon::SecFire()
{	
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecDir = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition( ); // + gpGlobals->v_up * -8 + gpGlobals->v_right * 8;
	Vector vecDest = vecSrc + vecDir * 9999;

	edict_t *pentIgnore;
	TraceResult tr;
	pentIgnore = m_pPlayer->edict();

	UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);
			
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
					
		if ( pEntity && !pEntity->IsBSPModel() && pEntity->IsAlive() && pEntity->pev->movetype == MOVETYPE_STEP)//only monster ents
		{
			switch (RANDOM_LONG(0,4)) 
			{
				case 0:	pEntity->TakeDamage( pev, pev, 50, DMG_SPECIALBURN ); break;
				case 1:	pEntity->TakeDamage( pev, pev, 50, DMG_SPECIALBURN ); break;
				case 2:	pEntity->TakeDamage( pev, pev, 50, DMG_SPECIALBURN );		
						EMIT_SOUND_DYN ( ENT(m_pPlayer->pev), CHAN_BODY, "player/damage/goslow.wav", 1, ATTN_NORM, 0, 70 );
						break;	
				case 3:	pEntity->TakeDamage( pev, pev, 50, DMG_SPECIALBURN ); break;
				case 4:	pEntity->TakeDamage( pev, pev, 50, DMG_SPECIALBURN ); break;
/*			
				case 0:	pEntity->TakeDamage( pev, pev, 50, DMG_FREEZE ); break;
				case 1:	pEntity->TakeDamage( pev, pev, 50, DMG_SPECIALBURN ); break;
				case 2:	pEntity->TakeDamage( pev, pev, 50, DMG_VELOCITY );		
						EMIT_SOUND_DYN ( ENT(m_pPlayer->pev), CHAN_BODY, "player/damage/goslow.wav", 1, ATTN_NORM, 0, 70 );
						break;	
				case 3:	pEntity->TakeDamage( pev, pev, 50, DMG_EXPAND ); break;
				case 4:	pEntity->TakeDamage( pev, pev, 50, DMG_SHRINK ); break;*/
			}
						
			CBaseEntity *pSoul = CBaseEntity::Create( "soul", pEntity->Center(), pEntity->pev->angles, edict() );
			pSoul->pev->dmg = pEntity->pev->max_health;
		}
		else//alive bsp model whitout movetype speed
		{					
		//	pEntity->pev->velocity = pEntity->pev->velocity + gpGlobals->v_forward * -500;
			pEntity->Use( m_pPlayer, m_pPlayer, USE_TOGGLE, 0 );
		}
	}
	return;
}

void CFinalWeapon::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	SendWeaponAnim( FINAL_IDLE1 );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.

}

BOOL CFinalWeapon :: VisionCheck( void )
{											
	CBaseEntity *pLooker;
	if (pev->netname)
	{
		pLooker = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
		while (pLooker!=NULL)
		{
			if (!(pev->spawnflags & SF_ONSIGHT_STATECHECK) || (pev->spawnflags & SF_ONSIGHT_STATECHECK && pLooker->GetState()!=STATE_OFF)){

				CBaseEntity *pSeen;
				if (pev->message)
				{
					pSeen = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
					if (!pSeen)
					{
						// must be a classname.
						pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));

						while (pSeen != NULL)
						{
							if (CanSee(pLooker, pSeen))
								return TRUE;
							pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
						}
					}
					else
					{
						while (pSeen != NULL){
							if (CanSee(pLooker, pSeen))
								return TRUE;
							pSeen = UTIL_FindEntityByTargetname(pSeen, STRING(pev->message));
						}
					}
				}
				else{
					if (CanSee(pLooker, this))
						return TRUE;
				}
			}
			pLooker = UTIL_FindEntityByTargetname(pLooker, STRING(pev->netname));
		}
		return FALSE;
	}
	else
	{
		pLooker = UTIL_FindEntityByClassname(NULL, "player");
		if (!pLooker)
		{
			return FALSE;
		}
		CBaseEntity *pSeen;
		if (pev->message)
			pSeen = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
		else
			return CanSee(pLooker, this);

		if (!pSeen){
		// must be a classname.
			pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
			while (pSeen != NULL){
				if (CanSee(pLooker, pSeen))
					return TRUE;
				pSeen = UTIL_FindEntityByClassname(pSeen, STRING(pev->message));
			}
			return FALSE;
		}
		else{
			while (pSeen != NULL){
				if (CanSee(pLooker, pSeen))
					return TRUE;
				pSeen = UTIL_FindEntityByTargetname(pSeen, STRING(pev->message));
			}
			return FALSE;
		}
	}
}

BOOL CFinalWeapon :: CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen)
{
	if (pev->frags && (pLooker->pev->origin - pSeen->pev->origin).Length() > pev->frags)
		return FALSE;

	if (pev->max_health < 360)
	{
		Vector2D	vec2LOS;
		float	flDot;
		float flComp = pev->health;
		UTIL_MakeVectors ( pLooker->pev->angles );
		vec2LOS = ( pSeen->pev->origin - pLooker->pev->origin ).Make2D();
		vec2LOS = vec2LOS.Normalize();
		flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );
		if ( pev->max_health == -1 )
		{
			CBaseMonster *pMonst = pLooker->MyMonsterPointer();
			if (pMonst)
				flComp = pMonst->m_flFieldOfView;
			else
				return FALSE; // not a monster, can't use M-M-M-MonsterVision
		}
		if (flDot <= flComp)
			return FALSE;
	}
	return TRUE;
}
