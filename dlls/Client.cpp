/***
*
*		   ��
*			 �    �������
*			� �   �  �  � 
*		   �   �  �  �  �
*		  �     � �  �  �
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
  
+New models, sprites and sounds to be precached
*Minor modifications on Kill Command
+Add multiple user commands (see clientcommands())

***/
/*

===== client.cpp ========================================================

  client/server game specific stuff

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "spectator.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"

#include "ctype.h"

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL int		g_iSkillLevel;
extern DLL_GLOBAL ULONG		g_ulFrameCount;

extern void CopyToBodyQue(entvars_t* pev);
extern int giPrecacheGrunt;
extern int giPrecacheModelsForDev;
extern int gmsgSayText;
extern int gmsgPlayMP3; //AJH - Killars MP3player

extern int g_teamplay;
extern int gmsgClcommand;

extern int gmsgMOTD;//test
extern int gmsgServerName;
extern int gmsgTbutton;

void LinkUserMessages( void );

/*
 * used by kill command and disconnect command
 * ROBIN: Moved here from player.cpp, to allow multiple player models
 */
void set_suicide_frame(entvars_t* pev)
{       
	if (!FStrEq(STRING(pev->model), "models/player.mdl"))
		return; // allready gibbed

//	pev->frame		= $deatha11;
	pev->solid		= SOLID_NOT;
	pev->movetype	= MOVETYPE_TOSS;
	pev->deadflag	= DEAD_DEAD;
	pev->nextthink	= -1;
}


/*
===========
ClientConnect

called when a player connects to a server
============
*/
BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ]  )
{	
	return g_pGameRules->ClientConnected( pEntity, pszName, pszAddress, szRejectReason );
}


/*
===========
ClientDisconnect

called when a player disconnects from a server

GLOBALS ASSUMED SET:  g_fGameOver
============
*/
void ClientDisconnect( edict_t *pEntity )
{
	if (g_fGameOver)
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if ( !pPlayer )
		return;

	// Un petit message � tout le monde
	UTIL_ClientPrintAll( HUD_PRINTTALK, "#pl_leftgame", STRING(pEntity->v.netname) );

	char text[256];
	sprintf( text, "- %s has left the game\n", STRING(pEntity->v.netname) );
	MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_STRING( text );
	MESSAGE_END();

	CSound *pSound;
	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( pEntity ) );
	{
		// since this client isn't around to think anymore, reset their sound. 
		if ( pSound )
		{
			pSound->Reset();
		}
	}

	MESSAGE_BEGIN( MSG_ONE, gmsgTbutton, NULL, pPlayer->pev );
		WRITE_SHORT( 0 );
	MESSAGE_END();

// since the edict doesn't get deleted, fix it so it doesn't interfere.
	pEntity->v.takedamage = DAMAGE_NO;// don't attract autoaim
	pEntity->v.solid = SOLID_NOT;// nonsolid
	UTIL_SetOrigin ( &pEntity->v, pEntity->v.origin );

	g_pGameRules->ClientDisconnected( pEntity );
}


// called by ClientKill and DeadThink
void respawn(entvars_t* pev, BOOL fCopyCorpse)
{
	if (gpGlobals->coop || gpGlobals->deathmatch)
	{
		if ( fCopyCorpse )
		{
			// make a copy of the dead body for appearances sake
			CopyToBodyQue(pev);
		}

		// respawn player
		GetClassPtr( (CBasePlayer *)pev)->Spawn( );
	}
	else
	{       // restart the entire server
		SERVER_COMMAND("reload\n");
	}
}

/*
============
ClientKill

Player entered the suicide command

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
============
*/
void ClientKill( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;

	CBasePlayer *pl = (CBasePlayer*) CBasePlayer::Instance( pev );

	if ( pl->m_fNextSuicideTime > gpGlobals->time )
		return;  // prevent suiciding too ofter

	pl->m_fNextSuicideTime = gpGlobals->time + 1;  // don't let them suicide for 5 seconds after suiciding

	// have the player kill themself
	pev->health = 0;
	pl->Killed( pev, GIB_NEVER );

	//SOUND
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/damage/suicide.wav", 0.9, ATTN_NORM); 

	// SHAKE
	pev->punchangle.x = RANDOM_LONG(0,34) - 5;
	pev->punchangle.z = RANDOM_LONG(0,49) - 25;
	pev->punchangle.y = RANDOM_LONG(0,89) - 45;
}

/*
===========
ClientPutInServer

called each time a player is spawned
============
*/
void ClientPutInServer( edict_t *pEntity )
{
	CBasePlayer *pPlayer;

	entvars_t *pev = &pEntity->v;

	pPlayer = GetClassPtr((CBasePlayer *)pev);
	pPlayer->SetCustomDecalFrames(-1); // Assume none;

	// Allocate a CBasePlayer for pev, and call spawn
	pPlayer->Spawn() ;

	// Reset interpolation during first frame
	pPlayer->pev->effects |= EF_NOINTERP;
}

#include "voice_gamemgr.h"
extern CVoiceGameMgr g_VoiceGameMgr;

//// HOST_SAY
// String comes in as
// say blah blah blah
// or as
// blah blah blah
// SysOp edit: You forgot "blah blah blah blah" ...
//
void Host_Say( edict_t *pEntity, int teamonly )
{
	CBasePlayer *client;
	int		j;
	char	*p;
	char	text[128];
	char    szTemp[256];
	const char *cpSay = "say";
	const char *cpSayTeam = "say_team";
	const char *pcmd = CMD_ARGV(0);

	// We can get a raw string now, without the "say " prepended
	if ( CMD_ARGC() == 0 )
		return;

	entvars_t *pev = &pEntity->v;
	CBasePlayer* player = GetClassPtr((CBasePlayer *)pev);

	//Not yet.
	if ( player->m_flNextChatTime > gpGlobals->time )
		 return;

	if ( !stricmp( pcmd, cpSay) || !stricmp( pcmd, cpSayTeam ) )
	{
		if ( CMD_ARGC() >= 2 )
		{
			p = (char *)CMD_ARGS();
		}
		else
		{
			// say with a blank message, nothing to do
			return;
		}
	}
	else  // Raw text, need to prepend argv[0]
	{
		if ( CMD_ARGC() >= 2 )
		{
			sprintf( szTemp, "%s %s", ( char * )pcmd, (char *)CMD_ARGS() );
		}
		else
		{
			// Just a one word command, use the first word...sigh
			sprintf( szTemp, "%s", ( char * )pcmd );
		}
		p = szTemp;
	}

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// make sure the text has content
	for ( char *pc = p; pc != NULL && *pc != 0; pc++ )
	{
		if ( isprint( *pc ) && !isspace( *pc ) )
		{
			pc = NULL;	// we've found an alphanumeric character,  so text is valid
			break;
		}
	}
	if ( pc != NULL )
		return;  // no character found, so say nothing

// turn on color set 2  (color on,  no sound)
	if ( teamonly )
		sprintf( text, "%c(TEAM) %s: ", 2, STRING( pEntity->v.netname ) );
	else
		sprintf( text, "%c%s: ", 2, STRING( pEntity->v.netname ) );

	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ( (int)strlen(p) > j )
		p[j] = 0;

	strcat( text, p );
	strcat( text, "\n" );


	player->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;

	// loop through all players
	// Start with the first player.
	// This may return the world in single player if the client types something between levels or during spawn
	// so check it, or it will infinite loop

	client = NULL;
	while ( ((client = (CBasePlayer*)UTIL_FindEntityByClassname( client, "player" )) != NULL) && (!FNullEnt(client->edict())) ) 
	{
		if ( !client->pev )
			continue;
		
		if ( client->edict() == pEntity )
			continue;

		if ( !(client->IsNetClient()) )	// Not a client ? (should never be true)
			continue;

		// can the receiver hear the sender? or has he muted him?
		if ( g_VoiceGameMgr.PlayerHasBlockedPlayer( client, player ) )
			continue;

		if ( teamonly && g_pGameRules->PlayerRelationship(client, CBaseEntity::Instance(pEntity)) != GR_TEAMMATE )
			continue;

		MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, client->pev );
			WRITE_BYTE( ENTINDEX(pEntity) );
			WRITE_STRING( text );
		MESSAGE_END();

	}

	// print to the sending client
	MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, &pEntity->v );
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_STRING( text );
	MESSAGE_END();

	// echo to server console
	g_engfuncs.pfnServerPrint( text );

	char * temp;
	if ( teamonly )
		temp = "say_team";
	else
		temp = "say";
	
	// team match?
	if ( g_teamplay )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" %s \"%s\"\n", 
			STRING( pEntity->v.netname ), 
			GETPLAYERUSERID( pEntity ),
			GETPLAYERAUTHID( pEntity ),
			g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pEntity ), "model" ),
			temp,
			p );
	}
	else
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%i>\" %s \"%s\"\n", 
			STRING( pEntity->v.netname ), 
			GETPLAYERUSERID( pEntity ),
			GETPLAYERAUTHID( pEntity ),
			GETPLAYERUSERID( pEntity ),
			temp,
			p );
	}
}
//TO DO: I need to make this brief system more customizable. I need to show at least, one picture in each brief
//(this .tga picture can show a map, or where it's the objetives) This could be great

