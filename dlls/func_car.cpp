/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// func_car. driving vehicle, knows traffic rules and reacts to environment (i hope it will..)
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"nodes.h"
#include	"effects.h"
#include	"decals.h"
#include	"soundent.h"
#include	"scripted.h"
#include	"game.h"
#include "particle_defs.h"
extern int gmsgParticles;

class CCarTrack : public CBaseEntity
{
public:
	void Precache(void);
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	int	Save( CSave &save ); 
	int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	CBaseEntity *pForForLastTarget;
	CBaseEntity *pForLastTarget;
	CBaseEntity *pLastTarget;
	CBaseEntity *pNextTarget;
	CBaseEntity *pNextNextTarget;
	CBaseEntity *pNextNextNextTarget;

};

//LINK_ENTITY_TO_CLASS( car_track, CCarTrack );

TYPEDESCRIPTION	CCarTrack::m_SaveData[] = 
{
	DEFINE_FIELD( CCarTrack, pForForLastTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CCarTrack, pForLastTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CCarTrack, pLastTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CCarTrack, pNextTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CCarTrack, pNextNextTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CCarTrack, pNextNextNextTarget, FIELD_CLASSPTR ),

};
IMPLEMENT_SAVERESTORE( CCarTrack, CBaseEntity );

class CCarModel : public CBaseAnimating
{
public:
	//void Precache( void );
	void Spawn( void );
	void ModelThink( void );

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
	
	CBaseEntity	*pCar; //the car we blong to
	float wheelyaw;
	int wheelcount;
};

LINK_ENTITY_TO_CLASS( car_model, CCarModel );

TYPEDESCRIPTION	CCarModel::m_SaveData[] = 
{
	DEFINE_FIELD( CCarModel, pCar, FIELD_CLASSPTR ),
	DEFINE_FIELD( CCarModel, wheelyaw, FIELD_FLOAT ),
	DEFINE_FIELD( CCarModel, wheelcount, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CCarModel, CBaseAnimating );


//carmodel bodygroups
#define CAR_MODEL_FRONT				0
#define CAR_MODEL_ROOF				1
#define CAR_MODEL_BACK				2
#define CAR_MODEL_FRONT_LEFT_DOOR	3
#define CAR_MODEL_FRONT_RIGHT_DOOR	4
#define CAR_MODEL_BACK_LEFT_DOOR	5
#define CAR_MODEL_BACK_RIGHT_DOOR	6
#define CAR_MODEL_FRONT_LEFT_WHEEL	7
#define CAR_MODEL_FRONT_RIGHT_WHEEL	8
#define CAR_MODEL_BACK_LEFT_WHEEL	9
#define CAR_MODEL_BACK_RIGHT_WHEEL	10


void CCarModel :: Spawn(void)
{
	wheelyaw = 0.0;
	wheelcount = 0;

	SetBoneController ( 0, wheelyaw );
	SetBoneController ( 1, 0 );
	SetBoneController ( 2, 0 );
	SetBoneController ( 3, 0 );

	pCar = Instance( pev->owner );

//	pev->scale = 2;//scale not supported on Am's source

	pev->body = 0; //undamaged car

	SetBodygroup( 0, 0 );
	SetBodygroup( 1, 0 );
	SetBodygroup( 2, 0 );

	SetUse(&CCarModel::Use);

	SetThink(&CCarModel::ModelThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetNextThink( 0.1 );
}

void CCarModel :: ModelThink(void)
{
	pev->angles = pCar->pev->angles;
	pev->origin = pCar->pev->origin;
	pev->origin.z = pCar->pev->origin.z - 14;//this determines the position???

//	pev->origin.z = pCar->pev->origin.z + 64;//this determines the position???
	
	pev->velocity = pCar->pev->velocity;
	pev->avelocity = pCar->pev->avelocity;

	SetBoneController ( 0, pev->avelocity.y );
}

void CCarModel :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pCar->Use( pActivator, pCaller, useType, value );
}


class CEnvCar : public CBaseEntity
{
public:
	//standard ent functions
	void Precache( void );
	void Spawn( void );
	void PostSpawn( void );
	void CarRespawn(void);

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	//sys test
	int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() | FCAP_FORCE_TRANSITION; }	// Always go across transitions

	//thinking functions
	void StartThink( void );
	void DriveThink( void );
	void StopThink( void );
	void IdleThink( void );

	//routing functions
	
	void GetRoute(void);

	void CheckRoute(void);
	void CheckRoutePos16(void);
	void CheckRoutePos15(void);
	void CheckRoutePos14(void);
	void CheckRoutePos13(void);
	void CheckRoutePos12(void);
	void CheckRoutePos11(void);
	void CheckRoutePos10(void);
	void CheckRoutePos9(void);
	void CheckRoutePos8(void);
	void CheckRoutePos7(void);
	void CheckRoutePos6(void);
	void CheckRoutePos5(void);
	void CheckRoutePos4(void);
	void CheckRoutePos3(void);
	void CheckRoutePos2(void);
	void CheckRoutePos1(void);

	int CalcCheckRoute(Vector vecPos1, Vector vecPos2);
	void ObstacleRoute(void);
	void ObstacleRouteCopy(void);
	void NewRouteStart(Vector vecPosNew, int countRoute, BOOL bReCheck);
	int AlreadyThere(Vector vecPos1, Vector vecPos2);


	//temp function to draw route
	void DrawLines(void);
	void DrawSprite(Vector vecPoint);


	//driving functions
	void DriveRoute(void);
	void DriveAtPosition(Vector vecPos, Vector vecDest, BOOL freeafter);
	void Accelerate( float value );
	void Decelerate( float value );


	//using functions
	void IdleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void PlayerUseForward(void);
	void PlayerUseBackward(void);
	void PlayerUseLeft(void);
	void PlayerUseRight(void);
	void PlayerUseBreak(void);
	void PlayerUseHeavyBreak(void);
	void PlayerUseHorn(void);
	void PlayerUseLeftOff(void);
	void PlayerUseRightOff(void);
	void PlayerUseExit(void);

	void DrawBrakeLines(void);
	//void DecalTrace( TraceResult *pTrace );

	virtual int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void Damage(Vector vecDamagePos, float flDamage);
	void DamageLocation(int location, float flDamage);
	void DamageAll(float flDamage);

	void Smoke(BOOL bDarkSmoke);
	void Explode(void);

	void EXPORT Touch ( CBaseEntity *pVictim );
	void Blocked( CBaseEntity *pOther );
	
	
	BOOL brushmodel;
	BOOL bRouteDone;

	BOOL checkrouteallowed;

	BOOL foundpathright;
	BOOL foundpathleft;

	int countObstacleRout;

	CBaseEntity *pForForTarget;
	CBaseEntity *pForTarget;
	CBaseEntity *pCurrentTarget;
	CBaseEntity *pNextTarget;
	CBaseEntity *pNextNextTarget;
	CBaseEntity *pNextNextNextTarget;

	CCarModel *pModel;

	//Positions for pathfinding
	Vector Pos1;
	Vector Pos2;
	Vector Pos3;
	Vector Pos4;
	Vector Pos5;
	Vector Pos6;
	Vector Pos7;
	Vector Pos8;
	Vector Pos9;
	Vector Pos10;
	Vector Pos11;
	Vector Pos12;
	Vector Pos13;
	Vector Pos14;
	Vector Pos15;
	Vector Pos16;
	Vector Pos17;
	Vector Pos18;

	Vector vecDirection;
	Vector vecAftercheck;

	Vector vecLeftFrontWheelPos;
	Vector vecRightFrontWheelPos;
	Vector vecLeftBackWheelPos;
	Vector vecRightBackWheelPos;

	float fl_speed;
	float fl_bouncespeed;
	BOOL bIsCarFlying;
	BOOL bPlayerExited;
	float fl_maxspeed;

	float fl_last_time_backward;
	float fl_last_time_forward;
	float fl_last_time_heavybreak;
	float fl_last_time_break;
	float fl_last_time_smoked;

	float fl_last_victim_time;
	float fl_last_victim_time1;

	BOOL wheel_left;
	BOOL wheel_right;

	Vector vecCurrentUp;// = Vector(0,0,1);
	Vector vecDesiredDir;
	
	int iSprite;// = PRECACHE_MODEL( "sprites/muz4.spr" );
	int iExplode;
	//int decal_index;

	int iSprite2;// = PRECACHE_MODEL( "sprites/muz4.spr" );

	//Health
	float fl_health;
	float fl_health_CAR_MODEL_FRONT;
	float fl_health_CAR_MODEL_BACK;

	float m_flNextBreakTime;
	float m_flNextSoundTime;

	Vector vecSavedStartOrigin;

	CBaseEntity *savedCarVictim;
	CBaseEntity *savedCarVictim1;

	Vector VecAffin(const Vector &vecStart, float ax, float ay, float az) 
	{ 
		 Vector vecEnd; 
 
		 UTIL_MakeVectors(Vector(ax,ay,az));//pev->angles); 
 
		 vecEnd.x = ( gpGlobals->v_forward.x * vecStart.x + gpGlobals->v_right.x * vecStart.y + gpGlobals->v_up.x * vecStart.z); 
		 vecEnd.y = ( gpGlobals->v_forward.y * vecStart.x + gpGlobals->v_right.y * vecStart.y + gpGlobals->v_up.y * vecStart.z); 
		 vecEnd.z = ( gpGlobals->v_forward.z * vecStart.x + gpGlobals->v_right.z * vecStart.y + gpGlobals->v_up.z * vecStart.z); 
 
		 return vecEnd; 
	}

	Vector CrossVector(const Vector &vecUp, const Vector &vecFor) 
	{ 
		 Vector vecResult; 
 
		 vecResult.x = (vecUp.y*vecFor.z - vecUp.z*vecFor.y); 
		 vecResult.y = (vecUp.z*vecFor.x - vecUp.x*vecFor.z); 
		 vecResult.z = (vecUp.x*vecFor.y - vecUp.y*vecFor.x); 
 
		 return vecResult; 
	}
};

LINK_ENTITY_TO_CLASS( func_car_brush, CEnvCar );
LINK_ENTITY_TO_CLASS( func_car, CEnvCar );
LINK_ENTITY_TO_CLASS( env_car_brush, CEnvCar );
LINK_ENTITY_TO_CLASS( env_car, CEnvCar );

TYPEDESCRIPTION	CEnvCar::m_SaveData[] = 
{
	
	DEFINE_FIELD( CEnvCar, brushmodel, FIELD_BOOLEAN ),
	DEFINE_FIELD( CEnvCar, bRouteDone, FIELD_BOOLEAN ),

	
	DEFINE_FIELD( CEnvCar, checkrouteallowed, FIELD_BOOLEAN ),

	
	DEFINE_FIELD( CEnvCar, countObstacleRout, FIELD_INTEGER ),

	DEFINE_FIELD( CEnvCar, pForForTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CEnvCar, pForTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CEnvCar, pCurrentTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CEnvCar, pNextTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CEnvCar, pNextNextTarget, FIELD_CLASSPTR ),
	DEFINE_FIELD( CEnvCar, pNextNextNextTarget, FIELD_CLASSPTR ),
	
	DEFINE_FIELD( CEnvCar, pModel, FIELD_CLASSPTR ),

	
	DEFINE_FIELD( CEnvCar, Pos1, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos2, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos3, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos4, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos5, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos6, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos7, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos8, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos9, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos10, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos11, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos12, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos13, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos14, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos15, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos16, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos17, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, Pos18, FIELD_VECTOR ),
	
	DEFINE_FIELD( CEnvCar, vecDirection, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, vecAftercheck, FIELD_VECTOR ),

	DEFINE_FIELD( CEnvCar, foundpathright, FIELD_BOOLEAN ),
	DEFINE_FIELD( CEnvCar, foundpathleft, FIELD_BOOLEAN ),

	
	DEFINE_FIELD( CEnvCar, bPlayerExited, FIELD_BOOLEAN ),
	DEFINE_FIELD( CEnvCar, bIsCarFlying, FIELD_BOOLEAN ),
	DEFINE_FIELD( CEnvCar, fl_speed, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_maxspeed, FIELD_FLOAT ),//sys
	DEFINE_FIELD( CEnvCar, fl_bouncespeed, FIELD_FLOAT ),

	DEFINE_FIELD( CEnvCar, fl_last_time_backward, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_last_time_forward, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_last_time_heavybreak, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_last_time_break, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_last_time_smoked, FIELD_FLOAT ),
	
	
	DEFINE_FIELD( CEnvCar, iSprite, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvCar, iExplode, FIELD_INTEGER ),
	
	//DEFINE_FIELD( CEnvCar, decal_index, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvCar, iSprite2, FIELD_INTEGER ),

	
	DEFINE_FIELD( CEnvCar, vecLeftFrontWheelPos, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, vecRightFrontWheelPos, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, vecLeftBackWheelPos, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, vecRightBackWheelPos, FIELD_VECTOR ),

	DEFINE_FIELD( CEnvCar, wheel_left, FIELD_BOOLEAN ),
	DEFINE_FIELD( CEnvCar, wheel_right, FIELD_BOOLEAN ),
	
	DEFINE_FIELD( CEnvCar, vecCurrentUp, FIELD_VECTOR ),
	DEFINE_FIELD( CEnvCar, vecDesiredDir, FIELD_VECTOR ),
	

	//health
	DEFINE_FIELD( CEnvCar, fl_health, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_health_CAR_MODEL_FRONT, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_health_CAR_MODEL_BACK, FIELD_FLOAT ),


	DEFINE_FIELD( CEnvCar, vecSavedStartOrigin, FIELD_VECTOR ),
	
	DEFINE_FIELD( CEnvCar, savedCarVictim, FIELD_CLASSPTR ),
	DEFINE_FIELD( CEnvCar, savedCarVictim1, FIELD_CLASSPTR ),

	DEFINE_FIELD( CEnvCar, fl_last_victim_time, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvCar, fl_last_victim_time1, FIELD_FLOAT ),

};

IMPLEMENT_SAVERESTORE( CEnvCar, CBaseEntity );


void CCarTrack :: Precache()
{
	PRECACHE_MODEL((char*)STRING(pev->model));
}

void CCarTrack :: Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetOrigin(pev, pev->origin);
	//carstop1
	SetUse(&CCarTrack::Use);

}

void CCarTrack :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{

	//nothing yet

}



void CEnvCar :: Precache()
{
	PRECACHE_MODEL( "sprites/steam1.spr" );
	iSprite = PRECACHE_MODEL( "sprites/steam1.spr" );// smoke
	iExplode = PRECACHE_MODEL( "sprites/fexplo.spr" );// smoke
	
	iSprite2 = PRECACHE_MODEL( "sprites/glow02.spr" );// smoke

	PRECACHE_MODEL("sprites/gargeye1.spr");

	PRECACHE_MODEL((char*)STRING(pev->model));
	
	PRECACHE_MODEL("models/cars/car2.mdl");

	//cars
	PRECACHE_SOUND("car_brake.wav");
	PRECACHE_SOUND("bus_horn.wav");
	PRECACHE_SOUND("car_engine.wav");
	PRECACHE_SOUND("first.wav");
	PRECACHE_SOUND("second.wav");
	PRECACHE_SOUND("third.wav");
}	

