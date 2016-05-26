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
#define SHOTGUN_RECOIL	0.8
#define PISTOL_RECOIL	0.4
#define SMG_RECOIL		0.8
#define RIFLE_RECOIL	1
#define MG_RECOIL		1.5

//#include "m16.h"
//#include "hud_spectator.h"
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>

#include "r_studioint.h"
#include "com_model.h"

#include "sound.h"

extern engine_studio_api_t IEngineStudio;

static int tracerCount[ 32 ];
extern cvar_t *cl_wallpuff;

extern "C" char PM_FindTextureType( char *name );

void V_PunchAxis( int axis, float punch );
void VectorAngles( const float *forward, float *angles );

extern cvar_t *cl_lw;

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

#define VECTOR_CONE_PERFECT Vector( 0.00001, 0.00001, 0.00001 )

#define VECTOR_CONE_1DEGREES Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES Vector( 0.06105, 0.06105, 0.06105 )	
#define VECTOR_CONE_8DEGREES Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES Vector( 0.17365, 0.17365, 0.17365 )

#define VECTOR_CONE_BAD			Vector( 0.27365, 0.27365, 0.27365 )
#define VECTOR_CONE_TOOBAD		Vector( 0.47365, 0.47365, 0.47365 )

void AnimWallPuff( struct tempent_s *ent, float frametime, float currenttime )
{
	VectorMA( ent->entity.baseline.origin, frametime, ent->entity.baseline.velocity, ent->entity.baseline.origin );
}
//sys test
void SpriteTracer( vec3_t pos, vec3_t oldpos, int modelindex, float r, float g, float b, float a )
{
	BEAM *pTracer = gEngfuncs.pEfxAPI->R_BeamPoints( 
				pos,
				oldpos,
				modelindex,
				0.1,	// life
				0.3,	// width
				0.0,	// amplitude
				a,
				0,
				0,
				0,
				r,
				g,
				b
			);

	if( pTracer )
	{
		gEngfuncs.Con_Printf( "Okay\n" );
		pTracer->flags |=  FBEAM_FADEIN;
	}
}
////////
// mazor begin
void EV_HLDM_MuzzleFlash(vec3_t pos, float amount)
{
      dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
      dl->origin = pos;
      dl->color.r = 255; // red
      dl->color.g = 255; // green
      dl->color.b = 128; // blue
      dl->radius = amount * 100;
      dl->die = gEngfuncs.GetClientTime() + 0.01;
}
// mazor end
/////

// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play
float EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType )
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	float fvolbar;
	char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char *pTextureName;
	char texname[ 64 ];
	char szbuffer[ 64 ];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( ptr );

	// FIXME check if playtexture sounds movevar is set
	//

	chTextureType = 0;

	// Player
	if ( entity >= 1 && entity <= gEngfuncs.GetMaxClients() )
	{
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	}
	else if ( entity == 0 )
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( ptr->ent, vecSrc, vecEnd );
		
		if ( pTextureName )
		{
			strcpy( texname, pTextureName );
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+')
			{
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
			{
				pTextureName++;
			}
			
			// '}}'
			strcpy( szbuffer, pTextureName );
			szbuffer[ CBTEXTURENAMEMAX - 1 ] = 0;
				
			// get texture type
			chTextureType = PM_FindTextureType( szbuffer );	
		}
	}
	
	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE: fvol = 0.9;	fvolbar = 0.6;
		rgsz[0] = "player/damage/ric_conc1.wav"; //SP: New ric sounds
		rgsz[1] = "player/damage/ric_conc2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL: fvol = 0.9; fvolbar = 0.3;
		rgsz[0] = "player/damage/ric_METAL1.wav";
		rgsz[1] = "player/damage/ric_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:	fvol = 0.9; fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:	fvol = 0.5; fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE: fvol = 0.9; fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:	fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH: fvol = 0.9; fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD: fvol = 0.9; fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:
		if (iBulletType == BULLET_PLAYER_CROWBAR)
			return 0.0; // crowbar already makes this sound
		fvol = 1.0;	fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}

	// play material hit sound
	if ( CVAR_GET_FLOAT( "slowmo" ) != 0 )
	gEngfuncs.pEventAPI->EV_PlaySound( 0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0,cnt-1)], fvol, fattn, 0, 50 + gEngfuncs.pfnRandomLong(0,0xf) );
	else
	gEngfuncs.pEventAPI->EV_PlaySound( 0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0,cnt-1)], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong(0,0xf) );

	return fvolbar;
}
/*
char *EV_HLDM_DamageDecal( physent_t *pe )
{
	static char decalname[ 32 ];
	int idx;

	if ( pe->classnumber == 1 )
	{
		idx = gEngfuncs.pfnRandomLong( 0, 2 );
		sprintf( decalname, "{break%i", idx + 1 );
	}
	else if ( pe->rendermode != kRenderNormal )
	{
		sprintf( decalname, "{bproof1" );
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong( 0, 4 );
		sprintf( decalname, "{shot%i", idx + 1 );
	}
	return decalname;
}
*/
/* 
TTT: Event which spawns a smokepuff and/or sparks at a given origin
Note that you have to precache the sprites in the game dll
*/
void EV_HLDM_SmokePuff( pmtrace_t *pTrace, float *vecSrc, float *vecEnd )
{
	//TO DO: make a decent smoke puff. I wanna see water splashes!. 
	//I need to use particle system (aurora or whatever)

	//sys params
	vec3_t angles, forward, right, up;
    VectorAngles( pTrace->plane.normal, angles );
    AngleVectors( angles, forward, up, right );
    forward.z = -forward.z;
//	int m_iMetalGlow;

	vec3_t fwd;

//	m_iMetalGlow = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/Metal_puff.spr" );
//	int  iWallsmoke = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/wallsmoke.spr");
//	int  iWallsmoke2 = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/wallsmoke2.spr");

	//sys params

    physent_t *pe;

    // get entity at endpoint
    pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

    if ( pe && pe->solid == SOLID_BSP )
    {    // if it's a solid wall / entity
        char chTextureType = CHAR_TEX_CONCRETE;
        char *pTextureName;
        char texname[ 64 ];
        char szbuffer[ 64 ];

        // get texture name
        pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( pTrace->ent, vecSrc, vecEnd );
        
        if ( pTextureName )
        {
            strcpy( texname, pTextureName );
            pTextureName = texname;

            // strip leading '-0' or '+0~' or '{' or '!'
            if (*pTextureName == '-' || *pTextureName == '+')
            {
                pTextureName += 2;
            }

            if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
            {
                pTextureName++;
            }
            
            // '}}'
            strcpy( szbuffer, pTextureName );
            szbuffer[ CBTEXTURENAMEMAX - 1 ] = 0;
                
            // get texture type
            chTextureType = PM_FindTextureType( szbuffer );    
        }
        
        bool fDoPuffs = false;
        bool fDoSparks = false;
        int a,r,g,b;

		int  iDebrisGlass = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/debris_glass.spr");

		vec3_t fwd;
		VectorAdd( pTrace->endpos, pTrace->plane.normal, fwd );

		switch (chTextureType)
        {
            // do smoke puff and eventually add sparks
            case CHAR_TEX_TILE:
	/*						
				switch( gEngfuncs.pfnRandomLong( 0, 2 ) )
				{
				case 0:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/tile1.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 1:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/tile2.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 2:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/tile3.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				}*/
				fDoPuffs = true;
						a = 128; //128
						r = 99; //200 all
						g = 99;
						b = 99;
				break;

            case CHAR_TEX_CONCRETE:/*
				switch( gEngfuncs.pfnRandomLong( 0, 2 ) )
				{
				case 0:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/concrete1.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 1:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/concrete2.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 2:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/concrete3.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				}*/
                fDoPuffs = true;
                a = 128; //128
                r = 50; //200 all
                g = 50;
                b = 50;
                break;
      
			case CHAR_TEX_METAL: 
		    case CHAR_TEX_VENT:
            case CHAR_TEX_GRATE: 
			case CHAR_TEX_COMPUTER:
                fDoSparks = (gEngfuncs.pfnRandomLong(1, 4) == 1); //(1, 2) == 1);

//				gEngfuncs.pEfxAPI->R_TempSprite( pTrace->endpos, vec3_origin, 0.1, m_iMetalGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT );
				// mazor begin
				EV_HLDM_MuzzleFlash( pTrace->endpos, 0.8 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
				// mazor end
						
				VectorAdd( pTrace->endpos, pTrace->plane.normal, fwd );
//				gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, pTrace->endpos, fwd, m_iMetalGlow, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
//					100, 50 );

				switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
				{
				case 0:
					gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "fisica/metal/b_impact1.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );
					break;
				case 1:
					gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "fisica/metal/b_impact2.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );
					break;
				}

				// spawn some sparks
				gEngfuncs.pEfxAPI->R_SparkShower( pTrace->endpos );
				// Show Sparks
				gEngfuncs.pEfxAPI->R_SparkEffect( pTrace->endpos, 8, -200, 200 );

				gEngfuncs.pEfxAPI->R_StreakSplash( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(-10, 10) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					0, 5, 50, 100, 500);
				gEngfuncs.pEfxAPI->R_StreakSplash( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(-10, 10) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					9, 5, 50, 100, 100);
				gEngfuncs.pEfxAPI->R_StreakSplash( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(-10, 10) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					0, 5, 50, 100, 500);
				gEngfuncs.pEfxAPI->R_StreakSplash( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(-10, 10) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					0, 5, 50, 100, 500);
				gEngfuncs.pEfxAPI->R_StreakSplash( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(-10, 10) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					0, 5, 50, 100, 500);

				//( float * pos, float * dir, int color, int count, float speed, int velocityMin, int velocityMax );
                break;
            
            // draw brown puff, but don't do sparks
            case CHAR_TEX_DIRT: 
//				gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRAY, pTrace->endpos, fwd, iDebrisGrass, 8, 0.1, 0.1, 255, 150, 150 );

				a = 250;
                r = 97;
                g = 86;
                b = 53;
                break;

            case CHAR_TEX_WOOD:
//				gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRAY, pTrace->endpos, fwd, iDebrisWood, 8, 0.1, 0.1, 255, 150, 150 );

                fDoPuffs = true;
/*
				switch( gEngfuncs.pfnRandomLong( 0, 2 ) )
				{
				case 0:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/wood1.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 1:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/wood2.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 2:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/wood3.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				}
*/
                a = 250;
                r = 97;
                g = 86;
                b = 53;
                break;

            // don't do anything if those textures (perhaps add something later...)
            default:
            case CHAR_TEX_GLASS:
				gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRAY, pTrace->endpos, fwd, iDebrisGlass, 8, 0.1, 0.1, 255, 150, 150 );
/*
				switch( gEngfuncs.pfnRandomLong( 0, 2 ) )
				{
				case 0:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/glass1.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 1:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/glass2.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				case 2:gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "WEAPONS/BulletImpact/glass3.wav", 1.5, ATTN_NORM, 0, PITCH_NORM );break;
				}
*/
                break;
        }
                
        if( fDoPuffs )
        {/*
            vec3_t angles, forward, right, up;

            VectorAngles( pTrace->plane.normal, angles );

            AngleVectors( angles, forward, up, right );
            forward.z = -forward.z;
            // get sprite index
            int  iWallsmoke = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/wallsmoke.spr");

			if ( cl_wallpuff->value == 1)
			{
				// ************SPRITE 1
				TEMPENTITY *pTemp = gEngfuncs.pEfxAPI->R_TempSprite( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(10, 30) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					1.4,//0.4
					iWallsmoke,
					kRenderTransAdd,
					kRenderFxNone,
					1.0,//1.0
					1, //0.3
					FTENT_SPRANIMATE | FTENT_FADEOUT
				
				);
				if(pTemp)
				{    // sprite created successfully, adjust some things
					pTemp->fadeSpeed = 2.0;//2
					pTemp->entity.curstate.framerate = 20.0;//20
					pTemp->entity.curstate.renderamt = a;
					pTemp->entity.curstate.rendercolor.r = r;
					pTemp->entity.curstate.rendercolor.g = g;
					pTemp->entity.curstate.rendercolor.b = b;
				}
				// ************SPRITE 1
				
				 // ************SPRITE 2
				 TEMPENTITY *pTemp2 = gEngfuncs.pEfxAPI->R_TempSprite( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(5, 10) + right * gEngfuncs.pfnRandomFloat(-3, 3) + up * gEngfuncs.pfnRandomFloat(0, 3),
					gEngfuncs.pfnRandomFloat(1, 3),//0.4
					iWallsmoke2,
					kRenderTransAdd,
					kRenderFxNone,
					gEngfuncs.pfnRandomFloat(1.0, 2.0),//1.0
					0.3, //0.3
					FTENT_SPRANIMATE | FTENT_FADEOUT);

				if(pTemp2)
				{    // sprite created successfully, adjust some things
					pTemp2->fadeSpeed = 1;//2
					pTemp2->entity.curstate.framerate = 10;//20, mas lento
					pTemp2->entity.curstate.renderamt = a;
					pTemp2->entity.curstate.rendercolor.r = r /2;
					pTemp2->entity.curstate.rendercolor.g = g /2;
					pTemp2->entity.curstate.rendercolor.b = b /2;
				}
				// ************SPRITE 2
			// ************SPRITE 3
				 TEMPENTITY *pTemp22 = gEngfuncs.pEfxAPI->R_TempSprite( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(5, 10) + right * gEngfuncs.pfnRandomFloat(-3, 3) + up * gEngfuncs.pfnRandomFloat(0, 3),
					gEngfuncs.pfnRandomFloat(1, 2.5),//0.4
					iWallsmoke2,
					kRenderTransAdd,
					kRenderFxNone,
					gEngfuncs.pfnRandomFloat(1.0, 2.0),//1.0
					0.3, //0.3
					FTENT_SPRANIMATE | FTENT_FADEOUT);

				if(pTemp22)
				{    // sprite created successfully, adjust some things
					pTemp22->fadeSpeed = 1;//2
					pTemp22->entity.curstate.framerate = 10;//20, mas lento
					pTemp22->entity.curstate.renderamt = a;
					pTemp22->entity.curstate.rendercolor.r = r /2;
					pTemp22->entity.curstate.rendercolor.g = g /2;
					pTemp22->entity.curstate.rendercolor.b = b /2;
				}					
		  }
			else if ( cl_wallpuff->value == 2)
			{	 
				// ************SPRITE 1
				TEMPENTITY *pTemp4 = gEngfuncs.pEfxAPI->R_TempSprite( pTrace->endpos, forward * gEngfuncs.pfnRandomFloat(10, 30) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					1.4,//0.4
					iWallsmoke,
					kRenderTransAdd,
					kRenderFxNone,

					//kRenderTransAdd,
					//kRenderFxNoDissipation,
					
					//kRenderTransAlpha,
					//kRenderFxNone,

					1.0,//1.0
					1, //0.3
					FTENT_SPRANIMATE | FTENT_FADEOUT
				
				);
				if(pTemp4)
				{    // sprite created successfully, adjust some things
					pTemp4->fadeSpeed = 2.0;//2
					pTemp4->entity.curstate.framerate = 20.0;//20
					pTemp4->entity.curstate.renderamt = a;
					pTemp4->entity.curstate.rendercolor.r = r;
					pTemp4->entity.curstate.rendercolor.g = g;
					pTemp4->entity.curstate.rendercolor.b = b;
				}
				// ************SPRITE 1
			}
			else
			{
				//nothing
			}*/
        }

        if( fDoSparks )
        {

        }
    }
}
#define DECAL_GENERIC		0
#define DECAL_METAL			1
#define DECAL_CONCRETE		2
#define DECAL_DIRT			3
#define DECAL_COMPUTER		4	
#define DECAL_WOOD			5
//#define DECAL_CUSTOM1		6
//#define DECAL_CUSTOM2		7
//#define DECAL_CUSTOM3		8
#define DECAL_SHOTPULSE		9