/*
Una copia desde multiplay_gamerules.cpp
A simple hack para ver objetivos del mapa al presionar "B", solo en SP. 

  1-La letra "B" llama al comando "brief" que hace ejecutar esta funcion.
  2-Esta funcion leer� el comando "mp_am_brief" que contiene la ruta necesaria al archivo de texto a mostrar.
  3-El texto a mostrar es cambiado al ejecutar cada mapa en world.cpp, por lo que cada uno tendr� un texto
  especifico

  NOTA: El usuario puede tener acceso a este texto modificando el cvar "mp_am_brief".

*/
// Send the message of the day
// read it chunk-by-chunk,  and send it in parts
// I DON'T THINK SO...

#define MAX_MOTD_CHUNK	  180//60
#define MAX_MOTD_LENGTH   2222 //1536// (MAX_MOTD_CHUNK * 4)

void SendMOTDToClient( edict_t *client )
{
	// read from the MOTD.txt file
	int length, char_count = 0;
	char *pFileList;

	//MOVIDO A WORLD.CPP
	char *aFileList = pFileList = (char*)LOAD_FILE_FOR_ME( (char *)CVAR_GET_STRING( "mp_am_brief" ), &length );

	// send the server name
	MESSAGE_BEGIN( MSG_ONE, gmsgServerName, NULL, client );
		WRITE_STRING( CVAR_GET_STRING("mp_am_brief_name") );
	MESSAGE_END();

	// Send the message of the day
	// read it chunk-by-chunk,  and send it in parts

	while ( pFileList && *pFileList && char_count < MAX_MOTD_LENGTH )
	{
		char chunk[MAX_MOTD_CHUNK+1];
		
		if ( strlen( pFileList ) < MAX_MOTD_CHUNK )
		{
			strcpy( chunk, pFileList );
		}
		else
		{
			strncpy( chunk, pFileList, MAX_MOTD_CHUNK );
			chunk[MAX_MOTD_CHUNK] = 0;		// strncpy doesn't always append the null terminator
		}

		char_count += strlen( chunk );
		if ( char_count < MAX_MOTD_LENGTH )
			pFileList = aFileList + char_count; 
		else
			*pFileList = 0;

		MESSAGE_BEGIN( MSG_ONE, gmsgMOTD, NULL, client );
			WRITE_BYTE( *pFileList ? FALSE : TRUE );	// FALSE means there is still more message to come
			WRITE_STRING( chunk );
		MESSAGE_END();
	}

	FREE_FILE( aFileList );
}

/*
===========
ClientCommand
called each time a player uses a "cmd" command
============
*/
extern float g_flWeaponCheat;

