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
//
// hud.cpp
//
// implementation of CHud class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"

#include "demo.h"
#include "demo_api.h"
#include "vgui_scorepanel.h"
#include "mp3.h"

#include "rain.h"
#include "pys_rope.h"
#include "particle_header.h"

// FGW
#include "bumpmap.h"

class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex < sizeof(g_PlayerExtraInfo)/sizeof(g_PlayerExtraInfo[0]) )
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

			if ( iTeam < 0 )
			{
				iTeam = 0;
			}

			iTeam = iTeam % iNumberOfTeamColors;

			color[0] = iTeamColors[iTeam][0];
			color[1] = iTeamColors[iTeam][1];
			color[2] = iTeamColors[iTeam][2];
		}
	}

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort && gViewPort->m_pScoreBoard )
			return !gViewPort->m_pScoreBoard->isVisible();
		else
			return false;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;


extern client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

extern cvar_t *sensitivity;
cvar_t *cl_lw = NULL;

// Mazor - used for view rolling when strafing
cvar_t *cl_rollangle;
cvar_t *cl_rollspeed;
cvar_t *cl_autoswitch;
cvar_t *cl_shelllife;
cvar_t *cl_wallpuff;
cvar_t *cl_pulso;
cvar_t *cl_expdetail;
cvar_t *cl_showplayer;
cvar_t *cl_gunsmoke;
cvar_t *cl_nvghires;
cvar_t *cl_oldmodels;

cvar_t *cl_luzbar_x;
cvar_t *cl_luzbar_y;
cvar_t *cl_hud_x;
cvar_t *cl_hud_y;
cvar_t *cl_detailsparks;

///test
/*
cvar_t *	cl_list_x;
cvar_t *	cl_list_y;
cvar_t *	cl_bucket_x;
cvar_t *	cl_bucket_y;
*/
///test
cvar_t *value_1;
cvar_t *value_2;
cvar_t *value_3;
cvar_t *showtime;

cvar_t *dd_1;
cvar_t *dd_2;

//TO DO: well im using the following cvars to my own needs. Works, but it's hard coded.
cvar_t *cl_showparachute;
cvar_t *cl_crosshair_ontarget;
cvar_t *cl_autohelp;
cvar_t *cl_showheadcrab;
cvar_t *neartalk;
cvar_t *neardoor;
cvar_t *nearweapon;
cvar_t *nearbutton;

cvar_t *forward;
/*
cvar_t *cl_showplayer_stand;
cvar_t *cl_showplayer_walk;
cvar_t *cl_showplayer_run;
*/

cvar_t *cl_fog;

cvar_t *cl_detailfire;
cvar_t *cl_detailmuzz;
//cvar_t *cl_overview;
cvar_t *game_tr_completed;
cvar_t *game_all_completed;

cvar_t *cl_playmusic;

void ShutdownInput (void);

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_Logo(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Logo(pszName, iSize, pbuf );
}

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ResetHUD(pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_ViewMode( pszName, iSize, pbuf );
	return 1;
}
// G-Cont. rain message
int __MsgFunc_RainData(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_RainData( pszName, iSize, pbuf );
}
//change body for weapon models
int __MsgFunc_SetBody(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_SetBody( pszName, iSize, pbuf );
	return 1;
}

//change skin for weapon models
int __MsgFunc_SetSkin(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_SetSkin( pszName, iSize, pbuf );
	return 1;
}
//LRC
int __MsgFunc_SetFog(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_SetFog( pszName, iSize, pbuf );
	return 1;
}
int __MsgFunc_Clcommand( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Clcommand( pszName, iSize, pbuf );
}

//LRC
int __MsgFunc_AddShine(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_AddShine( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SetFOV( pszName, iSize, pbuf );
}

int __MsgFunc_Concuss(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Concuss( pszName, iSize, pbuf );
}

int __MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GameMode( pszName, iSize, pbuf );
}
int __MsgFunc_PlayMP3(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_PlayMP3( pszName, iSize, pbuf );
}

// fo0z
/*
int __MsgFunc_FPlaySound(const char *pszName, int iSize, void *pbuf )
{
	gHUD.MsgFunc_FPlaySound( pszName, iSize, pbuf );
	return 1;
}
*/
// TFFree Command Menu
void __CmdFunc_OpenCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
}

