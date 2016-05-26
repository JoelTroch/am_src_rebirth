menu_struct_t TestMenu[] =
{
    { "T e s t M e n u", MENU_ALIGN_CENTER, FALSE, -1 },
    { "", MENU_ALIGN_LEFT, FALSE, -1 },
    { "Please Choose a Team", MENU_ALIGN_CENTER, FALSE, -1 },
    { "Red Team", MENU_ALIGN_LEFT, TRUE, 1 },
    { "Blue Team", MENU_ALIGN_LEFT, TRUE, 2 },
    { "Green Team", MENU_ALIGN_LEFT, TRUE, 3 },
    { "Yellow Team", MENU_ALIGN_LEFT, TRUE, 4 },
    { "", MENU_ALIGN_LEFT, FALSE, -1 },
    { "Use \'[\' and \']\'", MENU_ALIGN_LEFT, FALSE, -1 },
    { "To Move The Selection", MENU_ALIGN_LEFT, FALSE, -1 }
};

#include "hud.h"
#include "util.h"
#include "parsemsg.h"

#include "string.h"
#include "stdio.h"

DECLARE_MESSAGE ( m_NewMenu, MenuOpen );

int CNewMenu::Init ( void )
{
    gHUD.AddHudElem ( this );
   
    HOOK_MESSAGE ( MenuOpen );
   
    InitHUDData ();
   
    return 1;
}

void CNewMenu::InitHUDData ( void )
{
    UnpackRGB ( sRed, sGreen, sBlue, RGB_REDISH );
    UnpackRGB ( Red, Green, Blue, RGB_YELLOWISH );
   
    m_CurrentMenu = NULL;
    m_bMenuDisplayed = FALSE;
    m_bCanCancelMenu = FALSE;
    m_iTotalSlots = 0;
    m_iCurrentSelection = 0;
   
}

void CNewMenu::Reset ( void )
{
   
}

int CNewMenu::VidInit ( void )
{
    return 1;
}

int CNewMenu::Draw ( float flTime )
{
    int x_pos = ScreenWidth / 4;
    int y_pos = (ScreenHeight - (12 * m_iTotalSlots))/2;
   
    int char_widths = gHUD.m_scrinfo.charWidths[' '];
   
    int x_rel;
   
    for ( int i = 0; i < m_iTotalSlots; i++ )
    {
  x_rel = x_pos;
  if ( m_CurrentMenu[i].Align == MENU_ALIGN_CENTER )
  {
    for ( int cs = 0; m_CurrentMenu[i].Text[cs] != '\0'; cs++ ) { }
   
    x_rel = (ScreenWidth - (15*cs))/2;
  }
 
  if ( i == m_iCurrentSelection )
    gHUD.DrawHudString ( x_rel, y_pos, ScreenWidth, m_CurrentMenu[i].Text, sRed, sGreen, sBlue );
  else
    gHUD.DrawHudString ( x_rel, y_pos, ScreenWidth, m_CurrentMenu[i].Text, Red, Green, Blue );
 
  y_pos += 12;
    }
   
    return 1;
}

int CNewMenu::MsgFunc_MenuOpen ( const char *pszName, int iSize, void *pbuf )
{
    BEGIN_READ ( pbuf, iSize );
   
    int menu = READ_BYTE ();
    m_bCanCancelMenu = READ_BYTE ();
   
   
    m_CurrentMenu = TestMenu;
    m_iTotalSlots = 10;
    m_iCurrentSelection = 0;
   
    if ( !m_CurrentMenu[m_iCurrentSelection].Selectable )
  SelectNextOption ();

    // switch ( menu )
    // {
    // case 1:
    // break;
   
    // default:
    // break;
    // }
   
    m_iFlags |= HUD_ACTIVE;
    m_bMenuDisplayed = TRUE;
   
   
    return 1;
}

void CNewMenu::SelectItem ( void )
{
    if ( (m_CurrentMenu) && (m_bMenuDisplayed) && ( m_CurrentMenu[m_iCurrentSelection].Selectable) )
    {
  char cmd[32];
 
  sprintf ( cmd, "menuselect %d\n", m_CurrentMenu[m_iCurrentSelection].ReturnNumber );
 
  ClientCmd ( cmd );
 
  m_CurrentMenu = NULL;
  m_iTotalSlots = 0;
  m_iCurrentSelection = 0;
 
  m_bMenuDisplayed = FALSE;
  m_bCanCancelMenu = FALSE;
 
  m_iFlags &= ~HUD_ACTIVE;
  gHUD.m_iKeyBits &= ~IN_ATTACK;
    }
}

void CNewMenu::SelectNextOption ( void )
{
    if ( m_iCurrentSelection == (m_iTotalSlots - 1) )
  m_iCurrentSelection = 0;
   
    else
  m_iCurrentSelection++;
   
    while ( m_CurrentMenu[m_iCurrentSelection].Selectable != TRUE )
    {
  if ( m_iCurrentSelection == (m_iTotalSlots - 1) )
    m_iCurrentSelection = 0;
 
  else
    m_iCurrentSelection++;
    }
}

void CNewMenu::SelectPreviousOption ( void )
{
    if ( m_iCurrentSelection == 0 )
  m_iCurrentSelection = (m_iTotalSlots - 1);
   
    else
  m_iCurrentSelection--;
   
    while ( m_CurrentMenu[m_iCurrentSelection].Selectable != TRUE )
    {
  if ( m_iCurrentSelection == 0 )
    m_iCurrentSelection = (m_iTotalSlots - 1);
 
  else
    m_iCurrentSelection--;
    }
}
