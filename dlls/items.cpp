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
  
+S2P Physics
+XP Coins
+Silencers, parachute and NVG Items

***/
/*

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"

extern int gmsgItemPickup;
// advanced NVG
extern int gmsgNVG; // we need to send this message

extern cvar_t phys_save_angles;
extern cvar_t phys_normalize_angles;
extern cvar_t phys_globaltrace;

class CWorldItem : public CBaseEntity
{
public:
	void	KeyValue(KeyValueData *pkvd ); 
	void	Spawn( void );
	int		m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

void CWorldItem::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CWorldItem::Spawn( void )
{
	CBaseEntity *pEntity = NULL;

	switch (m_iType) 
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create( "item_battery", pev->origin, pev->angles );
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create( "item_antidote", pev->origin, pev->angles );
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create( "item_security", pev->origin, pev->angles );
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create( "item_suit", pev->origin, pev->angles );
		break;
	}

	if (!pEntity)
	{
		ALERT( at_console, "unable to create world_item %d\n", m_iType );
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY(edict());
}


void CItem::Spawn( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING( pev->classname ), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove( this );
		return;
	}
}

extern int gEvilImpulse101;
/*
void CItem :: NormalizeAngles( float *angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

void CItem::BounceCode()
{

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
	pev->velocity = pev->velocity * CVAR_GET_FLOAT("phys_friction" );
	pev->gravity = pev->gravity * CVAR_GET_FLOAT("phys_gravity" );

	//FOOZ FUNC PHY CODE
	//FOOZ FUNC PHY CODE
	//FOOZ FUNC PHY CODE
		
	Vector savedangles = Vector( 0, 0, 0 );
	int negate = 0;
	TraceResult tr;
	// look down directly to know the surface we're lying.
			
	if ( phys_globaltrace.value != 0  ) //no es cero esta _Activado_
	UTIL_TraceLine( pev->origin, pev->origin - Vector(64,64,64), ignore_monsters, edict(), &tr );
	else
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );

		for( int i = 0; i<3; i++ )
		{
		if( pev->angles.x < 0 )
			negate = 1;

			if ( phys_save_angles.value != 0  ) //no es cero esta _Activado_
			{
			if( fabs(pev->angles.x) < 45 )
				savedangles.x = 0;
			else if( fabs(pev->angles.x) >= 45 && fabs(pev->angles.x) <= 135 )
				savedangles.x = 90;
			else if( fabs(pev->angles.x) > 135 && fabs(pev->angles.x) <= 180 )
				savedangles.x = 180;
			}

		}

		#ifndef M_PI
		#define M_PI 3.14159265358979
		#endif
		#define ang2rad (2 * M_PI / 360)

          if ( tr.flFraction < 1.0 )
          {
			  
			 if ( phys_normalize_angles.value != 0  ) //no es cero esta _Activado_
			{
			Vector forward, right, angdir, angdiry;
			Vector Angles = pev->angles;
			//Fooz
			NormalizeAngles( Angles );
		
			UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
			angdir = forward;
			Vector left = -right;
			angdiry = left;
//-?
			pev->angles.x = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
			pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			}
		}

		#undef ang2rad
		  
		  if( negate )
			pev->angles.x -= savedangles.x;
		  else
			pev->angles.x += savedangles.x;
		  			
		pev->avelocity = Vector ( 0, RANDOM_FLOAT( 50, 100 ), 0 );
	}
}
*/
void CItem::ItemTouch( CBaseEntity *pOther )
{/*
	//sys add
	if (!(pev->flags & FL_ONGROUND))
	{
		if ( pOther->IsBSPModel() )
		{
			pev->angles.x = RANDOM_FLOAT( 50, -50 );
			pev->angles.y = RANDOM_FLOAT( 50, -50 );
			pev->angles.z = RANDOM_FLOAT( 50, -50 );
		}

		// play bounce sound
		int pitch = 95 + RANDOM_LONG(0,29);
			
		if (RANDOM_LONG(0,1))
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch);
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop2.wav", 1, ATTN_NORM, 0, pitch);
	}

	BounceCode();
*/
//	ALERT (at_console, "Some item is touching something!\n");
	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// ok, a player is touching this item, but can he have it?
	if ( !g_pGameRules->CanHaveItem( pPlayer, this ) )
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch( pPlayer ))
	{
		SUB_UseTargets( pOther, USE_TOGGLE, 0 );
		SetTouch( NULL );
		
		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
		if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		{
			Respawn(); 
		}
		else
		{
			UTIL_Remove( this );
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove( this );
	}
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin( pev, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.

	SetThink ( Materialize );
	pev->nextthink = g_pGameRules->FlItemRespawnTime( this ); 
	return this;
}

void CItem::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150 );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch( ItemTouch );
}