// TFC "special" command
void __CmdFunc_InputPlayerSpecial(void)
{
	if ( gViewPort )
	{
		gViewPort->InputPlayerSpecial();
	}
}

void __CmdFunc_CloseCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
}

void __CmdFunc_ToggleServerBrowser( void )
{
	if ( gViewPort )
	{
		gViewPort->ToggleServerBrowser();
	}
}
void __CmdFunc_StopMP3( void )
{
	gMP3.StopMP3();
}
// TFFree Command Menu Message Handlers
int __MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ValClass( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Feign(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Feign( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Detpack( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_BuildSt(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_BuildSt( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_RandomPC(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
	return 0;
}
 
int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Spectator(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec( pszName, iSize, pbuf );
	return 0;
}
 
//sky sys
int __MsgFunc_SetSky(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_SetSky( pszName, iSize, pbuf );
	return 1;
}

//rope sys
int __MsgFunc_AddRope( const char *pszName, int iSize, void *pbuf )
{
	gHUD.MsgFunc_AddRope( pszName, iSize, pbuf );
	return 1;
}

//BP - ParticleEmitter
int __MsgFunc_Particles(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Particles( pszName, iSize, pbuf );
	return 0;
}

//BP - Grass
int __MsgFunc_Grass(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Grass( pszName, iSize, pbuf );
	return 0;
}

// FGW
int __MsgFunc_BumpLight(const char *pszName, int iSize, void *pbuf)
{
	float rad, strength;
	Vector pos, col;
	int moveWithEnt;
	bool enabled;
	char* targetname;
	bool moveWithExtraInfo = false;
	Vector moveWithPos, moveWithAngles;
	int style;

	BEGIN_READ(pbuf, iSize);

	int msgtype = READ_BYTE();

	if (msgtype == 0)
	{
		// create a new light

		targetname = READ_STRING();

		pos.x = READ_COORD();
		pos.y = READ_COORD();
		pos.z = READ_COORD();

		rad = READ_COORD();
		strength = READ_COORD();
		col.x = READ_BYTE() / 255.0f;
		col.y = READ_BYTE() / 255.0f;
		col.z = READ_BYTE() / 255.0f;

		style = READ_BYTE();

		enabled = (READ_BYTE() ? true : false);

		moveWithEnt = READ_SHORT();

		if (moveWithEnt != -1 && READ_BYTE())
		{
			moveWithPos.x = READ_COORD();
			moveWithPos.y = READ_COORD();
			moveWithPos.z = READ_COORD();

			moveWithAngles.x = READ_ANGLE();
			moveWithAngles.y = READ_ANGLE();
			moveWithAngles.z = READ_ANGLE();

			moveWithExtraInfo = true;
		}

		g_BumpmapMgr.AddLight(targetname, pos, col, strength, rad, enabled, style, moveWithEnt, moveWithExtraInfo,
			moveWithPos, moveWithAngles);
	}
	else if (msgtype == 1)
	{
		// set the enabled/disabled state of an existing one

		targetname = READ_STRING();
		enabled = (READ_BYTE() ? true : false);

		g_BumpmapMgr.EnableLight(targetname, enabled);
	}
	else
	{
		gEngfuncs.Con_Printf("BUMPMAPPING: Bogus bump light message type: %i\n", msgtype); // Totally bogus, dude.
	}

	return 1;
}


// This is called every time the DLL is loaded
void CHud :: Init( void )
{
	//BP ParticleEmitter
	HOOK_MESSAGE( Particles );
	HOOK_MESSAGE( Grass );

	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( Concuss );
	HOOK_MESSAGE( SetFog ); //LRC
	HOOK_MESSAGE( AddShine ); //LRC
	HOOK_MESSAGE( Clcommand );
	HOOK_MESSAGE( RainData );//G-Cont. for rain control 
	HOOK_MESSAGE( SetSky ); //LRC
	HOOK_MESSAGE( AddRope ); //LRC
	HOOK_MESSAGE( SetBody );//change body for view weapon model
	HOOK_MESSAGE( SetSkin );//change skin for view weapon model

	HOOK_MESSAGE( PlayMP3 );

	HOOK_COMMAND( "stopmusic", StopMP3 );
	gMP3.Initialize();

	// TFFree CommandMenu
	HOOK_COMMAND( "+commandmenu", OpenCommandMenu );
	HOOK_COMMAND( "-commandmenu", CloseCommandMenu );
	HOOK_COMMAND( "ForceCloseCommandMenu", ForceCloseCommandMenu );
	HOOK_COMMAND( "special", InputPlayerSpecial );
	HOOK_COMMAND( "togglebrowser", ToggleServerBrowser );

	HOOK_MESSAGE( ValClass );
	HOOK_MESSAGE( TeamNames );
	HOOK_MESSAGE( Feign );
	HOOK_MESSAGE( Detpack );
	HOOK_MESSAGE( MOTD );
	HOOK_MESSAGE( BuildSt );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );

	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );

	// VGUI Menus
	HOOK_MESSAGE( VGUIMenu );

	// FGW
	HOOK_MESSAGE( BumpLight );

	CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );		// controls whether or not to suicide immediately on TF class switch
	CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round

     CVAR_CREATE( "hud_red","255", FCVAR_ARCHIVE );
     CVAR_CREATE( "hud_green", "160", FCVAR_ARCHIVE );
     CVAR_CREATE( "hud_blue", "0", FCVAR_ARCHIVE );

	 //newc
	CVAR_CREATE( "hud_newcross", "1", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "hud_newcross_size", "2", FCVAR_ARCHIVE );
	
	CVAR_CREATE( "zz_framerate", "10", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "zz_fadespeed", "10", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "zz_life", "0.1", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "zz_alpha", "1", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round

	CVAR_CREATE("r_shadows", "1", FCVAR_ARCHIVE );//on default
	CVAR_CREATE("r_paintball", "0", FCVAR_ARCHIVE );
	CVAR_CREATE( "gl_texturedetail", "2", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "cl_drawprops", "1", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round

	CVAR_CREATE( "cl_removeclienteffects", "1", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round

	g_ParticleCount = gEngfuncs.pfnRegisterVariable("cl_particlecount", "100", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	g_ParticleDebug = gEngfuncs.pfnRegisterVariable("cl_particledebug", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	g_ParticleSorts = gEngfuncs.pfnRegisterVariable("cl_particlesorts", "3", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_grassamount", "100", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

	ProcessWeather = gEngfuncs.pfnRegisterVariable( "cl_weather", "1", 0 );

	 //sp
	m_iLogo = 0;
	m_iFOV = 0;

	CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", 0 );
	default_fov = CVAR_CREATE( "default_fov", "90", 0 );
	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );
	RainInfo = gEngfuncs.pfnRegisterVariable( "cl_raininfo", "0", 0 );

	m_pSpriteList = NULL;
	m_pShinySurface = NULL; //LRC

	gEngfuncs.Con_Printf( "Resetting iPlayerBodyTime\n");

	// Clear any old HUD list
	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

	m_Ammo.Init();
	m_Health.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_Geiger.Init();
	m_Train.Init();
	m_Battery.Init();
	m_Flash.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_AmmoSecondary.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();
	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);

	m_Menu.Init();
	// advanced NVG
	m_NVG.Init();
	m_Cine.Init();
	m_Ricardo.Init();
	// advanced NVG
	m_HudRadar.Init();
	m_LuzBar.Init();
	
	m_FlashBar.Init();

	InitRain();	
	m_Money.Init();

	ServersInit();
	//ryokeen
//	gRopeRender.StartRenderer();
	m_Tbutton.Init();

	MsgFunc_ResetHUD(0, 0, NULL );

	CVAR_CREATE("xp_points", "0", FCVAR_ARCHIVE );//WHY HERE ???

//	CVAR_CREATE("cl_tips", "0", FCVAR_ARCHIVE );//WHY HERE ???
//	CVAR_CREATE("tip", "0", FCVAR_ARCHIVE );//WHY HERE ???
	CVAR_CREATE("cl_ragdoll", "0", FCVAR_ARCHIVE );//WHY HERE ???

	CVAR_CREATE("commentary", "0", FCVAR_ARCHIVE );//WHY HERE ???
	CVAR_CREATE("dev_override_prethink", "0", FCVAR_ARCHIVE );//WHY HERE ???

	CVAR_CREATE("score_killed", "0", FCVAR_ARCHIVE );//WHY HERE ???
	CVAR_CREATE("score_head", "0", FCVAR_ARCHIVE );//WHY HERE ???
	CVAR_CREATE("score_knifed", "0", FCVAR_ARCHIVE );//WHY HERE ???

	CVAR_CREATE( "slowmo", "0", FCVAR_ARCHIVE );//send messages bwetween cldll and dll

	CVAR_CREATE( "r_particles", "1", FCVAR_ARCHIVE );//particles

	CVAR_CREATE("r_glow", "0", FCVAR_ARCHIVE );
	//CVAR_CREATE("r_glowmode", "0", FCVAR_ARCHIVE ); //AJH this is now redundant
	CVAR_CREATE("r_glowstrength", "1", FCVAR_ARCHIVE );
	CVAR_CREATE("r_glowblur", "4", FCVAR_ARCHIVE );
	CVAR_CREATE("r_glowdark", "2", FCVAR_ARCHIVE );

	CVAR_CREATE("r_bluralpha", "1", FCVAR_ARCHIVE );
	CVAR_CREATE("r_blur", "0", FCVAR_ARCHIVE );

	viewEntityIndex = 0; // trigger_viewset stuff
	viewFlags = 0;

	// Mazor - used for view rolling when strafing
	cl_rollangle		= gEngfuncs.pfnRegisterVariable ( "cl_rollangle", "0.65", FCVAR_CLIENTDLL|FCVAR_ARCHIVE );
	cl_rollspeed		= gEngfuncs.pfnRegisterVariable ( "cl_rollspeed", "300", FCVAR_CLIENTDLL|FCVAR_ARCHIVE );
//new cvars
	cl_shelllife		= gEngfuncs.pfnRegisterVariable ( "cl_shelllife", "2.5", FCVAR_ARCHIVE );
	cl_pulso			= gEngfuncs.pfnRegisterVariable ( "cl_pulso", "1", FCVAR_ARCHIVE );
	cl_wallpuff			= gEngfuncs.pfnRegisterVariable ( "cl_wallpuff", "1", FCVAR_ARCHIVE );
	cl_expdetail		= gEngfuncs.pfnRegisterVariable ( "cl_expdetail", "1", FCVAR_ARCHIVE );
	cl_showplayer		= gEngfuncs.pfnRegisterVariable ( "cl_showplayer", "0", FCVAR_ARCHIVE );
	
	cl_detailsparks		= gEngfuncs.pfnRegisterVariable ( "cl_detailsparks", "0", FCVAR_ARCHIVE );
	cl_playmusic		= gEngfuncs.pfnRegisterVariable ( "cl_playmusic", "0", FCVAR_USERINFO|FCVAR_ARCHIVE );


	cl_detailfire		= gEngfuncs.pfnRegisterVariable ( "cl_detailfire", "0", FCVAR_ARCHIVE );
	cl_detailmuzz		= gEngfuncs.pfnRegisterVariable ( "cl_detailmuzz", "0", FCVAR_ARCHIVE );

	cl_autoswitch		= gEngfuncs.pfnRegisterVariable ( "cl_autoswitch", "1", FCVAR_USERINFO|FCVAR_ARCHIVE );

	cl_showparachute	= gEngfuncs.pfnRegisterVariable ( "cl_showparachute", "0", FCVAR_ARCHIVE );

	cl_fog	= gEngfuncs.pfnRegisterVariable ( "cl_fog", "0", FCVAR_ARCHIVE );
//	cl_overview = gEngfuncs.pfnRegisterVariable ( "cl_overview", "0", FCVAR_ARCHIVE );

	cl_showheadcrab		= gEngfuncs.pfnRegisterVariable ( "cl_showheadcrab", "0", FCVAR_ARCHIVE );
	
	forward		= gEngfuncs.pfnRegisterVariable ( "forward", "12", FCVAR_ARCHIVE );

	cl_gunsmoke			= gEngfuncs.pfnRegisterVariable ( "cl_gunsmoke", "1", FCVAR_ARCHIVE );
	cl_nvghires			= gEngfuncs.pfnRegisterVariable ( "cl_nvghires", "1", FCVAR_ARCHIVE );
	cl_oldmodels		= gEngfuncs.pfnRegisterVariable ( "cl_oldmodels", "0", FCVAR_ARCHIVE );
		
	cl_luzbar_x		= gEngfuncs.pfnRegisterVariable ( "cl_luzbar_x", "268", FCVAR_ARCHIVE );
	cl_luzbar_y		= gEngfuncs.pfnRegisterVariable ( "cl_luzbar_y", "455", FCVAR_ARCHIVE );

	cl_hud_x		= gEngfuncs.pfnRegisterVariable ( "cl_hud_x", "0", FCVAR_ARCHIVE );
	cl_hud_y		= gEngfuncs.pfnRegisterVariable ( "cl_hud_y", "2", FCVAR_ARCHIVE );
/*
	cl_list_x		= gEngfuncs.pfnRegisterVariable ( "cl_list_x", "0", FCVAR_ARCHIVE );
	cl_list_y		= gEngfuncs.pfnRegisterVariable ( "cl_list_y", "0", FCVAR_ARCHIVE );
	
	cl_bucket_x		= gEngfuncs.pfnRegisterVariable ( "cl_bucket_x", "0", FCVAR_ARCHIVE );
	cl_bucket_y		= gEngfuncs.pfnRegisterVariable ( "cl_bucket_y", "0", FCVAR_ARCHIVE );
*/
	showtime		= gEngfuncs.pfnRegisterVariable ( "showtime", "0", FCVAR_ARCHIVE );

	value_1		= gEngfuncs.pfnRegisterVariable ( "value_1", "0", FCVAR_ARCHIVE );
	value_2		= gEngfuncs.pfnRegisterVariable ( "value_2", "0", FCVAR_ARCHIVE );
	value_3		= gEngfuncs.pfnRegisterVariable ( "value_3", "0", FCVAR_ARCHIVE );

	dd_1		= gEngfuncs.pfnRegisterVariable ( "dd_1", "5", FCVAR_ARCHIVE );
	dd_2		= gEngfuncs.pfnRegisterVariable ( "dd_2", "5", FCVAR_ARCHIVE );

	cl_crosshair_ontarget	= gEngfuncs.pfnRegisterVariable ( "cl_crosshair_ontarget", "0", FCVAR_ARCHIVE );
	cl_autohelp	= gEngfuncs.pfnRegisterVariable ( "cl_autohelp", "1", FCVAR_ARCHIVE );

	neartalk	= gEngfuncs.pfnRegisterVariable ( "neartalk", "0", FCVAR_ARCHIVE );
	neardoor	= gEngfuncs.pfnRegisterVariable ( "neardoor", "0", FCVAR_ARCHIVE );
	nearweapon	= gEngfuncs.pfnRegisterVariable ( "nearweapon", "0", FCVAR_ARCHIVE );
	nearbutton	= gEngfuncs.pfnRegisterVariable ( "nearbutton", "0", FCVAR_ARCHIVE );

	game_tr_completed	= gEngfuncs.pfnRegisterVariable ( "game_tr_completed", "0", FCVAR_ARCHIVE );
	game_all_completed	= gEngfuncs.pfnRegisterVariable ( "game_all_completed", "0", FCVAR_ARCHIVE );
}

// CHud destructor
// cleans up memory allocated for m_rg* arrays
CHud :: ~CHud()
{
	delete pParticleManager;
	pParticleManager = NULL;

	delete [] m_rghSprites;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;

	gMP3.Shutdown();

	//LRC - clear all shiny surfaces
	if (m_pShinySurface)
	{
		delete m_pShinySurface;
		m_pShinySurface = NULL;
	}

	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	ResetRain();
	ServersShutdown();
}

// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rghSprites[] array
// returns 0 if sprite not found
int CHud :: GetSpriteIndex( const char *SpriteName )
{
	// look through the loaded sprite name list for SpriteName
	for ( int i = 0; i < m_iSpriteCount; i++ )
	{
		if ( strncmp( SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
			return i;
	}

	return -1; // invalid sprite
}

void CHud :: VidInit( void )
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	// ----------
	// Load Sprites
	// ---------
//	m_hsprFont = LoadSprite("sprites/%d_font.spr");
	
	m_hsprLogo = 0;	
	m_hsprCursor = 0;
	ResetRain();

	//LRC - clear all shiny surfaces
	if (m_pShinySurface)
	{
		delete m_pShinySurface;
		m_pShinySurface = NULL;
	}

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	// Only load this once
	if ( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList("scripts/hud.txt", &m_iSpriteCountAllRes);
//		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		if (m_pSpriteList)
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;
			for ( int j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
					m_iSpriteCount++;
				p++;
			}

			// allocated memory for sprite handle arrays
 			m_rghSprites = new HSPRITE[m_iSpriteCount];
			m_rgrcRects = new wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];

			p = m_pSpriteList;
			int index = 0;
			for ( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
				{
					char sz[256];
					sprintf(sz, "sprites/%s.spr", p->szSprite);
					m_rghSprites[index] = SPR_Load(sz);
					m_rgrcRects[index] = p->rc;
					strncpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );

					index++;
				}

				p++;
			}
		}
	}
	else
	{
		// we have already have loaded the sprite reference from hud.txt, but
		// we need to make sure all the sprites have been loaded (we've gone through a transition, or loaded a save game)
		client_sprite_t *p = m_pSpriteList;
		int index = 0;
		for ( int j = 0; j < m_iSpriteCountAllRes; j++ )
		{
			if ( p->iRes == m_iRes )
			{
				char sz[256];
				sprintf( sz, "sprites/%s.spr", p->szSprite );
				m_rghSprites[index] = SPR_Load(sz);
				index++;
			}

			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;

	m_Ammo.VidInit();
	m_Health.VidInit();
	m_Spectator.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Battery.VidInit();
	m_Flash.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_AmmoSecondary.VidInit();
	m_TextMessage.VidInit();
	m_StatusIcons.VidInit();
	GetClientVoiceMgr()->VidInit();
	// advanced NVG
	m_NVG.VidInit();
	// advanced NVG
	m_Cine.VidInit();
	m_Ricardo.VidInit();
	m_LuzBar.VidInit();
	m_FlashBar.VidInit();
	m_HudRadar.VidInit();
	m_Money.VidInit();
	m_Tbutton.VidInit();

	if(pParticleManager)
	{
		delete pParticleManager;
	}

	//sys add
	pParticleManager = new CParticleSystemManager;
	pParticleManager->PrecacheTextures();

	if ( CVAR_GET_FLOAT( "cl_playmusic" ) == 1 )
		gMP3.PlayMP3( "", -1 );
	  
//	gEngfuncs.Con_Printf("Arrange Mode %s, Copyright (C) 2004 - 2007, Arrange Mode Team and Contributors\n");
 //   gEngfuncs.Con_Printf("TArrange Mode %s comes with ABSOLUTELY NO WARRANTY\n");
 //   gEngfuncs.Con_Printf("This is free software, and you are welcome to redistribute it\n");
  //  gEngfuncs.Con_Printf("This software is licensed under the GNU LGPL and comes with a copy of its source\n");
}

int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

float g_lastFOV = 0.0;

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out)
{
	int len, start, end;

	len = strlen( in );
	
	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;
	
	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else 
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else 
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[ 1024 ];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if ( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if ( !stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV( void )
{
	if ( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		int i = 0;
		unsigned char buf[ 100 ];

		// Active
		*( float * )&buf[ i ] = g_lastFOV;
		i += sizeof( float );

		Demo_WriteBuffer( TYPE_ZOOM, i, buf );
	}

	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}

int CHud::MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int newfov = READ_BYTE();
	int def_fov = CVAR_GET_FLOAT( "default_fov" );

	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
//	if ( cl_lw && cl_lw->value )
//		return 1;
//SP VERC
	g_lastFOV = newfov;

	if ( newfov == 0 )
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if ( m_iFOV == def_fov )
	{  
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{  
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	return 1;
}


void CHud::AddHudElem(CHudBase *phudelem)
{
	HUDLIST *pdl, *ptemp;

//phudelem->Think();

	if (!phudelem)
		return;

	pdl = (HUDLIST *)malloc(sizeof(HUDLIST));
	if (!pdl)
		return;

	memset(pdl, 0, sizeof(HUDLIST));
	pdl->p = phudelem;

	if (!m_pHudList)
	{
		m_pHudList = pdl;
		return;
	}

	ptemp = m_pHudList;

	while (ptemp->pNext)
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

float CHud::GetSensitivity( void )
{
	return m_flMouseSensitivity;
}


