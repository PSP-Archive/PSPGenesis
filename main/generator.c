/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
//#include <signal.h>
#include <errno.h>
#include <malloc.h>

#include "generator.h"
#include "snprintf.h"

#include "ui.h"
#include "memz80.h"
#include "cpu68k.h"
#include "mem68k.h"
#include "cpuz80.h"
#include "vdp.h"
#include "gensound.h"
#include <pspkernel.h>

#include <pspgu.h>

#ifdef ALLEGRO
#include "allegro.h"
#endif

PSP_MODULE_INFO("PSPGenesis", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);

/*** variables externed in generator.h ***/

unsigned int __attribute__((aligned(16))) list[262144];

unsigned int gen_quit = 0;
unsigned int gen_debugmode = 0;
unsigned int gen_loglevel = 1;  /* 2 = NORMAL, 1 = CRITICAL */
unsigned int gen_autodetect = 1; /* 0 = no, 1 = yes */
unsigned int gen_musiclog = 0; /* 0 = no, 1 = GYM, 2 = GNM */
t_cartinfo gen_cartinfo;
char gen_leafname[128];

static int gen_freerom = 0;
static int gen_freesram = 0;

unsigned int gen_sixcont = 0; // 6-button controller enable
unsigned int gen_multitap = 0; // mutlitap enable

unsigned int gen_region = 0;

/*** forward references ***/

void gen_nicetext(char *out, char *in, unsigned int size);
uint16 gen_checksum(uint8 *start, unsigned int length);
void gen_setupcartinfo(void);


/*** Signal handler ***/

/*
RETSIGTYPE gen_sighandler(int signum)
{
  if (gen_debugmode) {
    if (signum == SIGINT) {
      if (gen_quit) {
        LOG_CRITICAL(("Bye!"));
        ui_final();
        ui_err("Exiting");
      } else {
        LOG_REQUEST(("Ping - current PC = 0x%X", regs.pc));
      }
      gen_quit = 1;
    }
  } else {
    ui_final();
    exit(0);
  }
  signal(signum, gen_sighandler);
}
*/

const int ciMaxRomSize = (1024*1024*4) + 1024;

uint8 *preallocrombuf = 0;
uint8 *preallocnewpbuf = 0;


YINT32 OnExit(void)
{
	return 0;
}

/*** Program entry point ***/

int main(int argc, char *argv[])
{
  int retval;
  t_sr test;

	//SetupCallbacks();

	YPSPLib_Init(argc, argv);

	YPSPKernel_CreateExitCallback(OnExit);

	YPSPVideo_SetVideoMode(0, YPSP_SCREEN_WIDTH, YPSP_SCREEN_HEIGHT);
	YPSPVideo_ScreenFrame(YPSPVIDEO_SCREENFRAMEMODE_OFFSCREEN, 0);


	/*
	int ret = pgaInit();
	if (ret)
	{
		pgFillvram(0x90909090);
		pgPrint(0,0,0xffff,"pga subsystem initialization failed.");
		pgScreenFlipV();
		while(1);
	}
	*/


	{
		if (0 == preallocrombuf)
		{
			preallocrombuf = (uint8*)malloc(ciMaxRomSize);
		}
		if (0 == preallocnewpbuf)
		{
			preallocnewpbuf = (uint8*)malloc(ciMaxRomSize);
		}
	}

  test.sr_int = 0;
  test.sr_struct.c = 1;
  if (test.sr_int != 1) {
    //r_printf("%s: compilation variable BYTES_HIGHFIRST not set correctly\n", argv[0]);
	while(1);
  }

  /* initialise user interface */
  if ((retval = ui_init()))
    return retval;

  /* initialise 68k memory system */
  if ((retval = mem68k_init()))
    ui_err("Failed to initialise mem68k module (%d)", retval);

  /* initialise z80 memory system */
  if ((retval = memz80_init()))
    ui_err("Failed to initialise memz80 module (%d)", retval);

  /* initialise vdp system */
  if ((retval = vdp_init()))
    ui_err("Failed to initialise vdp module (%d)", retval);

  /* initialise cpu system */
  if ((retval = cpu68k_init()))
    ui_err("Failed to initialise cpu68k module (%d)", retval);

  /* initialise z80 cpu system */
  if ((retval = cpuz80_init()))
    ui_err("Failed to initialise cpuz80 module (%d)", retval);

  /* initialise sound system */
  if ((retval = sound_init()))
    ui_err("Failed to initialise sound module (%d)", retval);

  //signal(SIGINT, gen_sighandler);

  /* enter user interface loop */
  ui_loop();

  sceGuTerm();

  YPSPLib_DeInit();

  return 0;
}

