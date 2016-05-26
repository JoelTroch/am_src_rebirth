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
#include "../hud.h"
#include "../cl_util.h"
#include "event_api.h"

extern "C"
{
// HLDM
void EV_FireGlock1( struct event_args_s *args  );
void EV_FireGlock2( struct event_args_s *args  );

void EV_FirePython( struct event_args_s *args  );
void EV_FireGauss( struct event_args_s *args  );
void EV_SpinGauss( struct event_args_s *args  );

void EV_FireCrossbow( struct event_args_s *args  );
void EV_FireCrossbow2( struct event_args_s *args  );

void EV_FireRpg( struct event_args_s *args  );
void EV_EgonFire( struct event_args_s *args  );
void EV_EgonStop( struct event_args_s *args  );
void EV_HornetGunFire( struct event_args_s *args  );
void EV_TripmineFire( struct event_args_s *args  );
void EV_SnarkFire( struct event_args_s *args  );
void EV_FirePistol( struct event_args_s *args  ); //ohne Silencer


void EV_TrainPitchAdjust( struct event_args_s *args );

void EV_SpasShell( struct event_args_s *args  );

void EV_FireHL2Crowbar( struct event_args_s *args  );
void EV_FireHL2ShotGunSingle( struct event_args_s *args  );
void EV_FireHL2ShotGunDouble( struct event_args_s *args  );

void EV_FireM249( struct event_args_s *args  );

void EV_FireMP5( struct event_args_s *args  );

void EV_FireM16( struct event_args_s *args  );
void EV_FireM162( struct event_args_s *args  );

void EV_FireDeagle( struct event_args_s *args  );
void EV_FireSniper( struct event_args_s *args  );
void EV_SniperShell( struct event_args_s *args  );
}


/*
======================
Game_HookEvents

Associate script file name with callback functions.  Callback's must be extern "C" so
 the engine doesn't get confused about name mangling stuff.  Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents( void )
{
	gEngfuncs.pfnHookEvent( "scripts/events/crossbow1.sc",				EV_FireCrossbow );

	gEngfuncs.pfnHookEvent( "scripts/events/egon_fire.sc",				EV_EgonFire );
	gEngfuncs.pfnHookEvent( "scripts/events/egon_stop.sc",				EV_EgonStop );

	gEngfuncs.pfnHookEvent( "scripts/events/m16.sc",					EV_FireM16 );
	gEngfuncs.pfnHookEvent( "scripts/events/m162.sc",					EV_FireM162 );

	gEngfuncs.pfnHookEvent( "scripts/events/python.sc",					EV_FirePython );
	gEngfuncs.pfnHookEvent( "scripts/events/gauss.sc",					EV_FireGauss );
	gEngfuncs.pfnHookEvent( "scripts/events/gaussspin.sc",				EV_SpinGauss );
	gEngfuncs.pfnHookEvent( "scripts/events/train.sc",					EV_TrainPitchAdjust );

	gEngfuncs.pfnHookEvent( "scripts/events/rpg.sc",					EV_FireRpg );

	gEngfuncs.pfnHookEvent( "scripts/events/firehornet.sc",				EV_HornetGunFire );
	gEngfuncs.pfnHookEvent( "scripts/events/tripfire.sc",				EV_TripmineFire );
	gEngfuncs.pfnHookEvent( "scripts/events/snarkfire.sc",				EV_SnarkFire );

	gEngfuncs.pfnHookEvent( "scripts/events/eagle.sc",				    EV_FireDeagle );//eagle

	gEngfuncs.pfnHookEvent( "scripts/events/Sniper.sc",					EV_FireSniper );
	gEngfuncs.pfnHookEvent( "scripts/events/sniper_shell.sc",			EV_SniperShell );

	gEngfuncs.pfnHookEvent( "scripts/events/shotgun_shell.sc",			EV_SpasShell );

	gEngfuncs.pfnHookEvent( "scripts/events/saw.sc",					EV_FireM249 );//saw?

	gEngfuncs.pfnHookEvent( "scripts/events/mp5.sc",					EV_FireMP5 );
	
	gEngfuncs.pfnHookEvent( "scripts/events/glock.sc",					EV_FirePistol );//glock

	gEngfuncs.pfnHookEvent( "scripts/events/crowbar.sc",			EV_FireHL2Crowbar );
	gEngfuncs.pfnHookEvent( "scripts/events/shotgun1.sc",			EV_FireHL2ShotGunSingle );
	gEngfuncs.pfnHookEvent( "scripts/events/shotgun2.sc",			EV_FireHL2ShotGunDouble );
}
