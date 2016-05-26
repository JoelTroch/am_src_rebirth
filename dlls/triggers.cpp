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
  
+trigger_command : "Console Command"
+trigger_changecvar : "Change Console Variable"
+trigger_changevalue : "Change any entity's values"
+trigger_displaymenu: "Wep Menu Selection"
+trigger_dialogue : "dialogue whit others"
+trigger_dialogcheck : "checks the points of the dialogue"
+trigger_onsight : "Trigger when A sees B"
+trigger_changebrief_info : "changes the brief and send it"
+trigger_startpatrol : "Trigger Start Patrol"
+trigger_random : "Trigger random" 
+trigger_xppoints : "add or remove xp points"
+trigger_check_cvar : "check a cvar and modifi it"
+env_sprite_grass (obsolete entity that creates a single grass sprite)

***/

/*

===== triggers.cpp ========================================================

  spawn and use functions for editor-placed triggers              

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"

#include "shake.h"//screenfade

#include "weapons.h"//sprites

#define	SF_TRIGGER_PUSH_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_TARGETONCE	1// Only fire hurt target once
#define	SF_TRIGGER_HURT_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define	SF_TRIGGER_HURT_NO_CLIENTS	8//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_CLIENTONLYFIRE	16// trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32// only clients may touch this trigger.

extern DLL_GLOBAL BOOL		g_fGameOver;

extern void SetMovedir(entvars_t* pev);
extern Vector VecBModelOrigin( entvars_t* pevBModel );

class CFrictionModifier : public CBaseEntity
{
public:
	void		Spawn( void );
	void		KeyValue( KeyValueData *pkvd );
	void EXPORT	ChangeFriction( CBaseEntity *pOther );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	static	TYPEDESCRIPTION m_SaveData[];

	float		m_frictionFraction;		// Sorry, couldn't resist this name :)
};

LINK_ENTITY_TO_CLASS( func_friction, CFrictionModifier );

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION	CFrictionModifier::m_SaveData[] = 
{
	DEFINE_FIELD( CFrictionModifier, m_frictionFraction, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE(CFrictionModifier,CBaseEntity);


// Modify an entity's friction
void CFrictionModifier :: Spawn( void )
{
	pev->solid = SOLID_TRIGGER;
	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	pev->movetype = MOVETYPE_NONE;
	SetTouch( ChangeFriction );
}


// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
void CFrictionModifier :: ChangeFriction( CBaseEntity *pOther )
{
	if ( g_pGameRules->IsMultiplayer() ) //no es cero esta _Activado_
	{
		// if this is a player, don't move him around!
	}
	else
	{
	if ( pOther->pev->movetype != MOVETYPE_BOUNCEMISSILE && pOther->pev->movetype != MOVETYPE_BOUNCE )
		pOther->pev->friction = m_frictionFraction;
	}
}



// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
void CFrictionModifier :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "modifier"))
	{
		m_frictionFraction = atof(pkvd->szValue) / 100.0;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

// This trigger will fire when the level spawns (or respawns if not fire once)
// It will check a global state before firing.  It supports delay and killtargets

#define SF_AUTO_FIREONCE		0x0001

class CAutoTrigger : public CBaseDelay
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Precache( void );
	void Think( void );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

private:
	int			m_globalstate;
	USE_TYPE	triggerType;
};
LINK_ENTITY_TO_CLASS( trigger_auto, CAutoTrigger );

TYPEDESCRIPTION	CAutoTrigger::m_SaveData[] = 
{
	DEFINE_FIELD( CAutoTrigger, m_globalstate, FIELD_STRING ),
	DEFINE_FIELD( CAutoTrigger, triggerType, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CAutoTrigger,CBaseDelay);

void CAutoTrigger::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi( pkvd->szValue );
		switch( type )
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}

void CAutoTrigger::Spawn( void )
{
	Precache();
}

void CAutoTrigger::Precache( void )
{
	pev->nextthink = gpGlobals->time + 0.1;
}

void CAutoTrigger::Think( void )
{
	if ( !m_globalstate || gGlobalState.EntityGetState( m_globalstate ) == GLOBAL_ON )
	{
		SUB_UseTargets( this, triggerType, 0 );
		if ( pev->spawnflags & SF_AUTO_FIREONCE )
			UTIL_Remove( this );
	}
}

//***************************
//***************************
//***************************
#define SF_RELAY_FIREONCE		0x0001
#define SF_DONT_SEND_CINEBARS  	0x0002

/*
SysOp: I give you my lastest entity: trigger_dialogue.

  Whit this new entity you can create easily question and choices. Of course you can chose the responses
  and the code will look for your response and will fire a entity.

  1-You can create simple dialogues, or create a big dialogues firing another trigger_dialogue after.
  2-You can select choises too. This a new a versatil entity.
  3-You can give score to each response, and check this score at end (in negotiation cases)

  Look for the examples. It's easy to use, but maybe some people found problems.

  TO DO: I don't know why I start to create the code using cvars. I need to create new values at cbase.h
		Make a decent keyboard check
*/

class CTriggerDialog : public CBaseDelay
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void EXPORT ButtonsThink( void );
	void EXPORT ResponsesList( void );

	void EXPORT HisResponse1( void );
	void EXPORT HisResponse2( void );
	void EXPORT HisResponse3( void );

	void EXPORT FireAfterResponse( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	//string_t fh_InitialQuestion;//usando pev noise

	string_t fh_HisResponseOne;
	string_t fh_HisResponseTwo;
	string_t fh_HisResponseThird;

	float fh_DelayTheirResponse;

	float fh_MyRespOnePoints;
	float fh_MyRespTwoPoints;
	float fh_MyRespThirdPoints;

	string_t fh_FireRespOne;
	string_t fh_FireRespTwo;
	string_t fh_FireRespThird;

	string_t fh_FireAfter;
	float fh_FireAfterDelay;

	BOOL ResponseSended;
};
LINK_ENTITY_TO_CLASS( trigger_dialogue, CTriggerDialog );

TYPEDESCRIPTION CTriggerDialog::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerDialog, fh_HisResponseOne, FIELD_STRING ),
	DEFINE_FIELD( CTriggerDialog, fh_HisResponseTwo, FIELD_STRING ),
	DEFINE_FIELD( CTriggerDialog, fh_HisResponseThird, FIELD_STRING ),

	DEFINE_FIELD( CTriggerDialog, fh_DelayTheirResponse, FIELD_FLOAT ),

	DEFINE_FIELD( CTriggerDialog, fh_MyRespOnePoints, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerDialog, fh_MyRespTwoPoints, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerDialog, fh_MyRespThirdPoints, FIELD_FLOAT ),

	DEFINE_FIELD( CTriggerDialog, fh_FireRespOne, FIELD_STRING ),
	DEFINE_FIELD( CTriggerDialog, fh_FireRespTwo, FIELD_STRING ),
	DEFINE_FIELD( CTriggerDialog, fh_FireRespThird, FIELD_STRING ),

	DEFINE_FIELD( CTriggerDialog, fh_FireAfter, FIELD_STRING ),
	DEFINE_FIELD( CTriggerDialog, fh_FireAfterDelay, FIELD_FLOAT ),

	DEFINE_FIELD( CTriggerDialog, ResponseSended, FIELD_BOOLEAN ),
};
IMPLEMENT_SAVERESTORE( CTriggerDialog, CBaseDelay );

void CTriggerDialog::KeyValue( KeyValueData *pkvd )
{	
	//DELAY ANTES DE SU RESPUESTA
	if (FStrEq(pkvd->szKeyName, "delay_their_response"))//
	{
		fh_DelayTheirResponse = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	//PUNTOS POR RESPUESTA
	else if (FStrEq(pkvd->szKeyName, "response_one_points"))//
	{
		fh_MyRespOnePoints = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "response_two_points"))//
	{
		fh_MyRespTwoPoints = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "response_third_points"))//
	{
		fh_MyRespThirdPoints = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	//ACTIVAR DESPUES DE MI X RESPUESTA
	else if (FStrEq(pkvd->szKeyName, "response_one_fire"))//
	{
		fh_FireRespOne = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "response_two_fire"))//
	{
		fh_FireRespTwo = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "response_third_fire"))//
	{
		fh_FireRespThird = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fire_after"))//
	{
		fh_FireAfter = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fire_after_delay"))//
	{
		fh_FireAfterDelay =  atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else
		CBaseDelay::KeyValue( pkvd );
}

void CTriggerDialog::Spawn( void )
{
	/*
	UTIL_SetSize( pev, Vector(-10, -10, -10), Vector(10, 10, 10));
	//bsp?
	pev->solid		= SOLID_BBOX;//solid not
	pev->movetype	= MOVETYPE_NOCLIP; //MOVETYPE_NOCLIP; 

	UTIL_SetOrigin( pev, pev->origin );
	*/
}

void CTriggerDialog::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pPlayer = CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );

//	if (!pActivator->IsPlayer()) // activator should be a player
//		return;

	//borrar cualquier numero disponible
	CVAR_SET_STRING( "selection", "0" );
	ResponseSended = FALSE;//resetear checkeo de envio de respuesta
	
	if ( !( pev->spawnflags & SF_DONT_SEND_CINEBARS ) )
	SERVER_COMMAND( "cinebars\n" );//started dialogue

	ALERT ( at_console, "CTriggerDialog::Use\n"); 

//	if ( pev->spawnflags & SF_RELAY_FIREONCE )
//		UTIL_Remove( this );
	
	if (pev->noise)//una pregunta
	{
		//UTIL_ShowMessageAll( STRING(pev->noise) );//enviar lista de respuestas ahora			
		UTIL_ShowMessage( STRING(pev->noise), pPlayer );
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 

		SetThink ( ResponsesList );//enviar la lista de respuestas en 5 segundos
		pev->nextthink = gpGlobals->time + 2.5;//EDIT: 5 no es demasiado?? Hagamos la mitad
	}
}

void CTriggerDialog::ResponsesList(void)
{		
	CBaseEntity *pPlayer = CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );

	ALERT ( at_console, "CTriggerDialog::ResponsesList\n"); 

	UTIL_ShowMessage( STRING(pev->message), pPlayer );
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 

	SetThink ( ButtonsThink );//pasar al think para comprobar cada boton
	pev->nextthink = gpGlobals->time + 2.5;//pasar a responder en 5 segundos
}


void CTriggerDialog::ButtonsThink( void )
{	
	ALERT ( at_console, "CTriggerDialog::ButtonsThink, Points: %f\n", CVAR_GET_FLOAT( "totaldialogpoints" )); 
	CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);

	//en Clientcommand se comprueba si se presionan y se vuelven verdaderos
	//este think comprueba si es verdadero algun valor, envia la funcion
	//e inmediatamnente resetea el valor a 0, para ser usado despues
		
	int iTotalDialogPoints = CVAR_GET_FLOAT( "totaldialogpoints" );

	if(ResponseSended == FALSE)//Si no fue enviado
	{
		UTIL_ShowMessageAll( STRING(ALLOC_STRING("CONV_HELP")));

		if ( CVAR_GET_FLOAT( "selection" ) == 1 )
		{		
			//pPlayer->iDialogueTotalPoints += fh_MyRespOnePoints;
			CVAR_SET_FLOAT( "totaldialogpoints", ( iTotalDialogPoints += fh_MyRespOnePoints ) );

			FireTargets( STRING(fh_FireRespOne), this, this, USE_TOGGLE, 0 );//use on?

			SetThink ( HisResponse1 );//pasar al think de la respuesta del otro
//			pev->nextthink = gpGlobals->time + fh_DelayTheirResponse;//pasar a responder en X segundos
			pev->nextthink = gpGlobals->time + 2.5;//pasar a responder en X segundos
				
			//UTIL_ShowMessageAll( STRING(ALLOC_STRING("")));//mmm esto no sobreescribe nada...
			ResponseSended = TRUE;
		}
			
		if ( CVAR_GET_FLOAT( "selection" ) == 2 )
		{
			//pPlayer->iDialogueTotalPoints += fh_MyRespTwoPoints;
			CVAR_SET_FLOAT( "totaldialogpoints", ( iTotalDialogPoints += fh_MyRespTwoPoints ) );

			FireTargets( STRING(fh_FireRespTwo), this, this, USE_TOGGLE, 0 );

			SetThink ( HisResponse2 );//pasar al think de la respuesta del otro
//			pev->nextthink = gpGlobals->time + fh_DelayTheirResponse;//pasar a responder en X segundos
			pev->nextthink = gpGlobals->time + 2.5;//pasar a responder en X segundos
						
			ResponseSended = TRUE;
		}

		if ( CVAR_GET_FLOAT( "selection" ) == 3 )
		{
			//pPlayer->iDialogueTotalPoints += fh_MyRespThirdPoints;
			CVAR_SET_FLOAT( "totaldialogpoints", ( iTotalDialogPoints += fh_MyRespThirdPoints ) );

			FireTargets( STRING(fh_FireRespThird), this, this, USE_TOGGLE, 0 );

			SetThink ( HisResponse3 );//pasar al think de la respuesta del otro
//			pev->nextthink = gpGlobals->time + fh_DelayTheirResponse;//pasar a responder en X segundos
			pev->nextthink = gpGlobals->time + 2.5;//pasar a responder en X segundos
			
			//UTIL_ShowMessageAll( STRING(ALLOC_STRING("")));
			ResponseSended = TRUE;
		}		
	}
	pev->nextthink = gpGlobals->time + 0.1;
}

void CTriggerDialog::HisResponse1(void)
{		
	ALERT ( at_console, "CTriggerDialog::HisResponse1\n"); 

	UTIL_ShowMessageAll( STRING(pev->noise1) );		
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 

	SetThink ( FireAfterResponse );
	pev->nextthink = gpGlobals->time + fh_FireAfterDelay;
}
void CTriggerDialog::HisResponse2(void)
{	
	ALERT ( at_console, "CTriggerDialog::HisResponse2\n"); 

	UTIL_ShowMessageAll( STRING(pev->noise2) );		
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 

	SetThink ( FireAfterResponse );
	pev->nextthink = gpGlobals->time + fh_FireAfterDelay;
}
void CTriggerDialog::HisResponse3(void)
{	
	ALERT ( at_console, "CTriggerDialog::HisResponse3\n"); 

	UTIL_ShowMessageAll( STRING(pev->noise3) );		
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 

	SetThink ( FireAfterResponse );
	pev->nextthink = gpGlobals->time + fh_FireAfterDelay;
}