char *EV_HLDM_DamageDecal( physent_t *pe, int special_weapon )
{
	static char decalname[ 32 ];
	int idx;
	int iPaintBall = CVAR_GET_FLOAT("r_paintball");

	if(iPaintBall)
	{
		idx = gEngfuncs.pfnRandomLong( 0, 18 );

		sprintf( decalname, "{splat%i", idx );//esto del idx es insólito...:S
	}
	else
	{
		//original code here
		if ( pe->classnumber == 1 )
		{
			idx = gEngfuncs.pfnRandomLong( 0, 2 );
			sprintf( decalname, "{break%i", idx + 1 );
		}
		else if ( pe->rendermode != kRenderNormal )
		{
			sprintf( decalname, "{bproof1" );
		}
		//sys test
		else
		{
			idx = gEngfuncs.pfnRandomLong( 0, 2 );//son 3 ! //1

			if (special_weapon == DECAL_METAL)
			{
				sprintf( decalname, "{hole_metal_%i", idx + 1 );
			}
			else if (special_weapon == DECAL_CONCRETE)
			{
				sprintf( decalname, "{hole_conc_%i", idx + 1 );
			}
			else if (special_weapon == DECAL_DIRT)
			{
				sprintf( decalname, "{hole_dirt_%i", idx + 1 );
			}
			else if (special_weapon == DECAL_COMPUTER)
			{
				sprintf( decalname, "{hole_comp_%i", idx + 1 );
			}
			else if (special_weapon == DECAL_WOOD)
			{
				sprintf( decalname, "{hole_wood_%i", idx + 1 );
			}
			else if (special_weapon == DECAL_SHOTPULSE)
			{
				sprintf( decalname, "{shot_pulse_%i", idx + 1 );
			}
			else
			{
				sprintf( decalname, "{shot%i", idx + 1 );//this is generic
			}
		}
	}

	//EDIT: ah, que pelotudo. Parece que eso de buscar texturas al azar con el mismo nombre es necesario
	//por ej. wood_1, wood_2, y wood_3. Parece q al poner uno solo se va todo a la mierda.
	return decalname;
}

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, int special_weapon )
{
	int iRand;
	physent_t *pe;

	iRand = gEngfuncs.pfnRandomLong(0,0x7FFF);
	if ( iRand < (0x7fff/2) )// not every bullet makes a sound.
	{
		switch( iRand % 5)
		{
		case 0:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 2:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 3:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		case 4:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
		}
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	// Only decal brush models such as the world etc.
	if (  decalName && decalName[0] && pe && ( pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP ) )
	{
		if ( CVAR_GET_FLOAT( "r_decals" ) )
		{
			gEngfuncs.pEfxAPI->R_DecalShoot( 
				gEngfuncs.pEfxAPI->Draw_DecalIndex( gEngfuncs.pEfxAPI->Draw_DecalIndexFromName( decalName ) ), 
				gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace ), 0, pTrace->endpos, 0 );
		}
	}
}

void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType, float *vecSrc, float *vecEnd )
{
	physent_t *pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );
   
	//yees
//	gEngfuncs.pEfxAPI->R_BulletImpactParticles( pTrace->endpos );

	if ( pe && ( pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP ) )
	{        
		//fix here?
			//yees
		gEngfuncs.pEfxAPI->R_BulletImpactParticles( pTrace->endpos );
		//gEngfuncs.pEfxAPI->R_BulletImpactParticles( pTrace->endpos );

		char chTextureType = CHAR_TEX_CONCRETE;
        char *pTextureName;
        char texname[ 64 ];
        char szbuffer[ 64 ];

		int texture_type = DECAL_GENERIC; //absolute generic
        // get texture name
        pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( pTrace->ent, vecSrc, vecEnd );
        
        if ( pTextureName )
        {
            strcpy( texname, pTextureName );
            pTextureName = texname;

            // strip leading '-0' or '+0~' or '{' or '!'
            if (*pTextureName == '-' || *pTextureName == '+')
            {
                pTextureName += 2;
            }

            if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
            {
                pTextureName++;
            }
            
            // '}}'
            strcpy( szbuffer, pTextureName );
            szbuffer[ CBTEXTURENAMEMAX - 1 ] = 0;
                
            // get texture type
            chTextureType = PM_FindTextureType( szbuffer );    
        }
		
        switch (chTextureType)
        {
            case CHAR_TEX_TILE:
            case CHAR_TEX_CONCRETE:
				texture_type = DECAL_CONCRETE;
                break;

            case CHAR_TEX_VENT:
            case CHAR_TEX_GRATE: 
            case CHAR_TEX_METAL: 
				texture_type = DECAL_METAL;
				break;

			case CHAR_TEX_COMPUTER:
				texture_type = DECAL_COMPUTER;
                break;
            
            case CHAR_TEX_DIRT: 
				texture_type = DECAL_DIRT;
				break;

            case CHAR_TEX_WOOD:
				texture_type = DECAL_WOOD;
                break;
            
			case CHAR_TEX_SLOSH:
                break;

            case CHAR_TEX_GLASS:
          //  case CHAR_TEX_COMPUTER:
                break;

            // don't do anything if those textures (perhaps add something later...)
            default:
				texture_type = DECAL_GENERIC;
			break;
        }

		switch( iBulletType )
		{
	
				case BULLET_MONSTER_12MM:
				EV_HLDM_GunshotDecalTrace( pTrace, EV_HLDM_DamageDecal( pe, DECAL_SHOTPULSE ), DECAL_SHOTPULSE );
				break;

				case BULLET_MONSTER_AK:
				case BULLET_MONSTER_MP5:
				case BULLET_MONSTER_9MM:
				case BULLET_PLAYER_9MM:	// Colt 1911	
				case BULLET_PLAYER_MP5:	//MP5
				case BULLET_PLAYER_AK74:
				case BULLET_PLAYER_DEAGLE:
				case BULLET_PLAYER_SNIPER:
				case BULLET_PLAYER_UZI:
				case BULLET_PLAYER_FAMAS:
				case BULLET_PLAYER_BER92F:
				case BULLET_PLAYER_GLOCK18:
				case BULLET_PLAYER_P90:
				case BULLET_PLAYER_USAS:
				case BULLET_PLAYER_BUCKSHOT:
				case BULLET_PLAYER_357:
				case BULLET_PLAYER_IRGUN:
			default:
				//debe ser aca
				EV_HLDM_GunshotDecalTrace( pTrace, EV_HLDM_DamageDecal( pe, texture_type ), texture_type );
			break;
		}
	}
}

int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount )
{
	int tracer = 0;
	int i;
	qboolean player = idx >= 1 && idx <= gEngfuncs.GetMaxClients() ? true : false;
	pmtrace_t tr, beam_tr;

	if ( iTracerFreq != 0 && ( (*tracerCount)++ % iTracerFreq) == 0 )
	{
		vec3_t vecTracerSrc;

		if ( player )
		{
			vec3_t offset( 0, 0, -4 );

			// adjust tracer position for player
			for ( i = 0; i < 3; i++ )
			{
				vecTracerSrc[ i ] = vecSrc[ i ] + offset[ i ] + right[ i ] * 2 + forward[ i ] * 16;
			}
		}
		else
		{
			VectorCopy( vecSrc, vecTracerSrc );
		}
		
		if ( iTracerFreq != 1 )		// guns that always trace also always decal
			tracer = 1;

		switch( iBulletType )
		{
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_MONSTER_9MM:
		case BULLET_MONSTER_12MM:
		default:
			EV_CreateTracer( vecTracerSrc, end );
			break;
		}
	}

	return tracer;
}