// Use CMD_ARGV,  CMD_ARGV, and CMD_ARGC to get pointers the character string command.
void ClientCommand( edict_t *pEntity )
{
	const char *pcmd = CMD_ARGV(0);
	const char *pstr;

	// Is the client spawned yet?
	if ( !pEntity->pvPrivateData )
		return;

	entvars_t *pev = &pEntity->v;

	if ( FStrEq(pcmd, "say" ) )
	{
		Host_Say( pEntity, 0 );
	}
	else if ( FStrEq(pcmd, "say_team" ) )
	{
		Host_Say( pEntity, 1 );
	}
	//TO DO: I need to prevent that the user can change the cvar at console. It needs to be changed ONLY by code.
	//Using this cvar it's the only way I can to store the total XPs in the config file (the user can copy their
	//config file and use it in another machine, for example, and it will keep their XP points)
	//The other way can be making a new .xp file, and open, close and write it when the code needs (bad idea).

	//I made this test but don't work
//sys
	/*
	else if ( FStrEq(pcmd, "xp_points" ) )
	{
		char command[128];

		// check the length of the command (prevents crash)
		// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
		strncpy( command, pcmd, 127 );
		command[127] = '\0';

		// tell the user they entered an unknown command
		ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Comando Desconocido: %s\n", command ) );
		return;//LRC
	}
*/
	else if ( FStrEq(pcmd, "stealth" ) )
	{
		if ( GetClassPtr((CBasePlayer *)pev)->m_fStealth )
		{
			GetClassPtr((CBasePlayer *)pev)->m_fStealth = FALSE;
			ClientPrint(pev, HUD_PRINTCENTER, "#StealthOff");

			SERVER_COMMAND( "cl_luzbar_x 1\n" );
			SERVER_COMMAND( "cl_luzbar_y 1\n" );
		}
		else //else
		{
			GetClassPtr((CBasePlayer *)pev)->m_fStealth = TRUE;
			ClientPrint(pev, HUD_PRINTCENTER, "#StealthOn");

			SERVER_COMMAND( "cl_luzbar_x 45\n" );
			SERVER_COMMAND( "cl_luzbar_y 30\n" );
		}
	}
		
	//BP particle system reiniting
	else if ( FStrEq(pcmd, "reinit_particles" ) )
	{
		// clear all particlesystems with this hijacked message
			
		//Ben - Little addition here so we can use the normal access to the CBasePlayer class
		CBasePlayer *pPlayer;
		pPlayer = GetClassPtr((CBasePlayer *)pev);

		extern int gmsgParticles;
		
		if ( CVAR_GET_FLOAT("r_particles" ) != 0 )			
		{
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
		}

		pPlayer->m_bSpawnPS = true;
		pPlayer->m_bSpawnGrass = true;
		pPlayer->m_flLastPSSpawn = 0.0;
		pPlayer->m_flLastGrassSpawn = 0.0;
		pPlayer->pLastGrassSpawned = NULL;
		pPlayer->pLastPSSpawned = NULL;
	}

	else if (FStrEq(pcmd, "flashlight" )) 
	{ 
		SERVER_COMMAND("impulse 100\n");

	//	ClientPrint(pev, HUD_PRINTCENTER, "#FlashSuit");
	}
	else if (FStrEq(pcmd, "VModEnable" )) 
	{ 
		//well, the player spawns so use this fake command to send logo
	//	SERVER_COMMAND("impulse 99\n");
	}

	else if (FStrEq(pcmd, "radar" )) 
	{
		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);

		if (pPlayer->bRadarIsOn == FALSE )
		{
			pPlayer->bRadarIsOn = TRUE;
			ClientPrint(pev, HUD_PRINTCENTER, "#RadarOn");
		}
		else
		{
			pPlayer->bRadarIsOn = FALSE;
			ClientPrint(pev, HUD_PRINTCENTER, "#RadarOff");
		}
	}

	else if (FStrEq(pcmd, "menubackground" )) 
	{
		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);

		if (pPlayer->m_fRicardoOn)
		{
			pPlayer->RicardoToggle(FALSE);
		}
		else
		{
			pPlayer->RicardoToggle(TRUE);
		}
	}
	
	else if (FStrEq(pcmd, "cinebars" )) 
	{ 
		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);

		if (pPlayer->m_fCineOn)
		{
			pPlayer->CineToggle(FALSE);
		}
		else
		{
			pPlayer->CineToggle(TRUE);
		}
	}

	else if (FStrEq(pcmd, "score" )) 
	{ 
		int iMonstersKilled = CVAR_GET_FLOAT( "score_killed" );
		int iMonstersKilledbyHead = CVAR_GET_FLOAT( "score_head" );
		int iMonstersKilledbyKnife = CVAR_GET_FLOAT( "score_knifed" );

		char szText[201];
		hudtextparms_t     hText;
		sprintf(szText, "Monsters Killed: %i\nHeadshots: %i\nKnifed: %i", iMonstersKilled,iMonstersKilledbyHead,iMonstersKilledbyKnife);
		memset(&hText, 0, sizeof(hText));
		hText.channel = 6;
		hText.x = -1;
		hText.y = -1; //85
		hText.effect = 1;    //0 Fade in/out 1//flickery 2//write out
		hText.r1 = hText.g1 = hText.b1 = 255;
		hText.a1 = 255;
		hText.r2 = hText.g2 = hText.b2 = 255;
		hText.a2 = 255;
		hText.fadeinTime = 0;
		hText.fadeoutTime = 0;
		hText.holdTime = 5;//1.5
		hText.fxTime = 0.5;							
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
			
		if ( pPlayer )
		{
			if ( ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) )//FIX
			UTIL_HudMessage( pPlayer, hText, szText );
		}
	}

	else if (FStrEq(pcmd, "clear_score" )) 
	{ 
		CVAR_SET_FLOAT( "score_killed", 0);
		CVAR_SET_FLOAT( "score_head", 0);
		CVAR_SET_FLOAT( "score_knifed", 0);
	}

	else if ( FStrEq(pcmd, "hev_voice" ) )
	{
		if ( GetClassPtr((CBasePlayer *)pev)->m_bHEVPlayVoice )
		{
			GetClassPtr((CBasePlayer *)pev)->m_bHEVPlayVoice = FALSE;
			ClientPrint(pev, HUD_PRINTCENTER, "#VoiceOff");
				
			// clear out the suit message cache so we don't keep chattering
			GetClassPtr((CBasePlayer *)pev)->SetSuitUpdate(NULL, FALSE, 0);
			
			//use this channel, if not, we won't hear the voice off sound
			//('cause the hev's voice is off, je, ermm... well... you know...)
			EMIT_SOUND_SUIT(GetClassPtr((CBasePlayer *)pev)->edict(), "!HEV_V1");
		}
		else 
		{
			GetClassPtr((CBasePlayer *)pev)->m_bHEVPlayVoice = TRUE;
			ClientPrint(pev, HUD_PRINTCENTER, "#VoiceON");

			EMIT_SOUND_SUIT(GetClassPtr((CBasePlayer *)pev)->edict(), "!HEV_V0");
		}
	}

	else if ( FStrEq(pcmd, "firemode" ) ) //if the command "firemode" is called
	{
		if ( GetClassPtr((CBasePlayer *)pev)->m_fFireMode == 0 )
		{
			GetClassPtr((CBasePlayer *)pev)->m_fFireMode = 1;
			ClientPrint(pev, HUD_PRINTCENTER, "#OICW_GlMode");
		}
		else if ( GetClassPtr((CBasePlayer *)pev)->m_fFireMode == 1 )
		{
			GetClassPtr((CBasePlayer *)pev)->m_fFireMode = 2;
			ClientPrint(pev, HUD_PRINTCENTER, "#OICW_Nvg");
		}
		else if ( GetClassPtr((CBasePlayer *)pev)->m_fFireMode == 2 )
		{
			GetClassPtr((CBasePlayer *)pev)->m_fFireMode = 0;
			ClientPrint(pev, HUD_PRINTCENTER, "#OICW_556Mode");
		}
  }
	else if ( FStrEq(pcmd, "help" ) )
	{
		ClientPrint(pev, HUD_PRINTCENTER, "#AmVer51");
	}

	else if ( FStrEq(pcmd, "brief" ) )
	{
		CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

		if ( !g_pGameRules->IsMultiplayer() )//SOLO EN SP
		SendMOTDToClient( pl->edict() );
		ClientPrint(pev, HUD_PRINTCENTER, "#LoadingInfo");//TEST TEST
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 
	}	
		
	//SELECTION HANDLING (for dialogues)
	else if ( FStrEq(pcmd, "select1" ) )
	{
		CVAR_SET_STRING( "selection", "1" );
			
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 
		ALERT ( at_console, "select1\n"); 
	}

	else if ( FStrEq(pcmd, "select2" ) )
	{
		CVAR_SET_STRING( "selection", "2" );
			
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 
		ALERT ( at_console, "select2\n"); 
	}	

	else if ( FStrEq(pcmd, "select3" ) )
	{
		CVAR_SET_STRING( "selection", "3" );
			
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/menu_on.wav", 0.9, ATTN_NORM); 
		ALERT ( at_console, "select3\n"); 
	}

	else if (FStrEq(pcmd, "menuradio1" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		if (pev->playerclass == 0)
			pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_Radio1");
		
		pPlayer->m_nmenudisp = 1;
		ALERT ( at_console, "menuradio1\n");
	} 
	else if (FStrEq(pcmd, "wepselect" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		if (pev->playerclass == 0)
			pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_Class1");
		
		pPlayer->m_nmenudisp = 2;//2
		ALERT ( at_console, "Menu_Class1\n");
	}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


	else if (FStrEq(pcmd, "create_entity" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		if (!giPrecacheModelsForDev)
		{
			giPrecacheModelsForDev = 1;
			UTIL_ShowMessageAll( STRING(ALLOC_STRING("Menu_Restart")));
		}
		else
		{
			//enabled
			pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_Create");
		
			pPlayer->m_nmenudisp = CREATE_ENTITY_TYPE;
		}
	}

	else if (FStrEq(pcmd, "create_entity_ai" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterAI");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterClass");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_ally" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterAlliesType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ALLY;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterEnemiesType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy_xen" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterXENType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY_XEN;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy_xrace" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterXRaceType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY_XRACE;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy_millitary" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterMillitaryType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY_MILLITARY;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy_hl2monsters" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterHL2MonstersType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY_HL2MONSTERS;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy_hl2monsters_combine" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterCombineType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY_HL2MONSTERS_COMBINE;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy_hl2monsters_zombie" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterZombiesType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY_HL2MONSTERS_ZOMBIES;
	}

	else if (FStrEq(pcmd, "create_entity_ai_class_enemy_terrorists" ))
	{
		CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer *) pev );

		pPlayer->ShowMenu (pPlayer, 0x23F, 0, 0, "#Menu_CreateMonsterTerroristsType");
		
		pPlayer->m_nmenudisp = CREATE_MONSTER_AI_CLASS_ENEMY_TERRORISTS;
	}

	
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

	//activar camara
	else if ( FStrEq(pcmd, "camera") )
	{		
		CBaseEntity *pPlayer = CBaseEntity::Instance(pEntity);

		if (CMD_ARGC() > 1)
		{
			FireTargets(CMD_ARGV(1), pPlayer, pPlayer, USE_TOGGLE, 0);
		}
	}

	else if (FStrEq(pcmd, "radio_backup" )) 
	{ 
		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pev);

		CBaseEntity *pEntityToCall = NULL;
	
		Vector VecSrc;
		VecSrc = pev->origin;
		
		if ( pPlayer->IsAlive() )
		{
			while ((pEntityToCall = UTIL_FindEntityInSphere( pEntityToCall, VecSrc, 384 )) != NULL)//512
			{	
				if ( (pEntityToCall->Classify() == CLASS_PLAYER_ALLY) || (pEntityToCall->Classify() == CLASS_HUMAN_PASSIVE))
				{
					if ( pEntityToCall->IsAlive() )
					{
						pEntityToCall->Use(pPlayer, pPlayer, USE_ON, 0);
								
					//	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/order/radio_backup.wav", 1.0, ATTN_NORM);

						UTIL_ShowMessageAll( STRING(ALLOC_STRING("SQUAD")));

					//	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 1000, 0.3 );//so, it can be detected
					}
				}
			}
		}
	}

	else if ( FStrEq(pcmd, "showmap") )
	{		
		SERVER_COMMAND( "camera camera_map\n" );//esto manda camara (activar) (camera_map)
	}

	else if ( FStrEq(pcmd, "fire") ) //LRC - trigger entities manually
	{								// but the "sv_cheats" value must 
		if (g_flWeaponCheat)		// be "1"
		{
			CBaseEntity *pPlayer = CBaseEntity::Instance(pEntity);
			if (CMD_ARGC() > 1)
			{
				FireTargets(CMD_ARGV(1), pPlayer, pPlayer, USE_TOGGLE, 0);
			}
			else
			{
				TraceResult tr;
				UTIL_MakeVectors(pev->v_angle);
				UTIL_TraceLine(
					pev->origin + pev->view_ofs,
					pev->origin + pev->view_ofs + gpGlobals->v_forward * 1000,
					dont_ignore_monsters, pEntity, &tr
				); 

				if (tr.pHit)
				{
					CBaseEntity *pHitEnt = CBaseEntity::Instance(tr.pHit);
					if (pHitEnt)
					{
						pHitEnt->Use(pPlayer, pPlayer, USE_TOGGLE, 0);
						ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Fired %s \"%s\"\n", STRING(pHitEnt->pev->classname), STRING(pHitEnt->pev->targetname) ) );
					}
				}
			}
		}
	}
	else if ( FStrEq(pcmd, "fullupdate" ) )
	{
		GetClassPtr((CBasePlayer *)pev)->ForceClientDllUpdate(); 
	}
	else if ( FStrEq(pcmd, "give" ) )
	{
		switch (RANDOM_LONG(1,3)) // 11 clases al azar
		{
			case 1: ALERT(at_console, "Perdon?\n"); break;
			case 2: ALERT(at_console, "Que?\n"); break;
			case 3: ALERT(at_console, "Jaja...\n"); break;
		}
	}
	else if ( FStrEq(pcmd, "dame" ) )
	{
		if ( g_flWeaponCheat != 0.0)
		{
			int iszItem = ALLOC_STRING( CMD_ARGV(1) );	// Make a copy of the classname
			GetClassPtr((CBasePlayer *)pev)->GiveNamedItem( STRING(iszItem) );
		}
	}

	else if ( FStrEq(pcmd, "drop" ) )
	{
		// player is dropping an item. 
		GetClassPtr((CBasePlayer *)pev)->DropPlayerItem((char *)CMD_ARGV(1));
	}
	else if ( FStrEq(pcmd, "fov" ) )
	{
		if ( g_flWeaponCheat && CMD_ARGC() > 1)
		{
			GetClassPtr((CBasePlayer *)pev)->m_iFOV = atoi( CMD_ARGV(1) );
		}
		else
		{
			CLIENT_PRINTF( pEntity, print_console, UTIL_VarArgs( "\"fov\" is \"%d\"\n", (int)GetClassPtr((CBasePlayer *)pev)->m_iFOV ) );
		}
	}
	else if ( FStrEq(pcmd, "use" ) )
	{
		GetClassPtr((CBasePlayer *)pev)->SelectItem((char *)CMD_ARGV(1));
	}
	else if (((pstr = strstr(pcmd, "weapon_")) != NULL)  && (pstr == pcmd))
	{
		GetClassPtr((CBasePlayer *)pev)->SelectItem(pcmd);
	}
	else if ( FStrEq(pcmd, "playaudio" ) )  //AJH - MP3/OGG player (based on killars MP3)
	{
		ALERT ( at_console, "playaudio %c\n", (char *)CMD_ARGV(1));

		MESSAGE_BEGIN( MSG_ONE, gmsgPlayMP3, NULL, ENT(pev) );
			WRITE_STRING( (char *)CMD_ARGV(1) ); 
		MESSAGE_END();
	}

	else if (FStrEq(pcmd, "lastinv" ))
	{
		GetClassPtr((CBasePlayer *)pev)->SelectLastItem();
	}
	else if ( FStrEq( pcmd, "spectate" ) && (pev->flags & FL_PROXY) )	// added for proxy support
	{
		CBasePlayer * pPlayer = GetClassPtr((CBasePlayer *)pev);

		edict_t *pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot( pPlayer );
		pPlayer->StartObserver( pev->origin, VARS(pentSpawnSpot)->angles);
	}
	else if ( g_pGameRules->ClientCommand( GetClassPtr((CBasePlayer *)pev), pcmd ) )
	{
		// MenuSelect returns true only if the command is properly handled,  so don't print a warning
	}
	else
	{
		// tell the user they entered an unknown command
		char command[128];

		// check the length of the command (prevents crash)
		// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
		strncpy( command, pcmd, 127 );
		command[127] = '\0';

		// tell the user they entered an unknown command
		ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Comando Desconocido: %s\n", command ) );
	}
}


