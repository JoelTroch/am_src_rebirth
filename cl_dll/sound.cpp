//===================================
//
//		SOUND ENGINE v1 by fo0z
//
//===================================

#include "hud.h"
#include "cl_util.h"
#include "r_efx.h"
#include "event_api.h"

#include "fmod.h"
#include "fmod_errors.h"
#include "sound.h"
#include "reverb.h"

#define MAX_SAMPLES 128

int freesample = NULL;
//int m_echo;
sample_s g_samples[MAX_SAMPLES];

FSOUND_DSPUNIT    *DrySFXUnit = NULL;

//===================================
//		INITIALIZE SYSTEM
//===================================
void SoundInit( void )
{
    CONPRINT("============================\n");
    CONPRINT("\n");
    CONPRINT("FMOD initialized\n");
    CONPRINT("\n");
    CONPRINT("============================\n");
	 /*
        INITIALIZE
    */
	//FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);
	//FSOUND_SetDriver(0);
	//FSOUND_SetMixer(FSOUND_MIXER_AUTODETECT);
    //FSOUND_SetBufferSize(100);  /* This is nescessary to get FX to work on output buffer */
    if (!FSOUND_Init(44100, 32, 0/*FSOUND_INIT_ENABLESYSTEMCHANNELFX*/))
    {
        gEngfuncs.Con_Printf("Error!\n");
        gEngfuncs.Con_Printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
        return;
    }

	FSOUND_3D_SetRolloffFactor(0.01); // TODO: attenuation due to environement
	FSOUND_3D_SetDopplerFactor(1); // normal factor
	FSOUND_3D_SetDistanceFactor(0.25); // unit to meter

	SetupReverb();
	DrySFXUnit = FSOUND_DSP_Create(NULL, FSOUND_DSP_DEFAULTPRIORITY_USER+100, 0);
	FSOUND_DSP_SetActive(DrySFXUnit, TRUE);

}

//===================================
//			SHUTDOWN SYSTEM
//===================================
void SoundShutDown( void )
{
	CloseReverb();
	FSOUND_DSP_Free(DrySFXUnit); 
    FSOUND_Close();
}

//===================================
//			UPDATE SYSTEM
//===================================
void SoundUpdate( void )
{
	vec3_t angles, forward, right, up, vel;
	static vec3_t lastpos;
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	
	static float lasttime;
	float frametime = gEngfuncs.GetClientTime() - lasttime;
	lasttime = gEngfuncs.GetClientTime();
	
	gEngfuncs.GetViewAngles( (float *)angles );
	AngleVectors( angles, forward, right, up );

	vel = player->curstate.origin - lastpos;
	player->curstate.velocity = vel;
	//VectorScale( vel, 0.1, vel ); // put it in m/s and scale correctly
	lastpos = player->curstate.origin;

	FSOUND_3D_Listener_SetAttributes(player->curstate.origin, vel,
		-forward.x, -forward.y, -forward.z, up.x, up.y, up.z); // update 'ears'

    FSOUND_Update();
	SoundUpdateSample();
}

//===================================
//		FIND and UPDATE 3d samples
//===================================
void SoundUpdateSample( void )
{
	int sampleid = 0, test = 0;
	vec3_t origin, vel;
	sample_s *cursamp;

	while( sampleid <= MAX_SAMPLES )
	{
		cursamp = &g_samples[sampleid];
		if( !cursamp->entindex )
		{
			sampleid++;
			continue;
		}
		if( !FSOUND_IsPlaying( cursamp->channel ))
		{
			FSOUND_FX_Disable(g_samples[sampleid].channel);
			FSOUND_Sample_Free(g_samples[sampleid].sample);
			g_samples[sampleid].channel = 0;
			g_samples[sampleid].entindex = 0;
			sampleid++;
			continue;
		}
		sampleid++;
		test++;

		origin = gEngfuncs.GetEntityByIndex( cursamp->entindex )->curstate.origin;
		vel = gEngfuncs.GetEntityByIndex( cursamp->entindex )->curstate.velocity;
		//FSOUND_3D_SetAttributes( cursamp->channel, origin, vel); // TODO: set velocity to sound
	}
	//gEngfuncs.Con_Printf("Samples: %i\n", test );
}