/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
//void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY )
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType,
    int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY , float recoil)
{
	int i;
	pmtrace_t tr;
	int iShot;
	int tracer;

	for ( iShot = 1; iShot <= cShots; iShot++ )	
	{
		if ( EV_IsLocal( idx ) )
{
vec3_t vec_Recoil;
gEngfuncs.GetViewAngles(vec_Recoil);
vec_Recoil.x -= recoil;
int i = gEngfuncs.pfnRandomLong(0, 1);
 if(i)
  {
    vec_Recoil.y -= recoil/2; //halbe stärke
    vec_Recoil.z -= recoil/2; //auch nur die halbe!
  }
  else
   {
    vec_Recoil.y += recoil/2; //halbe stärke</span>
    vec_Recoil.z += recoil/2; //auch nur die halbe!
  }
//nicht getestet
  gEngfuncs.SetViewAngles(vec_Recoil); //setzt den ViewAngle auf vec_Recoil.
}
		vec3_t vecDir, vecEnd;
			
		float x, y, z;
		//We randomize for the Shotgun.
		if ( iBulletType == BULLET_PLAYER_BUCKSHOT || BULLET_PLAYER_FAMAS )
		{
			do {
				x = gEngfuncs.pfnRandomFloat(-0.5,0.5) + gEngfuncs.pfnRandomFloat(-0.5,0.5);
				y = gEngfuncs.pfnRandomFloat(-0.5,0.5) + gEngfuncs.pfnRandomFloat(-0.5,0.5);
				z = x*x+y*y;
			} while (z > 1);

			for ( i = 0 ; i < 3; i++ )
			{
				vecDir[i] = vecDirShooting[i] + x * flSpreadX * right[ i ] + y * flSpreadY * up [ i ];
				vecEnd[i] = vecSrc[ i ] + flDistance * vecDir[ i ];
			}
		}//But other guns already have their spread randomized in the synched spread.
		else
		{

			for ( i = 0 ; i < 3; i++ )
			{
				vecDir[i] = vecDirShooting[i] + flSpreadX * right[ i ] + flSpreadY * up [ i ];
				vecEnd[i] = vecSrc[ i ] + flDistance * vecDir[ i ];
			}
		}

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );
	
		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();
	
		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );

		tracer = EV_HLDM_CheckTracer( idx, vecSrc, tr.endpos, forward, right, iBulletType, iTracerFreq, tracerCount );
	
		EV_HLDM_BulletFlyBySound( idx, vecSrc, vecEnd );

		// do damage, paint decals
		if ( tr.fraction != 1.0 )
		{
			switch(iBulletType)
			{			
				case BULLET_MONSTER_AK:
				case BULLET_MONSTER_MP5:
				case BULLET_MONSTER_9MM:
				case BULLET_PLAYER_9MM:	// Colt 1911	
				case BULLET_PLAYER_MP5:	//MP5
				case BULLET_PLAYER_AK74:
				case BULLET_PLAYER_DEAGLE:
				case BULLET_PLAYER_SNIPER:
				case BULLET_PLAYER_UZI:
				case BULLET_PLAYER_FAMAS:
				case BULLET_PLAYER_BER92F:
				case BULLET_PLAYER_GLOCK18:
				case BULLET_PLAYER_P90:
				case BULLET_PLAYER_USAS:
				case BULLET_PLAYER_BUCKSHOT:
				case BULLET_PLAYER_357:
				case BULLET_PLAYER_IRGUN:
				default:	
					if (CVAR_GET_FLOAT("cl_removeclienteffects") == 0)
					{
						EV_HLDM_PlayTextureSound( idx, &tr, vecSrc, vecEnd, iBulletType );
						EV_HLDM_DecalGunshot( &tr, iBulletType, vecSrc, vecEnd );
						EV_HLDM_SmokePuff( &tr, vecSrc, vecEnd );
					}
				break;
			}
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

//======================
//	    IRGUN
//======================

void EV_FireIRGUN( event_args_t *args )
{
  int idx;
  vec3_t origin;
  vec3_t angles;
  vec3_t velocity;
  int empty;
  int i;
  vec3_t ShellVelocity;
  vec3_t ShellOrigin;
//  int shell;
  vec3_t vecSrc, vecAiming;
  vec3_t vecSpread;
  vec3_t up, right, forward;
  float flSpread = 0.01;
  idx = args->entindex;
  VectorCopy( args->origin, origin );
  VectorCopy( args->angles, angles );
  VectorCopy( args->velocity, velocity );
int m_iGlow;

  empty = args->bparam1;
  AngleVectors( angles, forward, right, up );
  
//  shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/shell.mdl");
  
  if ( EV_IsLocal( idx ) )
  {
    EV_MuzzleFlash();
    gEngfuncs.pEventAPI->EV_WeaponAnimation( empty ? IRGUN_FIRE1 : IRGUN_FIRE1, 2 );
  }
  
//  EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );
//  EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 
  	
		switch( gEngfuncs.pfnRandomLong( 0, 2 ) )
		{
			case 0:
  gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/irgun/irgun_fire-1.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
			case 1:
  gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/irgun/irgun_fire-2.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
			case 2:
  gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/irgun/irgun_fire-3.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		}
		
  
  EV_GetGunPosition( args, vecSrc, origin );
  
  VectorCopy( forward, vecAiming );
  
  for ( i = 0; i < 3; i++ )
  {
    vecSpread[ i ] = flSpread;
  }
  
	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_IRGUN, 2, &tracerCount[idx-1], args->fparam1, args->fparam2, 0.2 );
		
	if ( EV_IsLocal( idx ) )
	{
		V_PunchAxis( 0, -2.0 );
	}

	//NEW CODE
	vec3_t vecEnd;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	VectorCopy( args->velocity, velocity );
	
	AngleVectors( angles, forward, right, up );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorMA( vecSrc, 8192, forward, vecEnd );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );
	
	//We hit something
	if ( tr.fraction < 1.0 )
	{
		physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent ); 

		//Not the world, let's assume we hit something organic ( dog, cat, uncle joe, etc ).
		if ( pe->solid != SOLID_BSP )
		{
			gEngfuncs.pEventAPI->EV_PlaySound( idx, tr.endpos, CHAN_BODY, "weapons/irgun/nearmiss.wav", 1, ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		}
		else if ( pe->rendermode == kRenderNormal ) 
		{
			gEngfuncs.pEventAPI->EV_PlaySound( 0, tr.endpos, CHAN_BODY, "weapons/irgun/nearmiss.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
		
			//Not underwater, do some sparks...
			if ( gEngfuncs.PM_PointContents( tr.endpos, NULL ) != CONTENTS_WATER)
				 gEngfuncs.pEfxAPI->R_SparkShower( tr.endpos );	
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();


	m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/iRifle_hit.spr" );
				
//	gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_SPRANIMATE); //1 FTENT_FADEOUT ); fix
	gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT );
}
  
//======================
//	    IRGUN
//======================

//======================
//	    GLOCK START
//======================
void EV_FireGlock1( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;


// Recoil -Start
    vec3_t viewangles;
    float staerkedesrecoils = 0.20; // Cantidad de recoil 1.00
// Recoil End

	int empty;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	
	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	empty = args->bparam1;
	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_9mm.mdl");// brass shell

	if ( EV_IsLocal( idx ) )
	{
        // Code Recoil Start
        gEngfuncs.GetViewAngles( (float *)viewangles );
        viewangles[PITCH] -= staerkedesrecoils;
        gEngfuncs.SetViewAngles( (float *)viewangles );
        // Code Recoil End

		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( empty ? GLOCK_FIRE_LAST : GLOCK_FIRE1, 2 );

		V_PunchAxis( 0, -2.0 );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/1911/1911_fire-1.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	
	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, 0, args->fparam1, args->fparam2, PISTOL_RECOIL );
}

void EV_FireGlock2( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_9mm.mdl");// brass shell

	if ( EV_IsLocal( idx ) )
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( GLOCK_FIRE1, 2 );

		V_PunchAxis( 0, -2.0 );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/1911/1911_fire-1.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	
	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx-1], args->fparam1, args->fparam2, PISTOL_RECOIL );
	
}
//======================
//	   GLOCK END
//======================

//======================
//	  SHOTGUN START
//======================
void EV_FireShotGunDouble( event_args_t *args )
{//THE PUMP
	int idx;
	vec3_t origin;
	vec3_t angles;

	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_buckshot.mdl");// brass shell

//	if ( EV_IsLocal( idx ) )
//	{
		// Add muzzle flash to current weapon model
//		EV_MuzzleFlash();
//		gEngfuncs.pEventAPI->EV_WeaponAnimation( SHOTGUN_PUMP1, 2 );
	//	V_PunchAxis( 0, -2.0 );
//	}
	

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHOTSHELL ); 

//	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/spas12/spas12-pump.wav", gEngfuncs.pfnRandomFloat(0.98, 1.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );

//	EV_GetGunPosition( args, vecSrc, origin );
//	VectorCopy( forward, vecAiming );

//	if ( gEngfuncs.GetMaxClients() > 1 )
//	{
//		EV_HLDM_FireBullets( idx, forward, right, up, 8, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.17365, 0.04362 );
//	}
///	else
//	{
//		EV_HLDM_FireBullets( idx, forward, right, up, 12, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.08716, 0.08716, SHOTGUN_RECOIL );
//	}
}

void EV_FireShotGunSingle( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
//	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

//	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_buckshot.mdl");// brass shell
		
	int empty;
	empty = args->bparam1;

	if ( EV_IsLocal( idx ) )
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
					
	//	gEngfuncs.pEventAPI->EV_WeaponAnimation( SHOTGUN_NEWFIRE1 + gEngfuncs.pfnRandomLong(0,1), 1 ); //pfnRandomLong(0,1), 1 );
	//	V_PunchAxis( 0, -8.0 );
																									
		switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
		{
			case 0:
			gEngfuncs.pEventAPI->EV_WeaponAnimation( empty ? SHOTGUN_FIRE1 : SHOTGUN_NEWFIRE1, 0 ); break;
			V_PunchAxis( 0, -7.0 );
			case 1:
			gEngfuncs.pEventAPI->EV_WeaponAnimation( empty ? SHOTGUN_FIRE1 : SHOTGUN_NEWFIRE1, 0 ); break;
			V_PunchAxis( 0, -8.0 );
		}
	}

//	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6 );

//	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHOTSHELL ); 

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/spas12/spas12-fire.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );

	EV_GetGunPosition( args, vecSrc, origin );

	// mazor begin
	EV_HLDM_MuzzleFlash( vecSrc, 1.2 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
	// mazor end

	VectorCopy( forward, vecAiming );

//	if ( gEngfuncs.GetMaxClients() > 1 )
//	{
//		EV_HLDM_FireBullets( idx, forward, right, up, 4, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.08716, 0.04362 );
//	}
//	else
//	{
		EV_HLDM_FireBullets( idx, forward, right, up, 6, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.08716, 0.08716, RIFLE_RECOIL );
//	}
}

void EV_SpasShell( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_buckshot.mdl");// brass shell
		
	int empty;
	empty = args->bparam1;

	if ( EV_IsLocal( idx ) )
	{
	//	V_PunchAxis( 0, -3.0 );
	}
	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );
	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHOTSHELL ); 
}