void CEnvCar :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->solid = SOLID_BSP;

	pev->takedamage = DAMAGE_YES;

	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	
	vecSavedStartOrigin = pev->origin;

	savedCarVictim = NULL;
	savedCarVictim1 = NULL;
	fl_last_victim_time = fl_last_victim_time1 = 0;

	//pev->effects |= EF_NODRAW;

	pev->avelocity = Vector(0,0,0);
	pev->velocity = Vector(0,0,0);

	fl_health = 1000;
	fl_health_CAR_MODEL_FRONT = 1000;//300
	fl_health_CAR_MODEL_BACK = 1000;

	pev->health = fl_health;
	
	pForForTarget = NULL;
	pForTarget = NULL;
	pCurrentTarget = NULL;
	pNextTarget = NULL;
	pNextNextTarget = NULL;
	pNextNextNextTarget = NULL;
	pModel = NULL;

	vecDirection = Pos1 = Pos2 = Pos3 = Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	checkrouteallowed = foundpathright = foundpathleft = bRouteDone = FALSE;
	
	vecLeftFrontWheelPos.x = pev->maxs.x;
	vecLeftFrontWheelPos.y = pev->mins.y;
	vecLeftFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightFrontWheelPos.x = pev->maxs.x;
	vecRightFrontWheelPos.y = pev->maxs.y;
	vecRightFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecLeftBackWheelPos.x = pev->mins.x;
	vecLeftBackWheelPos.y = pev->mins.y;
	vecLeftBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightBackWheelPos.x = pev->mins.x;
	vecRightBackWheelPos.y = pev->maxs.y;
	vecRightBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	fl_bouncespeed = fl_speed = 0;
	bIsCarFlying = FALSE;
	bPlayerExited = FALSE;
	
	wheel_left = FALSE;
	wheel_right = FALSE;
	
	vecCurrentUp = Vector(0,0,1);
	vecDesiredDir = Vector(0,0,0);

	fl_last_time_forward = fl_last_time_backward = fl_last_time_heavybreak = fl_last_time_break = fl_last_time_smoked = 0;
	
	SetTouch(&CEnvCar::Touch);
	SetUse(&CEnvCar::Use);

	SetThink(&CEnvCar::IdleThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetNextThink( 0.1 );

	//moved from postspawn, due to bug
	//create the model render ent
	pModel = GetClassPtr( (CCarModel *)NULL );

	pModel->pev->model = MAKE_STRING("models/cars/car2.mdl");	
	SET_MODEL(ENT(pModel->pev), "models/cars/car2.mdl");

//	pModel->pev->model = MAKE_STRING(pev->model);	
//	SET_MODEL(ENT(pModel->pev), STRING(pev->model));

	pModel->pev->origin = pev->origin;
	UTIL_SetOrigin(pModel->pev, pev->origin);

	pModel->pev->classname = MAKE_STRING("car_model");
	pModel->pev->solid = SOLID_NOT;
	pModel->pev->movetype = MOVETYPE_NOCLIP;
	pModel->pev->owner = edict();
	pModel->Spawn();
}


void CEnvCar :: Touch ( CBaseEntity *pVictim )
{
	//ALERT(at_notice, "TOUCHED\n");
	if (FClassnameIs( pVictim->pev, "monster_npc_male") || FClassnameIs( pVictim->pev, "monster_npc_female")) //we found a victim
	{
		pVictim->TakeDamage(pev, pev, pev->velocity.Length()/8, DMG_CRUSH); // KILL VICTIM HAHAHAHA
	}
	else if (FClassnameIs( pVictim->pev, "player"))
	{
		pVictim->TakeDamage(pev, pev, pev->velocity.Length()/4, DMG_CRUSH); // KILL VICTIM HAHAHAHA
	}

	pVictim->pev->velocity = pev->velocity*1.1;
}