void CTriggerDialog::FireAfterResponse(void)
{	
	FireTargets( STRING(fh_FireAfter), this, this, USE_TOGGLE, 0 );

	SetThink ( NULL );
	
	if ( !( pev->spawnflags & SF_DONT_SEND_CINEBARS ) )
	SERVER_COMMAND( "cinebars\n" );//finished dialogue

	if (pev->spawnflags & SF_RELAY_FIREONCE)
	UTIL_Remove( this );
}
//***************************

class CTriggerCkeckPoints : public CBaseDelay
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void EXPORT PointsCheck( void );

	int fh_PointsNeeded;
	int fh_ResetTotalPoints;

	string_t fh_FirePointsGood;
	string_t fh_FirePointsBad;
};
LINK_ENTITY_TO_CLASS( trigger_ckecktotalpoints, CTriggerCkeckPoints );
LINK_ENTITY_TO_CLASS( trigger_dialogcheck, CTriggerCkeckPoints );

TYPEDESCRIPTION CTriggerCkeckPoints::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerCkeckPoints, fh_PointsNeeded, FIELD_INTEGER ),

	DEFINE_FIELD( CTriggerCkeckPoints, fh_FirePointsGood, FIELD_STRING ),
	DEFINE_FIELD( CTriggerCkeckPoints, fh_FirePointsBad, FIELD_STRING ),

	DEFINE_FIELD( CTriggerCkeckPoints, fh_ResetTotalPoints, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( CTriggerCkeckPoints, CBaseDelay );

void CTriggerCkeckPoints::KeyValue( KeyValueData *pkvd )
{	
	if (FStrEq(pkvd->szKeyName, "points_needed"))//
	{
		fh_PointsNeeded = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fire_points_good"))//
	{
		fh_FirePointsGood = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fire_points_bad"))//
	{
		fh_FirePointsBad = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "reset_total_points"))//
	{
		fh_ResetTotalPoints = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}

void CTriggerCkeckPoints::Spawn( void )
{

}

void CTriggerCkeckPoints::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	ALERT ( at_console, "Total Points Acumulated: %f\n", CVAR_GET_FLOAT( "totaldialogpoints" )); 

	SetThink ( PointsCheck );//pasar al think del check...
	pev->nextthink = gpGlobals->time + 5;//... en 5 segundos
}

void CTriggerCkeckPoints::PointsCheck( void )
{		
	CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);

	if ( CVAR_GET_FLOAT( "totaldialogpoints" ) >= fh_PointsNeeded)
	{
		FireTargets( STRING(fh_FirePointsGood), this, this, USE_TOGGLE, 0 );
	}
	else
	{
		FireTargets( STRING(fh_FirePointsBad), this, this, USE_TOGGLE, 0 );
	}

	if(fh_ResetTotalPoints = 1)
	{
		CVAR_SET_FLOAT( "totaldialogpoints", 0 );
	}

	SetThink ( NULL );
}
//***************************
//***************************

#define SF_RELAY_FIREONCE		0x0001

class CTriggerRelay : public CBaseDelay
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

private:
	USE_TYPE	triggerType;
};
LINK_ENTITY_TO_CLASS( trigger_relay, CTriggerRelay );

TYPEDESCRIPTION	CTriggerRelay::m_SaveData[] = 
{
	DEFINE_FIELD( CTriggerRelay, triggerType, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CTriggerRelay,CBaseDelay);

void CTriggerRelay::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi( pkvd->szValue );
		switch( type )
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}


void CTriggerRelay::Spawn( void )
{
}




void CTriggerRelay::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SUB_UseTargets( this, triggerType, 0 );
	if ( pev->spawnflags & SF_RELAY_FIREONCE )
		UTIL_Remove( this );
}


//**********************************************************
// The Multimanager Entity - when fired, will fire up to 16 targets 
// at specified times.
// FLAG:		THREAD (create clones when triggered)
// FLAG:		CLONE (this is a clone for a threaded execution)

#define SF_MULTIMAN_CLONE		0x80000000
#define SF_MULTIMAN_THREAD		0x00000001

class CMultiManager : public CBaseToggle
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn ( void );
	void EXPORT ManagerThink ( void );
	void EXPORT ManagerUse   ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

#if _DEBUG
	void EXPORT ManagerReport( void );
#endif

	BOOL		HasTarget( string_t targetname );
	
	int ObjectCaps( void ) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	int		m_cTargets;	// the total number of targets in this manager's fire list.
	int		m_index;	// Current target
	float	m_startTime;// Time we started firing
	int		m_iTargetName	[ MAX_MULTI_TARGETS ];// list if indexes into global string array
	float	m_flTargetDelay [ MAX_MULTI_TARGETS ];// delay (in seconds) from time of manager fire to target fire
private:
	inline BOOL IsClone( void ) { return (pev->spawnflags & SF_MULTIMAN_CLONE) ? TRUE : FALSE; }
	inline BOOL ShouldClone( void ) 
	{ 
		if ( IsClone() )
			return FALSE;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) ? TRUE : FALSE; 
	}

	CMultiManager *Clone( void );
};
LINK_ENTITY_TO_CLASS( multi_manager, CMultiManager );

// Global Savedata for multi_manager
TYPEDESCRIPTION	CMultiManager::m_SaveData[] = 
{
	DEFINE_FIELD( CMultiManager, m_cTargets, FIELD_INTEGER ),
	DEFINE_FIELD( CMultiManager, m_index, FIELD_INTEGER ),
	DEFINE_FIELD( CMultiManager, m_startTime, FIELD_TIME ),
	DEFINE_ARRAY( CMultiManager, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS ),
	DEFINE_ARRAY( CMultiManager, m_flTargetDelay, FIELD_FLOAT, MAX_MULTI_TARGETS ),
};

IMPLEMENT_SAVERESTORE(CMultiManager,CBaseToggle);

void CMultiManager :: KeyValue( KeyValueData *pkvd )
{
	// UNDONE: Maybe this should do something like this:
	//CBaseToggle::KeyValue( pkvd );
	// if ( !pkvd->fHandled )
	// ... etc.

	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if ( m_cTargets < MAX_MULTI_TARGETS )
		{
			char tmp[128];

			UTIL_StripToken( pkvd->szKeyName, tmp );
			m_iTargetName [ m_cTargets ] = ALLOC_STRING( tmp );
			m_flTargetDelay [ m_cTargets ] = atof (pkvd->szValue);
			m_cTargets++;
			pkvd->fHandled = TRUE;
		}
	}
}


void CMultiManager :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	SetUse ( ManagerUse );
	SetThink ( ManagerThink);

	// Sort targets
	// Quick and dirty bubble sort
	int swapped = 1;

	while ( swapped )
	{
		swapped = 0;
		for ( int i = 1; i < m_cTargets; i++ )
		{
			if ( m_flTargetDelay[i] < m_flTargetDelay[i-1] )
			{
				// Swap out of order elements
				int name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				m_iTargetName[i] = m_iTargetName[i-1];
				m_flTargetDelay[i] = m_flTargetDelay[i-1];
				m_iTargetName[i-1] = name;
				m_flTargetDelay[i-1] = delay;
				swapped = 1;
			}
		}
	}
}


BOOL CMultiManager::HasTarget( string_t targetname )
{ 
	for ( int i = 0; i < m_cTargets; i++ )
		if ( FStrEq(STRING(targetname), STRING(m_iTargetName[i])) )
			return TRUE;
	
	return FALSE;
}


// Designers were using this to fire targets that may or may not exist -- 
// so I changed it to use the standard target fire code, made it a little simpler.
void CMultiManager :: ManagerThink ( void )
{
	float	time;

	time = gpGlobals->time - m_startTime;
	while ( m_index < m_cTargets && m_flTargetDelay[ m_index ] <= time )
	{
		FireTargets( STRING( m_iTargetName[ m_index ] ), m_hActivator, this, USE_TOGGLE, 0 );
		m_index++;
	}

	if ( m_index >= m_cTargets )// have we fired all targets?
	{
		SetThink( NULL );
		if ( IsClone() )
		{
			UTIL_Remove( this );
			return;
		}
		SetUse ( ManagerUse );// allow manager re-use 
	}
	else
		pev->nextthink = m_startTime + m_flTargetDelay[ m_index ];
}

CMultiManager *CMultiManager::Clone( void )
{
	CMultiManager *pMulti = GetClassPtr( (CMultiManager *)NULL );

	edict_t *pEdict = pMulti->pev->pContainingEntity;
	memcpy( pMulti->pev, pev, sizeof(*pev) );
	pMulti->pev->pContainingEntity = pEdict;

	pMulti->pev->spawnflags |= SF_MULTIMAN_CLONE;
	pMulti->m_cTargets = m_cTargets;
	memcpy( pMulti->m_iTargetName, m_iTargetName, sizeof( m_iTargetName ) );
	memcpy( pMulti->m_flTargetDelay, m_flTargetDelay, sizeof( m_flTargetDelay ) );

	return pMulti;
}


// The USE function builds the time table and starts the entity thinking.
void CMultiManager :: ManagerUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// In multiplayer games, clone the MM and execute in the clone (like a thread)
	// to allow multiple players to trigger the same multimanager
	if ( ShouldClone() )
	{
		CMultiManager *pClone = Clone();
		pClone->ManagerUse( pActivator, pCaller, useType, value );
		return;
	}

	m_hActivator = pActivator;
	m_index = 0;
	m_startTime = gpGlobals->time;

	SetUse( NULL );// disable use until all targets have fired

	SetThink ( ManagerThink );
	pev->nextthink = gpGlobals->time;
}

#if _DEBUG
void CMultiManager :: ManagerReport ( void )
{
	int	cIndex;

	for ( cIndex = 0 ; cIndex < m_cTargets ; cIndex++ )
	{
		ALERT ( at_console, "%s %f\n", STRING(m_iTargetName[cIndex]), m_flTargetDelay[cIndex] );
	}
}
#endif

//***********************************************************


//
// Render parameters trigger
//
// This entity will copy its render parameters (renderfx, rendermode, rendercolor, renderamt)
// to its targets when triggered.
//


// Flags to indicate masking off various render parameters that are normally copied to the targets
#define SF_RENDER_MASKFX	(1<<0)
#define SF_RENDER_MASKAMT	(1<<1)
#define SF_RENDER_MASKMODE	(1<<2)
#define SF_RENDER_MASKCOLOR	(1<<3)

class CRenderFxManager : public CBaseEntity
{
public:
	void Spawn( void );
	void Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( env_render, CRenderFxManager );


void CRenderFxManager :: Spawn ( void )
{
	pev->solid = SOLID_NOT;
}

void CRenderFxManager :: Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (!FStringNull(pev->target))
	{
		edict_t* pentTarget	= NULL;
		while ( 1 )
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			if (FNullEnt(pentTarget))
				break;

			entvars_t *pevTarget = VARS( pentTarget );
			if ( !FBitSet( pev->spawnflags, SF_RENDER_MASKFX ) )
				pevTarget->renderfx = pev->renderfx;
			if ( !FBitSet( pev->spawnflags, SF_RENDER_MASKAMT ) )
				pevTarget->renderamt = pev->renderamt;
			if ( !FBitSet( pev->spawnflags, SF_RENDER_MASKMODE ) )
				pevTarget->rendermode = pev->rendermode;
			if ( !FBitSet( pev->spawnflags, SF_RENDER_MASKCOLOR ) )
				pevTarget->rendercolor = pev->rendercolor;
		}
	}
}