#define SF_SUIT_SHORTLOGON		0x0001

class CItemSuit : public CItem
{
	void Spawn( void )
	{ 
		Precache( );

		if (pev->model) //SP: hagamos suit custom models!
			SET_MODEL( ENT(pev), STRING(pev->model) );
		else
			SET_MODEL( ENT(pev),"models/w_suit.mdl");

		//SET_MODEL(ENT(pev), "models/w_suit.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		if (pev->model)
			PRECACHE_MODEL( (char*)STRING(pev->model) );
		else
			PRECACHE_MODEL("models/w_suit.mdl");
		//PRECACHE_MODEL ("models/w_suit.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) )
			return FALSE;

		if ( pev->spawnflags & SF_SUIT_SHORTLOGON )
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon

/*		if(pPlayer->b_UsingHL2Weapon)
		{
			if ( pev->spawnflags & SF_SUIT_SHORTLOGON )
				EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
			else
				EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon
		}
		else
		{
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_KEVLAR0");		// short version of suit logon,
		}
*/
		pPlayer->pev->weapons |= (1<<WEAPON_SUIT);
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);



class CItemBattery : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_battery.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_battery.mdl");
		PRECACHE_SOUND( "items/gunpickup2.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->pev->deadflag != DEAD_NO )
		{
			return FALSE;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1<<WEAPON_SUIT)))
		{
			int pct;
			char szcharge[64];

			pPlayer->pev->armorvalue += gSkillData.batteryCapacity;
			pPlayer->pev->armorvalue = min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();

			
			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)( (float)(pPlayer->pev->armorvalue * 100.0) * (1.0/MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;
		
			sprintf( szcharge,"!HEV_%1dP", pct );
			//sys test
			EMIT_SOUND_SUIT(ENT(pev), szcharge);

			pPlayer->SetSuitUpdate("!HEV_BATTERY", FALSE, SUIT_NEXT_IN_1MIN);

			//pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return TRUE;		
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);


class CItemAntidote : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_antidote.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_antidote.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		pPlayer->SetSuitUpdate("!HEV_DET4", FALSE, SUIT_NEXT_IN_1MIN);
		
		pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);


class CItemSecurity : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_security.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_security.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		pPlayer->m_rgItems[ITEM_SECURITY] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);

