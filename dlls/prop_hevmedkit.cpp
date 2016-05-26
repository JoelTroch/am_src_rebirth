
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
#include "saverestore.h"
#include "skill.h"
#include "gamerules.h"

class CHevMedkit : public CBaseToggle
{
public:
	void Spawn( );
	void Precache( void );
	void EXPORT Off(void);
	void EXPORT Recharge(void);
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return (CBaseToggle :: ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
		virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	virtual STATE GetState( void );

	static	TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge; 
	int		m_iReactivate ; // DeathMatch Delay until reactvated
	int		m_iJuice;
	int		m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;

	BOOL m_bSeAcaboPosta;
//	BOOL m_bActivadorTieneFullVida;//movido a cbase.h

	Vector	m_angGun;
};

TYPEDESCRIPTION CHevMedkit::m_SaveData[] =
{
	DEFINE_FIELD( CHevMedkit, m_flNextCharge, FIELD_TIME ),
	DEFINE_FIELD( CHevMedkit, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD( CHevMedkit, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD( CHevMedkit, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD( CHevMedkit, m_flSoundTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CHevMedkit, CBaseEntity );

LINK_ENTITY_TO_CLASS(prop_hevmedkit, CHevMedkit);

void CHevMedkit::KeyValue( KeyValueData *pkvd )
{
	if (	FStrEq(pkvd->szKeyName, "style") ||
				FStrEq(pkvd->szKeyName, "height") ||
				FStrEq(pkvd->szKeyName, "value1") ||
				FStrEq(pkvd->szKeyName, "value2") ||
				FStrEq(pkvd->szKeyName, "value3"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CHevMedkit::Spawn()
{
	Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/props/prop_hevmedkit.mdl");

//	pev->solid		= SOLID_BSP;
	pev->movetype	= MOVETYPE_PUSH;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0) );//pointsize until it lands on the ground.

	m_iJuice = gSkillData.suitchargerCapacity;
	pev->frame = 0;	
	
	InitBoneControllers();
}

void CHevMedkit::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/props/prop_hevmedkit.mdl");

	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/medcharge4.wav");
}


void CHevMedkit::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	// if it's not a player, ignore
	if (!FClassnameIs(pActivator->pev, "player"))
		return;
	
	if (pActivator->m_bActivadorTieneFullVida)
		return;

	if (m_bSeAcaboPosta)
		return;

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		Off();

		pev->sequence = 2;
		ResetSequenceInfo( );
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_iJuice <= 0) || (!(pActivator->pev->weapons & (1<<WEAPON_SUIT))))
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshotno1.wav", 1.0, ATTN_NORM );

			m_bSeAcaboPosta = TRUE;

			pev->skin = 1;//lero lero fox! lero lero! vé? así se hacen las cosas :P :P
			//secuencias movidas a off
		}
		return;
	}

	SetNextThink( 0.25 );
	SetThink(&CHevMedkit::Off);

	// Time to recharge yet?
	if (m_flNextCharge >= gpGlobals->time)
		return;

	// Make sure that we have a caller
	if (!pActivator)
		return;

	m_hActivator = pActivator;

	//only recharge the player
	if (!m_hActivator->IsPlayer() )
		return;
	
	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM );
		m_flSoundTime = 0.56 + gpGlobals->time;

		ResetSequenceInfo( );
		pev->sequence = 1;//deploy arm
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "items/medcharge4.wav", 1.0, ATTN_NORM );
			
		ResetSequenceInfo( );
		pev->sequence = 6;//giving charge
	}

	// charge the player
	if (m_hActivator->pev->health < 100)
	{
		m_iJuice--;
		m_hActivator->pev->health += 1;

		if (m_hActivator->pev->health > 100)
			m_hActivator->pev->health = 100;

		UTIL_MakeAimVectors( pev->angles );

		Vector posGun, angGun, m_posTarget;

		m_posTarget = m_hActivator->Center( );

		GetAttachment( 1, posGun, angGun );

		Vector vecTarget = (m_posTarget - posGun).Normalize( );

		Vector vecOut;

		vecOut.x = DotProduct( gpGlobals->v_forward, vecTarget );
		vecOut.y = -DotProduct( gpGlobals->v_right, vecTarget );
		vecOut.z = DotProduct( gpGlobals->v_up, vecTarget );

		Vector angles = UTIL_VecToAngles (vecOut);

		angles.x = -angles.x;
		if (angles.y > 180)
			angles.y = angles.y - 360;
		if (angles.y < -180)
			angles.y = angles.y + 360;
		if (angles.x > 180)
			angles.x = angles.x - 360;
		if (angles.x < -180)
			angles.x = angles.x + 360;

		if (angles.x > m_angGun.x)
			m_angGun.x = min( angles.x, m_angGun.x + 12 );
		if (angles.x < m_angGun.x)
			m_angGun.x = max( angles.x, m_angGun.x - 12 );
		if (angles.y > m_angGun.y)
			m_angGun.y = min( angles.y, m_angGun.y + 12 );
		if (angles.y < m_angGun.y)
			m_angGun.y = max( angles.y, m_angGun.y - 12 );

		m_angGun.y = SetBoneController( 0, m_angGun.y );
		m_angGun.x = SetBoneController( 3, m_angGun.y );//test

		SetBoneController(1, RANDOM_LONG (-180, 180));
		SetBoneController(2, RANDOM_LONG (-180, 180));
	}
	
	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;
}

void CHevMedkit::Recharge(void)
{
	ALERT(at_notice, "CHevMedkit::Recharge\n");

	m_iJuice = gSkillData.suitchargerCapacity;
	pev->frame = 0;			

	SetThink(&CHevMedkit:: SUB_DoNothing );
}

void CHevMedkit::Off(void)
{
	ALERT(at_notice, "CHevMedkit::Off\n");
		
	pev->sequence = 2;//retract arm
	ResetSequenceInfo( );

	// Stop looping sound.
	if (m_iOn > 1)
		STOP_SOUND( ENT(pev), CHAN_STATIC, "items/medcharge4.wav" );

	m_iOn = 0;

	if ((!m_iJuice) &&  ( ( m_iReactivate = g_pGameRules->FlHEVChargerRechargeTime() ) > 0) )
	{
		SetNextThink( m_iReactivate );
		SetThink(&CHevMedkit::Recharge);
	}
	else
		SetThink(&CHevMedkit:: SUB_DoNothing );
}

STATE CHevMedkit::GetState( void )
{
	if (m_iOn == 2)
		return STATE_IN_USE;

	else if (m_iJuice)
		return STATE_ON;
	else
		return STATE_OFF;
}