class CBaseTrigger : public CBaseToggle
{
public:
	void EXPORT TeleportTouch ( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT MultiTouch( CBaseEntity *pOther );
	void EXPORT HurtTouch ( CBaseEntity *pOther );
	void EXPORT CDAudioTouch ( CBaseEntity *pOther );
	void ActivateMultiTrigger( CBaseEntity *pActivator );
	void EXPORT MultiWaitOver( void );
	void EXPORT CounterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void InitTrigger( void );

	virtual int	ObjectCaps( void ) { return CBaseToggle :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS( trigger, CBaseTrigger );

/*
================
InitTrigger
================
*/
void CBaseTrigger::InitTrigger( )
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (pev->angles != g_vecZero)
		SetMovedir(pev);
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	if ( CVAR_GET_FLOAT("showtriggers") == 0 )
		SetBits( pev->effects, EF_NODRAW );
}


//
// Cache user-entity-field values until spawn is called.
//

void CBaseTrigger :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int) atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damagetype"))
	{
		m_bitsDamageInflict = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn( void );
	void EXPORT RadiationThink( void );
};

LINK_ENTITY_TO_CLASS( trigger_hurt, CTriggerHurt );

//
// trigger_monsterjump
//
class CTriggerMonsterJump : public CBaseTrigger
{
public:
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void Think( void );
};

LINK_ENTITY_TO_CLASS( trigger_monsterjump, CTriggerMonsterJump );


void CTriggerMonsterJump :: Spawn ( void )
{
	SetMovedir ( pev );
	
	InitTrigger ();

	pev->nextthink = 0;
	pev->speed = 200;
	m_flHeight = 150;

	if ( !FStringNull ( pev->targetname ) )
	{// if targetted, spawn turned off
		pev->solid = SOLID_NOT;
		UTIL_SetOrigin( pev, pev->origin ); // Unlink from trigger list
		SetUse( ToggleUse );
	}
}


void CTriggerMonsterJump :: Think( void )
{
	pev->solid = SOLID_NOT;// kill the trigger for now !!!UNDONE
	UTIL_SetOrigin( pev, pev->origin ); // Unlink from trigger list
	SetThink( NULL );
}

void CTriggerMonsterJump :: Touch( CBaseEntity *pOther )
{
	entvars_t *pevOther = pOther->pev;

	if ( !FBitSet ( pevOther->flags , FL_MONSTER ) ) 
	{// touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;
	
	if ( FBitSet ( pevOther->flags, FL_ONGROUND ) ) 
	{// clear the onground so physics don't bitch
		pevOther->flags &= ~FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;
	pev->nextthink = gpGlobals->time;
}

// mp3 player, killar
#define SF_REMOVE_ON_FIRE 1

class CTargetFMODAudio : public CPointEntity
{
public:
     void Spawn( void );

     void Use( CBaseEntity *pActivator, CBaseEntity *pCaller,
          USE_TYPE useType, float value );

     BOOL m_bPlaying;
};

LINK_ENTITY_TO_CLASS( ambient_fmodstream, CTargetFMODAudio );
LINK_ENTITY_TO_CLASS( trigger_mp3audio, CTargetFMODAudio );

void CTargetFMODAudio :: Spawn( void )
{
     pev->solid = SOLID_NOT;
     pev->movetype = MOVETYPE_NONE;

     m_bPlaying = FALSE; // start out not playing
}

void CTargetFMODAudio::Use( CBaseEntity *pActivator, CBaseEntity *pCaller,
     USE_TYPE useType, float value )
{
     char command[64];

 //    if (!pActivator->IsPlayer()) // activator should be a player
 //       return;
//sys: just remove. this cause some crashes when the player is hidden in dark or smoke

     if (!m_bPlaying) // if we're not playing, start playing!
          m_bPlaying = TRUE;
     else
     {     // if we're already playing, stop the mp3
          m_bPlaying = FALSE;
          CLIENT_COMMAND(pActivator->edict(), "stopaudio\n");
          return;
     }

     // issue the play/loop command
     sprintf(command, "playaudio %s\n", STRING(pev->message));

     CLIENT_COMMAND(pActivator->edict(), command);

     // remove if set
     if (FBitSet(pev->spawnflags, SF_REMOVE_ON_FIRE))
          UTIL_Remove(this);
}
//=====================================
//
// trigger_cdaudio - starts/stops cd audio tracks
//
class CTriggerCDAudio : public CBaseTrigger
{
public:
	void Spawn( void );

	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void PlayTrack( void );
	void Touch ( CBaseEntity *pOther );
};

LINK_ENTITY_TO_CLASS( trigger_cdaudio, CTriggerCDAudio );

//
// Changes tracks or stops CD when player touches
//
// !!!HACK - overloaded HEALTH to avoid adding new field
void CTriggerCDAudio :: Touch ( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
	{// only clients may trigger these events
		return;
	}

	PlayTrack();
}

void CTriggerCDAudio :: Spawn( void )
{
	InitTrigger();
}

void CTriggerCDAudio::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	PlayTrack();
}

void PlayCDTrack( int iTrack )
{
	edict_t *pClient;
	
	// manually find the single player. 
	pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	
	// Can't play if the client is not connected!
	if ( !pClient )
		return;

	if ( iTrack < -1 || iTrack > 30 )
	{
		ALERT ( at_console, "TriggerCDAudio - Track %d out of range\n" );
		return;
	}

	if ( iTrack == -1 )
	{
		CLIENT_COMMAND ( pClient, "cd pause\n");
	}
	else
	{
		char string [ 64 ];

		sprintf( string, "cd play %3d\n", iTrack );
		CLIENT_COMMAND ( pClient, string);
	}
}


// only plays for ONE client, so only use in single play!
void CTriggerCDAudio :: PlayTrack( void )
{
	PlayCDTrack( (int)pev->health );
	
	SetTouch( NULL );
	UTIL_Remove( this );
}


// This plays a CD track when fired or when the player enters it's radius
class CTargetCDAudio : public CPointEntity
{
public:
	void			Spawn( void );
	void			KeyValue( KeyValueData *pkvd );

	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void			Think( void );
	void			Play( void );
};

LINK_ENTITY_TO_CLASS( target_cdaudio, CTargetCDAudio );

void CTargetCDAudio :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

void CTargetCDAudio :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	if ( pev->scale > 0 )
		pev->nextthink = gpGlobals->time + 1.0;
}

void CTargetCDAudio::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	Play();
}

// only plays for ONE client, so only use in single play!
void CTargetCDAudio::Think( void )
{
	edict_t *pClient;
	
	// manually find the single player. 
	pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	
	// Can't play if the client is not connected!
	if ( !pClient )
		return;
	
	pev->nextthink = gpGlobals->time + 0.5;

	if ( (pClient->v.origin - pev->origin).Length() <= pev->scale )
		Play();

}

void CTargetCDAudio::Play( void ) 
{ 
	PlayCDTrack( (int)pev->health );
	UTIL_Remove(this); 
}

//=====================================

//
// trigger_hurt - hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
//
//int gfToggleState = 0; // used to determine when all radiation trigger hurts have called 'RadiationThink'

void CTriggerHurt :: Spawn( void )
{
	InitTrigger();
	SetTouch ( HurtTouch );

	if ( !FStringNull ( pev->targetname ) )
	{
		SetUse ( ToggleUse );
	}
	else
	{
		SetUse ( NULL );
	}

	if (m_bitsDamageInflict & DMG_RADIATION)
	{
		SetThink ( RadiationThink );
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5); 
	}

	if ( FBitSet (pev->spawnflags, SF_TRIGGER_HURT_START_OFF) )// if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	UTIL_SetOrigin( pev, pev->origin );		// Link into the list
}

// trigger hurt that causes radiation will do a radius
// check and set the player's geiger counter level
// according to distance from center of trigger

void CTriggerHurt :: RadiationThink( void )
{

	edict_t *pentPlayer;
	CBasePlayer *pPlayer = NULL;
	float flRange;
	entvars_t *pevTarget;
	Vector vecSpot1;
	Vector vecSpot2;
	Vector vecRange;
	Vector origin;
	Vector view_ofs;

	// check to see if a player is in pvs
	// if not, continue	

	// set origin to center of trigger so that this check works
	origin = pev->origin;
	view_ofs = pev->view_ofs;

	pev->origin = (pev->absmin + pev->absmax) * 0.5;
	pev->view_ofs = pev->view_ofs * 0.0;

	pentPlayer = FIND_CLIENT_IN_PVS(edict());

	pev->origin = origin;
	pev->view_ofs = view_ofs;

	// reset origin

	if (!FNullEnt(pentPlayer))
	{
 
		pPlayer = GetClassPtr( (CBasePlayer *)VARS(pentPlayer));

		pevTarget = VARS(pentPlayer);

		// get range to player;

		vecSpot1 = (pev->absmin + pev->absmax) * 0.5;
		vecSpot2 = (pevTarget->absmin + pevTarget->absmax) * 0.5;
		
		vecRange = vecSpot1 - vecSpot2;
		flRange = vecRange.Length();

		// if player's current geiger counter range is larger
		// than range to this trigger hurt, reset player's
		// geiger counter range 

		if (pPlayer->m_flgeigerRange >= flRange)
			pPlayer->m_flgeigerRange = flRange;
	}

	pev->nextthink = gpGlobals->time + 0.25;
}

//
// ToggleUse - If this is the USE function for a trigger, its state will toggle every time it's fired
//
void CBaseTrigger :: ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (pev->solid == SOLID_NOT)
	{// if the trigger is off, turn it on
		pev->solid = SOLID_TRIGGER;
		
		// Force retouch
		gpGlobals->force_retouch++;
	}
	else
	{// turn the trigger off
		pev->solid = SOLID_NOT;
	}
	UTIL_SetOrigin( pev, pev->origin );
}

// When touched, a hurt trigger does DMG points of damage each half-second
void CBaseTrigger :: HurtTouch ( CBaseEntity *pOther )
{
	float fldmg;

	if ( !pOther->pev->takedamage )
		return;

	if ( (pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYTOUCH) && !pOther->IsPlayer() )
	{
		// this trigger is only allowed to touch clients, and this ain't a client.
		return;
	}

	if ( (pev->spawnflags & SF_TRIGGER_HURT_NO_CLIENTS) && pOther->IsPlayer() )
		return;

	// HACKHACK -- In multiplayer, players touch this based on packet receipt.
	// So the players who send packets later aren't always hurt.  Keep track of
	// how much time has passed and whether or not you've touched that player
	if ( g_pGameRules->IsMultiplayer() )
	{
		if ( pev->dmgtime > gpGlobals->time )
		{
			if ( gpGlobals->time != pev->pain_finished )
			{// too early to hurt again, and not same frame with a different entity
				if ( pOther->IsPlayer() )
				{
					int playerMask = 1 << (pOther->entindex() - 1);

					// If I've already touched this player (this time), then bail out
					if ( pev->impulse & playerMask )
						return;

					// Mark this player as touched
					// BUGBUG - There can be only 32 players!
					pev->impulse |= playerMask;
				}
				else
				{
					return;
				}
			}
		}
		else
		{
			// New clock, "un-touch" all players
			pev->impulse = 0;
			if ( pOther->IsPlayer() )
			{
				int playerMask = 1 << (pOther->entindex() - 1);

				// Mark this player as touched
				// BUGBUG - There can be only 32 players!
				pev->impulse |= playerMask;
			}
		}
	}
	else	// Original code -- single player
	{
		if ( pev->dmgtime > gpGlobals->time && gpGlobals->time != pev->pain_finished )
		{// too early to hurt again, and not same frame with a different entity
			return;
		}
	}



	// If this is time_based damage (poison, radiation), override the pev->dmg with a 
	// default for the given damage type.  Monsters only take time-based damage
	// while touching the trigger.  Player continues taking damage for a while after
	// leaving the trigger

	fldmg = pev->dmg * 0.5;	// 0.5 seconds worth of damage, pev->dmg is damage/second


	// JAY: Cut this because it wasn't fully realized.  Damage is simpler now.
#if 0
	switch (m_bitsDamageInflict)
	{
	default: break;
	case DMG_POISON:		fldmg = POISON_DAMAGE/4; break;
	case DMG_NERVEGAS:		fldmg = NERVEGAS_DAMAGE/4; break;
	case DMG_RADIATION:		fldmg = RADIATION_DAMAGE/4; break;
	case DMG_PARALYZE:		fldmg = PARALYZE_DAMAGE/4; break; // UNDONE: cut this? should slow movement to 50%
	case DMG_ACID:			fldmg = ACID_DAMAGE/4; break;
	case DMG_SLOWBURN:		fldmg = SLOWBURN_DAMAGE/4; break;
	case DMG_SLOWFREEZE:	fldmg = SLOWFREEZE_DAMAGE/4; break;
	}
#endif

	if ( fldmg < 0 )
		pOther->TakeHealth( -fldmg, m_bitsDamageInflict );
	else
		pOther->TakeDamage( pev, pev, fldmg, m_bitsDamageInflict );

	// Store pain time so we can get all of the other entities on this frame
	pev->pain_finished = gpGlobals->time;

	// Apply damage every half second
	pev->dmgtime = gpGlobals->time + 0.5;// half second delay until this trigger can hurt toucher again

  
	
	if ( pev->target )
	{
		// trigger has a target it wants to fire. 
		if ( pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYFIRE )
		{
			// if the toucher isn't a client, don't fire the target!
			if ( !pOther->IsPlayer() )
			{
				return;
			}
		}

		SUB_UseTargets( pOther, USE_TOGGLE, 0 );
		if ( pev->spawnflags & SF_TRIGGER_HURT_TARGETONCE )
			pev->target = 0;
	}
}


/*QUAKED trigger_multiple (.5 .5 .5) ? notouch
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
If notouch is set, the trigger is only fired by other entities, not by touching.
NOTOUCH has been obsoleted by trigger_relay!
sounds
1)      secret
2)      beep beep
3)      large switch
4)
NEW
if a trigger has a NETNAME, that NETNAME will become the TARGET of the triggered object.
*/
class CTriggerMultiple : public CBaseTrigger
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( trigger_multiple, CTriggerMultiple );


void CTriggerMultiple :: Spawn( void )
{
	if (m_flWait == 0)
		m_flWait = 0.2;

	InitTrigger();

	ASSERTSZ(pev->health == 0, "trigger_multiple with health");
//	UTIL_SetOrigin(pev, pev->origin);
//	SET_MODEL( ENT(pev), STRING(pev->model) );
//	if (pev->health > 0)
//		{
//		if (FBitSet(pev->spawnflags, SPAWNFLAG_NOTOUCH))
//			ALERT(at_error, "trigger_multiple spawn: health and notouch don't make sense");
//		pev->max_health = pev->health;
//UNDONE: where to get pfnDie from?
//		pev->pfnDie = multi_killed;
//		pev->takedamage = DAMAGE_YES;
//		pev->solid = SOLID_BBOX;
//		UTIL_SetOrigin(pev, pev->origin);  // make sure it links into the world
//		}
//	else
		{
			SetTouch( MultiTouch );
		}
	}


/*QUAKED trigger_once (.5 .5 .5) ? notouch
Variable sized trigger. Triggers once, then removes itself.  You must set the key "target" to the name of another object in the level that has a matching
"targetname".  If "health" is set, the trigger must be killed to activate.
If notouch is set, the trigger is only fired by other entities, not by touching.
if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
sounds
1)      secret
2)      beep beep
3)      large switch
4)
*/
class CTriggerOnce : public CTriggerMultiple
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( trigger_once, CTriggerOnce );
void CTriggerOnce::Spawn( void )
{
	m_flWait = -1;
	
	CTriggerMultiple :: Spawn();
}