/*
========================
ClientUserInfoChanged

called after the player changes
userinfo - gives dll a chance to modify it before
it gets sent into the rest of the engine.
========================
*/
void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer )
{
	// Is the client spawned yet?
	if ( !pEntity->pvPrivateData )
		return;
	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	if ( pEntity->v.netname && STRING(pEntity->v.netname)[0] != 0 && !FStrEq( STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" )) )
	{
		char sName[256];
		char *pName = g_engfuncs.pfnInfoKeyValue( infobuffer, "name" );
		strncpy( sName, pName, sizeof(sName) - 1 );
		sName[ sizeof(sName) - 1 ] = '\0';

		// First parse the name and remove any %'s
		for ( char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++ )
		{
			// Replace it with a space
			if ( *pApersand == '%' )
				*pApersand = ' ';
		}

		// Set the name
		g_engfuncs.pfnSetClientKeyValue( ENTINDEX(pEntity), infobuffer, "name", sName );

		char text[256];
		sprintf( text, "* %s changed name to %s\n", STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
			WRITE_BYTE( ENTINDEX(pEntity) );
			WRITE_STRING( text );
		MESSAGE_END();

		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				g_engfuncs.pfnInfoKeyValue( infobuffer, "model" ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				GETPLAYERUSERID( pEntity ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
	}

	g_pGameRules->ClientUserInfoChanged( GetClassPtr((CBasePlayer *)&pEntity->v), infobuffer );
}

static int g_serveractive = 0;

void ServerDeactivate( void )
{
	// It's possible that the engine will call this function more times than is necessary
	//  Therefore, only run it one time for each call to ServerActivate 
	if ( g_serveractive != 1 )
	{
		return;
	}

	g_serveractive = 0;

	// Peform any shutdown operations here...
	//
}

void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	int				i;
	CBaseEntity		*pClass;

	// Every call to ServerActivate should be matched by a call to ServerDeactivate
	g_serveractive = 1;

	// Clients have not been initialized yet
	for ( i = 0; i < edictCount; i++ )
	{
		if ( pEdictList[i].free )
			continue;
		
		// Clients aren't necessarily initialized until ClientPutInServer()
		if ( i < clientMax || !pEdictList[i].pvPrivateData )
			continue;

		pClass = CBaseEntity::Instance( &pEdictList[i] );
		// Activate this entity if it's got a class & isn't dormant
		if ( pClass && !(pClass->pev->flags & FL_DORMANT) )
		{
			pClass->Activate();
		}
		else
		{
			ALERT( at_console, "Can't instance %s\n", STRING(pEdictList[i].v.classname) );
		}
	}

	// Link user messages here to make sure first client can get them...
	LinkUserMessages();
}


/*
================
PlayerPreThink

Called every frame before physics are run
================
*/
void PlayerPreThink( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PreThink( );
}

/*
================
PlayerPostThink

Called every frame after physics are run
================
*/
void PlayerPostThink( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PostThink( );
}



void ParmsNewLevel( void )
{
}


void ParmsChangeLevel( void )
{
	// retrieve the pointer to the save data
	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

	if ( pSaveData )
		pSaveData->connectionCount = BuildChangeList( pSaveData->levelList, MAX_LEVEL_CONNECTIONS );
}


//
// GLOBALS ASSUMED SET:  g_ulFrameCount
//
void StartFrame( void )
{
	if ( g_pGameRules )
		g_pGameRules->Think();

	if ( g_fGameOver )
		return;

	gpGlobals->teamplay = teamplay.value;
	g_ulFrameCount++;
}


void ClientPrecache( void )
{
	// setup precaches always needed
	PRECACHE_SOUND("player/sprayer.wav");			// spray paint sound for PreAlpha

	PRECACHE_SOUND("player/order/1.wav");
	PRECACHE_SOUND("player/order/2.wav");
	PRECACHE_SOUND("player/order/3.wav");
	PRECACHE_SOUND("player/order/4.wav");
	PRECACHE_SOUND("player/order/5.wav");

	PRECACHE_SOUND("player/order/radio_backup.wav");
	
	PRECACHE_SOUND("player/parachute.wav");//sys
	PRECACHE_SOUND("player/menu_on.wav");//sys

	//XP POINTS
	PRECACHE_SOUND("zombie/claw_strike1.wav");//sys
	PRECACHE_SOUND("zombie/claw_strike2.wav");//sys
	PRECACHE_SOUND("player/damage/goslow.wav");//sys

	PRECACHE_SOUND("player/pl_fallpain2.wav");		
	PRECACHE_SOUND("player/pl_fallpain3.wav");		

	PRECACHE_SOUND("player/pl_step1.wav");		// walk on concrete
	PRECACHE_SOUND("player/pl_step2.wav");
	PRECACHE_SOUND("player/pl_step3.wav");
	PRECACHE_SOUND("player/pl_step4.wav");

	PRECACHE_SOUND("player/jump01.wav");
	PRECACHE_SOUND("player/jump02.wav");
	PRECACHE_SOUND("player/jump03.wav");
	PRECACHE_SOUND("player/jump04.wav");

	PRECACHE_SOUND("common/npc_step1.wav");		// NPC walk on concrete
	PRECACHE_SOUND("common/npc_step2.wav");
	PRECACHE_SOUND("common/npc_step3.wav");
	PRECACHE_SOUND("common/npc_step4.wav");

	PRECACHE_SOUND("player/pl_metal1.wav");		// walk on metal
	PRECACHE_SOUND("player/pl_metal2.wav");
	PRECACHE_SOUND("player/pl_metal3.wav");
	PRECACHE_SOUND("player/pl_metal4.wav");

	PRECACHE_SOUND("player/pl_dirt1.wav");		// walk on dirt
	PRECACHE_SOUND("player/pl_dirt2.wav");
	PRECACHE_SOUND("player/pl_dirt3.wav");
	PRECACHE_SOUND("player/pl_dirt4.wav");

	PRECACHE_SOUND("player/pl_duct1.wav");		// walk in duct
	PRECACHE_SOUND("player/pl_duct2.wav");
	PRECACHE_SOUND("player/pl_duct3.wav");
	PRECACHE_SOUND("player/pl_duct4.wav");

	PRECACHE_SOUND("player/pl_grate1.wav");		// walk on grate
	PRECACHE_SOUND("player/pl_grate2.wav");
	PRECACHE_SOUND("player/pl_grate3.wav");
	PRECACHE_SOUND("player/pl_grate4.wav");

	PRECACHE_SOUND("player/pl_slosh1.wav");		// walk in shallow water
	PRECACHE_SOUND("player/pl_slosh2.wav");
	PRECACHE_SOUND("player/pl_slosh3.wav");
	PRECACHE_SOUND("player/pl_slosh4.wav");

	PRECACHE_SOUND("player/pl_tile1.wav");		// walk on tile
	PRECACHE_SOUND("player/pl_tile2.wav");
	PRECACHE_SOUND("player/pl_tile3.wav");
	PRECACHE_SOUND("player/pl_tile4.wav");
	PRECACHE_SOUND("player/pl_tile5.wav");

	PRECACHE_SOUND("player/pl_swim1.wav");		// breathe bubbles
	PRECACHE_SOUND("player/pl_swim2.wav");
	PRECACHE_SOUND("player/pl_swim3.wav");
	PRECACHE_SOUND("player/pl_swim4.wav");

	PRECACHE_SOUND("player/pl_ladder1.wav");	// climb ladder rung
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");

	PRECACHE_SOUND("player/pl_wade1.wav");		// wade in water
	PRECACHE_SOUND("player/pl_wade2.wav");
	PRECACHE_SOUND("player/pl_wade3.wav");
	PRECACHE_SOUND("player/pl_wade4.wav");

	PRECACHE_SOUND("debris/wood1.wav");			// hit wood texture
	PRECACHE_SOUND("debris/wood2.wav");
	PRECACHE_SOUND("debris/wood3.wav");

	PRECACHE_SOUND("plats/train_use1.wav");		// use a train

	PRECACHE_SOUND("buttons/spark5.wav");		// hit computer texture
	PRECACHE_SOUND("buttons/spark6.wav");
	PRECACHE_SOUND("debris/glass1.wav");
	PRECACHE_SOUND("debris/glass2.wav");
	PRECACHE_SOUND("debris/glass3.wav");

	PRECACHE_SOUND( SOUND_FLASHLIGHT_ON );
	PRECACHE_SOUND( SOUND_FLASHLIGHT_OFF );

// player gib sounds
	PRECACHE_SOUND("common/bodysplat.wav");		               

	PRECACHE_SOUND("player/pl_pain5.wav");
	PRECACHE_SOUND("player/pl_pain6.wav");
	PRECACHE_SOUND("player/pl_pain7.wav");
	
//|SP|
	PRECACHE_SOUND( "weapons/weapon_deploy.wav" ); 

	PRECACHE_SOUND( "player/damage/headshot1.wav" ); 
	PRECACHE_SOUND( "player/damage/headshot2.wav" ); // HUMAN HEADSHOTs
	PRECACHE_SOUND( "player/damage/headshot3.wav" );

	PRECACHE_SOUND( "player/damage/helmet1.wav" ); // Casco head
	PRECACHE_SOUND( "player/damage/helmet2.wav" );
	PRECACHE_SOUND( "player/damage/helmet3.wav" );

	PRECACHE_SOUND( "player/damage/hit_kevlar-1.wav" );
	PRECACHE_SOUND( "player/damage/hit_kevlar-2.wav" ); // si le pega en el chaleco
	PRECACHE_SOUND( "player/damage/hit_kevlar-3.wav" );

	PRECACHE_SOUND( "player/damage/die_water.wav" );

	PRECACHE_SOUND( "player/damage/corazon.wav" );
	PRECACHE_SOUND( "player/damage/respiracion.wav" );

	PRECACHE_SOUND( "player/damage/stomach1.wav" );
	PRECACHE_SOUND( "player/damage/stomach2.wav" );//stomago

	PRECACHE_SOUND( "player/damage/fredisdead.wav" );

//	PRECACHE_MODEL("models/parachute_open.mdl");

	PRECACHE_MODEL("models/NULL.mdl");//used by player body
	PRECACHE_MODEL("sprites/null.spr");

	PRECACHE_MODEL("models/player.mdl");
	PRECACHE_MODEL("models/player/body/body.mdl");
	
	// hud sounds
	PRECACHE_SOUND("common/wpn_hudoff.wav");
	PRECACHE_SOUND("common/wpn_hudon.wav");
	PRECACHE_SOUND("common/wpn_moveselect.wav");
	PRECACHE_SOUND("common/wpn_select.wav");
	PRECACHE_SOUND("common/wpn_denyselect.wav");

	if (giPrecacheGrunt)
		UTIL_PrecacheOther("monster_human_grunt");
	
	if (giPrecacheModelsForDev)
	{
		UTIL_PrecacheOther("monster_human_grunt");
		UTIL_PrecacheOther("monster_human_grunt_rpg");
		UTIL_PrecacheOther("monster_hgrunt_opfor");

		UTIL_PrecacheOther("monster_zombie");
		UTIL_PrecacheOther("monster_headcrab");

		UTIL_PrecacheOther("monster_gargantua");
/*
		UTIL_PrecacheOther("npc_combine_metrocop");
		UTIL_PrecacheOther("npc_combine");
		UTIL_PrecacheOther("npc_combine_supersoldier");

		UTIL_PrecacheOther("npc_zombie");
		UTIL_PrecacheOther("npc_zombie_fast");
		UTIL_PrecacheOther("npc_zombie_poison");
		UTIL_PrecacheOther("npc_headcrab_poison");
*/
//		UTIL_PrecacheOther("monster_terrorist");
		UTIL_PrecacheOther("monster_hgrunt_ally");

		UTIL_PrecacheOther("monster_barney");
		UTIL_PrecacheOther("monster_otis");
		UTIL_PrecacheOther("monster_hevsci");
		UTIL_PrecacheOther("monster_scientist");

		UTIL_PrecacheOther("monster_houndeye");
		UTIL_PrecacheOther("monster_alien_grunt");
		UTIL_PrecacheOther("monster_alien_slave");
	}
}

/*
===============
GetGameDescription

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return "Arrange Mode";
}

/*
================
Sys_Error

Engine is going to shut down, allows setting a breakpoint in game .dll to catch that occasion
================
*/
void Sys_Error( const char *error_string )
{
	// Default case, do nothing.  MOD AUTHORS:  Add code ( e.g., _asm { int 3 }; here to cause a breakpoint for debugging your game .dlls
}

/*
================
PlayerCustomization

A new player customization has been registered on the server
UNDONE:  This only sets the # of frames of the spray can logo
animation right now.
================
*/
void PlayerCustomization( edict_t *pEntity, customization_t *pCust )
{
	entvars_t *pev = &pEntity->v;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_console, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_console, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
	case t_decal:
		pPlayer->SetCustomDecalFrames(pCust->nUserData2); // Second int is max # of frames.
		break;
	case t_sound:
	case t_skin:
	case t_model:
		// Ignore for now.
		break;
	default:
		ALERT(at_console, "PlayerCustomization:  Unknown customization type!\n");
		break;
	}
}

/*
================
SpectatorConnect

A spectator has joined the game
================
*/
void SpectatorConnect( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorConnect( );
}

/*
================
SpectatorConnect

A spectator has left the game
================
*/
void SpectatorDisconnect( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorDisconnect( );
}

/*
================
SpectatorConnect

A spectator has sent a usercmd
================
*/
void SpectatorThink( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorThink( );
}

////////////////////////////////////////////////////////
// PAS and PVS routines for client messaging
//

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas )
{
	Vector org;
	edict_t *pView = pClient;

	// Find the client's PVS
	if ( pViewEntity )
	{
		pView = pViewEntity;
	}

	if ( pClient->v.flags & FL_PROXY )
	{
		*pvs = NULL;	// the spectator proxy sees
		*pas = NULL;	// and hears everything
		return;
	}

	org = pView->v.origin + pView->v.view_ofs;
	if ( pView->v.flags & FL_DUCKING )
	{
		org = org + ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	}

	*pvs = ENGINE_SET_PVS ( (float *)&org );
	*pas = ENGINE_SET_PAS ( (float *)&org );
}

#include "entity_state.h"

/*
AddToFullPack

Return 1 if the entity state has been filled in for the ent and the entity will be propagated to the client, 0 otherwise

state is the server maintained copy of the state info that is transmitted to the client
a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
e and ent are the entity that is being added to the update, if 1 is returned
host is the player's edict of the player whom we are sending the update to
player is 1 if the ent/e is a player and 0 otherwise
pSet is either the PAS or PVS that we previous set up.  We can use it to ask the engine to filter the entity against the PAS or PVS.
we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.  Caching the value is valid in that case, but still only for the current frame
*/
int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet )
{
	int					i;

	// don't send if flagged for NODRAW and it's not the host getting the message
	if ( ( ent->v.effects == EF_NODRAW ) &&
		 ( ent != host ) )
		return 0;

	// Ignore ents without valid / visible models
	if ( !ent->v.modelindex || !STRING( ent->v.model ) )
		return 0;

	// Don't send spectators to other players
	if ( ( ent->v.flags & FL_SPECTATOR ) && ( ent != host ) )
	{
		return 0;
	}

	// Ignore if not the host and not touching a PVS/PAS leaf
	// If pSet is NULL, then the test will always succeed and the entity will be added to the update
	if ( ent != host )
	{
		if ( !ENGINE_CHECK_VISIBILITY( (const struct edict_s *)ent, pSet ) )
		{
			return 0;
		}
	}


	// Don't send entity to local client if the client says it's predicting the entity itself.
	if ( ent->v.flags & FL_SKIPLOCALHOST )
	{
		if ( ( hostflags & 1 ) && ( ent->v.owner == host ) )
			return 0;
	}
	
	if ( host->v.groupinfo )
	{
		UTIL_SetGroupTrace( host->v.groupinfo, GROUP_OP_AND );

		// Should always be set, of course
		if ( ent->v.groupinfo )
		{
			if ( g_groupop == GROUP_OP_AND )
			{
				if ( !(ent->v.groupinfo & host->v.groupinfo ) )
					return 0;
			}
			else if ( g_groupop == GROUP_OP_NAND )
			{
				if ( ent->v.groupinfo & host->v.groupinfo )
					return 0;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset( state, 0, sizeof( *state ) );

	// Assign index so we can track this entity from frame to frame and
	//  delta from it.
	state->number	  = e;
	state->entityType = ENTITY_NORMAL;
	
	// Flag custom entities.
	if ( ent->v.flags & FL_CUSTOMENTITY )
	{
		state->entityType = ENTITY_BEAM;
	}

	// 
	// Copy state data
	//

	// Round animtime to nearest millisecond
	state->animtime   = (int)(1000.0 * ent->v.animtime ) / 1000.0;

	memcpy( state->origin, ent->v.origin, 3 * sizeof( float ) );
	memcpy( state->angles, ent->v.angles, 3 * sizeof( float ) );
	memcpy( state->mins, ent->v.mins, 3 * sizeof( float ) );
	memcpy( state->maxs, ent->v.maxs, 3 * sizeof( float ) );

	memcpy( state->startpos, ent->v.startpos, 3 * sizeof( float ) );
	memcpy( state->endpos, ent->v.endpos, 3 * sizeof( float ) );

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;

	state->modelindex = ent->v.modelindex;
		
	state->frame      = ent->v.frame;

	state->skin       = ent->v.skin;
	state->effects    = ent->v.effects;

	// This non-player entity is being moved by the game .dll and not the physics simulation system
	//  make sure that we interpolate it's position on the client if it moves
	if ( !player &&
		 ent->v.animtime &&
		 ent->v.velocity[ 0 ] == 0 && 
		 ent->v.velocity[ 1 ] == 0 && 
		 ent->v.velocity[ 2 ] == 0 )
	{
		state->eflags |= EFLAG_SLERP;
	}

	state->scale	  = ent->v.scale;
	state->solid	  = ent->v.solid;
	state->colormap   = ent->v.colormap;

	state->movetype   = ent->v.movetype;
	state->sequence   = ent->v.sequence;
	state->framerate  = ent->v.framerate;
	state->body       = ent->v.body;

	for (i = 0; i < 4; i++)
	{
		state->controller[i] = ent->v.controller[i];
	}

	for (i = 0; i < 2; i++)
	{
		state->blending[i]   = ent->v.blending[i];
	}

	state->rendermode    = ent->v.rendermode;
	state->renderamt     = ent->v.renderamt; 
	state->renderfx      = ent->v.renderfx;
	state->rendercolor.r = ent->v.rendercolor.x;
	state->rendercolor.g = ent->v.rendercolor.y;
	state->rendercolor.b = ent->v.rendercolor.z;

	state->aiment = 0;
	if ( ent->v.aiment )
	{
		state->aiment = ENTINDEX( ent->v.aiment );
	}

	state->owner = 0;
	if ( ent->v.owner )
	{
		int owner = ENTINDEX( ent->v.owner );
		
		// Only care if owned by a player
		if ( owner >= 1 && owner <= gpGlobals->maxClients )
		{
			state->owner = owner;	
		}
	}

	// HACK:  Somewhat...
	// Class is overridden for non-players to signify a breakable glass object ( sort of a class? )
	if ( !player )
	{
		state->playerclass  = ent->v.playerclass;
	}

	// Special stuff for players only
	if ( player )
	{
		memcpy( state->basevelocity, ent->v.basevelocity, 3 * sizeof( float ) );

		state->weaponmodel  = MODEL_INDEX( STRING( ent->v.weaponmodel ) );
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction     = ent->v.friction;

		state->gravity      = ent->v.gravity;
//		state->team			= ent->v.team;
//		
		state->usehull      = ( ent->v.flags & FL_DUCKING ) ? 1 : 0;
		state->health		= ent->v.health;
	}

	return 1;
}

// defaults for clientinfo messages
#define	DEFAULT_VIEWHEIGHT	28

/*
===================
CreateBaseline

Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
===================
*/
void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs )
{
	baseline->origin		= entity->v.origin;
	baseline->angles		= entity->v.angles;
	baseline->frame			= entity->v.frame;
	baseline->skin			= (short)entity->v.skin;

	// render information
	baseline->rendermode	= (byte)entity->v.rendermode;
	baseline->renderamt		= (byte)entity->v.renderamt;
	baseline->rendercolor.r	= (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g	= (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b	= (byte)entity->v.rendercolor.z;
	baseline->renderfx		= (byte)entity->v.renderfx;

	if ( player )
	{
		baseline->mins			= player_mins;
		baseline->maxs			= player_maxs;

		baseline->colormap		= eindex;
		baseline->modelindex	= playermodelindex;
		baseline->friction		= 1.0;
		baseline->movetype		= MOVETYPE_WALK;

		baseline->scale			= entity->v.scale;
		baseline->solid			= SOLID_SLIDEBOX;
		baseline->framerate		= 1.0;
		baseline->gravity		= 1.0;

	}
	else
	{
		baseline->mins			= entity->v.mins;
		baseline->maxs			= entity->v.maxs;

		baseline->colormap		= 0;
		baseline->modelindex	= entity->v.modelindex;//SV_ModelIndex(pr_strings + entity->v.model);
		baseline->movetype		= entity->v.movetype;

		baseline->scale			= entity->v.scale;
		baseline->solid			= entity->v.solid;
		baseline->framerate		= entity->v.framerate;
		baseline->gravity		= entity->v.gravity;
	}
}

typedef struct
{
	char name[32];
	int	 field;
} entity_field_alias_t;

#define FIELD_ORIGIN0			0
#define FIELD_ORIGIN1			1
#define FIELD_ORIGIN2			2
#define FIELD_ANGLES0			3
#define FIELD_ANGLES1			4
#define FIELD_ANGLES2			5

static entity_field_alias_t entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
};

void Entity_FieldInit( struct delta_s *pFields )
{
	entity_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN0 ].name );
	entity_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN1 ].name );
	entity_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN2 ].name );
	entity_field_alias[ FIELD_ANGLES0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES0 ].name );
	entity_field_alias[ FIELD_ANGLES1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES1 ].name );
	entity_field_alias[ FIELD_ANGLES2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES2 ].name );
}

