/****
*
*  Over Ground (2002-2003) - Mod pour Half-Life - SDK
*
*  Code source de Tigerknee (tigerknee@voila.fr)
*  Plus d'infos sur le site internet du mod :
*  http://og.portailmods.com
*  
****/

#ifndef MP3_H
#define MP3_H

#include "fmoddyn.h"
#include "windows.h"

#define FSOUND_DEFAULT	( FSOUND_NORMAL | FSOUND_LOOP_OFF )
#define ARRAYSIZE(p)		(sizeof(p)/sizeof(p[0]))

typedef struct mp3_s
{

	char name[256];

} mp3_t;

signed char F_CALLBACKAPI endcallback( FSOUND_STREAM *stream, void *buff, int len, int param );

class CMP3
{
private:

	FSOUND_STREAM	*m_Stream;

	int				m_iIsPlaying;
	int				m_channel;

public:

	FMOD_INSTANCE	*m_hMod;

	void			Initialize( void );
	void			Shutdown( void );
	void			PlayMP3( const char *pszSong, int playlisttrack );
	void			StopMP3( void );
	void			StopMusic( void );
	void			Volume( void );

	void			PlayFile( const char *file, int flags, int volume );

	void			RandomMusicPlay	( void );
	void			CreatePlaylist	( void );
	void			NextSong		( void );
	void			RemoveCurrent	( void );

	bool			state;

	mp3_t			playlist[256];
	int				nbtrack;
	int				currenttrack;
};

extern CMP3 gMP3;

#endif