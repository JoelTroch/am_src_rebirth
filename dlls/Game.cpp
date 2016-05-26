/***
*
*		   같
*			 �    같같같�
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
  
+new cvars
+load cfg files when engine starts

***/


#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "game.h"


cvar_t	mp_am_lenguage		= {"mp_am_lenguage","0", FCVAR_SERVER | FCVAR_ARCHIVE }; //off defecto

cvar_t	mp_am_mapmusic			= {"mp_am_mapmusic","default.mp3", FCVAR_SERVER | FCVAR_ARCHIVE }; //off defecto

cvar_t	mp_am_brief			= {"mp_am_brief","maps/default.txt", FCVAR_SERVER | FCVAR_ARCHIVE }; //off defecto
cvar_t	mp_am_brief_name		= {"mp_am_brief_name","Informes", FCVAR_SERVER | FCVAR_ARCHIVE }; //off defecto

cvar_t	mp_am_warning_sound		= {"mp_am_warning_sound","1", FCVAR_SERVER | FCVAR_ARCHIVE };

cvar_t oldmodels = { "mp_am_oldmodels", "0", FCVAR_SERVER | FCVAR_ARCHIVE };

cvar_t	value_x		= {"value_x","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	value_y		= {"value_y","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	value_z		= {"value_z","1", FCVAR_SERVER | FCVAR_ARCHIVE };

cvar_t devlight = { "dev_light", "0" };
cvar_t detalle = { "detail_game", "0" };

cvar_t	displaysoundlist = {"displaysoundlist","0"};
cvar_t	mp3player = { "mp3player", "1", FCVAR_SERVER };
cvar_t	mp3volume = { "mp3volume", "1", FCVAR_SERVER };

cvar_t	selection		= {"selection","0", FCVAR_SERVER };
cvar_t	totaldialogpoints		= {"totaldialogpoints","0", FCVAR_SERVER };

// multiplayer server rules
cvar_t	fragsleft	= {"mp_fragsleft","0", FCVAR_SERVER | FCVAR_UNLOGGED };	  // Don't spam console/log files/users with this changing
cvar_t	timeleft	= {"mp_timeleft","0" , FCVAR_SERVER | FCVAR_UNLOGGED };	  // "      "

// multiplayer server rules
cvar_t	teamplay	= {"mp_teamplay","0", FCVAR_SERVER };
cvar_t	fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER };
cvar_t	timelimit	= {"mp_timelimit","0", FCVAR_SERVER };
cvar_t	friendlyfire= {"mp_friendlyfire","0", FCVAR_SERVER };
cvar_t	falldamage	= {"mp_falldamage","0", FCVAR_SERVER };
cvar_t	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER };
cvar_t	forcerespawn= {"mp_forcerespawn","1", FCVAR_SERVER };
cvar_t	flashlight	= {"mp_flashlight","0", FCVAR_SERVER };
cvar_t	aimcrosshair= {"mp_autocrosshair","1", FCVAR_SERVER };
cvar_t	decalfrequency = {"decalfrequency","30", FCVAR_SERVER };
cvar_t	teamlist = {"mp_teamlist","hgrunt;scientist", FCVAR_SERVER };
cvar_t	teamoverride = {"mp_teamoverride","1" };
cvar_t	defaultteam = {"mp_defaultteam","0" };
cvar_t	allowmonsters={"mp_allowmonsters","0", FCVAR_SERVER };

cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER };
//FCVAR_SERVER
//EDIT: quizas me arrepienta de esto:
cvar_t	mp_am_noblastgibs	= {"mp_am_noblastgibs","1", FCVAR_SERVER | FCVAR_ARCHIVE }; //es off
cvar_t	mp_am_gore			= {"mp_am_gore","1", FCVAR_SERVER | FCVAR_ARCHIVE }; //off defecto

//cvar_t	mp_am_flyattack		= {"mp_am_flyattack","1", FCVAR_SERVER | FCVAR_ARCHIVE };
//cvar_t	mp_am_movecorpses	= {"mp_am_movecorpses","1", FCVAR_SERVER | FCVAR_ARCHIVE };



cvar_t	mp_am_truquitos		= {"mp_am_truquitos","0", FCVAR_SERVER | FCVAR_ARCHIVE }; //off defecto

//por que, por que! | Cheat????
cvar_t	mp_am_weaponweight	= {"mp_am_weaponweight","1", FCVAR_SERVER /*| FCVAR_ARCHIVE*/ };//dont save this
cvar_t	mp_am_default_weapons	= {"mp_am_default_weapons","1", FCVAR_SERVER /*| FCVAR_ARCHIVE*/ };//dont save this

