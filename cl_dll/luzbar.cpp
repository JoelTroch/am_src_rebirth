#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>


//***********************
#include <stdio.h>
#include <windows.h>
#include <gl/gl.h>
//***********************

//TO DO: hard coded file. I need to add the new sprites in HUD.txt. 
//If anybody can correct this, okey. I made some improvements in this code, but I haven't time now.

DECLARE_MESSAGE(m_LuzBar, LuzBar)
//DECLARE_MESSAGE(m_Flash, Flashlight)

int CHudLuzBar::Init(void)
{
	m_fFade = 0;
	m_fOn = 0;

//	HOOK_MESSAGE(Flashlight);
	HOOK_MESSAGE(LuzBar);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
};

void CHudLuzBar::Reset(void)
{
	m_fFade = 0;
	m_fOn = 0;
}

int CHudLuzBar::VidInit(void)
{
	int HUD_flash_empty = gHUD.GetSpriteIndex( "luz_vacia" );
	int HUD_flash_full = gHUD.GetSpriteIndex( "luz_llena" );
	int HUD_flash_beam = gHUD.GetSpriteIndex( "luz_beam" );

	m_hSprite1 = gHUD.GetSprite(HUD_flash_empty);
	m_hSprite2 = gHUD.GetSprite(HUD_flash_full);
	m_hBeam = gHUD.GetSprite(HUD_flash_beam);
	m_prc1 = &gHUD.GetSpriteRect(HUD_flash_empty);
	m_prc2 = &gHUD.GetSpriteRect(HUD_flash_full);
	m_prcBeam = &gHUD.GetSpriteRect(HUD_flash_beam);
	m_iWidth = m_prc2->right - m_prc2->left;

	m_hLuzBack = 0;
	m_hHealthArmor = 0;
	m_hHudTalk = 0;
	m_hHudHandWeapon = 0;
	m_hHudHand = 0;

	return 1;
};

