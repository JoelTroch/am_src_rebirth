
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "fmod.h"
#include "reverb.h"

#define	ENABLEREVERB	TRUE
#define RECORDRATE      44100
#define RECORDLEN       (RECORDRATE * 5)    /* 5 seconds at RECORDRATE khz */
#define OUTPUTRATE      44100

#define REVERB_NUMTAPS	3 // 7

typedef struct
{
	FSOUND_DSPUNIT	*Unit;
	char			*historybuff;		/* storage space for tap history */
	char			*workarea;			/* a place to hold 1 buffer worth of data (for reverb) */
	int				delayms;			/* delay of reverb tab in milliseconds */
	int				volume;				/* volume of reverb tab */
	int				pan;				/* pan of reverb tab */
	int				historyoffset;		/* running offset into history buffer */
	int				historylen;			/* size of history buffer in SAMPLES */
} REVERBTAP;

/*
    Reverb stuff
*/
REVERBTAP		DSP_ReverbTap[REVERB_NUMTAPS];

/*
[
	[DESCRIPTION]
	Callback to mix in one reverb tap.  It copies the buffer into its own history buffer also.

	[PARAMETERS]
	'originalbuffer'	Pointer to the original mixbuffer, not any buffers passed down 
						through the dsp chain.  They are in newbuffer.
	'newbuffer'			Pointer to buffer passed from previous DSP unit.
	'length'			Length in SAMPLES of buffer being passed.
	'param'				User parameter.  In this case it is a pointer to DSP_LowPassBuffer.
 
	[RETURN_VALUE]
	a pointer to the buffer that was passed in, with a tap mixed into it.

	[REMARKS]
]
*/
void * F_CALLBACKAPI DSP_ReverbCallback(void *originalbuffer, void *newbuffer, int length, void *param)
{
	int		     mixertype = FSOUND_GetMixer();
    int          count;
    int          bytesperoutputsample;
	REVERBTAP	*tap = (REVERBTAP *)param;
    union sample
    {
        void         *vptr;
        signed int   *dptr;
        signed short *wptr;
        float        *fptr;
    };

    if (mixertype == FSOUND_MIXER_MMXP5 || mixertype == FSOUND_MIXER_MMXP6 || mixertype == FSOUND_MIXER_QUALITY_MMXP5 || mixertype == FSOUND_MIXER_QUALITY_MMXP6)
    {
	    bytesperoutputsample = 4;   // 16bit stereo
    }
    else
    {
	    bytesperoutputsample = 8;   // 32bit stereo
    }

	// reverb history buffer is a ringbuffer.  If the length makes the copy wrap, then split the copy 
	// into end part, and start part.. 
	if (tap->historyoffset + length > tap->historylen)
	{
		int taillen = tap->historylen - tap->historyoffset;
		int startlen = length - taillen;

		// mix a scaled version of history buffer into output
		FSOUND_DSP_MixBuffers(newbuffer, tap->historybuff + (tap->historyoffset << 2), taillen, OUTPUTRATE, tap->volume, tap->pan, FSOUND_STEREO | FSOUND_16BITS);
		FSOUND_DSP_MixBuffers((char *)newbuffer+(taillen * bytesperoutputsample), tap->historybuff, startlen, OUTPUTRATE, tap->volume, tap->pan, FSOUND_STEREO | FSOUND_16BITS);

		// now copy input into reverb/history buffer 
        {
            signed short *dest;
        	union sample src;

            dest = (signed short *)(tap->historybuff + (tap->historyoffset << 2));
            src.vptr = newbuffer;

            for (count=0; count < taillen * 2; count++)
            {
                int val;
                
                if (mixertype == FSOUND_MIXER_QUALITY_FPU)
                {
                    val = (int)src.fptr[count];
                }
                else if (mixertype == FSOUND_MIXER_MMXP5 || mixertype == FSOUND_MIXER_MMXP6 || mixertype == FSOUND_MIXER_QUALITY_MMXP5 || mixertype == FSOUND_MIXER_QUALITY_MMXP6)
                {
                    val = (int)src.wptr[count];
                }
                else
                {
                    val = (int)src.dptr[count];
                }               

                val = (val > 32767 ? 32767 : val < -32768 ? -32768 : val);
                dest[count] = val;
            }	
        }
        {
            signed short *dest;
        	union sample src;

            dest = (signed short *)tap->historybuff;    // always 16bit
            src.vptr = (char *)newbuffer + (taillen * bytesperoutputsample);

            for (count=0; count < startlen * 2; count++)
            {
                int val;
                
                if (mixertype == FSOUND_MIXER_QUALITY_FPU)
                {
                    val = (int)src.fptr[count];
                }
                else if (mixertype == FSOUND_MIXER_MMXP5 || mixertype == FSOUND_MIXER_MMXP6 || mixertype == FSOUND_MIXER_QUALITY_MMXP5 || mixertype == FSOUND_MIXER_QUALITY_MMXP6)
                {
                    val = (int)src.wptr[count];
                }
                else
                {
                    val = (int)src.dptr[count];
                }               

                val = (val > 32767 ? 32767 : val < -32768 ? -32768 : val);
                dest[count] = val;
            }	
        }

	}
	// no wrapping reverb buffer, just write dest
	else
	{
		// mix a scaled version of history buffer into output
		FSOUND_DSP_MixBuffers(newbuffer, tap->historybuff + (tap->historyoffset << 2), length, OUTPUTRATE, tap->volume, tap->pan, FSOUND_STEREO | FSOUND_16BITS);

		// now copy input into reverb/history buffer 
        {
            signed short *dest;
        	union sample src = { newbuffer };

            dest = (signed short *)(tap->historybuff + (tap->historyoffset << 2));

            for (count=0; count < length * 2; count++)
            {
                int val;

                if (mixertype == FSOUND_MIXER_QUALITY_FPU)
                {
                    val = (int)src.fptr[count];
                }
                else if (mixertype == FSOUND_MIXER_MMXP5 || mixertype == FSOUND_MIXER_MMXP6 || mixertype == FSOUND_MIXER_QUALITY_MMXP5 || mixertype == FSOUND_MIXER_QUALITY_MMXP6)
                {
                    val = (int)src.wptr[count];
                }
                else
                {
                    val = (int)src.dptr[count];
                }               
                val = (val > 32767 ? 32767 : val < -32768 ? -32768 : val);
                dest[count] = val;
            }	
        }
	}


	tap->historyoffset += length;
	if (tap->historyoffset >= tap->historylen) 
    {
		tap->historyoffset -= tap->historylen;
    }

	// reverb history has been mixed into new buffer, so return it.
	return newbuffer;
}



