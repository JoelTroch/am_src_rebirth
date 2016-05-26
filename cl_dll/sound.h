//===================================
//
//		SOUND ENGINE v1 by fo0z
//
//===================================

#include "fmod.h"

void SoundInit( void );
void SoundShutDown( void );

void SoundUpdate( void );
void SoundUpdateSample( void );

void SoundPlay( vec3_t origin, char *soundname, int vol, int pitchscale );
void SoundDynPlay( vec3_t origin, char *soundname );
void SoundEntPlay( int entindex, char *soundname, int vol, int pitchscale );

typedef struct sample_s
{
	int channel;
	int entindex;
	FSOUND_SAMPLE *sample;
} sample_s;
