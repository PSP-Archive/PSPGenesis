//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPINPUT_H
#define __YPSPINPUT_H

#include <YPSPTypes.h>

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------
YBOOL	YPSPInput_Init();

void	YPSPInput_WaitKey();
void	YPSPInput_WaitForKey(YUINT32 a_uKey);

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

//Button bit masks
#define YPSPINPUT_SQUARE	0x8000
#define YPSPINPUT_TRIANGLE	0x1000
#define YPSPINPUT_CIRCLE	0x2000
#define YPSPINPUT_CROSS		0x4000
#define YPSPINPUT_UP		0x0010
#define YPSPINPUT_DOWN		0x0040
#define YPSPINPUT_LEFT		0x0080
#define YPSPINPUT_RIGHT		0x0020
#define YPSPINPUT_START		0x0008
#define YPSPINPUT_SELECT	0x0001
#define YPSPINPUT_LTRIGGER	0x0100
#define YPSPINPUT_RTRIGGER	0x0200

// analog indexes
#define YPSPINPUT_X_AXIS	0
#define YPSPINPUT_Y_AXIS	1

// Returned control data
typedef struct
{
   YUINT32	frame;
   YUINT32	buttons;
   YUCHAR8	analog[4];
   YUINT32	unused;
} YPSP_ControllerData;

YPSP_ControllerData *YPSP_ReadControllerData();
YPSP_ControllerData *YPSP_GetCurControllerData();

#endif //__YPSPINPUT_H
