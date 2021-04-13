#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "generator.h"
#include "snprintf.h"

#include "cpu68k.h"
#include "cpuz80.h"
#include "ui.h"
#include "vdp.h"
#include "event.h"
#include "gensound.h"
#include "mem68k.h"
#include "uip.h"
#include "state.h"

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//#define BANKSIZE 512*384*2

// joypad data
//int paddata;
//int data2;


/*
#define MAXX		(MAX_WIDTH/16)
#define MAXY		(MAX_HEIGHT/16)
#define SIZE		(MAXX * MAXY * 6 + 10)
#define SCREEN_SIZE	(MAX_WIDTH/16 * MAX_HEIGHT/16 * 6 + 10)
*/

int width, height;
int origin_x, origin_y;

// screen 
static t_uipinfo *uip_uipinfo = NULL;   /* uipinfo */
static uint8 uip_displaybanknum = 0;    /* view this one, write to other one */

static int uip_forceredshift = -1;      /* if set, forces red shift pos */
static int uip_forcegreenshift = -1;    /* if set, forces green shift pos */
static int uip_forceblueshift = -1;     /* if set, forces blue shift pos */

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
int uip_init(t_uipinfo *uipinfo)
{
	// todo
	uip_uipinfo = uipinfo;
	
	return 0;
}

int uip_initjoysticks(void)
{
	return 1;
}

static void clearBackground(u_int val)
{
	YPSPVideo_Fillvram(val);
/*
	int i;
    u_int *p = (u_int*)UnCachedAddress();

    for (i = 0; i < 16*16; i++) {
        p[i] = val;
    }
*/
}

int uip_vgamode(void)
{
	uip_uipinfo->linewidth = 640.0f;

//	uip_uipinfo->screenmem0 = Buf0;
//	uip_uipinfo->screenmem1 = Buf1;
/*
	if ((uip_uipinfo->screenmem0 = (uint8 *)malloc(sizeof(uint8) * BANKSIZE)) == NULL)
	{
		uip_textmode();
		r_printf("Failed to find start of screen memory - bank 0\n");
		while(1);
	}
	if ((uip_uipinfo->screenmem1 = (uint8 *)malloc(sizeof(uint8) * BANKSIZE)) == NULL)
	{
		uip_textmode();
		r_printf("Failed to find start of screen memory - bank 1\n");
		while(1);
	}
*/
	if (uip_forceredshift != -1 && uip_forcegreenshift != -1 && uip_forceblueshift != -1)
	{
		uip_uipinfo->redshift = uip_forceredshift;
		uip_uipinfo->greenshift = uip_forcegreenshift;
		uip_uipinfo->blueshift = uip_forceblueshift;
	}
	else
	{
		//r_printf("Assuming colour bit positions 11,6,0 for 64k colour mode\n");
		uip_uipinfo->blueshift = 0;
		uip_uipinfo->greenshift = 6;
		uip_uipinfo->redshift = 11;
	}
	uip_uipinfo->blueshift = 16;
	uip_uipinfo->greenshift = 8;
	uip_uipinfo->redshift = 0;

		//uip_uipinfo->blueshift =10;
		//uip_uipinfo->greenshift = 5;
		//uip_uipinfo->redshift = 0;

    width = 640;
    height = 480;
    clearBackground(0x80404040);

//	uip_displaybank(0);           // set current to 0th bank
//	uip_clearscreen();            // clear bank
//	uip_displaybank(-1);          // toggle bank
//	uip_clearscreen();            // clear bank

	return 0;
}

void uip_copyscreenblocks16(uint16 *pScr)
{
	//pgBitBlt2(0,0,320,224,(const unsigned short*)pScr);
}

void uip_copyscreenblocks(uint32 *pScr)
{
	static int c = 0;
	if (c == 0)
	{
//		pgFillvram(0x90909090);
	}
	else
	{
//		pgFillvram(0x40404040);
	}
	c = 1-c;

	//pgBitBlt32(0,0,320,224,(const unsigned int*)pScr);
	YPSPVideo_BitBlt32(0,0,320,224,(const unsigned int*)pScr,320);

/*char szBuffer[256];
static siDebugFrames = 0;
++siDebugFrames;
sprintf(szBuffer, "Rendering Frame: %d", siDebugFrames);
pgFillvram(0x90909090);
pgPrint(0,0,0xffff,szBuffer);
*/


return;

}

void uip_displaybank(int bank)
{
	uip_checkkeyboard();

	YPSPVideo_ScreenFlip();

/*	{
		int i, j;
		uint32 *pBuf = (uint32 *)(uip_uipinfo->screenmem_w);

	}
*/
	if (uip_displaybanknum)
	{
		uip_displaybanknum = 0;
	}
	else
	{
		uip_displaybanknum = 1;
	}
}

void uip_clearscreen(void)
{
//	memset(uip_uipinfo->screenmem_w, 0, BANKSIZE);
}

void uip_textmode(void)
{
}


extern int g_iSkip;
extern uint8 ui_state;

int uip_checkkeyboard(void)
{
	YPSP_ControllerData *pPadData = YPSP_GetCurControllerData();

	int		pad;

	pad = 0;
	mem68k_cont.cont1[pad].a = (pPadData->buttons & (YPSPINPUT_SQUARE|YPSPINPUT_TRIANGLE)) ? 1 : 0;
	mem68k_cont.cont1[pad].b = (pPadData->buttons & YPSPINPUT_CROSS) ? 1 : 0;
	mem68k_cont.cont1[pad].c = (pPadData->buttons & YPSPINPUT_CIRCLE) ? 1 : 0;
	mem68k_cont.cont1[pad].start	= (pPadData->buttons & YPSPINPUT_START) ? 1 : 0;
	mem68k_cont.cont1[pad].left	= (pPadData->buttons & YPSPINPUT_LEFT) ? 1 : 0;
	mem68k_cont.cont1[pad].right	= (pPadData->buttons & YPSPINPUT_RIGHT) ? 1 : 0;
	mem68k_cont.cont1[pad].up		= (pPadData->buttons & YPSPINPUT_UP) ? 1 : 0;
	mem68k_cont.cont1[pad].down	= (pPadData->buttons & YPSPINPUT_DOWN) ? 1 : 0;

	if (pPadData->buttons & YPSPINPUT_SELECT)
	{
		ui_state = 3;
	}

	return 0;
}

void uip_vsync(void)
{
}

unsigned int uip_whichbank(void)
{
  /* returns 0 or 1 - the bank being VIEWED */
  return uip_displaybanknum;
}

void uip_singlebank(void)

{
	uip_uipinfo->screenmem_w = uip_displaybanknum ? uip_uipinfo->screenmem1 : uip_uipinfo->screenmem0;
}

void uip_doublebank(void)
{
	uip_uipinfo->screenmem_w = uip_displaybanknum ? uip_uipinfo->screenmem0 : uip_uipinfo->screenmem1;
}

uint8 uip_getchar(void)
{
	return 0;
}

void uip_clearmiddle(void)
{
	int i;

	for (i = 0; i < 240; i++)
	{
		memset(uip_uipinfo->screenmem_w + (uip_uipinfo->linewidth) * (120 + i) + 160 * 2, 0, 2 * 320);
	}
}

int uip_setcolourbits(int red, int green, int blue)
{
	uip_forceredshift = red;
	uip_forcegreenshift = green;
	uip_forceblueshift = blue;
	return 0;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
