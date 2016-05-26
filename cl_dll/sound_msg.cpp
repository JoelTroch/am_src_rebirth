//===================================
//
//		SOUND ENGINE v1 by fo0z
//
//===================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"

#include "sound.h"

// fo0z
void CHud :: MsgFunc_FPlaySound( const char *pszName, int iSize, void *pbuf )
{
	vec3_t origin;

	BEGIN_READ( pbuf, iSize );

	origin[0] = READ_COORD();
	origin[1] = READ_COORD();
	origin[2] = READ_COORD();
	int entindex = READ_BYTE();
	char *szSoundname = READ_STRING();
	int vol = READ_BYTE();
	int pitch = READ_BYTE();

	if( entindex > 0 )
		SoundEntPlay( entindex, szSoundname, vol, pitch );	// for dynamique ent sound
	else
		SoundPlay( origin, szSoundname, vol, pitch );	// for static sound

}