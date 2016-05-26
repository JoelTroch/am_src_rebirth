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

#include "shake.h" //This is required for the screen fade

#define HEAL_DEFAULT_GIVE		1
#define HEAL_MAX_CARRY			5
#define HEAL_WEIGHT				1 //ddd!

//DELAYS
// Fire Anim: 5.35
// Reload Anim: 3.05

enum heal_e
{
	HEAL_IDLE1 = 0,
	HEAL_IDLE2,

	HEAL_FIRE1,
	HEAL_FIRE2,

	HEAL_RELOAD,

	HEAL_DEPLOY,
	HEAL_HOLSTER
};

LINK_ENTITY_TO_CLASS( weapon_heal, CHeal );
LINK_ENTITY_TO_CLASS( weapon_syringe, CHeal );

//=========================================================
//=========================================================
void CHeal::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_heal"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/heal/w_heal.mdl");
	m_iId = WEAPON_HEAL;

	m_iDefaultAmmo = HEAL_DEFAULT_GIVE; // 

	FallInit();// get ready to fall down.
}


void CHeal::Precache( void )
{
	PRECACHE_MODEL("models/weapons/heal/v_heal.mdl");
	PRECACHE_MODEL("models/weapons/heal/w_heal.mdl");
	PRECACHE_MODEL("models/weapons/heal/p_heal.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_556.mdl");// brass shellTE_MODEL

	PRECACHE_SOUND ("weapons/heal/heal_fire-1.wav");// H to the K

	PRECACHE_SOUND ("items/smallmedkit1.wav");
}

int CHeal::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "heal";
	p->iMaxAmmo1 = HEAL_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1; //M203_GRENADE_MAX_CARRY;
	p->iMaxClip = HEAL_DEFAULT_GIVE; //CLIP
	p->iSlot = 0;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_HEAL;
	p->iWeight = HEAL_WEIGHT;
	p->weaponName = "Heal Syringe";


	return 1;
}

int CHeal::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_Heal"); //digamos al cliente

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();

		return TRUE;
	}
	return FALSE;
}

BOOL CHeal::Deploy( )
{
	m_pPlayer->m_fCrosshairOff = TRUE;
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
	return DefaultDeploy("models/weapons/heal/v_heal.mdl", "models/weapons/heal/p_heal.mdl", HEAL_DEPLOY, "mp5a4" );
}

void CHeal::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_fCrosshairOff = FALSE;
	m_fInReload = FALSE;// cancel any reload in progress.
	// turn off announcements
	if (m_fSick == 1)
	{
		ALERT ( at_console, "m_fSick = 1\n" );
		//esta "enfermo" con toxinas en la sangre, dentro de 10 seg va a morir igual
	}
	else
	{//usando batteria o curandose, si hace holster, detener la curacion
		pev->nextthink = 0.0;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5; //0.5
	if (m_iClip)
		SendWeaponAnim( HEAL_HOLSTER ); //KELLY:? HEAL_HOLSTER
	else
		SendWeaponAnim( HEAL_HOLSTER );
}