void CEnvCar :: Blocked( CBaseEntity *pOther )
{
	//ALERT(at_notice, "BLOCKED\n");
	pOther->TakeDamage(pev, pev, 9999, DMG_CRUSH); // KILL VICTIM HAHAHAHA
}
void CEnvCar :: IdleThink()
{
	if ( FClassnameIs ( pev, "car_rally" ) )//sys
	fl_maxspeed = 2400;
	else if ( FClassnameIs ( pev, "car_fork" ) )
	fl_maxspeed = 800;
	else if ( FClassnameIs ( pev, "vehicle_tank" ) )
	fl_maxspeed = 700;
	else
	fl_maxspeed = 1000;

	Vector ModelAngles;

	pev->fuser4 = fl_speed;

	if (fl_speed == 0 && fl_bouncespeed == 0)
	{
		pev->velocity = Vector(0,0,0);
		pev->avelocity = Vector(0,0,0);
	}

	if (fl_health_CAR_MODEL_FRONT <= 0)
		fl_health = fl_health_CAR_MODEL_FRONT;

	if (pev->health == 2)
	{
		SetThink(NULL);
		CarRespawn();
	}

	if (fl_health <= 0)
	{
		pev->health = 1;
		fl_health = 1;
		pev->velocity = Vector(0,0,0);
		pev->avelocity = Vector(0,0,0);
		fl_speed = fl_bouncespeed = 0;
		pModel->pev->velocity = Vector(0,0,0);
		pModel->pev->avelocity = Vector(0,0,0);
	//	Explode();//sys
	}

	Vector vecDir;
	UTIL_MakeVectorsPrivate(pev->angles, vecDir, NULL, NULL);

	if (!bIsCarFlying)
	{
		pev->velocity = vecDesiredDir*fl_bouncespeed + vecDir.Normalize()*fl_speed;
		if (fl_bouncespeed > 0)
		{
			fl_bouncespeed -= 8;
			if (fl_bouncespeed < 20)
				fl_bouncespeed = 0;
		}
		else if (fl_bouncespeed < 0)
		{
			fl_bouncespeed += 8;
			if (fl_bouncespeed > -0)
				fl_bouncespeed = 0;
		}
	}

	

	

	Vector vecStart;
	Vector vecEnd;
	//Vector vecCurrentUp;

	TraceResult tr_origin;
	TraceResult tr_left_front;
	TraceResult tr_right_front;
	TraceResult tr_left_back;
	TraceResult tr_right_back;

	//UTIL_MakeVectorsPrivate(pev->angles,NULL,NULL,vecCurrentUp);

	vecStart = vecEnd = pev->origin;
	
	vecEnd = vecStart - vecCurrentUp.Normalize()*24;

	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, dont_ignore_glass, edict(), &tr_origin);

	Vector vecLeftFrontWheelPosStart;
	Vector vecLeftFrontWheelPosEnd;
	vecLeftFrontWheelPosStart = vecLeftFrontWheelPosEnd = VecAffin(vecLeftFrontWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	
	vecLeftFrontWheelPosEnd = vecLeftFrontWheelPosStart - vecCurrentUp.Normalize()*24;
	
	Vector vecRightFrontWheelPosStart;
	Vector vecRightFrontWheelPosEnd;
	vecRightFrontWheelPosStart = vecRightFrontWheelPosEnd = VecAffin(vecRightFrontWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	
	vecRightFrontWheelPosEnd = vecRightFrontWheelPosStart - vecCurrentUp.Normalize()*24;

	
	Vector vecLeftBackWheelPosStart;
	Vector vecLeftBackWheelPosEnd;
	vecLeftBackWheelPosStart = vecLeftBackWheelPosEnd = VecAffin(vecLeftBackWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	
	vecLeftBackWheelPosEnd = vecLeftBackWheelPosStart - vecCurrentUp.Normalize()*24;

	
	Vector vecRightBackWheelPosStart;
	Vector vecRightBackWheelPosEnd;
	vecRightBackWheelPosStart = vecRightBackWheelPosEnd = VecAffin(vecRightBackWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	
	vecRightBackWheelPosEnd = vecRightBackWheelPosStart - vecCurrentUp.Normalize()*24;


	UTIL_TraceLine(pev->origin+vecLeftFrontWheelPosStart, pev->origin+vecLeftFrontWheelPosEnd, dont_ignore_monsters, dont_ignore_glass, edict(), &tr_left_front);
	UTIL_TraceLine(pev->origin+vecRightFrontWheelPosStart, pev->origin+vecRightFrontWheelPosEnd, dont_ignore_monsters, dont_ignore_glass, edict(), &tr_right_front);
	UTIL_TraceLine(pev->origin+vecLeftBackWheelPosStart, pev->origin+vecLeftBackWheelPosEnd, dont_ignore_monsters, dont_ignore_glass, edict(), &tr_left_back);
	UTIL_TraceLine(pev->origin+vecRightBackWheelPosStart, pev->origin+vecRightBackWheelPosEnd, dont_ignore_monsters, dont_ignore_glass, edict(), &tr_right_back);

	ModelAngles = pev->angles;


	Vector vecCurrentDir;
	Vector vecCurrentSide;
	UTIL_MakeVectorsPrivate(pev->angles, vecCurrentDir, vecCurrentSide, NULL);

	Vector vecPlaneNormal;

	if (tr_origin.fAllSolid)
	{
		vecCurrentUp = Vector(0,0,1);
		vecCurrentDir.z = 0;

		pev->angles.x = UTIL_VecToAngles(vecCurrentDir).x;
		pev->angles.z = UTIL_VecToAngles(vecCurrentDir).z;

		pev->origin.z = tr_origin.vecEndPos.z + 16;

		if (bIsCarFlying)
		{
			if (pev->velocity.z <= -100)
				EMIT_SOUND_DYN(edict(),CHAN_ITEM,"debris/bustmetal2.wav",1.0,ATTN_NORM,0,100);
			pev->velocity.z = 0;
		}

		bIsCarFlying = FALSE;
	}
	else if ((tr_origin.flFraction == 1.0) && (tr_left_front.flFraction + tr_right_front.flFraction + tr_left_back.flFraction + tr_right_back.flFraction) >= 2.0)
	{
		vecCurrentUp = Vector(0,0,1);

		bIsCarFlying = TRUE;

		if (fl_speed > 2)
			fl_speed -= 8;
		else if (fl_speed < -2)
			fl_speed += 8;
		else
			fl_speed = 0;

		pev->velocity.x = vecDir.Normalize().x*fl_speed;
		pev->velocity.y = vecDir.Normalize().y*fl_speed;

		if (pev->velocity.z > -fl_maxspeed)
			pev->velocity.z -= 20;
		else
			pev->velocity.z = -fl_maxspeed;
	}
	else
	{
		if (bIsCarFlying)
		{
			if (pev->velocity.z <= -100)
				EMIT_SOUND_DYN(edict(),CHAN_ITEM,"debris/bustmetal2.wav",1.0,ATTN_NORM,0,100);
			pev->velocity.z = 0;
		}

		bIsCarFlying = FALSE;

		/*
		Vector vecA = Vector(0,0,0);
		Vector vecB = Vector(0,0,0);
		Vector vecC = Vector(0,0,0);

		int pointtotake = 0;

		if (tr_left_front.flFraction < 1.0)
		{
			vecA = tr_left_front.vecEndPos;
			pointtotake = 1;
		}
		if (tr_right_front.flFraction < 1.0)
		{
			if (vecA != Vector(0,0,0))
				vecB = tr_right_front.vecEndPos;
			else
			{
				vecA = tr_right_front.vecEndPos;
				pointtotake = 2;
			}

		}
		if (tr_left_back.flFraction < 1.0)
		{
			if (vecA != Vector(0,0,0))
			{
				if (vecB != Vector(0,0,0))
					vecC = tr_left_back.vecEndPos;
				else
					vecB = tr_left_back.vecEndPos;
			}
			else
			{
				vecA = tr_left_back.vecEndPos;
				pointtotake = 3;
			}

		}
		if (tr_right_back.flFraction < 1.0)
		{
			if (vecA != Vector(0,0,0))
			{
				if (vecB.z < tr_right_back.vecEndPos.z)
					vecC = tr_right_back.vecEndPos;
				else if (vecB.z < tr_right_back.vecEndPos.z)
					vecB = tr_right_back.vecEndPos;
			}
			else
			{
				vecA = tr_right_back.vecEndPos;
				pointtotake = 4;
			}

		}

		
		if (vecC == Vector(0,0,0))
		{
			if (vecA == Vector(0,0,0) || vecB == Vector(0,0,0))
			{
				bIsCarFlying = TRUE;
				SetNextThink( 0.02 );
				return;
			}
			else if (tr_origin.flFraction == 1.0)
			{
				bIsCarFlying = TRUE;
				SetNextThink( 0.02 );
				return;
			}
			else
				vecC = tr_origin.vecEndPos;
		}


		vecPlaneNormal = CrossVector(vecB-vecA, vecC-vecA);
		if (vecPlaneNormal.z < 0)
			vecPlaneNormal = -vecPlaneNormal;
		*/
		vecPlaneNormal = tr_origin.vecPlaneNormal;


		if ((vecPlaneNormal.x == 0) && (vecPlaneNormal.y == 0))
		{
			vecCurrentUp = Vector(0,0,1);
			
			vecCurrentDir.z = 0;

			pev->angles.x = UTIL_VecToAngles(vecCurrentDir).x;
			pev->angles.z = UTIL_VecToAngles(vecCurrentDir).z;

			if (tr_origin.flFraction < 1.0)
				pev->origin.z = tr_origin.vecEndPos.z + 16;
		}
		else
		{
			vecCurrentUp = vecPlaneNormal;

			Vector vecFloorRightAffin;
			Vector vecDesiredDir;
			Vector vecDesiredZed;
			Vector vecDesiredDirModel;
			Vector vecDesiredZedModel;


			vecFloorRightAffin = CrossVector(vecCurrentDir,vecPlaneNormal);
			vecDesiredDir  = CrossVector(vecFloorRightAffin,vecPlaneNormal);
			vecDesiredZed  = CrossVector(vecPlaneNormal,vecDesiredDir);
			vecDesiredDirModel  = CrossVector(vecPlaneNormal,vecFloorRightAffin);
			vecDesiredZedModel  = CrossVector(vecDesiredDir,vecPlaneNormal);


			//ALERT(at_notice, "vecDesiredDir ist %f %f %f\n", vecDesiredDir.x, vecDesiredDir.y, vecDesiredDir.z);

			pev->angles.x = UTIL_VecToAngles(vecDesiredDir).x;
			ModelAngles.x = UTIL_VecToAngles(vecDesiredDirModel).x;

			pev->angles.z = UTIL_VecToAngles(vecDesiredZedModel).x;
			ModelAngles.z = UTIL_VecToAngles(vecDesiredZedModel).x;

			ModelAngles.y = pev->angles.y;

			//if (tr_origin.flFraction < 1.0)
				pev->origin = tr_origin.vecEndPos + vecPlaneNormal.Normalize()*16;//vecPlaneNormal.Normalize()*16;

				/*
			switch (pointtotake)
			{
			case 1:
				pev->origin = (tr_left_front.vecEndPos-vecLeftFrontWheelPosStart) + vecPlaneNormal.Normalize()*16;
				break;
			case 2:
				pev->origin = (tr_right_front.vecEndPos-vecRightFrontWheelPosStart) + vecPlaneNormal.Normalize()*16;
				break;
			case 3:
				pev->origin = (tr_left_back.vecEndPos-vecLeftBackWheelPosStart) + vecPlaneNormal.Normalize()*16;
				break;
			case 4:
				pev->origin = (tr_right_back.vecEndPos-vecRightBackWheelPosStart) + vecPlaneNormal.Normalize()*16;
				break;
			
			}
				*/

		}

	}

	 //TODO: COLLISION FUN
	TraceResult trAlongFront;
	TraceResult trAlongBack;

	BOOL alongfront = FALSE;
	BOOL alongback = FALSE;

	Vector vecHullStart;
	Vector vecHullEnd;
	Vector vecFrontNormal;
	Vector vecBackNormal;

	int y;
	y = 0;

	vecHullStart = vecHullEnd = pev->origin;

	//alongfront
	for (y = pev->mins.y; y < pev->maxs.y; y++)
	{
		vecHullStart = pev->origin;

		vecHullStart = vecHullStart + vecCurrentSide.Normalize() * y;

		vecHullEnd = vecHullStart + vecCurrentDir*pev->maxs.x;

		UTIL_TraceHull( vecHullStart, vecHullEnd, dont_ignore_monsters, point_hull, ENT(pev), &trAlongFront );

		if ( trAlongFront.flFraction != 1.0 )
		{
			vecDesiredDir = vecCurrentDir +2*CrossProduct(CrossProduct(vecCurrentDir,-trAlongFront.vecPlaneNormal),-trAlongFront.vecPlaneNormal);
			alongfront = TRUE;

			if (trAlongFront.pHit)
			{
				CBaseEntity *pVictim = CBaseEntity::Instance(trAlongFront.pHit);;
				if (pVictim)
				{
					if (FClassnameIs( pVictim->pev, "monster_npc_male") || FClassnameIs( pVictim->pev, "monster_npc_female")) //we found a victim
					{
						pVictim->TakeDamage(pev, pev, pev->velocity.Length()/40, DMG_CRUSH); // KILL VICTIM HAHAHAHA
					}
					else if (FClassnameIs( pVictim->pev, "player"))
					{
						pVictim->TakeDamage(pev, pev, pev->velocity.Length()/40, DMG_CRUSH); // KILL VICTIM HAHAHAHA
					}
					else if (FClassnameIs( pVictim->pev, "func_car_brush"))
					{
						if (!savedCarVictim || savedCarVictim != pVictim || fl_last_victim_time < gpGlobals->time +1)
						{
							TraceResult trTest;
							UTIL_TraceLine(pev->origin, pev->origin+vecCurrentDir.Normalize()*128,ignore_monsters,ignore_glass,edict(),&trTest);
							if (trTest.pHit && Instance(trTest.pHit) == pVictim)
							{
								pVictim->TakeDamage(pev, pev, pev->velocity.Length()/40, DMG_GENERIC); // KILL VICTIM HAHAHAHA
								savedCarVictim = pVictim;
								if (fl_speed > 0)
									pVictim->pev->velocity = pVictim->pev->velocity + pev->velocity;
								else
									pVictim->pev->velocity = pVictim->pev->velocity + vecCurrentDir.Normalize()*4;
								pev->velocity = pev->velocity*0.5;
								fl_speed *= 0.5;
								fl_last_victim_time = gpGlobals->time;
								alongfront = FALSE; //we do not want to take damage if we hit another car ^^
							}
						}
					}
				}
			}
		}
	}

	//alongback
	for (y = pev->mins.y; y < pev->maxs.y; y++)
	{
		vecHullStart = pev->origin;

		vecHullStart = vecHullStart + vecCurrentSide.Normalize() * y;

		vecHullEnd = vecHullStart + vecCurrentDir*pev->mins.x;

		UTIL_TraceHull( vecHullStart, vecHullEnd, dont_ignore_monsters, point_hull, ENT(pev), &trAlongBack );

		if ( trAlongBack.flFraction != 1.0 )
		{
			vecDesiredDir = vecCurrentDir +2*CrossProduct(CrossProduct(vecCurrentDir,-trAlongBack.vecPlaneNormal),-trAlongBack.vecPlaneNormal);
			alongback = TRUE;

			if (trAlongBack.pHit)
			{
				CBaseEntity *pVictim1 = CBaseEntity::Instance(trAlongBack.pHit);
				if (pVictim1)
				{
					if (FClassnameIs( pVictim1->pev, "monster_npc_male") || FClassnameIs( pVictim1->pev, "monster_npc_female")) //we found a victim
					{
						pVictim1->TakeDamage(pev, pev, pev->velocity.Length()/40, DMG_CRUSH); // KILL VICTIM HAHAHAHA
					}
					else if (FClassnameIs( pVictim1->pev, "player"))
					{
						pVictim1->TakeDamage(pev, pev, pev->velocity.Length()/40, DMG_CRUSH); // KILL VICTIM HAHAHAHA
					}
					else if (FClassnameIs( pVictim1->pev, "func_car_brush"))
					{
						if (!savedCarVictim1 || savedCarVictim != pVictim1 || fl_last_victim_time1 < gpGlobals->time +1)
						{
							TraceResult trTest;
							UTIL_TraceLine(pev->origin, pev->origin+vecCurrentDir.Normalize()*128,ignore_monsters,ignore_glass,edict(),&trTest);
							if (trTest.pHit && Instance(trTest.pHit) == pVictim1)
							{
								pVictim1->TakeDamage(pev, pev, pev->velocity.Length()/40, DMG_GENERIC); // KILL VICTIM HAHAHAHA
								savedCarVictim1 = pVictim1;
								if (fl_speed > 0)
									pVictim1->pev->velocity = pVictim1->pev->velocity + pev->velocity;
								else
									pVictim1->pev->velocity = pVictim1->pev->velocity + vecCurrentDir.Normalize()*4;
								pev->velocity = pev->velocity*0.5;
								fl_speed *= 0.5;
								fl_last_victim_time1 = gpGlobals->time;
								alongback = FALSE; //we do not want to take damage if we hit another car ^^
							}
						}
					}
				}
			}
		}
	}

	
	if (alongfront)
	{
		float damage;
		damage = 1;
		if (fl_speed < 0)
			damage += -fl_speed;
		else
			damage += fl_speed;

		if (fl_bouncespeed < 0)
			damage += -fl_bouncespeed;
		else
			damage += fl_bouncespeed;

//		DamageLocation(CAR_MODEL_FRONT, damage/10);

		/*
		if (DotProduct(vecCurrentDir,trAlongFront.vecPlaneNormal) < 0)
			pev->avelocity = Vector(0,1,0) * -30;
		else if (DotProduct(vecCurrentDir,trAlongFront.vecPlaneNormal) > 0)
			pev->avelocity = Vector(0,1,0) * 30;
			*/


		if (fl_speed > 100)
		{
			fl_bouncespeed = fl_speed *-0.5;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		else if (fl_speed > 50)
		{
			fl_bouncespeed = fl_speed*-0.75;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		else if (fl_speed > 20)
		{
			fl_bouncespeed = fl_speed*-1;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		else if (fl_speed > 0)
		{
			fl_bouncespeed = -20;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		fl_speed = fl_bouncespeed/4;
		//PlayerUseRight
	}

	if (alongback)
	{
		float damage;
		damage = 1;
		if (fl_speed < 0)
			damage += -fl_speed;
		else
			damage += fl_speed;

		if (fl_bouncespeed < 0)
			damage += -fl_bouncespeed;
		else
			damage += fl_bouncespeed;

//		DamageLocation(CAR_MODEL_BACK, damage/10);

		//if (!alongfront)
		{
			/*
			if (DotProduct(vecCurrentDir,trAlongBack.vecPlaneNormal) < 0)
				pev->avelocity = Vector(0,1,0) * 30;
			else if (DotProduct(vecCurrentDir,trAlongBack.vecPlaneNormal) > 0)
				pev->avelocity = Vector(0,1,0) * -30;
				*/
		}
		if (alongfront)
		{
			pev->velocity = Vector(0,0,0);//stop, you are stuck
			pev->avelocity = Vector(0,0,0);//stop, you are stuck
		}
		else if (fl_speed < -100)
		{
			fl_bouncespeed = fl_speed *-0.5;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		else if (fl_speed < -50)
		{
			fl_bouncespeed = fl_speed*-0.75;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		else if (fl_speed < -20)
		{
			fl_bouncespeed = fl_speed*-1;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		else if (fl_speed < 0)
		{
			fl_bouncespeed = 20;
			pev->velocity = vecDesiredDir.Normalize()*fl_bouncespeed; //bounce back, half speed
		}
		fl_speed = fl_bouncespeed/4;
	}

	//render model
	pModel->pev->velocity = pev->velocity;
	pModel->pev->avelocity = pev->avelocity;
	
	pModel->pev->angles = ModelAngles;
	pModel->pev->origin = pev->origin;
	/*
	Vector vecBla = pev->origin;
	Vector vecBla2 = pev->origin;
	vecBla.z += 64;
	vecBla2.z += 128;
	*/
	//RenderModel(iSprite, vecBla);
	//RenderModel(iSprite2, vecBla2);

	if (fl_last_time_smoked < gpGlobals->time) //do not smoke to often
	{
		if (fl_health_CAR_MODEL_FRONT < 100)
		{
			if (RANDOM_LONG(0,fl_health_CAR_MODEL_FRONT) < 10)
				Smoke(TRUE);
		}
		else if (fl_health_CAR_MODEL_FRONT < 200)
		{
			if (RANDOM_LONG(0,fl_health_CAR_MODEL_FRONT) < 20)
				Smoke(FALSE);
		}

		fl_last_time_smoked = gpGlobals->time + 0.1;
	}

	if (!bIsCarFlying && bPlayerExited)
	{
		PlayerUseHeavyBreak();

		if ( FClassnameIs ( pev, "car_fork" ) )
		pModel->SetBodygroup( 1, 0 );//freeman is not on car
			
		ALERT ( at_console, "NOT USING CAR\n");	

	}
	else
	{
		if ( FClassnameIs ( pev, "car_fork" ) )
		pModel->SetBodygroup( 1, 1);//freeman is on car

		ALERT ( at_console, "USING CAR\n");	
	}
	
//sys
	float flTempVelocityTest = pev->velocity.Length2D() / 4;
/*
	if(flTempVelocityTest >= 200) 
	pModel->pev->framerate = 2;
	else if(flTempVelocityTest >= 100) 
	pModel->pev->framerate = 1;
	else
	pModel->pev->framerate = 0;
	*/
//	EMIT_SOUND_DYN( ENT(pev), CHAN_ITEM, "car_engine.wav", 0.1, ATTN_NORM, 0, flTempVelocityTest );

	if(flTempVelocityTest == 40) 
	EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, "first.wav", 0.9, ATTN_NORM, 0, 100 );

	if(flTempVelocityTest == 70) 
	EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, "second.wav", 0.9, ATTN_NORM, 0, 100 );

	if(flTempVelocityTest == 120) 
	EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, "third.wav", 0.9, ATTN_NORM, 0, 100 );

	if (flTempVelocityTest >= 250)
	flTempVelocityTest = 250;

	if (flTempVelocityTest <= 25)
	flTempVelocityTest = 25;

	if(!bPlayerExited)//player is using car
	{
		if (flTempVelocityTest == 0)
		EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, "motor_idle_loop1.wav", 0.9, ATTN_NORM, 0, 25 );

		if ( m_flNextSoundTime < gpGlobals->time )
		{
			EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, "motor_idle_loop1.wav", 0.9, ATTN_NORM, 0, flTempVelocityTest );
		
			m_flNextSoundTime = gpGlobals->time + 0.1;//3
		}
	}
	/*	
	char szCommand[256];

	int idefault_fov = CVAR_GET_FLOAT( "default_fov" );

	idefault_fov -= 100;

	if (idefault_fov <= 90)
	idefault_fov = 90;

	sprintf( szCommand, "default_fov %f\n", idefault_fov );

	SERVER_COMMAND(szCommand);
	*/
	ALERT ( at_console, "velocity is: %f Km, Health FT: %f, Health BK: %f, sound pitch %f\n", flTempVelocityTest,fl_health_CAR_MODEL_FRONT,fl_health_CAR_MODEL_BACK, flTempVelocityTest);	

	if ( m_flNextBreakTime < gpGlobals->time )
	{
	//	PlayerUseBreak();

		m_flNextBreakTime = gpGlobals->time + 1;
	}

	SetNextThink( 0.02 );//0.05
	//pev->nextthink = pev->ltime + 0.001;// + delay;
}

void CEnvCar :: PostSpawn()
{

}


/*
//carmodel bodygroups
#define CAR_MODEL_FRONT				0;
#define CAR_MODEL_ROOF				1;
#define CAR_MODEL_BACK				2;
#define CAR_MODEL_FRONT_LEFT_DOOR	3;
#define CAR_MODEL_FRONT_RIGHT_DOOR	4;
#define CAR_MODEL_BACK_LEFT_DOOR	5;
#define CAR_MODEL_BACK_RIGHT_DOOR	6;
#define CAR_MODEL_FRONT_LEFT_WHEEL	7;
#define CAR_MODEL_FRONT_RIGHT_WHEEL	8;
#define CAR_MODEL_BACK_LEFT_WHEEL	9;
#define CAR_MODEL_BACK_RIGHT_WHEEL	10;
*/

void CEnvCar :: Explode(void)
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );
		WRITE_SHORT( iExplode );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z + 32 );
		WRITE_BYTE( 1 ); // scale * 0.1
		WRITE_BYTE( 10  ); // framerate
		WRITE_BYTE( TE_EXPLFLAG_NONE  ); // flags
	MESSAGE_END();

}

void CEnvCar :: CarRespawn(void)
{
	UTIL_Remove(pModel);

	CEnvCar	*pNewCar;
	pNewCar = GetClassPtr( (CEnvCar *)NULL );

	pNewCar->pev->classname = MAKE_STRING("func_car_brush");
	pNewCar->pev->model = pev->model;
	pNewCar->pev->origin = vecSavedStartOrigin;
	pNewCar->pev->angles = Vector(0,0,0);
	pNewCar->Spawn();
	
	

	//SET_MODEL( ENT(pNewCar->pev), STRING(pev->model) );


	//UTIL_SetOrigin(pNewCar, vecSavedStartOrigin);

	//pNewCar->pev->solid = SOLID_BSP;
	//pNewCar->pev->movetype = MOVETYPE_PUSH;

	UTIL_Remove(this);
	//Spawn();
}

void CEnvCar :: Smoke(BOOL bDarkSmoke)
{
	Vector vecCurrentDir;
	Vector vecCurrentSide;
	Vector vecCurrentUp;
	Vector vecPosition;
	UTIL_MakeVectorsPrivate(pev->angles,vecCurrentDir,vecCurrentSide,vecCurrentUp);

	vecPosition.x = pev->origin.x + vecCurrentDir.Normalize().x * pev->maxs.x * 0.75;
	vecPosition.y = pev->origin.y + vecCurrentDir.Normalize().y * pev->maxs.x * 0.75;
	vecPosition.z = pev->origin.z + vecCurrentDir.Normalize().z * pev->maxs.x * 0.75;

	if (!bDarkSmoke)
	{
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/steam1.spr", vecPosition, TRUE );
		if ( pSprite )
		{
			pSprite->pev->velocity.x = 0;
			pSprite->pev->velocity.y = 0;
			pSprite->pev->velocity.z = 40;
			pSprite->SetScale( RANDOM_FLOAT(0.1,1) );
			pSprite->AnimateAndDie(10);
			//pSprite->SetNextThink( 3 );
			//pSprite->SetThink(&CSprite:: SUB_Remove );
			pSprite->SetTransparency( kRenderTransAlpha, (fl_health_CAR_MODEL_FRONT-100)*2, (fl_health_CAR_MODEL_FRONT-100)*2, (fl_health_CAR_MODEL_FRONT-100)*2, 255, kRenderFxNone );
		}
	}
	else
	{
		CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", vecPosition - vecCurrentUp.Normalize()*12, TRUE );

		pSprite->SetTransparency( kRenderTransAlpha, 100, 100, 100, 255, kRenderFxNone );
		pSprite->SetScale( RANDOM_FLOAT(1,2) );
		pSprite->Expand( 5 , RANDOM_FLOAT( 30.0, 60.0 )   );

		pSprite->pev->avelocity.x = RANDOM_FLOAT( -50, 50 );//rotar?
		pSprite->pev->avelocity.y = RANDOM_FLOAT( -50, 50 );
		pSprite->pev->avelocity.z = RANDOM_FLOAT( -50, 50 );

		pSprite->pev->velocity.z = RANDOM_FLOAT( 10, 30 );
		pSprite->pev->velocity.x = RANDOM_FLOAT( -10, 10 );
		pSprite->pev->velocity.y = RANDOM_FLOAT( -10, 10 );
	}
}

void CEnvCar :: DamageLocation(int location, float flDamage)
{/*
	ALERT(at_notice, "Damage, Health vorne ist %f, Health hinten ist %f, Damage ist %f\n",fl_health_CAR_MODEL_FRONT,fl_health_CAR_MODEL_BACK,flDamage);
	
	fl_health -= flDamage;//(1000hp)
	if (location == CAR_MODEL_FRONT)
	{
		fl_health_CAR_MODEL_FRONT -= flDamage;//(3x100hp)
		if (fl_health_CAR_MODEL_FRONT < 100)
			pModel->SetBodygroup( CAR_MODEL_FRONT, 2 );
		else if (fl_health_CAR_MODEL_FRONT < 200)
			pModel->SetBodygroup( CAR_MODEL_FRONT, 1 );
	}
	else if (location == CAR_MODEL_BACK)
	{
		fl_health_CAR_MODEL_BACK -= flDamage;//(3x100hp)
		if (fl_health_CAR_MODEL_BACK < 100)
			pModel->SetBodygroup( CAR_MODEL_BACK, 2 );
		else if (fl_health_CAR_MODEL_BACK < 200)
			pModel->SetBodygroup( CAR_MODEL_BACK, 1 );
	}*/
}


void CEnvCar :: Damage(Vector vecDamagePos, float flDamage)
{

}

void CEnvCar :: DamageAll(float flDamage)
{
	if (flDamage > RANDOM_FLOAT(50,200))
	{

	}
}


int CEnvCar :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
//	ALERT(at_notice, "Damage, Health front is %f, Health back is %f, Damage is %f\n",fl_health_CAR_MODEL_FRONT,fl_health_CAR_MODEL_BACK,flDamage);

	fl_health -= flDamage;
	fl_health_CAR_MODEL_BACK -= flDamage;
	fl_health_CAR_MODEL_FRONT -= flDamage;
	return 1;

	if ((bitsDamageType == DMG_BLAST) && (flDamage > 50))
	{
		DamageAll(flDamage);
		return 1;
	}

	Vector vecAttackDir;
	TraceResult tr;
	
	UTIL_MakeVectorsPrivate(pevAttacker->angles,vecAttackDir,NULL,NULL);

	UTIL_TraceLine(pevAttacker->origin,pevAttacker->origin+vecAttackDir*4096,dont_ignore_monsters,dont_ignore_glass, NULL, &tr);

	if (tr.pHit == edict())
	{
		Damage(tr.vecEndPos, flDamage);
	}

	return 1;
}


void CEnvCar :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	//ALERT (at_notice, "Car used\n");

	if (pActivator && pCaller)
	{
		if ( (pActivator == pCaller) && (FClassnameIs(pActivator->pev, "player")) )
		{
			switch ((int)value)
			{
			case 0:
				PlayerUseForward();
				break;
			case 1:
				PlayerUseBackward();
				break;
			case 2:
				PlayerUseLeft();
				break;
			case 3:
				PlayerUseRight();
				break;
			case 4:
				PlayerUseBreak();
				break;
			case 5:
				PlayerUseHorn();
				break;
			case 6:
				PlayerUseExit();
				break;
			case 7:
				PlayerUseLeftOff();
				break;
			case 8:
				PlayerUseRightOff();
				break;
			}
		}
	}
}

