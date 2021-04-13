#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <pspgu.h>

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
#include "uiplot.h"

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void ui_rendertoscreen(void);
void ui_rendertoscreen2(void);
void ui_newframe(void);
static void ui_simpleplot(void);

void ui_updateMenu(void);


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
static uint8 ui_frameskip = 0;  /* 0 for dynamic */
static uint8 ui_actualskip = 0; /* the last skip we did (1..) */
uint8 ui_state = 0;      /* 0=stop, 1=paused, 2=play */

//static char *ui_initload = NULL;		// filename to load on init
//static int ui_saverom = 0;          /* flag to save rom and quit */

static uint16 *ui_screen0;      /* pointer to screen block for bank 0 */
static uint16 *ui_screen1;      /* pointer to screen block for bank 1 */
static uint16 *ui_newscreen;    /* pointer to new screen block */
static uint16 ui_screen[3][320 * 240];     /* screen buffers */

static uint8 ui_plotfield = 0;  /* flag indicating plotting this field */
uint8 ui_clearnext = 0;         /* flag indicating redraw required */
uint8 ui_vdpsimple = 0;         /* 0=raster, 1=cell based plotter */

static t_uipinfo ui_uipinfo;    /* uipinfo filled in by uip 'sub-system' */

static int ui_joysticks = 0;    /* number of joysticks */
uint32 ui_fkeys = 0;

static char * __attribute__((aligned(16))) pTempFrame = 0;

int g_iCurScreenScale = 1;
int	g_iCurScreenOffsetX = 0;
int	g_iCurScreenOffsetY = 0;
int	g_iCurScreenWidthX = 0;
int	g_iCurScreenWidthY = 0;

YCHAR8	g_szCurPath[YPSPFILE_MAX_PATH];
YCHAR8	g_szCurRom[YPSPFILE_MAX_PATH] = "";

#define			g_iMaxStatusTextLen		256
YCHAR8			g_szStatusText[g_iMaxStatusTextLen] = "";
YFLOAT32		g_fStatusTextTime = 0;
YPSPTimer		g_oStatusTextTimer;

PSPGenesis_SaveState g_oSaveStates;
PSPGenesis_SaveState *g_pCurSaveState = &g_oSaveStates;
int g_iClearScreen = 0;

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

#define MAX_ROMS	1024

int g_iNumRoms = 0;
char g_szRomList[MAX_ROMS][96];
char g_acIsDir[MAX_ROMS];

void ui_addstatustext(YCHAR8 *a_szText, YFLOAT32 a_fTime)
{
//	YPSPTimer_Reset(&g_oStatusTextTimer);
//	strcpy(g_szStatusText, a_szText);
//	g_fStatusTextTime = a_fTime;
}

void ui_resetstatustext()
{
	g_szStatusText[0] = 0;
}

void ui_updatestatustext(YBOOL a_bReadController)
{
	if (g_szStatusText[0])
	{
		if (a_bReadController)
		{
			YPSP_ReadControllerData();
		}
		if (g_oStatusTextTimer.m_fCurTime < g_fStatusTextTime)
		{
			YPSPVideo_PrintStringFilled(0, 0, 0xffff, 0, g_szStatusText, 0);
		}
		else
		{
			YUCHAR8 *pVRAM = YPSPVideo_GetVramFrameAddr(0, 0, 0);
			memset(pVRAM, 0, 12 * (LINESIZE*2));
			pVRAM = YPSPVideo_GetVramFrameAddr(1, 0, 0);
			memset(pVRAM, 0, 12 * (LINESIZE*2));

			g_szStatusText[0] = 0;
		}
		YPSPTimer_Update(&g_oStatusTextTimer);
	}
}



int ui_init(int argc, char *argv[])
{
	/*
	if ((ui_initload = malloc(strlen(szRomName + 1))) == NULL)
	{
		r_printf("Out of memory\n");
		while(1);
	}
	strcpy(ui_initload, szRomName);
	*/


	if (uip_init(&ui_uipinfo))
	{
		//r_printf("Failed to initialise platform dependent UI\n");
		//while(1);
	}
	ui_joysticks = uip_initjoysticks();
	//r_printf("%d joysticks detected\n", ui_joysticks);

	sound_on = 1;

	/* ui_newscreen is where the emulation data is rendered to - it is
	then compared with ui_screen0 or ui_screen1 depending on which
	screen bank it is being written to, and the delta changes are sent
	to screen memory.  The ui_screenX and ui_newscreen pointers are then
	switched, and the next frame/field begins... */
	memset(ui_screen[0], 0, sizeof(ui_screen[0]));
	memset(ui_screen[1], 0, sizeof(ui_screen[1]));
	ui_screen0 = ui_screen[0];
	ui_screen1 = ui_screen[1];
	ui_newscreen = ui_screen[2];

	//if (pTempFrame == 0)
	{
		//pTempFrame = (char *)malloc(sizeof(char) * (320+16) * (240+16));
		pTempFrame = (char *)malloc(sizeof(char) * (512) * (512));
	}


	// get working directory
	{
		YPSPFile_GetFilePath("Roms/", g_szCurPath, YPSPFILE_MAX_PATH);
		YINT32 iDirHandle = YPSP_DirOpen(g_szCurPath);
		if (iDirHandle < 0)
		{
			strcpy(g_szCurPath, YPSPFile_GetWorkingDir());
		}
		else
		{
			YPSP_DirClose(iDirHandle);
		}
	}

	return 0;
}