class CItemLongJump : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_longjump.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_longjump.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->m_fLongJump )
		{
			return FALSE;
		}

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )
		{
			pPlayer->m_fLongJump = TRUE;// player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue( pPlayer->edict(), "slj", "1" );

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();

			EMIT_SOUND_SUIT( pPlayer->edict(), "!HEV_A1" );	// Play the longjump sound UNDONE: Kelly? correct sound?
			return TRUE;		
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_longjump, CItemLongJump );
/*
class CItemParachute : public CItem //the parachute item class
{
void Spawn( void ) //the spawn function
{
Precache( ); //calls the precache function
SET_MODEL(ENT(pev), "models/w_parachute.mdl"); //sets the model lying in the world to the longjump model. I think it looks similar to a parachute. You can also use the TFC backpack model for example
CItem::Spawn( ); //I think this declares the spawn function of the parachute as a public item spawn function
}
	void Precache( void ) //The precache function we called in spawn
	{
		PRECACHE_MODEL ("models/w_parachute.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
	}
	BOOL MyTouch( CBasePlayer *pPlayer ) //MyTouch (if we pick the parachute up...)
	{
		if ( pPlayer->m_fParachute ) //if we have the parachute already...
		{
			return FALSE; //dont let us pick it up again
		}

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
		{
			pPlayer->m_fParachute = TRUE; //...tell halflife we have the item
		
			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Parachute"); //digamos al cliente

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev ); //this is a message with which we tell the client that we have picked up an item
				WRITE_STRING( STRING(pev->classname) ); //it tells the game that it''s a parachute we just picked up
			MESSAGE_END(); //here ends the message we started above

			return TRUE; //return true to avoid errors
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_parachute, CItemParachute ); //makes an entity from the class we just wrote

class CItemMp5Sil : public CItem //the parachute item class
{
	void Spawn( void ) //the spawn function
	{
		Precache( ); //calls the precache function
		SET_MODEL(ENT(pev), "models/weapons/w_Silencer.mdl"); //sets the model lying in the world to the longjump model. I think it looks similar to a parachute. You can also use the TFC backpack model for example
		CItem::Spawn( ); //I think this declares the spawn function of the parachute as a public item spawn function
	}
	void Precache( void ) //The precache function we called in spawn
	{
		PRECACHE_MODEL ("models/weapons/w_Silencer.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
	}
	BOOL MyTouch( CBasePlayer *pPlayer ) //MyTouch (if we pick the silencer up...)
	{
		if ( pPlayer->m_bMp5SilAdd ) //if we have the silencer...
		{
			return FALSE; //dont let us pick it up again
		}

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
		{
			pPlayer->m_bMp5SilAdd = TRUE; //yeah! silencer!

			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Mp5_PickupKit"); //digamos al cliente

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev ); //this is a message with which we tell the client that we have picked up an item
				WRITE_STRING( STRING(pev->classname) ); //it tells the game that it''s a parachute we just picked up
			MESSAGE_END(); //here ends the message we started above

			return TRUE; //return true to avoid errors
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_mp5_silencer, CItemMp5Sil ); //makes an entity from the class we just wrote

class CItemBerSil : public CItem //the parachute item class
{
	void Spawn( void ) //the spawn function
	{
		Precache( ); //calls the precache function
		SET_MODEL(ENT(pev), "models/weapons/w_Silencer.mdl"); //sets the model lying in the world to the longjump model. I think it looks similar to a parachute. You can also use the TFC backpack model for example
		CItem::Spawn( ); //I think this declares the spawn function of the parachute as a public item spawn function
	}
	void Precache( void ) //The precache function we called in spawn
	{
		PRECACHE_MODEL ("models/weapons/w_Silencer.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
	}
	BOOL MyTouch( CBasePlayer *pPlayer ) //MyTouch (if we pick the silencer up...)
	{
		if ( pPlayer->m_bSilAdd ) //if we have the silencer...
		{
			return FALSE; //dont let us pick it up again
		}

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
		{
			pPlayer->m_bSilAdd = TRUE; //yeah! silencer!

			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Pickup_BerSilencer"); //digamos al cliente

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev ); //this is a message with which we tell the client that we have picked up an item
				WRITE_STRING( STRING(pev->classname) ); //it tells the game that it''s a parachute we just picked up
			MESSAGE_END(); //here ends the message we started above

			return TRUE; //return true to avoid errors
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_ber92f_silencer, CItemBerSil );

class CItemM4a1Sil : public CItem //the parachute item class
{
	void Spawn( void ) //the spawn function
	{
		Precache( ); //calls the precache function
		SET_MODEL(ENT(pev), "models/weapons/w_Silencer.mdl"); //sets the model lying in the world to the longjump model. I think it looks similar to a parachute. You can also use the TFC backpack model for example
		CItem::Spawn( ); //I think this declares the spawn function of the parachute as a public item spawn function
	}
	void Precache( void ) //The precache function we called in spawn
	{
		PRECACHE_MODEL ("models/weapons/w_Silencer.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
	}
	BOOL MyTouch( CBasePlayer *pPlayer ) //MyTouch (if we pick the silencer up...)
	{
		if ( pPlayer->m_bM16SilAdd ) //if we have the silencer...
		{
			return FALSE; //dont let us pick it up again
		}

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
		{
			pPlayer->m_bM16SilAdd = TRUE; //yeah! silencer!
		
			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Pickup_m4a1Silencer"); //digamos al cliente

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev ); //this is a message with which we tell the client that we have picked up an item
				WRITE_STRING( STRING(pev->classname) ); //it tells the game that it''s a parachute we just picked up
			MESSAGE_END(); //here ends the message we started above

			return TRUE; //return true to avoid errors
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_m4a1_silencer, CItemM4a1Sil ); 

class CItemUziSil : public CItem //the parachute item class
{
	void Spawn( void ) //the spawn function
	{
		Precache( ); //calls the precache function
		SET_MODEL(ENT(pev), "models/weapons/w_Silencer.mdl"); //sets the model lying in the world to the longjump model. I think it looks similar to a parachute. You can also use the TFC backpack model for example
		CItem::Spawn( ); //I think this declares the spawn function of the parachute as a public item spawn function
	}
	void Precache( void ) //The precache function we called in spawn
	{
		PRECACHE_MODEL ("models/weapons/w_Silencer.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
	}
	BOOL MyTouch( CBasePlayer *pPlayer ) //MyTouch (if we pick the silencer up...)
	{
		if ( pPlayer->m_bUZISilAdd ) //if we have the silencer...
		{
			return FALSE; //dont let us pick it up again
		}

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
		{
			pPlayer->m_bUZISilAdd = TRUE; //yeah! silencer!
		
			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Pickup_UziSilencer"); //digamos al cliente

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev ); //this is a message with which we tell the client that we have picked up an item
				WRITE_STRING( STRING(pev->classname) ); //it tells the game that it''s a parachute we just picked up
			MESSAGE_END(); //here ends the message we started above

			return TRUE; //return true to avoid errors
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_uzi_silencer, CItemUziSil );

class CItemAllSil : public CItem //the parachute item class
{
	void Spawn( void ) //the spawn function
	{
		Precache( ); //calls the precache function
		SET_MODEL(ENT(pev), "models/weapons/w_all_extended.mdl"); //sets the model lying in the world to the longjump model. I think it looks similar to a parachute. You can also use the TFC backpack model for example
		CItem::Spawn( ); //I think this declares the spawn function of the parachute as a public item spawn function
	}
	void Precache( void ) //The precache function we called in spawn
	{
		PRECACHE_MODEL ("models/weapons/w_all_extended.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
	}
	BOOL MyTouch( CBasePlayer *pPlayer ) //MyTouch (if we pick the silencer up...)
	{
//		if ( pPlayer->m_bUZISilAdd ) //if we have the silencer...
//		{
//			return FALSE; //dont let us pick it up again
//		}

		if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
		{
			pPlayer->m_bUZISilAdd = TRUE; //yeah! silencer!
			pPlayer->m_bM16SilAdd = TRUE; //yeah! silencer!
			pPlayer->m_bSilAdd = TRUE; //yeah! silencer!
			pPlayer->m_bMp5SilAdd = TRUE; //yeah! silencer!

			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Pickup_MasterKit"); //digamos al cliente

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev ); //this is a message with which we tell the client that we have picked up an item
				WRITE_STRING( STRING(pev->classname) ); //it tells the game that it''s a parachute we just picked up
			MESSAGE_END(); //here ends the message we started above

			return TRUE; //return true to avoid errors
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_all_silencer, CItemAllSil );
*/	

