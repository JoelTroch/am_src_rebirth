/*
ARRANGEMENT Source Code Version 1.0.9
Developers Readme File

Thanks again for downloading our Source Code of Half-Life: Arrangement. We hope this Source Code will be useful for the most people that are working today in Half-Life Engine.

Before continue, please read the file "GNU LESSER GENERAL PUBLIC LICENSE" for copying, use and distribution details.

1] LIST OF FEATURES AND DESCRIPTION OF AM
2] QUICK REVISION OF SDK
3] WHAT's NEW ON 1.0.9?
4] NEW CONSOLE VARS
5] WHAT's LEFT?
6] LICENSE DETAILS
7] FINAL WORDS

1 - LIST OF FEATURES AND DESCRIPTION OF AM

[Description of HL:Arrangement]

Arrangement, a Half-Life 1 Modification

Forget about all minor modifications you played. 

Arrangement let's you feel in the skin of Fred Thompson when he starts to work for "The Agency", a secret organization to protect the world against terrorists groups.

When he accept the job, the missions starts to be part of his life. Playing as Fred you will meet a lot of new scenaries in lot of places in the world.

Our codders works hard to try to change some things to make bigger and detailed maps. On Am, a lot of props are used, more than a normal Half-Life map can support. Almost forget, on Am you will see realistic rain, snow, fog, dust and new effects that you never seen. Thanks that, our mappers can make a very detailed maps to give you a very realistic an inmersive game.

For mission purposes, you will use a lot of equipment, such as Lockpicks, Pistols, Large guns, grenades, Syringes, Flashlights, NVG, Parachutes, vehicles... and more... Each weapon has a secondary fire for add silencers, turn on termal vision, zoom, scope and more.

Arrangement mix a real FPS whit a RPG style. So that, you can earn XP points when you kill some enemies. Of course, headshots scores better than a simple kill, and a burned enemy scores better than a enemy killed by a headshot and so on... Those XP points will let you use new abilities, buy weapons, make kung-fu kicks, and more...

Am also includes new technological improvements. You can turn on/off whenever you want to best performance (by Advanced Menu).
S2P Physics let's you fell how the enemies react very well when you shoot at them. Any weapon or item will fall and bounce in a realistic manner. It also align monsters and weapons to floor, prevent "model clip" when you kill them, simulate GTA:SA ragdoll and much more.

BG Particle-Grass system it's a new concept of effects in-game. Thanks that you can see realist fire, smoke, snow, blood, wall puffs, fog and so on… You can add those particle managers whit just a simple entity and you can edit each param in-game! It means you can test so easily your particle system fast and so better. You can also increase-decease the amount of particles accesing a Cvar command (for best qualitiy-performance)

For C-17 maps your can add HL2 Monster entities and weapons too. All new weapons, effects, sounds and more are avaiable for you, if you want to create a new SP game.

Arrangement it's also better to mapping purposes. Add new prop entitites it's very easy. You can also select your own model for each weapon, item, and monster enemies. There's included all Spirit of Half-Life improvements (not Movewhit or Locus system by now) as well. It's so easy to map. Thanks to VHE 3.5 we can show models on map and create a new concept of mapping.

But that's not the best part. You have the code avaiable to fit your own needs. Instead of add AGEiA Physics and all those graphical improvements, we worked hard to try make a new mod whit all you want: new gameplay, new weapons, all common enemies and allies and new effects.

If you're codder and you have a good modeller and a good mapper, I'm sure you can make a new very impressive Mod so easily. 

Go ahead and read what you can do!

[List of Features]

-31 New weapons with silencers, zoom, fire modes and more.
-New Enemies.
-New Allies.
-Included Stealth or Dynamic Mode. You can hide in shadows.
-SVS Night Vision or Infrared Vision.
-Rain, Dust, Snow, effects Customizable per Map (made by G-Cont)
-Fully working BG Particle system
-MP3, OGG Player
-Briefing System (you can add your own text in a message box and see it in the game pressing B)
-Functional Half-Life 2 Enemies and Allies in Half-Life 1 Engine.
-Experience Points: When you kill a monster you gain score (depending the style of death)
-Widescreen Style for Cut-Scenes
-S2P Physics (c), the best physics in Half-Life without any external Physics core.
-First Person Camera Improvement: You can see your own body and make kung fu attacks.
-You can start dialogues whit any person and select your responses.
-Improvement in models entities to keep good framerates.
-New gameplay.

-Spirit Entities:

-ambient_fmodstream
-env_sky
-hud_sprite
-monster_generic
-monster_generic_dead
-trigger_command
-trigger_changecvar
-trigger_changevalue
-trigger_onsight
-trigger_startpatrol
-player_weaponstrip
-func_shine

-Modified Spirit Entities:

-env_fog (totaly workeable under STEAM platform)
-env_rain (splashes added)
-env_model (added Field of View, and Level of Detail System -for map improvements-)

New Arrangement Entites:

-env_sprite_grass (a single grass sprite with Field of View, and Level of Detail System -obsolete now-)
-monster_zombie_barney
-monster_zombie_soldier
-monster_zombie_human
-monster_robot
-monster_male_assassin
-monster_shocktrooper
-monster_bird (a lot of birds)
-monster_gonome
-monster_hgrunt_ally
-monster_scientist_crowbar (scientist armed with crowbar)
-monster_mini_gargantua (like svencoop)
-monster_hgrunt_opfor (human grunts of OP4 -new weapons and bodygroups-)
-monster_pit_drone
-monster_otis
-monster_handgrenade (a live hand grenade)
-trigger_displaymenu (display a weapon menu selection)
-trigger_dialogue (you can start a dialogue with persons and choose responses)
-trigger_changebrief_info (change Brief info and displays the window)
-func_phy (object with simple physics)
-func_semtex_zone (a zone for detonate your semtex -like CounterStrike style-)
-func_wall_detail (func_wall entity with steps of detail -Low, normal, High detailed-)
-info_savetitle (entity to change your game's savegame -don't work for now-)
-xp_point_gold (adds 10 XP points)
-xp_point_plate (adds 5 XP points)
-xp_point_brown (adds 1 XP point)

New 1.0.9 entities (+ added * modified)

*monster_he_grenade (Live Handgrenade) 
+monster_smoke_grenade (Live Handgrenade) 
+monster_terrorist (A smart terrorist that can shoot 7 different weapons and more -check the source for details-)
+monster_agency_member (A smart Ally that can shoot 7 different weapons and more -check the source for details-)
+func_grass (Area Grass)
+env_particleemitter (Particle System Emitter)
+trigger_sequence (entity to use .seq file for STEAM)
+trigger_displaymenu (Wep Menu Selection)
+trigger_dialogue (dialogue whit others)
+trigger_dialogcheck (checks the points of the dialogue)
*trigger_random (Trigger random) 
+trigger_xppoints (add or remove xp points)
+trigger_check_cvar (check a cvar and modify it)
*trigger_sunflare (fully working now on save/restore)

]Prop Entities

+prop_cable (Cables whit different sizes, types, colors and animations)
+prop_cable_small
+prop_bugs (go ahead and check it)
+prop_palm01
+prop_palm02
+prop_palmC01
+prop_bush01
+prop_bush02
+prop_fern01


New Arrangement Entites from Half Life 2:

-npc_antlion
-npc_combine
-npc_combine_metrocop
-npc_combine_supersoldier 
-npc_combine_random
-npc_combine_helicopter
-npc_combine_turret
-npc_vortigaunt
-npc_zombie_fast
-npc_zombie


2 - QUICK REVISION OF SDK

The SDK (Standart Development Kit) only contains the Source Code of the 1.0.8 Version made in a Half-Life SDK 2.3.

The main "problem" (because Arrangement's Source Code was not free to public, at first) it's the contents of the Code. In some parts the code  seems to be "hard-coded" (make things work in the bad way). There are a lot of messages called and I used some cvars to interact with client side, in Player.cpp - Prethink().

You will see some comments in the Source Code, many of those comments seems to be in Spanish (sorry). After think on open my Source Code I start to write the new comments in English.

Remember to hit ALT+F7 if you want to change your compiler's options (.dll paths and so on).

3 - WHAT's NEW ON 1.0.9?

- New commented code.
- New folders and reorganization of files.
- Each file contains description of how it's changed (server side only).
- New BG Particle system (used for decals, explosion, blood bursts, smoke and more).
- New entities and effects added.
- New improvements on AI.
- Most important bugs removed.
- Code improvements.


4 - WHAT's LEFT?

Thanks to the people who help me in the previous version, there is not much to add. 

However we still need:

-Finish some actual code in the Mod. The developers can see what's left typing "//to do" (without quotes) at the "Search" window.
-We added new schedules and we make new smart enemies (like monster_terrorist). Anyway we stiill need to add shoot&run improvement.
-Add 3D Sound, Shaders, Mirrors and real-time monitors. We have fully working source for that (thanks developers!) but is not integrated on Arrangement. If you want to cooperate whit us, we can send it those packages to you, if you want to add them into Arrangement's source code.
-Play .AVI files in game. Just before release this new version, I fully linked Avikit for that purpose, but I need to create a .AVI loader and a new entity to use it easily on hammer. It doesn't seems to be difficulty just glGenTextures, glBindTexture, glEnable, glTexImage2D and finally FSOUND_Init.

5 - NEW CONSOLE VARS

[CLIENT CONVARS]

cl_detailfire   	:       	1|0 		-Uses a detailed fire (more effects).
cl_detailmuzz   	:       	1|0 		-Uses a detailed muzzleflash (more effects).
cl_detailsparks 	:       	1|0 		-Uses a detailed spark (more effects).
cl_drawprops    	:       	1|0 		-Draw|Hide prop entities (restart required).
cl_expdetail    	:       	0|1|2 		-Normal explosion, High explosion, Particle based explosion
cl_grassamount  	:			0-9999 	-% of Grass Particles (restart required)
cl_gunsmoke     	:       	1|0 		-Draw|Hide gun's smoke
cl_nvghires     	:       	1|0 		-Uses a detailed NVG (more effects)
cl_particlecount 	:			0-9999 	-% of Particles -restart required-
cl_particlesorts 	:			1|0 		-Light parse.
cl_playmusic    	:       	1|0 		-Plays random music (restart required).
cl_pulso        	:       	1|0 		-Idle momement to view.
cl_rollangle    	:       	0-9999 	-View roll angle.
cl_rollspeed    	:     		0-9999 	-View roll speed.
cl_shelllife    	:       	0-9999 	-Shell life.
cl_showplayer   	:       	1|0 		-Draw|Hide Player's body.
cl_wallpuff     	:       	1|0 		-Draw|Hide Wall puff (don't work I think).
cl_weather      	:       	1|0 		-Draw|Hide Weather effects (rain,snow,dust).

[SPECIAL DEVELOPERS CONVARS]

dev_light       	:       	1|0 -Debug Light Detect system.
cl_particledebug	:      		1|0 -Debug particles.
cl_raininfo     	:       	1|0 -Debug rain.
cl_removeclienteffects :		1|0 -Removes all client side based effects (aren't required).

[HUD MODIFIERS CONVARS]

hud_blue        	:      0-255 -HUD blue amount.
hud_gren			:		0-255 -HUD green amount.
hud_newcross    	:      0|1|2 -Normal crosshair, dynamic crosshair, beta crosshair.
hud_red         	:      0-255 -HUD red amount.

[GAMEPLAY CONVARS]

mp_am_weaponweight		:    1|0 -Uses or not weapon weight system.

[S2P Physics CONVARS]

phys_avelocity 			:		0.800
phys_enable    			:			1
phys_flyattack 			:       	1
phys_friction  			:   	0.800
phys_gravity   			:    	0.800
phys_movecorpses 		:			0
phys_movehit    		:       	0
phys_normalize_angles 	: 			1
phys_objects    		:       	1
phys_simulateragdoll 	:  			1
phys_velocity   		:    	0.800

[RENDER CONVARS]

r_glow          	:        0|1|2 	-Off, High Bloom, Low end Bloom.
r_glowblur      	:        6
r_glowdark     		:        4
r_glowstrength  	:        0
r_paintball     	:        1|0 	-Changes all weapon's decals by paint (used on hammer on training map).
r_particles     	:        1|0 	-Turn on/off Particle Based systems.
r_shadows     		:        1|0 	-Turn on/off shadows.

[MUZZLEFLASH MODIFIERS CONVARS -cl_detailmuzz required-]

zz_alpha        	:      	1
zz_fadespeed   		:    	10
zz_framerate    	:     	10
zz_life         	:    	0.010

[NOT REGISTERED CONVARS -used by code and mapping stuff-]

xp_points		-Holds in a .cfg the amount of XP Points.
stealth			-Turn on/off Stealth Mode.
reinit_particles-Re init particle system.
nvgmode			-Changes NVG Mode.
firemode		-Switch weapon fire mode.
help			-Shows message's version.
brief			-Shows Brief window.
select1			-Dialogues handling (it must be deleted and make a decent keyboard check)
select2			-Dialogues handling (it must be deleted and make a decent keyboard check)
select3			-Dialogues handling (it must be deleted and make a decent keyboard check)
parachute		-Open/Close parachute.
showmap			-Fires a entity called "camera_map" on the map
fire			-Classic SoHL command. Fires the entity you are looking at.
dame			-Replaces "give" command.
drop			-Drop your weapon.
playaudio		-OverGround MP3/OGG player.
complain		-Player's voice for try to surrender terrorists.
flashlight		-Selects flashlight weapon.
indream			-Convar to use in final stage (to be used on hammer).
slowmotion		-Fires Slow motion (is not fully working now so don't try to use it).
radar			-Shows a radar that detect entities.
nvg				-Turn on/off Special Googles.
menubackground	-Shows a simple image.
cinebars		-For widescreen effects.
Score			-Shows a message whit monsters killed, n° of headshots and more.
clear_score		-Clears total score (used on hammer when you starts a new game).
wepselect		-Shows a weapon menu (used on hammer when you start a mission -not in BlackMesa-).
radio_backup	-Calls for help (your ally members will look for you)

6 - LICENSE DETAILS

You are free to copy, distribute or modify the Source Code following your needs. You can use, of course, modified weapons and models to make your own Mod using Arrangement's Code. If you want to change something, there is the source code avaivable for you.

YOU NOT ARE ALLOWED TO USE OUR SOURCE CODE IN ANYTHING RELATED TO ARRANGEMENT's HISTORY LINE.
Copyright © 2004-2007 AR Software. All rights reserved. ArrangeMode, the ArrangeMode logo, Arrangement, the Arrangement logo, S2PPhysics, the S2PPhysics logo are either registered trademarks or trademarks of AR Software in Argentina and/or other countries. Spirit of Half-Life, BattleGrounds, OverGround, HL:Town, Half-Life, Half-Life 2, WebPlace and all other trademarks are the property of their respective owners.
Half-Life: Arrangement uses Spirit Of Half-Life (c) Source Code. See http://spirit.valve-erc.com for details.

7 - FINAL WORDS

Well, as I said, thanks to Laurie Cheers and all the people of Spirit of Half-Life forums. Really thanks of all of them.

I also want to say thanks to the following:

-The people who email me to say me things about the proyect.
-The persons who want to help me before and the new mappers, modellers and codders who want to join us. Thanks you very much.
-The persons who want I join in their proyects. Sorry boys. I'd like to help every one but I'm need to finish Am as soon as possible. I can cooperate anyway, so keep mailin' me! I never said "no".
-Phillip Marlowe (www.planetphillip.com) for trust in us.
-WebPlace (for provide Hardware -mmm... thanks!-)

Thanks to all the people that make good or bad comments about the Mod. They help us to make a better Mod.

Right. The Code it's here, ready to use for every one. Use at your own needs.

Remember one more time: We still need voice artists and a few black mesa maps. If you want to cooperate whit us (in any way), contact me at: Sysop_Axis@hotmail.com and you will discover the true meaning of Half-Life: Arrangement.

SysOp, Leader of Half-Life: Arrangement Proyect.
*/

