#ifdef ALLEGRO
END_OF_MAIN();
#endif

/*** gen_reset - reset system ***/

void gen_reset(void)
{
  gen_quit = 0;

  mem68k_init();

  vdp_reset();
  
  cpu68k_reset();
  
  cpuz80_reset();
  
  if (sound_reset()) {
    ui_err("sound failure");
  }

}

/*** gen_softreset - reset system ***/

void gen_softreset(void)
{
  cpu68k_reset();
}

/*** gen_loadimage - load ROM image ***/

char *gen_loadimage(const char *filename)
{
	int imagetype;//, file, bytes, bytesleft;
	//  struct stat statbuf;
	const char *extension;
	//uint8 *buffer;
	unsigned int blocks, x, i;
	uint8 *newp;
	char *p;

	/* Remove current file */
	if (cpu68k_rom)
	{
		if (gen_freerom)
		{
			//free(cpu68k_rom);
		}
		cpu68k_rom = NULL;
	}

  /* Load file */
/*  if (stat(filename, &statbuf) != 0)
    return ("Unable to stat file.");
  cpu68k_romlen = statbuf.st_size;
*/
/*
//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM.");
//pgScreenFlipV();
	if ((file = sceIoOpen(filename, PSP_O_RDONLY)) < 0)
	{
		cpu68k_rom = NULL;
		cpu68k_romlen = 0;
		return ("Unable to open file.");
	}
//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM..");
//pgScreenFlipV();

	// get rom len (seek doesnt seem to work)
	cpu68k_romlen = 0;
	//cpu68k_romlen = 524800;//r_sceLseek(file, 0, PSP_SEEK_END);
	//cpu68k_romlen = sceIoLseek(file, 0, PSP_SEEK_END);

	{
		const int ibufsize = 64000;
		//unsigned char *pszbuf = (unsigned char *)malloc(ibufsize);
		do {
			if ((bytes = sceIoRead(file, preallocrombuf, ibufsize)) <= 0)
			{
				break;
			}
			cpu68k_romlen += bytes;
		}
		while (1);
	}
	sceIoClose(file);

	// reopen file (Since seek doesnt work...)
	if ((file = sceIoOpen(filename, PSP_O_RDONLY)) < 0)
	{
		cpu68k_rom = NULL;
		cpu68k_romlen = 0;
		return ("Unable to open file.");
	}
*/

//char szTempBuf[256];
//sprintf(szTempBuf, "Rom: %s, len: %d", filename, cpu68k_romlen);
//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM...");
//pgPrint(0,0,0xffff,szTempBuf);
//pgScreenFlipV();

	gen_freerom = 1;
	memset(preallocrombuf, 0, ciMaxRomSize);
	memset(preallocnewpbuf, 0, ciMaxRomSize);

	cpu68k_rom = preallocrombuf;
	int iRomLen = YPSPFile_ReadFileToBuffer(filename, cpu68k_rom, ciMaxRomSize);
	if (iRomLen < 0x200)
	{
		return ("File is too small");
	}
	cpu68k_romlen = (unsigned int)iRomLen;

	//r_printf("rom length: %d", cpu68k_romlen);

	// allocate enough memory plus 16 bytes for disassembler to cope
	// with the last instruction
	//if ((cpu68k_rom = malloc(cpu68k_romlen + 16)) == NULL)
	{
		//cpu68k_romlen = 0;
		//return ("Out of memory!");
	}

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM....");
//pgScreenFlipV();
	/*
	bytesleft = cpu68k_romlen;
	do {
		if ((bytes = sceIoRead(file, buffer, bytesleft)) <= 0)
		{
			break;
		}
		buffer += bytes;
		bytesleft -= bytes;
	}
	while (bytesleft >= 0);
	//sceIoRead(file, buffer, bytesleft);
//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM.....");
//pgScreenFlipV();

	sceIoClose(file);
	*/

	imagetype = 1;                // BIN file by default

	// SMD file format check - Richard Bannister
	if ((cpu68k_rom[8] == 0xAA) && (cpu68k_rom[9] == 0xBB) &&
		cpu68k_rom[10] == 0x06)
	{
		imagetype = 2;              //SMD file
	}
	// check for interleaved 'SEGA'
	if (cpu68k_rom[0x280] == 'E' && cpu68k_rom[0x281] == 'A' &&
		cpu68k_rom[0x2280] == 'S' && cpu68k_rom[0x2281] == 'G')
	{
		imagetype = 2;              //SMD file
	}

	// Check extension is not wrong
	extension = filename + strlen(filename) - 3;
	if (extension > filename)
	{
		if (!strcasecmp(extension, "smd") && (imagetype != 2))
		{
			//r_printf("File extension (smd) does not match detected type (bin)");
		}
		if (!strcasecmp(extension, "bin") && (imagetype != 1))
		{
			//r_printf("File extension (bin) does not match detected type (smd)");
		}
	}
//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM.......");
//pgScreenFlipV();

	// convert to standard BIN file format
	switch (imagetype)
	{
		case 1:	//BIN
		{
			break;
		}
		case 2:	//SMD
		{
			blocks = (cpu68k_romlen - 512) / 16384;
			if (blocks * 16384 + 512 != cpu68k_romlen)
			{
				return ("Image is corrupt.");
			}
			/*
			if ((newp = malloc(cpu68k_romlen - 512)) == NULL)
			{
				cpu68k_rom = NULL;
				cpu68k_romlen = 0;
				return ("Out of memory!");
			}
			*/
			newp = preallocnewpbuf;

			for (i = 0; i < blocks; i++)
			{
				for (x = 0; x < 8192; x++)
				{
					newp[i * 16384 + x * 2 + 0] = cpu68k_rom[512 + i * 16384 + 8192 + x];
					newp[i * 16384 + x * 2 + 1] = cpu68k_rom[512 + i * 16384 + x];
				}
			}
			//free(cpu68k_rom);
			cpu68k_rom = newp;
			cpu68k_romlen -= 512;
			break;
		}
		default:
		{
			return ("Unknown image type");
			break;
		}
	}

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM........");
//pgScreenFlipV();
//is this icky?
	if ((p = strrchr(filename, '/')) == NULL &&
		(p = strrchr(filename, '\\')) == NULL)
	{
		snprintf(gen_leafname, sizeof(gen_leafname), "%s", filename);
	}
	else
	{
		snprintf(gen_leafname, sizeof(gen_leafname), "%s", p + 1);
	}
	if ((p = strrchr(gen_leafname, '.')) != NULL)
	{
		if ((!strcasecmp(p, ".smd")) || (!strcasecmp(p, ".bin")))
		{
			*p = '\0';
		}
	}
	if (gen_leafname[0] == '\0')
	{
		snprintf(gen_leafname, sizeof(gen_leafname), "rom");
	}

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM..........");
//pgScreenFlipV();
	gen_setupcartinfo();

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM.........");
//pgScreenFlipV();
	if (gen_autodetect)
	{
		if (gen_cartinfo.flag_usa)
		{
			gen_region = 2;
		}
		else if (gen_cartinfo.flag_europe)
		{
			gen_region = 3;
		}
		else if (gen_cartinfo.flag_japan)
		{
			gen_region = 0;
		}
		else
		{
			gen_region = (!gen_cartinfo.flag_usa && !gen_cartinfo.flag_japan && gen_cartinfo.flag_europe) ? 1 : 0;
		}
	}

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM...........");
//pgScreenFlipV();
	// reset system
	gen_reset();

//pgFillvram(0x90909090);
//pgPrint(0,0,0xffff,"Loading ROM............");
//pgScreenFlipV();
	if (gen_cartinfo.checksum != (cpu68k_rom[0x18e] << 8 | cpu68k_rom[0x18f]))
	{
		//r_printf("Warning: Checksum does not match in ROM (%04X)",
		//			(cpu68k_rom[0x18e] << 8 | cpu68k_rom[0x18f]));
	}

	//r_printf("\nLoaded '%s'/'%s\n' (%s %04X %s)\n", gen_cartinfo.name_domestic,
	//				gen_cartinfo.name_overseas, gen_cartinfo.version,
	//				gen_cartinfo.checksum, gen_cartinfo.country);
	return NULL;
}