class CCoin : public CItem
{	
public:
	void Spawn( void );
	void BounceCode( void );
	void NormalizeAngles( float *angles );
	void EXPORT ItemTouch( CBaseEntity *pOther );
	BOOL MyTouch( CBasePlayer *pPlayer );

	void Precache( void );
};

BOOL CCoin::MyTouch( CBasePlayer *pPlayer )
{
	return FALSE;
	//overloaded function
}

void CCoin::Spawn( void )
{ 
	Precache( );
		
	SET_MODEL( ENT(pev),"models/coin.mdl");
	
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	pev->gravity = 0.5;
	pev->friction = 0.5;
		
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-10, -10, 0), Vector(10, 10, 10));
		
	SetTouch( ItemTouch );
}
LINK_ENTITY_TO_CLASS(xp_point_coin, CCoin);

void CCoin::Precache( void )
{
	PRECACHE_MODEL ("models/coin.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
	
	PRECACHE_SOUND( "coins/pickup1.wav" );
	PRECACHE_SOUND( "coins/pickup2.wav" );	
	
	PRECACHE_SOUND( "coins/coinBounce1.wav" );
	PRECACHE_SOUND( "coins/coinBounce2.wav" );

}

void CCoin :: NormalizeAngles( float *angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

void CCoin::BounceCode()
{
	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
	pev->velocity = pev->velocity * CVAR_GET_FLOAT("phys_friction" );
	pev->gravity = pev->gravity * CVAR_GET_FLOAT("phys_gravity" );

	//FOOZ FUNC PHY CODE
	//FOOZ FUNC PHY CODE
	//FOOZ FUNC PHY CODE
		
	Vector savedangles = Vector( 0, 0, 0 );
	int negate = 0;
	TraceResult tr;
	// look down directly to know the surface we're lying.
			
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );

		for( int i = 0; i<3; i++ )
		{
		if( pev->angles.x < 0 )
			negate = 1;
		}

		#ifndef M_PI
		#define M_PI 3.14159265358979
		#endif
		#define ang2rad (2 * M_PI / 360)

          if ( tr.flFraction < 1.0 )
          {
			Vector forward, right, angdir, angdiry;
			Vector Angles = pev->angles;
			//Fooz
			NormalizeAngles( Angles );
		
			UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
			angdir = forward;
			Vector left = -right;
			angdiry = left;
//-?
			pev->angles.x = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
			pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
			pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
		}

		#undef ang2rad
		  
		  if( negate )
			pev->angles.x -= savedangles.x;
		  else
			pev->angles.x += savedangles.x;
		  			
		pev->avelocity = Vector ( 0, RANDOM_FLOAT( 50, 100 ), 0 );
	}
}