void EV_SniperShell( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_sniper.mdl");// brass shell
		
	int empty;
	empty = args->bparam1;

	if ( EV_IsLocal( idx ) )
	{
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );
	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 
}

//======================
//	   SHOTGUN END
//======================

enum mymp5_e
{
	M16_LONGIDLE = 0,
	M16_IDLE1,
	M16_LAUNCH,
	M16_RELOAD,
	M16_DEPLOY,
	M16_FIRE1,
	M16_FIRE2,
	M16_FIRE3,
	M16_HOLSTER,
};

//======================
//	    M16 START
//======================
void EV_FireM16( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/shell.mdl");// brass shell
	
	if ( EV_IsLocal( idx ) )
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( M16_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2 );

	//	V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -2, 2 ) );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m16/m16_fire-1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m16/m16_fire-2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}

	EV_GetGunPosition( args, vecSrc, origin );

	// mazor begin
	EV_HLDM_MuzzleFlash( vecSrc, 1.5 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
	// mazor end

	VectorCopy( forward, vecAiming );
	//usando BULLET_PLAYER_MP5 por que tiene tracers... m)
	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP5, 0, &tracerCount[idx-1], args->fparam1, args->fparam2, RIFLE_RECOIL );
}

// We only predict the animation and sound
// The grenade is still launched from the server.
void EV_FireM162( event_args_t *args )
{
	int idx;
	vec3_t origin;
	
	idx = args->entindex;
	VectorCopy( args->origin, origin );

	if ( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( M16_LAUNCH, 2 );
		V_PunchAxis( 0, -10 );
	}
	
	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glauncher.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glauncher2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}
}
//======================
//		 M16END
//======================
//======================
//	   PHYTON START 
//	     ( .357 )
//======================
void EV_FirePython( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		// Python uses different body in multiplayer versus single player
		int multiplayer = gEngfuncs.GetMaxClients() == 1 ? 0 : 1;

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( PYTHON_FIRE1, multiplayer ? 1 : 0 );

		V_PunchAxis( 0, -10.0 );
	}

	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/357/357_FIRE-1.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/357/357_FIRE-2.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM );
		break;
	}

	EV_GetGunPosition( args, vecSrc, origin );
	
	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_357, 0, 0, args->fparam1, args->fparam2, RIFLE_RECOIL );	//??
}
//======================
//	    PHYTON END 
//	     ( .357 )
//======================

//======================
//	   GAUSS START 
//======================
#define SND_CHANGE_PITCH	(1<<7)		// duplicated in protocol.h change sound pitch

enum gauss_e 
{
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
//	GAUSS_FIDGET,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_DRAW,
	GAUSS_HOLSTER,
};

void EV_SpinGauss( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int iSoundState = 0;

	int pitch;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	pitch = args->iparam1;

	iSoundState = args->bparam1 ? SND_CHANGE_PITCH : 0;

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, iSoundState, pitch );
}

/*
==============================
EV_StopPreviousGauss

==============================
*/
void EV_StopPreviousGauss( int idx )
{
	// Make sure we don't have a gauss spin event in the queue for this guy
	gEngfuncs.pEventAPI->EV_KillEvents( idx, "scripts/events/gaussspin.sc" );
	gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_WEAPON, "ambience/pulsemachine.wav" );
}

extern float g_flApplyVel;

void EV_FireGauss( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	float flDamage = args->fparam1;
	int primaryfire = args->bparam1;

	int m_fPrimaryFire = args->bparam1;
	int m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	vec3_t vecSrc;
	vec3_t vecDest;
	edict_t		*pentIgnore;
	pmtrace_t tr, beam_tr;
	float flMaxFrac = 1.0;
	int	nTotal = 0;
	int fHasPunched = 0;
	int fFirstBeam = 1;
	int	nMaxHits = 10;
	physent_t *pEntity;
	int m_iBeam, m_iGlow, m_iBalls;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	if ( args->bparam2 )
	{
		EV_StopPreviousGauss( idx );
		return;
	}

//	Con_Printf( "Firing gauss with %f\n", flDamage );
	EV_GetGunPosition( args, vecSrc, origin );

	m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/smoke.spr" );
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/hotglow.spr" );
	
	AngleVectors( angles, forward, right, up );

	VectorMA( vecSrc, 8192, forward, vecDest );

	if ( EV_IsLocal( idx ) )
	{
		V_PunchAxis( 0, -2.0 );
		gEngfuncs.pEventAPI->EV_WeaponAnimation( GAUSS_FIRE2, 2 );

		if ( m_fPrimaryFire == false )
			 g_flApplyVel = flDamage;	
			 
	}

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/gauss2.wav", 0.5 + flDamage * (1.0 / 400.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );

	while (flDamage > 10 && nMaxHits > 0)
	{
		nMaxHits--;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );
		
		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();
	
		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecDest, PM_STUDIO_BOX, -1, &tr );

		gEngfuncs.pEventAPI->EV_PopPMStates();

		if ( tr.allsolid )
			break;

		if (fFirstBeam)
		{
			if ( EV_IsLocal( idx ) )
			{
				// Add muzzle flash to current weapon model
				EV_MuzzleFlash();
			}
			fFirstBeam = 0;

			gEngfuncs.pEfxAPI->R_BeamEntPoint( 
				idx | 0x1000,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				m_fPrimaryFire ? 128.0 : flDamage,
				0,
				0,
				0,
				m_fPrimaryFire ? 255 : 255,
				m_fPrimaryFire ? 128 : 255,
				m_fPrimaryFire ? 0 : 255
			);
		}
		else
		{
			gEngfuncs.pEfxAPI->R_BeamPoints( vecSrc,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				m_fPrimaryFire ? 128.0 : flDamage,
				0,
				0,
				0,
				m_fPrimaryFire ? 255 : 255,
				m_fPrimaryFire ? 128 : 255,
				m_fPrimaryFire ? 0 : 255
			);
		}

		pEntity = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent );
		if ( pEntity == NULL )
			break;

		if ( pEntity->solid == SOLID_BSP )
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct( tr.plane.normal, forward );

			if (n < 0.5) // 60 degrees	
			{
				//ALERT( at_console, "reflect %f\n", n );
				// reflect
				vec3_t r;
			
				VectorMA( forward, 2.0 * n, tr.plane.normal, r );

				flMaxFrac = flMaxFrac - tr.fraction;
				
				VectorCopy( r, forward );

				VectorMA( tr.endpos, 8.0, forward, vecSrc );
				VectorMA( vecSrc, 8192.0, forward, vecDest );

				gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT );

				vec3_t fwd;
				VectorAdd( tr.endpos, tr.plane.normal, fwd );

				gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
									255, 100 );

				// lose energy
				if ( n == 0 )
				{
					n = 0.1;
				}
				
				flDamage = flDamage * (1 - n);

			}
			else
			{
				// tunnel
//				EV_HLDM_DecalGunshot( &tr, BULLET_MONSTER_12MM,vecSrc );
				EV_HLDM_DecalGunshot( &tr, BULLET_MONSTER_12MM, vecSrc, vecDest );
				gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 1.0, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT );

				// limit it to one hole punch
				if (fHasPunched)
				{
					break;
				}
				fHasPunched = 1;
				
				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if ( !m_fPrimaryFire )
				{
					vec3_t start;

					VectorMA( tr.endpos, 8.0, forward, start );

					// Store off the old count
					gEngfuncs.pEventAPI->EV_PushPMStates();
						
					// Now add in all of the players.
					gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );

					gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
					gEngfuncs.pEventAPI->EV_PlayerTrace( start, vecDest, PM_STUDIO_BOX, -1, &beam_tr );

					if ( !beam_tr.allsolid )
					{
						vec3_t delta;
						float n;

						// trace backwards to find exit point

						gEngfuncs.pEventAPI->EV_PlayerTrace( beam_tr.endpos, tr.endpos, PM_STUDIO_BOX, -1, &beam_tr );

						VectorSubtract( beam_tr.endpos, tr.endpos, delta );
						
						n = Length( delta );

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;
							flDamage -= n;

							// absorption balls
							{
								vec3_t fwd;
								VectorSubtract( tr.endpos, forward, fwd );
								gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
									255, 100 );
							}

	//////////////////////////////////// WHAT TO DO HERE
							// CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

//							EV_HLDM_DecalGunshot( &beam_tr, BULLET_MONSTER_12MM,vecSrc, vecEnd );
							EV_HLDM_DecalGunshot( &beam_tr, BULLET_MONSTER_12MM, vecSrc, vecDest );

							gEngfuncs.pEfxAPI->R_TempSprite( beam_tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT );
			
							// balls
							{
								vec3_t fwd;
								VectorSubtract( beam_tr.endpos, forward, fwd );
								gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, beam_tr.endpos, fwd, m_iBalls, (int)(flDamage * 0.3), 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 200,
									255, 40 );
							}
							
							VectorAdd( beam_tr.endpos, forward, vecSrc );
						}
					}
					else
					{
						flDamage = 0;
					}

					gEngfuncs.pEventAPI->EV_PopPMStates();
				}
				else
				{
					if ( m_fPrimaryFire )
					{
						// slug doesn't punch through ever with primary 
						// fire, so leave a little glowy bit and make some balls
						gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT );
			
						{
							vec3_t fwd;
							VectorAdd( tr.endpos, tr.plane.normal, fwd );
							gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
								255, 200 );
						}
					}

					flDamage = 0;
				}
			}
		}
		else
		{
			VectorAdd( tr.endpos, forward, vecSrc );
		}
	}
}
//======================
//	   GAUSS END 
//======================

//======================
//	   CROWBAR START
//======================

enum crowbar_e 
{
	KNIFE_IDLE = 0,
	KNIFE_SLASH1,
	KNIFE_SLASH2,
	KNIFE_DEPLOY,
	KNIFE_STAB_HIT,
	KNIFE_STAB,
	KNIFE_HOLSTER,
};

int g_iSwing;
//int g_iSwing2;

//Only predict the miss sounds, hit sounds are still played 
//server side, so players don't get the wrong idea.
void EV_Knife( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	
	//Play Swing sound
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife/knife_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM); 

	 
	if ( EV_IsLocal( idx ) )
	{
		V_PunchAxis( 0, -2.0 );
		gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_SLASH1, 1 );

		switch( (g_iSwing++) % 3 )
		{
			case 0:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( KNIFE_SLASH1, 1 ); break;
			case 1:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( KNIFE_SLASH2, 1 ); break;
			case 2:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( KNIFE_SLASH1, 1 ); break;


		}
	}
}

//server side, so players don't get the wrong idea.
void EV_Knife_stab( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	
	//Play Swing sound
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/knife/knife_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM); 

	 
	if ( EV_IsLocal( idx ) )
	{
		V_PunchAxis( 0, -2.0 );
		gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_STAB, 1 );

		switch( (g_iSwing++) % 3 )
		{
			case 0:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( KNIFE_STAB, 1 ); break;
			case 1:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( KNIFE_STAB, 1 ); break;
			case 2:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( KNIFE_STAB, 1 ); break;


		}
	}
}

//======================
//	  CROSSBOW START
//======================

enum crossbow_e {
	CROSSBOW_IDLE1 = 0,	// full
	CROSSBOW_IDLE2,		// empty
	CROSSBOW_FIDGET1,	// full
	CROSSBOW_FIDGET2,	// empty
	CROSSBOW_FIRE1,		// full
	CROSSBOW_FIRE_VACIO,// reload
	CROSSBOW_FIRE3,		// empty
	CROSSBOW_RELOAD,	// from empty
	CROSSBOW_DRAW1,		// full
	CROSSBOW_DRAW2,		// empty
	CROSSBOW_HOLSTER1,	// full
	CROSSBOW_HOLSTER2,	// empty
};

