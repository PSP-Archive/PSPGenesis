//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPKERNEL_H
#define __YPSPKERNEL_H

#include <YPSPTypes.h>

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

typedef YINT32 (*YPSPExitCallbackFn)(void);

YBOOL	YPSPKernel_CreateExitCallback(YPSPExitCallbackFn a_pExitCallbackFn);
YBOOL	YPSPKernel_IsExiting();

YUINT32	YPSPKernel_GetFreeHeapMemory();
YUINT32 YPSPKernel_GetUsedMemory();
YUINT32 YPSPKernel_GetFreeMemory();

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

void	YPSP_KernelSleepThreadCB(void);
YINT32	YPSP_KernelCreateCallback(const YCHAR8 *a_szName, void *a_pFunc, void *a_pArgs);
YINT32	YPSP_KernelRegisterExitCallback(YINT32 a_iCbID);
void	YPSP_KernelExitGame(void);

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

YINT32	YPSP_PowerSetClockFrequency(int unknown1, int unknown2, int unknown3);

#endif //__YPSPKERNEL_H