void CCoin::ItemTouch( CBaseEntity *pOther )
{
	if (!(pev->flags & FL_ONGROUND))
	{
		if ( pOther->IsBSPModel() )
		{
			pev->avelocity.x = RANDOM_FLOAT( -500, 500 );
			pev->avelocity.y = RANDOM_FLOAT( -500, 500 );
			pev->avelocity.z = RANDOM_FLOAT( -500, 500 );
		}
			
		if (RANDOM_LONG(0,1))
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/coinBounce1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/coinBounce2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
	}

	BounceCode();

	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// ok, a player is touching this item, but can he have it?
	if ( !g_pGameRules->CanHaveItem( pPlayer, this ) )
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch( pPlayer ))
	{
		SUB_UseTargets( pOther, USE_TOGGLE, 0 );
		SetTouch( NULL );
		
		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
				
		UTIL_Remove( this );
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove( this );
	}
}

///////////////////////////////////////////
class CGoldCoin : public CCoin
{
	void Spawn( void );
	BOOL MyTouch( CBasePlayer *pPlayer );
	void Precache( void );
};

BOOL CGoldCoin::MyTouch( CBasePlayer *pPlayer )
{
	if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
	{
		int iXPPoints = CVAR_GET_FLOAT( "xp_points" );

		CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 10) );

		if (RANDOM_LONG(0,1))
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/pickup1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/pickup2.wav", 1, ATTN_NORM, 0, PITCH_NORM);

		return TRUE; //return true to avoid errors
	}
	return FALSE;
}

