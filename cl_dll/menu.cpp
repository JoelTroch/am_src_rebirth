/****
*
*  Over Ground (2002-2003) - Mod pour Half-Life - SDK
*
*  Code source de Tigerknee (tigerknee@voila.fr)
*  Plus d'infos sur le site internet du mod :
*  http://og.portailmods.com
*  
****/

#include <string.h>
#include <stdio.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "vgui_TeamFortressViewport.h"

#define MAX_MENU_STRING	1024
char g_szMenuString[MAX_MENU_STRING];
char g_szPrelocalisedMenuString[MAX_MENU_STRING];

int KB_ConvertString( char *in, char **ppout );

DECLARE_MESSAGE( m_Menu, ShowMenu );

int CHudMenu :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( ShowMenu );

	InitHUDData();

	return 1;
}

void CHudMenu :: InitHUDData( void )
{
	m_fMenuDisplayed = 0;
	m_bitsValidSlots = 0;
	Reset();
}

void CHudMenu :: Reset( void )
{
	g_szPrelocalisedMenuString[0] = 0;
	m_fWaitingForMore = FALSE;
}

int CHudMenu :: VidInit( void )
{
	return 1;
}

int CHudMenu :: Draw( float flTime )
{
	bool perso_color = false;

	int tmp1 = 0;
	int tmp2 = 0;
	int tmp3 = 0;

	int r = 255;
	int g = 255;
	int b = 255;

	// check for if menu is set to disappear
	if ( m_flShutoffTime > 0 )
	{
		if ( m_flShutoffTime <= gHUD.m_flTime )
		{  // times up, shutoff
			m_fMenuDisplayed = 0;
			m_iFlags &= ~HUD_ACTIVE;
			return 1;
		}
	}

	// don't draw the menu if the scoreboard is being shown
	if ( gViewPort && gViewPort->IsScoreBoardVisible() )
		return 1;

	// draw the menu, along the left-hand side of the screen

	// count the number of newlines
	int nlc = 0;
	for ( int i = 0; i < MAX_MENU_STRING && g_szMenuString[i] != '\0'; i++ )
	{
		if ( g_szMenuString[i] == '\n' )
			nlc++;
	}

	// center it
	int y = (ScreenHeight/2) - ((nlc/2)*12) - 40; // make sure it is above the say text
	int x = 20;

	i = 0;

	while ( i < MAX_MENU_STRING && g_szMenuString[i] != '\0' )
	{
		if ( g_szMenuString[i] == 92 )
		{
			int j;

			j = i + 1;

			if (g_szMenuString[j] == 67 || g_szMenuString[j] == 99)
			{
				perso_color = true;
				i += 2;
			}
			else
				perso_color = false;

			if (perso_color)
			{
				tmp1 = 0;
				tmp2 = 0;
				tmp3 = 0;

				i++;

				int k;
				k = 0;

				int f;
				f = 0;

				for ( k = 0; k < 3; k++)
				{
					for ( f = 0; f < 3; f++)
					{
						int tmp;

						if (g_szMenuString[i] == 48)
							tmp = 0;
						else if (g_szMenuString[i] == 49)
							tmp = 1;
						else if (g_szMenuString[i] == 50)
							tmp = 2;
						else if (g_szMenuString[i] == 51)
							tmp = 3;
						else if (g_szMenuString[i] == 52)
							tmp = 4;
						else if (g_szMenuString[i] == 53)
							tmp = 5;
						else if (g_szMenuString[i] == 54)
							tmp = 6;
						else if (g_szMenuString[i] == 55)
							tmp = 7;
						else if (g_szMenuString[i] == 56)
							tmp = 8;
						else if (g_szMenuString[i] == 57)
							tmp = 9;
						else
							tmp = 10;

						if (tmp != 10)
						{
							if (f == 0)
								tmp1 = tmp;
							else if (f == 1)
								tmp2 = tmp;
							else
								tmp3 = tmp;
						}

						if (tmp == 10)
						{
							if (f == 0)
							{
								tmp1 = 2;
								tmp2 = 5;
								tmp3 = 5;
							}

							if (f == 1)
							{
								tmp3 = tmp1;
								tmp1 = 0;
								tmp2 = 0;
							}

							if (f == 2)
							{
								tmp3 = tmp2;
								tmp2 = tmp1;
								tmp1 = 0;
							}
						}
						else
							i++;
					}

					if (k == 0)
						r = (tmp1 * 100) + (tmp2 * 10) + tmp3;

					if (k == 1)
						g = (tmp1 * 100) + (tmp2 * 10) + tmp3;

					if (k == 2)
						b = (tmp1 * 100) + (tmp2 * 10) + tmp3;
			
					i++;
				}
			}
			else
			{
				r = 255;
				g = 255;
				b = 255;
			}
		}

		perso_color = false;
		
		if (r > 255)
			r = 255;

		if (r < 0)
			r = 0;

		if (g > 255)
			g = 255;

		if (g < 0)
			g = 0;

		if (b > 255)
			b = 255;

		if (b < 0)
			b = 0;

		if ( g_szMenuString[i] == '\n' )
		{
			x = 20;
			y += 12;
			i++;
		}
		else
		{
			TextMessageDrawChar( x, y, g_szMenuString[i], r, g, b );

			x += 10;
			i++;
		}

	}
	
	return 1;
}