void CBaseTrigger :: MultiTouch( CBaseEntity *pOther )
{
	entvars_t	*pevToucher;

	pevToucher = pOther->pev;

	// Only touch clients, monsters, or pushables (depending on flags)
	if ( ((pevToucher->flags & FL_CLIENT) && !(pev->spawnflags & SF_TRIGGER_NOCLIENTS)) ||
		 ((pevToucher->flags & FL_MONSTER) && (pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS)) ||
		 (pev->spawnflags & SF_TRIGGER_PUSHABLES) && FClassnameIs(pevToucher,"func_pushable") )
	{

#if 0
		// if the trigger has an angles field, check player's facing direction
		if (pev->movedir != g_vecZero)
		{
			UTIL_MakeVectors( pevToucher->angles );
			if ( DotProduct( gpGlobals->v_forward, pev->movedir ) < 0 )
				return;         // not facing the right way
		}
#endif
		
		ActivateMultiTrigger( pOther );
	}
}


//
// the trigger was just touched/killed/used
// self.enemy should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
//
void CBaseTrigger :: ActivateMultiTrigger( CBaseEntity *pActivator )
{
	if (pev->nextthink > gpGlobals->time)
		return;         // still waiting for reset time

	if (!UTIL_IsMasterTriggered(m_sMaster,pActivator))
		return;

	if (FClassnameIs(pev, "trigger_secret"))
	{
		if ( pev->enemy == NULL || !FClassnameIs(pev->enemy, "player"))
			return;
		gpGlobals->found_secrets++;
	}

	if (!FStringNull(pev->noise))
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

// don't trigger again until reset
// pev->takedamage = DAMAGE_NO;

	m_hActivator = pActivator;
	SUB_UseTargets( m_hActivator, USE_TOGGLE, 0 );

	if ( pev->message && pActivator->IsPlayer() )
	{
		UTIL_ShowMessage( STRING(pev->message), pActivator );
//		CLIENT_PRINTF( ENT( pActivator->pev ), print_center, STRING(pev->message) );
	}

	if (m_flWait > 0)
	{
		SetThink( MultiWaitOver );
		pev->nextthink = gpGlobals->time + m_flWait;
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch( NULL );
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(  SUB_Remove );
	}
}


// the wait time has passed, so set back up for another activation
void CBaseTrigger :: MultiWaitOver( void )
{
//	if (pev->max_health)
//		{
//		pev->health		= pev->max_health;
//		pev->takedamage	= DAMAGE_YES;
//		pev->solid		= SOLID_BBOX;
//		}
	SetThink( NULL );
}


// ========================= COUNTING TRIGGER =====================================

//
// GLOBALS ASSUMED SET:  g_eoActivator
//
void CBaseTrigger::CounterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_cTriggersLeft--;
	m_hActivator = pActivator;

	if (m_cTriggersLeft < 0)
		return;
	
	BOOL fTellActivator =
		(m_hActivator != 0) &&
		FClassnameIs(m_hActivator->pev, "player") &&
		!FBitSet(pev->spawnflags, SPAWNFLAG_NOMESSAGE);
	if (m_cTriggersLeft != 0)
	{
		if (fTellActivator)
		{
			// UNDONE: I don't think we want these Quakesque messages
			switch (m_cTriggersLeft)
			{
			case 1:		ALERT(at_console, "Only 1 more to go...");		break;
			case 2:		ALERT(at_console, "Only 2 more to go...");		break;
			case 3:		ALERT(at_console, "Only 3 more to go...");		break;
			default:	ALERT(at_console, "There are more to go...");	break;
			}
		}
		return;
	}

	// !!!UNDONE: I don't think we want these Quakesque messages
	if (fTellActivator)
		ALERT(at_console, "Sequence completed!");
	
	ActivateMultiTrigger( m_hActivator );
}


/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.
If nomessage is not set, it will print "1 more.. " etc when triggered and
"sequence complete" when finished.  After the counter has been triggered "cTriggersLeft"
times (default 2), it will fire all of it's targets and remove itself.
*/
class CTriggerCounter : public CBaseTrigger
{
public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS( trigger_counter, CTriggerCounter );

void CTriggerCounter :: Spawn( void )
{
	// By making the flWait be -1, this counter-trigger will disappear after it's activated
	// (but of course it needs cTriggersLeft "uses" before that happens).
	m_flWait = -1;

	if (m_cTriggersLeft == 0)
		m_cTriggersLeft = 2;
	SetUse( CounterUse );
}

// ====================== TRIGGER_CHANGELEVEL ================================

class CTriggerVolume : public CPointEntity	// Derive from point entity so this doesn't move across levels
{
public:
	void		Spawn( void );
};

LINK_ENTITY_TO_CLASS( trigger_transition, CTriggerVolume );

// Define space that travels across a level transition
void CTriggerVolume :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	pev->model = NULL;
	pev->modelindex = 0;
}


// Fires a target after level transition and then dies
class CFireAndDie : public CBaseDelay
{
public:
	void Spawn( void );
	void Precache( void );
	void Think( void );
	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() | FCAP_FORCE_TRANSITION; }	// Always go across transitions
};
LINK_ENTITY_TO_CLASS( fireanddie, CFireAndDie );

void CFireAndDie::Spawn( void )
{
	pev->classname = MAKE_STRING("fireanddie");
	// Don't call Precache() - it should be called on restore
}


void CFireAndDie::Precache( void )
{
	// This gets called on restore
	pev->nextthink = gpGlobals->time + m_flDelay;
}


void CFireAndDie::Think( void )
{
	SUB_UseTargets( this, USE_TOGGLE, 0 );
	UTIL_Remove( this );
}


#define SF_CHANGELEVEL_USEONLY		0x0002
class CChangeLevel : public CBaseTrigger
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT UseChangeLevel ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TriggerChangeLevel( void );
	void EXPORT ExecuteChangeLevel( void );
	void EXPORT TouchChangeLevel( CBaseEntity *pOther );
	void ChangeLevelNow( CBaseEntity *pActivator );

	static edict_t *FindLandmark( const char *pLandmarkName );
	static int ChangeList( LEVELLIST *pLevelList, int maxList );
	static int AddTransitionToList( LEVELLIST *pLevelList, int listCount, const char *pMapName, const char *pLandmarkName, edict_t *pentLandmark );
	static int InTransitionVolume( CBaseEntity *pEntity, char *pVolumeName );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	char m_szMapName[cchMapNameMost];		// trigger_changelevel only:  next map
	char m_szLandmarkName[cchMapNameMost];		// trigger_changelevel only:  landmark on next map
	int		m_changeTarget;
	float	m_changeTargetDelay;
};
LINK_ENTITY_TO_CLASS( trigger_changelevel, CChangeLevel );

// Global Savedata for changelevel trigger
TYPEDESCRIPTION	CChangeLevel::m_SaveData[] = 
{
	DEFINE_ARRAY( CChangeLevel, m_szMapName, FIELD_CHARACTER, cchMapNameMost ),
	DEFINE_ARRAY( CChangeLevel, m_szLandmarkName, FIELD_CHARACTER, cchMapNameMost ),
	DEFINE_FIELD( CChangeLevel, m_changeTarget, FIELD_STRING ),
	DEFINE_FIELD( CChangeLevel, m_changeTargetDelay, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE(CChangeLevel,CBaseTrigger);

//
// Cache user-entity-field values until spawn is called.
//

void CChangeLevel :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "map"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT( at_error, "Map name '%s' too long (32 chars)\n", pkvd->szValue );
		strcpy(m_szMapName, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "landmark"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT( at_error, "Landmark name '%s' too long (32 chars)\n", pkvd->szValue );
		strcpy(m_szLandmarkName, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "changetarget"))
	{
		m_changeTarget = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "changedelay"))
	{
		m_changeTargetDelay = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue( pkvd );
}


/*QUAKED trigger_changelevel (0.5 0.5 0.5) ? NO_INTERMISSION
When the player touches this, he gets sent to the map listed in the "map" variable.  Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission spot and display stats.
*/

void CChangeLevel :: Spawn( void )
{
	if ( FStrEq( m_szMapName, "" ) )
		ALERT( at_console, "a trigger_changelevel doesn't have a map" );

	if ( FStrEq( m_szLandmarkName, "" ) )
		ALERT( at_console, "trigger_changelevel to %s doesn't have a landmark", m_szMapName );

	if (!FStringNull ( pev->targetname ) )
	{
		SetUse ( UseChangeLevel );
	}
	InitTrigger();
	if ( !(pev->spawnflags & SF_CHANGELEVEL_USEONLY) )
		SetTouch( TouchChangeLevel );
//	ALERT( at_console, "TRANSITION: %s (%s)\n", m_szMapName, m_szLandmarkName );
}


void CChangeLevel :: ExecuteChangeLevel( void )
{
	MESSAGE_BEGIN( MSG_ALL, SVC_CDTRACK );
		WRITE_BYTE( 3 );
		WRITE_BYTE( 3 );
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();
}


FILE_GLOBAL char st_szNextMap[cchMapNameMost];
FILE_GLOBAL char st_szNextSpot[cchMapNameMost];

edict_t *CChangeLevel :: FindLandmark( const char *pLandmarkName )
{
	edict_t	*pentLandmark;

	pentLandmark = FIND_ENTITY_BY_STRING( NULL, "targetname", pLandmarkName );
	while ( !FNullEnt( pentLandmark ) )
	{
		// Found the landmark
		if ( FClassnameIs( pentLandmark, "info_landmark" ) )
			return pentLandmark;
		else
			pentLandmark = FIND_ENTITY_BY_STRING( pentLandmark, "targetname", pLandmarkName );
	}
	ALERT( at_error, "Can't find landmark %s\n", pLandmarkName );
	return NULL;
}


//=========================================================
// CChangeLevel :: Use - allows level transitions to be 
// triggered by buttons, etc.
//
//=========================================================
void CChangeLevel :: UseChangeLevel ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	ChangeLevelNow( pActivator );
}

void CChangeLevel :: ChangeLevelNow( CBaseEntity *pActivator )
{
	edict_t	*pentLandmark;
	LEVELLIST	levels[16];

	ASSERT(!FStrEq(m_szMapName, ""));

	// Don't work in deathmatch
	if ( g_pGameRules->IsDeathmatch() )
		return;

	// Some people are firing these multiple times in a frame, disable
	if ( gpGlobals->time == pev->dmgtime )
		return;

	pev->dmgtime = gpGlobals->time;


	CBaseEntity *pPlayer = CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );
	if ( !InTransitionVolume( pPlayer, m_szLandmarkName ) )
	{
		ALERT( at_aiconsole, "Player isn't in the transition volume %s, aborting\n", m_szLandmarkName );
		return;
	}

	// Create an entity to fire the changetarget
	if ( m_changeTarget )
	{
		CFireAndDie *pFireAndDie = GetClassPtr( (CFireAndDie *)NULL );
		if ( pFireAndDie )
		{
			// Set target and delay
			pFireAndDie->pev->target = m_changeTarget;
			pFireAndDie->m_flDelay = m_changeTargetDelay;
			pFireAndDie->pev->origin = pPlayer->pev->origin;
			// Call spawn
			DispatchSpawn( pFireAndDie->edict() );
		}
	}
	// This object will get removed in the call to CHANGE_LEVEL, copy the params into "safe" memory
	strcpy(st_szNextMap, m_szMapName);

	m_hActivator = pActivator;
	SUB_UseTargets( pActivator, USE_TOGGLE, 0 );
	st_szNextSpot[0] = 0;	// Init landmark to NULL

	// look for a landmark entity		
	pentLandmark = FindLandmark( m_szLandmarkName );
	if ( !FNullEnt( pentLandmark ) )
	{
		strcpy(st_szNextSpot, m_szLandmarkName);
		gpGlobals->vecLandmarkOffset = VARS(pentLandmark)->origin;
	}
//	ALERT( at_console, "Level touches %d levels\n", ChangeList( levels, 16 ) );
	ALERT( at_console, "\n\nCHANGE LEVEL: %s %s\n\n\n", st_szNextMap, st_szNextSpot );
	
	CHANGE_LEVEL( st_szNextMap, st_szNextSpot );
}