void CGoldCoin::Spawn( void )
{ 
	Precache( );
		
	SET_MODEL( ENT(pev),"models/coins/goldXP.mdl");
	
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	pev->gravity = 0.8;
	pev->friction = 0.8;
		
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-5, -5, 0), Vector(5, 5, 5));
	
	SetTouch( ItemTouch );
}
LINK_ENTITY_TO_CLASS(xp_point_gold, CGoldCoin);

void CGoldCoin::Precache( void )
{
	PRECACHE_SOUND( "coins/pickup1.wav" );
	PRECACHE_SOUND( "coins/pickup2.wav" );	
	
	PRECACHE_SOUND( "coins/coinBounce1.wav" );
	PRECACHE_SOUND( "coins/coinBounce2.wav" );

	PRECACHE_MODEL ("models/coins/goldXP.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
}

///////////////////////////////////////////
class CPlateCoin : public CCoin
{
	void Spawn( void );
	BOOL MyTouch( CBasePlayer *pPlayer );
	void Precache( void );
};

BOOL CPlateCoin::MyTouch( CBasePlayer *pPlayer )
{
	if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
	{
		int iXPPoints = CVAR_GET_FLOAT( "xp_points" );

		CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 5) );

		if (RANDOM_LONG(0,1))
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/pickup1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/pickup2.wav", 1, ATTN_NORM, 0, PITCH_NORM);

		return TRUE; //return true to avoid errors
	}
	return FALSE;
}

void CPlateCoin::Spawn( void )
{ 
	Precache( );
		
	SET_MODEL( ENT(pev),"models/coins/PlateXP.mdl");
	
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	pev->gravity = 0.8;
	pev->friction = 0.8;
		
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-5, -5, 0), Vector(5, 5, 5));
		
	SetTouch( ItemTouch );
}
LINK_ENTITY_TO_CLASS(xp_point_plate, CPlateCoin);

void CPlateCoin::Precache( void )
{
	PRECACHE_MODEL ("models/coins/PlateXP.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
}

///////////////////////////////////////////
class CBrownCoin : public CCoin
{
	void Spawn( void );
	BOOL MyTouch( CBasePlayer *pPlayer );
	void Precache( void );
};

BOOL CBrownCoin::MyTouch( CBasePlayer *pPlayer )
{
	if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
	{
		int iXPPoints = CVAR_GET_FLOAT( "xp_points" );

		CVAR_SET_FLOAT( "xp_points", ( iXPPoints += 1) );

		if (RANDOM_LONG(0,1))
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/pickup1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "coins/pickup2.wav", 1, ATTN_NORM, 0, PITCH_NORM);

		return TRUE; //return true to avoid errors
	}
	return FALSE;
}

void CBrownCoin::Spawn( void )
{ 
	Precache( );
		
	SET_MODEL( ENT(pev),"models/coins/BrownXP.mdl");
	
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	pev->gravity = 0.8;
	pev->friction = 0.8;
		
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-5, -5, 0), Vector(5, 5, 5));
		
	SetTouch( ItemTouch );
}
LINK_ENTITY_TO_CLASS(xp_point_brown, CBrownCoin);

void CBrownCoin::Precache( void )
{
	PRECACHE_MODEL ("models/coins/BrownXP.mdl"); //this just precaches the model, otherwise you would get an error that it''s not precached. You can''t use unprecached models
}