/* setup to run from ROM in memory */

void gen_loadmemrom(const char *rom, int romlen)
{
  cpu68k_rom = (char *)rom; /* I won't alter it, promise */
  cpu68k_romlen = romlen;
  gen_freerom = 0;
  gen_setupcartinfo();
  gen_reset();
}

/* setup gen_cartinfo from current loaded rom */

void gen_setupcartinfo(void)
{
  unsigned int i;
  char *p;

  memset(&gen_cartinfo, 0, sizeof(gen_cartinfo));
  gen_nicetext(gen_cartinfo.console, (char *)(cpu68k_rom + 0x100), 16);
  gen_nicetext(gen_cartinfo.copyright, (char *)(cpu68k_rom + 0x110), 16);
  gen_nicetext(gen_cartinfo.name_domestic, (char *)(cpu68k_rom + 0x120), 48);
  gen_nicetext(gen_cartinfo.name_overseas, (char *)(cpu68k_rom + 0x150), 48);
  if (cpu68k_rom[0x180] == 'G' && cpu68k_rom[0x181] == 'M') {
    gen_cartinfo.prodtype = pt_game;
  } else if (cpu68k_rom[0x180] == 'A' && cpu68k_rom[0x181] == 'I') {
    gen_cartinfo.prodtype = pt_education;
  } else {
    gen_cartinfo.prodtype = pt_unknown;
  }
  gen_nicetext(gen_cartinfo.version, (char *)(cpu68k_rom + 0x182), 12);
  gen_cartinfo.checksum = gen_checksum(((uint8 *)cpu68k_rom) + 0x200,
                                       cpu68k_romlen - 0x200);
  gen_nicetext(gen_cartinfo.memo, (char *)(cpu68k_rom + 0x1C8), 28);
  for (i = 0x1f0; i < 0x1ff; i++) {
    if (cpu68k_rom[i] == 'J')
      gen_cartinfo.flag_japan = 1;
    if (cpu68k_rom[i] == 'U')
      gen_cartinfo.flag_usa = 1;
    if (cpu68k_rom[i] == 'E')
      gen_cartinfo.flag_europe = 1;
  }
  if (cpu68k_rom[0x1f0] >= '1' && cpu68k_rom[0x1f0] <= '9') {
    gen_cartinfo.hardware = cpu68k_rom[0x1f0] - '0';
  } else if (cpu68k_rom[0x1f0] >= 'A' && cpu68k_rom[0x1f0] <= 'F') {
    gen_cartinfo.hardware = cpu68k_rom[0x1f0] - 'A' + 10;
  }
  p = gen_cartinfo.country;
  for (i = 0x1f0; i < 0x200; i++) {
    if (cpu68k_rom[i] != 0 && cpu68k_rom[i] != 32)
      *p++ = cpu68k_rom[i];
  }
  *p = '\0';

  if(cpu68k_srambuff) {
  	if(gen_freesram) free(cpu68k_srambuff);
	cpu68k_sram = NULL;
	cpu68k_srambuff = NULL;
  }

  // Setup SRAM - Added by Nick Van Veen. Based on code from DGen.
  if(cpu68k_rom[0x1b1] == 'A' && cpu68k_rom[0x1b0] == 'R' && 1) {
  	 cpu68k_sramstart =	cpu68k_rom[0x1b4] << 24 | cpu68k_rom[0x1b5] << 16 |
				cpu68k_rom[0x1b6] << 8  | cpu68k_rom[0x1b7];
     	 cpu68k_sramend = 	cpu68k_rom[0x1b8] << 24 | cpu68k_rom[0x1b9] << 16 |
  	         		cpu68k_rom[0x1ba] << 8  | cpu68k_rom[0x1bb];
	// Make sure start is even, end is odd, for alignment
	// A ROM that I came across had the start and end bytes of
	// the save ram the same and wouldn't work.  Fix this as seen
	// fit, I know it could probably use some work. [PKH]
 	if(cpu68k_sramstart != cpu68k_sramend) {
        	if(cpu68k_sramstart & 1) --cpu68k_sramstart;
        	if(!(cpu68k_sramend & 1)) ++cpu68k_sramend;

		cpu68k_sramend++;

        	cpu68k_sramlen = cpu68k_sramend - cpu68k_sramstart;
			cpu68k_srambuff = (uint8 *)memalign(16,cpu68k_sramlen+64); // first 64 bytes is game name
        	cpu68k_sram = cpu68k_srambuff + 64;

		gen_freesram = 1;

		// If save RAM does not overlap main ROM, set it active by default since
		// a few games can't manage to properly switch it on/off.
		if(cpu68k_sramstart >= cpu68k_romlen)
	  		cpu68k_sramactive = 1;
	} else {
        	cpu68k_sramstart = cpu68k_sramlen = cpu68k_sramend = 0;
        	cpu68k_sram = NULL;
			cpu68k_srambuff = NULL;
		cpu68k_sramactive = 0;
	}
  }
  else
  {
 	cpu68k_sramstart = cpu68k_sramlen = cpu68k_sramend = 0;
	cpu68k_sram = NULL;
	cpu68k_srambuff = NULL;
	cpu68k_sramactive = 0;
  }

}

