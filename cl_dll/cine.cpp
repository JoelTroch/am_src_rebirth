#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "event_api.h"
#include "vgui_TeamFortressViewport.h"

#include "triangleapi.h"

//TO DO: Need I to use sprites? Draw it using triAPI?

DECLARE_MESSAGE(m_Cine, CineBars )

int CHudCine::Init(void)
{
	HOOK_MESSAGE( CineBars );

	m_iOn = 0;

	m_iFlags = 0;

	gHUD.AddHudElem(this);

	return 1;
}


int CHudCine::VidInit(void)
{
//	m_hFlickerCine = LoadSprite("sprites/nvg_noise.spr");

	return 1;
}

int CHudCine::Draw(float fTime)
{
	/*
	if (m_iOn)
	{
		int x, y, w, h;
		int frame;

		SPR_Set(m_hFlickerCine, 50, 50, 50 );// COLOR

		// play at 15fps
		frame = (int)(fTime * 15) % SPR_Frames(m_hFlickerCine);

		w = SPR_Width(m_hFlickerCine,0);
		h = SPR_Height(m_hFlickerCine,0);

		for(y = -(rand() % h); y < ScreenHeight; y += h) 
		{
			for(x = -(rand() % w); x < ScreenWidth; x += w) 
			{
				SPR_DrawAdditive( frame, x, y, NULL );
			}
		}
			
	//	 gViewPort->ShowCine();
	}
	else
	{
	//	gViewPort->HideCine();
	}
	*/
	return 1;
}

int CHudCine::MsgFunc_CineBars(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	m_iOn = READ_BYTE();
	
	if (m_iOn==1)
	{
		m_iFlags |= HUD_ACTIVE;
	//	gViewPort->ShowScope();
		gViewPort->ShowCine();
	}
	else
	{
		m_iFlags &= ~HUD_ACTIVE;
	//	gViewPort->HideScope();
		gViewPort->HideCine();
	}
	

	return 1;
}
void CHudCine::Reset(void)
{
	m_iOn = 0;
}

