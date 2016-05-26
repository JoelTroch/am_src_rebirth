#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CScopePanel::CScopePanel(int x,int y,int wide,int tall) : Label("",0,0,ScreenWidth,ScreenHeight)
{
   char sz[64];

   sprintf( sz, "%i_nvg", ScreenWidth );

   Scope = LoadTGANoRes( sz );
}

void CScopePanel::Update()
{
   setImage( Scope );
}