//=====================
// EV_BoltCallback
// This function is used to correct the origin and angles 
// of the bolt, so it looks like it's stuck on the wall.
//=====================
void EV_BoltCallback ( struct tempent_s *ent, float frametime, float currenttime )
{
	ent->entity.origin = ent->entity.baseline.vuser1;
	ent->entity.angles = ent->entity.baseline.vuser2;
}

void EV_FireCrossbow2( event_args_t *args )
{
	vec3_t vecSrc, vecEnd;
	vec3_t up, right, forward;
	pmtrace_t tr;

	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	VectorCopy( args->velocity, velocity );
	
	AngleVectors( angles, forward, right, up );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorMA( vecSrc, 8192, forward, vecEnd );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0,0xF) );
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0,0xF) );

	if ( EV_IsLocal( idx ) )
	{
		if ( args->iparam1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE1, 1 );
		else if ( args->iparam2 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE1, 1 );
	}

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );
	
	//We hit something
	if ( tr.fraction < 1.0 )
	{
		physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent ); 

		//Not the world, let's assume we hit something organic ( dog, cat, uncle joe, etc ).
		if ( pe->solid != SOLID_BSP )
		{
			switch( gEngfuncs.pfnRandomLong(0,1) )
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, PITCH_NORM ); break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, PITCH_NORM ); break;
			}
		}
		//Stick to world but don't stick to glass, it might break and leave the bolt floating. It can still stick to other non-transparent breakables though.
		else if ( pe->rendermode == kRenderNormal ) 
		{
			gEngfuncs.pEventAPI->EV_PlaySound( 0, tr.endpos, CHAN_BODY, "weapons/xbow_hit1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, PITCH_NORM );
		
			//Not underwater, do some sparks...
			if ( gEngfuncs.PM_PointContents( tr.endpos, NULL ) != CONTENTS_WATER)
				 gEngfuncs.pEfxAPI->R_SparkShower( tr.endpos );

			vec3_t vBoltAngles;
			int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/crossbow_bolt.mdl" );

			VectorAngles( forward, vBoltAngles );

			TEMPENTITY *bolt = gEngfuncs.pEfxAPI->R_TempModel( tr.endpos - forward * 10, Vector( 0, 0, 0), vBoltAngles , 5, iModelIndex, TE_BOUNCE_NULL );
			
			if ( bolt )
			{
				bolt->flags |= ( FTENT_CLIENTCUSTOM ); //So it calls the callback function.
				bolt->entity.baseline.vuser1 = tr.endpos - forward * 10; // Pull out a little bit
				bolt->entity.baseline.vuser2 = vBoltAngles; //Look forward!
				bolt->callback = EV_BoltCallback; //So we can set the angles and origin back. (Stick the bolt to the wall)
			}
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

//TODO: Fully predict the fliying bolt.
void EV_FireCrossbow( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0,0xF) );
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0,0xF) );

	//Only play the weapon anims if I shot it. 
	if ( EV_IsLocal( idx ) )
	{
		if ( args->iparam1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE1, 1 );
		else if ( args->iparam2 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE1, 1 );

		V_PunchAxis( 0, -2.0 );
	}
}

//======================
//	   CROSSBOW END 
//======================

//======================
//	    RPG START 
//======================
enum rpg_e {
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,		// to reload
	RPG_FIRE2,		// to empty
	RPG_HOLSTER1,	// loaded
	RPG_DRAW1,		// loaded
	RPG_HOLSTER2,	// unloaded
	RPG_DRAW_UL,	// unloaded
	RPG_IDLE_UL,	// unloaded idle
	RPG_FIDGET_UL,	// unloaded fidget
};

void EV_FireRpg( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/rocketfire1.wav", 0.9, ATTN_NORM, 0, PITCH_NORM );
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/glauncher.wav", 0.7, ATTN_NORM, 0, PITCH_NORM );

	//Only play the weapon anims if I shot it. 
	if ( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( RPG_FIRE2, 1 );
	
	//	V_PunchAxis( 0, -5.0 );//more!
		V_PunchAxis( 0, -8.0 );
	}
}
//======================
//	     RPG END 
//======================

//======================
//	    EGON END 
//======================
enum egon_e {
	EGON_IDLE1 = 0,
	EGON_FIDGET1,
	EGON_ALTFIREON,
	EGON_ALTFIRECYCLE,
	EGON_ALTFIREOFF,
	EGON_FIRE1,
	EGON_FIRE2,
	EGON_FIRE3,
	EGON_FIRE4,
	EGON_DRAW,
	EGON_HOLSTER
};

int g_fireAnims1[] = { EGON_FIRE1, EGON_FIRE2, EGON_FIRE3, EGON_FIRE4 };
int g_fireAnims2[] = { EGON_ALTFIRECYCLE };

enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE};

#define	EGON_PRIMARY_VOLUME		450
#define EGON_BEAM_SPRITE		"sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE		"sprites/XSpark1.spr"
#define EGON_SOUND_OFF			"weapons/egon_off1.wav"
#define EGON_SOUND_RUN			"weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP		"weapons/egon_windup2.wav"

#define ARRAYSIZE(p)		(sizeof(p)/sizeof(p[0]))

BEAM *pBeam;
BEAM *pBeam2;

void EV_EgonFire( event_args_t *args )
{
	int idx, iFireState, iFireMode;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	iFireState = args->iparam1;
	iFireMode = args->iparam2;
	int iStartup = args->bparam1;


	if ( iStartup )
	{
		if ( iFireMode == FIRE_WIDE )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125 );
		else
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100 );
	}
	else
	{
		if ( iFireMode == FIRE_WIDE )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.98, ATTN_NORM, 0, 125 );
		else
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.9, ATTN_NORM, 0, 100 );
	}

	//Only play the weapon anims if I shot it.
	if ( EV_IsLocal( idx ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation ( g_fireAnims1[ gEngfuncs.pfnRandomLong( 0, 3 ) ], 1 );

	if ( iStartup == 1 && EV_IsLocal( idx ) && !pBeam && !pBeam2 && cl_lw->value ) //Adrian: Added the cl_lw check for those lital people that hate weapon prediction.
	{
		vec3_t vecSrc, vecEnd, origin, angles, forward, right, up;
		pmtrace_t tr;

		cl_entity_t *pl = gEngfuncs.GetEntityByIndex( idx );

		if ( pl )
		{
			VectorCopy( gHUD.m_vecAngles, angles );
			
			AngleVectors( angles, forward, right, up );

			EV_GetGunPosition( args, vecSrc, pl->origin );

			VectorMA( vecSrc, 2048, forward, vecEnd );

			gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );	
				
			// Store off the old count
			gEngfuncs.pEventAPI->EV_PushPMStates();
			
			// Now add in all of the players.
			gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	

			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );

			gEngfuncs.pEventAPI->EV_PopPMStates();

			int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( EGON_BEAM_SPRITE );

			float r = 50.0f;
			float g = 50.0f;
			float b = 125.0f;

			if ( IEngineStudio.IsHardware() )
			{
				r /= 100.0f;
				g /= 100.0f;
			}
				
		
			pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint ( idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.5, 0.2, 0.7, 55, 0, 0, r, g, b );

			if ( pBeam )
				 pBeam->flags |= ( FBEAM_SINENOISE );
 
			pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint ( idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 5.0, 0.08, 0.7, 25, 0, 0, r, g, b );
		}
	}
}

void EV_EgonStop( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy ( args->origin, origin );

	gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, EGON_SOUND_RUN );
	
	if ( args->iparam1 )
		 gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_OFF, 0.98, ATTN_NORM, 0, 100 );

	if ( EV_IsLocal( idx ) ) 
	{
		if ( pBeam )
		{
			pBeam->die = 0.0;
			pBeam = NULL;
		}
			
		
		if ( pBeam2 )
		{
			pBeam2->die = 0.0;
			pBeam2 = NULL;
		}
	}
}
//======================
//	    EGON END 
//======================

//======================
//	   HORNET START
//======================
enum hgun_e {
	HGUN_IDLE1 = 0,
	HGUN_FIDGETSWAY,
	HGUN_FIDGETSHAKE,
	HGUN_DOWN,
	HGUN_UP,
	HGUN_SHOOT
};

void EV_HornetGunFire( event_args_t *args )
{
	int idx, iFireMode;
	vec3_t origin, angles, vecSrc, forward, right, up;


	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	iFireMode = args->iparam1;

	//Only play the weapon anims if I shot it.
	if ( EV_IsLocal( idx ) )
	{
		V_PunchAxis( 0, gEngfuncs.pfnRandomLong ( 0, 2 ) );
		gEngfuncs.pEventAPI->EV_WeaponAnimation ( HGUN_SHOOT, 1 );
	}

	switch ( gEngfuncs.pfnRandomLong ( 0 , 2 ) )
	{
		case 0:	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "agrunt/ag_fire1.wav", 1, ATTN_NORM, 0, 100 );	break;
		case 1:	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "agrunt/ag_fire2.wav", 1, ATTN_NORM, 0, 100 );	break;
		case 2:	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "agrunt/ag_fire3.wav", 1, ATTN_NORM, 0, 100 );	break;
	}
}
//======================
//	   HORNET END
//======================

//======================
//	   TRIPMINE START
//======================
enum tripmine_e {
	TRIPMINE_IDLE1 = 0,
	TRIPMINE_IDLE2,
	TRIPMINE_ARM1,
	TRIPMINE_ARM2,
	TRIPMINE_FIDGET,
	TRIPMINE_HOLSTER,
	TRIPMINE_DRAW,
	TRIPMINE_WORLD,
	TRIPMINE_GROUND,
};

//We only check if it's possible to put a trip mine
//and if it is, then we play the animation. Server still places it.
void EV_TripmineFire( event_args_t *args )
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy( args->origin, vecSrc );
	VectorCopy( args->angles, angles );

	AngleVectors ( angles, forward, NULL, NULL );
		
	if ( !EV_IsLocal ( idx ) )
		return;

	// Grab predicted result for local player
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight( view_ofs );

	vecSrc = vecSrc + view_ofs;

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecSrc + forward * 128, PM_NORMAL, -1, &tr );

	//Hit something solid
	if ( tr.fraction < 1.0 )
		 gEngfuncs.pEventAPI->EV_WeaponAnimation ( TRIPMINE_DRAW, 0 );
	
	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   TRIPMINE END
//======================

//======================
//	   SQUEAK START
//======================
enum squeak_e {
	SQUEAK_IDLE1 = 0,
	SQUEAK_FIDGETFIT,
	SQUEAK_FIDGETNIP,
	SQUEAK_DOWN,
	SQUEAK_UP,
	SQUEAK_THROW
};

#define VEC_HULL_MIN		Vector(-16, -16, -36)
#define VEC_DUCK_HULL_MIN	Vector(-16, -16, -18 )

void EV_SnarkFire( event_args_t *args )
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy( args->origin, vecSrc );
	VectorCopy( args->angles, angles );

	AngleVectors ( angles, forward, NULL, NULL );
		
	if ( !EV_IsLocal ( idx ) )
		return;
	
	if ( args->ducking )
		vecSrc = vecSrc - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	
	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc + forward * 20, vecSrc + forward * 64, PM_NORMAL, -1, &tr );

	//Find space to drop the thing.
	if ( tr.allsolid == 0 && tr.startsolid == 0 && tr.fraction > 0.25 )
		 gEngfuncs.pEventAPI->EV_WeaponAnimation ( SQUEAK_THROW, 0 );
	
	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   SQUEAK END