//
// GLOBALS ASSUMED SET:  st_szNextMap
//
void CChangeLevel :: TouchChangeLevel( CBaseEntity *pOther )
{
	if (!FClassnameIs(pOther->pev, "player"))
		return;

	extern int gmsgParticles;
	MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
		WRITE_SHORT(0);
		WRITE_BYTE(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_SHORT(9999);
		WRITE_STRING("");
	MESSAGE_END();

//	MessageBox(NULL, "Particle engine has been reseted, press OK to change level", " ", MB_OK | MB_ICONWARNING);

	ChangeLevelNow( pOther );
}


// Add a transition to the list, but ignore duplicates 
// (a designer may have placed multiple trigger_changelevels with the same landmark)
int CChangeLevel::AddTransitionToList( LEVELLIST *pLevelList, int listCount, const char *pMapName, const char *pLandmarkName, edict_t *pentLandmark )
{
	int i;

	if ( !pLevelList || !pMapName || !pLandmarkName || !pentLandmark )
		return 0;

	for ( i = 0; i < listCount; i++ )
	{
		if ( pLevelList[i].pentLandmark == pentLandmark && strcmp( pLevelList[i].mapName, pMapName ) == 0 )
			return 0;
	}
	strcpy( pLevelList[listCount].mapName, pMapName );
	strcpy( pLevelList[listCount].landmarkName, pLandmarkName );
	pLevelList[listCount].pentLandmark = pentLandmark;
	pLevelList[listCount].vecLandmarkOrigin = VARS(pentLandmark)->origin;

	return 1;
}

int BuildChangeList( LEVELLIST *pLevelList, int maxList )
{
	return CChangeLevel::ChangeList( pLevelList, maxList );
}


int CChangeLevel::InTransitionVolume( CBaseEntity *pEntity, char *pVolumeName )
{
	edict_t	*pentVolume;


	if ( pEntity->ObjectCaps() & FCAP_FORCE_TRANSITION )
		return 1;

	// If you're following another entity, follow it through the transition (weapons follow the player)
	if ( pEntity->pev->movetype == MOVETYPE_FOLLOW )
	{
		if ( pEntity->pev->aiment != NULL )
			pEntity = CBaseEntity::Instance( pEntity->pev->aiment );
	}

	int inVolume = 1;	// Unless we find a trigger_transition, everything is in the volume

	pentVolume = FIND_ENTITY_BY_TARGETNAME( NULL, pVolumeName );
	while ( !FNullEnt( pentVolume ) )
	{
		CBaseEntity *pVolume = CBaseEntity::Instance( pentVolume );
		
		if ( pVolume && FClassnameIs( pVolume->pev, "trigger_transition" ) )
		{
			if ( pVolume->Intersects( pEntity ) )	// It touches one, it's in the volume
				return 1;
			else
				inVolume = 0;	// Found a trigger_transition, but I don't intersect it -- if I don't find another, don't go!
		}
		pentVolume = FIND_ENTITY_BY_TARGETNAME( pentVolume, pVolumeName );
	}

	return inVolume;
}


// We can only ever move 512 entities across a transition
#define MAX_ENTITY 512

// This has grown into a complicated beast
// Can we make this more elegant?
// This builds the list of all transitions on this level and which entities are in their PVS's and can / should
// be moved across.
int CChangeLevel::ChangeList( LEVELLIST *pLevelList, int maxList )
{
	edict_t	*pentChangelevel, *pentLandmark;
	int			i, count;

	count = 0;

	// Find all of the possible level changes on this BSP
	pentChangelevel = FIND_ENTITY_BY_STRING( NULL, "classname", "trigger_changelevel" );
	if ( FNullEnt( pentChangelevel ) )
		return 0;
	while ( !FNullEnt( pentChangelevel ) )
	{
		CChangeLevel *pTrigger;
		
		pTrigger = GetClassPtr((CChangeLevel *)VARS(pentChangelevel));
		if ( pTrigger )
		{
			// Find the corresponding landmark
			pentLandmark = FindLandmark( pTrigger->m_szLandmarkName );
			if ( pentLandmark )
			{
				// Build a list of unique transitions
				if ( AddTransitionToList( pLevelList, count, pTrigger->m_szMapName, pTrigger->m_szLandmarkName, pentLandmark ) )
				{
					count++;
					if ( count >= maxList )		// FULL!!
						break;
				}
			}
		}
		pentChangelevel = FIND_ENTITY_BY_STRING( pentChangelevel, "classname", "trigger_changelevel" );
	}

	if ( gpGlobals->pSaveData && ((SAVERESTOREDATA *)gpGlobals->pSaveData)->pTable )
	{
		CSave saveHelper( (SAVERESTOREDATA *)gpGlobals->pSaveData );

		for ( i = 0; i < count; i++ )
		{
			int j, entityCount = 0;
			CBaseEntity *pEntList[ MAX_ENTITY ];
			int			 entityFlags[ MAX_ENTITY ];

			// Follow the linked list of entities in the PVS of the transition landmark
			edict_t *pent = UTIL_EntitiesInPVS( pLevelList[i].pentLandmark );

			// Build a list of valid entities in this linked list (we're going to use pent->v.chain again)
			while ( !FNullEnt( pent ) )
			{
				CBaseEntity *pEntity = CBaseEntity::Instance(pent);
				if ( pEntity )
				{
//					ALERT( at_console, "Trying %s\n", STRING(pEntity->pev->classname) );
					int caps = pEntity->ObjectCaps();
					if ( !(caps & FCAP_DONT_SAVE) )
					{
						int flags = 0;

						// If this entity can be moved or is global, mark it
						if ( caps & FCAP_ACROSS_TRANSITION )
							flags |= FENTTABLE_MOVEABLE;
						if ( pEntity->pev->globalname && !pEntity->IsDormant() )
							flags |= FENTTABLE_GLOBAL;
						if ( flags )
						{
							pEntList[ entityCount ] = pEntity;
							entityFlags[ entityCount ] = flags;
							entityCount++;
							if ( entityCount > MAX_ENTITY )
								ALERT( at_error, "Too many entities across a transition!" );
						}
						else
							ALERT( at_console, "Failed %s\n", STRING(pEntity->pev->classname) );
					}
					else
						ALERT( at_console, "DON'T SAVE %s\n", STRING(pEntity->pev->classname) );
				}
				pent = pent->v.chain;
			}

			for ( j = 0; j < entityCount; j++ )
			{
				// Check to make sure the entity isn't screened out by a trigger_transition
				if ( entityFlags[j] && InTransitionVolume( pEntList[j], pLevelList[i].landmarkName ) )
				{
					// Mark entity table with 1<<i
					int index = saveHelper.EntityIndex( pEntList[j] );
					// Flag it with the level number
					saveHelper.EntityFlagsSet( index, entityFlags[j] | (1<<i) );
				}
				else
					ALERT( at_console, "Screened out %s\n", STRING(pEntList[j]->pev->classname) );
			}
		}
	}

	return count;
}

/*
go to the next level for deathmatch
only called if a time or frag limit has expired
*/
void NextLevel( void )
{
	edict_t* pent;
	CChangeLevel *pChange;
	
	// find a trigger_changelevel
	pent = FIND_ENTITY_BY_CLASSNAME(NULL, "trigger_changelevel");
	
	// go back to start if no trigger_changelevel
	if (FNullEnt(pent))
	{
		gpGlobals->mapname = ALLOC_STRING("start");
		pChange = GetClassPtr( (CChangeLevel *)NULL );
		strcpy(pChange->m_szMapName, "start");
	}
	else
		pChange = GetClassPtr( (CChangeLevel *)VARS(pent));
	
	strcpy(st_szNextMap, pChange->m_szMapName);
	g_fGameOver = TRUE;
	
	if (pChange->pev->nextthink < gpGlobals->time)
	{
		pChange->SetThink( CChangeLevel::ExecuteChangeLevel );
		pChange->pev->nextthink = gpGlobals->time + 0.1;
	}
}


// ============================== LADDER =======================================

class CLadder : public CBaseTrigger
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Precache( void );
};
LINK_ENTITY_TO_CLASS( func_ladder, CLadder );


void CLadder :: KeyValue( KeyValueData *pkvd )
{
	CBaseTrigger::KeyValue( pkvd );
}


//=========================================================
// func_ladder - makes an area vertically negotiable
//=========================================================
void CLadder :: Precache( void )
{
	// Do all of this in here because we need to 'convert' old saved games
	pev->solid = SOLID_NOT;
	pev->skin = CONTENTS_LADDER;
	if ( CVAR_GET_FLOAT("showtriggers") == 0 )
	{
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0;
	}
	pev->effects &= ~EF_NODRAW;
}


void CLadder :: Spawn( void )
{
	Precache();

	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	pev->movetype = MOVETYPE_PUSH;
}


// ========================== A TRIGGER THAT PUSHES YOU ===============================

class CTriggerPush : public CBaseTrigger
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void Touch( CBaseEntity *pOther );
};
LINK_ENTITY_TO_CLASS( trigger_push, CTriggerPush );


void CTriggerPush :: KeyValue( KeyValueData *pkvd )
{
	CBaseTrigger::KeyValue( pkvd );
}


/*QUAKED trigger_push (.5 .5 .5) ? TRIG_PUSH_ONCE
Pushes the player
*/

void CTriggerPush :: Spawn( )
{
	if ( pev->angles == g_vecZero )
		pev->angles.y = 360;
	InitTrigger();

	if (pev->speed == 0)
		pev->speed = 100;

	if ( FBitSet (pev->spawnflags, SF_TRIGGER_PUSH_START_OFF) )// if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	SetUse( ToggleUse );

	UTIL_SetOrigin( pev, pev->origin );		// Link into the list
}


void CTriggerPush :: Touch( CBaseEntity *pOther )
{
	entvars_t* pevToucher = pOther->pev;

	// UNDONE: Is there a better way than health to detect things that have physics? (clients/monsters)
	switch( pevToucher->movetype )
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
	case MOVETYPE_FOLLOW:
		return;
	}

	if ( pevToucher->solid != SOLID_NOT && pevToucher->solid != SOLID_BSP )
	{
		// Instant trigger, just transfer velocity and remove
		if (FBitSet(pev->spawnflags, SF_TRIG_PUSH_ONCE))
		{
			pevToucher->velocity = pevToucher->velocity + (pev->speed * pev->movedir);
			if ( pevToucher->velocity.z > 0 )
				pevToucher->flags &= ~FL_ONGROUND;
			UTIL_Remove( this );
		}
		else
		{	// Push field, transfer to base velocity
			Vector vecPush = (pev->speed * pev->movedir);
			if ( pevToucher->flags & FL_BASEVELOCITY )
				vecPush = vecPush +  pevToucher->basevelocity;

			pevToucher->basevelocity = vecPush;

			pevToucher->flags |= FL_BASEVELOCITY;
//			ALERT( at_console, "Vel %f, base %f\n", pevToucher->velocity.z, pevToucher->basevelocity.z );
		}
	}
}


//======================================
// teleport trigger
//
//

void CBaseTrigger :: TeleportTouch( CBaseEntity *pOther )
{
	entvars_t* pevToucher = pOther->pev;
	edict_t	*pentTarget = NULL;

	// Only teleport monsters or clients
	if ( !FBitSet( pevToucher->flags, FL_CLIENT|FL_MONSTER ) )
		return;
    
	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;
 	
	if ( !( pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS ) )
	{// no monsters allowed!
		if ( FBitSet( pevToucher->flags, FL_MONSTER ) )
		{
			return;
		}
	}

	if ( ( pev->spawnflags & SF_TRIGGER_NOCLIENTS ) )
	{// no clients allowed
		if ( pOther->IsPlayer() )
		{
			return;
		}
	}
	
	pentTarget = FIND_ENTITY_BY_TARGETNAME( pentTarget, STRING(pev->target) );
	if (FNullEnt(pentTarget))
	   return;	
	
	Vector tmp = VARS( pentTarget )->origin;

	if ( pOther->IsPlayer() )
	{
		tmp.z -= pOther->pev->mins.z;// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
	}

	tmp.z++;

	pevToucher->flags &= ~FL_ONGROUND;
	
	UTIL_SetOrigin( pevToucher, tmp );

	pevToucher->angles = pentTarget->v.angles;

	if ( pOther->IsPlayer() )
	{
		pevToucher->v_angle = pentTarget->v.angles;
	}

	pevToucher->fixangle = TRUE;
	pevToucher->velocity = pevToucher->basevelocity = g_vecZero;
}


class CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS( trigger_teleport, CTriggerTeleport );

void CTriggerTeleport :: Spawn( void )
{
	InitTrigger();

	SetTouch( TeleportTouch );
}


LINK_ENTITY_TO_CLASS( info_teleport_destination, CPointEntity );



class CTriggerSave : public CBaseTrigger
{
public:
	void Spawn( void );
	void EXPORT SaveTouch( CBaseEntity *pOther );
};
LINK_ENTITY_TO_CLASS( trigger_autosave, CTriggerSave );

void CTriggerSave::Spawn( void )
{
	if ( g_pGameRules->IsDeathmatch() )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	InitTrigger();
	SetTouch( SaveTouch );
}

void CTriggerSave::SaveTouch( CBaseEntity *pOther )
{
	if ( !UTIL_IsMasterTriggered( m_sMaster, pOther ) )
		return;

	// Only save on clients
	if ( !pOther->IsPlayer() )
		return;
    
	SetTouch( NULL );
	UTIL_Remove( this );
	SERVER_COMMAND( "autosave\n" );
		
	ClientPrint(pOther->pev, HUD_PRINTCENTER, "#Checkpoint");
}

#define SF_ENDSECTION_USEONLY		0x0001

class CTriggerEndSection : public CBaseTrigger
{
public:
	void Spawn( void );
	void EXPORT EndSectionTouch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT EndSectionUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( trigger_endsection, CTriggerEndSection );


void CTriggerEndSection::EndSectionUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Only save on clients
	if ( pActivator && !pActivator->IsNetClient() )
		return;
    
	SetUse( NULL );

	if ( pev->message )
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove( this );
}

void CTriggerEndSection::Spawn( void )
{
	if ( g_pGameRules->IsDeathmatch() )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	InitTrigger();

	SetUse ( EndSectionUse );
	// If it is a "use only" trigger, then don't set the touch function.
	if ( ! (pev->spawnflags & SF_ENDSECTION_USEONLY) )
		SetTouch( EndSectionTouch );
}

void CTriggerEndSection::EndSectionTouch( CBaseEntity *pOther )
{
	// Only save on clients
	if ( !pOther->IsNetClient() )
		return;
    
	SetTouch( NULL );

	if (pev->message)
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove( this );
}

void CTriggerEndSection :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "section"))
	{
//		m_iszSectionName = ALLOC_STRING( pkvd->szValue );
		// Store this in message so we don't have to write save/restore for this ent
		pev->message = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue( pkvd );
}


class CTriggerGravity : public CBaseTrigger
{
public:
	void Spawn( void );
	void EXPORT GravityTouch( CBaseEntity *pOther );
};
LINK_ENTITY_TO_CLASS( trigger_gravity, CTriggerGravity );

void CTriggerGravity::Spawn( void )
{
	InitTrigger();
	SetTouch( GravityTouch );
}

void CTriggerGravity::GravityTouch( CBaseEntity *pOther )
{
	// Only save on clients
	if ( !pOther->IsPlayer() )
		return;

	pOther->pev->gravity = pev->gravity;
}

//===========================================================
//LRC- trigger_startpatrol
//===========================================================
class CTriggerSetPatrol : public CBaseDelay
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

private:
	int		m_iszPath;
};
LINK_ENTITY_TO_CLASS( trigger_startpatrol, CTriggerSetPatrol );

TYPEDESCRIPTION	CTriggerSetPatrol::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerSetPatrol, m_iszPath, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE(CTriggerSetPatrol,CBaseDelay);

void CTriggerSetPatrol::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iszPath"))
	{
		m_iszPath = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}

void CTriggerSetPatrol::Spawn( void )
{
}


