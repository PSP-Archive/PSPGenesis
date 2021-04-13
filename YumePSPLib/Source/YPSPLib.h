//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPLIB_H
#define __YPSPLIB_H

#include <YPSPTypes.h>
#include <YPSPFile.h>
#include <YPSPInput.h>
#include <YPSPKernel.h>
#include <YPSPSound.h>
#include <YPSPThread.h>
#include <YPSPVideo.h>
#include <YPSPFont.h>
#include <YPSPTime.h>
#include <YPSPDebug.h>
//#include <YPSPLibcReplace.h>

YBOOL YPSPLib_Init(YINT32 a_iArgc, YCHAR8 *argv[]);
void YPSPLib_DeInit(void);

#endif //__YPSPLIB_H
