/****
*
*  Over Ground (2002-2003) - Mod pour Half-Life - SDK
*
*  Code source de Tigerknee (tigerknee@voila.fr)
*  Plus d'infos sur le site internet du mod :
*  http://og.portailmods.com
*  
****/

#include "hud.h"
#include "cl_util.h"
#include "fmoddyn.h"

#include <io.h>

extern		vec3_t		v_angles;
extern		vec3_t		v_origin;

signed char F_CALLBACKAPI endcallback( FSOUND_STREAM *stream, void *buff, int len, int param )
{
	if ( gMP3.state )
	{
		// On arrête le stream en cours
		gMP3.StopMusic();

		// Pour changer de musique
		gMP3.NextSong();

		return true;
	}
	
	else

		return false;
}

void CMP3::Initialize( void )
{
	char	dllname[ 256 ];

	sprintf( dllname, "%s\\cl_dlls\\fmod.dll", gEngfuncs.pfnGetGameDirectory() );

	m_hMod = FMOD_CreateInstance( dllname );

	m_iIsPlaying = 0;

	memset( playlist, 0, sizeof( playlist ) );
	nbtrack			= 0;
	currenttrack	= -1;
	state			= false;

	m_hMod->FSOUND_SetOutput			( FSOUND_OUTPUT_DSOUND );
	m_hMod->FSOUND_SetDriver			( 0 );
	m_hMod->FSOUND_Init					( 44100, 1, 0 );//32 channels???
}

void CMP3::Shutdown( void )
{
	StopMP3();
	m_hMod->FSOUND_Close();
	
	FMOD_FreeInstance( m_hMod );
}

void CMP3::StopMP3( void )
{
	state	= false;
	StopMusic();
}

void CMP3::StopMusic( void )
{
	if ( m_iIsPlaying )
	{
		m_iIsPlaying = 0;
		m_hMod->FSOUND_Stream_Close( m_Stream );
	}
}

void CMP3::PlayMP3( const char *pszSong, int playlisttrack )
{	
	if ( !state )
		CreatePlaylist();

	if ( m_iIsPlaying )
	{
		if ( strcmp( pszSong, "" ) == 0 )
		{
			// On arrête la musique en cours
			StopMusic();

			// Et on la retire
			RemoveCurrent();
		}
		else

			// Sinon on arrête tout
			StopMP3();
	}

	if ( strcmp( pszSong, "" ) == 0 )
	{
		// Si y'a aucun paramètre alors on joue une musique au hasard
		RandomMusicPlay();
		return;
	}

	else
	{
		char song[256];

		sprintf( song, "%s/sound/fmod/%s", gEngfuncs.pfnGetGameDirectory(), pszSong );

		if ( playlisttrack < 0 )
			m_Stream = m_hMod->FSOUND_Stream_Open( song, FSOUND_NORMAL | FSOUND_LOOP_NORMAL, 0, 0 );
		else
		{
			m_Stream = m_hMod->FSOUND_Stream_Open( song, FSOUND_NORMAL | FSOUND_LOOP_OFF, 0, 0 );
			m_hMod->FSOUND_Stream_SetEndCallback( m_Stream, endcallback, 0 );
		}

		if( m_Stream )
		{
			m_channel = m_hMod->FSOUND_Stream_Play( FSOUND_FREE, m_Stream );

			state			= true;

			m_iIsPlaying	= 1;
			currenttrack	= playlisttrack;
		} 
		else
		{
			gEngfuncs.Con_Printf( "Aucun fichier son %s\n", song );
			m_iIsPlaying = 0;
		}
	}
}

void CMP3 :: Volume( void )
{
	// Le joueur peut régler le volume des MP3 directement depuis le menu de Steam,
	// on s'arrange pour changer le volume des musique en fonction de cette cvar
	if ( m_iIsPlaying )
		m_hMod->FSOUND_SetVolume( m_channel, CVAR_GET_FLOAT( "MP3Volume" ) * 100 );
}

void CMP3 :: CreatePlaylist( void )
{
	struct		_finddata_t music_file;
	long		hFile;
	char		media[512];
	int			i, j;

	char	*mediatype[]	=	{
									"wav",
									"mp2",
									"mp3",
									"ogg",
								};

	i = 0;
	 
	for ( j = 0; j < ARRAYSIZE( mediatype ); j++ )
	{
		sprintf( media, "%s/sound/fmod/*.%s", gEngfuncs.pfnGetGameDirectory(), mediatype[j] );

		// On cherche le premier et ensuite on cherche tous les autres
		if( ( hFile = _findfirst( media, &music_file ) ) == -1L )
		{
			// Rien trouvé
			;
		}
		else
		{
			// On stocke la première musique dans le tableau
			strcpy( playlist[i].name, music_file.name);

			i++;

			if ( i >= ARRAYSIZE( playlist ) )
				break;

			// On cherche le reste des fichiers
			while( _findnext( hFile, &music_file ) == 0 )
			{
				strcpy( playlist[i].name, music_file.name );

				i++;

				if ( i >= ARRAYSIZE( playlist ) )
					break;
			}

			_findclose( hFile );
		}
	}

	// On copie la playlist
	nbtrack		= i;
}

void CMP3 :: RandomMusicPlay( void )
{
	int	random;

	if ( nbtrack >= 1 )
	{
		// Parfait on a notre tableau maintenant avec toutes nos musiques
		// On va choisir une musique au hasard
		random = gEngfuncs.pfnRandomLong( 0, ( nbtrack - 1 ) );

		// On joue la chanson
		PlayMP3( playlist[random].name, random );
	}
	else

		gEngfuncs.Con_Printf( "Aucun fichier son dans le repertoire music\n" );
}

void CMP3 :: RemoveCurrent ( void )
{
	if ( currenttrack < 0 )

		return;

	// Une musique de moins
	nbtrack--;

	// On a le numéro de la musique dans la playlist, on va l'enlever
	for ( int i = 0; i <= nbtrack; i++ )
	{
		if ( ( i >= currenttrack ) && ( i < ( ARRAYSIZE( playlist ) - 1 ) ) )

			playlist[i] = playlist[(i+1)];

		else if ( i == ( ARRAYSIZE( playlist ) - 1 ) )

			strcpy( playlist[i].name, "" );
	}

	// On verifie si y'a toujours des musiques dans la playlist
	// sinon on en recréer une
	if ( nbtrack <= 0 )
		CreatePlaylist();
}

void CMP3 :: NextSong ( void )
{
	// On enlève la musique que l'on vient de jouer
	RemoveCurrent();

	// On joue une autre chanson au hasard maintenant
	RandomMusicPlay();
}

void CMP3 :: PlayFile( const char *file, int flags, int volume )
{
	FSOUND_STREAM	*m_sound;
	char			song[ 256 ];
	int				channel_used;

	sprintf( song, "%s/sound/fmod/%s", gEngfuncs.pfnGetGameDirectory(), file );

	m_sound		= m_hMod->FSOUND_Stream_Open( song, FSOUND_NORMAL | flags, 0, 0 );

	if( m_sound )
	{
		channel_used = m_hMod->FSOUND_Stream_Play( FSOUND_FREE, m_sound );
		m_hMod->FSOUND_SetVolume( channel_used, volume );
	}

	else

		gEngfuncs.Con_DPrintf( "Failed to play sound %s\n", file );
}