/*
void Detect_Country_Genesis(void)
{
	int c_tab[3] = {4, 1, 8};
	int gm_tab[3] = {1, 0, 1};
	int cm_tab[3] = {0, 0, 1};
	int i, coun = 0;
	char c;
	
	if (!strnicmp((char *) &Rom_Data[0x1F0], "eur", 3)) coun |= 8;
	else if (!strnicmp((char *) &Rom_Data[0x1F0], "usa", 3)) coun |= 4;
	else if (!strnicmp((char *) &Rom_Data[0x1F0], "jap", 3)) coun |= 1;
	else for(i = 0; i < 4; i++)
	{
		c = toupper(Rom_Data[0x1F0 + i]);
		
		if (c == 'U') coun |= 4;
		else if (c == 'J') coun |= 1;
		else if (c == 'E') coun |= 8;
		else if (c < 16) coun |= c;
		else if ((c >= '0') && (c <= '9')) coun |= c - '0';
		else if ((c >= 'A') && (c <= 'F')) coun |= c - 'A' + 10;
	}

	if (coun & c_tab[Country_Order[0]])
	{
		Game_Mode = gm_tab[Country_Order[0]];
		CPU_Mode = cm_tab[Country_Order[0]];
	}
	else if (coun & c_tab[Country_Order[1]])
	{
		Game_Mode = gm_tab[Country_Order[1]];
		CPU_Mode = cm_tab[Country_Order[1]];
	}
	else if (coun & c_tab[Country_Order[2]])
	{
		Game_Mode = gm_tab[Country_Order[2]];
		CPU_Mode = cm_tab[Country_Order[2]];
	}
	else if (coun & 2)
	{
		Game_Mode = 0;
		CPU_Mode = 1;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
	}

	if (Game_Mode)
	{
		if (CPU_Mode) Put_Info("Europe system (50 FPS)", 1500);
		else Put_Info("USA system (60 FPS)", 1500);
	}
	else
	{
		if (CPU_Mode) Put_Info("Japan system (50 FPS)", 1500);
		else Put_Info("Japan system (60 FPS)", 1500);
	}

	if (CPU_Mode)
	{
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;
	}
	else
	{
		_32X_VDP.Mode |= 0x8000;
		VDP_Status &= 0xFFFE;
	}
}
*/