/*
==================
Entity_Encode

Callback for sending entity_state_t info over network. 
FIXME:  Move to script
==================
*/
void Entity_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->impacttime != 0 ) && ( t->starttime != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );

		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

static entity_field_alias_t player_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
};

void Player_FieldInit( struct delta_s *pFields )
{
	player_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN0 ].name );
	player_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN1 ].name );
	player_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN2 ].name );
}

/*
==================
Player_Encode

Callback for sending entity_state_t for players info over network. 
==================
*/
void Player_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Player_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

#define CUSTOMFIELD_ORIGIN0			0
#define CUSTOMFIELD_ORIGIN1			1
#define CUSTOMFIELD_ORIGIN2			2
#define CUSTOMFIELD_ANGLES0			3
#define CUSTOMFIELD_ANGLES1			4
#define CUSTOMFIELD_ANGLES2			5
#define CUSTOMFIELD_SKIN			6
#define CUSTOMFIELD_SEQUENCE		7
#define CUSTOMFIELD_ANIMTIME		8

entity_field_alias_t custom_entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
	{ "skin",				0 },
	{ "sequence",			0 },
	{ "animtime",			0 },
};

void Custom_Entity_FieldInit( struct delta_s *pFields )
{
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].name );
}

/*
==================
Custom_Encode

Callback for sending entity_state_t info ( for custom entities ) over network. 
FIXME:  Move to script
==================
*/
void Custom_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int beamType;
	static int initialized = 0;

	if ( !initialized )
	{
		Custom_Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	beamType = t->rendermode & 0x0f;
		
	if ( beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field );
	}

	if ( beamType != BEAM_POINTS )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field );
	}

	if ( beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field );
	}

	// animtime is compared by rounding first
	// see if we really shouldn't actually send it
	if ( (int)f->animtime == (int)t->animtime )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field );
	}
}