/*
[
	[DESCRIPTION]
	Initializes reverb, creates DSP units and history buffers for all reverb tabs

	[PARAMETERS]
 
	[RETURN_VALUE]
	void

	[REMARKS]
]
*/
void SetupReverb()
{
	/*
	    REVERB SETUP
	*/
	/* something to fiddle with. */
	//int delay[REVERB_NUMTAPS]	= { 131, 149, 173, 211, 281, 401, 457};	/* prime numbers make it sound good! */
	//int volume[REVERB_NUMTAPS]	= { 120, 100,  95,  90,  80,  60,  50};
	//int pan[REVERB_NUMTAPS]		= { 100, 128, 128, 152, 128, 100, 152};
	int delay[REVERB_NUMTAPS]	= { 150, 170, 200};
	int volume[REVERB_NUMTAPS]	= { 120, 100,  95};
	int pan[REVERB_NUMTAPS]		= { 100, 128, 128};
	int count;

	for (count=0; count< REVERB_NUMTAPS; count++)
	{
		DSP_ReverbTap[count].delayms		= delay[count];	
		DSP_ReverbTap[count].volume			= volume[count];
		DSP_ReverbTap[count].pan			= pan[count];
		DSP_ReverbTap[count].historyoffset	= 0;
		DSP_ReverbTap[count].historylen		= (DSP_ReverbTap[count].delayms * 44100 / 1000);
		if (DSP_ReverbTap[count].historylen < FSOUND_DSP_GetBufferLength())
			DSP_ReverbTap[count].historylen = FSOUND_DSP_GetBufferLength();	/* just in case our calc is not the same. */

		DSP_ReverbTap[count].historybuff	= (char *)calloc(DSP_ReverbTap[count].historylen, 4);	/* * 4 is for 16bit stereo (mmx only) */
		DSP_ReverbTap[count].workarea		= NULL;
		DSP_ReverbTap[count].Unit			= FSOUND_DSP_Create(&DSP_ReverbCallback, FSOUND_DSP_DEFAULTPRIORITY_USER+20+(count*2), (void *)&DSP_ReverbTap[count]);

		FSOUND_DSP_SetActive(DSP_ReverbTap[count].Unit, TRUE);
	}
}


/*
[
	[DESCRIPTION]
	Shuts down and frees anything to do with the software reverb

	[PARAMETERS]
 
	[RETURN_VALUE]
	void

	[REMARKS]
]
*/
void CloseReverb()
{
	int count;

	for (count=0; count<REVERB_NUMTAPS; count++)
	{
		FSOUND_DSP_Free(DSP_ReverbTap[count].Unit);	
		DSP_ReverbTap[count].Unit = NULL;

		free(DSP_ReverbTap[count].historybuff);		
		DSP_ReverbTap[count].historybuff = NULL;

		free(DSP_ReverbTap[count].workarea);		
		DSP_ReverbTap[count].workarea = NULL;
	}
}