//======================

void EV_TrainPitchAdjust( event_args_t *args )
{
	int idx;
	vec3_t origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	int stop;
	
	char sz[ 256 ];

	idx = args->entindex;
	
	VectorCopy( args->origin, origin );

	us_params = (unsigned short)args->iparam1;
	stop	  = args->bparam1;

	m_flVolume	= (float)(us_params & 0x003f)/40.0;
	noise		= (int)(((us_params) >> 12 ) & 0x0007);
	pitch		= (int)( 10.0 * (float)( ( us_params >> 6 ) & 0x003f ) );

	switch ( noise )
	{
	case 1: strcpy( sz, "plats/ttrain1.wav"); break;
	case 2: strcpy( sz, "plats/ttrain2.wav"); break;
	case 3: strcpy( sz, "plats/ttrain3.wav"); break; 
	case 4: strcpy( sz, "plats/ttrain4.wav"); break;
	case 5: strcpy( sz, "plats/ttrain6.wav"); break;
	case 6: strcpy( sz, "plats/ttrain7.wav"); break;
	default:
		// no sound
		strcpy( sz, "" );
		return;
	}

	if ( stop )
	{
		gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, sz );
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch );
	}
}




int EV_TFC_IsAllyTeam( int iTeam1, int iTeam2 )
{
	return 0;
}

void EV_HLDM_BulletFlyBySound ( int idx, vec3_t start, vec3_t end )
{
	// make the incidental sounds - all of these should already be precached on the server
	vec3_t	soundPoint;
	char *zngs[17];
	int cnt;
	int iRand;  // sound randomizer

	// so where are we standing now?	
	cl_entity_t *pthisplayer = gEngfuncs.GetLocalPlayer();

	iRand = gEngfuncs.pfnRandomLong(1,10);

	// Dunno should we use viewangles or not?
	soundPoint = Vector( 0, 0, 0 );

	if ( !EV_IsLocal(idx) )
	{
		// did the bullet just pass our radius based on our origin?
		if( EV_PointLineIntersect(start, end, pthisplayer->origin, 150, soundPoint ) )
		{
			// if so play flyby sound
			if (iRand < 5)
			{
				zngs[0]		= "weapons/whizz1.wav";
				zngs[1]		= "weapons/whizz2.wav";
				zngs[2]		= "weapons/whizz3.wav";
				zngs[3]		= "weapons/whizz4.wav";
				zngs[4]		= "weapons/whizz5.wav";
				zngs[5]		= "weapons/whizz6.wav";
				zngs[6]		= "weapons/whizz7.wav";
				zngs[7]		= "weapons/whizz8.wav";
				zngs[8]		= "weapons/whizz9.wav";
				zngs[9]		= "weapons/whizz10.wav";
				zngs[10]	= "weapons/whizz11.wav";
				zngs[11]	= "weapons/whizz12.wav";
				zngs[12]	= "weapons/whizz13.wav";
				zngs[13]	= "weapons/whizz14.wav";
				zngs[14]	= "weapons/whizz15.wav";
				zngs[15]	= "weapons/whizz16.wav";
				zngs[16]	= "weapons/whizz17.wav";		
				cnt = 17;
				
				gEngfuncs.pEventAPI->EV_PlaySound( pthisplayer->index, soundPoint,
					CHAN_STATIC, zngs[gEngfuncs.pfnRandomLong(0,cnt-1)],
					gEngfuncs.pfnRandomFloat(0.92, 1.0),
					ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
			}
		}
	}
}
			


//======================
//   DEAGLE START
//======================

enum deagle_e 
{
	DEAGLE_IDLE1 = 0,
	DEAGLE_IDLE2,
	DEAGLE_IDLE3,
	DEAGLE_FIRE1,
	DEAGLE_FIRE_EMPTY,

	DEAGLE_RELOAD,	
	DEAGLE_RELOAD_EMPTY,

	DEAGLE_DEPLOY,
	DEAGLE_HOLSTER,
};


// EVENTS DEAGLE

void EV_FireDeagle( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	int empty;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	
	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	empty = args->bparam1;
	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_50cal.mdl");// brass shell

	if ( EV_IsLocal( idx ) )
	{
/*		 // Code Recoil Start
        gEngfuncs.GetViewAngles( (float *)viewangles );
        viewangles[PITCH] -= staerkedesrecoils;
        gEngfuncs.SetViewAngles( (float *)viewangles );
  */      // Code Recoil End

		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( empty ? DEAGLE_FIRE_EMPTY : DEAGLE_FIRE1, 2 ); // 2 );

		V_PunchAxis( 0, -4.0 ); //-6
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/deagle/deagle_fire-1.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );

	EV_HLDM_MuzzleFlash( vecSrc, 1.8 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
	
	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_DEAGLE, 0, 0, args->fparam1, args->fparam2, RIFLE_RECOIL );																		//DEAGLE
}

void EV_FireDeagle2( event_args_t *args )
{
	/*
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_50cal.mdl");// brass shell

	if ( EV_IsLocal( idx ) )
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( DEAGLE_FIRE1, 2 );

		V_PunchAxis( 0, -4.0 );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/deagle/deagle_fire-1.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );

	EV_GetGunPosition( args, vecSrc, origin );
	
	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_DEAGLE, 0, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	*/
}
//======================
//	  CROSSBOW START
//======================
enum sniper_e
{
	SNIPER_DEPLOY = 0,

	SNIPER_SLOW_IDLE1,

	SNIPER_FIRE1,
	SNIPER_FIRE_LAST,

	SNIPER_RELOAD1,

	SNIPER_SLOW_IDLE2,

	SNIPER_HOLSTER,
};

//======================
//	   SNIPER START
//======================
void EV_FireSniper( event_args_t *args )
{
    int idx;
    vec3_t origin;
    vec3_t angles; 
    vec3_t velocity;

    vec3_t ShellVelocity;
    vec3_t ShellOrigin; 
	//int shell;
    vec3_t vecSrc, vecAiming; 
    vec3_t up, right, forward;
    float flSpread = 0.01;
    
    idx = args->entindex; 
    VectorCopy( args->origin, origin );
    VectorCopy( args->angles, angles ); 
    VectorCopy( args->velocity, velocity );  
    AngleVectors( angles, forward, right, up );  
   // shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_9mm.mdl");

    if ( EV_IsLocal( idx ) )
    {
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( SNIPER_FIRE1 + gEngfuncs.pfnRandomLong(0,1), 1 ); //pfnRandomLong(0,2), 2 );

        V_PunchAxis( 0, -6.0 ); //-10
	}

//	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

//	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

		switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
		{
		case 0:
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/sniper/sniper_fire-1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/sniper/sniper_fire-2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
			break;
		}

	EV_GetGunPosition( args, vecSrc, origin );

// mazor begin
EV_HLDM_MuzzleFlash( vecSrc, 0.6 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
// mazor end

	VectorCopy( forward, vecAiming );

	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SNIPER, 0, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	}
	else
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SNIPER, 0, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	}
}
//======================
//	   SNIPER END 
//======================

//======================
//        FAMAS START
//======================

//	FAMAS ANIMACIONES

enum famas_e
{
	FAMAS_IDLE1 = 0,
	FAMAS_IDLE2,
	FAMAS_FIRE1,
	FAMAS_FIRE2,
	FAMAS_RELOAD,
	FAMAS_RELOAD_LAST,
	FAMAS_DEPLOY,
	FAMAS_HOLSTER,
	FAMAS_FIRE1_RND,
	FAMAS_FIRE2_RND,

};


//======================
//	    M249 START
//======================
enum M249_e
{
	M249_IDLE = 0,
	M249_IDLE2,
	M249_DRAW,
	M249_HOLSTER,
	M249_SHOOT,
	M249_SHOOT2,
	M249_SHOOT3,
	M249_RELOAD,
	M249_RELOAD2
};

void EV_FireM249( event_args_t *args )
{
    int idx;
    vec3_t origin;
    vec3_t angles; 
    vec3_t velocity;
/*
    // Recoil -Start
    vec3_t viewangles;
    float staerkedesrecoils = 0.70; // Cantidad de recoil
 */   // Recoil End
    
    vec3_t ShellVelocity;
    vec3_t ShellOrigin; int shell;
    vec3_t vecSrc, vecAiming; 
    vec3_t up, right, forward;
    float flSpread = 0.01;
    
    idx = args->entindex; 
    VectorCopy( args->origin, origin );
    VectorCopy( args->angles, angles ); 
    VectorCopy( args->velocity, velocity );  
    AngleVectors( angles, forward, right, up );  
    shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_M249.mdl");

    if ( EV_IsLocal( idx ) )
    {
/*        // Code Recoil Start
        gEngfuncs.GetViewAngles( (float *)viewangles );
        viewangles[PITCH] -= staerkedesrecoils;
        gEngfuncs.SetViewAngles( (float *)viewangles );
 */       // Code Recoil End
        

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( M249_SHOOT + gEngfuncs.pfnRandomLong(0,1), 1 ); //pfnRandomLong(0,2), 2 );

		V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -5, 5 ) );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m249/M249_fire-1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m249/M249_fire-2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}

	EV_GetGunPosition( args, vecSrc, origin );

// mazor begin
EV_HLDM_MuzzleFlash( vecSrc, 0.8 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
// mazor end

	VectorCopy( forward, vecAiming );

/*	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_M249, 2, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	}
	else
	{
*/		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_M249, 2, &tracerCount[idx-1], args->fparam1, args->fparam2, MG_RECOIL );
//	}
}
//******


//----------
// mp7 Event
//-----------
enum hl2mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