void CEnvCar :: IdleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	Use( pActivator, pCaller, useType, value );
}



void CEnvCar :: StopThink(void)
{
	pev->velocity = Vector(0,0,0);
	/*
	SetThink(NULL);
	SetUse(&CEnvCar::IdleUse);
	*/
	DrawLines();

	pev->nextthink = gpGlobals->time + 0.1;
	SetNextThink( 0.1 );
}

int CEnvCar :: AlreadyThere(Vector vecPos1, Vector vecPos2)
{
	if ( (vecPos1-vecPos2).Length() < 4)
		return 1;
	return 0;
}

void CEnvCar :: GetRoute(void)
{
	ALERT(at_notice, "GetRoute\n");
	if (bRouteDone) //we have a route already
		return;

	ALERT(at_notice, "Getting Route");

	if (!pCurrentTarget)//we don't have a target to drive at
	{
		while ((pCurrentTarget = UTIL_FindEntityInSphere( pCurrentTarget, pev->origin, 1024 )) != NULL)
		{
			if (FClassnameIs( pCurrentTarget->pev, "car_track"))
				break; //we found a track
		}
	}

	if (!pCurrentTarget) //there is no track to drive at, we have to stop
	{
		ALERT(at_notice, "No Track");
		SetThink(&CEnvCar::StopThink);
		pev->nextthink = gpGlobals->time + 0.1;
		SetNextThink( 0.1 );
		bRouteDone = FALSE;
		return;
	}

	if (pCurrentTarget)
	{
		if (!FClassnameIs( pCurrentTarget->pev, "car_track"))
		{
			//ALERT(at_notice, "Target is No Track");
			pCurrentTarget = NULL;
			SetThink(&CEnvCar::StopThink);
			pev->nextthink = gpGlobals->time + 0.1;
			SetNextThink( 0.1 );
			bRouteDone = FALSE;
			return;
		}

		ALERT(at_notice, "Track found");
		pNextTarget = UTIL_FindEntityByTargetname (NULL, STRING(pCurrentTarget->pev->target) );
		if (pNextTarget)
			pNextNextTarget = UTIL_FindEntityByTargetname (NULL, STRING(pNextTarget->pev->target) );
		if (pNextNextTarget)
			pNextNextNextTarget = UTIL_FindEntityByTargetname (NULL, STRING(pNextNextTarget->pev->target) );
	}

	if (AlreadyThere(pCurrentTarget->pev->origin,pev->origin)) //we are already there, take next track
	{
		ALERT(at_notice, "Already There\n");
		pev->origin = pCurrentTarget->pev->origin;
		pForTarget = pCurrentTarget;
		pCurrentTarget = pNextTarget;
		pNextTarget = pNextNextTarget;
		pNextNextTarget = UTIL_FindEntityByTargetname (NULL, STRING(pNextTarget->pev->target) );
	}

	if (pCurrentTarget)
		Pos1 = pCurrentTarget->pev->origin;
	if (pNextTarget)
		Pos2 = pNextTarget->pev->origin;
	if (pNextNextTarget)
		Pos3 = pNextNextTarget->pev->origin;
	if (pNextNextNextTarget)
		Pos4 = pNextNextNextTarget->pev->origin;

	vecDirection = Pos1 - pev->origin;

	//vecDirection.Normalize();

	ALERT(at_notice, "Route done");
	bRouteDone = TRUE;
}