int CHudLuzBar:: MsgFunc_LuzBar(const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x)/100.0;

	return 1;
}
int CHudLuzBar::Draw(float flTime)
{
	//sys add:
//	return 0;

//******
	/*
		glDisable(GL_TEXTURE_2D);
		glBlendFunc( GL_DST_COLOR, GL_SRC_COLOR );
		glEnable( GL_BLEND );
		glDepthMask(GL_FALSE);
  
		glBegin( GL_QUADS );
		
		glColor4f(0.5,0.5,0.5,10.0);

		//GL_RGBA
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 0, 0, 512, 512, 0);

		glVertex2f(0.0,0.0);
		glVertex2f(ScreenWidth,0.0);
		glVertex2f(ScreenWidth,ScreenHeight);
		glVertex2f(0.0,ScreenHeight);

		glEnd();

		glDepthMask(GL_TRUE);
		glEnable(GL_TEXTURE_2D);
		*/
//*******

//	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_FLASHLIGHT | HIDEHUD_ALL ) )
//		return 1;

	int r, g, b, x, y, a;
//	int ha_x, ha_y;
	wrect_t rc;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;

	if (m_fOn)
		a = 225;
	else
		a = 225;
//		a = MIN_ALPHA;

	y = ScreenWidth / (CVAR_GET_FLOAT( "cl_luzbar_y" ));
	x = ScreenHeight / (CVAR_GET_FLOAT( "cl_luzbar_x" ));

	//si es mayor al permitido, poner en rojo y avisar
	if (m_flBat > 0.20)//20
	{
		UnpackRGB(r,g,b, RGB_REDISH);
//		gHUD.DrawHudString( x, y, ScreenWidth/2, "Visible", 155, 0, 0 );
//		gHUD.DrawHudString( 10, 10, ScreenWidth/2, "Visible", 155, 0, 0 );

	}
	else
	{
//		gHUD.DrawHudString( x, y, ScreenWidth/2, "Oculto", 200, 200, 200 );
//		gHUD.DrawHudString( 10, 10, ScreenWidth/2, "Oculto", 200, 200, 200 );
		UnpackRGB(r,g,b, RGB_YELLOWISH);
	}

	ScaleColors(r, g, b, a);

	//DRAW CROSSHAIR AT CENTER

	//resolutions hack
	/*
	if ( ScreenWidth = 640 && ScreenHeight = 480 )
	if ( ScreenWidth = 800 && ScreenHeight = 600 )
	if ( ScreenWidth = 1024 && ScreenHeight = 768 )
	*/
	int xpos = ScreenWidth / 2; //One forth across the screen. Useful in the case of various screen resolutions.
	int ypos = ScreenHeight / 2; //Very middle of the screen.

	//edit... -32 respecto a resolucion?
	xpos -= 32;//centered
	ypos -= 32;

	if (!m_hHealthArmor)
	{
		m_hHealthArmor = LoadSprite("sprites/hud_cross.spr"); // quick and nasty way of displaying a sprite - you should precache instead
	}

	if (CVAR_GET_FLOAT("cl_crosshair_ontarget") == 1)//is on target? right! draw in red color!			
	SPR_Set( m_hHealthArmor, 255, 0, 0);	
	else
	SPR_Set( m_hHealthArmor, r, g, b);	

	if (CVAR_GET_FLOAT("hud_newcross_size") != 0)
	if ( CVAR_GET_FLOAT( "hud_newcross" ) >= 2 )
	SPR_DrawAdditive( 0, xpos, ypos, NULL);
	

	//DRAW TALK SPRITE
	//128 x 64 size

	//position params
//	xpos = ScreenWidth / 2;//centered?
		
	xpos = ScreenWidth / 45;
	ypos = ScreenHeight / 15;//below stealth bar

//	xpos -= CVAR_GET_FLOAT( "cl_hud_x" );//centered
//	ypos -= CVAR_GET_FLOAT( "cl_hud_y" );

	if (!m_hHudTalk)
	{
		m_hHudTalk = LoadSprite("sprites/hud_talk.spr"); // quick and nasty way of displaying a sprite - you should precache instead
	}

	if (CVAR_GET_FLOAT("neartalk") == 1)
	{
		SPR_Set( m_hHudTalk, r, g, b);	
		SPR_DrawAdditive( 0, xpos, ypos, NULL);
	}

	//DRAW WEAPON SPRITE
	if (!m_hHudHandWeapon)
	{
		m_hHudHandWeapon = LoadSprite("sprites/hud_handwep.spr"); // quick and nasty way of displaying a sprite - you should precache instead
	}

	if (CVAR_GET_FLOAT("nearweapon") == 1)
	{
		if (CVAR_GET_FLOAT("neartalk") == 0)//only draw if the player is not near to dialogue
		{
			SPR_Set( m_hHudHandWeapon, r, g, b);	
			SPR_DrawAdditive( 0, xpos, ypos, NULL);
		}
	}

	//DRAW USE ONLY SPRITE
	if (!m_hHudHand)
	{
		m_hHudHand = LoadSprite("sprites/hud_hand.spr"); // quick and nasty way of displaying a sprite - you should precache instead
	}

	if (CVAR_GET_FLOAT("nearbutton") == 1)
	{
		if (CVAR_GET_FLOAT("nearweapon") == 0)//only draw if the player is not near to dialogue, and some weapon
		if (CVAR_GET_FLOAT("neartalk") == 0)
		{
			SPR_Set( m_hHudHand, r, g, b);//255	
			SPR_DrawAdditive( 0, xpos, ypos, NULL);
		}
	}


// Load radar background sprite
	if (!m_hLuzBack)
	{
		m_hLuzBack = LoadSprite("sprites/luz_background.spr"); // quick and nasty way of displaying a sprite - you should precache instead
	}
	SPR_Set( m_hLuzBack, r, g, b);	
	SPR_Draw( 0, x, y, NULL);
// Load radar background sprite

	// Draw the flashlight casing
	SPR_Set(m_hSprite1, r, g, b );
	SPR_DrawAdditive( 0,  x, y, m_prc1);

	if ( m_fOn )
	{  // draw the flashlight beam
	//	x = ScreenWidth - m_iWidth/2;
	
		//x = XRES(CVAR_GET_FLOAT( "cl_luzbar_x" ));

		SPR_Set( m_hBeam, r, g, b );
		SPR_DrawAdditive( 0, x, y, m_prcBeam );
	}

	// draw the flashlight energy level
	//x = ScreenWidth - m_iWidth - m_iWidth/2 ;

	//x = XRES(CVAR_GET_FLOAT( "cl_luzbar_x" ));

	int iOffset = m_iWidth * (1.0 - m_flBat);
	if (iOffset < m_iWidth)
	{
		rc = *m_prc2;
//		rc.left += iOffset;
		rc.left += iOffset;

		SPR_Set(m_hSprite2, r, g, b );
		SPR_DrawAdditive( 0, x + iOffset, y, &rc);
	}
	return 1;
}