/*** get_nicetext - take a string, remove spaces and capitalise ***/

void gen_nicetext(char *out, char *in, unsigned int size)
{
  int flag, i;
  int c;
  char *start = out;

  flag = 0;                     /* set if within word, e.g. make lowercase */
  i = size;                     /* maximum number of chars in input */
  while ((c = *in++) && --i > 0) {
    if (isalpha(c)) {
      if (!flag) {
        /* make uppercase */
        flag = 1;
        if (islower(c))
          *out++ = c - 'z' + 'Z';
        else
          *out++ = c;
      } else {
        /* make lowercase */
        if (isupper(c))
          *out++ = (c) - 'Z' + 'z';
        else
          *out++ = c;
      }
    } else if (c == ' ') {
      if (flag)
        *out++ = c;
      flag = 0;
    } else if (isprint(c) && c != '\t') {
      flag = 0;
      *out++ = c;
    }
  }
  while (out > start && out[-1] == ' ')
    out--;
  *out++ = '\0';
}

/*** gen_checksum - get Genesis-style checksum of memory block ***/

uint16 gen_checksum(uint8 *start, unsigned int length)
{
  uint16 checksum = 0;

  if (length & 1) {
    length &= ~1;
    //LOG_CRITICAL(("checksum routines given odd length (%d)", length));
  }

  for (; length; length -= 2, start += 2) {
    checksum += start[0] << 8;
    checksum += start[1];
  }
  return checksum;
}
