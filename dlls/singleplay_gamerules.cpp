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

/**

  CHANGES ON THIS FILE:
  
+weapon menu
+game_pl equip will be touched when you spawn (so that you will have the weapons)

***/
//
// teamplay_gamerules.cpp
//
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"skill.h"
#include	"items.h"

extern DLL_GLOBAL CGameRules	*g_pGameRules;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgScoreInfo;
extern int gmsgMOTD;
// FGW
#include "bumplight.h"

//extern	void DumpStadistics (void);

//=========================================================
//=========================================================
CHalfLifeRules::CHalfLifeRules( void )
{
	RefreshSkillData();
}

//=========================================================
//=========================================================
void CHalfLifeRules::Think ( void )
{
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsMultiplayer( void )
{
	return FALSE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsDeathmatch ( void )
{
	return FALSE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsCoOp( void )
{
	return FALSE;
}


//=========================================================
//=========================================================
BOOL CHalfLifeRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	if ( !pPlayer->m_pActiveItem )
	{
		// player doesn't have an active item!
		return TRUE;
	}

	if ( !pPlayer->m_pActiveItem->CanHolster() )
	{
		return FALSE;
	}

	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules :: GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{
	return FALSE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules :: ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] )
{
	return TRUE;
}

void CHalfLifeRules :: InitHUD( CBasePlayer *pl )
{
}

//=========================================================
//=========================================================
void CHalfLifeRules :: ClientDisconnected( edict_t *pClient )
{
	//DumpStadistics();
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	// subtract off the speed at which a player is allowed to fall without being hurt,
	// so damage will be based on speed beyond that, not the entire fall
	pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

//=========================================================
//=========================================================
void CHalfLifeRules :: PlayerSpawn( CBasePlayer *pPlayer )
{
	CBaseEntity	*pWeaponEntity = NULL;

	//LRC- support the new "start with HEV" flag...
	
	if (g_startSuit)
	{
		pPlayer->pev->weapons |= (1<<WEAPON_SUIT);
	}/*
	if (g_startFlash)
	{
		pPlayer->pev->weapons |= (1<<WEAPON_FLASHLIGHT);
	}
	*/

// LRC what's wrong with allowing "game_player_equip" entities in single player? (The
// level designer is God: if he wants the player to start with a weapon, we should allow it!)

	//Okay Laurie... If you say... :)

	while ( pWeaponEntity = UTIL_FindEntityByClassname( pWeaponEntity, "game_player_equip" ))
	{
		pWeaponEntity->Touch( pPlayer );
	}

// fgw
	edict_t *pFind; 

	pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "light_bump" );

	while ( !FNullEnt( pFind ) )
	{
		CBaseEntity *pEnt = CBaseEntity::Instance( pFind );
		CBumpLight *pLight = (CBumpLight *)pEnt;

		if ( pLight )
		{
			pLight->CreateOnClient();
		}

		pFind = FIND_ENTITY_BY_CLASSNAME( pFind, "light_bump" );
	}
/*
extern int gmsgSetFog;
	
		//create global fog
		MESSAGE_BEGIN( MSG_ONE, gmsgSetFog, NULL, pPlayer->pev);
//		MESSAGE_BEGIN( MSG_ALL, gmsgSetFog);
			WRITE_BYTE ( 222 );
			WRITE_BYTE ( 222 );
			WRITE_BYTE ( 222 );
			WRITE_SHORT ( 0 );
			WRITE_SHORT ( 0 );
			WRITE_SHORT ( 2000 );
		MESSAGE_END();*/
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules :: AllowAutoTargetCrosshair( void )
{
	return ( g_iSkillLevel == SKILL_EASY );
}

//=========================================================
//=========================================================
void CHalfLifeRules :: PlayerThink( CBasePlayer *pPlayer )
{
}


//=========================================================
//=========================================================
BOOL CHalfLifeRules :: FPlayerCanRespawn( CBasePlayer *pPlayer )
{
	return TRUE;
}

//=========================================================
//=========================================================
float CHalfLifeRules :: FlPlayerSpawnTime( CBasePlayer *pPlayer )
{
	return gpGlobals->time;//now!
}

//=========================================================
// IPointsForKill - how many points awarded to anyone
// that kills this player?
//=========================================================
int CHalfLifeRules :: IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
	return 1;
}

//=========================================================
// PlayerKilled - someone/something killed this player
//=========================================================
void CHalfLifeRules :: PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
}

//=========================================================
// Deathnotice
//=========================================================
void CHalfLifeRules::DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
}

