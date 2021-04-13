//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPSOUND_H
#define __YPSPSOUND_H

#include <YPSPTypes.h>

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------
YBOOL YPSPSound_Init();
void YPSPSound_DeInit();
//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

#define YPSPSOUNDFLAG_PLAY_ONCE				(1<<0)
#define YPSPSOUNDFLAG_PLAY_LOOPING			(1<<1)

YINT32	YPSPSound_CreateSoundBuffer(YINT32 a_iFreq, YINT8 a_iChannels, YFLOAT32 a_fBufLenSecs);
YBOOL	YPSPSound_DestroySoundBuffer(YINT32 a_iChannelID);
YBOOL	YPSPSound_Play(YINT32 a_iChannelID, YUINT32 a_uFlags);
YBOOL	YPSPSound_Stop(YINT32 a_iChannelID);
void	YPSPSound_ClearBuffer(YINT32 a_iChannelID);
YINT32	YPSPSound_NumFreeBufferSamples(YINT32 a_iChannelID);
YINT32	YPSPSound_GetSamplesBuffered(YINT32 a_iChannelID);
YBOOL	YPSPSound_GetCurrentPosition(YINT32 a_iChannelID, YINT32 *a_piReadPos, YINT32 *a_piWritePos);
YBOOL	YPSPSound_Lock(YINT32 a_iChannelID, YINT32 a_iSamples, YUINT16 **a_pBuf1, YUINT32 *a_piSamples1, YUINT16 **a_pBuf2, YUINT32 *a_piSamples2, YUINT32 a_uFlags);
YBOOL	YPSPSound_Unlock(YINT32 a_iChannelID, YINT32 a_iSamplesWritten);
YBOOL	YPSPSound_SetFrequency(YINT32 a_iChannelID, YINT32 a_iFreq);
YUINT32	YPSPSoundDebug_GetChannelFlags(YINT32 a_iChannelID);
void	YPSPSoundDebug_DisplayStats(YINT32 a_iChannelID);

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

#endif //__YPSPSOUND_H