void CTriggerSetPatrol::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pTarget = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ));
	CBaseEntity *pPath = UTIL_FindEntityByTargetname( NULL, STRING( m_iszPath ) );

	if (pTarget && pPath)
	{
		CBaseMonster *pMonster = pTarget->MyMonsterPointer();
		if (pMonster) pMonster->StartPatrol(pPath);
	}
}







// this is a really bad idea.
class CTriggerChangeTarget : public CBaseDelay
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

private:
	int		m_iszNewTarget;
};
LINK_ENTITY_TO_CLASS( trigger_changetarget, CTriggerChangeTarget );

TYPEDESCRIPTION	CTriggerChangeTarget::m_SaveData[] = 
{
	DEFINE_FIELD( CTriggerChangeTarget, m_iszNewTarget, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeTarget,CBaseDelay);

void CTriggerChangeTarget::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iszNewTarget"))
	{
		m_iszNewTarget = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}

void CTriggerChangeTarget::Spawn( void )
{
}


void CTriggerChangeTarget::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pTarget = UTIL_FindEntityByString( NULL, "targetname", STRING( pev->target ) );

	if (pTarget)
	{
		pTarget->pev->target = m_iszNewTarget;
		CBaseMonster *pMonster = pTarget->MyMonsterPointer( );
		if (pMonster)
		{
			pMonster->m_pGoalEnt = NULL;
		}
	}
}
//LRC - you thought _that_ was a bad idea? Check this baby out...
class CTriggerChangeValue : public CBaseDelay
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

private:
	int		m_iszNewValue;
};
LINK_ENTITY_TO_CLASS( trigger_changevalue, CTriggerChangeValue );

TYPEDESCRIPTION	CTriggerChangeValue::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerChangeValue, m_iszNewValue, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeValue,CBaseDelay);

void CTriggerChangeValue::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iszNewValue"))
	{
		m_iszNewValue = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}

void CTriggerChangeValue::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
//	CBaseEntity *pTarget = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ), pActivator );

	CBaseEntity *pTarget = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );

	if (pTarget)
	{
		KeyValueData mypkvd;
		mypkvd.szKeyName = (char*)STRING(pev->netname);
		mypkvd.szValue = (char*)STRING(m_iszNewValue);
		mypkvd.fHandled = FALSE;
		pTarget->KeyValue(&mypkvd);
		//Error if not handled?
	}
}

//=====================================================
// trigger_command: activate a console command
//=====================================================
class CTriggerCommand : public CBaseEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};
LINK_ENTITY_TO_CLASS( trigger_command, CTriggerCommand );

void CTriggerCommand::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	char szCommand[256];

	if (pev->netname)
	{
		sprintf( szCommand, "%s\n", STRING(pev->netname) );
		SERVER_COMMAND( szCommand );
	}
}

//=====================================================
// trigger_command: activate a console command
//=====================================================
//"trigger_changebrief_info" same as "trigger_command" except you can specify the choices

class CTriggerCommandBrief : public CBaseEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};
LINK_ENTITY_TO_CLASS( trigger_changebrief_info, CTriggerCommandBrief );

void CTriggerCommandBrief::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	char szCommand[256];

	if (pev->netname)
	{
		sprintf( szCommand, "%s\n", STRING(pev->netname) );
		SERVER_COMMAND( szCommand );
		
		//fix: para no activarlo despues con algo. Se cambia el txt y se envia el cuadro al instante
		SERVER_COMMAND("brief\n");
		//error: despues de activar algun comando de consola se envia el mensaje "reporte de mision ..." 
		//crear nueva clase
	}
}

#define SF_CAMERA_PLAYER_POSITION	1
#define SF_CAMERA_PLAYER_TARGET		2
#define SF_CAMERA_PLAYER_TAKECONTROL 4

class CTriggerCamera : public CBaseDelay
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT FollowTarget( void );
	void Move(void);

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;
	CBaseEntity *m_pentPath;
	int	  m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	int	  m_state;
	
};
LINK_ENTITY_TO_CLASS( trigger_camera, CTriggerCamera );

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION	CTriggerCamera::m_SaveData[] = 
{
	DEFINE_FIELD( CTriggerCamera, m_hPlayer, FIELD_EHANDLE ),
	DEFINE_FIELD( CTriggerCamera, m_hTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( CTriggerCamera, m_pentPath, FIELD_CLASSPTR ),
	DEFINE_FIELD( CTriggerCamera, m_sPath, FIELD_STRING ),
	DEFINE_FIELD( CTriggerCamera, m_flWait, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerCamera, m_flReturnTime, FIELD_TIME ),
	DEFINE_FIELD( CTriggerCamera, m_flStopTime, FIELD_TIME ),
	DEFINE_FIELD( CTriggerCamera, m_moveDistance, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerCamera, m_targetSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerCamera, m_initialSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerCamera, m_acceleration, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerCamera, m_deceleration, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerCamera, m_state, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CTriggerCamera,CBaseDelay);

void CTriggerCamera::Spawn( void )
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;							// Remove model & collisions
	pev->renderamt = 0;								// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;

	m_initialSpeed = pev->speed;
	if ( m_acceleration == 0 )
		m_acceleration = 500;
	if ( m_deceleration == 0 )
		m_deceleration = 500;
}


void CTriggerCamera :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}



void CTriggerCamera::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, m_state ) )
		return;

	// Toggle state
	m_state = !m_state;
	if (m_state == 0)
	{
//
//	R2D2RIGO's IDEA
//
//		if (pPlayer->m_fCineOn)
//		{
//			pPlayer->CineToggle(FALSE);
//		}
//
//	R2D2RIGO's IDEA
//
		m_flReturnTime = gpGlobals->time;
		return;
	}
	if ( !pActivator || !pActivator->IsPlayer() )
	{
		pActivator = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex( 1 ));
	}
		
	m_hPlayer = pActivator;

	m_flReturnTime = gpGlobals->time + m_flWait;
	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;


//	R2D2RIGO's IDEA

    //CLIENT_COMMAND(pActivator->edict(), "cinebars\n");

//	R2D2RIGO's IDEA


	if (FBitSet (pev->spawnflags, SF_CAMERA_PLAYER_TARGET ) )
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	// Nothing to look at!
	if ( m_hTarget == NULL )
	{
		return;
	}


	if (FBitSet (pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL ) )
	{
		((CBasePlayer *)pActivator)->EnableControl(FALSE);
	}

	if ( m_sPath )
	{
		m_pentPath = Instance( FIND_ENTITY_BY_TARGETNAME ( NULL, STRING(m_sPath)) );
	}
	else
	{
		m_pentPath = NULL;
	}

	m_flStopTime = gpGlobals->time;
	if ( m_pentPath )
	{
		if ( m_pentPath->pev->speed != 0 )
			m_targetSpeed = m_pentPath->pev->speed;
		
		m_flStopTime += m_pentPath->GetDelay();
	}

	// copy over player information
	if (FBitSet (pev->spawnflags, SF_CAMERA_PLAYER_POSITION ) )
	{
		UTIL_SetOrigin( pev, pActivator->pev->origin + pActivator->pev->view_ofs );
		pev->angles.x = -pActivator->pev->angles.x;
		pev->angles.y = pActivator->pev->angles.y;
		pev->angles.z = 0;
		pev->velocity = pActivator->pev->velocity;
	}
	else
	{
		pev->velocity = Vector( 0, 0, 0 );
	}

	SET_VIEW( pActivator->edict(), edict() );

	SET_MODEL(ENT(pev), STRING(pActivator->pev->model) );

	// follow the player down
	SetThink( FollowTarget );
	pev->nextthink = gpGlobals->time;

	m_moveDistance = 0;
	Move();
}


void CTriggerCamera::FollowTarget( )
{
	if (m_hPlayer == NULL)
		return;

	if (m_hTarget == NULL || m_flReturnTime < gpGlobals->time)
	{
		if (m_hPlayer->IsAlive( ))
		{
			SET_VIEW( m_hPlayer->edict(), m_hPlayer->edict() );
			((CBasePlayer *)((CBaseEntity *)m_hPlayer))->EnableControl(TRUE);
		}
		SUB_UseTargets( this, USE_TOGGLE, 0 );
		pev->avelocity = Vector( 0, 0, 0 );
		m_state = 0;
		return;
	}

	Vector vecGoal = UTIL_VecToAngles( m_hTarget->pev->origin - pev->origin );
	vecGoal.x = -vecGoal.x;

	if (pev->angles.y > 360)
		pev->angles.y -= 360;

	if (pev->angles.y < 0)
		pev->angles.y += 360;

	float dx = vecGoal.x - pev->angles.x;
	float dy = vecGoal.y - pev->angles.y;

	if (dx < -180) 
		dx += 360;
	if (dx > 180) 
		dx = dx - 360;
	
	if (dy < -180) 
		dy += 360;
	if (dy > 180) 
		dy = dy - 360;

	pev->avelocity.x = dx * 40 * gpGlobals->frametime;
	pev->avelocity.y = dy * 40 * gpGlobals->frametime;


	if (!(FBitSet (pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))	
	{
		pev->velocity = pev->velocity * 0.8;
		if (pev->velocity.Length( ) < 10.0)
			pev->velocity = g_vecZero;
	}

	pev->nextthink = gpGlobals->time;

	Move();
}

void CTriggerCamera::Move()
{
	// Not moving on a path, return
	if (!m_pentPath)
		return;

	// Subtract movement from the previous frame
	m_moveDistance -= pev->speed * gpGlobals->frametime;

	// Have we moved enough to reach the target?
	if ( m_moveDistance <= 0 )
	{
		// Fire the passtarget if there is one
		if ( m_pentPath->pev->message )
		{
			FireTargets( STRING(m_pentPath->pev->message), this, this, USE_TOGGLE, 0 );
			if ( FBitSet( m_pentPath->pev->spawnflags, SF_CORNER_FIREONCE ) )
				m_pentPath->pev->message = 0;
		}
		// Time to go to the next target
		m_pentPath = m_pentPath->GetNextTarget();

		// Set up next corner
		if ( !m_pentPath )
		{
			pev->velocity = g_vecZero;
		}
		else 
		{
			if ( m_pentPath->pev->speed != 0 )
				m_targetSpeed = m_pentPath->pev->speed;

			Vector delta = m_pentPath->pev->origin - pev->origin;
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + m_pentPath->GetDelay();
		}
	}

	if ( m_flStopTime > gpGlobals->time )
		pev->speed = UTIL_Approach( 0, pev->speed, m_deceleration * gpGlobals->frametime );
	else
		pev->speed = UTIL_Approach( m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime );

	float fraction = 2 * gpGlobals->frametime;
	pev->velocity = ((pev->movedir * pev->speed) * fraction) + (pev->velocity * (1-fraction));
}


class CTargetMenu : public CPointEntity
{
public:
     void Spawn( void );

     void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( trigger_weapon_menu, CTargetMenu );
LINK_ENTITY_TO_CLASS( trigger_displaymenu, CTargetMenu );

void CTargetMenu :: Spawn( void )
{
 //    pev->solid = SOLID_NOT;
 //    pev->movetype = MOVETYPE_NONE;
}

void CTargetMenu::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
     if (!pActivator->IsPlayer()) // activator should be a player
          return;

     CLIENT_COMMAND(pActivator->edict(), "wepselect\n");
}

class CXPPoits : public CPointEntity
{
public:
     void Spawn( void );

     void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( trigger_xppoints, CXPPoits );

void CXPPoits :: Spawn( void )
{
    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_NONE;
}

void CXPPoits::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
//	if (!pActivator->IsPlayer()) // activator should be a player
//		return;
		
	int iXPPoints = CVAR_GET_FLOAT( "xp_points" );

	int iResult = iXPPoints += pev->frags;

//iXPPoints 5410 + -1700 = 3712
	if (iResult > 0)//if we have xp points for that
	{
		UTIL_ShowMessageAll( STRING(pev->noise1) );	//have points
		FireTargets( STRING(pev->target), this, this, USE_TOGGLE, 0 );
		
		//take out XP points
		if (iXPPoints <= 0)//just in case
		CVAR_SET_FLOAT( "xp_points", 0);
		else	
		CVAR_SET_FLOAT( "xp_points", ( iXPPoints += pev->frags) );
	}
	else
	{
		UTIL_ShowMessageAll( STRING(pev->noise2) );	//dont have points
		FireTargets( STRING(pev->enemy), this, this, USE_TOGGLE, 0 );
	}

}


class CCvarCheck : public CPointEntity
{
public:
     void Spawn( void );

     void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( trigger_check_cvar, CCvarCheck );

void CCvarCheck :: Spawn( void )
{
    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_NONE;
}

void CCvarCheck::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
//	if (!pActivator->IsPlayer()) // activator should be a player
//		return;

	char szFragsCommand[256];
//	char szNoiseNewCommand[256];
	int iValueToCheck;
	int iCvar;

	sprintf( szFragsCommand, "%s\n", STRING(pev->netname) );

//	sprintf( szNoiseNewCommand, "%i\n", pev->frags );

//	if ( pev->frags )
//	SERVER_COMMAND(szNoiseNewCommand);//listo :)

	iCvar = CVAR_GET_FLOAT( STRING(pev->netname) );//esto buscará el valor de "ricardo"
	iValueToCheck = pev->speed;

	if ( iValueToCheck == iCvar)
	{	
		UTIL_ShowMessageAll( STRING(pev->noise1) );//mostrar el mensaje de correcto
		FireTargets( STRING(pev->target), this, this, USE_TOGGLE, 0 );//activar subtargets
		ALERT ( at_aiconsole, "Command: %s, Value to check: %i, Actual Value: %i,Result: TRUE, Forr0 is Gay: TRUE\n", STRING(pev->netname),iValueToCheck,iCvar);
	}
	else
	{	
		UTIL_ShowMessageAll( STRING(pev->noise2) );//mostrar el mensaje de error
		FireTargets( STRING(pev->message), this, this, USE_TOGGLE, 0 );//activar subtargets
		ALERT ( at_aiconsole, "Command: %s, Value to check: %i, Actual Value: %i,Result: FALSE, SysOp is Gay: FALSE\n", STRING(pev->netname),iValueToCheck,iCvar);
	}
}

//===========================================================
//LRC- trigger_onsight
//===========================================================
#define SF_ONSIGHT_NOLOS		0x00001
#define SF_ONSIGHT_NOGLASS		0x00002
#define SF_ONSIGHT_STATECHECK	0x00004 //AJH
#define SF_ONSIGHT_ACTIVE		0x08000
#define SF_ONSIGHT_DEMAND		0x10000

class CTriggerOnSight : public CBaseDelay
{
public:
	void Spawn( void );
	void Think( void );
	BOOL VisionCheck( void );
	BOOL CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen);
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	STATE GetState();
};