void CEnvCar :: DrawLines(void)
{
	//ALERT(at_notice, "DrawLines\n");
	if (Pos1.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );

		WRITE_COORD( Pos1.x );
		WRITE_COORD( Pos1.y );
		WRITE_COORD( Pos1.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 255 );		// particle colour
	MESSAGE_END();
	}


	if (Pos1.Length() && Pos2.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( Pos1.x );
		WRITE_COORD( Pos1.y );
		WRITE_COORD( Pos1.z );

		WRITE_COORD( Pos2.x );
		WRITE_COORD( Pos2.y );
		WRITE_COORD( Pos2.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 120 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 255 );		// particle colour
	MESSAGE_END();
	}


	if (Pos2.Length() && Pos3.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( Pos2.x );
		WRITE_COORD( Pos2.y );
		WRITE_COORD( Pos2.z );

		WRITE_COORD( Pos3.x );
		WRITE_COORD( Pos3.y );
		WRITE_COORD( Pos3.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 255 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 150 );		// particle colour
	MESSAGE_END();
	}


	if (Pos3.Length() && Pos4.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( Pos3.x );
		WRITE_COORD( Pos3.y );
		WRITE_COORD( Pos3.z );

		WRITE_COORD( Pos4.x );
		WRITE_COORD( Pos4.y );
		WRITE_COORD( Pos4.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 255 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
	MESSAGE_END();
	}


	if (Pos4.Length() && Pos5.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( Pos4.x );
		WRITE_COORD( Pos4.y );
		WRITE_COORD( Pos4.z );

		WRITE_COORD( Pos5.x );
		WRITE_COORD( Pos5.y );
		WRITE_COORD( Pos5.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 255 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
	MESSAGE_END();
	}


	if (Pos5.Length() && Pos6.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( Pos5.x );
		WRITE_COORD( Pos5.y );
		WRITE_COORD( Pos5.z );

		WRITE_COORD( Pos6.x );
		WRITE_COORD( Pos6.y );
		WRITE_COORD( Pos6.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 255 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
	MESSAGE_END();
	}


	if (Pos6.Length() && Pos7.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( Pos6.x );
		WRITE_COORD( Pos6.y );
		WRITE_COORD( Pos6.z );

		WRITE_COORD( Pos7.x );
		WRITE_COORD( Pos7.y );
		WRITE_COORD( Pos7.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 255 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
	MESSAGE_END();
	}


	if (Pos7.Length() && Pos8.Length())
	{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LINE );

		WRITE_COORD( Pos7.x );
		WRITE_COORD( Pos7.y );
		WRITE_COORD( Pos7.z );

		WRITE_COORD( Pos8.x );
		WRITE_COORD( Pos8.y );
		WRITE_COORD( Pos8.z );

		WRITE_SHORT( 1 );
		WRITE_BYTE( 255 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
		WRITE_BYTE( 0 );		// particle colour
	MESSAGE_END();
	}

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos1.x );	//position
		WRITE_COORD( Pos1.y );
		WRITE_COORD( Pos1.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos2.x );	//position
		WRITE_COORD( Pos2.y );
		WRITE_COORD( Pos2.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos3.x );	//position
		WRITE_COORD( Pos3.y );
		WRITE_COORD( Pos3.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos4.x );	//position
		WRITE_COORD( Pos4.y );
		WRITE_COORD( Pos4.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos5.x );	//position
		WRITE_COORD( Pos5.y );
		WRITE_COORD( Pos5.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos6.x );	//position
		WRITE_COORD( Pos6.y );
		WRITE_COORD( Pos6.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos7.x );	//position
		WRITE_COORD( Pos7.y );
		WRITE_COORD( Pos7.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos8.x );	//position
		WRITE_COORD( Pos8.y );
		WRITE_COORD( Pos8.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos9.x );	//position
		WRITE_COORD( Pos9.y );
		WRITE_COORD( Pos9.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos10.x );	//position
		WRITE_COORD( Pos10.y );
		WRITE_COORD( Pos10.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos11.x );	//position
		WRITE_COORD( Pos11.y );
		WRITE_COORD( Pos11.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos12.x );	//position
		WRITE_COORD( Pos12.y );
		WRITE_COORD( Pos12.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos13.x );	//position
		WRITE_COORD( Pos13.y );
		WRITE_COORD( Pos13.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos14.x );	//position
		WRITE_COORD( Pos14.y );
		WRITE_COORD( Pos14.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos15.x );	//position
		WRITE_COORD( Pos15.y );
		WRITE_COORD( Pos15.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos16.x );	//position
		WRITE_COORD( Pos16.y );
		WRITE_COORD( Pos16.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos17.x );	//position
		WRITE_COORD( Pos17.y );
		WRITE_COORD( Pos17.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SPRITE );

		WRITE_COORD( Pos18.x );	//position
		WRITE_COORD( Pos18.y );
		WRITE_COORD( Pos18.z );

		WRITE_SHORT( iSprite );		//index
		WRITE_BYTE( 1 );		// scale * 0.1
		WRITE_BYTE( 255 );		// brightness
	MESSAGE_END();

}

void CEnvCar :: StartThink(void)
{
	//ALERT(at_notice, "StartThink\n");
	GetRoute();

	if (!bRouteDone)
		return;

	countObstacleRout = 0;
	ObstacleRoute();

	if (!bRouteDone)
		return;

	if (AlreadyThere(Pos1, pev->origin))
		NewRouteStart(Pos2, 1, FALSE);

	if (!bRouteDone)
		return;

	if (checkrouteallowed)
	{
		CheckRoute();

		if (Pos3.Length())
			CheckRoutePos1();
		if (Pos4.Length())
			CheckRoutePos2();
		if (Pos5.Length())
			CheckRoutePos3();
		if (Pos6.Length())
			CheckRoutePos4();
		if (Pos7.Length())
			CheckRoutePos5();
		if (Pos8.Length())
			CheckRoutePos6();
		if (Pos9.Length())
			CheckRoutePos7();
		if (Pos10.Length())
			CheckRoutePos8();
		if (Pos11.Length())
			CheckRoutePos9();
		if (Pos12.Length())
			CheckRoutePos10();
		if (Pos13.Length())
			CheckRoutePos11();
		if (Pos14.Length())
			CheckRoutePos12();
		if (Pos15.Length())
			CheckRoutePos13();
		if (Pos16.Length())
			CheckRoutePos14();
		if (Pos17.Length())
			CheckRoutePos15();
		if (Pos18.Length())
			CheckRoutePos16();
	}

	DrawLines();

	DriveRoute();

	pev->nextthink = gpGlobals->time + 0.1;
	SetNextThink( 0.1 );
}

void CEnvCar :: DriveRoute(void)
{
	if (pev->avelocity.Length())
	{
	if (pev->angles.x > 360)
		pev->angles.x -=360;
	if (pev->angles.y > 360)
		pev->angles.y -=360;
	if (pev->angles.z > 360)
		pev->angles.z -=360;
	
	if (pev->angles.x < 0)
		pev->angles.x +=360;
	if (pev->angles.y < 0)
		pev->angles.y +=360;
	if (pev->angles.z < 0)
		pev->angles.z +=360;
	}

	Vector vecAngles = UTIL_VecToAngles(Pos1-pev->origin);

	if (!AlreadyThere(pev->angles, vecAngles))
	{
		Vector vecDestDelta = vecAngles - pev->angles;
		float flTravelTime = vecDestDelta.Length() / pev->velocity.Length();
		pev->avelocity = vecDestDelta / flTravelTime;
	}
	else
	{
		pev->avelocity = Vector (0,0,0);
		pev->angles = vecAngles;
	}
	Vector vecVelocity;

	UTIL_MakeVectorsPrivate(pev->angles, vecVelocity, NULL, NULL);

	pev->velocity = vecVelocity.Normalize()*20;//cars only drive forward, not to the side(Pos1 - pev->origin).Normalize()*20;
}

int CEnvCar :: CalcCheckRoute(Vector vecPos1, Vector vecPos2)
{
	TraceResult tr;
	UTIL_TraceLine(vecPos1, vecPos2, dont_ignore_monsters, dont_ignore_glass, ENT( pev ), &tr);

	if (tr.flFraction == 1.0)
	{
		return 1; //nothing hit, possible way
	}

	if (tr.pHit) //there is an entity
	{
		CBaseEntity *pHitEnt = CBaseEntity::Instance(tr.pHit);
		if ( (pHitEnt->pev->solid == SOLID_NOT) || (pHitEnt->pev->solid == SOLID_TRIGGER) ) //not solid entity, can be ignored
		{
			return 1;
		}
	}
	return 0;
}

void CEnvCar :: CheckRoutePos16(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos16,Pos18))
	{
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos15(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos15,Pos18))
	{
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos15,Pos17))
	{
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos14(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos14,Pos18))
	{
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos14,Pos17))
	{
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos14,Pos16))
	{
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos13(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos13,Pos18))
	{
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos13,Pos17))
	{
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos13,Pos16))
	{
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos13,Pos15))
	{
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos12(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos12,Pos18))
	{
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos12,Pos17))
	{
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos12,Pos16))
	{
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos12,Pos15))
	{
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos12,Pos14))
	{
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos11(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos11,Pos18))
	{
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos11,Pos17))
	{
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos11,Pos16))
	{
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos11,Pos15))
	{
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos11,Pos14))
	{
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos11,Pos13))
	{
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos10(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos10,Pos18))
	{
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos10,Pos17))
	{
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos10,Pos16))
	{
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos10,Pos15))
	{
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos10,Pos14))
	{
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos10,Pos13))
	{
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos10,Pos12))
	{
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos9(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos9,Pos18))
	{
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos9,Pos17))
	{
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos9,Pos16))
	{
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos9,Pos15))
	{
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos9,Pos14))
	{
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos9,Pos13))
	{
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos9,Pos12))
	{
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos9,Pos11))
	{
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos8(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos8,Pos18))
	{
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos8,Pos17))
	{
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos8,Pos16))
	{
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos8,Pos15))
	{
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos8,Pos14))
	{
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos8,Pos13))
	{
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos8,Pos12))
	{
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos8,Pos11))
	{
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos8,Pos10))
	{
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos7(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos7,Pos18))
	{
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos7,Pos17))
	{
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos7,Pos16))
	{
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos7,Pos15))
	{
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos7,Pos14))
	{
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos7,Pos13))
	{
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos7,Pos12))
	{
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos7,Pos11))
	{
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos7,Pos10))
	{
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(Pos7,Pos9))
	{
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos6(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos6,Pos18))
	{
		Pos7 = Pos18;
		Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos6,Pos17))
	{
		Pos7 = Pos17;
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos6,Pos16))
	{
		Pos7 = Pos16;
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos6,Pos15))
	{
		Pos7 = Pos15;
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos6,Pos14))
	{
		Pos7 = Pos14;
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos6,Pos13))
	{
		Pos7 = Pos13;
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos6,Pos12))
	{
		Pos7 = Pos12;
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos6,Pos11))
	{
		Pos7 = Pos11;
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos6,Pos10))
	{
		Pos7 = Pos10;
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(Pos6,Pos9))
	{
		Pos7 = Pos9;
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos8.Length() && CalcCheckRoute(Pos6,Pos8))
	{
		Pos7 = Pos8;
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos5(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos5,Pos18))
	{
		Pos6 = Pos18;
		Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos5,Pos17))
	{
		Pos6 = Pos17;
		Pos7 = Pos18;
		Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos5,Pos16))
	{
		Pos6 = Pos16;
		Pos7 = Pos17;
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos5,Pos15))
	{
		Pos6 = Pos15;
		Pos7 = Pos16;
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos5,Pos14))
	{
		Pos6 = Pos14;
		Pos7 = Pos15;
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos5,Pos13))
	{
		Pos6 = Pos13;
		Pos7 = Pos14;
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos5,Pos12))
	{
		Pos6 = Pos12;
		Pos7 = Pos13;
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos5,Pos11))
	{
		Pos6 = Pos11;
		Pos7 = Pos12;
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos5,Pos10))
	{
		Pos6 = Pos10;
		Pos7 = Pos11;
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(Pos5,Pos9))
	{
		Pos6 = Pos9;
		Pos7 = Pos10;
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos8.Length() && CalcCheckRoute(Pos5,Pos8))
	{
		Pos6 = Pos8;
		Pos7 = Pos9;
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos7.Length() && CalcCheckRoute(Pos5,Pos7))
	{
		Pos6 = Pos7;
		Pos7 = Pos8;
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos4(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos4,Pos18))
	{
		Pos5 = Pos18;
		Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos4,Pos17))
	{
		Pos5 = Pos17;
		Pos6 = Pos18;
		Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos4,Pos16))
	{
		Pos5 = Pos16;
		Pos6 = Pos17;
		Pos7 = Pos18;
		Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos4,Pos15))
	{
		Pos5 = Pos15;
		Pos6 = Pos16;
		Pos7 = Pos17;
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos4,Pos14))
	{
		Pos5 = Pos14;
		Pos6 = Pos15;
		Pos7 = Pos16;
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos4,Pos13))
	{
		Pos5 = Pos13;
		Pos6 = Pos14;
		Pos7 = Pos15;
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos4,Pos12))
	{
		Pos5 = Pos12;
		Pos6 = Pos13;
		Pos7 = Pos14;
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos4,Pos11))
	{
		Pos5 = Pos11;
		Pos6 = Pos12;
		Pos7 = Pos13;
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos4,Pos10))
	{
		Pos5 = Pos10;
		Pos6 = Pos11;
		Pos7 = Pos12;
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(Pos4,Pos9))
	{
		Pos5 = Pos9;
		Pos6 = Pos10;
		Pos7 = Pos11;
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos8.Length() && CalcCheckRoute(Pos4,Pos8))
	{
		Pos5 = Pos8;
		Pos6 = Pos9;
		Pos7 = Pos10;
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos7.Length() && CalcCheckRoute(Pos4,Pos7))
	{
		Pos5 = Pos7;
		Pos6 = Pos8;
		Pos7 = Pos9;
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos6.Length() && CalcCheckRoute(Pos4,Pos6))
	{
		Pos5 = Pos6;
		Pos6 = Pos7;
		Pos7 = Pos8;
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos3(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos3,Pos18))
	{
		Pos4 = Pos18;
		Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos3,Pos17))
	{
		Pos4 = Pos17;
		Pos5 = Pos18;
		Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos3,Pos16))
	{
		Pos4 = Pos16;
		Pos5 = Pos17;
		Pos6 = Pos18;
		Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos3,Pos15))
	{
		Pos4 = Pos15;
		Pos5 = Pos16;
		Pos6 = Pos17;
		Pos7 = Pos18;
		Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos3,Pos14))
	{
		Pos4 = Pos14;
		Pos5 = Pos15;
		Pos6 = Pos16;
		Pos7 = Pos17;
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos3,Pos13))
	{
		Pos4 = Pos13;
		Pos5 = Pos14;
		Pos6 = Pos15;
		Pos7 = Pos16;
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos3,Pos12))
	{
		Pos4 = Pos12;
		Pos5 = Pos13;
		Pos6 = Pos14;
		Pos7 = Pos15;
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos3,Pos11))
	{
		Pos4 = Pos11;
		Pos5 = Pos12;
		Pos6 = Pos13;
		Pos7 = Pos14;
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos3,Pos10))
	{
		Pos4 = Pos10;
		Pos5 = Pos11;
		Pos6 = Pos12;
		Pos7 = Pos13;
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(Pos3,Pos9))
	{
		Pos4 = Pos9;
		Pos5 = Pos10;
		Pos6 = Pos11;
		Pos7 = Pos12;
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos8.Length() && CalcCheckRoute(Pos3,Pos8))
	{
		Pos4 = Pos8;
		Pos5 = Pos9;
		Pos6 = Pos10;
		Pos7 = Pos11;
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos7.Length() && CalcCheckRoute(Pos3,Pos7))
	{
		Pos4 = Pos7;
		Pos5 = Pos8;
		Pos6 = Pos9;
		Pos7 = Pos10;
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos6.Length() && CalcCheckRoute(Pos3,Pos6))
	{
		Pos4 = Pos6;
		Pos5 = Pos7;
		Pos6 = Pos8;
		Pos7 = Pos9;
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos5.Length() && CalcCheckRoute(Pos3,Pos5))
	{
		Pos4 = Pos5;
		Pos5 = Pos6;
		Pos6 = Pos7;
		Pos7 = Pos8;
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos2(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos2,Pos18))
	{//this is most unlikely, but not impossible
		Pos3 = Pos18;
		Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos2,Pos17))
	{
		Pos3 = Pos17;
		Pos4 = Pos18;
		Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos2,Pos16))
	{
		Pos3 = Pos16;
		Pos4 = Pos17;
		Pos5 = Pos18;
		Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos2,Pos15))
	{
		Pos3 = Pos15;
		Pos4 = Pos16;
		Pos5 = Pos17;
		Pos6 = Pos18;
		Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos2,Pos14))
	{
		Pos3 = Pos14;
		Pos4 = Pos15;
		Pos5 = Pos16;
		Pos6 = Pos17;
		Pos7 = Pos18;
		Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos2,Pos13))
	{
		Pos3 = Pos13;
		Pos4 = Pos14;
		Pos5 = Pos15;
		Pos6 = Pos16;
		Pos7 = Pos17;
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos2,Pos12))
	{
		Pos3 = Pos12;
		Pos4 = Pos13;
		Pos5 = Pos14;
		Pos6 = Pos15;
		Pos7 = Pos16;
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos2,Pos11))
	{
		Pos3 = Pos11;
		Pos4 = Pos12;
		Pos5 = Pos13;
		Pos6 = Pos14;
		Pos7 = Pos15;
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos2,Pos10))
	{
		Pos3 = Pos10;
		Pos4 = Pos11;
		Pos5 = Pos12;
		Pos6 = Pos13;
		Pos7 = Pos14;
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(Pos2,Pos9))
	{
		Pos3 = Pos9;
		Pos4 = Pos10;
		Pos5 = Pos11;
		Pos6 = Pos12;
		Pos7 = Pos13;
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos8.Length() && CalcCheckRoute(Pos2,Pos8))
	{
		Pos3 = Pos8;
		Pos4 = Pos9;
		Pos5 = Pos10;
		Pos6 = Pos11;
		Pos7 = Pos12;
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos7.Length() && CalcCheckRoute(Pos2,Pos7))
	{
		Pos3 = Pos7;
		Pos4 = Pos8;
		Pos5 = Pos9;
		Pos6 = Pos10;
		Pos7 = Pos11;
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos6.Length() && CalcCheckRoute(Pos2,Pos6))
	{
		Pos3 = Pos6;
		Pos4 = Pos7;
		Pos5 = Pos8;
		Pos6 = Pos9;
		Pos7 = Pos10;
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos5.Length() && CalcCheckRoute(Pos2,Pos5))
	{
		Pos3 = Pos5;
		Pos4 = Pos6;
		Pos5 = Pos7;
		Pos6 = Pos8;
		Pos7 = Pos9;
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos4.Length() && CalcCheckRoute(Pos2,Pos4))
	{
		Pos3 = Pos4;
		Pos4 = Pos5;
		Pos5 = Pos6;
		Pos6 = Pos7;
		Pos7 = Pos8;
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoutePos1(void)
{
	if (Pos18.Length() && CalcCheckRoute(Pos1,Pos18))
	{//this is most unlikely, but not impossible
		Pos2 = Pos18;
		Pos3 = Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(Pos1,Pos17))
	{
		Pos2 = Pos17;
		Pos3 = Pos18;
		Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(Pos1,Pos16))
	{
		Pos2 = Pos16;
		Pos3 = Pos17;
		Pos4 = Pos18;
		Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(Pos1,Pos15))
	{
		Pos2 = Pos15;
		Pos3 = Pos16;
		Pos4 = Pos17;
		Pos5 = Pos18;
		Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(Pos1,Pos14))
	{
		Pos2 = Pos14;
		Pos3 = Pos15;
		Pos4 = Pos16;
		Pos5 = Pos17;
		Pos6 = Pos18;
		Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(Pos1,Pos13))
	{
		Pos2 = Pos13;
		Pos3 = Pos14;
		Pos4 = Pos15;
		Pos5 = Pos16;
		Pos6 = Pos17;
		Pos7 = Pos18;
		Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(Pos1,Pos12))
	{
		Pos2 = Pos12;
		Pos3 = Pos13;
		Pos4 = Pos14;
		Pos5 = Pos15;
		Pos6 = Pos16;
		Pos7 = Pos17;
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(Pos1,Pos11))
	{
		Pos2 = Pos11;
		Pos3 = Pos12;
		Pos4 = Pos13;
		Pos5 = Pos14;
		Pos6 = Pos15;
		Pos7 = Pos16;
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(Pos1,Pos10))
	{
		Pos2 = Pos10;
		Pos3 = Pos11;
		Pos4 = Pos12;
		Pos5 = Pos13;
		Pos6 = Pos14;
		Pos7 = Pos15;
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(Pos1,Pos9))
	{
		Pos2 = Pos9;
		Pos3 = Pos10;
		Pos4 = Pos11;
		Pos5 = Pos12;
		Pos6 = Pos13;
		Pos7 = Pos14;
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos8.Length() && CalcCheckRoute(Pos1,Pos8))
	{
		Pos2 = Pos8;
		Pos3 = Pos9;
		Pos4 = Pos10;
		Pos5 = Pos11;
		Pos6 = Pos12;
		Pos7 = Pos13;
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos7.Length() && CalcCheckRoute(Pos1,Pos7))
	{
		Pos2 = Pos7;
		Pos3 = Pos8;
		Pos4 = Pos9;
		Pos5 = Pos10;
		Pos6 = Pos11;
		Pos7 = Pos12;
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos6.Length() && CalcCheckRoute(Pos1,Pos6))
	{
		Pos2 = Pos6;
		Pos3 = Pos7;
		Pos4 = Pos8;
		Pos5 = Pos9;
		Pos6 = Pos10;
		Pos7 = Pos11;
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos5.Length() && CalcCheckRoute(Pos1,Pos5))
	{
		Pos2 = Pos5;
		Pos3 = Pos6;
		Pos4 = Pos7;
		Pos5 = Pos8;
		Pos6 = Pos9;
		Pos7 = Pos10;
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos4.Length() && CalcCheckRoute(Pos1,Pos4))
	{
		Pos2 = Pos4;
		Pos3 = Pos5;
		Pos4 = Pos6;
		Pos5 = Pos7;
		Pos6 = Pos8;
		Pos7 = Pos9;
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos3.Length() && CalcCheckRoute(Pos1,Pos3))
	{
		Pos2 = Pos3;
		Pos3 = Pos4;
		Pos4 = Pos5;
		Pos5 = Pos6;
		Pos6 = Pos7;
		Pos7 = Pos8;
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: CheckRoute(void)
{
	if (Pos18.Length() && CalcCheckRoute(pev->origin,Pos18))
	{//this is most unlikely, but not impossible
		Pos1 = Pos18;
		Pos2 = Pos3 = Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos17.Length() && CalcCheckRoute(pev->origin,Pos17))
	{
		Pos1 = Pos17;
		Pos2 = Pos18;
		Pos3 = Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos16.Length() && CalcCheckRoute(pev->origin,Pos16))
	{
		Pos1 = Pos16;
		Pos2 = Pos17;
		Pos3 = Pos18;
		Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos15.Length() && CalcCheckRoute(pev->origin,Pos15))
	{
		Pos1 = Pos15;
		Pos2 = Pos16;
		Pos3 = Pos17;
		Pos4 = Pos18;
		Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos14.Length() && CalcCheckRoute(pev->origin,Pos14))
	{
		Pos1 = Pos14;
		Pos2 = Pos15;
		Pos3 = Pos16;
		Pos4 = Pos17;
		Pos5 = Pos18;
		Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos13.Length() && CalcCheckRoute(pev->origin,Pos13))
	{
		Pos1 = Pos13;
		Pos2 = Pos14;
		Pos3 = Pos15;
		Pos4 = Pos16;
		Pos5 = Pos17;
		Pos6 = Pos18;
		Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos12.Length() && CalcCheckRoute(pev->origin,Pos12))
	{
		Pos1 = Pos12;
		Pos2 = Pos13;
		Pos3 = Pos14;
		Pos4 = Pos15;
		Pos5 = Pos16;
		Pos6 = Pos17;
		Pos7 = Pos18;
		Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos11.Length() && CalcCheckRoute(pev->origin,Pos11))
	{
		Pos1 = Pos11;
		Pos2 = Pos12;
		Pos3 = Pos13;
		Pos4 = Pos14;
		Pos5 = Pos15;
		Pos6 = Pos16;
		Pos7 = Pos17;
		Pos8 = Pos18;
		Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos10.Length() && CalcCheckRoute(pev->origin,Pos10))
	{
		Pos1 = Pos10;
		Pos2 = Pos11;
		Pos3 = Pos12;
		Pos4 = Pos13;
		Pos5 = Pos14;
		Pos6 = Pos15;
		Pos7 = Pos16;
		Pos8 = Pos17;
		Pos9 = Pos18;
		Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos9.Length() && CalcCheckRoute(pev->origin,Pos9))
	{
		Pos1 = Pos9;
		Pos2 = Pos10;
		Pos3 = Pos11;
		Pos4 = Pos12;
		Pos5 = Pos13;
		Pos6 = Pos14;
		Pos7 = Pos15;
		Pos8 = Pos16;
		Pos9 = Pos17;
		Pos10 = Pos18;
		Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos8.Length() && CalcCheckRoute(pev->origin,Pos8))
	{
		Pos1 = Pos8;
		Pos2 = Pos9;
		Pos3 = Pos10;
		Pos4 = Pos11;
		Pos5 = Pos12;
		Pos6 = Pos13;
		Pos7 = Pos14;
		Pos8 = Pos15;
		Pos9 = Pos16;
		Pos10 = Pos17;
		Pos11 = Pos18;
		Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos7.Length() && CalcCheckRoute(pev->origin,Pos7))
	{
		Pos1 = Pos7;
		Pos2 = Pos8;
		Pos3 = Pos9;
		Pos4 = Pos10;
		Pos5 = Pos11;
		Pos6 = Pos12;
		Pos7 = Pos13;
		Pos8 = Pos14;
		Pos9 = Pos15;
		Pos10 = Pos16;
		Pos11 = Pos17;
		Pos12 = Pos18;
		Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos6.Length() && CalcCheckRoute(pev->origin,Pos6))
	{
		Pos1 = Pos6;
		Pos2 = Pos7;
		Pos3 = Pos8;
		Pos4 = Pos9;
		Pos5 = Pos10;
		Pos6 = Pos11;
		Pos7 = Pos12;
		Pos8 = Pos13;
		Pos9 = Pos14;
		Pos10 = Pos15;
		Pos11 = Pos16;
		Pos12 = Pos17;
		Pos13 = Pos18;
		Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos5.Length() && CalcCheckRoute(pev->origin,Pos5))
	{
		Pos1 = Pos5;
		Pos2 = Pos6;
		Pos3 = Pos7;
		Pos4 = Pos8;
		Pos5 = Pos9;
		Pos6 = Pos10;
		Pos7 = Pos11;
		Pos8 = Pos12;
		Pos9 = Pos13;
		Pos10 = Pos14;
		Pos11 = Pos15;
		Pos12 = Pos16;
		Pos13 = Pos17;
		Pos14 = Pos18;
		Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos4.Length() && CalcCheckRoute(pev->origin,Pos4))
	{
		Pos1 = Pos4;
		Pos2 = Pos5;
		Pos3 = Pos6;
		Pos4 = Pos7;
		Pos5 = Pos8;
		Pos6 = Pos9;
		Pos7 = Pos10;
		Pos8 = Pos11;
		Pos9 = Pos12;
		Pos10 = Pos13;
		Pos11 = Pos14;
		Pos12 = Pos15;
		Pos13 = Pos16;
		Pos14 = Pos17;
		Pos15 = Pos18;
		Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos3.Length() && CalcCheckRoute(pev->origin,Pos3))
	{
		Pos1 = Pos3;
		Pos2 = Pos4;
		Pos3 = Pos5;
		Pos4 = Pos6;
		Pos5 = Pos7;
		Pos6 = Pos8;
		Pos7 = Pos9;
		Pos8 = Pos10;
		Pos9 = Pos11;
		Pos10 = Pos12;
		Pos11 = Pos13;
		Pos12 = Pos14;
		Pos13 = Pos15;
		Pos14 = Pos16;
		Pos15 = Pos17;
		Pos16 = Pos18;
		Pos17 = Pos18 = Vector(0,0,0);
	}
	if (Pos2.Length() && CalcCheckRoute(pev->origin,Pos2))
	{
		Pos1 = Pos2;
		Pos2 = Pos3;
		Pos3 = Pos4;
		Pos4 = Pos5;
		Pos5 = Pos6;
		Pos6 = Pos7;
		Pos7 = Pos8;
		Pos8 = Pos9;
		Pos9 = Pos10;
		Pos10 = Pos11;
		Pos11 = Pos12;
		Pos12 = Pos13;
		Pos13 = Pos14;
		Pos14 = Pos15;
		Pos15 = Pos16;
		Pos16 = Pos17;
		Pos17 = Pos18;
		Pos18 = Vector(0,0,0);
	}
}

void CEnvCar :: NewRouteStart(Vector vecPosNew, int countRoute, BOOL bReCheck)
{
	switch (countRoute)
	{
	case 0:
	case 1:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = Pos7;
		Pos7 = Pos6;
		Pos6 = Pos5;
		Pos5 = Pos4;
		Pos4 = Pos3;
		Pos3 = Pos2;
		Pos2 = Pos1;
		Pos1 = vecPosNew;
		break;
	case 2:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = Pos7;
		Pos7 = Pos6;
		Pos6 = Pos5;
		Pos5 = Pos4;
		Pos4 = Pos3;
		Pos3 = Pos2;
		Pos2 = vecPosNew;
		break;
	case 3:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = Pos7;
		Pos7 = Pos6;
		Pos6 = Pos5;
		Pos5 = Pos4;
		Pos4 = Pos3;
		Pos3 = vecPosNew;
		break;
	case 4:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = Pos7;
		Pos7 = Pos6;
		Pos6 = Pos5;
		Pos5 = Pos4;
		Pos4 = vecPosNew;
		break;
	case 5:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = Pos7;
		Pos7 = Pos6;
		Pos6 = Pos5;
		Pos5 = vecPosNew;
		break;
	case 6:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = Pos7;
		Pos7 = Pos6;
		Pos6 = vecPosNew;
		break;
	case 7:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = Pos7;
		Pos7 = vecPosNew;
		break;
	case 8:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = Pos8;
		Pos8 = vecPosNew;
		break;
	case 9:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = Pos9;
		Pos9 = vecPosNew;
		break;
	case 10:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = Pos10;
		Pos10 = vecPosNew;
		break;
	case 11:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = Pos11;
		Pos11 = vecPosNew;
		break;
	case 12:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = Pos12;
		Pos12 = vecPosNew;
		break;
	case 13:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = Pos13;
		Pos13 = vecPosNew;
		break;
	case 14:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = Pos14;
		Pos14 = vecPosNew;
		break;
	case 15:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = Pos15;
		Pos15 = vecPosNew;
		break;
	case 16:
		Pos18 = Pos17;
		Pos17 = Pos16;
		Pos16 = vecPosNew;
		break;
	case 17:
		Pos18 = Pos17;
		Pos17 = vecPosNew;
		break;
	case 18:
		Pos18 = vecPosNew;
		break;
	}

	if (bReCheck)
		ObstacleRoute();//and again
}


//temp function
void CEnvCar :: DrawSprite(Vector vecPoint)
{
	CSprite *pSprite = CSprite::SpriteCreate( "sprites/gargeye1.spr", vecPoint, TRUE );
	pSprite->SetTransparency( kRenderTransAlpha, 0, 0, 255, 255, kRenderFxNone );
	pSprite->SetScale( 0.1 );
}

void CEnvCar :: ObstacleRouteCopy(void)
{
	ObstacleRoute();
}

void CEnvCar :: ObstacleRoute(void)
{
	ALERT(at_notice, "ObstacleRoute\n");

	//this is for multiple checks
	Vector vecCheck;
	Vector vecToCheck;
	BOOL HitStructure;
	TraceResult tr;

//ReturnSpot:

	countObstacleRout += 1;

	if (countObstacleRout > 18) //not more than 18, because we can only save 18 waypoints
	{
		SetThink(&CEnvCar::StopThink);
		pev->nextthink = gpGlobals->time + 0.1;
		SetNextThink( 0.1 );
		bRouteDone = FALSE;
		return;
	}

	switch (countObstacleRout)
	{
	case 0:
	case 1:
		checkrouteallowed = FALSE;
		vecCheck = pev->origin;
		vecToCheck = Pos1;
		break;
	case 2:
		checkrouteallowed = TRUE;
		vecCheck = vecAftercheck;
		vecToCheck = Pos2;
		break;
	case 3:
		vecCheck = vecAftercheck;
		vecToCheck = Pos3;
		break;
	case 4:
		vecCheck = vecAftercheck;
		vecToCheck = Pos4;
		break;
	case 5:
		vecCheck = vecAftercheck;
		vecToCheck = Pos5;
		break;
	case 6:
		vecCheck = vecAftercheck;
		vecToCheck = Pos6;
		break;
	case 7:
		vecCheck = vecAftercheck;
		vecToCheck = Pos7;
		break;
	case 8:
		vecCheck = vecAftercheck;
		vecToCheck = Pos8;
		break;
	case 9:
		vecCheck = vecAftercheck;
		vecToCheck = Pos9;
		break;
	case 10:
		vecCheck = vecAftercheck;
		vecToCheck = Pos10;
		break;
	case 11:
		vecCheck = vecAftercheck;
		vecToCheck = Pos11;
		break;
	case 12:
		vecCheck = vecAftercheck;
		vecToCheck = Pos12;
		break;
	case 13:
		vecCheck = vecAftercheck;
		vecToCheck = Pos13;
		break;
	case 14:
		vecCheck = vecAftercheck;
		vecToCheck = Pos14;
		break;
	case 15:
		vecCheck = vecAftercheck;
		vecToCheck = Pos15;
		break;
	case 16:
		vecCheck = vecAftercheck;
		vecToCheck = Pos16;
		break;
	case 17:
		vecCheck = vecAftercheck;
		vecToCheck = Pos17;
		break;
	case 18:
		vecCheck = vecAftercheck;
		vecToCheck = Pos18;
		break;
	}


	HitStructure = FALSE;

	UTIL_TraceLine(vecCheck, vecToCheck, dont_ignore_monsters, dont_ignore_glass, ENT( pev ), &tr);

	if (tr.flFraction == 1.0)
	{
		//ALERT(at_notice, "Nichts im Weg\n");
		
		if ( (countObstacleRout > 1) && (countObstacleRout <= 18) )// if we are creating a route to our target check next points
		{
			//ObstacleRouteCopy();
			//goto ReturnSpot;
		}

		return; //nothing hit, drive
	}

	if (tr.pHit) //there is an entity
	{
		CBaseEntity *pHitEnt = CBaseEntity::Instance(tr.pHit);
		if ( (pHitEnt->pev->solid == SOLID_NOT) || (pHitEnt->pev->solid == SOLID_TRIGGER) ) //ignore this entity and continue search
		{
			//ALERT(at_notice, "Leeres Entity im Weg, egal\n");
			
			if (countObstacleRout>1)
				NewRouteStart(tr.vecEndPos, countObstacleRout, TRUE);

			return;
		}
		
		if ( (pHitEnt->pev->flags & FL_MONSTER) || (pHitEnt->pev->flags & FL_CLIENT) )//player or npc
		{
			//ALERT(at_notice, "Mensch im Weg\n");
			float curspeed = (pev->velocity).Length();
			float distance = (tr.vecEndPos - vecCheck).Length();
			
			if (curspeed > distance/5)
			{
				//ALERT(at_notice, "Schneller als 5Sekunden da\n");
				SetThink(&CEnvCar::StopThink);
				pev->nextthink = gpGlobals->time + 0.1;
				SetNextThink( 0.1 );
				//bRouteDone = FALSE;
			}
			else
				//ALERT(at_notice, "Langsamer als 5Sekunden da\n");

			if (countObstacleRout>1)
				NewRouteStart(tr.vecEndPos, countObstacleRout, TRUE);

			return;
		}
		else if ( (pHitEnt->pev->solid == SOLID_SLIDEBOX) || (pHitEnt->pev->solid == SOLID_BBOX) || (pHitEnt->pev->solid == SOLID_BSP) ) //a wall or so
		{
			//ALERT(at_notice, "Entitywand im Weg\n");
			//we have to find a way around the obstacle
			HitStructure = TRUE;
		}
	}

	if (HitStructure || tr.vecPlaneNormal.Length() )
	{
		//ALERT(at_notice, "Irgendeine feste Substanz im Weg\n");
		Vector vecDir;
		Vector vecDir2;
		Vector vecDir3;
		float fLength;


		Vector vecTryRight;
		Vector vecTryLeft;

		if (tr.vecPlaneNormal.Length())
			vecDir3 = vecDir2 = vecDir = tr.vecPlaneNormal.Normalize();
		else
			vecDir3 = vecDir2 = vecDir = (tr.vecEndPos - vecCheck).Normalize()*2; //double size for more accuracy (due to some bad angles, a car could try to drive into an obstacle if this is 1)

		vecDir3.z = vecDir2.z = vecDir.z = 0;


		vecDir2.x = cos(M_PI*0.5)*vecDir.x + sin(M_PI*0.5)*vecDir.y;
		vecDir2.y = -sin(M_PI*0.5)*vecDir.x + cos(M_PI*0.5)*vecDir.y;
		vecDir = vecDir2;



		vecTryRight = vecTryLeft = tr.vecEndPos - vecDir3.Normalize()*5;
		
		
		//temp
		CBeam *pLine;
		pLine = CBeam::BeamCreate( "sprites/laserbeam.spr", 2 );
		pLine->PointsInit( vecTryLeft, vecTryLeft+vecDir*100 );
		pLine->SetColor( 0, 255, 0 );
		pLine->SetBrightness( 255 );
		//temp
		CBeam *pLine2;
		pLine2 = CBeam::BeamCreate( "sprites/laserbeam.spr", 2 );
		pLine2->PointsInit( vecTryLeft, vecTryLeft-vecDir*100 );
		pLine2->SetColor( 0, 255, 0 );
		pLine2->SetBrightness( 255 );
		//temp
		CBeam *pLine3;
		pLine3 = CBeam::BeamCreate( "sprites/laserbeam.spr", 2 );
		pLine3->PointsInit( vecTryLeft, vecTryLeft+vecDir3*100 );
		pLine3->SetColor( 255, 0, 0 );
		pLine3->SetBrightness( 255 );
		//temp
		CBeam *pLine4;
		pLine4 = CBeam::BeamCreate( "sprites/laserbeam.spr", 2 );
		pLine4->PointsInit( vecTryLeft, vecTryLeft-vecDir3*100 );
		pLine4->SetColor( 255, 0, 0 );
		pLine4->SetBrightness( 255 );
		//temp



		fLength = (tr.vecEndPos - vecCheck).Length();

		TraceResult tr1;
		TraceResult tr2;
		TraceResult tr3;

		int foundpath;
		foundpath = 0;


			for (int findwayleft=16; findwayleft<4096; findwayleft+=16)
			{
				if ((countObstacleRout < 2) || foundpathright)
				{
					vecTryRight.x += vecDir.x*findwayleft; //check if there is a way on the right
					vecTryRight.y += vecDir.y*findwayleft; //check if there is a way on the right

					UTIL_TraceLine(vecCheck, vecTryRight, dont_ignore_monsters, dont_ignore_glass, ENT( pev ), &tr1);
				
					if (tr1.flFraction == 1.0)
					{
						vecAftercheck = vecTryRight;

						vecAftercheck.x += vecDir.x*findwayleft; //take point one step further to the side, so the car doesnt crash into a wall thats too near
						vecAftercheck.y += vecDir.y*findwayleft; 
						vecAftercheck.x += tr.vecPlaneNormal.Normalize().x*64; //take point 64units away from wall, so the car doesnt crash into a wall thats too near
						vecAftercheck.y += tr.vecPlaneNormal.Normalize().y*64; 

						foundpath = 1;
						foundpathright = TRUE;
						foundpathleft = FALSE;
						break;
					}
					
					if ( (countObstacleRout>2) && (!(tr1.vecPlaneNormal.Normalize() == tr.vecPlaneNormal.Normalize())) ) //we hit a new wall
					{
						countObstacleRout-=1;
						
						NewRouteStart(vecTryRight+(tr1.vecPlaneNormal.Normalize()*-100), countObstacleRout+1, TRUE);//set next waypoint to one behind the new wall, to continue a search
						foundpath = 2;
						foundpathright = TRUE;
						foundpathleft = FALSE;
						break;
					}

				}



				if ((countObstacleRout < 2) || foundpathleft)
				{
					vecTryLeft.x -= vecDir.x*findwayleft; //check if there is a way on the left
					vecTryLeft.y -= vecDir.y*findwayleft; //check if there is a way on the left

					UTIL_TraceLine(vecCheck, vecTryLeft, dont_ignore_monsters, dont_ignore_glass, ENT( pev ), &tr2);

					if (tr2.flFraction == 1.0)
					{
						vecAftercheck = vecTryLeft;

						vecAftercheck.x -= vecDir.x*findwayleft; //take point one step further to the side, so the car doesnt crash into a wall thats too near
						vecAftercheck.y -= vecDir.y*findwayleft; 
						vecAftercheck.x += tr.vecPlaneNormal.Normalize().x*64; //take point 64units away from wall, so the car doesnt crash into a wall thats too near
						vecAftercheck.y += tr.vecPlaneNormal.Normalize().y*64; 

						foundpath = 1;
						foundpathright = FALSE;
						foundpathleft = TRUE;
						break;
					}

					if ( (countObstacleRout>2) && (!(tr2.vecPlaneNormal.Normalize() == tr.vecPlaneNormal.Normalize())) ) //we hit a new wall
					{
						countObstacleRout-=1;

						NewRouteStart(vecTryLeft+(tr2.vecPlaneNormal.Normalize()*-100), countObstacleRout+1, TRUE);//set next waypoint to one behind the new wall, to continue a search
						foundpath = 2;
						foundpathright = FALSE;
						foundpathleft = TRUE;
						break;
					}


				}
			}

		if (!foundpath)
		{
			pCurrentTarget = NULL;
			SetThink(&CEnvCar::StopThink);
			pev->nextthink = gpGlobals->time + 0.1;
			SetNextThink( 0.1 );
			foundpathright = foundpathleft = bRouteDone = FALSE;
			return;
		}
		if (foundpath == 1)
			NewRouteStart(vecAftercheck, countObstacleRout, TRUE);//we found a new point, set new path and continue

	}
	
}

/*
typedef struct
	{
	int		fAllSolid;			// if true, plane is not valid
	int		fStartSolid;		// if true, the initial point was in a solid area
	int		fInOpen;
	int		fInWater;
	float	flFraction;			// time completed, 1.0 = didn't hit anything
	vec3_t	vecEndPos;			// final position
	float	flPlaneDist;
	vec3_t	vecPlaneNormal;		// surface normal at impact
	edict_t	*pHit;				// entity the surface is on
	int		iHitgroup;			// 0 == generic, non zero is specific body part
	} TraceResult;
*/



void CEnvCar :: PlayerUseForward(void)
{
	if (bIsCarFlying)
		return;

	bPlayerExited = FALSE;

	fl_last_time_forward = gpGlobals->time;
//sys
	float fl_acceleration;

	if ( FClassnameIs ( pev, "car_rally" ) )
	{
		fl_acceleration = 20;
	}
	else
	{
		fl_acceleration = 4;//test
	}

	fl_speed += fl_acceleration;
	if (fl_speed > fl_maxspeed)
		fl_speed = fl_maxspeed;
	else if (fl_speed < -fl_maxspeed)
		fl_speed = -fl_maxspeed;


	Vector vecDir;
	UTIL_MakeVectorsPrivate(pev->angles, vecDir, NULL, NULL);
	pev->velocity = vecDir.Normalize()*fl_speed + vecDesiredDir*fl_bouncespeed;
}


void CEnvCar :: PlayerUseBackward(void)
{
	if (bIsCarFlying)
		return;
	
	bPlayerExited = FALSE;

	fl_last_time_backward = gpGlobals->time;

	float fl_deceleration;
	fl_deceleration = 4;


	fl_speed -= fl_deceleration;
	if (fl_speed > fl_maxspeed)
		fl_speed = fl_maxspeed;
	else if (fl_speed < -fl_maxspeed)
		fl_speed = -fl_maxspeed;

	Vector vecDir;
	UTIL_MakeVectorsPrivate(pev->angles, vecDir, NULL, NULL);
	pev->velocity = vecDir.Normalize()*fl_speed + vecDesiredDir*fl_bouncespeed;
}


void CEnvCar :: PlayerUseLeft(void)
{
	pModel->SetBoneController ( 0, 60 );
	wheel_left = TRUE;

	if (bIsCarFlying)
	{
		//PlayerUseLeftOff();
		return;
	}
	
	bPlayerExited = FALSE;

	float fl_value;
	fl_value = 1;

	if ((fl_speed < 0 && fl_last_time_forward > gpGlobals->time - 0.1) || (fl_speed > 0 && fl_last_time_backward > gpGlobals->time - 0.1))
	{
		fl_value = 2;
	}

	if (fl_last_time_break > gpGlobals->time - 0.1)
	{	
		if (fl_last_time_heavybreak > gpGlobals->time)
		{
			fl_value = 6;
		}
		else
			fl_value = 2;
	}

	float fl_maxrotspeed;
	fl_maxrotspeed = pev->velocity.Length();
	if (fl_maxrotspeed > 50*fl_value)
		fl_maxrotspeed = 50*fl_value;

	if (fl_speed > 0)
		pev->avelocity = Vector(0,1,0) * fl_maxrotspeed;
	else if (fl_speed < 0)
		pev->avelocity = Vector(0,1,0) * -fl_maxrotspeed;
}


void CEnvCar :: PlayerUseRight(void)
{
	pModel->SetBoneController ( 0, -60 );
	wheel_right = TRUE;

	if (bIsCarFlying)
	{
		//PlayerUseRightOff();
		return;
	}

	bPlayerExited = FALSE;

	float fl_value;
	fl_value = 1;

	if ((fl_speed < 0 && fl_last_time_forward > gpGlobals->time - 0.1) || (fl_speed > 0 && fl_last_time_backward > gpGlobals->time - 0.1))
	{
		fl_value = 2;
	}

	if (fl_last_time_break > gpGlobals->time - 0.1)
	{	
		if (fl_last_time_heavybreak > gpGlobals->time)
		{
			fl_value = 6;
		}
		else
			fl_value = 2;
	}

	float fl_maxrotspeed;
	fl_maxrotspeed = pev->velocity.Length();
	if (fl_maxrotspeed > (50*fl_value))
		fl_maxrotspeed = 50*fl_value;

	if (fl_speed > 0)
		pev->avelocity = Vector(0,1,0) * -fl_maxrotspeed;
	else if (fl_speed < 0)
		pev->avelocity = Vector(0,1,0) * fl_maxrotspeed;
}

void CEnvCar :: PlayerUseBreak(void)
{
	if (bIsCarFlying)
		return;

	bPlayerExited = FALSE;

	fl_last_time_break = gpGlobals->time;

	if (fl_speed > 0 && fl_last_time_backward >= (gpGlobals->time - 0.1))
	{
		PlayerUseHeavyBreak();
		return;
	}
	else if (fl_speed < 0 && fl_last_time_forward >= (gpGlobals->time - 0.1))
	{
		PlayerUseHeavyBreak();
		return;
	}

	float fl_deceleration;
	fl_deceleration = 8;

	if (fl_speed > 8)
		fl_speed -= fl_deceleration;
	else if (fl_speed < -8)
		fl_speed += fl_deceleration;
	else
		fl_speed = 0;


	if (fl_bouncespeed > 20)
	{
		fl_bouncespeed -= 4;
		if (fl_bouncespeed < 20)
			fl_bouncespeed = 0;
	}
	else if (fl_bouncespeed < -20)
	{
		fl_bouncespeed += 4;
		if (fl_bouncespeed > -0)
			fl_bouncespeed = 0;
	}
	else
		fl_bouncespeed = 0;

	if (fl_speed > fl_maxspeed)
		fl_speed = fl_maxspeed;
	else if (fl_speed < -fl_maxspeed)
		fl_speed = -fl_maxspeed;

	Vector vecDir;
	UTIL_MakeVectorsPrivate(pev->angles, vecDir, NULL, NULL);
	pev->velocity = vecDir.Normalize()*fl_speed;
}

void CEnvCar :: PlayerUseHeavyBreak(void)
{
	if (bIsCarFlying)
		return;

	if ((fl_speed > 48) || (fl_speed < -48))
	{
		if ( fl_last_time_heavybreak < gpGlobals->time - (6/pev->velocity.Length()) ) //draw every 6 units
			DrawBrakeLines();

		if ( (fl_last_time_heavybreak < gpGlobals->time-0.1)) //noise every 0.1 seconds
			EMIT_SOUND_DYN( ENT(pev), CHAN_ITEM, "car_brake.wav", 1, ATTN_NORM, 0, 100 );
	}

	float fl_deceleration;
	fl_deceleration = 12;

	fl_last_time_heavybreak = gpGlobals->time;

	if (fl_speed > 12)
		fl_speed -= fl_deceleration;
	else if (fl_speed < -12)
		fl_speed += fl_deceleration;
	else
		fl_speed = 0;

	if (fl_bouncespeed > 20)
	{
		fl_bouncespeed -= 6;
		if (fl_bouncespeed < 20)
			fl_bouncespeed = 0;
	}
	else if (fl_bouncespeed < -20)
	{
		fl_bouncespeed += 6;
		if (fl_bouncespeed > -0)
			fl_bouncespeed = 0;
	}
	else
		fl_bouncespeed = 0;

	if (fl_speed > fl_maxspeed)
		fl_speed = fl_maxspeed;
	else if (fl_speed < -fl_maxspeed)
		fl_speed = -fl_maxspeed;

	Vector vecDir;
	UTIL_MakeVectorsPrivate(pev->angles, vecDir, NULL, NULL);
	pev->velocity = vecDir.Normalize()*fl_speed;
}

void CEnvCar :: DrawBrakeLines(void)
{
	Vector vecLeftFront;
	Vector vecRightFront;
	Vector vecLeftBack;
	Vector vecRightBack;

	Vector vecLeftFront2;
	Vector vecRightFront2;
	Vector vecLeftBack2;
	Vector vecRightBack2;

	TraceResult trLeftFront;
	TraceResult trRightFront;
	TraceResult trLeftBack;
	TraceResult trRightBack;

	vecLeftFront = vecLeftFront2 = VecAffin(vecLeftFrontWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	vecLeftFront2.z -= 24;
	
	vecRightFront = vecRightFront2 = VecAffin(vecRightFrontWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	vecRightFront2.z -= 24;
	
	vecLeftBack = vecLeftBack2 = VecAffin(vecLeftBackWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	vecLeftBack2.z -= 24;
	
	vecRightBack = vecRightBack2 = VecAffin(vecRightBackWheelPos,pev->angles.x,pev->angles.y,pev->angles.z);
	vecRightBack2.z -= 24;

	UTIL_TraceLine(pev->origin + vecLeftFront,pev->origin + vecLeftFront2,ignore_monsters,dont_ignore_glass,edict(),&trLeftFront);
	UTIL_TraceLine(pev->origin + vecRightFront,pev->origin + vecRightFront2,ignore_monsters,dont_ignore_glass,edict(),&trRightFront);
	UTIL_TraceLine(pev->origin + vecLeftBack,pev->origin + vecLeftBack2,ignore_monsters,dont_ignore_glass,edict(),&trLeftBack);
	UTIL_TraceLine(pev->origin + vecRightBack,pev->origin + vecRightBack2,ignore_monsters,dont_ignore_glass,edict(),&trRightBack);

	if (pev->angles.y < 0)
		pev->angles.y += 360;
	else if (pev->angles.y >= 360)
		pev->angles.y -= 360;
	
	int decalyaw = 0;
	int decal = DECAL_SMALLSCORCH1;

	float angleyaw;

	if (pev->angles.y > 180)
		angleyaw = pev->angles.y - 180;
	else
		angleyaw = pev->angles.y;

	if (angleyaw <= 22.5 || angleyaw >= 157.5)
		decalyaw = 0;
	else if (angleyaw >= 22.5 && angleyaw <= 67.5)
		decalyaw = 45;
	else if (angleyaw >= 67.5 && angleyaw <= 112.5)
		decalyaw = 90;
	else if (angleyaw >= 112.5 && angleyaw <= 157.5)
		decalyaw = 135;

	//BOOL again;
	//again = FALSE;

//Again:

	switch (decalyaw)
	{
	case 135:
		decal = DECAL_SMALLSCORCH1;
		break;
	case 90:
		decal = DECAL_SMALLSCORCH1;
		break;
	case 45:
		decal = DECAL_SMALLSCORCH1;
		break;
	default:
	case 0:
		decal = DECAL_SMALLSCORCH1;
		break;
	
	}
	
	//if (!again)
	{

		UTIL_DecalTrace( &trLeftBack, decal  );
		UTIL_DecalTrace( &trRightBack, decal  );

		/*
		if (wheel_left)
		{
			again = TRUE;
			decalyaw += 45;
			if (decalyaw > 135)
				decalyaw -= 180;
			goto Again;
		}
		else if (wheel_right)
		{
			again = TRUE;
			decalyaw -= 45;
			if (decalyaw < 0)
				decalyaw += 180;
			goto Again;
		}
		*/

	}

	UTIL_DecalTrace( &trLeftFront, decal );
	UTIL_DecalTrace( &trRightFront, decal  );

	if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
	{/*
		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_SHORT(iDefaultDrop);
		MESSAGE_END();*/
	}
}

void CEnvCar :: PlayerUseHorn(void)
{
	bPlayerExited = FALSE;

	EMIT_SOUND_DYN( ENT(pev), CHAN_ITEM, "bus_horn.wav", 1, ATTN_NORM, 0, 100 );
}

void CEnvCar :: PlayerUseLeftOff(void)
{
	pModel->SetBoneController ( 0, 0 );
	pev->avelocity = Vector(0,0,0);
	wheel_left = FALSE;
	wheel_right = FALSE;
}

void CEnvCar :: PlayerUseRightOff(void)
{
	PlayerUseLeftOff();
}

void CEnvCar :: PlayerUseExit(void)
{
	PlayerUseHeavyBreak();

	bPlayerExited = TRUE;
}

class CEnvCustomCar : public CEnvCar
{
public:
	//standard ent functions
	void Precache( void );
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( car_rally, CEnvCustomCar );

void CEnvCustomCar :: Precache()
{
	PRECACHE_MODEL( "sprites/steam1.spr" );
	iSprite = PRECACHE_MODEL( "sprites/steam1.spr" );// smoke
	iExplode = PRECACHE_MODEL( "sprites/fexplo.spr" );// smoke
	
	iSprite2 = PRECACHE_MODEL( "sprites/glow02.spr" );// smoke

	PRECACHE_MODEL("sprites/gargeye1.spr");

	PRECACHE_MODEL((char*)STRING(pev->model));
	
	PRECACHE_MODEL("models/cars/car3.mdl");

	//cars
	PRECACHE_SOUND("car_brake.wav");
	PRECACHE_SOUND("bus_horn.wav");
	PRECACHE_SOUND("car_engine.wav");
	PRECACHE_SOUND("first.wav");
	PRECACHE_SOUND("second.wav");
	PRECACHE_SOUND("third.wav");
}	

void CEnvCustomCar :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->solid = SOLID_BSP;

	pev->takedamage = DAMAGE_YES;

	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	
	vecSavedStartOrigin = pev->origin;

	savedCarVictim = NULL;
	savedCarVictim1 = NULL;
	fl_last_victim_time = fl_last_victim_time1 = 0;

	//pev->effects |= EF_NODRAW;

	pev->avelocity = Vector(0,0,0);
	pev->velocity = Vector(0,0,0);

	fl_health = 1000;
	fl_health_CAR_MODEL_FRONT = 1000;
	fl_health_CAR_MODEL_BACK = 1000;

	pev->health = fl_health;
	
	pForForTarget = NULL;
	pForTarget = NULL;
	pCurrentTarget = NULL;
	pNextTarget = NULL;
	pNextNextTarget = NULL;
	pNextNextNextTarget = NULL;
	pModel = NULL;

	vecDirection = Pos1 = Pos2 = Pos3 = Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	checkrouteallowed = foundpathright = foundpathleft = bRouteDone = FALSE;
	
	vecLeftFrontWheelPos.x = pev->maxs.x;
	vecLeftFrontWheelPos.y = pev->mins.y;
	vecLeftFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightFrontWheelPos.x = pev->maxs.x;
	vecRightFrontWheelPos.y = pev->maxs.y;
	vecRightFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecLeftBackWheelPos.x = pev->mins.x;
	vecLeftBackWheelPos.y = pev->mins.y;
	vecLeftBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightBackWheelPos.x = pev->mins.x;
	vecRightBackWheelPos.y = pev->maxs.y;
	vecRightBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	fl_bouncespeed = fl_speed = 0;
	bIsCarFlying = FALSE;
	bPlayerExited = FALSE;
	
	wheel_left = FALSE;
	wheel_right = FALSE;
	
	vecCurrentUp = Vector(0,0,1);
	vecDesiredDir = Vector(0,0,0);

	fl_last_time_forward = fl_last_time_backward = fl_last_time_heavybreak = fl_last_time_break = fl_last_time_smoked = 0;

	SetTouch(&CEnvCar::Touch);
	SetUse(&CEnvCar::Use);

	SetThink(&CEnvCar::IdleThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetNextThink( 0.1 );

	//moved from postspawn, due to bug
	//create the model render ent
	pModel = GetClassPtr( (CCarModel *)NULL );

	pModel->pev->model = MAKE_STRING("models/cars/car3.mdl");	
	SET_MODEL(ENT(pModel->pev), "models/cars/car3.mdl");

	pModel->pev->origin = pev->origin;
	UTIL_SetOrigin(pModel->pev, pev->origin);

	pModel->pev->classname = MAKE_STRING("car3_model");
	pModel->pev->solid = SOLID_NOT;
	pModel->pev->movetype = MOVETYPE_NOCLIP;
	pModel->pev->owner = edict();
	pModel->Spawn();
}

//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\*/

class CEnvCombineCar : public CEnvCar
{
public:
	//standard ent functions
	void Precache( void );
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( car_fork, CEnvCombineCar );

void CEnvCombineCar :: Precache()
{
	PRECACHE_MODEL( "sprites/steam1.spr" );
	iSprite = PRECACHE_MODEL( "sprites/steam1.spr" );// smoke
	iExplode = PRECACHE_MODEL( "sprites/fexplo.spr" );// smoke
	
	iSprite2 = PRECACHE_MODEL( "sprites/glow02.spr" );// smoke

	PRECACHE_MODEL("sprites/gargeye1.spr");

	PRECACHE_MODEL((char*)STRING(pev->model));
	
	PRECACHE_MODEL("models/cars/fork.mdl");

	//cars
	PRECACHE_SOUND("car_brake.wav");
	PRECACHE_SOUND("bus_horn.wav");
	PRECACHE_SOUND("car_engine.wav");
}	

void CEnvCombineCar :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->solid = SOLID_BSP;

	pev->takedamage = DAMAGE_YES;

	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	
	vecSavedStartOrigin = pev->origin;

	savedCarVictim = NULL;
	savedCarVictim1 = NULL;
	fl_last_victim_time = fl_last_victim_time1 = 0;

	//test
	pev->effects |= EF_NODRAW;

	pev->avelocity = Vector(0,0,0);
	pev->velocity = Vector(0,0,0);

	fl_health = 2000;
	fl_health_CAR_MODEL_FRONT = 2000;
	fl_health_CAR_MODEL_BACK = 2000;

	pev->health = fl_health;
	
	pForForTarget = NULL;
	pForTarget = NULL;
	pCurrentTarget = NULL;
	pNextTarget = NULL;
	pNextNextTarget = NULL;
	pNextNextNextTarget = NULL;
	pModel = NULL;

	vecDirection = Pos1 = Pos2 = Pos3 = Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	checkrouteallowed = foundpathright = foundpathleft = bRouteDone = FALSE;
	
	vecLeftFrontWheelPos.x = pev->maxs.x;
	vecLeftFrontWheelPos.y = pev->mins.y;
	vecLeftFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightFrontWheelPos.x = pev->maxs.x;
	vecRightFrontWheelPos.y = pev->maxs.y;
	vecRightFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecLeftBackWheelPos.x = pev->mins.x;
	vecLeftBackWheelPos.y = pev->mins.y;
	vecLeftBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightBackWheelPos.x = pev->mins.x;
	vecRightBackWheelPos.y = pev->maxs.y;
	vecRightBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	fl_bouncespeed = fl_speed = 0;
	bIsCarFlying = FALSE;
	bPlayerExited = FALSE;
	
	wheel_left = FALSE;
	wheel_right = FALSE;
	
	vecCurrentUp = Vector(0,0,1);
	vecDesiredDir = Vector(0,0,0);

	fl_last_time_forward = fl_last_time_backward = fl_last_time_heavybreak = fl_last_time_break = fl_last_time_smoked = 0;

	SetTouch(&CEnvCar::Touch);
	SetUse(&CEnvCar::Use);

	SetThink(&CEnvCar::IdleThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetNextThink( 0.1 );

	//moved from postspawn, due to bug
	//create the model render ent
	pModel = GetClassPtr( (CCarModel *)NULL );

	pModel->pev->model = MAKE_STRING("models/cars/fork.mdl");	
	SET_MODEL(ENT(pModel->pev), "models/cars/fork.mdl");

	pModel->pev->origin = pev->origin;

	UTIL_SetOrigin(pModel->pev, pev->origin);

	pModel->pev->classname = MAKE_STRING("car_fork_model");
	pModel->pev->solid = SOLID_NOT;
	pModel->pev->movetype = MOVETYPE_NOCLIP;
	pModel->pev->owner = edict();

	if ( FClassnameIs ( pev, "car_fork" ) )
	{
		pModel->SetBodygroup( 0, 0 );//freeman is not on car
		pModel->SetBodygroup( 1, 0 );//freeman is not on car
	}

	pModel->Spawn();
}

//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\
//************************************************************************************************************\\*/


class CEnvTank : public CEnvCar
{
public:
	//standard ent functions
	void Precache( void );
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( vehicle_tank, CEnvTank );

void CEnvTank :: Precache()
{
	PRECACHE_MODEL( "sprites/steam1.spr" );
	iSprite = PRECACHE_MODEL( "sprites/steam1.spr" );// smoke
	iExplode = PRECACHE_MODEL( "sprites/fexplo.spr" );// smoke
	
	iSprite2 = PRECACHE_MODEL( "sprites/glow02.spr" );// smoke

	PRECACHE_MODEL("sprites/gargeye1.spr");

	PRECACHE_MODEL((char*)STRING(pev->model));
	
	PRECACHE_MODEL("models/cars/tank.mdl");

	//cars
	PRECACHE_SOUND("car_brake.wav");
	PRECACHE_SOUND("bus_horn.wav");
	PRECACHE_SOUND("car_engine.wav");
	PRECACHE_SOUND("first.wav");
	PRECACHE_SOUND("second.wav");
	PRECACHE_SOUND("third.wav");
}	

void CEnvTank :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->solid = SOLID_BSP;

	pev->takedamage = DAMAGE_YES;

	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	
	vecSavedStartOrigin = pev->origin;

	savedCarVictim = NULL;
	savedCarVictim1 = NULL;
	fl_last_victim_time = fl_last_victim_time1 = 0;

	//test
	pev->effects |= EF_NODRAW;

	pev->avelocity = Vector(0,0,0);
	pev->velocity = Vector(0,0,0);

	fl_health = 2000;
	fl_health_CAR_MODEL_FRONT = 2000;
	fl_health_CAR_MODEL_BACK = 2000;

	pev->health = fl_health;
	
	pForForTarget = NULL;
	pForTarget = NULL;
	pCurrentTarget = NULL;
	pNextTarget = NULL;
	pNextNextTarget = NULL;
	pNextNextNextTarget = NULL;
	pModel = NULL;

	vecDirection = Pos1 = Pos2 = Pos3 = Pos4 = Pos5 = Pos6 = Pos7 = Pos8 = Pos9 = Pos10 = Pos11 = Pos12 = Pos13 = Pos14 = Pos15 = Pos16 = Pos17 = Pos18 = Vector(0,0,0);
	checkrouteallowed = foundpathright = foundpathleft = bRouteDone = FALSE;
	
	vecLeftFrontWheelPos.x = pev->maxs.x;
	vecLeftFrontWheelPos.y = pev->mins.y;
	vecLeftFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightFrontWheelPos.x = pev->maxs.x;
	vecRightFrontWheelPos.y = pev->maxs.y;
	vecRightFrontWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecLeftBackWheelPos.x = pev->mins.x;
	vecLeftBackWheelPos.y = pev->mins.y;
	vecLeftBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	vecRightBackWheelPos.x = pev->mins.x;
	vecRightBackWheelPos.y = pev->maxs.y;
	vecRightBackWheelPos.z = 0;//pev->origin.z;// + pev->mins.z;

	fl_bouncespeed = fl_speed = 0;
	bIsCarFlying = FALSE;
	bPlayerExited = FALSE;
	
	wheel_left = FALSE;
	wheel_right = FALSE;
	
	vecCurrentUp = Vector(0,0,1);
	vecDesiredDir = Vector(0,0,0);

	fl_last_time_forward = fl_last_time_backward = fl_last_time_heavybreak = fl_last_time_break = fl_last_time_smoked = 0;

	SetTouch(&CEnvCar::Touch);
	SetUse(&CEnvCar::Use);

	SetThink(&CEnvCar::IdleThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetNextThink( 0.1 );

	//moved from postspawn, due to bug
	//create the model render ent
	pModel = GetClassPtr( (CCarModel *)NULL );

	pModel->pev->model = MAKE_STRING("models/cars/tank.mdl");	
	SET_MODEL(ENT(pModel->pev), "models/cars/tank.mdl");

	pModel->pev->origin = pev->origin;

	UTIL_SetOrigin(pModel->pev, pev->origin);

	pModel->pev->classname = MAKE_STRING("car_tank");
	pModel->pev->solid = SOLID_NOT;
	pModel->pev->movetype = MOVETYPE_NOCLIP;
	pModel->pev->owner = edict();
	pModel->Spawn();
}
