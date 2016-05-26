#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CRicardoPanel::CRicardoPanel(int x,int y,int wide,int tall) : Label("",0,0,ScreenWidth,ScreenHeight)
{
   char sz[64];

   sprintf( sz, "%i_menu", ScreenWidth );

   Ricardo = LoadTGANoRes( sz );
}

void CRicardoPanel::Update()
{
   setImage( Ricardo );
}

