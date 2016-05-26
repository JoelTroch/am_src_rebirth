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

// This weapon it's going to be used in the final stage of Agency. I need to change the model first
// and add new effects( (like Soldier of fortune).

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

enum OICW_e
{
	OICW_IDLE1 = 0,

	OICW_RELOAD,
	OICW_DEPLOY,

	OICW_FIRE1,
	OICW_FIRE2,
	OICW_FIRE3
};

LINK_ENTITY_TO_CLASS( weapon_oicw, COICW );

//=========================================================
//=========================================================
void COICW::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_oicw"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/oicw/w_oicw.mdl");
	m_iId = WEAPON_OICW;

	m_iDefaultAmmo = OICW_DEFAULT_GIVE; // 

	FallInit();// get ready to fall down.
}


void COICW::Precache( void )
{
	PRECACHE_MODEL("models/weapons/oicw/v_oicw.mdl");
	PRECACHE_MODEL("models/weapons/oicw/w_oicw.mdl");
	PRECACHE_MODEL("models/weapons/oicw/p_oicw.mdl");

	m_iShell = PRECACHE_MODEL ("models/weapons/shell_556.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/weapons/oicw/v_oicw_scope.mdl");

	PRECACHE_SOUND ("weapons/oicw/oicw_fire-1.wav");// H to the K
	PRECACHE_SOUND ("weapons/oicw/oicw_fire-2.wav");// H to the K

//	PRECACHE_SOUND ("weapons/pistol_reload.wav");

	m_usOICW	= PRECACHE_EVENT( 1, "scripts/events/oicw.sc" );
	m_usOICWgl	= PRECACHE_EVENT( 1, "scripts/events/oicw_gl.sc" );
}

int COICW::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556";
	p->iMaxAmmo1 = _556_MAX_AMMO;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = OICW_DEFAULT_GIVE;
	p->iSlot = 6;
	p->iPosition = 8;
    p->iFlags    = ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_OICW;
	p->iWeight = P90_WEIGHT;
	p->weaponName = "oicw";


	return 1;
}

int COICW::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Pickup_oicw"); //digamos al cliente
/*
			char szText[201];
            hudtextparms_t     hText;

            sprintf(szText, "#Pickup_oicw");

            memset(&hText, 0, sizeof(hText));
            hText.channel = 4;//CANAL 4, NO REEMPLAZAR!!!!
			hText.x = 0.03;
			hText.y = 0.88; //85

            hText.effect = 0;    //0 Fade in/out 1//flickery 2//write out
            
            hText.r1 = hText.g1 = hText.b1 = 255;
            hText.a1 = 255;

            hText.r2 = hText.g2 = hText.b2 = 255;
            hText.a2 = 255;

            hText.fadeinTime = 0.2;
            hText.fadeoutTime = 1;
            hText.holdTime = 2.5;//1.5
            hText.fxTime = 0.5;

            UTIL_HudMessage(m_pPlayer, hText, szText);
*/
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();

		return TRUE;
	}
	return FALSE;
}

BOOL COICW::Deploy( )
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/weapon_deploy.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
	return DefaultDeploy("models/weapons/oicw/v_oicw.mdl", "models/weapons/oicw/p_oicw.mdl", OICW_DEPLOY, "mp5a4" );
}

void COICW::Holster( int skiplocal /* = 0 */ )
{
	if (m_fInZoom == TRUE) 
	{
		m_pPlayer->b_EstaEnZoom = FALSE;

		#ifndef CLIENT_DLL
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/OICW/v_OICW.mdl");
		#else
				LoadVModel ( "models/weapons/OICW/v_OICW.mdl", m_pPlayer );
		#endif
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
		m_fInZoom = FALSE;
	}

	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5; //0.5
}