void PostLoadSavePause()
{
	YPSP_ControllerData *pCurInput = YPSP_GetCurControllerData();
	pCurInput->buttons = 0;
	pCurInput->analog[YPSPINPUT_X_AXIS] = 128;
	pCurInput->analog[YPSPINPUT_Y_AXIS] = 128;
	YPSPVideo_WaitVSyncN(10);
}

extern YINT32	g_iAudioOutputChan;

int ui_loop(void)
{
//char szBuffer[512];
//static int siDebugFrames = 0;

	//char		*p;

	//r_printf("Loading Rom\n");

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading Rom");
//pgScreenFlipV();

	//gen_loadmemrom((const char *)g_aaRom, g_iaRomSize);
	//gen_loadimage("fatms0:/a");
	//ui_state = 2;
	ui_state = 3;

	//if (p)
	{
		/*
		pgFillvram(0x90909090);
		pgPrint(0,0,0xffff,"Failed to Load ROM");
		pgScreenFlipV();
		*/
	//	while(1);
	}


/*
	if (ui_initload)
	{
		p = gen_loadimage(ui_initload);
		if (p)
		{
			r_printf("Failed to load ROM image: %s\n", p);
			while(1);
		}
		if (ui_saverom)
		{
*/

/*			snprintf(bigbuffer, sizeof(bigbuffer) - 1, "%s (%X-%s)",
						gen_cartinfo.name_overseas, gen_cartinfo.checksum,
						gen_cartinfo.country);
			if ((f = open(bigbuffer, 0777)) == -1
				|| write(f, cpu68k_rom, cpu68k_romlen) == -1 || close(f) == -1)
			{
				r_printf("Failed to write file: %s\n", strerror(errno));
			}
			r_printf("Successfully wrote: %s\n", bigbuffer);
			gen_quit = 1;
*/
/*			while(1);
		}
		ui_state = 2;
	}
	else
	{
		r_printf("You must specify a ROM to load\n");
		while(1);
	}
*/

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Initialising Screen");
//pgScreenFlipV();

	if (uip_vgamode())
	{
//		r_printf("Failed to start VGA mode");
//		while(1);
	}

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Set Screen Shifts");
//pgScreenFlipV();

	uiplot_setshifts(ui_uipinfo.redshift, ui_uipinfo.greenshift, ui_uipinfo.blueshift);

	gen_quit = 0;

//	r_printf("Entering Main Loop\n");

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Entering Main Emulation Loop");
//pgScreenFlipV();

//pgFillvram(0x0);
//pgScreenFlipV();
//pgFillvram(0x0);
//pgScreenFlipV();

	while (!YPSPKernel_IsExiting())
	{
		if (gen_quit)
		{
			ui_state = 3;
		}

		switch (ui_state)
		{
			case 0:	//stopped
			{
				break;
			}
			case 1:	// paused
			{
				break;
			}
			case 2:	// playing
			{
				ui_fkeys = 0;
				ui_newframe();
				event_doframe();

/*
++siDebugFrames;
sprintf(szBuffer, "Rendering Frame: %d", siDebugFrames);
pgFillvram(0x90909090);
pgPrint(0,0,0xffff,szBuffer);
pgScreenFlipV();
//*/
				break;
			}
			case 3:
			{
				ui_updateMenu();
				break;
			}
		}

//		YPSP_ReadControllerData();

		{
			YPSP_ControllerData *pCurInput = YPSP_GetCurControllerData();
			//if ((pCurInput->buttons & (YPSPINPUT_LTRIGGER|YPSPINPUT_RTRIGGER)) == (YPSPINPUT_LTRIGGER|YPSPINPUT_RTRIGGER))
			{
			}

			if (pCurInput->analog[YPSPINPUT_X_AXIS] > 245)
			{
				++g_iCurScreenScale;
				if (g_iCurScreenScale >= 5)
				{
					g_iCurScreenScale = 0;
				}
				g_iClearScreen = 1;
				PostLoadSavePause();
			//int	g_iCurScreenOffsetX = 0;
			//int	g_iCurScreenOffsetY = 0;
			//int	g_iCurScreenWidthX = 0;
			//int	g_iCurScreenWidthY = 0;
			}

			if (pCurInput->buttons & YPSPINPUT_LTRIGGER)
			{
				if (pCurInput->analog[YPSPINPUT_X_AXIS] < 10)
				{
					if (LoadStateFromDisk(g_pCurSaveState, 1))
					{
						ui_addstatustext("Loaded State Slot 1 from Disk", 3.0f);
					}
					PostLoadSavePause();
				}
				/*
				else
				{
					if (LoadStateFromMemory(g_pCurSaveState))
					{
						ui_addstatustext("Loaded State Slot 1 from Memory", 3.0f);
					}
				}
				*/
			}
			else if (pCurInput->buttons & YPSPINPUT_RTRIGGER)
			{
				if (pCurInput->analog[YPSPINPUT_X_AXIS] < 10)
				{
					if (SaveStateToDisk(g_pCurSaveState, 1))
					{
						ui_addstatustext("Saved State Slot 1 to Disk", 3.0f);
					}
					PostLoadSavePause();
				}
				/*
				else
				{
					if (SaveStateToMemory(g_pCurSaveState))
					{
						ui_addstatustext("Saved State Slot 1 to Memory", 3.0f);
					}
				}
				*/
			}
		}
	}

	return 0;
}