void EV_FireMP5( event_args_t *args )
{
    int idx;
    vec3_t origin;
    vec3_t angles; 
    vec3_t velocity;

   // Recoil -Start
    vec3_t viewangles;
    float staerkedesrecoils = 0.30; // Cantidad de recoil
    // Recoil End
    
    vec3_t ShellVelocity;
    vec3_t ShellOrigin; int shell;
    vec3_t vecSrc, vecAiming; 
    vec3_t up, right, forward;
    float flSpread = 0.01;
    
    idx = args->entindex; 
    VectorCopy( args->origin, origin );
    VectorCopy( args->angles, angles ); 
    VectorCopy( args->velocity, velocity );  
    AngleVectors( angles, forward, right, up );  
    shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_9mm.mdl");

    if ( EV_IsLocal( idx ) )
    {
        // Code Recoil Start
        gEngfuncs.GetViewAngles( (float *)viewangles );
        viewangles[PITCH] -= staerkedesrecoils;
        gEngfuncs.SetViewAngles( (float *)viewangles );
       // Code Recoil End
        

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( MP5_FIRE1 + gEngfuncs.pfnRandomLong(0,1), 1 ); //pfnRandomLong(0,1), 1 );

		//add old style???
//		V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -1, 2 ) );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp5/mp5_fire-1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/mp5/mp5_fire-2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}

	EV_GetGunPosition( args, vecSrc, origin );

// mazor begin
EV_HLDM_MuzzleFlash( vecSrc, 0.8 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
// mazor end

	VectorCopy( forward, vecAiming );

/*	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_P90, 0, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	}
	else
	{
*/		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_P90, 0, &tracerCount[idx-1], args->fparam1, args->fparam2, SMG_RECOIL );//SMG_RECOIL
//	}
}
//-----------
// P90 END
//------------

//======================
//    USP_TACTICAL START
//======================
//Lo que really matters son los numers
enum hl2glock_e {
	GLOCK_HL2_IDLE1 = 0,
	GLOCK_HL2_IDLE2,
	GLOCK_HL2_IDLE3,
	GLOCK_HL2_SHOOT,
	GLOCK_HL2_SHOOT_EMPTY,
	GLOCK_HL2_RELOAD,
	GLOCK_HL2_RELOAD_NOT_EMPTY,
	GLOCK_HL2_DRAW,
	GLOCK_HL2_HOLSTER,
	GLOCK_HL2_ADD_SILENCER
};

void EV_FirePistol( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;


// Recoil -Start
    vec3_t viewangles;
    float staerkedesrecoils = 0.20; // Cantidad de recoil 1.00
// Recoil End

	int empty;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	
	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	empty = args->bparam1;
	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_9mm.mdl");// brass shell

	if ( EV_IsLocal( idx ) )
	{
        // Code Recoil Start
        gEngfuncs.GetViewAngles( (float *)viewangles );
        viewangles[PITCH] -= staerkedesrecoils;
        gEngfuncs.SetViewAngles( (float *)viewangles );
        // Code Recoil End

		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( empty ? GLOCK_HL2_SHOOT_EMPTY : GLOCK_HL2_SHOOT, 2 );

		V_PunchAxis( 0, -2.0 );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
    {
    case 0:
        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glock/glock_fire-1.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
        break;
    case 1:
        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glock/glock_fire-2.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
        break;
    }

	
//	SoundEntPlay( idx, "../media/pistol.mp3", 150, 100 + gEngfuncs.pfnRandomLong(-5,5) );

	EV_GetGunPosition( args, vecSrc, origin );
	
	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, 0, args->fparam1, args->fparam2, PISTOL_RECOIL );
  }
//======================
//       USP_TACTICAL END
//======================

//======================
//	    svd START
//======================
enum svd_e
{
	SVD_IDLE1 = 0,

	SVD_RELOAD,//4.05

	SVD_DEPLOY,//1.36

	SVD_FIRE1,//1.10
	SVD_FIRE2
};

void EV_FireSvd( event_args_t *args )
{
    int idx;
    vec3_t origin;
    vec3_t angles; 
    vec3_t velocity;

//     Recoil -Start
    vec3_t viewangles;
    float staerkedesrecoils = 0.20; // Cantidad de recoil
   //  Recoil End
   
    vec3_t ShellVelocity;
    vec3_t ShellOrigin; 
	int shell;
    vec3_t vecSrc, vecAiming; 
    vec3_t up, right, forward;
    float flSpread = 0.01;
    
    idx = args->entindex; 
    VectorCopy( args->origin, origin );
    VectorCopy( args->angles, angles ); 
    VectorCopy( args->velocity, velocity );  
    AngleVectors( angles, forward, right, up );  
    shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_9mm.mdl");

    if ( EV_IsLocal( idx ) )
	{
		 // Code Recoil Start
        gEngfuncs.GetViewAngles( (float *)viewangles );
        viewangles[PITCH] -= staerkedesrecoils;
        gEngfuncs.SetViewAngles( (float *)viewangles );
       // Code Recoil End

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( SVD_FIRE1 + gEngfuncs.pfnRandomLong(0,1), 1 ); //pfnRandomLong(0,2), 2 );

		//V_PunchAxis( 0, -3.0 );
		V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -3, 2 ) );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -14, 8, true );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m82/m82_fire-1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m82/m82_fire-2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}

	EV_GetGunPosition( args, vecSrc, origin );

// mazor begin
EV_HLDM_MuzzleFlash( vecSrc, 0.2 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
// mazor end

	VectorCopy( forward, vecAiming );
	
	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP5, 0, &tracerCount[idx-1], args->fparam1, args->fparam2, 2 );

/*
	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SNIPER, 0, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	}
	else
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_SNIPER, 0, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	}
	*/
}

//======================
//		LX START 
//======================
#define SND_CHANGE_PITCH	(1<<7)		// duplicated in protocol.h change sound pitch

enum lx_e 
{
	LX_IDLE = 0,
	LX_RELOAD,
	LX_DRAW,
	LX_FIRE1,
	LX_FIRE2,
	LX_FIRE3,

	LX_HOLSTER
};
/*
void EV_SpinGauss( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int iSoundState = 0;

	int pitch;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	pitch = args->iparam1;

	iSoundState = args->bparam1 ? SND_CHANGE_PITCH : 0;

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, iSoundState, pitch );
}

/*
==============================
EV_StopPreviousGauss

==============================
*/
/*
void EV_StopPreviousGauss( int idx )
{
	// Make sure we don't have a gauss spin event in the queue for this guy
	gEngfuncs.pEventAPI->EV_KillEvents( idx, "events/gaussspin.sc" );
	gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_WEAPON, "ambience/pulsemachine.wav" );
}
*/
extern float g_flApplyVel;

void EV_FireLx( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	float flDamage = args->fparam1;
	int primaryfire = args->bparam1;

	int m_fPrimaryFire = args->bparam1;
	int m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	vec3_t vecSrc;
	vec3_t vecDest;
	edict_t		*pentIgnore;
	pmtrace_t tr, beam_tr;
	float flMaxFrac = 1.0;
	int	nTotal = 0;
	int fHasPunched = 0;
	int fFirstBeam = 1;
	int	nMaxHits = 10;
	physent_t *pEntity;
	int m_iBeam, m_iGlow, m_iBalls, m_iFireEff;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	if ( args->bparam2 )
	{
		EV_StopPreviousGauss( idx );
		return;
	}

//	Con_Printf( "Firing gauss with %f\n", flDamage );
	EV_GetGunPosition( args, vecSrc, origin );

	m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/lx_beam.spr" );
//	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/lx_balls.spr" );
	m_iBalls = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/lx_balls.spr" );
	m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/lx_glow.spr" );
	m_iFireEff = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/lx_effect.spr" );

	AngleVectors( angles, forward, right, up );

	VectorMA( vecSrc, 8192, forward, vecDest );

	if ( EV_IsLocal( idx ) )
	{
		//V_PunchAxis( 0, -6.0 ); //-2
		V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -4, -1 ) );
		gEngfuncs.pEventAPI->EV_WeaponAnimation( LX_FIRE2, 2 );

		if ( m_fPrimaryFire == false )
			 g_flApplyVel = flDamage;	
			 
	}

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/lx38/lx38_fire-2.wav", 0.5 + flDamage * (1.0 / 400.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );

	while (flDamage > 10 && nMaxHits > 0)
	{
		nMaxHits--;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );
		
		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();
	
		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecDest, PM_STUDIO_BOX, -1, &tr );

		gEngfuncs.pEventAPI->EV_PopPMStates();

		if ( tr.allsolid )
			break;

		if (fFirstBeam)
		{
			if ( EV_IsLocal( idx ) )
			{
				// Add muzzle flash to current weapon model
			//	EV_MuzzleFlash();
			}
			fFirstBeam = 0;

			gEngfuncs.pEfxAPI->R_BeamEntPoint( 
				idx | 0x1000,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				m_fPrimaryFire ? 128.0 : flDamage,
				0,
				0,
				0,
				m_fPrimaryFire ? 96 : 255, //m_fPrimaryFire ? 255 : 255, //color
				m_fPrimaryFire ? 128 : 255,
				m_fPrimaryFire ? 96 : 255
			);
		}
		else
		{
			gEngfuncs.pEfxAPI->R_BeamPoints( vecSrc,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				m_fPrimaryFire ? 128.0 : flDamage,
				0,
				0,
				0,
				m_fPrimaryFire ? 96 : 255, //m_fPrimaryFire ? 255 : 255,
				m_fPrimaryFire ? 128 : 255,
				m_fPrimaryFire ? 96 : 255
			);
		}

		pEntity = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent );
		if ( pEntity == NULL )
			break;

		if ( pEntity->solid == SOLID_BSP )
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct( tr.plane.normal, forward );

			if (n < 0.5) // 60 degrees	
			{
				// ALERT( at_console, "reflect %f\n", n );
				// reflect
				vec3_t r;
			
				VectorMA( forward, 2.0 * n, tr.plane.normal, r );

				flMaxFrac = flMaxFrac - tr.fraction;
				
				VectorCopy( r, forward );

				VectorMA( tr.endpos, 8.0, forward, vecSrc );
				VectorMA( vecSrc, 8192.0, forward, vecDest );

				gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT );

				vec3_t fwd;
				VectorAdd( tr.endpos, tr.plane.normal, fwd );

				gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
									255, 100 );

				// lose energy
				if ( n == 0 )
				{
					n = 0.1;
				}
				
				flDamage = flDamage * (1 - n);

			}
			else
			{
				// tunnel
				EV_HLDM_DecalGunshot( &tr, BULLET_MONSTER_12MM, vecSrc, vecDest );

				gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 1.0, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT );

				// limit it to one hole punch
				if (fHasPunched)
				{
					break;
				}
				fHasPunched = 1;
				
				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if ( !m_fPrimaryFire )
				{
					vec3_t start;

					VectorMA( tr.endpos, 8.0, forward, start );

					// Store off the old count
					gEngfuncs.pEventAPI->EV_PushPMStates();
						
					// Now add in all of the players.
					gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );

					gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
					gEngfuncs.pEventAPI->EV_PlayerTrace( start, vecDest, PM_STUDIO_BOX, -1, &beam_tr );

					if ( !beam_tr.allsolid )
					{
						vec3_t delta;
						float n;

						// trace backwards to find exit point

						gEngfuncs.pEventAPI->EV_PlayerTrace( beam_tr.endpos, tr.endpos, PM_STUDIO_BOX, -1, &beam_tr );

						VectorSubtract( beam_tr.endpos, tr.endpos, delta );
						
						n = Length( delta );

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;
							flDamage -= n;

							// absorption balls
							{
								vec3_t fwd;
								VectorSubtract( tr.endpos, forward, fwd );
								gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
									255, 100 );
							}

	//////////////////////////////////// WHAT TO DO HERE
							// CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

							EV_HLDM_DecalGunshot( &beam_tr, BULLET_MONSTER_12MM, vecSrc, vecDest );

							gEngfuncs.pEfxAPI->R_TempSprite( beam_tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_SPRANIMATE); //fix
			
							gEngfuncs.pEfxAPI->R_TempSprite( beam_tr.endpos, vec3_origin, 0.1, m_iFireEff, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT );

							// balls
							{
								vec3_t fwd;
								VectorSubtract( beam_tr.endpos, forward, fwd );
								gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, beam_tr.endpos, fwd, m_iBalls, (int)(flDamage * 0.3), 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 200,
									255, 40 );
							}
							
							VectorAdd( beam_tr.endpos, forward, vecSrc );
						}
					}
					else
					{
						flDamage = 0;
					}

					gEngfuncs.pEventAPI->EV_PopPMStates();
				}
				else
				{
					if ( m_fPrimaryFire )
					{
						// slug doesn't punch through ever with primary 
						// fire, so leave a little glowy bit and make some balls
						gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_SPRANIMATE); //1 FTENT_FADEOUT ); fix
						
						gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iFireEff, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT );
		
						{
							vec3_t fwd;
							VectorAdd( tr.endpos, tr.plane.normal, fwd );
							gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
								255, 200 );
						}
					}

					flDamage = 0;
				}
			}
		}
		else
		{
			VectorAdd( tr.endpos, forward, vecSrc );
		}
	}
}
//======================
//	   LX END 
//======================

//======================
//        M16 START
//======================

//	M16 ANIMACIONES