//===================================
//		PLAY NON moving 3d samples
//===================================
void SoundPlay( vec3_t origin, char *soundname, int vol, int pitchscale )
{
	sample_s *sample = 0;
	FSOUND_SAMPLE *samp = 0;
	char sound[256];
	int channel;
	float pitch;

	sprintf( sound, "%s/media/%s", gEngfuncs.pfnGetGameDirectory(), soundname );

    samp = FSOUND_Sample_Load(FSOUND_FREE, sound, FSOUND_HW3D /*| FSOUND_ENABLEFX*/, 0, 0);
    if (!samp)
    {
        gEngfuncs.Con_Printf("Error!\n");
        gEngfuncs.Con_Printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
        return;
    }

	channel = FSOUND_PlaySoundEx(FSOUND_FREE, samp, /*DrySFXUnit*/0, FALSE);

	FSOUND_3D_SetAttributes(channel, origin, NULL); // TODO: set velocity to sound
	FSOUND_SetVolume(channel, vol );

	pitch = FSOUND_GetFrequency( channel );
	pitch *= float(pitchscale)/100;
	gEngfuncs.Con_Printf("pitch: %f\n", pitch );
	FSOUND_SetFrequency( channel, (int)pitch );

    gEngfuncs.Con_Printf("Channel: %i\n", channel );
}

void SoundDynPlay( vec3_t origin, char *soundname )
{

}

//===================================
//		PLAY moving 3d samples
//===================================
void SoundEntPlay( int entindex, char *soundname, int vol, int pitchscale )
{
	sample_s sample, *cursamp;
	FSOUND_SAMPLE *samp = 0;
	char sound[256];
	int channel, sampleid = 0;
	float pitch;

	sprintf( sound, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), soundname );

    /* PCM,44,100 Hz, 8 Bit, Mono */
    samp = FSOUND_Sample_Load(FSOUND_FREE, sound, FSOUND_HW3D/*|FSOUND_ENABLEFX*/, 0, 0);
    if (!samp)
    {
        gEngfuncs.Con_Printf("Error!\n");
        gEngfuncs.Con_Printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
        return;
    }

	channel = FSOUND_PlaySoundEx(FSOUND_FREE, samp, NULL/*DrySFXUnit*/, FALSE);

	FSOUND_3D_SetAttributes(channel, gEngfuncs.GetEntityByIndex( entindex )->curstate.origin, NULL);
	FSOUND_SetVolume( channel, vol );

	//FSOUND_SetPan(channel, 20);
	pitch = FSOUND_GetFrequency( channel );
	pitch *= float(pitchscale)/100;
	gEngfuncs.Con_Printf("pitch: %f\n", pitch );
	FSOUND_SetFrequency( channel, (int)pitch );

	sample.channel = channel;	
	sample.entindex = entindex;
	sample.sample = samp;

	while( sampleid <= MAX_SAMPLES )
	{
		cursamp = &g_samples[sampleid];
		if( !cursamp->channel )
		{
			gEngfuncs.Con_Printf("Ya de la place a: %i\n", sampleid );
			g_samples[sampleid] = sample;
			break;
		}
		sampleid++;
	}
}

/*
		FSOUND_SetPaused(FSOUND_ALL, TRUE);
		FSOUND_FX_Enable(FSOUND_ALL, FSOUND_FX_FLANGER);	
		int echo = FSOUND_FX_Enable(FSOUND_ALL, FSOUND_FX_I3DL2REVERB);
		FSOUND_FX_SetI3DL2Reverb( echo, -1000, 0, 0.0,
		1.49, 0.83, -2602, 0.007,
		200, 0.011, 100.0, 100.0, 5000.0);
        FSOUND_SetPaused(FSOUND_ALL, FALSE);
*/

	//int eq1 = FSOUND_FX_Enable(channel, FSOUND_FX_PARAMEQ);
	//int eq2 = FSOUND_FX_Enable(channel, FSOUND_FX_PARAMEQ);
	//int echo1 = FSOUND_FX_Enable(channel, FSOUND_FX_ECHO);
	//echoid2 = FSOUND_FX_Enable(FSOUND_SYSTEMCHANNEL, FSOUND_FX_ECHO);
	//flangeid = FSOUND_FX_Enable(FSOUND_SYSTEMCHANNEL, FSOUND_FX_FLANGER);

	//FSOUND_FX_SetEcho(echo1,  8.0f, 40.0f, 490.0f, 500.0f, TRUE);
	//FSOUND_FX_SetEcho(echoid,  80.0f, 30.0f, 100.0f, 100.0f, TRUE);
	//FSOUND_FX_SetEcho(echoid2,  100, 70.0f, 10, 10, FALSE);
	//FSOUND_FX_SetParamEQ(eq1, 8000, 36, -15);
	//FSOUND_FX_SetParamEQ(eq2, 16000, 36, -15);