//static uint32 *pTempFrame32 = 0;
//static uint16 *pTempFrame16 = 0;
//int g_iSkip = 3;

#include "render.h"
	static int counter = 0;

// ui_line - it is time to render a line
void ui_line(int line)
{
/*
	if (line < 0 || line >= (int)vdp_vislines)
	{
		return;
	}
	if (ui_frameskip < g_iSkip)
	{
		return;
	}

	if (ui_vdpsimple)
	{ 
		if (line == (int)(vdp_vislines >> 1))
		{
			// if we're in simple cell-based mode, plot when half way
			// down screen //
			ui_simpleplot();
		}
		return;
	}

	return;
	*/
/*
char szBuffer[256];
sprintf(szBuffer, "Rendering Line: %d", line);
pgFillvram(0x90909090);
pgPrint(0,0,0xffff,szBuffer);
pgScreenFlipV();
//*/

	static uint8 gfx[320];
	unsigned int width = (vdp_reg[12] & 1) ? 320 : 256;

//*
	if (!ui_plotfield)
	{
		return;
	}
//*/
	if (line < 0 || line >= (int)vdp_vislines)
	{
		return;
	}
/*
	if (ui_frameskip < g_iSkip)
	{
		return;
	}
//*/
	if (ui_vdpsimple)
	{ 
		if (line == (int)(vdp_vislines >> 1))
		{
			// if we're in simple cell-based mode, plot when half way
			// down screen //
			ui_simpleplot();
		}
	}
	else
	{
		//vdp_renderline(line, pTempFrame + (line*320), 0);
		draw_scanline((char *)pTempFrame, (320+16), line);
	}
	
return;

	if (!ui_plotfield)
	{
		return;
	}

	if (line < 0 || line >= (int)vdp_vislines)
	{
		return;
	}

	if (ui_vdpsimple)
	{ 
		if (line == (int)(vdp_vislines >> 1))
		{
			// if we're in simple cell-based mode, plot when half way
			// down screen //
			ui_simpleplot();
		}
		return;
	}
	// we are plotting this frame, and we're not doing a simple plot at
	// the end of it all
	switch ((vdp_reg[12] >> 1) & 3)
	{
		case 0:                    // normal
		case 1:                    // interlace simply doubled up
		case 2:                    // invalid
		{
			vdp_renderline(line, gfx, 0);
			break;
		}
		case 3:                    // interlace with double resolution
		{
			vdp_renderline(line, gfx, vdp_oddframe);
			break;
		}
	}
	uiplot_checkpalcache(0);
	uiplot_convertdata16(gfx, ui_newscreen + line * 320, width);
}

// ui_endfield - end of field reached
void ui_endfield(void)
{
/*	if (ui_frameskip>=g_iSkip)
	{
		uip_displaybank(-1);
		ui_frameskip = 0;
	}
	ui_frameskip++;
	return;

//*/
//	static int counter = 0;

/*
	if (ui_frameskip>=g_iSkip)
	{
		if (ui_vdpsimple)
		{
			uip_displaybank(-1);
		}
		else
		{
			//ui_rendertoscreen();	// plot ui_newscreen to screen
			ui_rendertoscreen2();
		}
		ui_frameskip = 0;
	}
	ui_frameskip++;
return;
//*/

	if (ui_plotfield)
	{
		ui_rendertoscreen2();	// plot ui_newscreen to screen
	}

	/*
	if (ui_frameskip == 0)
	{
		// dynamic frame skipping
		counter++;
		if (sound_feedback >= 0)
		{
			ui_actualskip = counter;
			counter = 0;
		}
	}
	else
	{
		ui_actualskip = ui_frameskip;
	}
	*/

/*
	if (ui_plotfield)
	{
		ui_rendertoscreen();	// plot ui_newscreen to screen
	}

	if (ui_frameskip == 0)
	{
		// dynamic frame skipping
		counter++;
		if (sound_feedback >= 0)
		{
			ui_actualskip = counter;
			counter = 0;
		}
	}
	else
	{
		ui_actualskip = ui_frameskip;
	}
*/
}