void CHeal::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}
	if (m_iClip <= 0)
	{
		if (!m_fFireOnEmpty) //si no seguis apretando:
		Reload( );
		else // y si seguis con el dedo en el mouse...
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			m_flNextPrimaryAttack = 0.15;
		}
		return;
	}

    if ( m_pPlayer->pev->velocity.Length2D() <= 50 )//menor e igual
	{
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
/*
  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif*/
		if (m_fMode == 0) 
		{
			if (m_pPlayer->pev->health < 100)
			{
				SendWeaponAnim( HEAL_FIRE1 );	
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Heal_Fire"); //digamos al cliente
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/heal/heal_fire-1.wav", 1, ATTN_NORM);
				SetThink( ChargeOk );
				pev->nextthink = gpGlobals->time + 1.2;
			}
			else
			{
				SendWeaponAnim( HEAL_IDLE1 ); //SP: Por las dudas
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Heal_NoNeed"); //digamos al cliente
			}
		}
		else if (m_fMode == 1) 
		{
			if ( m_pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY)
			{
				SendWeaponAnim( HEAL_FIRE1 );	
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Heal_Fire"); //digamos al cliente
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/heal/heal_fire-1.wav", 1, ATTN_NORM);
				SetThink( BatteryOk );
				pev->nextthink = gpGlobals->time + 1.2;
			}
			else
			{
				SendWeaponAnim( HEAL_IDLE1 ); //SP: Por las dudas
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Heal_NoNeed"); //digamos al cliente
			}
		}
		else if (m_fMode == 2) 
		{
			SendWeaponAnim( HEAL_FIRE1 );	
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Heal_Fire"); //digamos al cliente
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/heal/heal_fire-1.wav", 1, ATTN_NORM);
			SetThink( SuicideOk );
			pev->nextthink = gpGlobals->time + 1.2;
		}
	}
	else
	{
		SendWeaponAnim( HEAL_IDLE1 ); //SP: Por las dudas
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Heal_CantFire"); //digamos al cliente
	}

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.20; // delay 2.30

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.20; // delay

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}
void CHeal::SecondaryAttack()
{
		if (m_fMode == 0) 
		{
			m_fMode = 1;
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#BatteryMode"); //digamos al cliente
			Deploy();
		}
		else if (m_fMode == 1) 
		{
			m_fMode = 2;
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#SuicideMode"); //digamos al cliente
			Deploy();
		}
		else if (m_fMode == 2) 
		{
			m_fMode = 0;
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#HealMode"); //digamos al cliente
			Deploy();
		}
	
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5; 
}
void CHeal::ChargeOk()
{				
	//el doble del medkit
	m_pPlayer->TakeHealth (gSkillData.healthkitCapacity * 2, DMG_GENERIC); //energia
	m_iClip--;// charge succesfull!

//	pev->nextthink = -1;
	return;
} 

void CHeal::BatteryOk()
{	
	m_pPlayer->pev->armorvalue += gSkillData.batteryCapacity *2;//edit: el doble

	if (m_pPlayer->pev->armorvalue >100)
	{
		m_pPlayer->pev->armorvalue = 100;//FIX, para no tener 142 de escudo
	}

	m_iClip--;// charge succesfull!

//	pev->nextthink = -1;
	return;
}
void CHeal::SuicideOk()
{	
	m_iClip--;// charge succesfull!
	//saquemos un poquito de vida (el pinchazo duele :)	
	//EDIT: de paso aparece el logo de poison, bárbaro xD
	m_pPlayer->TakeDamage(pev, pev, 2, DMG_POISON);

	//ahhh 10 segundos para morir!!
	SetThink( SuicideNow );
	pev->nextthink = gpGlobals->time + 5;//EDIT 5 segs...
	ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Heal_5seconds");
	//latidos y latidos... ahh
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "player/damage/breath_hearth.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	m_fSick = 1;
	pev->punchangle.y = RANDOM_LONG(0,89) - 45;

	return;
}

void CHeal::SuicideNow()
{	
	//hack
//	m_pPlayer->pev->health = 0;
	m_pPlayer->TakeDamage(pev, pev, 333, DMG_PARALYZE | DMG_NEVERGIB);//999 hace q vuele, nah

	pev->nextthink = -1;// no mas "thinks", me voy a matar...
	return;
}

void CHeal::Reload( void )
{
	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 1, HEAL_RELOAD, 3.05 );
	else
		iResult = DefaultReload( 1, HEAL_RELOAD, 3.05 );

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
}

void CHeal::WeaponIdle( void )
{
	//Nothing to see here...
}

class CHealAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/heal/w_healclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/heal/w_healclip.mdl");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 1, "heal", HEAL_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_healclip, CHealAmmoClip );