LINK_ENTITY_TO_CLASS( trigger_onsight, CTriggerOnSight );

void CTriggerOnSight :: Spawn( void )
{
	if (pev->target || pev->noise)
		// if we're going to have to trigger stuff, start thinking
		pev->nextthink = 1;
	else
		// otherwise, just check whenever someone asks about our state.
		pev->spawnflags |= SF_ONSIGHT_DEMAND;

	if (pev->max_health > 0)
	{
		pev->health = cos(pev->max_health/2 * M_PI/180.0);
//		ALERT(at_debug, "Cosine is %f\n", pev->health);
	}
}

STATE CTriggerOnSight :: GetState( void )
{
	if (pev->spawnflags & SF_ONSIGHT_DEMAND)
		return VisionCheck()?STATE_ON:STATE_OFF;
	else
		return (pev->spawnflags & SF_ONSIGHT_ACTIVE)?STATE_ON:STATE_OFF;
}

void CTriggerOnSight :: Think( void )
{
	// is this a sensible rate?
	pev->nextthink = 0.1;
//	if (!UTIL_IsMasterTriggered(m_sMaster, NULL))
//	{
//		pev->spawnflags &= ~SF_ONSIGHT_ACTIVE;
//		return;
//	}

	if (VisionCheck())
	{
		if (!FBitSet(pev->spawnflags, SF_ONSIGHT_ACTIVE))
		{
			FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
			FireTargets(STRING(pev->noise1), this, this, USE_ON, 0);
			pev->spawnflags |= SF_ONSIGHT_ACTIVE;
		}
	}
	else
	{
		if (pev->spawnflags & SF_ONSIGHT_ACTIVE)
		{
			FireTargets(STRING(pev->noise), this, this, USE_TOGGLE, 0);
			FireTargets(STRING(pev->noise1), this, this, USE_OFF, 0);
			pev->spawnflags &= ~SF_ONSIGHT_ACTIVE;
		}
	}
}

BOOL CTriggerOnSight :: VisionCheck( void )			//AJH modifed to check if multiple entities can see
{													// and GetState check (stops dead monsters seeing)
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
			ALERT(at_error, "trigger_onsight can't find player!?\n");
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

/*BOOL CTriggerOnSight :: VisionCheck( void )
{
	CBaseEntity *pLooker;
	if (pev->netname)
	{
		pLooker = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
		if (!pLooker)
			return FALSE; // if we can't find the eye entity, give up
	}
	else
	{
		pLooker = UTIL_FindEntityByClassname(NULL, "player");
		if (!pLooker)
		{
			ALERT(at_error, "trigger_onsight can't find player!?\n");
			return FALSE;
		}
	}

	CBaseEntity *pSeen;
	if (pev->message)
		pSeen = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
	else
		return CanSee(pLooker, this);

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
		return FALSE;
	}
	else
	{
		while (pSeen != NULL)
		{
			if (CanSee(pLooker, pSeen))
				return TRUE;
			pSeen = UTIL_FindEntityByTargetname(pSeen, STRING(pev->message));
		}
		return FALSE;
	}
}*/

// by the criteria we're using, can the Looker see the Seen entity?
BOOL CTriggerOnSight :: CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen)
{
	// out of range?
	if (pev->frags && (pLooker->pev->origin - pSeen->pev->origin).Length() > pev->frags)
		return FALSE;

	// check FOV if appropriate
	if (pev->max_health < 360)
	{
		// copied from CBaseMonster's FInViewCone function
		Vector2D	vec2LOS;
		float	flDot;
		float flComp = pev->health;
		UTIL_MakeVectors ( pLooker->pev->angles );
		vec2LOS = ( pSeen->pev->origin - pLooker->pev->origin ).Make2D();
		vec2LOS = vec2LOS.Normalize();
		flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

//		ALERT(at_debug, "flDot is %f\n", flDot);

		if ( pev->max_health == -1 )
		{
			CBaseMonster *pMonst = pLooker->MyMonsterPointer();
			if (pMonst)
				flComp = pMonst->m_flFieldOfView;
			else
				return FALSE; // not a monster, can't use M-M-M-MonsterVision
		}

		// outside field of view
		if (flDot <= flComp)
			return FALSE;
	}

	// check LOS if appropriate
	if (!FBitSet(pev->spawnflags, SF_ONSIGHT_NOLOS))
	{
		TraceResult tr;
		if (SF_ONSIGHT_NOGLASS)
			UTIL_TraceLine( pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, ignore_glass, pLooker->edict(), &tr );
		else
			UTIL_TraceLine( pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, dont_ignore_glass, pLooker->edict(), &tr );
		if (tr.flFraction < 1.0 && tr.pHit != pSeen->edict())
			return FALSE;
	}

	return TRUE;
}
//************
class CFuncWhatever : public CBaseTrigger
{
public:
void Spawn( void );
void EXPORT Touch( CBaseEntity *pOther );
};

LINK_ENTITY_TO_CLASS( func_semtex_zone, CFuncWhatever );

void CFuncWhatever :: Spawn( void )
{
	InitTrigger();
	SetTouch(&CFuncWhatever::Touch );
}

void CFuncWhatever :: Touch (CBaseEntity *pOther)
{
	ALERT (at_console, "Touching func_semtex_zone!!\n");

	if ( !pOther->IsPlayer() )
	{
		return;
	}

	// spoit sound here

	pOther->b_InFuncSemtexZone = TRUE;	
}
//************
class CFuncSecretZone : public CBaseTrigger
{
public:
void Spawn( void );
void EXPORT Touch( CBaseEntity *pOther );
};

LINK_ENTITY_TO_CLASS( func_secret_zone, CFuncSecretZone );

void CFuncSecretZone :: Spawn( void )
{
	InitTrigger();
	SetTouch(&CFuncSecretZone::Touch );
}

void CFuncSecretZone :: Touch (CBaseEntity *pOther)
{	
	if (!pOther->IsPlayer()) // activator should be a player
	return;

	ClientPrint(pOther->pev, HUD_PRINTCENTER, "#SecretPlace" );

	EMIT_SOUND(ENT(pev), CHAN_VOICE, "fvox/cheater2.wav", 1, ATTN_NORM);

	return;//message sended!
}

//===========================================================
//trigger_sunflare
//===========================================================
#define SF_FADESCREEN		8
#define SF_CHANGECOLORS		16
#define SF_CHEAPSUNFLARE	32//new version that uses less sprites

class CTriggerSunflare : public CBaseDelay
{
public:
	void Spawn( void );
	void Think( void );
	BOOL VisionCheck( void );
	BOOL CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen);

	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Precache( void );

	CSprite *pSprite1;
	CSprite *pSprite2;
	CSprite *pSprite3;
	CSprite *pSprite4;
	CSprite *pSprite5;

	float LightVariation;

	float RedValue;
	float GreenValue;
	float BlueValue;
};

LINK_ENTITY_TO_CLASS( trigger_sunflare, CTriggerSunflare );
LINK_ENTITY_TO_CLASS( env_sunflare, CTriggerSunflare );

TYPEDESCRIPTION	CTriggerSunflare::m_SaveData[] = 
{
	DEFINE_FIELD( CTriggerSunflare, LightVariation, FIELD_FLOAT ),

	DEFINE_FIELD( CTriggerSunflare, RedValue, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerSunflare, GreenValue, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerSunflare, BlueValue, FIELD_FLOAT )
};

IMPLEMENT_SAVERESTORE( CTriggerSunflare, CBaseDelay );

void CTriggerSunflare :: Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;

	Precache();

	//this is needed
	SET_MODEL(ENT(pev), "sprites/null.spr");
		
	LightVariation = pev->renderamt;
	
	if ( FStringNull( pev->renderamt ))
	LightVariation = 150;

	RedValue = pev->rendercolor.x;//R
	GreenValue = pev->rendercolor.y;//G
	BlueValue = pev->rendercolor.z;//B //its must be 200
	
	pev->nextthink = gpGlobals->time + 0.1;
}

void CTriggerSunflare::Precache( void )
{
	PRECACHE_MODEL( "sprites/sunflare.spr" );
	PRECACHE_MODEL( "sprites/null.spr" );
	
	//spawn here
	
	if ( !(pev->spawnflags & SF_CHEAPSUNFLARE) )
	{
		pSprite1 = CSprite::SpriteCreate( "sprites/sunflare.spr", pev->origin, FALSE );				
		pSprite2 = CSprite::SpriteCreate( "sprites/sunflare.spr", pev->origin, FALSE );				
		pSprite3 = CSprite::SpriteCreate( "sprites/sunflare.spr", pev->origin, FALSE );	
	}

	pSprite4 = CSprite::SpriteCreate( "sprites/sunflare.spr", pev->origin, FALSE );				
	pSprite5 = CSprite::SpriteCreate( "sprites/sunflare.spr", pev->origin, FALSE );	

	ALERT ( at_console, "Creating Sprites...\n");
}
void CTriggerSunflare :: Think( void )
{
	if (VisionCheck())
	{
		if ( !(pev->spawnflags & SF_CHEAPSUNFLARE) )
		{
			if (pSprite1->pev->scale <= (10.0 + pev->scale) )//scale fgd entry
				pSprite1->pev->scale += 0.1;

			if (pSprite2->pev->scale <= (8.0 + pev->scale) )
				pSprite2->pev->scale += 0.1;

			if (pSprite3->pev->scale <= (6.0 + pev->scale) )
				pSprite3->pev->scale += 0.1;
		}
		if (pSprite4->pev->scale <= (4.0 + pev->scale) )
			pSprite4->pev->scale += 0.1;

		if (pSprite5->pev->scale <= (2.0 + pev->scale) )
			pSprite5->pev->scale += 0.1;
		
		if (pev->spawnflags & SF_FADESCREEN)
		{
			CBaseEntity *pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, 9999 )) != NULL) 
			{
			  if ( pEntity->IsPlayer() ) 
			  {
				UTIL_ScreenFade( pEntity, Vector(155,155,155), 0.5, 1, 111, FFADE_IN );
			  }
			}
		}
	}
	else
	{
		if ( !(pev->spawnflags & SF_CHEAPSUNFLARE) )
		{
			if (pSprite1->pev->scale >= (0.6 + pev->scale) )
				pSprite1->pev->scale -= 0.1;
				
			if (pSprite2->pev->scale >= (0.5 + pev->scale) )
				pSprite2->pev->scale -= 0.1;

			if (pSprite3->pev->scale >= (0.4 + pev->scale) )
				pSprite3->pev->scale -= 0.1;
		}

		if (pSprite4->pev->scale >= (0.3 + pev->scale) )
			pSprite4->pev->scale -= 0.1;

		if (pSprite5->pev->scale >= (0.2 + pev->scale) )// 0.1 - 0.1 ?
			pSprite5->pev->scale -= 0.1;
	}

	//Sys: change those values only by 1 or -1 per time
	if (pev->spawnflags & SF_CHANGECOLORS)
	BlueValue += RANDOM_FLOAT( -1, 1 );

	LightVariation += RANDOM_FLOAT( -1, 1 );

	//Color FIX
	if (BlueValue >= 255)
		BlueValue = 255;

	if (BlueValue <= 0)
		BlueValue = 1;

	//Light FIX
	if (LightVariation >= 255)
		LightVariation = 255;
	
	if (LightVariation <= 0)
		LightVariation = 1;
	
	if ( !(pev->spawnflags & SF_CHEAPSUNFLARE) )
	{
		pSprite1->SetTransparency( kRenderGlow, RedValue, GreenValue, BlueValue, LightVariation, kRenderFxNoDissipation );
		pSprite2->SetTransparency( kRenderGlow, RedValue, GreenValue, BlueValue, LightVariation, kRenderFxNoDissipation );
		pSprite3->SetTransparency( kRenderGlow, RedValue, GreenValue, BlueValue, LightVariation, kRenderFxNoDissipation );
	}

	pSprite4->SetTransparency( kRenderGlow, RedValue, GreenValue, BlueValue, LightVariation, kRenderFxNoDissipation );
	pSprite5->SetTransparency( kRenderGlow, RedValue, GreenValue, BlueValue, LightVariation, kRenderFxNoDissipation );

	pev->nextthink = 0.3;
}

