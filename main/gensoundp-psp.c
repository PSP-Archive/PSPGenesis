#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "generator.h"
#include "gensound.h"
#include "gensoundp.h"
#include "vdp.h"
#include "ui.h"

#include <stdio.h>

short *pSample = 0;
int filesize = 0, iOffset = 0;
short *loadwav(const char *filename)
{
	return 0;
}


//sint16 *pLBuf = 0;
sint16 pLBuf[1024] __attribute__((aligned (64)));
sint16 pRBuf[1024] __attribute__((aligned (64)));
sint16 pLRBuf[1024*2] __attribute__((aligned (64)));
YINT32	g_iAudioOutputChan = -1;

extern unsigned int ui_soundspeedfix;


int soundp_start(void)
{
	static int iSoundInited = 0;

	if (!iSoundInited)
	{
		g_iAudioOutputChan = YPSPSound_CreateSoundBuffer(48000, 2, 0.5f);
		YPSPSound_Play(g_iAudioOutputChan, YPSPSOUNDFLAG_PLAY_LOOPING);

		iSoundInited = 1;

		{
			int k;
			for (k = 0; k < 960; ++k)
			{
				pLBuf[k] = 0;
				pRBuf[k] = 0;
			}
		}
	}

	YINT32	iFreq = sound_speed;
	if (ui_soundspeedfix)
	{
		iFreq = (sound_speed * 60) / 40;
	}
	YPSPSound_SetFrequency(g_iAudioOutputChan, iFreq);

	//YPSPSound_SetFrequency(g_iAudioOutputChan, sound_speed);

	return 0;
}

void soundp_stop(void)
{
	YPSPSound_ClearBuffer(g_iAudioOutputChan);
	YPSPSound_Stop(g_iAudioOutputChan);
}

int soundp_samplesbuffered(void)
{
	return YPSPSound_GetSamplesBuffered(g_iAudioOutputChan);
}

extern unsigned int sound_speed;

void soundp_output(uint16 *left, uint16 *right, unsigned int samples)
{
	int i, j = 0, k;
	for (i = 0; i < samples;++i)
	{
		{
			pLRBuf[j++] = ((sint16)left[i]);
			pLRBuf[j++] = ((sint16)right[i]);
		}
	}

	//memcpy(pLBuf, left, samples>>1);

	{
		YUINT16 *pBuf1, *pBuf2;
		YUINT32 iSamples1, iSamples2;

		if (YPSPSound_Lock(g_iAudioOutputChan, samples, &pBuf1, &iSamples1, &pBuf2, &iSamples2, 0))
		{
			if (pBuf1)
			{
				memcpy(pBuf1, pLRBuf, iSamples1<<2);
			}
			if (pBuf2)
			{
				memcpy(pBuf2, (pLRBuf+(iSamples1<<1)), iSamples2<<2);
			}

			YPSPSound_Unlock(g_iAudioOutputChan, iSamples1+iSamples2);
		}
	}

}
