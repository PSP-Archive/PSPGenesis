//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPVIDEO_H
#define __YPSPVIDEO_H

#include <YPSPTypes.h>
#include "YPSPFont.h"

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------
YBOOL YPSPVideo_Init();
void YPSPVideo_DeInit();

void YPSPVideo_SetVideoMode(YINT32 a_iMode, YINT32 a_iWidth, YINT32 a_iHeight);

void YPSPVideo_WaitVSync();
void YPSPVideo_WaitVSyncN(YINT32 a_iCount);

#define YPSPVIDEO_SCREENFRAMEMODE_INVISIBLE		0
#define YPSPVIDEO_SCREENFRAMEMODE_ONSCREEN		1
#define YPSPVIDEO_SCREENFRAMEMODE_OFFSCREEN		2

void YPSPVideo_ScreenFrame(YINT32 a_iMode, YINT32 a_iVisibleFrame);
void YPSPVideo_ScreenFlip();
void YPSPVideo_ScreenFlipVSync();
YUCHAR8 *YPSPVideo_GetVramAddr(YUINT32 x, YUINT32 y);
YUCHAR8 *YPSPVideo_GetVramFrameAddr(YINT32 a_iFrame, YUINT32 x, YUINT32 y);

void YPSPVideo_Fillvram(YUINT32 a_uColor);
void YPSPVideo_BitBlt16(YUINT32 x, YUINT32 y, YUINT32 w, YUINT32 h, const YUINT16 *srcdata, YUINT32 a_uSrcStridePixels);
void YPSPVideo_BitBlt16_Swapped(YUINT32 x, YUINT32 y, YUINT32 w, YUINT32 h, const YUINT16 *srcdata, YUINT32 a_uSrcStridePixels);
void YPSPVideo_BitBlt32(YUINT32 x, YUINT32 y, YUINT32 w, YUINT32 h, const YUINT32 *srcdata, YUINT32 a_uSrcStridePixels);
void YPSPVideo_Fillrect(YUINT32 x, YUINT32 y, YINT32 w, YINT32 h, YUINT32 color);

void YPSPVideo_PrintString(YUINT32 x, YUINT32 y, YUINT32 color, const YCHAR8 *str, YBitmapFont *a_pFont);
void YPSPVideo_PrintString16(YUINT32 x, YUINT32 y, YUINT32 color, const YCHAR16 *str, YBitmapFont *a_pFont);
YINT32 YPSPVideo_PutChar(YUINT32 x, YUINT32 y, YUINT32 color, const YCHAR8 ch, YBitmapFont *a_pFont);
YINT32 YPSPVideo_PutChar16(YUINT32 x, YUINT32 y, YUINT32 color, const YCHAR16 ch, YBitmapFont *a_pFont);

void YPSPVideo_PrintStringFilled(YUINT32 x, YUINT32 y, YUINT32 fgcolor, YUINT32 bgcolor, const YCHAR8 *str, YBitmapFont *a_pFont);
void YPSPVideo_PrintStringFilled16(YUINT32 x, YUINT32 y, YUINT32 fgcolor, YUINT32 bgcolor,  const YCHAR16 *str, YBitmapFont *a_pFont);
YINT32 YPSPVideo_PutCharFilled(YUINT32 x, YUINT32 y, YUINT32 fgcolor, YUINT32 bgcolor, const YCHAR8 ch, YBitmapFont *a_pFont);
YINT32 YPSPVideo_PutCharFilled16(YUINT32 x, YUINT32 y, YUINT32 fgcolor, YUINT32 bgcolor, const YCHAR16 ch, YBitmapFont *a_pFont);

void YPSPVideo_ScreenError(const YCHAR8 *str);
void YPSPVideo_ScreenDebug(const YCHAR8 *str);
void YPSPVideo_ScreenDebugPause(const YCHAR8 *str);
void YPSPVideo_ClearScreens(YUINT32 color);

YFLOAT32 YPSPVideo_DisplayGetFramePerSec();

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------
#define YPSP_SCREEN_WIDTH		480
#define YPSP_SCREEN_HEIGHT		272

#define		PIXELSIZE		1				//in short
#define		LINESIZE		512				//in short
#define		FRAMESIZEBYTES	0x44000			//in byte

#endif //__YPSPVIDEO_H