/*
=================
RegisterEncoders

Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
=================
*/
void RegisterEncoders( void )
{
	DELTA_ADDENCODER( "Entity_Encode", Entity_Encode );
	DELTA_ADDENCODER( "Custom_Encode", Custom_Encode );
	DELTA_ADDENCODER( "Player_Encode", Player_Encode );
}

int GetWeaponData( struct edict_s *player, struct weapon_data_s *info )
{
#if defined( CLIENT_WEAPONS )
	int i;
	weapon_data_t *item;
	entvars_t *pev = &player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );
	CBasePlayerWeapon *gun;
	
	ItemInfo II;

	memset( info, 0, 32 * sizeof( weapon_data_t ) );

	if ( !pl )
		return 1;

	// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( pl->m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem *pPlayerItem = pl->m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
				if ( gun && gun->UseDecrement() )
				{
					// Get The ID.
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );

					if ( II.iId >= 0 && II.iId < 32 )
					{
						item = &info[ II.iId ];
					 	
						item->m_iId						= II.iId;
						item->m_iClip					= gun->m_iClip;

						item->m_flTimeWeaponIdle		= max( gun->m_flTimeWeaponIdle, -0.001 );
						item->m_flNextPrimaryAttack		= max( gun->m_flNextPrimaryAttack, -0.001 );
						item->m_flNextSecondaryAttack	= max( gun->m_flNextSecondaryAttack, -0.001 );
						item->m_fInReload				= gun->m_fInReload;
						item->m_fInSpecialReload		= gun->m_fInSpecialReload;
						item->fuser1					= max( gun->pev->fuser1, -0.001 );
						item->fuser2					= gun->m_flStartThrow;
						item->fuser3					= gun->m_flReleaseThrow;
						item->iuser1					= gun->m_chargeReady;
						item->iuser2					= gun->m_fInAttack;
						item->iuser3					= gun->m_fireState;
		//SP			
						item->iuser3					= gun->m_fDefaultAnim; //Hier ist Sie die Variable die Wunder bewirkt

											
//						item->m_flPumpTime				= max( gun->m_flPumpTime, -0.001 );
					}
				}
				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}