void ui_final(void)
{
}

void ui_log_debug3(const char *text, ...)
{
}

void ui_log_debug2(const char *text, ...)
{
}

void ui_log_debug1(const char *text, ...)
{
}

void ui_log_user(const char *text, ...)
{
}

void ui_log_verbose(const char *text, ...)
{
}

void ui_log_normal(const char *text, ...)
{
}

void ui_log_critical(const char *text, ...)
{
}

void ui_log_request(const char *text, ...)
{
}

void ui_err(const char *text, ...)
{
    static char buff[4096];
    va_list args;

    va_start(args, text);
    vsnprintf(buff, 4096, text, args);
	va_end(args);

	YPSPVideo_ScreenError(buff);
}

void ui_musiclog(uint8 *data, unsigned int length)
{
}


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void uip_copyscreenblocks(uint32 *pScr);
void uip_copyscreenblocks16(uint16 *pScr);

void ui_rendertoscreen2(void)
{
	//char szFree[256];
	if (pTempFrame != 0)
	{
		//YPSPVideo_Fillvram(0x0);
		//sprintf(szFree, "Used: %d Free: %d", YPSPKernel_GetUsedMemory(), YPSPKernel_GetFreeMemory());
		//YPSPVideo_PrintString(0, 0, 0xffff, szFree, 0);

		//vdp_renderframe(pTempFrame + (8 * 320) + 8, 320);
		uiplot_checkpalcache(1);

		static int iLastRes = 0;
		int frameoffset;
		int w;
		uint16 *pVRAM;
		if(vdp_reg[12] & 1)
		{
			frameoffset = ((320+16) * (8));
			w = 320;
			if (!iLastRes)
			{
				//YPSPVideo_Fillvram(0x0);
				//YPSPVideo_ScreenFlip();
				//YPSPVideo_Fillvram(0x0);
				iLastRes = 1;
				g_iClearScreen = 1;
			}
		}
		else
		{
			frameoffset = ((320+16) * (8));
			w = 256;
			if (iLastRes)
			{
				//YPSPVideo_Fillvram(0x0);
				//YPSPVideo_ScreenFlip();
				//YPSPVideo_Fillvram(0x0);
				iLastRes = 0;
				g_iClearScreen = 1;
			}
		}

		switch (g_iCurScreenScale)
		{
			case 1:	g_iCurScreenOffsetX = ((480-364)>>1); g_iCurScreenOffsetY = 0; g_iCurScreenWidthX = 480-g_iCurScreenOffsetX; g_iCurScreenWidthY = 272; break;
			case 2:	g_iCurScreenOffsetX = ((480-388)>>1); g_iCurScreenOffsetY = 0; g_iCurScreenWidthX = 480-g_iCurScreenOffsetX; g_iCurScreenWidthY = 272; break;
			case 3:	g_iCurScreenOffsetX = ((480-424)>>1); g_iCurScreenOffsetY = 0; g_iCurScreenWidthX = 480-g_iCurScreenOffsetX; g_iCurScreenWidthY = 272; break;
			case 4:	g_iCurScreenOffsetX = 0; g_iCurScreenOffsetY = 0; g_iCurScreenWidthX = 480; g_iCurScreenWidthY = 272; break;
			default: g_iCurScreenOffsetX = ((480-w)>>1); g_iCurScreenOffsetY = ((272-vdp_vislines)>>1); g_iCurScreenWidthX = 480-g_iCurScreenOffsetX; g_iCurScreenWidthY = 272-g_iCurScreenOffsetY; break;
		}

		//uiplot_convertdata16_pspscreendirect(pTempFrame+frameoffset, pVRAM, w, (320+16), vdp_vislines, LINESIZE);
		uiplot_convertdata16_pspscreendirect(pTempFrame+frameoffset, w, (320+16), vdp_vislines);

		uip_displaybank(-1);
	}
}