enum M4A2_e
{
	M4A2_IDLE1 = 0,
	M4A2_IDLE2,
	M4A2_FIRE1,
	M4A2_FIRE2,
	M4A2_FIREBURST1,
	M4A2_FIREBURST2,
	M4A2_RELOAD1,
	M4A2_RELOAD2,
	M4A2_RELOAD_VACIO,
	M4A2_RELOAD_NMC1,
	M4A2_RELOAD_NMC2,
	M4A2_RELOAD_NMC_VACIO,
	M4A2_DEPLOY_FIRST,
	M4A2_DEPLOY,
	M4A2_HOLSTER,

	M4A2_GL_ON,

	M4A2_GL_FIRE1,
	M4A2_GL_RELOAD1,
	M4A2_GL_RELOAD_NMC1,
	M4A2_GL_IDLE,

	M4A2_GL_OFF,

	M4A2_GL_HOLSTER,
		
	M4A2_GL_NEWANIMSBYME,
	M4A2_GL_NEWFIRE
};

// EVENT M16
void EV_FireM4A2( event_args_t *args )
{
    int idx;
    vec3_t origin;
    vec3_t angles; 
    vec3_t velocity;

 /*   // Recoil -Start
    vec3_t viewangles;
    float staerkedesrecoils = 0.30; // Cantidad de recoil 0.50
  */  // Recoil End
    
    vec3_t ShellVelocity;
    vec3_t ShellOrigin; int shell;
    vec3_t vecSrc, vecAiming; 
    vec3_t up, right, forward;
    float flSpread = 0.01;
    
    idx = args->entindex; 
    VectorCopy( args->origin, origin );
    VectorCopy( args->angles, angles ); 
    VectorCopy( args->velocity, velocity );  
    AngleVectors( angles, forward, right, up );  
    shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_556.mdl");

    if ( EV_IsLocal( idx ) )
    {
        // Code Recoil Start
  /*      gEngfuncs.GetViewAngles( (float *)viewangles );
        viewangles[PITCH] -= staerkedesrecoils;
        gEngfuncs.SetViewAngles( (float *)viewangles );
  */      // Code Recoil End
        

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( M4A2_FIRE1 + gEngfuncs.pfnRandomLong(0,1), 1 ); //pfnRandomLong(0,2), 2 );

		V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -3, -2 ) );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -14, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 


		if ( CVAR_GET_FLOAT( "slowmo" ) != 0 )
		{
			switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m16/m16_fire-1.wav", 1, ATTN_NORM, 0, 50 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m16/m16_fire-2.wav", 1, ATTN_NORM, 0, 50 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
				break;
			}
				}
		else
		{
			switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m16/m16_fire-1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/m16/m16_fire-2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
				break;
			}
		}

	EV_GetGunPosition( args, vecSrc, origin );

// mazor begin
EV_HLDM_MuzzleFlash( vecSrc, 1.0 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
// mazor end

	VectorCopy( forward, vecAiming );

/*	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_M16, 0, &tracerCount[idx-1], args->fparam1, args->fparam2 );
	}
	else
	{
*/		EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_M16, 0, &tracerCount[idx-1], args->fparam1, args->fparam2, RIFLE_RECOIL );
//	}
}

// We only predict the animation and sound
// The grenade is still launched from the server.
void EV_FireM4A2Gl( event_args_t *args )
{
	int idx;
	vec3_t origin;
	
	idx = args->entindex;
	VectorCopy( args->origin, origin );

	if ( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( M4A2_GL_NEWFIRE, 1 );
		V_PunchAxis( 0, -10 );
	}
	
	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glauncher.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glauncher2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}
}


/******************
	OICW
******************/
enum OICW_e
{
	OICW_IDLE1 = 0,

	OICW_RELOAD,
	OICW_DEPLOY,

	OICW_FIRE1,
	OICW_FIRE2,
	OICW_FIRE3
};

void EV_FireOICW( event_args_t *args )
{
    int idx;
    vec3_t origin;
    vec3_t angles; 
    vec3_t velocity;

 /*   // Recoil -Start
    vec3_t viewangles;
    float staerkedesrecoils = 0.30; // Cantidad de recoil 0.50
  */  // Recoil End
    
    vec3_t ShellVelocity;
    vec3_t ShellOrigin; int shell;
    vec3_t vecSrc, vecAiming; 
    vec3_t up, right, forward;
    float flSpread = 0.01;
    
    idx = args->entindex; 
    VectorCopy( args->origin, origin );
    VectorCopy( args->angles, angles ); 
    VectorCopy( args->velocity, velocity );  
    AngleVectors( angles, forward, right, up );  
    shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/weapons/shell_556.mdl");

    if ( EV_IsLocal( idx ) )
    {
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();																//1 1
		gEngfuncs.pEventAPI->EV_WeaponAnimation( OICW_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2 ); //pfnRandomLong(0,2), 2 );

		V_PunchAxis( 0, gEngfuncs.pfnRandomFloat( -3, -2 ) );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -14, 4 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHELL ); 

	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/oicw/oicw_fire-1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/oicw/oicw_fire-2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}

	EV_GetGunPosition( args, vecSrc, origin );

	// mazor begin
	EV_HLDM_MuzzleFlash( vecSrc, 1.5 + gEngfuncs.pfnRandomFloat( -0.2, 0.2 ) ); //1.0
	// mazor end

	VectorCopy( forward, vecAiming );

	EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_M16, 0, &tracerCount[idx-1], args->fparam1, args->fparam2, RIFLE_RECOIL );
}

// We only predict the animation and sound
// The grenade is still launched from the server.
void EV_FireOICWgl( event_args_t *args )
{
	int idx;
	vec3_t origin;
	
	idx = args->entindex;
	VectorCopy( args->origin, origin );

	if ( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( OICW_FIRE1, 1 );
		V_PunchAxis( 0, -10 );
	}
	
	switch( gEngfuncs.pfnRandomLong( 0, 1 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glauncher.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/glauncher2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong( 0, 0xf ) );
		break;
	}
}

void EV_Smoke( event_args_t *args )
{
	//TO DO: I need to delete this. I'm using some fuck normal sprites in server side.
	int  iWallsmoke = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/humo.spr");
   
	vec3_t origin;
    vec3_t up, right, forward;

		// ************SPRITE 1
				TEMPENTITY *pTemph = gEngfuncs.pEfxAPI->R_TempSprite( origin, forward * gEngfuncs.pfnRandomFloat(10, 30) + right * gEngfuncs.pfnRandomFloat(-6, 6) + up * gEngfuncs.pfnRandomFloat(0, 6),
					1.4,//0.4
					iWallsmoke,
					kRenderTransAlpha,//add
					kRenderFxNone,

					//kRenderTransAdd,
					//kRenderFxNoDissipation,
					
					//kRenderTransAlpha,
					//kRenderFxNone,

					1.0,//1.0
					1, //0.3
					FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEWORLD
				
				);
				if(pTemph)
				{    // sprite created successfully, adjust some things
					pTemph->fadeSpeed = 2.0;//2
					pTemph->entity.curstate.framerate = 20.0;//20
					pTemph->entity.curstate.renderamt = 222;
					pTemph->entity.curstate.rendercolor.r = 50;
					pTemph->entity.curstate.rendercolor.g = 50;
					pTemph->entity.curstate.rendercolor.b = 50;
				}
	// ************SPRITE 1
}

void EV_SpriteTracer( event_args_t *args )
{
	//TO DO: oh, this. Fooz Code
	//I know how works now, but need I a laser? (the USAS weapon doesn't need a laser sight, c'mon, its a shotgun)
	int  iLaserSmall = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/laserDOT.spr");

	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	float flDamage = args->fparam1;
	int primaryfire = args->bparam1;

	int m_fPrimaryFire = args->bparam1;
	int m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	vec3_t vecSrc;
	vec3_t vecDest;
	pmtrace_t tr, beam_tr;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	if ( args->bparam2 )
	{
		EV_StopPreviousGauss( idx );
		return;
	}

	EV_GetGunPosition( args, vecSrc, origin );

	int m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/lx_beam.spr" );

	AngleVectors( angles, forward, right, up );

	VectorMA( vecSrc, 8192, forward, vecDest );

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );
		
		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();
	
		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecDest, PM_STUDIO_BOX, -1, &tr );

		gEngfuncs.pEventAPI->EV_PopPMStates();

			if ( EV_IsLocal( idx ) )
			{
				// Add muzzle flash to current weapon model
			//	EV_MuzzleFlash();
			}
//	BEAM		*( *R_BeamEntPoint )			
//( int startEnt, float * end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );

			gEngfuncs.pEfxAPI->R_BeamEntPoint( 
				idx | 0x4000,
				tr.endpos,
				m_iBeam,
				0.001,
				1,//2.5
				0.0,
				50.0,//255
				0,
				0,
				0,
				255, //m_fPrimaryFire ? 255 : 255, //color
				0,
				0
			);
//*( *R_BeamRing )				
//( int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
			
		
//	SpriteTracer( vecDest, tr.endpos, iLaserSmall, 0.7, 0.0, 0.0, 0.3 );

//	SpriteTracer( tr.endpos, oldpos, iLaserSmall, 0.7, 0.0, 0.0, 0.3 );

}


//======================
//	   CROWBAR START
//======================
enum hl2crowbar_e {
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_ATTACK3HIT
};

int g_iSwinghl2;

//Only predict the miss sounds, hit sounds are still played 
//server side, so players don't get the wrong idea.
void EV_FireHL2Crowbar( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	
	//Play Swing sound
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM); 

	if ( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( CROWBAR_ATTACK1MISS, 1 );

		switch( (g_iSwinghl2++) % 3 )
		{
			case 0:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( CROWBAR_ATTACK1MISS, 1 ); break;
			case 1:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( CROWBAR_ATTACK2MISS, 1 ); break;
			case 2:
				gEngfuncs.pEventAPI->EV_WeaponAnimation ( CROWBAR_ATTACK3MISS, 1 ); break;
		}
	}
}
//======================
//	   CROWBAR END 
//======================

enum hl2shotgun_e {
	SHOTGUN_HL2_IDLE = 0,
	SHOTGUN_HL2_FIRE,
	SHOTGUN_HL2_FIRE2,
	SHOTGUN_HL2_RELOAD,
	SHOTGUN_HL2_PUMP,
	SHOTGUN_HL2_START_RELOAD,
	SHOTGUN_HL2_DRAW,
	SHOTGUN_HL2_HOLSTER,
	SHOTGUN_HL2_IDLE4,
	SHOTGUN_HL2_IDLE_DEEP
};

//======================
//	  HL2 SHOTGUN START
//======================
void EV_FireHL2ShotGunDouble( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	int j;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/shotgunshell.mdl");// brass shell

	if ( EV_IsLocal( idx ) )
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( SHOTGUN_HL2_FIRE2, 2 );
		V_PunchAxis( 0, -10.0 );
	}

	for ( j = 0; j < 2; j++ )
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6 );

		EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHOTSHELL ); 
	}

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/shotgun/dbarrel1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 8, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.17365, 0.04362 );
	}
	else
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 12, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.08716, 0.08716 );
	}
}

void EV_FireHL2ShotGunSingle( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex ("models/shotgunshell.mdl");// brass shell

	if ( EV_IsLocal( idx ) )
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation( SHOTGUN_HL2_FIRE, 2 );

		V_PunchAxis( 0, -5.0 );
	}

	EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6 );

	EV_EjectBrass ( ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHOTSHELL ); 

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/shotgun/sbarrel1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 4, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.08716, 0.04362 );
	}
	else
	{
		EV_HLDM_FireBullets( idx, forward, right, up, 6, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx-1], 0.08716, 0.08716 );
	}
}
//======================
//	   SHOTGUN END
//======================