#else
	memset( info, 0, 32 * sizeof( weapon_data_t ) );
#endif
	return 1;
}

/*
=================
UpdateClientData

Data sent to current client only
engine sets cd to 0 before calling.
=================
*/
void UpdateClientData ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd )
{
	cd->flags			= ent->v.flags;
	cd->health			= ent->v.health;

	cd->viewmodel		= MODEL_INDEX( STRING( ent->v.viewmodel ) );

	cd->waterlevel		= ent->v.waterlevel;
	cd->watertype		= ent->v.watertype;
	cd->weapons			= ent->v.weapons;

	// Vectors
	cd->origin			= ent->v.origin;
	cd->velocity		= ent->v.velocity;
	cd->view_ofs		= ent->v.view_ofs;
	cd->punchangle		= ent->v.punchangle;

	cd->bInDuck			= ent->v.bInDuck;
	cd->flTimeStepSound = ent->v.flTimeStepSound;
	cd->flDuckTime		= ent->v.flDuckTime;
	cd->flSwimTime		= ent->v.flSwimTime;
	cd->waterjumptime	= ent->v.teleport_time;

	strcpy( cd->physinfo, ENGINE_GETPHYSINFO( ent ) );

	cd->maxspeed		= ent->v.maxspeed;
	cd->fov				= ent->v.fov;
	cd->weaponanim		= ent->v.weaponanim;

	cd->pushmsec		= ent->v.pushmsec;

#if defined( CLIENT_WEAPONS )
	if ( sendweapons )
	{
		entvars_t *pev = (entvars_t *)&ent->v;
		CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

		if ( pl )
		{
			cd->m_flNextAttack	= pl->m_flNextAttack;
			cd->fuser2			= pl->m_flNextAmmoBurn;
			cd->fuser3			= pl->m_flAmmoStartCharge;
			cd->vuser1.x		= pl->ammo_9mm;
			cd->vuser1.y		= pl->ammo_357;
			cd->vuser1.z		= pl->ammo_argrens;
			cd->ammo_nails		= pl->ammo_bolts;
			cd->ammo_shells		= pl->ammo_buckshot;
			cd->ammo_rockets	= pl->ammo_rockets;
			cd->ammo_cells		= pl->ammo_uranium;
			cd->vuser2.x		= pl->ammo_hornets;
			

			if ( pl->m_pActiveItem )
			{
				CBasePlayerWeapon *gun;
				gun = (CBasePlayerWeapon *)pl->m_pActiveItem->GetWeaponPtr();
				if ( gun && gun->UseDecrement() )
				{
					ItemInfo II;
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );

					cd->m_iId = II.iId;

					cd->vuser3.z	= gun->m_iSecondaryAmmoType;
					cd->vuser4.x	= gun->m_iPrimaryAmmoType;
					cd->vuser4.y	= pl->m_rgAmmo[gun->m_iPrimaryAmmoType];
					cd->vuser4.z	= pl->m_rgAmmo[gun->m_iSecondaryAmmoType];
					
					if ( pl->m_pActiveItem->m_iId == WEAPON_HL_RPG )
					{
						cd->vuser2.y = ( ( CRpg * )pl->m_pActiveItem)->m_fSpotActive;
						cd->vuser2.z = ( ( CRpg * )pl->m_pActiveItem)->m_cActiveRockets;
					}
				}
			}
		}
	}
