
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>


DECLARE_MESSAGE(m_FlashBar, FlashBar)

int CHudFlashBar::Init(void)
{
	m_fFade = 0;
	m_fOn = 0;//always on?

	HOOK_MESSAGE(FlashBar);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
};

void CHudFlashBar::Reset(void)
{
	m_fFade = 0;
	m_fOn = 0;
}

int CHudFlashBar::VidInit(void)
{
	int HUD_flash_empty = gHUD.GetSpriteIndex( "luz_vacia" );
	int HUD_flash_full = gHUD.GetSpriteIndex( "luz_llena" );
	int HUD_flash_beam = gHUD.GetSpriteIndex( "luz_beam" );

	m_hSprite1 = gHUD.GetSprite(HUD_flash_empty);
	m_hSprite2 = gHUD.GetSprite(HUD_flash_full);
	m_hLuzBarBack = 0;

	m_hBeam = gHUD.GetSprite(HUD_flash_beam);
	m_prc1 = &gHUD.GetSpriteRect(HUD_flash_empty);
	m_prc2 = &gHUD.GetSpriteRect(HUD_flash_full);
	m_prcBeam = &gHUD.GetSpriteRect(HUD_flash_beam);
	m_iWidth = m_prc2->right - m_prc2->left;

	return 1;
};

int CHudFlashBar:: MsgFunc_FlashBar(const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_fOn = READ_BYTE();
	int x = READ_BYTE();//fade? amount?
	m_iBat = x;
	m_flBat = ((float)x)/100.0;//100

	return 1;
}

int CHudFlashBar::Draw(float flTime)
{
//sys add:
	return 0;


//	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_FLASHLIGHT | HIDEHUD_ALL ) )
	if ( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL )
		return 1;

	int r, g, b, x, y, a;
	wrect_t rc;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;

	if (m_fOn)
		a = 225;
	else
		a = MIN_ALPHA;

	if (m_flBat < 0.20)
		UnpackRGB(r,g,b, RGB_REDISH);
	else
		UnpackRGB(r,g,b, RGB_YELLOWISH);

	x = ScreenWidth / 2.3;
	y = ScreenHeight / 50;

// Load radar background sprite
	if (!m_hLuzBarBack)
	{
		m_hLuzBarBack = LoadSprite("sprites/luz_background.spr"); // quick and nasty way of displaying a sprite - you should precache instead
	}
	SPR_Set( m_hLuzBarBack, 255, 255, 255);	
	SPR_Draw( 0, x, y, NULL);
// Load radar background sprite

	ScaleColors(r, g, b, a);

	// Draw the flashlight casing
	SPR_Set(m_hSprite1, r, g, b );
	SPR_DrawAdditive( 0,  x, y, m_prc1);
/*
	if ( m_fOn )
	{  // draw the flashlight beam
		//x = ScreenWidth - m_iWidth/2;

		SPR_Set( m_hBeam, r, g, b );
		SPR_DrawAdditive( 0, x, y, m_prcBeam );
	}
*/
	// draw the flashlight energy level
	//x = ScreenWidth - m_iWidth - m_iWidth/2 ;

	int iOffset = m_iWidth * (1.0 - m_flBat);
	if (iOffset < m_iWidth)
	{
		rc = *m_prc2;
		rc.left += iOffset;

		SPR_Set(m_hSprite2, r, g, b );
		SPR_DrawAdditive( 0, x + iOffset, y, &rc);
	}

	return 1;
}