void ui_rendertoscreen(void)
{
	if (pTempFrame != 0)
	{
//static int bDrawn = 0;
//static int isFrame = 0;

//isFrame++;
		/*
		if (pTempFrame32 == 0)
		{
			pTempFrame32 = (uint32*)malloc(sizeof(uint32) *320*224);
		}
		//*/
		/*
		if (pTempFrame16 == 0)
		{
			pTempFrame16 = (uint16*)malloc(sizeof(uint16) *320*224);
		}
		*/


		uip_singlebank();	/// get current bank

		uiplot_checkpalcache(1);
		//uiplot_convertdata32(pTempFrame, pTempFrame32, 320*224);
		//uiplot_convertdata16(pTempFrame, pTempFrame16, 320*224);

		uint16 *pVRAM = (uint16*)YPSPVideo_GetVramAddr(80, 24);
		//uiplot_convertdata16_pspscreendirect(pTempFrame, pVRAM, 320, 224, LINESIZE);

		// draw page
//if (bDrawn == 0)
//{
		//uip_copyscreenblocks(pTempFrame32);
		//uip_copyscreenblocks16(pTempFrame16);
//if (isFrame > 400/3)
//{
/*	int file = -1;
	{
		r_sceWrite(file, (unsigned char *)pTempFrame, 320*224*4);
		r_sceClose(file);
		bDrawn = 1;
	}
}
		uip_displaybank(-1);
}
*/
		uip_displaybank(-1);
	}
/*
	uint16 **oldscreenpp = uip_whichbank()? &ui_screen1 : &ui_screen0;
	uint16 *scrtmp;
	uint16 *newlinedata, *oldlinedata;

	unsigned int line;
	unsigned int nominalwidth = (vdp_reg[12] & 1) ? 320 : 256;
	unsigned int yoffset = (vdp_reg[1] & 1 << 3) ? 0 : 8;
	unsigned int xoffset = (vdp_reg[12] & 1) ? 0 : 32;
	
	uint8 *screen;

	for (line = 0; line < vdp_vislines; line++)
	{
		newlinedata = ui_newscreen + line * 320;
		oldlinedata = *oldscreenpp + line * 320;

		screen = (ui_uipinfo.screenmem_w + 320 + xoffset * 2 +
				  ui_uipinfo.linewidth * (120 + line + yoffset));

		uiplot_render16_x1(newlinedata, oldlinedata, screen, nominalwidth);
	}

	uip_displaybank(-1);
	uip_vsync();

	//swap ui_screenX and ui_newscreen pointers
	scrtmp = *oldscreenpp;
	*oldscreenpp = ui_newscreen;
	ui_newscreen = scrtmp;
*/
}

void ui_newframe(void)
{
	static int hmode = 0;
	//static int skipcount = 0;
	//static char frameplots[1024];   // 60 for NTSC, 50 for PAL
	//static unsigned int frameplots_i = 0;
	//unsigned int i;
	//int fps;

	// we store whether we plotted the previous field as this is important
	// for doing weave interlacing - we don't want to weave two distant fields
	//ui_plotprevfield = ui_plotfield;

	/*
	if (frameplots_i > vdp_framerate)
	{
		frameplots_i = 0;
	}
	*/

	static int skipcounter = 0;
	if (((vdp_reg[12] >> 1) & 3) && vdp_oddframe)
	{
		// interlace mode, and we're about to do an odd field - we always leave
		//   ui_plotfield alone so we do fields in pairs
	}
	else
	{
		ui_plotfield = 0;
		if (ui_frameskip == 0)
		{
			++skipcounter;
			if (sound_feedback != -1)
			{
				ui_plotfield = 1;
				YPSP_ReadControllerData();
				skipcounter = 0;
			}
		}
		else
		{
			if (cpu68k_frames % ui_frameskip == 0)
			{
				ui_plotfield = 1;
			}
		}
	}

	if (!ui_plotfield)
	{
		//skipcount++;
		//frameplots[frameplots_i++] = 0;
		return;
	}

	if (hmode != (vdp_reg[12] & 1))
	{
	    ui_clearnext = 2;
	}

	if (ui_clearnext)
	{
		// horizontal size has changed, so clear whole screen and setup
		// borders etc again
		ui_clearnext--;
		hmode = vdp_reg[12] & 1;
		memset(uip_whichbank()? ui_screen1 : ui_screen0, 0, sizeof(ui_screen[0]));
		uip_clearscreen();
	}
	
	// count the frames we've plotted in the last vdp_framerate real frames
	/*
	fps = 0;
	for (i = 0; i < vdp_framerate; i++)
	{
		if (frameplots[i])
		{
			fps++;
		}
	}

	frameplots[frameplots_i++] = 1;

	*/

	// draw help text
	ui_updatestatustext(YFALSE);
}