#endif
}

/*
=================
CmdStart

We're about to run this usercmd for the specified player.  We can set up groupinfo and masking here, etc.
This is the time to examine the usercmd for anything extra.  This call happens even if think does not.
=================
*/
void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

	if( !pl )
		return;

	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_SetGroupTrace( pl->pev->groupinfo, GROUP_OP_AND );
	}

	pl->random_seed = random_seed;
}

/*
=================
CmdEnd

Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
=================
*/
void CmdEnd ( const edict_t *player )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );

	if( !pl )
		return;
	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_UnsetGroupTrace();
	}
}

/*
================================
ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = VEC_HULL_MIN;
		maxs = VEC_HULL_MAX;
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = VEC_DUCK_HULL_MIN;
		maxs = VEC_DUCK_HULL_MAX;
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
CreateInstancedBaselines

Create pseudo-baselines for items that aren't placed in the map at spawn time, but which are likely
to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
================================
*/
void CreateInstancedBaselines ( void )
{
	int iret = 0;
	entity_state_t state;

	memset( &state, 0, sizeof( state ) );

	// Create any additional baselines here for things like grendates, etc.
	// iret = ENGINE_INSTANCE_BASELINE( pc->pev->classname, &state );

	// Destroy objects.
	//UTIL_Remove( pc );
}

/*
================================
InconsistentFile

One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
 Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
================================
*/
int	InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message )
{
	// Server doesn't care?
	if ( CVAR_GET_FLOAT( "mp_consistency" ) != 1 )
		return 0;

	// Default behavior is to kick the player
	sprintf( disconnect_message, "Server is enforcing file consistency for %s\n", filename );

	// Kick now with specified disconnect message.
	return 1;
}

/*
================================
AllowLagCompensation

 The game .dll should return 1 if lag compensation should be allowed ( could also just set
  the sv_unlag cvar.
 Most games right now should return 0, until client-side weapon prediction code is written
  and tested for them ( note you can predict weapons, but not do lag compensation, too, 
  if you want.
================================
*/
int AllowLagCompensation( void )
{
	return 1;
}