// selects an item from the menu
void CHudMenu :: SelectMenuItem( int menu_item )
{
	// if menu_item is in a valid slot,  send a menuselect command to the server
	if ( (menu_item > 0) && (m_bitsValidSlots & (1 << (menu_item-1))) )
	{
		char szbuf[32];

		sprintf( szbuf, "menuselect %d\n", menu_item );
		ClientCmd( szbuf );

		// remove the menu
		m_fMenuDisplayed = 0;
		m_iFlags &= ~HUD_ACTIVE;
	}
}


// Message handler for ShowMenu message
// takes four values:
//		short: a bitfield of keys that are valid input
//		char : the duration, in seconds, the menu should stay up. -1 means is stays until something is chosen.
//		byte : a boolean, TRUE if there is more string yet to be received before displaying the menu, FALSE if it's the last string
//		string: menu string to display
// if this message is never received, then scores will simply be the combined totals of the players.
int CHudMenu :: MsgFunc_ShowMenu( const char *pszName, int iSize, void *pbuf )
{
	char *temp = NULL;

	BEGIN_READ( pbuf, iSize );

	m_bitsValidSlots = READ_SHORT();
	int DisplayTime = READ_CHAR();
	int NeedMore = READ_BYTE();

	if ( DisplayTime > 0 )
		m_flShutoffTime = DisplayTime + gHUD.m_flTime;
	else
		m_flShutoffTime = -1;

	if ( m_bitsValidSlots && !gViewPort->m_pCurrentMenu && !gViewPort->m_pCurrentCommandMenu )
	{
		if ( !m_fWaitingForMore ) // this is the start of a new menu
		{
			strncpy( g_szPrelocalisedMenuString, READ_STRING(), MAX_MENU_STRING );
		}
		else
		{  // append to the current menu string
			strncat( g_szPrelocalisedMenuString, READ_STRING(), MAX_MENU_STRING - strlen(g_szPrelocalisedMenuString) );
		}
		g_szPrelocalisedMenuString[MAX_MENU_STRING-1] = 0;  // ensure null termination (strncat/strncpy does not)

		if ( !NeedMore )
		{  // we have the whole string, so we can localise it now
			strcpy( g_szMenuString, gHUD.m_TextMessage.BufferedLocaliseTextString( g_szPrelocalisedMenuString ) );

			// Swap in characters
			if ( KB_ConvertString( g_szMenuString, &temp ) )
			{
				strcpy( g_szMenuString, temp );
				free( temp );
			}
		}

		m_fMenuDisplayed = 1;
		m_iFlags |= HUD_ACTIVE;
	}
	else
	{
		m_fMenuDisplayed = 0; // no valid slots means that the menu should be turned off
		m_iFlags &= ~HUD_ACTIVE;
	}

	m_fWaitingForMore = NeedMore;

	return 1;
}