static void ui_simpleplot(void)
{
	/*

	unsigned int line;
	unsigned int width = (vdp_reg[12] & 1) ? 320 : 256;

	uint16 *pVRAM;

	// cell mode - entire frame done here
	uiplot_checkpalcache(0);
	vdp_renderframe(pTempFrame + (8 * (320 + 16)) + 8, 320 + 16); //plot frame
	for (line = 0; line < vdp_vislines; line++)
	{
		pVRAM = (uint16*)pgGetVramAddr(0, line);
		uiplot_convertdata16_pspscreendirect(pTempFrame + 8 + (line + 8) * (320 + 16), pVRAM, width, (320 + 16), 1, PSP_SCREEN_WSTRIDE);
		//uiplot_convertdata16(gfx + 8 + (line + 8) * (320 + 16),
		//ui_newscreen + line * 320, width);
	}
	//*/

//*

	static int iLastRes = 0;
	int frameoffset;
	int w;
	uint16 *pVRAM;
	if(vdp_reg[12] & 1)
	{
		frameoffset = ((320+16) * (8));
		w = 320;
		if (!iLastRes)
		{
			YPSPVideo_Fillvram(0x0);
			YPSPVideo_ScreenFlip();
			YPSPVideo_Fillvram(0x0);
			iLastRes = 1;
		}
	}
	else
	{
		frameoffset = ((320+16) * (8));
		w = 256;
		if (iLastRes)
		{
			YPSPVideo_Fillvram(0x0);
			YPSPVideo_ScreenFlip();
			YPSPVideo_Fillvram(0x0);
			iLastRes = 0;
		}
	}

	uiplot_checkpalcache(0);
	vdp_renderframe(pTempFrame+frameoffset, 320 + 16); //plot frame
	//uiplot_convertdata16_pspscreendirect(pTempFrame+frameoffset, pVRAM, w, (320+16), vdp_vislines, LINESIZE);
//*/
}

extern unsigned int gen_autodetect;//= 1; /* 0 = no, 1 = yes */
extern unsigned int gen_region;// = 0;

unsigned int ui_enable_auto_sram = 0;
unsigned int ui_soundspeedfix = 0;

YPSP_DirEntry	s_oCurDirEntry;

int iSelectedRom = 0;
int iRegion = -1;
int iCPUSpeed = 0;

#define MAKERGB5551(r, g, b)			\
			tr = (r & 0xff);			\
			tg = ((g >> 8) & 0xff);		\
			tb = ((b >> 16) & 0xff);

void GradientFillvram()
{
//	YPSPVideo_Fillvram(0x90909090);

	uint16 *pVRAM = 0;
	YINT32	x, y;

	YUINT16 r = 0, g = 0, b = 40;
	YUINT16 tr = 0, tg = 0, tb = 0;
	YINT32 iFlip = 0;

	for (y = 0; y < 272; ++y)
	{
		pVRAM = (uint16 *)YPSPVideo_GetVramAddr(0, y);
		for (x = 0; x < 480; ++x)
		{
			MAKERGB5551(r, g, b);
			*pVRAM = ((uint16)((tr >> 3) | (tg << 2) | (tb << 7)));
			++pVRAM;
		}

		iFlip = 1 - iFlip;
		if (iFlip)
		{
			++g;
		}
		else
		{
			++r;
		}
	}

	r = 255; g = 255; b = 255;
	pVRAM = (uint16 *)YPSPVideo_GetVramAddr(0, 26);
	for (x = 0; x < 480; ++x)
	{
		MAKERGB5551(r, g, b);
		*pVRAM = ((uint16)((tr >> 3) | (tg << 2) | (tb << 7)));
		++pVRAM;

		--b;
		--g;
		if (b == 0)
		{
			b = 1;
			g = 1;
			--r;
		}
	}
}

void ui_refreshromlist(void)
{
	int	iFirstRom = iSelectedRom - 10;
	if (iFirstRom < 0)
	{
		iFirstRom = 0;
	}
	int iLastRom = iSelectedRom + 10;

	if ((iLastRom-iFirstRom) < 20)
	{
		iLastRom = iFirstRom + 20;
	}
	if (iLastRom > g_iNumRoms)
	{
		iLastRom = g_iNumRoms;
	}

	GradientFillvram();
	//YPSPVideo_Fillvram(0x90909090);

	char szTemp[256];
	sprintf(szTemp, "PSPGenesis");
	YPSPVideo_PrintString(8,0,0xff,szTemp,0);

	sprintf(szTemp, "[CPU: %dMhz] [auto sram: %s]",
		((iCPUSpeed)?333:222),
		(ui_enable_auto_sram?"y":"n")
		);
	YPSPVideo_PrintString(10*9,0,0xee,szTemp,0);

	sprintf(szTemp, "Current Dir: [ %s ]", g_szCurPath);
	YPSPVideo_PrintString(8,1*12,0xffff,szTemp,0);

	int iCurRom;
	int iCurRow = 3;
	unsigned long color;
	for (iCurRom = iFirstRom; iCurRom < iLastRom; ++iCurRom)
	{
		color = 0xffff;
		if (iCurRom == iSelectedRom)
		{
			color = 0xff80;
			if (iRegion < 0)
			{
				sprintf(szTemp, "AUTO->");
			}
			else if (iRegion == 0)
			{
				sprintf(szTemp, " JAP->");
			}
			else if (iRegion == 1)
			{
				sprintf(szTemp, "UDEF->");
			}
			else if (iRegion == 2)
			{
				sprintf(szTemp, " USA->");
			}
			else if (iRegion == 3)
			{
				sprintf(szTemp, " EUR->");
			}
			YPSPVideo_PrintString(2*10,iCurRow*12,color,szTemp,0);
		}
		if (g_acIsDir[iCurRom])
		{
			YPSPVideo_PrintString(7*10,iCurRow*12,color,"[Dir]",0);
			YPSPVideo_PrintString(10*10,iCurRow*12,color,g_szRomList[iCurRom],0);
		}
		else
		{
			YPSPVideo_PrintString(7*10,iCurRow*12,color,g_szRomList[iCurRom],0);
		}
		++iCurRow;

	}
	
	YPSPVideo_ScreenFlipVSync();
}

