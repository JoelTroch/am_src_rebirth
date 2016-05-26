#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <stdio.h>

//SysOp: Money, Score, XP Points, what's the difference? :D
#define MIN_MONEY 0
#define MAX_MONEY 99999

DECLARE_MESSAGE(m_Money, Money)

int CHudMoney::Init(void) 
{
    HOOK_MESSAGE(Money); 
    gHUD.AddHudElem(this);

    return 1;
};

int CHudMoney::VidInit(void)
{
	m_hSprite1 = 0;
    return 1;
};

int CHudMoney::MsgFunc_Money(const char *pszName,  int iSize, void *pbuf )
{
    BEGIN_READ(pbuf, iSize);
     
      Money = READ_SHORT();

    m_iFlags |= HUD_ACTIVE; 
  
    return 1;
}

int CHudMoney::Draw(float flTime)
{
	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))//fux
	return 1;

	int r, g, b, x, y;
    r = 255;
    g = 255;
    b = 255;

	x = ScreenWidth / CVAR_GET_FLOAT( "cl_hud_x" );
	y = ScreenHeight / CVAR_GET_FLOAT( "cl_hud_y" );

// Load radar background sprite
	if (!m_hSprite1)
	{
		m_hSprite1 = LoadSprite("sprites/hud_xp.spr"); // quick and nasty way of displaying a sprite - you should precache instead
	}
	SPR_Set( m_hSprite1, 255, 255, 255);	
	SPR_DrawAdditive( 0, x, y, NULL);
// Load radar background sprite


    if (Money < MIN_MONEY)
        Money = MIN_MONEY;
    else if (Money > MAX_MONEY)
        Money = MAX_MONEY;

    char string[5];


    _itoa(Money, string, 10);

//backup
//	char *text = CHudTextMessage::BufferedLocaliseTextString( "#XP_LABEL" );

//	gHUD.DrawHudString(ScreenWidth-220, ScreenHeight/2-300, ScreenWidth, text, r, g, b);
//	gHUD.DrawHudString(ScreenWidth-50, ScreenHeight/2-300, ScreenWidth, string, r, g, b);

	gHUD.DrawHudString(x, y, ScreenWidth, string, r, g, b);

	return 1;
}
