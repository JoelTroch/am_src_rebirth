#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CCinePanel::CCinePanel(int x,int y,int wide,int tall) : Label("",0,0,ScreenWidth,ScreenHeight)
{
   char sz[64];

   sprintf( sz, "%i_cine", ScreenWidth );

   Cine = LoadTGANoRes( sz );
}

void CCinePanel::Update()
{
   setImage( Cine );
}