YBOOL GetFileList(YCHAR8 *a_szFullPath)
{
	YINT32	iStrLen;

	g_iNumRoms = 0;

	YINT32			iDirHandle = YPSP_DirOpen(a_szFullPath);
	if (iDirHandle < 0)
	{
		YPSPVideo_ScreenError("Failed to read directory!");
		iSelectedRom = 0;
		return YFALSE;
	}

	while (YPSP_DirRead(iDirHandle, &s_oCurDirEntry) > 0)
	{
		if ((s_oCurDirEntry.type == YPSPFILE_TYPE_DIR) && (stricmp(s_oCurDirEntry.name, ".") == 0))
		{
			continue;
		}

		if (s_oCurDirEntry.type != YPSPFILE_TYPE_DIR)
		{
			iStrLen = strlen(s_oCurDirEntry.name);
			if (iStrLen < 4)
			{
				continue;
			}
			if (stricmp(&s_oCurDirEntry.name[iStrLen-4], ".smd") != 0
			 && stricmp(&s_oCurDirEntry.name[iStrLen-4], ".bin") != 0)
			{
				continue;
			}
		}

		strcpy(g_szRomList[g_iNumRoms], s_oCurDirEntry.name);
		g_acIsDir[g_iNumRoms] = (s_oCurDirEntry.type == YPSPFILE_TYPE_DIR) ? 1 : 0;

		++g_iNumRoms;
		if (g_iNumRoms >= MAX_ROMS)
		{
			break;
		}
	}

	YPSP_DirClose(iDirHandle);

	if (iSelectedRom < 0)
	{
		iSelectedRom = 0;
	}
	if (iSelectedRom >= g_iNumRoms)
	{
		iSelectedRom = g_iNumRoms - 1;
	}

	return YTRUE;
}

YBOOL ChangeDir(YCHAR8 *a_szCurDir, YCHAR8 *a_szDir)
{
	if (stricmp(a_szDir, "..") == 0)
	{
		YINT32 i = strlen(a_szCurDir);
		if (i > 0)
		{
			// strip off trailing slash first
			if (a_szCurDir[i-1] == '/' || a_szCurDir[i-1] == '\\')
			{
				a_szCurDir[i-1] = 0;
				--i;
			}

			// now strip off last directory
			for (; i > 0 && a_szCurDir[i-1]!='/' && a_szCurDir[i-1]!='\\'; --i)
			{
				a_szCurDir[i-1] = 0;
			}
		}
	}
	else
	{
		YINT32	iStrLen = strlen(a_szCurDir);
		strcpy(&a_szCurDir[iStrLen], a_szDir);
		// add trailing slash
		iStrLen = strlen(a_szCurDir);
		a_szCurDir[iStrLen] = '/';
		a_szCurDir[iStrLen+1] = 0;
	}
	return YTRUE;
}


extern int SaveSRAM();
extern int LoadSRAM();
void InitGU();