void COICW::PrimaryAttack()
{	
	if (m_iGlmode == 1)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0)//si tiene gls
		{
			GlAttack();
		}
		else// si no tiene, sacar el modoGL
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#OICW_NoMoreGrenades");
			m_iGlmode = 0;
		}
		
		return;
	}
	else
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
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);

			m_flNextPrimaryAttack = 0.2;
			return;
		}

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		m_iClip--;

		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
		Vector vecDir;

		if (m_fInZoom == TRUE)//si tiene zoom
		{
			vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_PERFECT, 8192, BULLET_PLAYER_M16, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
		}
		else// FALSE
		{
	// ### COD RECOIL START ###
			if ( m_pPlayer->pev->velocity.Length2D() <= 220 )
			{
				if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
				{	
					if ( m_pPlayer->pev->flags & FL_DUCKING ) 
					{	//  si esta en el suelo y agachado
						vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_M16, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
					}
					else // si no es porq esta parado
					{																//perfe 3d
						vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_M16, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
					}
				}
				else // si no esta en el suelo es porque esta en el aire
				{
					if ( m_pPlayer->pev->flags & FL_DUCKING ) 
					{// y si esta agachado en el aire gana un punto
						vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_M16, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
					}						// 15 puntos igual
					else
					{ // y si no esta agachado 15 puntos (still on air)
						vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_M16, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
					}
				}
			}
			else
			{
				if ( m_pPlayer->pev->flags & FL_ONGROUND ) 
				{																//perfe 3d
					vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_8DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
				}
				else // si no esta en el suelo es porque esta en el aire
				{
					vecDir = m_pPlayer->FireBulletsThroughWalls( 1, vecSrc, vecAiming, VECTOR_CONE_20DEGREES, 2048, BULLET_PLAYER_AK74, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
				}
			}
	// ### COD RECOIL END ###
		}//end Zoom

	  int flags;
	#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
	#else
		flags = 0;
	#endif

		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usOICW, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1; // 0.1; delay

		if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1; // 0.1; delay

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}//CIERRE GLMODE
}
void COICW::SecondaryAttack()
{
	if (m_fInZoom == FALSE) //false
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45;
		m_fInZoom = TRUE;//**ON**
		m_pPlayer->b_EstaEnZoom = TRUE;

		#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/OICW/v_OICW_scope.mdl");
		#else
			LoadVModel ( "models/weapons/OICW/v_OICW_scope.mdl", m_pPlayer ); 
		#endif								

		UTIL_ScreenFade( m_pPlayer, Vector(0,0,0), 1, 0.1, 255, FFADE_IN); //fade to red 
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));   
	}
	else
	{
		m_pPlayer->b_EstaEnZoom = FALSE;

		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		m_fInZoom = FALSE;//**OFF**

		#ifndef CLIENT_DLL
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/OICW/v_OICW.mdl");//model default
		#else
				LoadVModel ( "models/weapons/OICW/v_OICW.mdl", m_pPlayer );
		#endif

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));   
	}

//	pev->nextthink = UTIL_WeaponTimeBase() + 0.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5; 
}

void COICW::GlAttack()
{
	ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Boom!"); //digamos al cliente

//	if (!(m_pPlayer->m_afButtonPressed & IN_ATTACK2)) //only one shoot by mousclick
//   return;

  //  m_fDefaultAnim = 1;//Nur zu sicherheit

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;
			
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

 	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	// we don't add in player velocity anymore.
	CGrenade::ShootContact( m_pPlayer->pev, 
							m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16, 
							gpGlobals->v_forward * 800 );

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usOICWgl );
	
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.
 
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	return; // return y no ejecutar más codigo.
}

int COICW::SecondaryAmmoIndex( void )
{
	return m_iSecondaryAmmoType;
}

void COICW::Reload( void )
{
	if (m_fInZoom == TRUE) 
	{
		#ifndef CLIENT_DLL
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/weapons/OICW/v_OICW.mdl");
		#else
				LoadVModel ( "models/weapons/OICW/v_OICW.mdl", m_pPlayer );
		#endif
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
		m_fInZoom = FALSE;
	}

	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload( 30, OICW_RELOAD, 2.9 );
	else
		iResult = DefaultReload( 30, OICW_RELOAD, 2.9 );

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_pPlayer->SetAnimation( PLAYER_RELOAD ); 
			
		Vector vecSrc	 = m_pPlayer->GetGunPosition( );
		CBaseEntity *pEntity = Create( "item_clip_rifle", vecSrc, pev->angles );
	}
	m_pPlayer->SetAnimation( PLAYER_RELOAD );
}

void COICW::WeaponIdle( void )
{
	if (m_pPlayer->m_fFireMode == 0)
	{
		m_iGlmode = 0;
		ALERT (at_console, "556 ammo \n");
	}
	else if (m_pPlayer->m_fFireMode == 1)
	{
		m_iGlmode = 1;
		ALERT (at_console, "GL ammo \n");
	}
	else if (m_pPlayer->m_fFireMode == 2)
	{
		m_iGlmode = 0;
	//	m_pPlayer->m_fNvgOn = TRUE; //nvg on
	}

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = OICW_IDLE1;	
		break;
	
	default:
	case 1:
		iAnim = OICW_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

class COICWAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/weapons/oicw/w_oicwclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/weapons/oicw/w_oicwclip.mdl");
//		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 30, "556", _556_MAX_AMMO) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_oicwclip, COICWAmmoClip );