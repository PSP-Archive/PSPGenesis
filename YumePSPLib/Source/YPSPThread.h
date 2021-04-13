//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPTHREAD_H
#define __YPSPTHREAD_H

#include <YPSPTypes.h>

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

typedef YINT32 (*YPSPThreadCallbackFn)(YINT32 a_iArgc, YCHAR8 *argv);

YINT32	YPSP_KernelCreateThread(const YCHAR8 *a_szName, YPSPThreadCallbackFn a_pFn, YUINT32 a_uPriority, YUINT32 a_uStackSize, YUINT32 a_uAttribs, void *a_pUnknown);
YINT32	YPSP_KernelStartThread(YINT32 a_iThreadHandle, YINT32 a_iArgc, void *a_pArgV);
void	YPSP_KernelExitThread(YINT32 a_iRet);
YINT32	YPSP_KernelWaitThreadEnd(YINT32 a_iThreadHandle, void *a_pUnknown);
YINT32	YPSP_KernelDeleteThread(YINT32 a_iThreadHandle);
YINT32	YPSP_KernelSleepThread(void);
YINT32	YPSP_KernelWakeupThread(YINT32 a_iThreadID);
YINT32	YPSP_KernelResumeThread(YINT32 a_iThreadID);

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------


#endif //__YPSPTHREAD_H