void ui_updateMenu(void)
{
	int			i;

	sound_stop();

	YPSP_PowerSetClockFrequency(222, 222, 111);

	YPSPVideo_SetVideoMode(0, YPSP_SCREEN_WIDTH, YPSP_SCREEN_HEIGHT);
	YPSPVideo_ScreenFrame(YPSPVIDEO_SCREENFRAMEMODE_OFFSCREEN, 0);

	if (ui_enable_auto_sram)
	{
		SaveSRAM();
	}

	ui_resetstatustext();

	GradientFillvram();
	//YPSPVideo_Fillvram(0x90909090);
	YPSPVideo_ScreenFlipVSync();


	ui_soundspeedfix = 0;
	ui_vdpsimple = 0;
	sound_on = 1;

	iCPUSpeed = 1;
	ui_enable_auto_sram = 1;

	GetFileList(g_szCurPath);
	ui_refreshromlist();

	while (1)
	{
		YPSP_ControllerData *pPadData = YPSP_ReadControllerData();

		if (pPadData->buttons & YPSPINPUT_UP)
		{
			--iSelectedRom;
			if (iSelectedRom < 0)
			{
				iSelectedRom = 0;
			}
			ui_refreshromlist();
			YPSPVideo_WaitVSyncN(4);
		}
		else if (pPadData->buttons & YPSPINPUT_DOWN)
		{
			++iSelectedRom;
			if (iSelectedRom >= g_iNumRoms)
			{
				iSelectedRom = g_iNumRoms-1;
			}
			ui_refreshromlist();
			YPSPVideo_WaitVSyncN(4);
		}
		else if (pPadData->buttons & YPSPINPUT_LEFT)
		{
			iSelectedRom -= 8;
			if (iSelectedRom < 0)
			{
				iSelectedRom = 0;
			}
			ui_refreshromlist();
			YPSPVideo_WaitVSyncN(4);
		}
		else if (pPadData->buttons & YPSPINPUT_RIGHT)
		{
			iSelectedRom += 8;
			if (iSelectedRom >= g_iNumRoms)
			{
				iSelectedRom = g_iNumRoms-1;
			}
			ui_refreshromlist();
			YPSPVideo_WaitVSyncN(4);
		}
		else if (pPadData->buttons & YPSPINPUT_TRIANGLE)
		{
			ui_enable_auto_sram = 1 - ui_enable_auto_sram;
			ui_refreshromlist();
			YPSPVideo_WaitVSyncN(10);
		}
		else if (pPadData->buttons & YPSPINPUT_CROSS)
		{
			iCPUSpeed = 1 - iCPUSpeed;
			ui_refreshromlist();
			YPSPVideo_WaitVSyncN(10);
		}
		else if (pPadData->buttons & YPSPINPUT_RTRIGGER)
		{
			++iRegion;
			if (iRegion > 3)
			{
				iRegion = -1;
			}
			if (iRegion == 1)
			{
				iRegion = 2;
			}

			if (iRegion < 0)
			{
				iRegion = -1;
				gen_region = 0;
				gen_autodetect = 1;
			}
			else
			{
				gen_region = iRegion;
				gen_autodetect = 0;
			}

			for (i = 0; i < 8; ++i)
			{
				ui_refreshromlist();
			}
		}
		else if (pPadData->buttons & YPSPINPUT_CIRCLE)
		{
			if (g_acIsDir[iSelectedRom])
			{
				ChangeDir(g_szCurPath, g_szRomList[iSelectedRom]);
				iSelectedRom = 1;
				GetFileList(g_szCurPath);
				ui_refreshromlist();
				YPSPVideo_WaitVSyncN(10);
			}
			else
			{
				YINT32	iPathLen = strlen(g_szCurPath);
				strcpy(g_szCurRom, g_szCurPath);
				strcpy(&g_szCurRom[iPathLen], g_szRomList[iSelectedRom]);

				char *p = gen_loadimage(g_szCurRom);
				if (p)
				{
					YPSPVideo_Fillvram(0x90909090);
					YPSPVideo_PrintString(0,0,0xffff,"Failed to Load ROM",0);
					YPSPVideo_ScreenFlipVSync();
					YPSPVideo_WaitVSyncN(10);
				}
				else
				{
					g_pCurSaveState->uValid = 0;

					if (ui_enable_auto_sram)
					{
						LoadSRAM();
					}

					if (iCPUSpeed)
					{
						YPSP_PowerSetClockFrequency(333, 333, 166);
					}
					else
					{
						YPSP_PowerSetClockFrequency(222, 222, 111);
					}
					YPSPVideo_WaitVSync(10);

					YPSPVideo_ClearScreens(0);
					ui_state = 2;

					InitGU();

					break;
				}
			}
		}

		ui_refreshromlist();

	}
}

void InitGU()
{
	sceKernelDcacheWritebackAll();
	sceGuInit();
	sceGuStart(0,list);

#define BUF_WIDTH (512)
#define SCR_WIDTH (YPSP_SCREEN_WIDTH)
#define SCR_HEIGHT (YPSP_SCREEN_HEIGHT)
#define PIXEL_SIZE (4) // change this if you change to another screenmode
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) // zbuffer seems to be 16-bit?

	sceGuDrawBuffer(GE_PSM_8888,(void*)0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)FRAME_SIZE,BUF_WIDTH);
	sceGuDepthBuffer((void*)(FRAME_SIZE*2),BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_STATE_SCISSOR);
	sceGuFrontFace(GE_FACE_CW);
	sceGuEnable(GU_STATE_TEXTURE);
	sceGuClear(GE_CLEAR_COLOR|GE_CLEAR_DEPTH);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_DISPLAY_ON);
}