BOOL CTriggerSunflare :: VisionCheck( void )			//AJH modifed to check if multiple entities can see
{													// and GetState check (stops dead monsters seeing)
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
			ALERT(at_error, "trigger_onsight can't find player!?\n");
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
// by the criteria we're using, can the Looker see the Seen entity?
BOOL CTriggerSunflare :: CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen)
{
	// out of range?
	if (pev->frags && (pLooker->pev->origin - pSeen->pev->origin).Length() > pev->frags)
		return FALSE;

	// check FOV if appropriate
	if (pev->max_health < 360)
	{
		// copied from CBaseMonster's FInViewCone function
		Vector2D	vec2LOS;
		float	flDot;
		float flComp = pev->health;
		UTIL_MakeVectors ( pLooker->pev->angles );
		vec2LOS = ( pSeen->pev->origin - pLooker->pev->origin ).Make2D();
		vec2LOS = vec2LOS.Normalize();
		flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

//		ALERT(at_debug, "flDot is %f\n", flDot);

		if ( pev->max_health == -1 )
		{
			CBaseMonster *pMonst = pLooker->MyMonsterPointer();
			if (pMonst)
				flComp = pMonst->m_flFieldOfView;
			else
				return FALSE; // not a monster, can't use M-M-M-MonsterVision
		}

		// outside field of view
		if (flDot <= flComp)
			return FALSE;
	}
/*
	// check LOS if appropriate
	if (!FBitSet(pev->spawnflags, SF_ONSIGHT_NOLOS))
	{
		TraceResult tr;
		if (SF_ONSIGHT_NOGLASS)
			UTIL_TraceLine( pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, ignore_glass, pLooker->edict(), &tr );
		else
			UTIL_TraceLine( pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, dont_ignore_glass, pLooker->edict(), &tr );
	
		if (tr.flFraction < 1.0 && tr.pHit != pSeen->edict())
			return FALSE;
	}
*/
	TraceResult tr;

	UTIL_TraceLine( pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, dont_ignore_glass, pLooker->edict(), &tr );
	
	if (tr.flFraction < 1.0 && tr.pHit != pSeen->edict())
		return FALSE;

	return TRUE;
}
//************


//===========================================================
//Sys0p- env_sprite_grass
//===========================================================
#define SF_DONTUSELOD			8
#define SF_USELOD_HIGH			16
#define SF_USELOD_LOW			32

#define SF_LOWLIGHT			128
#define SF_SNOW				256

#define SF_DEBUG_MODE		1024

class CEnvGrass : public CBaseDelay
{
public:
	void Spawn( void );
	void Think( void );
	BOOL VisionCheck( void );
	BOOL CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen);
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void Precache( void );
};

LINK_ENTITY_TO_CLASS( env_sprite_grass, CEnvGrass );

void CEnvGrass :: Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	//this is needed?
	if (pev->max_health > 0)
	{
		pev->health = cos(pev->max_health/2 * M_PI/180.0);
	}

	Precache();

	switch (RANDOM_LONG(0,3))
	{
		case 0:	pev->scale			= 0.5; pev->origin.z += 25; break;
		case 1:	pev->scale			= 0.4; pev->origin.z += 20;break;
		case 2:	pev->scale			= 0.3; pev->origin.z += 10;break;
		case 3:	pev->scale			= 0.2; pev->origin.z += 5;break;
	}

	UTIL_SetOrigin(pev, pev->origin);
/*
	if ( FBitSet ( pev->spawnflags, SF_LOWLIGHT ))
	SET_MODEL(ENT(pev), "sprites/grass_black.spr");
	else if ( FBitSet ( pev->spawnflags, SF_SNOW ))
	SET_MODEL(ENT(pev), "sprites/grass_snow.spr");
	else
	SET_MODEL(ENT(pev), "sprites/grass_normal.spr");
*/
	if (pev->model)
	SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
	SET_MODEL(ENT(pev), "sprites/grass_normal.spr");

/*		
	if ( RANDOM_FLOAT ( 0, 1 ) < 0.75 )
	pev->angles.y += RANDOM_FLOAT( 0, 180 );
	else
	pev->angles.y -= RANDOM_FLOAT( 0, 180 );
*/
	/*
	if ( FStringNull( pev->frags ))
	{
		switch (RANDOM_LONG(0,4))
		{
			case 0:	SET_MODEL(ENT(pev), "sprites/ikgrass1.spr"); break;
			case 1:	SET_MODEL(ENT(pev), "sprites/ikgrass2.spr"); break;
			case 2:	SET_MODEL(ENT(pev), "sprites/ikgrass3.spr"); break;
			case 3:	SET_MODEL(ENT(pev), "sprites/ikgrass4.spr"); break;
			case 4:	SET_MODEL(ENT(pev), "sprites/ikgrass5.spr"); break;
		}
	}
	*/

	pev->effects &= ~EF_NODRAW;//hide it by default (unless the player is near)

	pev->rendermode = kRenderTransTexture;//this is needed?
	pev->renderamt = 0;//dont draw

	pev->nextthink = 1;
}

void CEnvGrass::Precache( void )
{
/*
	PRECACHE_MODEL( "sprites/grass_black.spr" );
	PRECACHE_MODEL( "sprites/grass_snow.spr" );
//	PRECACHE_MODEL( "models/grass_normal.mdl" );
*/
	if (pev->model)
	PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
	PRECACHE_MODEL( "sprites/grass_normal.spr" );
}
void CEnvGrass :: Think( void )
{
	CBaseEntity *pEntity = NULL;
		
	if (VisionCheck())
	{
		pev->effects &= ~EF_NODRAW;

//		if ( FBitSet ( pev->spawnflags, SF_DEBUG_MODE ))
//		ALERT ( at_console, "VisionCheck OK, pev->effects &= ~EF_NODRAW\n" );
	}
	else
	{
		pev->effects |= EF_NODRAW;

//		if ( FBitSet ( pev->spawnflags, SF_DEBUG_MODE ))
//		ALERT ( at_console, "VisionCheck FAILED, pev->effects |= EF_NODRAW\n" );
	}

//PERFORMANCE FIX: This func it's too slow! Let's check only if we can see the player
//	while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, 9999 )) != NULL) 
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL) 
	{
		if ( pEntity->IsPlayer() ) 
		{
			float flDist = (pEntity->Center() - pev->origin).Length();

			if ( flDist >= 900)
			{
				if ( FBitSet ( pev->spawnflags, SF_DEBUG_MODE ))
				ALERT ( at_console, "Entity is too far!, pev->effects |= EF_NODRAW\n" );

				pev->effects |= EF_NODRAW;//no dibujar si está lejos
			}

			if (!(pev->spawnflags & SF_DONTUSELOD))//si no tiene NO usar lod = Usa LOD
			{
				if (pev->spawnflags & SF_USELOD_HIGH)//demasiado LOD
				{
					if ( flDist >= 300)
					pev->effects |= EF_NODRAW;
				
					if (flDist >= 300)//más rapido
						pev->renderamt = 0;

					else if (flDist >= 295)
							pev->renderamt = 10;
					else if (flDist >= 290)
							pev->renderamt = 20;
					else if (flDist >= 285)
							pev->renderamt = 30;
					else if (flDist >= 280)
							pev->renderamt = 40;
					else if (flDist >= 275)
							pev->renderamt = 50;
					else if (flDist >= 270)
							pev->renderamt = 60;
					else if (flDist >= 265)
							pev->renderamt = 70;
					else if (flDist >= 260)
							pev->renderamt = 80;
					else if (flDist >= 255)
							pev->renderamt = 90;
					else if (flDist >= 250)
							pev->renderamt = 100;
					else if (flDist >= 245)
							pev->renderamt = 110;
					else if (flDist >= 240)
							pev->renderamt = 120;
					else if (flDist >= 235)
							pev->renderamt = 130;
					else if (flDist >= 230)
							pev->renderamt = 140;
					else if (flDist >= 225)
							pev->renderamt = 150;
					else if (flDist >= 220)
							pev->renderamt = 160;
					else if (flDist >= 215)
							pev->renderamt = 170;
					else if (flDist >= 210)
							pev->renderamt = 180;
					else if (flDist >= 200)
							pev->renderamt = 190;
					else if (flDist >= 195)
							pev->renderamt = 200;
					else if (flDist >= 190)
							pev->renderamt = 210;
					else if (flDist >= 185)
							pev->renderamt = 220;
					else if (flDist >= 180)
							pev->renderamt = 230;
					else if (flDist >= 175)
							pev->renderamt = 240;
					else
							pev->renderamt = 255;
				}
				else if (pev->spawnflags & SF_USELOD_LOW)//poco LOD
				{
					if ( flDist >= 900)
					pev->effects |= EF_NODRAW;
				
					if (flDist >= 900)
						pev->renderamt = 0;

					else if (flDist >= 890)
							pev->renderamt = 10;
					else if (flDist >= 880)
							pev->renderamt = 20;
					else if (flDist >= 870)
							pev->renderamt = 30;
					else if (flDist >= 860)
							pev->renderamt = 40;
					else if (flDist >= 850)
							pev->renderamt = 50;
					else if (flDist >= 840)
							pev->renderamt = 60;
					else if (flDist >= 830)
							pev->renderamt = 70;
					else if (flDist >= 820)
							pev->renderamt = 80;
					else if (flDist >= 810)
							pev->renderamt = 90;
					else if (flDist >= 800)
							pev->renderamt = 100;
					else if (flDist >= 790)
							pev->renderamt = 110;
					else if (flDist >= 780)
							pev->renderamt = 120;
					else if (flDist >= 770)
							pev->renderamt = 130;
					else if (flDist >= 760)
							pev->renderamt = 140;
					else if (flDist >= 750)
							pev->renderamt = 150;
					else if (flDist >= 740)
							pev->renderamt = 160;
					else if (flDist >= 730)
							pev->renderamt = 170;
					else if (flDist >= 720)
							pev->renderamt = 180;
					else if (flDist >= 710)
							pev->renderamt = 190;
					else if (flDist >= 700)
							pev->renderamt = 200;
					else if (flDist >= 690)
							pev->renderamt = 210;
					else if (flDist >= 680)
							pev->renderamt = 220;
					else if (flDist >= 670)
							pev->renderamt = 230;
					else if (flDist >= 660)
							pev->renderamt = 240;
					else
							pev->renderamt = 255;
				}
				else//normal LOD
				{
					if ( flDist >= 500)//900 it's too larger... using 500
					pev->effects |= EF_NODRAW;
				
					if (flDist >= 500)
						pev->renderamt = 0;

					else if (flDist >= 490)
							pev->renderamt = 10;
					else if (flDist >= 480)
							pev->renderamt = 20;
					else if (flDist >= 470)
							pev->renderamt = 30;
					else if (flDist >= 460)
							pev->renderamt = 40;
					else if (flDist >= 450)
							pev->renderamt = 50;
					else if (flDist >= 440)
							pev->renderamt = 60;
					else if (flDist >= 430)
							pev->renderamt = 70;
					else if (flDist >= 420)
							pev->renderamt = 80;
					else if (flDist >= 410)
							pev->renderamt = 90;
					else if (flDist >= 400)
							pev->renderamt = 100;
					else if (flDist >= 390)
							pev->renderamt = 110;
					else if (flDist >= 380)
							pev->renderamt = 120;
					else if (flDist >= 370)
							pev->renderamt = 130;
					else if (flDist >= 360)
							pev->renderamt = 140;
					else if (flDist >= 350)
							pev->renderamt = 150;
					else if (flDist >= 340)
							pev->renderamt = 160;
					else if (flDist >= 330)
							pev->renderamt = 170;
					else if (flDist >= 320)
							pev->renderamt = 180;
					else if (flDist >= 310)
							pev->renderamt = 190;
					else if (flDist >= 300)
							pev->renderamt = 200;
					else if (flDist >= 290)
							pev->renderamt = 210;
					else if (flDist >= 280)
							pev->renderamt = 220;
					else if (flDist >= 270)
							pev->renderamt = 230;
					else if (flDist >= 260)
							pev->renderamt = 240;
					else
							pev->renderamt = 255;
				}

			}

			if ( FBitSet ( pev->spawnflags, SF_DEBUG_MODE ))
			{			
				ALERT(at_console, "renderamt is %f\n", pev->renderamt);

				if ( flDist >= 900)
				ALERT(at_console, "Entity: env_grass Is now HIDDEN\n");
				else
				ALERT(at_console, "Entity: env_grass Is now VISIBLE\n");
			}
		}
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

BOOL CEnvGrass :: VisionCheck( void )
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
BOOL CEnvGrass :: CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen)
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

	/*
	TraceResult tr;
	UTIL_TraceLine( pLooker->EyePosition(), pSeen->pev->origin, ignore_monsters, dont_ignore_glass, pLooker->edict(), &tr );
	
	if (tr.flFraction < 1.0 && tr.pHit != pSeen->edict())
		return FALSE;
*/
	return TRUE;
}
//************


//******************

class CTriggerRandom : public CBaseEntity
{
public:
	void Spawn( void );
	void Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void KeyValue( KeyValueData *pkvd );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	int		m_iszTarget1;
	int		m_iszTarget2;
	int		m_iszTarget3;
	int		m_iszTarget4;
};

LINK_ENTITY_TO_CLASS( trigger_random, CTriggerRandom );

TYPEDESCRIPTION	CTriggerRandom::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerRandom, m_iszTarget1, FIELD_STRING),	
	DEFINE_FIELD( CTriggerRandom, m_iszTarget2, FIELD_STRING),
	DEFINE_FIELD( CTriggerRandom, m_iszTarget3, FIELD_STRING),
	DEFINE_FIELD( CTriggerRandom, m_iszTarget4, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerRandom,CBaseEntity);

void CTriggerRandom :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iszTarget1"))
	{
		m_iszTarget1 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTarget2"))
	{
		m_iszTarget2 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTarget3"))
	{
		m_iszTarget3 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTarget4"))
	{
		m_iszTarget4 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CTriggerRandom :: Spawn ( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype	= MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;
}

void CTriggerRandom :: Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	switch (RANDOM_LONG(0,3))
	{
		case 0: FireTargets( STRING(m_iszTarget1), this, this, USE_ON, 0 ); 	
			ALERT( at_console, "Firing entity \"%s\"\n", STRING(m_iszTarget1) );
			break;
		case 1: FireTargets( STRING(m_iszTarget2), this, this, USE_ON, 0 ); 	
			ALERT( at_console, "Firing entity \"%s\"\n", STRING(m_iszTarget2) );
			break;
		case 2: FireTargets( STRING(m_iszTarget3), this, this, USE_ON, 0 ); 	
			ALERT( at_console, "Firing entity \"%s\"\n", STRING(m_iszTarget3) );
			break;
		case 3: FireTargets( STRING(m_iszTarget4), this, this, USE_ON, 0 ); 	
			ALERT( at_console, "Firing entity \"%s\"\n", STRING(m_iszTarget4) );
			break;	
	}	
}