//=========================================================
// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world
//=========================================================
void CHalfLifeRules :: PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHalfLifeRules :: FlWeaponRespawnTime( CBasePlayerItem *pWeapon )
{
	return -1;
}

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHalfLifeRules :: FlWeaponTryRespawn( CBasePlayerItem *pWeapon )
{
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeRules :: VecWeaponRespawnSpot( CBasePlayerItem *pWeapon )
{
	return pWeapon->pev->origin;
}

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHalfLifeRules :: WeaponShouldRespawn( CBasePlayerItem *pWeapon )
{
	return GR_WEAPON_RESPAWN_NO;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::CanHaveItem( CBasePlayer *pPlayer, CItem *pItem )
{
	return TRUE;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem )
{
}

//=========================================================
//=========================================================
int CHalfLifeRules::ItemShouldRespawn( CItem *pItem )
{
	return GR_ITEM_RESPAWN_NO;
}


//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHalfLifeRules::FlItemRespawnTime( CItem *pItem )
{
	return -1;
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->pev->origin;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsAllowedToSpawn( CBaseEntity *pEntity )
{
	return TRUE;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount )
{
}

//=========================================================
//=========================================================
int CHalfLifeRules::AmmoShouldRespawn( CBasePlayerAmmo *pAmmo )
{
	return GR_AMMO_RESPAWN_NO;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo )
{
	return -1;
}

//=========================================================
//=========================================================
Vector CHalfLifeRules::VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo )
{
	return pAmmo->pev->origin;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlHealthChargerRechargeTime( void )
{
	return 0;// don't recharge
}

//=========================================================
//=========================================================
int CHalfLifeRules::DeadPlayerWeapons( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_GUN_NO;
}

//=========================================================
//=========================================================
int CHalfLifeRules::DeadPlayerAmmo( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_AMMO_NO;
}

//=========================================================
//=========================================================
int CHalfLifeRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	// why would a single player in half life need this? 
	return GR_NOTTEAMMATE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules :: FAllowMonsters( void )
{
	return TRUE;
}
BOOL CHalfLifeRules ::ClientCommand( CBasePlayer *pPlayer, const char *pcmd )
{
	if ( FStrEq( pcmd, "menuselect" ) )
    {
		extern int gmsgClcommand;

        if ( CMD_ARGC() < 2 )
            return TRUE;

        int slot = atoi( CMD_ARGV(1) );

		//if (pPlayer->pev->playerclass == 0) 

		//wepselect
		if (pPlayer->m_nmenudisp == 2)
		{
			if (slot == 1) 
			{
				pPlayer->GiveNamedItem( "weapon_knife" ); 
				pPlayer->GiveNamedItem( "weapon_glock18" );
				pPlayer->GiveAmmo( 200, "9mm", _9MM_MAX_CARRY );
				pPlayer->GiveNamedItem( "weapon_AK47" );
				pPlayer->GiveAmmo( 150, "ammo_AK47", AK74_MAX_AMMO );
			}

			if (slot == 2) 
			{
				pPlayer->GiveNamedItem( "weapon_heal" );
				pPlayer->GiveAmmo( 5, "heal", 5 ); //Medicine!
				pPlayer->GiveNamedItem( "weapon_knife" );
				pPlayer->GiveNamedItem( "weapon_ber92f" );
				pPlayer->GiveNamedItem( "item_ber92f_silencer" );//silencer
				pPlayer->GiveAmmo( 200, "9mm", _9MM_MAX_CARRY );
				pPlayer->GiveNamedItem( "weapon_sniper" );
				pPlayer->GiveAmmo( 10, "ammo_sniper", SNIPER_MAX_AMMO );// 4 full reloads
			}

			if (slot == 3) 
			{
				pPlayer->GiveNamedItem( "weapon_heal" );
				pPlayer->GiveAmmo( 5, "heal", 5 ); //Medicine!
				pPlayer->GiveNamedItem( "weapon_knife" );
				pPlayer->GiveNamedItem( "weapon_hegrenade" );
				pPlayer->GiveNamedItem( "weapon_9mmhandgun" );
				pPlayer->GiveAmmo( 130, "9mm", _9MM_MAX_CARRY );
				pPlayer->GiveNamedItem( "weapon_9mmAR" );
				pPlayer->GiveNamedItem( "item_mp5_silencer" );//silencer
				pPlayer->GiveAmmo( 200, "9mm", _9MM_MAX_CARRY );// 4 full reloads
			}

			if (slot == 4) 
			{
				pPlayer->GiveNamedItem( "weapon_heal" );
				pPlayer->GiveAmmo( 5, "heal", 5 ); //Medicine!
				pPlayer->GiveNamedItem( "weapon_knife" ); 
				pPlayer->GiveNamedItem( "weapon_ber92f" );
				pPlayer->GiveAmmo( 200, "9mm", _9MM_MAX_CARRY );
				pPlayer->GiveNamedItem( "weapon_uzi" );
				pPlayer->GiveNamedItem( "item_uzi_silencer" );//silencer
			}

			if (slot == 5) 
			{
				pPlayer->GiveNamedItem( "weapon_knife" ); 
				pPlayer->GiveNamedItem( "item_battery" );
				pPlayer->GiveNamedItem( "weapon_deagle" );
				pPlayer->GiveAmmo( 28, "ammo_deagle", DEAGLE_MAX_CARRY );
				pPlayer->GiveNamedItem( "weapon_m4a1" );
				pPlayer->GiveAmmo( 150, "556", _556_MAX_AMMO );
				pPlayer->GiveNamedItem( "item_m4a1_silencer" );//silencer
			}

			if (slot == 6) 
			{
				pPlayer->GiveNamedItem( "weapon_knife" ); //que mas queres?
				pPlayer->GiveNamedItem( "weapon_9mmhandgun" );
				pPlayer->GiveAmmo( 130, "9mm", _9MM_MAX_CARRY );
				pPlayer->GiveNamedItem( "weapon_M249" );
				pPlayer->GiveAmmo( 200, "ammo_M249", M249_MAX_AMMO );// 4 full reloads
			}
		}//eo wepselect
			
		UTIL_MakeVectors( Vector( 0, pPlayer->pev->v_angle.y, 0 ) );

		CBaseEntity *pEntity = NULL;

		if (pPlayer->m_nmenudisp == CREATE_ENTITY_TYPE)//select entity
		{
			if (slot == 1) 
				SERVER_COMMAND("create_entity_ai\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI)//AI enabled?
		{
			if (slot == 1)//yes
			{
				SERVER_COMMAND("create_entity_ai_class\n");
				pPlayer->m_bSpawnWhitAI = TRUE;
						
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("AIEnabled")));
			}
			if (slot == 2)//no
			{
				SERVER_COMMAND("create_entity_ai_class\n");
				pPlayer->m_bSpawnWhitAI = FALSE;
							
				UTIL_ShowMessageAll( STRING(ALLOC_STRING("AIDisabled")));
			}
			if (slot == 6)//back
				SERVER_COMMAND("create_entity\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS)//class type
		{
			if (slot == 1) 
				SERVER_COMMAND("create_entity_ai_class_ally\n");
			if (slot == 2) 
				SERVER_COMMAND("create_entity_ai_class_enemy\n");
			if (slot == 6)//back
				SERVER_COMMAND("create_entity_ai\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ALLY)
		{
			if(!pPlayer->m_bSpawnWhitAI)
			{
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "cine_monster_scientist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "cine_monster_hevsci", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "cine_monster_barney", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "cine_monster_otis", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "monster_hgrunt_ally", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}
			else
			{
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "monster_scientist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "monster_hevsci", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "monster_barney", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "monster_otis", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "monster_hgrunt_ally", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}

			if (slot == 6)//back
				SERVER_COMMAND("create_entity_ai_class\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY)
		{
			if (slot == 1) 
				SERVER_COMMAND("create_entity_ai_class_enemy_xen\n");
			if (slot == 2) 
				SERVER_COMMAND("create_entity_ai_class_enemy_xrace\n");
			if (slot == 3) 
				SERVER_COMMAND("create_entity_ai_class_enemy_millitary\n");
			if (slot == 4) 
				SERVER_COMMAND("create_entity_ai_class_enemy_hl2monsters\n");
			if (slot == 5) 
				SERVER_COMMAND("create_entity_ai_class_enemy_terrorists\n");
			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY_XEN)
		{
			if(!pPlayer->m_bSpawnWhitAI)
			{
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "cine_monster_alien_slave", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "cine_monster_alien_grunt", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "cine_monster_zombie", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "cine_monster_gargantua", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "cine_monster_houndeye", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}
			else
			{
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "monster_alien_slave", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "monster_alien_grunt", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "monster_zombie", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "monster_gargantua", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "monster_houndeye", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}

			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class_enemy\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY_XRACE)
		{
			if (slot == 1) 
			{
			}
			if (slot == 2) 
			{
			}
			if (slot == 3) 
			{
			}
			if (slot == 4) 
			{
			}
			if (slot == 5) 
			{
			}

			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class_enemy\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY_MILLITARY)
		{
			if(!pPlayer->m_bSpawnWhitAI)
			{
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "cine_monster_human_grunt", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "cine_monster_hgrunt_opfor", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "cine_monster_human_grunt_rpg", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "cine_monster_turret", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "cine_monster_turret", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}
			else
			{
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "monster_human_grunt", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "monster_hgrunt_opfor", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "monster_human_grunt_rpg", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "monster_turret", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "monster_turret", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}
			
			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class_enemy\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY_HL2MONSTERS)
		{
			if (slot == 1) 
				SERVER_COMMAND("create_entity_ai_class_enemy_hl2monsters_combine\n");
			if (slot == 2) 
				SERVER_COMMAND("create_entity_ai_class_enemy_hl2monsters_zombie\n");

			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class_enemy\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY_HL2MONSTERS_COMBINE)
		{
			if(!pPlayer->m_bSpawnWhitAI)
			{/*
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "cine_npc_combine", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "cine_npc_combine_metrocop", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "cine_npc_combine_supersoldier", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "cine_npc_combine", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "cine_npc_combine", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);*/
			}
			else
			{/*
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "npc_combine", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "npc_combine_metrocop", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "npc_combine_supersoldier", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "npc_combine", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "npc_combine", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
		*/	}
			
			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class_enemy_hl2monsters\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY_HL2MONSTERS_ZOMBIES)
		{
			if(!pPlayer->m_bSpawnWhitAI)
			{/*
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "cine_npc_zombie", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "cine_npc_zombie_fast", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "cine_npc_zombie_poison", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "cine_npc_headcrab_fast", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "cine_npc_headcrab_poison", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
		*/	}
			else
			{/*
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "npc_zombie", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "npc_zombie_fast", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "npc_zombie_poison", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "npc_headcrab_fast", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "npc_headcrab_poison", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
		*/	}

			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class_enemy_hl2monsters\n");
		}
		if (pPlayer->m_nmenudisp == CREATE_MONSTER_AI_CLASS_ENEMY_TERRORISTS)
		{
			if(!pPlayer->m_bSpawnWhitAI)
			{
/*
				pPlayer->g_iNextEntToSpawn = NPC_HUMAN_GRUNT;
				pPlayer->g_iNextEntToSpawn = NPC_HUMAN_GRUNT_RPG;
				pPlayer->g_iNextEntToSpawn = NPC_HUMAN_GRUNT_OPFOR;

				pPlayer->g_iNextEntToSpawn = NPC_ZOMBIE_HL;
				pPlayer->g_iNextEntToSpawn = NPC_HEADCRAB_HL;
				pPlayer->g_iNextEntToSpawn = NPC_GARGANTUA;

				pPlayer->g_iNextEntToSpawn = NPC_COMBINE;
				pPlayer->g_iNextEntToSpawn = NPC_COMBINE_METROCOP;
				pPlayer->g_iNextEntToSpawn = NPC_COMBINE_SUPERSOLDIER;

				pPlayer->g_iNextEntToSpawn = NPC_ZOMBIE;
				pPlayer->g_iNextEntToSpawn = NPC_ZOMBIE_FAST;
				pPlayer->g_iNextEntToSpawn = NPC_ZOMBIE_POISON;

				pPlayer->g_iNextEntToSpawn = NPC_AGENCY_MEMBER;
				pPlayer->g_iNextEntToSpawn = NPC_BARNEY;
				pPlayer->g_iNextEntToSpawn = NPC_OTIS;
				pPlayer->g_iNextEntToSpawn = NPC_HEVSCI;
				pPlayer->g_iNextEntToSpawn = NPC_SCIENTIST;
				pPlayer->g_iNextEntToSpawn = NPC_HOUNDEYE;
				pPlayer->g_iNextEntToSpawn = NPC_ALIEN_GRUNT;
				pPlayer->g_iNextEntToSpawn = NPC_ALIEN_SLAVE;	
*/	
//				pPlayer->g_iNextEntToSpawn = NPC_TERRORIST;

				if (slot == 1) 
					pEntity = CBaseEntity::Create( "cine_monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "cine_monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "cine_monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "cine_monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "cine_monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}
			else
			{
				if (slot == 1) 
					pEntity = CBaseEntity::Create( "monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 2) 
					pEntity = CBaseEntity::Create( "monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 3) 
					pEntity = CBaseEntity::Create( "monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 4) 
					pEntity = CBaseEntity::Create( "monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
				if (slot == 5) 
					pEntity = CBaseEntity::Create( "monster_terrorist", pPlayer->pev->origin + gpGlobals->v_forward * 128, pPlayer->pev->angles);
			}

			if (slot == 6) 
				SERVER_COMMAND("create_entity_ai_class_enemy\n");
		}

		pPlayer->g_iNextEntToSpawn = 0;//clear ent

        return TRUE;
    }

	return FALSE;
}