//PHYSICS
cvar_t	phys_enable 			= {"phys_enable","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_objects			= {"phys_objects","1", FCVAR_SERVER | FCVAR_ARCHIVE };

cvar_t	phys_velocity			= {"phys_velocity","0.9", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_avelocity			= {"phys_avelocity","0.9", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_gravity			= {"phys_gravity","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_friction			= {"phys_friction","0.8", FCVAR_SERVER | FCVAR_ARCHIVE };

cvar_t	phys_flyattack			= {"phys_flyattack","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_movecorpses		= {"phys_movecorpses","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_movehit			= {"phys_movehit","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_normalize_angles	= {"phys_normalize_angles","1", FCVAR_SERVER | FCVAR_ARCHIVE };
cvar_t	phys_simulateragdoll		= {"phys_simulateragdoll","1", FCVAR_SERVER | FCVAR_ARCHIVE };

//cvar_t	xp_points			= {"xp_points","0", FCVAR_ARCHIVE };

// Engine Cvars
cvar_t 	*g_psv_gravity = NULL;
cvar_t	*g_psv_aim = NULL;
cvar_t	*g_footsteps = NULL;

//CVARS FOR SKILL LEVEL SETTINGS
// Agrunt
cvar_t	sk_agrunt_health1 = {"sk_agrunt_health1","0"};
cvar_t	sk_agrunt_health2 = {"sk_agrunt_health2","0"};
cvar_t	sk_agrunt_health3 = {"sk_agrunt_health3","0"};

cvar_t	sk_agrunt_dmg_punch1 = {"sk_agrunt_dmg_punch1","0"};
cvar_t	sk_agrunt_dmg_punch2 = {"sk_agrunt_dmg_punch2","0"};
cvar_t	sk_agrunt_dmg_punch3 = {"sk_agrunt_dmg_punch3","0"};

// Apache
cvar_t	sk_apache_health1	= {"sk_apache_health1","0"};
cvar_t	sk_apache_health2	= {"sk_apache_health2","0"};
cvar_t	sk_apache_health3	= {"sk_apache_health3","0"};

// Barney
cvar_t	sk_barney_health1	= {"sk_barney_health1","0"};
cvar_t	sk_barney_health2	= {"sk_barney_health2","0"};
cvar_t	sk_barney_health3	= {"sk_barney_health3","0"};

// Bullsquid
cvar_t	sk_bullsquid_health1 = {"sk_bullsquid_health1","0"};
cvar_t	sk_bullsquid_health2 = {"sk_bullsquid_health2","0"};
cvar_t	sk_bullsquid_health3 = {"sk_bullsquid_health3","0"};

cvar_t	sk_bullsquid_dmg_bite1 = {"sk_bullsquid_dmg_bite1","0"};
cvar_t	sk_bullsquid_dmg_bite2 = {"sk_bullsquid_dmg_bite2","0"};
cvar_t	sk_bullsquid_dmg_bite3 = {"sk_bullsquid_dmg_bite3","0"};

cvar_t	sk_bullsquid_dmg_whip1 = {"sk_bullsquid_dmg_whip1","0"};
cvar_t	sk_bullsquid_dmg_whip2 = {"sk_bullsquid_dmg_whip2","0"};
cvar_t	sk_bullsquid_dmg_whip3 = {"sk_bullsquid_dmg_whip3","0"};

cvar_t	sk_bullsquid_dmg_spit1 = {"sk_bullsquid_dmg_spit1","0"};
cvar_t	sk_bullsquid_dmg_spit2 = {"sk_bullsquid_dmg_spit2","0"};
cvar_t	sk_bullsquid_dmg_spit3 = {"sk_bullsquid_dmg_spit3","0"};


// Big Momma
cvar_t	sk_bigmomma_health_factor1 = {"sk_bigmomma_health_factor1","1.0"};
cvar_t	sk_bigmomma_health_factor2 = {"sk_bigmomma_health_factor2","1.0"};
cvar_t	sk_bigmomma_health_factor3 = {"sk_bigmomma_health_factor3","1.0"};

cvar_t	sk_bigmomma_dmg_slash1 = {"sk_bigmomma_dmg_slash1","50"};
cvar_t	sk_bigmomma_dmg_slash2 = {"sk_bigmomma_dmg_slash2","50"};
cvar_t	sk_bigmomma_dmg_slash3 = {"sk_bigmomma_dmg_slash3","50"};

cvar_t	sk_bigmomma_dmg_blast1 = {"sk_bigmomma_dmg_blast1","100"};
cvar_t	sk_bigmomma_dmg_blast2 = {"sk_bigmomma_dmg_blast2","100"};
cvar_t	sk_bigmomma_dmg_blast3 = {"sk_bigmomma_dmg_blast3","100"};

cvar_t	sk_bigmomma_radius_blast1 = {"sk_bigmomma_radius_blast1","250"};
cvar_t	sk_bigmomma_radius_blast2 = {"sk_bigmomma_radius_blast2","250"};
cvar_t	sk_bigmomma_radius_blast3 = {"sk_bigmomma_radius_blast3","250"};

// Gargantua
cvar_t	sk_gargantua_health1 = {"sk_gargantua_health1","0"};
cvar_t	sk_gargantua_health2 = {"sk_gargantua_health2","0"};
cvar_t	sk_gargantua_health3 = {"sk_gargantua_health3","0"};

cvar_t	sk_gargantua_dmg_slash1	= {"sk_gargantua_dmg_slash1","0"};
cvar_t	sk_gargantua_dmg_slash2	= {"sk_gargantua_dmg_slash2","0"};
cvar_t	sk_gargantua_dmg_slash3	= {"sk_gargantua_dmg_slash3","0"};

cvar_t	sk_gargantua_dmg_fire1 = {"sk_gargantua_dmg_fire1","0"};
cvar_t	sk_gargantua_dmg_fire2 = {"sk_gargantua_dmg_fire2","0"};
cvar_t	sk_gargantua_dmg_fire3 = {"sk_gargantua_dmg_fire3","0"};

cvar_t	sk_gargantua_dmg_stomp1	= {"sk_gargantua_dmg_stomp1","0"};
cvar_t	sk_gargantua_dmg_stomp2	= {"sk_gargantua_dmg_stomp2","0"};
cvar_t	sk_gargantua_dmg_stomp3	= {"sk_gargantua_dmg_stomp3","0"};


// Hassassin
cvar_t	sk_hassassin_health1 = {"sk_hassassin_health1","0"};
cvar_t	sk_hassassin_health2 = {"sk_hassassin_health2","0"};
cvar_t	sk_hassassin_health3 = {"sk_hassassin_health3","0"};

//Zombie Poison
cvar_t	sk_zombie_poison_health1 = {"sk_zombie_poison_health1","0"};
cvar_t	sk_zombie_poison_health2 = {"sk_zombie_poison_health2","0"};
cvar_t	sk_zombie_poison_health3 = {"sk_zombie_poison_health3","0"};

cvar_t	sk_zombie_poison_melee1 = {"sk_zombie_poison_melee1","0"};
cvar_t	sk_zombie_poison_melee2 = {"sk_zombie_poison_melee2","0"};
cvar_t	sk_zombie_poison_melee3 = {"sk_zombie_poison_melee3","0"};

// Headcrab
cvar_t	sk_headcrab_health1 = {"sk_headcrab_health1","0"};
cvar_t	sk_headcrab_health2 = {"sk_headcrab_health2","0"};
cvar_t	sk_headcrab_health3 = {"sk_headcrab_health3","0"};

cvar_t	sk_headcrab_dmg_bite1 = {"sk_headcrab_dmg_bite1","0"};
cvar_t	sk_headcrab_dmg_bite2 = {"sk_headcrab_dmg_bite2","0"};
cvar_t	sk_headcrab_dmg_bite3 = {"sk_headcrab_dmg_bite3","0"};

// Hgrunt_ak 
/*
cvar_t	sk_hgrunt_ak_health1 = {"sk_hgrunt_ak_health1","0"};
cvar_t	sk_hgrunt_ak_health2 = {"sk_hgrunt_ak_health2","0"};
cvar_t	sk_hgrunt_ak_health3 = {"sk_hgrunt_ak_health3","0"};

cvar_t	sk_hgrunt_ak_kick1 = {"sk_hgrunt_ak_kick1","0"};
cvar_t	sk_hgrunt_ak_kick2 = {"sk_hgrunt_ak_kick2","0"};
cvar_t	sk_hgrunt_ak_kick3 = {"sk_hgrunt_ak_kick3","0"};

cvar_t	sk_hgrunt_ak_pellets1 = {"sk_hgrunt_ak_pellets1","0"};
cvar_t	sk_hgrunt_ak_pellets2 = {"sk_hgrunt_ak_pellets2","0"};
cvar_t	sk_hgrunt_ak_pellets3 = {"sk_hgrunt_ak_pellets3","0"};

cvar_t	sk_hgrunt_ak_gspeed1 = {"sk_hgrunt_ak_gspeed1","0"};
cvar_t	sk_hgrunt_ak_gspeed2 = {"sk_hgrunt_ak_gspeed2","0"};
cvar_t	sk_hgrunt_ak_gspeed3 = {"sk_hgrunt_ak_gspeed3","0"};
*/
// Hgrunt 
cvar_t	sk_hgrunt_health1 = {"sk_hgrunt_health1","0"};
cvar_t	sk_hgrunt_health2 = {"sk_hgrunt_health2","0"};
cvar_t	sk_hgrunt_health3 = {"sk_hgrunt_health3","0"};

cvar_t	sk_hgrunt_kick1 = {"sk_hgrunt_kick1","0"};
cvar_t	sk_hgrunt_kick2 = {"sk_hgrunt_kick2","0"};
cvar_t	sk_hgrunt_kick3 = {"sk_hgrunt_kick3","0"};

cvar_t	sk_hgrunt_pellets1 = {"sk_hgrunt_pellets1","0"};
cvar_t	sk_hgrunt_pellets2 = {"sk_hgrunt_pellets2","0"};
cvar_t	sk_hgrunt_pellets3 = {"sk_hgrunt_pellets3","0"};

cvar_t	sk_hgrunt_gspeed1 = {"sk_hgrunt_gspeed1","0"};
cvar_t	sk_hgrunt_gspeed2 = {"sk_hgrunt_gspeed2","0"};
cvar_t	sk_hgrunt_gspeed3 = {"sk_hgrunt_gspeed3","0"};
/*
// Hgrunt_de
cvar_t	sk_hgrunt_de_health1 = {"sk_hgrunt_de_health1","0"};
cvar_t	sk_hgrunt_de_health2 = {"sk_hgrunt_de_health2","0"};
cvar_t	sk_hgrunt_de_health3 = {"sk_hgrunt_de_health3","0"};

cvar_t	sk_hgrunt_de_kick1 = {"sk_hgrunt_de_kick1","0"};
cvar_t	sk_hgrunt_de_kick2 = {"sk_hgrunt_de_kick2","0"};
cvar_t	sk_hgrunt_de_kick3 = {"sk_hgrunt_de_kick3","0"};

cvar_t	sk_hgrunt_de_pellets1 = {"sk_hgrunt_de_pellets1","0"};
cvar_t	sk_hgrunt_de_pellets2 = {"sk_hgrunt_de_pellets2","0"};
cvar_t	sk_hgrunt_de_pellets3 = {"sk_hgrunt_de_pellets3","0"};

cvar_t	sk_hgrunt_de_gspeed1 = {"sk_hgrunt_de_gspeed1","0"};
cvar_t	sk_hgrunt_de_gspeed2 = {"sk_hgrunt_de_gspeed2","0"};
cvar_t	sk_hgrunt_de_gspeed3 = {"sk_hgrunt_de_gspeed3","0"};

// Hgrunt_cs
cvar_t	sk_hgrunt_cs_health1 = {"sk_hgrunt_cs_health1","0"};
cvar_t	sk_hgrunt_cs_health2 = {"sk_hgrunt_cs_health2","0"};
cvar_t	sk_hgrunt_cs_health3 = {"sk_hgrunt_cs_health3","0"};

cvar_t	sk_hgrunt_cs_kick1 = {"sk_hgrunt_cs_kick1","0"};
cvar_t	sk_hgrunt_cs_kick2 = {"sk_hgrunt_cs_kick2","0"};
cvar_t	sk_hgrunt_cs_kick3 = {"sk_hgrunt_cs_kick3","0"};

cvar_t	sk_hgrunt_cs_pellets1 = {"sk_hgrunt_cs_pellets1","0"};
cvar_t	sk_hgrunt_cs_pellets2 = {"sk_hgrunt_cs_pellets2","0"};
cvar_t	sk_hgrunt_cs_pellets3 = {"sk_hgrunt_cs_pellets3","0"};

cvar_t	sk_hgrunt_cs_gspeed1 = {"sk_hgrunt_cs_gspeed1","0"};
cvar_t	sk_hgrunt_cs_gspeed2 = {"sk_hgrunt_cs_gspeed2","0"};
cvar_t	sk_hgrunt_cs_gspeed3 = {"sk_hgrunt_cs_gspeed3","0"};
*/
// OTIS
cvar_t	sk_otis_health1	= {"sk_otis_health1","0"};
cvar_t	sk_otis_health2	= {"sk_otis_health2","0"};
cvar_t	sk_otis_health3	= {"sk_otis_health3","0"};

// Houndeye
cvar_t	sk_houndeye_health1 = {"sk_houndeye_health1","0"};
cvar_t	sk_houndeye_health2 = {"sk_houndeye_health2","0"};
cvar_t	sk_houndeye_health3 = {"sk_houndeye_health3","0"};

cvar_t	sk_houndeye_dmg_blast1 = {"sk_houndeye_dmg_blast1","0"};
cvar_t	sk_houndeye_dmg_blast2 = {"sk_houndeye_dmg_blast2","0"};
cvar_t	sk_houndeye_dmg_blast3 = {"sk_houndeye_dmg_blast3","0"};


// ISlave
cvar_t	sk_islave_health1 = {"sk_islave_health1","0"};
cvar_t	sk_islave_health2 = {"sk_islave_health2","0"};
cvar_t	sk_islave_health3 = {"sk_islave_health3","0"};

cvar_t	sk_islave_dmg_claw1 = {"sk_islave_dmg_claw1","0"};
cvar_t	sk_islave_dmg_claw2 = {"sk_islave_dmg_claw2","0"};
cvar_t	sk_islave_dmg_claw3 = {"sk_islave_dmg_claw3","0"};

cvar_t	sk_islave_dmg_clawrake1	= {"sk_islave_dmg_clawrake1","0"};
cvar_t	sk_islave_dmg_clawrake2	= {"sk_islave_dmg_clawrake2","0"};
cvar_t	sk_islave_dmg_clawrake3	= {"sk_islave_dmg_clawrake3","0"};
	
cvar_t	sk_islave_dmg_zap1 = {"sk_islave_dmg_zap1","0"};
cvar_t	sk_islave_dmg_zap2 = {"sk_islave_dmg_zap2","0"};
cvar_t	sk_islave_dmg_zap3 = {"sk_islave_dmg_zap3","0"};


// Icthyosaur
cvar_t	sk_ichthyosaur_health1	= {"sk_ichthyosaur_health1","0"};
cvar_t	sk_ichthyosaur_health2	= {"sk_ichthyosaur_health2","0"};
cvar_t	sk_ichthyosaur_health3	= {"sk_ichthyosaur_health3","0"};

cvar_t	sk_ichthyosaur_shake1	= {"sk_ichthyosaur_shake1","0"};
cvar_t	sk_ichthyosaur_shake2	= {"sk_ichthyosaur_shake2","0"};
cvar_t	sk_ichthyosaur_shake3	= {"sk_ichthyosaur_shake3","0"};


// Leech
cvar_t	sk_leech_health1 = {"sk_leech_health1","0"};
cvar_t	sk_leech_health2 = {"sk_leech_health2","0"};
cvar_t	sk_leech_health3 = {"sk_leech_health3","0"};

cvar_t	sk_leech_dmg_bite1 = {"sk_leech_dmg_bite1","0"};
cvar_t	sk_leech_dmg_bite2 = {"sk_leech_dmg_bite2","0"};
cvar_t	sk_leech_dmg_bite3 = {"sk_leech_dmg_bite3","0"};

// Controller
cvar_t	sk_controller_health1 = {"sk_controller_health1","0"};
cvar_t	sk_controller_health2 = {"sk_controller_health2","0"};
cvar_t	sk_controller_health3 = {"sk_controller_health3","0"};

cvar_t	sk_controller_dmgzap1 = {"sk_controller_dmgzap1","0"};
cvar_t	sk_controller_dmgzap2 = {"sk_controller_dmgzap2","0"};
cvar_t	sk_controller_dmgzap3 = {"sk_controller_dmgzap3","0"};

cvar_t	sk_controller_speedball1 = {"sk_controller_speedball1","0"};
cvar_t	sk_controller_speedball2 = {"sk_controller_speedball2","0"};
cvar_t	sk_controller_speedball3 = {"sk_controller_speedball3","0"};

cvar_t	sk_controller_dmgball1 = {"sk_controller_dmgball1","0"};
cvar_t	sk_controller_dmgball2 = {"sk_controller_dmgball2","0"};
cvar_t	sk_controller_dmgball3 = {"sk_controller_dmgball3","0"};

// Nihilanth
cvar_t	sk_nihilanth_health1 = {"sk_nihilanth_health1","0"};
cvar_t	sk_nihilanth_health2 = {"sk_nihilanth_health2","0"};
cvar_t	sk_nihilanth_health3 = {"sk_nihilanth_health3","0"};

cvar_t	sk_nihilanth_zap1 = {"sk_nihilanth_zap1","0"};
cvar_t	sk_nihilanth_zap2 = {"sk_nihilanth_zap2","0"};
cvar_t	sk_nihilanth_zap3 = {"sk_nihilanth_zap3","0"};

// Scientist
cvar_t	sk_scientist_health1 = {"sk_scientist_health1","0"};
cvar_t	sk_scientist_health2 = {"sk_scientist_health2","0"};
cvar_t	sk_scientist_health3 = {"sk_scientist_health3","0"};


// Snark
cvar_t	sk_snark_health1 = {"sk_snark_health1","0"};
cvar_t	sk_snark_health2 = {"sk_snark_health2","0"};
cvar_t	sk_snark_health3 = {"sk_snark_health3","0"};

cvar_t	sk_snark_dmg_bite1 = {"sk_snark_dmg_bite1","0"};
cvar_t	sk_snark_dmg_bite2 = {"sk_snark_dmg_bite2","0"};
cvar_t	sk_snark_dmg_bite3 = {"sk_snark_dmg_bite3","0"};

cvar_t	sk_snark_dmg_pop1 = {"sk_snark_dmg_pop1","0"};
cvar_t	sk_snark_dmg_pop2 = {"sk_snark_dmg_pop2","0"};
cvar_t	sk_snark_dmg_pop3 = {"sk_snark_dmg_pop3","0"};



// Zombie
cvar_t	sk_zombie_health1 = {"sk_zombie_health1","0"};
cvar_t	sk_zombie_health2 = {"sk_zombie_health2","0"};
cvar_t	sk_zombie_health3 = {"sk_zombie_health3","0"};

cvar_t	sk_zombie_dmg_one_slash1 = {"sk_zombie_dmg_one_slash1","0"};
cvar_t	sk_zombie_dmg_one_slash2 = {"sk_zombie_dmg_one_slash2","0"};
cvar_t	sk_zombie_dmg_one_slash3 = {"sk_zombie_dmg_one_slash3","0"};

cvar_t	sk_zombie_dmg_both_slash1 = {"sk_zombie_dmg_both_slash1","0"};
cvar_t	sk_zombie_dmg_both_slash2 = {"sk_zombie_dmg_both_slash2","0"};
cvar_t	sk_zombie_dmg_both_slash3 = {"sk_zombie_dmg_both_slash3","0"};

cvar_t	sk_scientist_crowbar_hit1 = {"sk_scientist_crowbar_hit1","0"};
cvar_t	sk_scientist_crowbar_hit2 = {"sk_scientist_crowbar_hit2","0"};
cvar_t	sk_scientist_crowbar_hit3 = {"sk_scientist_crowbar_hit3","0"};

// gonome
cvar_t	sk_gonome_health1 = {"sk_gonome_health1","0"};
cvar_t	sk_gonome_health2 = {"sk_gonome_health2","0"};
cvar_t	sk_gonome_health3 = {"sk_gonome_health3","0"};

cvar_t	sk_gonome_dmg_one_slash1 = {"sk_gonome_dmg_one_slash1","0"};
cvar_t	sk_gonome_dmg_one_slash2 = {"sk_gonome_dmg_one_slash2","0"};
cvar_t	sk_gonome_dmg_one_slash3 = {"sk_gonome_dmg_one_slash3","0"};

cvar_t	sk_gonome_dmg_both_slash1 = {"sk_gonome_dmg_both_slash1","0"};
cvar_t	sk_gonome_dmg_both_slash2 = {"sk_gonome_dmg_both_slash2","0"};
cvar_t	sk_gonome_dmg_both_slash3 = {"sk_gonome_dmg_both_slash3","0"};

//pitdrone
cvar_t sk_pit_drone_health1 = {"sk_pit_drone_health1","0"};
cvar_t sk_pit_drone_health2 = {"sk_pit_drone_health2","0"};
cvar_t sk_pit_drone_health3 = {"sk_pit_drone_health3","0"};

// strooper
cvar_t	sk_strooper_health1 = {"sk_strooper_health1","0"};
cvar_t	sk_strooper_health2 = {"sk_strooper_health2","0"};
cvar_t	sk_strooper_health3 = {"sk_strooper_health3","0"};

// robot
cvar_t	sk_robot_health1 = {"sk_robot_health1","0"};
cvar_t	sk_robot_health2 = {"sk_robot_health2","0"};
cvar_t	sk_robot_health3 = {"sk_robot_health3","0"};

//Turret
cvar_t	sk_turret_health1 = {"sk_turret_health1","0"};
cvar_t	sk_turret_health2 = {"sk_turret_health2","0"};
cvar_t	sk_turret_health3 = {"sk_turret_health3","0"};


// MiniTurret
cvar_t	sk_miniturret_health1 = {"sk_miniturret_health1","0"};
cvar_t	sk_miniturret_health2 = {"sk_miniturret_health2","0"};
cvar_t	sk_miniturret_health3 = {"sk_miniturret_health3","0"};


// Sentry Turret
cvar_t	sk_sentry_health1 = {"sk_sentry_health1","0"};
cvar_t	sk_sentry_health2 = {"sk_sentry_health2","0"};
cvar_t	sk_sentry_health3 = {"sk_sentry_health3","0"};


// PLAYER WEAPONS
cvar_t	sk_plr_crowbar1 = {"sk_plr_crowbar1","0"};
cvar_t	sk_plr_crowbar2 = {"sk_plr_crowbar2","0"};
cvar_t	sk_plr_crowbar3 = {"sk_plr_crowbar3","0"};

// Crowbar whack
cvar_t	sk_plr_knife1 = {"sk_plr_knife1","0"};
cvar_t	sk_plr_knife2 = {"sk_plr_knife2","0"};
cvar_t	sk_plr_knife3 = {"sk_plr_knife3","0"};

// Crowbar whack

cvar_t	sk_plr_knife_stab1 = {"sk_plr_knife_stab1","0"};
cvar_t	sk_plr_knife_stab2 = {"sk_plr_knife_stab2","0"};
cvar_t	sk_plr_knife_stab3 = {"sk_plr_knife_stab3","0"};

// Glock Round
cvar_t	sk_plr_9mm_bullet1 = {"sk_plr_9mm_bullet1","0"};
cvar_t	sk_plr_9mm_bullet2 = {"sk_plr_9mm_bullet2","0"};
cvar_t	sk_plr_9mm_bullet3 = {"sk_plr_9mm_bullet3","0"};

// 357 Round
cvar_t	sk_plr_357_bullet1 = {"sk_plr_357_bullet1","0"};
cvar_t	sk_plr_357_bullet2 = {"sk_plr_357_bullet2","0"};
cvar_t	sk_plr_357_bullet3 = {"sk_plr_357_bullet3","0"};

// MP5 Round
cvar_t	sk_plr_9mmAR_bullet1 = {"sk_plr_9mmAR_bullet1","0"};
cvar_t	sk_plr_9mmAR_bullet2 = {"sk_plr_9mmAR_bullet2","0"};
cvar_t	sk_plr_9mmAR_bullet3 = {"sk_plr_9mmAR_bullet3","0"};

// M16 Round
cvar_t	sk_plr_M16_bullet1 = {"sk_plr_M16_bullet1","0"};
cvar_t	sk_plr_M16_bullet2 = {"sk_plr_M16_bullet2","0"};
cvar_t	sk_plr_M16_bullet3 = {"sk_plr_M16_bullet3","0"};

// AK74 ROUND

cvar_t	sk_plr_AK74_bullet1 = {"sk_plr_AK74_bullet1","0"};
cvar_t	sk_plr_AK74_bullet2 = {"sk_plr_AK74_bullet2","0"};
cvar_t	sk_plr_AK74_bullet3 = {"sk_plr_AK74_bullet3","0"};

// UZI ROUND

cvar_t	sk_plr_UZI_bullet1 = {"sk_plr_UZI_bullet1","0"};
cvar_t	sk_plr_UZI_bullet2 = {"sk_plr_UZI_bullet2","0"};
cvar_t	sk_plr_UZI_bullet3 = {"sk_plr_UZI_bullet3","0"};

//	m82 ROUN
cvar_t	sk_plr_m82_bullet1 = {"sk_plr_m82_bullet1","0"};
cvar_t	sk_plr_m82_bullet2 = {"sk_plr_m82_bullet2","0"};
cvar_t	sk_plr_m82_bullet3 = {"sk_plr_m82_bullet3","0"};

// c4bomb ROUND
	
cvar_t	sk_plr_c4bomb1 = {"sk_plr_c4bomb1","0"};
cvar_t	sk_plr_c4bomb2 = {"sk_plr_c4bomb2","0"};
cvar_t	sk_plr_c4bomb3 = {"sk_plr_c4bomb3","0"};

// p90 ROUND
cvar_t	sk_plr_p90_bullet1 = {"sk_plr_p90_bullet1","0"};
cvar_t	sk_plr_p90_bullet2 = {"sk_plr_p90_bullet2","0"};
cvar_t	sk_plr_p90_bullet3 = {"sk_plr_p90_bullet3","0"};

// 92F ROUND

cvar_t	sk_plr_ber92F_bullet1 = {"sk_plr_ber92F_bullet1","0"};
cvar_t	sk_plr_ber92F_bullet2 = {"sk_plr_ber92F_bullet2","0"};
cvar_t	sk_plr_ber92F_bullet3 = {"sk_plr_ber92F_bullet3","0"};

// glock18 ROUND
cvar_t	sk_plr_glock18_bullet1 = {"sk_plr_glock18_bullet1","0"};
cvar_t	sk_plr_glock18_bullet2 = {"sk_plr_glock18_bullet2","0"};
cvar_t	sk_plr_glock18_bullet3 = {"sk_plr_glock18_bullet3","0"};

// M249 ROUND

cvar_t	sk_plr_M249_bullet1 = {"sk_plr_M249_bullet1","0"};
cvar_t	sk_plr_M249_bullet2 = {"sk_plr_M249_bullet2","0"};
cvar_t	sk_plr_M249_bullet3 = {"sk_plr_M249_bullet3","0"};

// DEAGLE

cvar_t	sk_plr_deagle_bullet1 = {"sk_plr_deagle_bullet1","0"};
cvar_t	sk_plr_deagle_bullet2 = {"sk_plr_deagle_bullet2","0"};
cvar_t	sk_plr_deagle_bullet3 = {"sk_plr_deagle_bullet3","0"};

// SIG Sniper

cvar_t	sk_plr_sniper_bullet1 = {"sk_plr_sniper_bullet1","0"};
cvar_t	sk_plr_sniper_bullet2 = {"sk_plr_sniper_bullet2","0"};
cvar_t	sk_plr_sniper_bullet3 = {"sk_plr_sniper_bullet3","0"};

// SIG Sniper
cvar_t	sk_plr_scout_bullet1 = {"sk_plr_scout_bullet1","0"};
cvar_t	sk_plr_scout_bullet2 = {"sk_plr_scout_bullet2","0"};
cvar_t	sk_plr_scout_bullet3 = {"sk_plr_scout_bullet3","0"};

// usas ROUND
cvar_t	sk_plr_usas_buckshot1 = {"sk_plr_usas_buckshot1","0"};
cvar_t	sk_plr_usas_buckshot2 = {"sk_plr_usas_buckshot2","0"};
cvar_t	sk_plr_usas_buckshot3 = {"sk_plr_usas_buckshot3","0"};

// FAMAS ROUND

cvar_t	sk_plr_famas_bullet1 = {"sk_plr_famas_bullet1","0"};
cvar_t	sk_plr_famas_bullet2 = {"sk_plr_famas_bullet2","0"};
cvar_t	sk_plr_famas_bullet3 = {"sk_plr_famas_bullet3","0"};

// M203 grenade
cvar_t	sk_plr_9mmAR_grenade1 = {"sk_plr_9mmAR_grenade1","0"};
cvar_t	sk_plr_9mmAR_grenade2 = {"sk_plr_9mmAR_grenade2","0"};
cvar_t	sk_plr_9mmAR_grenade3 = {"sk_plr_9mmAR_grenade3","0"};


// Shotgun buckshot
cvar_t	sk_plr_buckshot1 = {"sk_plr_buckshot1","0"};
cvar_t	sk_plr_buckshot2 = {"sk_plr_buckshot2","0"};
cvar_t	sk_plr_buckshot3 = {"sk_plr_buckshot3","0"};


// Crossbow
cvar_t	sk_plr_xbow_bolt_client1 = {"sk_plr_xbow_bolt_client1","0"};
cvar_t	sk_plr_xbow_bolt_client2 = {"sk_plr_xbow_bolt_client2","0"};
cvar_t	sk_plr_xbow_bolt_client3 = {"sk_plr_xbow_bolt_client3","0"};

cvar_t	sk_plr_xbow_bolt_monster1 = {"sk_plr_xbow_bolt_monster1","0"};
cvar_t	sk_plr_xbow_bolt_monster2 = {"sk_plr_xbow_bolt_monster2","0"};
cvar_t	sk_plr_xbow_bolt_monster3 = {"sk_plr_xbow_bolt_monster3","0"};


// RPG
cvar_t	sk_plr_rpg1 = {"sk_plr_rpg1","0"};
cvar_t	sk_plr_rpg2 = {"sk_plr_rpg2","0"};
cvar_t	sk_plr_rpg3 = {"sk_plr_rpg3","0"};


// Zero Point Generator
cvar_t	sk_plr_gauss1 = {"sk_plr_gauss1","0"};
cvar_t	sk_plr_gauss2 = {"sk_plr_gauss2","0"};
cvar_t	sk_plr_gauss3 = {"sk_plr_gauss3","0"};


// Tau Cannon
cvar_t	sk_plr_egon_narrow1 = {"sk_plr_egon_narrow1","0"};
cvar_t	sk_plr_egon_narrow2 = {"sk_plr_egon_narrow2","0"};
cvar_t	sk_plr_egon_narrow3 = {"sk_plr_egon_narrow3","0"};

cvar_t	sk_plr_egon_wide1 = {"sk_plr_egon_wide1","0"};
cvar_t	sk_plr_egon_wide2 = {"sk_plr_egon_wide2","0"};
cvar_t	sk_plr_egon_wide3 = {"sk_plr_egon_wide3","0"};


// Hand Grendade
cvar_t	sk_plr_hand_grenade1 = {"sk_plr_hand_grenade1","0"};
cvar_t	sk_plr_hand_grenade2 = {"sk_plr_hand_grenade2","0"};
cvar_t	sk_plr_hand_grenade3 = {"sk_plr_hand_grenade3","0"};


// Satchel Charge
cvar_t	sk_plr_satchel1	= {"sk_plr_satchel1","0"};
cvar_t	sk_plr_satchel2	= {"sk_plr_satchel2","0"};
cvar_t	sk_plr_satchel3	= {"sk_plr_satchel3","0"};


// Tripmine
cvar_t	sk_plr_tripmine1 = {"sk_plr_tripmine1","0"};
cvar_t	sk_plr_tripmine2 = {"sk_plr_tripmine2","0"};
cvar_t	sk_plr_tripmine3 = {"sk_plr_tripmine3","0"};


// WORLD WEAPONS
cvar_t	sk_12mm_bullet1 = {"sk_12mm_bullet1","0"};
cvar_t	sk_12mm_bullet2 = {"sk_12mm_bullet2","0"};
cvar_t	sk_12mm_bullet3 = {"sk_12mm_bullet3","0"};

cvar_t	sk_9mmAR_bullet1 = {"sk_9mmAR_bullet1","0"};
cvar_t	sk_9mmAR_bullet2 = {"sk_9mmAR_bullet2","0"};
cvar_t	sk_9mmAR_bullet3 = {"sk_9mmAR_bullet3","0"};

cvar_t	sk_9mm_bullet1 = {"sk_9mm_bullet1","0"};
cvar_t	sk_9mm_bullet2 = {"sk_9mm_bullet2","0"};
cvar_t	sk_9mm_bullet3 = {"sk_9mm_bullet3","0"};

//NEW BY ME
cvar_t	sk_ak_bullet1 = {"sk_ak_bullet1","0"};
cvar_t	sk_ak_bullet2 = {"sk_ak_bullet2","0"};
cvar_t	sk_ak_bullet3 = {"sk_ak_bullet3","0"};

cvar_t	sk_556_bullet1 = {"sk_556_bullet1","0"};
cvar_t	sk_556_bullet2 = {"sk_556_bullet2","0"};
cvar_t	sk_556_bullet3 = {"sk_556_bullet3","0"};

cvar_t	sk_deagle_bullet1 = {"sk_deagle_bullet1","0"};
cvar_t	sk_deagle_bullet2 = {"sk_deagle_bullet2","0"};
cvar_t	sk_deagle_bullet3 = {"sk_deagle_bullet3","0"};

cvar_t	sk_awp_bullet1 = {"sk_awp_bullet1","0"};
cvar_t	sk_awp_bullet2 = {"sk_awp_bullet2","0"};
cvar_t	sk_awp_bullet3 = {"sk_awp_bullet3","0"};

cvar_t sk_plr_irgun_bullet1 = {"sk_plr_irgun_bullet1","0"};
cvar_t sk_plr_irgun_bullet2 = {"sk_plr_irgun_bullet2","0"};
cvar_t sk_plr_irgun_bullet3 = {"sk_plr_irgun_bullet3","0"};

// HORNET
cvar_t	sk_hornet_dmg1 = {"sk_hornet_dmg1","0"};
cvar_t	sk_hornet_dmg2 = {"sk_hornet_dmg2","0"};
cvar_t	sk_hornet_dmg3 = {"sk_hornet_dmg3","0"};

// HEALTH/CHARGE
cvar_t	sk_suitcharger1	= { "sk_suitcharger1","0" };
cvar_t	sk_suitcharger2	= { "sk_suitcharger2","0" };		
cvar_t	sk_suitcharger3	= { "sk_suitcharger3","0" };		

cvar_t	sk_battery1	= { "sk_battery1","0" };			
cvar_t	sk_battery2	= { "sk_battery2","0" };			
cvar_t	sk_battery3	= { "sk_battery3","0" };			

cvar_t	sk_healthcharger1	= { "sk_healthcharger1","0" };		
cvar_t	sk_healthcharger2	= { "sk_healthcharger2","0" };		
cvar_t	sk_healthcharger3	= { "sk_healthcharger3","0" };		

cvar_t	sk_healthkit1	= { "sk_healthkit1","0" };		
cvar_t	sk_healthkit2	= { "sk_healthkit2","0" };		
cvar_t	sk_healthkit3	= { "sk_healthkit3","0" };		

cvar_t	sk_scientist_heal1	= { "sk_scientist_heal1","0" };	
cvar_t	sk_scientist_heal2	= { "sk_scientist_heal2","0" };	
cvar_t	sk_scientist_heal3	= { "sk_scientist_heal3","0" };	


// monster damage adjusters
cvar_t	sk_monster_head1	= { "sk_monster_head1","2" };
cvar_t	sk_monster_head2	= { "sk_monster_head2","2" };
cvar_t	sk_monster_head3	= { "sk_monster_head3","2" };

cvar_t	sk_monster_chest1	= { "sk_monster_chest1","1" };
cvar_t	sk_monster_chest2	= { "sk_monster_chest2","1" };
cvar_t	sk_monster_chest3	= { "sk_monster_chest3","1" };

cvar_t	sk_monster_stomach1	= { "sk_monster_stomach1","1" };
cvar_t	sk_monster_stomach2	= { "sk_monster_stomach2","1" };
cvar_t	sk_monster_stomach3	= { "sk_monster_stomach3","1" };

cvar_t	sk_monster_arm1	= { "sk_monster_arm1","1" };
cvar_t	sk_monster_arm2	= { "sk_monster_arm2","1" };
cvar_t	sk_monster_arm3	= { "sk_monster_arm3","1" };

cvar_t	sk_monster_leg1	= { "sk_monster_leg1","1" };
cvar_t	sk_monster_leg2	= { "sk_monster_leg2","1" };
cvar_t	sk_monster_leg3	= { "sk_monster_leg3","1" };

// player damage adjusters
cvar_t	sk_player_head1	= { "sk_player_head1","2" };
cvar_t	sk_player_head2	= { "sk_player_head2","2" };
cvar_t	sk_player_head3	= { "sk_player_head3","2" };

cvar_t	sk_player_chest1	= { "sk_player_chest1","1" };
cvar_t	sk_player_chest2	= { "sk_player_chest2","1" };
cvar_t	sk_player_chest3	= { "sk_player_chest3","1" };

cvar_t	sk_player_stomach1	= { "sk_player_stomach1","1" };
cvar_t	sk_player_stomach2	= { "sk_player_stomach2","1" };
cvar_t	sk_player_stomach3	= { "sk_player_stomach3","1" };

cvar_t	sk_player_arm1	= { "sk_player_arm1","1" };
cvar_t	sk_player_arm2	= { "sk_player_arm2","1" };
cvar_t	sk_player_arm3	= { "sk_player_arm3","1" };

cvar_t	sk_player_leg1	= { "sk_player_leg1","1" };
cvar_t	sk_player_leg2	= { "sk_player_leg2","1" };
cvar_t	sk_player_leg3	= { "sk_player_leg3","1" };

// END Cvars for Skill Level settings

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
	// Register cvars here:	
	CVAR_REGISTER (&oldmodels);

	CVAR_REGISTER (&value_x);
	CVAR_REGISTER (&value_y);
	CVAR_REGISTER (&value_z);

	CVAR_REGISTER (&mp_am_mapmusic);//test

	CVAR_REGISTER (&phys_enable);//test	
	CVAR_REGISTER (&phys_objects);//test

	CVAR_REGISTER (&phys_velocity);//test
	CVAR_REGISTER (&phys_avelocity);//test
	CVAR_REGISTER (&phys_gravity);//test
	CVAR_REGISTER (&phys_friction);//test

	CVAR_REGISTER (&phys_flyattack);//test
	CVAR_REGISTER (&phys_movecorpses);//test
	CVAR_REGISTER (&phys_movehit);//test
	CVAR_REGISTER (&phys_normalize_angles);//test
	CVAR_REGISTER (&phys_simulateragdoll);//test

	CVAR_REGISTER (&mp_am_brief);
	CVAR_REGISTER (&mp_am_brief_name);
	
	CVAR_REGISTER (&mp_am_warning_sound);

	CVAR_REGISTER (&devlight);
	CVAR_REGISTER (&detalle);
	CVAR_REGISTER (&selection);
	CVAR_REGISTER (&totaldialogpoints);

	CVAR_REGISTER (&mp_am_noblastgibs);
	CVAR_REGISTER (&mp_am_gore);
	
//	CVAR_REGISTER (&mp_am_flyattack);	
//	CVAR_REGISTER (&mp_am_movecorpses);
	CVAR_REGISTER (&mp_am_lenguage);	

	CVAR_REGISTER (&mp_am_truquitos);	

	CVAR_REGISTER (&mp_am_weaponweight);
	CVAR_REGISTER (&mp_am_default_weapons);

//	CVAR_REGISTER (&xp_points);

	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
	g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );

	CVAR_REGISTER (&displaysoundlist);

	CVAR_REGISTER (&teamplay);
	CVAR_REGISTER (&fraglimit);
	CVAR_REGISTER (&timelimit);

	CVAR_REGISTER (&fragsleft);
	CVAR_REGISTER (&timeleft);

	CVAR_REGISTER (&friendlyfire);
	CVAR_REGISTER (&falldamage);
	CVAR_REGISTER (&weaponstay);
	CVAR_REGISTER (&forcerespawn);
	CVAR_REGISTER (&flashlight);
	CVAR_REGISTER (&aimcrosshair);
	CVAR_REGISTER (&decalfrequency);
	CVAR_REGISTER (&teamlist);
	CVAR_REGISTER (&teamoverride);
	CVAR_REGISTER (&defaultteam);
	CVAR_REGISTER (&allowmonsters);
	//CVAR_REGISTER (&mp3player); //G-Cont. Stuff for Mp3 player	//AJH - Too late, I've already added mp3/ogg
    //CVAR_REGISTER (&mp3volume); //G-Cont. Stuff for Mp3 player	//AJH

	CVAR_REGISTER (&mp_chattime);

// REGISTER CVARS FOR SKILL LEVEL STUFF
	// Agrunt
	CVAR_REGISTER ( &sk_agrunt_health1 );// {"sk_agrunt_health1","0"};
	CVAR_REGISTER ( &sk_agrunt_health2 );// {"sk_agrunt_health2","0"};
	CVAR_REGISTER ( &sk_agrunt_health3 );// {"sk_agrunt_health3","0"};

	CVAR_REGISTER ( &sk_agrunt_dmg_punch1 );// {"sk_agrunt_dmg_punch1","0"};
	CVAR_REGISTER ( &sk_agrunt_dmg_punch2 );// {"sk_agrunt_dmg_punch2","0"};
	CVAR_REGISTER ( &sk_agrunt_dmg_punch3 );// {"sk_agrunt_dmg_punch3","0"};

	// Apache
	CVAR_REGISTER ( &sk_apache_health1 );// {"sk_apache_health1","0"};
	CVAR_REGISTER ( &sk_apache_health2 );// {"sk_apache_health2","0"};
	CVAR_REGISTER ( &sk_apache_health3 );// {"sk_apache_health3","0"};

	// Barney
	CVAR_REGISTER ( &sk_barney_health1 );// {"sk_barney_health1","0"};
	CVAR_REGISTER ( &sk_barney_health2 );// {"sk_barney_health2","0"};
	CVAR_REGISTER ( &sk_barney_health3 );// {"sk_barney_health3","0"};

	// Bullsquid
	CVAR_REGISTER ( &sk_bullsquid_health1 );// {"sk_bullsquid_health1","0"};
	CVAR_REGISTER ( &sk_bullsquid_health2 );// {"sk_bullsquid_health2","0"};
	CVAR_REGISTER ( &sk_bullsquid_health3 );// {"sk_bullsquid_health3","0"};

	CVAR_REGISTER ( &sk_bullsquid_dmg_bite1 );// {"sk_bullsquid_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_bite2 );// {"sk_bullsquid_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_bite3 );// {"sk_bullsquid_dmg_bite3","0"};

	CVAR_REGISTER ( &sk_bullsquid_dmg_whip1 );// {"sk_bullsquid_dmg_whip1","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_whip2 );// {"sk_bullsquid_dmg_whip2","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_whip3 );// {"sk_bullsquid_dmg_whip3","0"};

	CVAR_REGISTER ( &sk_bullsquid_dmg_spit1 );// {"sk_bullsquid_dmg_spit1","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_spit2 );// {"sk_bullsquid_dmg_spit2","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_spit3 );// {"sk_bullsquid_dmg_spit3","0"};


	CVAR_REGISTER ( &sk_bigmomma_health_factor1 );// {"sk_bigmomma_health_factor1","1.0"};
	CVAR_REGISTER ( &sk_bigmomma_health_factor2 );// {"sk_bigmomma_health_factor2","1.0"};
	CVAR_REGISTER ( &sk_bigmomma_health_factor3 );// {"sk_bigmomma_health_factor3","1.0"};

	CVAR_REGISTER ( &sk_bigmomma_dmg_slash1 );// {"sk_bigmomma_dmg_slash1","50"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_slash2 );// {"sk_bigmomma_dmg_slash2","50"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_slash3 );// {"sk_bigmomma_dmg_slash3","50"};

	CVAR_REGISTER ( &sk_bigmomma_dmg_blast1 );// {"sk_bigmomma_dmg_blast1","100"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_blast2 );// {"sk_bigmomma_dmg_blast2","100"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_blast3 );// {"sk_bigmomma_dmg_blast3","100"};

	CVAR_REGISTER ( &sk_bigmomma_radius_blast1 );// {"sk_bigmomma_radius_blast1","250"};
	CVAR_REGISTER ( &sk_bigmomma_radius_blast2 );// {"sk_bigmomma_radius_blast2","250"};
	CVAR_REGISTER ( &sk_bigmomma_radius_blast3 );// {"sk_bigmomma_radius_blast3","250"};

	// Gargantua
	CVAR_REGISTER ( &sk_gargantua_health1 );// {"sk_gargantua_health1","0"};
	CVAR_REGISTER ( &sk_gargantua_health2 );// {"sk_gargantua_health2","0"};
	CVAR_REGISTER ( &sk_gargantua_health3 );// {"sk_gargantua_health3","0"};

	CVAR_REGISTER ( &sk_gargantua_dmg_slash1 );// {"sk_gargantua_dmg_slash1","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_slash2 );// {"sk_gargantua_dmg_slash2","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_slash3 );// {"sk_gargantua_dmg_slash3","0"};

	CVAR_REGISTER ( &sk_gargantua_dmg_fire1 );// {"sk_gargantua_dmg_fire1","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_fire2 );// {"sk_gargantua_dmg_fire2","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_fire3 );// {"sk_gargantua_dmg_fire3","0"};

	CVAR_REGISTER ( &sk_gargantua_dmg_stomp1 );// {"sk_gargantua_dmg_stomp1","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_stomp2 );// {"sk_gargantua_dmg_stomp2","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_stomp3	);// {"sk_gargantua_dmg_stomp3","0"};


	// Hassassin
	CVAR_REGISTER ( &sk_hassassin_health1 );// {"sk_hassassin_health1","0"};
	CVAR_REGISTER ( &sk_hassassin_health2 );// {"sk_hassassin_health2","0"};
	CVAR_REGISTER ( &sk_hassassin_health3 );// {"sk_hassassin_health3","0"};


	// Headcrab
	CVAR_REGISTER ( &sk_headcrab_health1 );// {"sk_headcrab_health1","0"};
	CVAR_REGISTER ( &sk_headcrab_health2 );// {"sk_headcrab_health2","0"};
	CVAR_REGISTER ( &sk_headcrab_health3 );// {"sk_headcrab_health3","0"};

	CVAR_REGISTER ( &sk_headcrab_dmg_bite1 );// {"sk_headcrab_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_headcrab_dmg_bite2 );// {"sk_headcrab_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_headcrab_dmg_bite3 );// {"sk_headcrab_dmg_bite3","0"};


	// Hgrunt 
	CVAR_REGISTER ( &sk_hgrunt_health1 );// {"sk_hgrunt_health1","0"};
	CVAR_REGISTER ( &sk_hgrunt_health2 );// {"sk_hgrunt_health2","0"};
	CVAR_REGISTER ( &sk_hgrunt_health3 );// {"sk_hgrunt_health3","0"};

	CVAR_REGISTER ( &sk_hgrunt_kick1 );// {"sk_hgrunt_kick1","0"};
	CVAR_REGISTER ( &sk_hgrunt_kick2 );// {"sk_hgrunt_kick2","0"};
	CVAR_REGISTER ( &sk_hgrunt_kick3 );// {"sk_hgrunt_kick3","0"};

	CVAR_REGISTER ( &sk_hgrunt_pellets1 );
	CVAR_REGISTER ( &sk_hgrunt_pellets2 );
	CVAR_REGISTER ( &sk_hgrunt_pellets3 );

	CVAR_REGISTER ( &sk_hgrunt_gspeed1 );
	CVAR_REGISTER ( &sk_hgrunt_gspeed2 );
	CVAR_REGISTER ( &sk_hgrunt_gspeed3 );

	//Zombie Poison
	CVAR_REGISTER ( &sk_zombie_poison_health1 );
	CVAR_REGISTER ( &sk_zombie_poison_health2 );
	CVAR_REGISTER ( &sk_zombie_poison_health3 );

	CVAR_REGISTER ( &sk_zombie_poison_melee1 );
	CVAR_REGISTER ( &sk_zombie_poison_melee2 );
	CVAR_REGISTER ( &sk_zombie_poison_melee3 );

			// Hgrunt_cs
			/*
	CVAR_REGISTER ( &sk_hgrunt_cs_health1 );// {"sk_hgrunt_cs_health1","0"};
	CVAR_REGISTER ( &sk_hgrunt_cs_health2 );// {"sk_hgrunt_cs_health2","0"};
	CVAR_REGISTER ( &sk_hgrunt_cs_health3 );// {"sk_hgrunt_cs_health3","0"};

	CVAR_REGISTER ( &sk_hgrunt_cs_kick1 );// {"sk_hgrunt_cs_kick1","0"};
	CVAR_REGISTER ( &sk_hgrunt_cs_kick2 );// {"sk_hgrunt_cs_kick2","0"};
	CVAR_REGISTER ( &sk_hgrunt_cs_kick3 );// {"sk_hgrunt_cs_kick3","0"};

	CVAR_REGISTER ( &sk_hgrunt_cs_pellets1 );
	CVAR_REGISTER ( &sk_hgrunt_cs_pellets2 );
	CVAR_REGISTER ( &sk_hgrunt_cs_pellets3 );

	CVAR_REGISTER ( &sk_hgrunt_cs_gspeed1 );
	CVAR_REGISTER ( &sk_hgrunt_cs_gspeed2 );
	CVAR_REGISTER ( &sk_hgrunt_cs_gspeed3 );*/

		// Hgrunt_de
		/*
	CVAR_REGISTER ( &sk_hgrunt_de_health1 );// {"sk_hgrunt_de_health1","0"};
	CVAR_REGISTER ( &sk_hgrunt_de_health2 );// {"sk_hgrunt_de_health2","0"};
	CVAR_REGISTER ( &sk_hgrunt_de_health3 );// {"sk_hgrunt_de_health3","0"};

	CVAR_REGISTER ( &sk_hgrunt_de_kick1 );// {"sk_hgrunt_de_kick1","0"};
	CVAR_REGISTER ( &sk_hgrunt_de_kick2 );// {"sk_hgrunt_de_kick2","0"};
	CVAR_REGISTER ( &sk_hgrunt_de_kick3 );// {"sk_hgrunt_de_kick3","0"};

	CVAR_REGISTER ( &sk_hgrunt_de_pellets1 );
	CVAR_REGISTER ( &sk_hgrunt_de_pellets2 );
	CVAR_REGISTER ( &sk_hgrunt_de_pellets3 );

	CVAR_REGISTER ( &sk_hgrunt_de_gspeed1 );
	CVAR_REGISTER ( &sk_hgrunt_de_gspeed2 );
	CVAR_REGISTER ( &sk_hgrunt_de_gspeed3 );*/

		// Hgrunt AK
		/*
	CVAR_REGISTER ( &sk_hgrunt_ak_health1 );// {"sk_HGrunt_ak_health1","0"};
	CVAR_REGISTER ( &sk_hgrunt_ak_health2 );// {"sk_HGrunt_ak_health2","0"};
	CVAR_REGISTER ( &sk_hgrunt_ak_health3 );// {"sk_HGrunt_ak_health3","0"};

	CVAR_REGISTER ( &sk_hgrunt_ak_kick1 );// {"sk_HGrunt_ak_kick1","0"};
	CVAR_REGISTER ( &sk_hgrunt_ak_kick2 );// {"sk_HGrunt_ak_kick2","0"};
	CVAR_REGISTER ( &sk_hgrunt_ak_kick3 );// {"sk_HGrunt_ak_kick3","0"};

	CVAR_REGISTER ( &sk_hgrunt_ak_pellets1 );
	CVAR_REGISTER ( &sk_hgrunt_ak_pellets2 );
	CVAR_REGISTER ( &sk_hgrunt_ak_pellets3 );

	CVAR_REGISTER ( &sk_hgrunt_ak_gspeed1 );
	CVAR_REGISTER ( &sk_hgrunt_ak_gspeed2 );
	CVAR_REGISTER ( &sk_hgrunt_ak_gspeed3 );*/

	// OTIS
	CVAR_REGISTER ( &sk_otis_health1 );// {"sk_HGrunt_ak_health1","0"};
	CVAR_REGISTER ( &sk_otis_health2 );// {"sk_hgrunt_pi_health2","0"};
	CVAR_REGISTER ( &sk_otis_health3 );// {"sk_hgrunt_pi_health3","0"};

	
	// Houndeye
	CVAR_REGISTER ( &sk_houndeye_health1 );// {"sk_houndeye_health1","0"};
	CVAR_REGISTER ( &sk_houndeye_health2 );// {"sk_houndeye_health2","0"};
	CVAR_REGISTER ( &sk_houndeye_health3 );// {"sk_houndeye_health3","0"};

	CVAR_REGISTER ( &sk_houndeye_dmg_blast1 );// {"sk_houndeye_dmg_blast1","0"};
	CVAR_REGISTER ( &sk_houndeye_dmg_blast2 );// {"sk_houndeye_dmg_blast2","0"};
	CVAR_REGISTER ( &sk_houndeye_dmg_blast3 );// {"sk_houndeye_dmg_blast3","0"};


	// ISlave
	CVAR_REGISTER ( &sk_islave_health1 );// {"sk_islave_health1","0"};
	CVAR_REGISTER ( &sk_islave_health2 );// {"sk_islave_health2","0"};
	CVAR_REGISTER ( &sk_islave_health3 );// {"sk_islave_health3","0"};

	CVAR_REGISTER ( &sk_islave_dmg_claw1 );// {"sk_islave_dmg_claw1","0"};
	CVAR_REGISTER ( &sk_islave_dmg_claw2 );// {"sk_islave_dmg_claw2","0"};
	CVAR_REGISTER ( &sk_islave_dmg_claw3 );// {"sk_islave_dmg_claw3","0"};

	CVAR_REGISTER ( &sk_islave_dmg_clawrake1	);// {"sk_islave_dmg_clawrake1","0"};
	CVAR_REGISTER ( &sk_islave_dmg_clawrake2	);// {"sk_islave_dmg_clawrake2","0"};
	CVAR_REGISTER ( &sk_islave_dmg_clawrake3	);// {"sk_islave_dmg_clawrake3","0"};
		
	CVAR_REGISTER ( &sk_islave_dmg_zap1 );// {"sk_islave_dmg_zap1","0"};
	CVAR_REGISTER ( &sk_islave_dmg_zap2 );// {"sk_islave_dmg_zap2","0"};
	CVAR_REGISTER ( &sk_islave_dmg_zap3 );// {"sk_islave_dmg_zap3","0"};


	// Icthyosaur
	CVAR_REGISTER ( &sk_ichthyosaur_health1	);// {"sk_ichthyosaur_health1","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_health2	);// {"sk_ichthyosaur_health2","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_health3	);// {"sk_ichthyosaur_health3","0"};

	CVAR_REGISTER ( &sk_ichthyosaur_shake1	);// {"sk_ichthyosaur_health3","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_shake2	);// {"sk_ichthyosaur_health3","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_shake3	);// {"sk_ichthyosaur_health3","0"};



	// Leech
	CVAR_REGISTER ( &sk_leech_health1 );// {"sk_leech_health1","0"};
	CVAR_REGISTER ( &sk_leech_health2 );// {"sk_leech_health2","0"};
	CVAR_REGISTER ( &sk_leech_health3 );// {"sk_leech_health3","0"};

	CVAR_REGISTER ( &sk_leech_dmg_bite1 );// {"sk_leech_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_leech_dmg_bite2 );// {"sk_leech_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_leech_dmg_bite3 );// {"sk_leech_dmg_bite3","0"};


	// Controller
	CVAR_REGISTER ( &sk_controller_health1 );
	CVAR_REGISTER ( &sk_controller_health2 );
	CVAR_REGISTER ( &sk_controller_health3 );

	CVAR_REGISTER ( &sk_controller_dmgzap1 );
	CVAR_REGISTER ( &sk_controller_dmgzap2 );
	CVAR_REGISTER ( &sk_controller_dmgzap3 );

	CVAR_REGISTER ( &sk_controller_speedball1 );
	CVAR_REGISTER ( &sk_controller_speedball2 );
	CVAR_REGISTER ( &sk_controller_speedball3 );

	CVAR_REGISTER ( &sk_controller_dmgball1 );
	CVAR_REGISTER ( &sk_controller_dmgball2 );
	CVAR_REGISTER ( &sk_controller_dmgball3 );

	// Nihilanth
	CVAR_REGISTER ( &sk_nihilanth_health1 );// {"sk_nihilanth_health1","0"};
	CVAR_REGISTER ( &sk_nihilanth_health2 );// {"sk_nihilanth_health2","0"};
	CVAR_REGISTER ( &sk_nihilanth_health3 );// {"sk_nihilanth_health3","0"};

	CVAR_REGISTER ( &sk_nihilanth_zap1 );
	CVAR_REGISTER ( &sk_nihilanth_zap2 );
	CVAR_REGISTER ( &sk_nihilanth_zap3 );

	// Scientist
	CVAR_REGISTER ( &sk_scientist_health1 );// {"sk_scientist_health1","0"};
	CVAR_REGISTER ( &sk_scientist_health2 );// {"sk_scientist_health2","0"};
	CVAR_REGISTER ( &sk_scientist_health3 );// {"sk_scientist_health3","0"};


	// Snark
	CVAR_REGISTER ( &sk_snark_health1 );// {"sk_snark_health1","0"};
	CVAR_REGISTER ( &sk_snark_health2 );// {"sk_snark_health2","0"};
	CVAR_REGISTER ( &sk_snark_health3 );// {"sk_snark_health3","0"};

	CVAR_REGISTER ( &sk_snark_dmg_bite1 );// {"sk_snark_dmg_bite1","0"};
	CVAR_REGISTER ( &sk_snark_dmg_bite2 );// {"sk_snark_dmg_bite2","0"};
	CVAR_REGISTER ( &sk_snark_dmg_bite3 );// {"sk_snark_dmg_bite3","0"};

	CVAR_REGISTER ( &sk_snark_dmg_pop1 );// {"sk_snark_dmg_pop1","0"};
	CVAR_REGISTER ( &sk_snark_dmg_pop2 );// {"sk_snark_dmg_pop2","0"};
	CVAR_REGISTER ( &sk_snark_dmg_pop3 );// {"sk_snark_dmg_pop3","0"};

//pitdrone
	CVAR_REGISTER ( &sk_pit_drone_health1 );// {"sk_zombie_health1","0"};
	CVAR_REGISTER ( &sk_pit_drone_health2 );// {"sk_zombie_health3","0"};
	CVAR_REGISTER ( &sk_pit_drone_health3 );// {"sk_zombie_health3","0"};

	// Zombie
	CVAR_REGISTER ( &sk_zombie_health1 );// {"sk_zombie_health1","0"};
	CVAR_REGISTER ( &sk_zombie_health2 );// {"sk_zombie_health3","0"};
	CVAR_REGISTER ( &sk_zombie_health3 );// {"sk_zombie_health3","0"};

	CVAR_REGISTER ( &sk_zombie_dmg_one_slash1 );// {"sk_zombie_dmg_one_slash1","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_one_slash2 );// {"sk_zombie_dmg_one_slash2","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_one_slash3 );// {"sk_zombie_dmg_one_slash3","0"};

	CVAR_REGISTER ( &sk_zombie_dmg_both_slash1 );// {"sk_zombie_dmg_both_slash1","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_both_slash2 );// {"sk_zombie_dmg_both_slash2","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_both_slash3 );// {"sk_zombie_dmg_both_slash3","0"};
	
	CVAR_REGISTER ( &sk_scientist_crowbar_hit1 );// {"sk_scientist_crowbar_hit1","0"};
	CVAR_REGISTER ( &sk_scientist_crowbar_hit2);// {"sk_scientist_crowbar_hit2","0"};
	CVAR_REGISTER ( &sk_scientist_crowbar_hit3 );// {"sk_scientist_crowbar_hit3","0"};

	// strooper
	CVAR_REGISTER ( &sk_strooper_health1 );// {"sk_gonome_health1","0"};
	CVAR_REGISTER ( &sk_strooper_health2 );// {"sk_gonome_health3","0"};
	CVAR_REGISTER ( &sk_strooper_health3 );// {"sk_gonome_health3","0"};

	// robot
	CVAR_REGISTER ( &sk_robot_health1 );// {"sk_gonome_health1","0"};
	CVAR_REGISTER ( &sk_robot_health2 );// {"sk_gonome_health3","0"};
	CVAR_REGISTER ( &sk_robot_health3 );// {"sk_gonome_health3","0"};

	// gonome
	CVAR_REGISTER ( &sk_gonome_health1 );// {"sk_gonome_health1","0"};
	CVAR_REGISTER ( &sk_gonome_health2 );// {"sk_gonome_health3","0"};
	CVAR_REGISTER ( &sk_gonome_health3 );// {"sk_gonome_health3","0"};

	CVAR_REGISTER ( &sk_gonome_dmg_one_slash1 );// {"sk_gonome_dmg_one_slash1","0"};
	CVAR_REGISTER ( &sk_gonome_dmg_one_slash2 );// {"sk_gonome_dmg_one_slash2","0"};
	CVAR_REGISTER ( &sk_gonome_dmg_one_slash3 );// {"sk_gonome_dmg_one_slash3","0"};

	CVAR_REGISTER ( &sk_gonome_dmg_both_slash1 );// {"sk_gonome_dmg_both_slash1","0"};
	CVAR_REGISTER ( &sk_gonome_dmg_both_slash2 );// {"sk_gonome_dmg_both_slash2","0"};
	CVAR_REGISTER ( &sk_gonome_dmg_both_slash3 );// {"sk_gonome_dmg_both_slash3","0"};


	//Turret
	CVAR_REGISTER ( &sk_turret_health1 );// {"sk_turret_health1","0"};
	CVAR_REGISTER ( &sk_turret_health2 );// {"sk_turret_health2","0"};
	CVAR_REGISTER ( &sk_turret_health3 );// {"sk_turret_health3","0"};


	// MiniTurret
	CVAR_REGISTER ( &sk_miniturret_health1 );// {"sk_miniturret_health1","0"};
	CVAR_REGISTER ( &sk_miniturret_health2 );// {"sk_miniturret_health2","0"};
	CVAR_REGISTER ( &sk_miniturret_health3 );// {"sk_miniturret_health3","0"};


	// Sentry Turret
	CVAR_REGISTER ( &sk_sentry_health1 );// {"sk_sentry_health1","0"};
	CVAR_REGISTER ( &sk_sentry_health2 );// {"sk_sentry_health2","0"};
	CVAR_REGISTER ( &sk_sentry_health3 );// {"sk_sentry_health3","0"};


	// PLAYER WEAPONS
	
	CVAR_REGISTER ( &sk_plr_crowbar1 );// {"sk_plr_knife1","0"};
	CVAR_REGISTER ( &sk_plr_crowbar2 );// {"sk_plr_cknife2","0"};
	CVAR_REGISTER ( &sk_plr_crowbar3 );// {"sk_plr_knife3","0"};

	// Crowbar whack
	CVAR_REGISTER ( &sk_plr_knife1 );// {"sk_plr_knife1","0"};
	CVAR_REGISTER ( &sk_plr_knife2 );// {"sk_plr_cknife2","0"};
	CVAR_REGISTER ( &sk_plr_knife3 );// {"sk_plr_knife3","0"};

	// Crowbar whack
	CVAR_REGISTER ( &sk_plr_knife_stab1 );// {"sk_plr__stab1","0"};
	CVAR_REGISTER ( &sk_plr_knife_stab2 );// {"sk_plr_stabe2","0"};
	CVAR_REGISTER ( &sk_plr_knife_stab3 );// {"sk_plr_stabfe3","0"};

	// Glock Round
	CVAR_REGISTER ( &sk_plr_9mm_bullet1 );// {"sk_plr_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_9mm_bullet2 );// {"sk_plr_9mm_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_9mm_bullet3 );// {"sk_plr_9mm_bullet3","0"};

	// 357 Round
	CVAR_REGISTER ( &sk_plr_357_bullet1 );// {"sk_plr_357_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_357_bullet2 );// {"sk_plr_357_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_357_bullet3 );// {"sk_plr_357_bullet3","0"};

	// MP5 Round
	CVAR_REGISTER ( &sk_plr_9mmAR_bullet1 );// {"sk_plr_9mmAR_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_bullet2 );// {"sk_plr_9mmAR_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_bullet3 );// {"sk_plr_9mmAR_bullet3","0"};

	// M16 Round
	CVAR_REGISTER ( &sk_plr_M16_bullet1 );// {"sk_plr_M16_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_M16_bullet2 );// {"sk_plr_M16_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_M16_bullet3 );// {"sk_plr_M16_bullet3","0"};

	// AK74 Round
	CVAR_REGISTER ( &sk_plr_AK74_bullet1 );// {"sk_plr_AK74_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_AK74_bullet2 );// {"sk_plr_AK74_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_AK74_bullet3 );// {"sk_plr_AK74_bullet3","0"};

	// m82 Round
	CVAR_REGISTER ( &sk_plr_m82_bullet1 );// {"sk_plr_UZI_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_m82_bullet2 );// {"sk_plr_UZI_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_m82_bullet3 );// {"sk_plr_UZI_bullet3","0"};

	// UZI Round
	CVAR_REGISTER ( &sk_plr_UZI_bullet1 );// {"sk_plr_UZI_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_UZI_bullet2 );// {"sk_plr_UZI_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_UZI_bullet3 );// {"sk_plr_UZI_bullet3","0"};

	// c4 Round
	CVAR_REGISTER ( &sk_plr_c4bomb1 );// {"sk_plr_c4bomb1","0"};
	CVAR_REGISTER ( &sk_plr_c4bomb2 );// {"sk_plr_c4bomb2","0"};
	CVAR_REGISTER ( &sk_plr_c4bomb3 );// {"sk_plr_c4bomb3","0"};

	// p90 Round
	CVAR_REGISTER ( &sk_plr_p90_bullet1 );// {"sk_plr_p90_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_p90_bullet2 );// {"sk_plr_p90_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_p90_bullet3 );// {"sk_plr_p90_bullet3","0"};

	// ber92F Round
	CVAR_REGISTER ( &sk_plr_ber92F_bullet1 );// {"sk_plr","0"};
	CVAR_REGISTER ( &sk_plr_ber92F_bullet2 );// {"sk_plet2","0"};
	CVAR_REGISTER ( &sk_plr_ber92F_bullet3 );// {"sk__bullet3","0"};

	// glock18 Round
	CVAR_REGISTER ( &sk_plr_glock18_bullet1 );// {"sk_plr","0"};
	CVAR_REGISTER ( &sk_plr_glock18_bullet2 );// {"sk_plet2","0"};
	CVAR_REGISTER ( &sk_plr_glock18_bullet3 );// {"sk__bullet3","0"};

	// M249 Round
	CVAR_REGISTER ( &sk_plr_M249_bullet1 );// {"sk_plr_M249_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_M249_bullet2 );// {"sk_plr_M249_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_M249_bullet3 );// {"sk_plr_M249_bullet3","0"};

	// deagle
	CVAR_REGISTER ( &sk_plr_deagle_bullet1 );// {"sk_plr_deagle_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_deagle_bullet2 );// {"sk_plr_deagle_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_deagle_bullet3 );// {"sk_plr_deagle_bullet3","0"};

	// sniper
	CVAR_REGISTER ( &sk_plr_sniper_bullet1 );// {"sk_plr_sniper_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_sniper_bullet2 );// {"sk_plr_sniper_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_sniper_bullet3 );// {"sk_plr_sniper_bullet3","0"};

	// sniper
	CVAR_REGISTER ( &sk_plr_scout_bullet1 );
	CVAR_REGISTER ( &sk_plr_scout_bullet2 );
	CVAR_REGISTER ( &sk_plr_scout_bullet3 );

	// FAMAS Round
	CVAR_REGISTER ( &sk_plr_famas_bullet1 );// {"sk_plr_famas_bullet1","0"};
	CVAR_REGISTER ( &sk_plr_famas_bullet2 );// {"sk_plr_famas_bullet2","0"};
	CVAR_REGISTER ( &sk_plr_famas_bullet3 );// {"sk_plr_famas_bullet3","0"};

	// usas Round
	CVAR_REGISTER ( &sk_plr_usas_buckshot1 );
	CVAR_REGISTER ( &sk_plr_usas_buckshot2 );
	CVAR_REGISTER ( &sk_plr_usas_buckshot3 );

	// M203 grenade
	CVAR_REGISTER ( &sk_plr_9mmAR_grenade1 );// {"sk_plr_9mmAR_grenade1","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_grenade2 );// {"sk_plr_9mmAR_grenade2","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_grenade3 );// {"sk_plr_9mmAR_grenade3","0"};


	// Shotgun buckshot
	CVAR_REGISTER ( &sk_plr_buckshot1 );// {"sk_plr_buckshot1","0"};
	CVAR_REGISTER ( &sk_plr_buckshot2 );// {"sk_plr_buckshot2","0"};
	CVAR_REGISTER ( &sk_plr_buckshot3 );// {"sk_plr_buckshot3","0"};


	// Crossbow
	CVAR_REGISTER ( &sk_plr_xbow_bolt_monster1 );// {"sk_plr_xbow_bolt1","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_monster2 );// {"sk_plr_xbow_bolt2","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_monster3 );// {"sk_plr_xbow_bolt3","0"};

	CVAR_REGISTER ( &sk_plr_xbow_bolt_client1 );// {"sk_plr_xbow_bolt1","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_client2 );// {"sk_plr_xbow_bolt2","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_client3 );// {"sk_plr_xbow_bolt3","0"};


	// RPG
	CVAR_REGISTER ( &sk_plr_rpg1 );// {"sk_plr_rpg1","0"};
	CVAR_REGISTER ( &sk_plr_rpg2 );// {"sk_plr_rpg2","0"};
	CVAR_REGISTER ( &sk_plr_rpg3 );// {"sk_plr_rpg3","0"};


	// Gauss Gun
	CVAR_REGISTER ( &sk_plr_gauss1 );// {"sk_plr_gauss1","0"};
	CVAR_REGISTER ( &sk_plr_gauss2 );// {"sk_plr_gauss2","0"};
	CVAR_REGISTER ( &sk_plr_gauss3 );// {"sk_plr_gauss3","0"};


	// Egon Gun
	CVAR_REGISTER ( &sk_plr_egon_narrow1 );// {"sk_plr_egon_narrow1","0"};
	CVAR_REGISTER ( &sk_plr_egon_narrow2 );// {"sk_plr_egon_narrow2","0"};
	CVAR_REGISTER ( &sk_plr_egon_narrow3 );// {"sk_plr_egon_narrow3","0"};

	CVAR_REGISTER ( &sk_plr_egon_wide1 );// {"sk_plr_egon_wide1","0"};
	CVAR_REGISTER ( &sk_plr_egon_wide2 );// {"sk_plr_egon_wide2","0"};
	CVAR_REGISTER ( &sk_plr_egon_wide3 );// {"sk_plr_egon_wide3","0"};


	// Hand Grendade
	CVAR_REGISTER ( &sk_plr_hand_grenade1 );// {"sk_plr_hand_grenade1","0"};
	CVAR_REGISTER ( &sk_plr_hand_grenade2 );// {"sk_plr_hand_grenade2","0"};
	CVAR_REGISTER ( &sk_plr_hand_grenade3 );// {"sk_plr_hand_grenade3","0"};


	// Satchel Charge
	CVAR_REGISTER ( &sk_plr_satchel1 );// {"sk_plr_satchel1","0"};
	CVAR_REGISTER ( &sk_plr_satchel2 );// {"sk_plr_satchel2","0"};
	CVAR_REGISTER ( &sk_plr_satchel3 );// {"sk_plr_satchel3","0"};


	// Tripmine
	CVAR_REGISTER ( &sk_plr_tripmine1 );// {"sk_plr_tripmine1","0"};
	CVAR_REGISTER ( &sk_plr_tripmine2 );// {"sk_plr_tripmine2","0"};
	CVAR_REGISTER ( &sk_plr_tripmine3 );// {"sk_plr_tripmine3","0"};


	// WORLD WEAPONS
	CVAR_REGISTER ( &sk_12mm_bullet1 );// {"sk_12mm_bullet1","0"};
	CVAR_REGISTER ( &sk_12mm_bullet2 );// {"sk_12mm_bullet2","0"};
	CVAR_REGISTER ( &sk_12mm_bullet3 );// {"sk_12mm_bullet3","0"};

	CVAR_REGISTER ( &sk_9mmAR_bullet1 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_9mmAR_bullet2 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_9mmAR_bullet3 );// {"sk_9mm_bullet1","0"};

	CVAR_REGISTER ( &sk_9mm_bullet1 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_9mm_bullet2 );// {"sk_9mm_bullet2","0"};
	CVAR_REGISTER ( &sk_9mm_bullet3 );// {"sk_9mm_bullet3","0"};

//NEW

	CVAR_REGISTER ( &sk_ak_bullet1 );// {"sk_12mm_bullet1","0"};
	CVAR_REGISTER ( &sk_ak_bullet2 );// {"sk_12mm_bullet2","0"};
	CVAR_REGISTER ( &sk_ak_bullet3 );// {"sk_12mm_bullet3","0"};

	CVAR_REGISTER ( &sk_556_bullet1 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_556_bullet2 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_556_bullet3 );// {"sk_9mm_bullet1","0"};

	CVAR_REGISTER ( &sk_deagle_bullet1 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_deagle_bullet2 );// {"sk_9mm_bullet2","0"};
	CVAR_REGISTER ( &sk_deagle_bullet3 );// {"sk_9mm_bullet3","0"};

	CVAR_REGISTER ( &sk_awp_bullet1 );// {"sk_9mm_bullet1","0"};
	CVAR_REGISTER ( &sk_awp_bullet2 );// {"sk_9mm_bullet2","0"};
	CVAR_REGISTER ( &sk_awp_bullet3 );// {"sk_9mm_bullet3","0"};

	CVAR_REGISTER ( &sk_plr_irgun_bullet1 );
	CVAR_REGISTER ( &sk_plr_irgun_bullet2 );
	CVAR_REGISTER ( &sk_plr_irgun_bullet3 );

	// HORNET
	CVAR_REGISTER ( &sk_hornet_dmg1 );// {"sk_hornet_dmg1","0"};
	CVAR_REGISTER ( &sk_hornet_dmg2 );// {"sk_hornet_dmg2","0"};
	CVAR_REGISTER ( &sk_hornet_dmg3 );// {"sk_hornet_dmg3","0"};

	// HEALTH/SUIT CHARGE DISTRIBUTION
	CVAR_REGISTER ( &sk_suitcharger1 );
	CVAR_REGISTER ( &sk_suitcharger2 );
	CVAR_REGISTER ( &sk_suitcharger3 );

	CVAR_REGISTER ( &sk_battery1 );
	CVAR_REGISTER ( &sk_battery2 );
	CVAR_REGISTER ( &sk_battery3 );

	CVAR_REGISTER ( &sk_healthcharger1 );
	CVAR_REGISTER ( &sk_healthcharger2 );
	CVAR_REGISTER ( &sk_healthcharger3 );

	CVAR_REGISTER ( &sk_healthkit1 );
	CVAR_REGISTER ( &sk_healthkit2 );
	CVAR_REGISTER ( &sk_healthkit3 );

	CVAR_REGISTER ( &sk_scientist_heal1 );
	CVAR_REGISTER ( &sk_scientist_heal2 );
	CVAR_REGISTER ( &sk_scientist_heal3 );

// monster damage adjusters
	CVAR_REGISTER ( &sk_monster_head1 );
	CVAR_REGISTER ( &sk_monster_head2 );
	CVAR_REGISTER ( &sk_monster_head3 );

	CVAR_REGISTER ( &sk_monster_chest1 );
	CVAR_REGISTER ( &sk_monster_chest2 );
	CVAR_REGISTER ( &sk_monster_chest3 );

	CVAR_REGISTER ( &sk_monster_stomach1 );
	CVAR_REGISTER ( &sk_monster_stomach2 );
	CVAR_REGISTER ( &sk_monster_stomach3 );

	CVAR_REGISTER ( &sk_monster_arm1 );
	CVAR_REGISTER ( &sk_monster_arm2 );
	CVAR_REGISTER ( &sk_monster_arm3 );

	CVAR_REGISTER ( &sk_monster_leg1 );
	CVAR_REGISTER ( &sk_monster_leg2 );
	CVAR_REGISTER ( &sk_monster_leg3 );

// player damage adjusters
	CVAR_REGISTER ( &sk_player_head1 );
	CVAR_REGISTER ( &sk_player_head2 );
	CVAR_REGISTER ( &sk_player_head3 );

	CVAR_REGISTER ( &sk_player_chest1 );
	CVAR_REGISTER ( &sk_player_chest2 );
	CVAR_REGISTER ( &sk_player_chest3 );

	CVAR_REGISTER ( &sk_player_stomach1 );
	CVAR_REGISTER ( &sk_player_stomach2 );
	CVAR_REGISTER ( &sk_player_stomach3 );

	CVAR_REGISTER ( &sk_player_arm1 );
	CVAR_REGISTER ( &sk_player_arm2 );
	CVAR_REGISTER ( &sk_player_arm3 );

	CVAR_REGISTER ( &sk_player_leg1 );
	CVAR_REGISTER ( &sk_player_leg2 );
	CVAR_REGISTER ( &sk_player_leg3 );
// END REGISTER CVARS FOR SKILL LEVEL STUFF


	SERVER_COMMAND( "exec cfg/skill.cfg\n" );

	
	SERVER_COMMAND( "exec cfg/pl_movement.cfg\n" );
	SERVER_COMMAND( "exec cfg/pl_zoom.cfg\n" );

	SERVER_COMMAND( "exec cfg/s2p_physics.cfg\n" );
}

