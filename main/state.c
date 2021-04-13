/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "generator.h"
#include "snprintf.h"

#include "state.h"
#include "ui.h"
#include "cpu68k.h"
#include "cpuz80.h"
#include "../cmz80/z80.h"
#include "vdp.h"
#include "gensound.h"
//#include "fm.h"

void ui_addstatustext(YCHAR8 *a_szText, YFLOAT32 a_fTime);
extern YCHAR8 g_szCurRom[YPSPFILE_MAX_PATH];

extern int YM2612_Save(unsigned char SAVE[0x200]);
extern int YM2612_Restore(unsigned char SAVE[0x200]);
extern void PSG_Save_State(void);
extern void PSG_Restore_State(void);
extern unsigned int PSG_Save[8];


typedef struct _t_statelist {
  struct _t_statelist *next;
  char *mod;
  char *name;
  uint8 instance;
  uint32 bytes;
  uint32 size;
  uint8 *data;
} t_statelist;

FILE *state_outputfile;         /* the file handle to place data blocks */
uint8 state_transfermode;       /* 0 = save, 1 = load */
uint8 state_major;              /* major version */
uint8 state_minor;              /* minor version */
t_statelist *state_statelist;   /* loaded state */

int SaveStateToMemory(PSPGenesis_SaveState *a_pState)
{
	memcpy(a_pState->vdp_vram, vdp_vram, 64*1024);
	memcpy(a_pState->vdp_cram, vdp_cram, 128);
	memcpy(a_pState->vdp_vsram, vdp_vsram, 80);
	memcpy(a_pState->vdp_reg, vdp_reg, 25);
	a_pState->gen_region = gen_region;
	a_pState->vdp_ctrlflag = vdp_ctrlflag;
	a_pState->vdp_code = (uint8)vdp_code;
	a_pState->vdp_first = vdp_first;
	a_pState->vdp_second = vdp_second;
	a_pState->vdp_dmabytes = vdp_dmabytes;
	a_pState->vdp_address = vdp_address;
	memcpy(a_pState->cpu68k_ram, cpu68k_ram, 0x10000);
	memcpy(&a_pState->regs, &regs, sizeof(t_regs));
	memcpy(a_pState->cpuz80_ram, cpuz80_ram, 0x2000);
	a_pState->cpuz80_active = cpuz80_active;
	a_pState->cpuz80_resetting = cpuz80_resetting;
	a_pState->cpuz80_bank = cpuz80_bank;
	memcpy(&a_pState->cpuz80_regs, Z80_Context, sizeof(Z80_Regs));
	a_pState->cpuz80_after_EI = after_EI;

	YM2612_Save(a_pState->YM2612_Regs);

	PSG_Save_State();
	memcpy(a_pState->PSG_Regs, PSG_Save, 32);	

	a_pState->uValid = 1;

	return 1;
}

int LoadStateFromMemory(PSPGenesis_SaveState *a_pState)
{
	if (!a_pState->uValid)
	{
		return 0;
	}

	memcpy(vdp_vram, a_pState->vdp_vram, 64*1024);
	memcpy(vdp_cram, a_pState->vdp_cram, 128);
	memcpy(vdp_vsram, a_pState->vdp_vsram, 80);
	memcpy(vdp_reg, a_pState->vdp_reg, 25);
	gen_region = a_pState->gen_region;
	vdp_code = a_pState->vdp_code;
	vdp_first = a_pState->vdp_first;
	vdp_second = a_pState->vdp_second;
	vdp_dmabytes = a_pState->vdp_dmabytes;
	vdp_address = a_pState->vdp_address;
	memcpy(cpu68k_ram, a_pState->cpu68k_ram, 0x10000);
	memcpy(&regs, &a_pState->regs, sizeof(t_regs));
	memcpy(cpuz80_ram, a_pState->cpuz80_ram, 0x2000);
	cpuz80_active = a_pState->cpuz80_active;
	cpuz80_resetting = a_pState->cpuz80_resetting;
	cpuz80_bank = a_pState->cpuz80_bank;
	memcpy(Z80_Context, &a_pState->cpuz80_regs, sizeof(Z80_Regs));
	after_EI = a_pState->cpuz80_after_EI;

	YM2612_Restore(a_pState->YM2612_Regs);

	memcpy(PSG_Save, a_pState->PSG_Regs, 32);
	PSG_Restore_State();

	return 1;
}

int LoadStateFromDisk(PSPGenesis_SaveState *a_pState, int a_iSlot)
{
	if (g_szCurRom[0] == 0)
	{
		return 0;
	}

	char szBaseName[YPSPFILE_MAX_PATH];
	char szFileName[YPSPFILE_MAX_PATH] = "Saves/";
	if (YPSPFile_GetFileBaseName(g_szCurRom, szBaseName, YPSPFILE_MAX_PATH) == YFALSE)
	{
		return 0;
	}
	YINT32 i, iBaseLen = strlen(szBaseName);
	for (i = 0; i < iBaseLen; ++i)
	{
		szFileName[i+6] = szBaseName[i];
		if (szFileName[i+6] == '.')
		{
			++i;
			break;
		}
	}
	sprintf(&szFileName[i+6], "s%02d", a_iSlot);

	YINT32 iStateFile = YPSP_fopen(szFileName, YPSPFILE_O_RDONLY);
	if (iStateFile >= 0)
	{
		YINT64 iLen = YPSP_fseek(iStateFile, 0, YPSPFILE_SEEK_END);
		if (iLen != (sizeof(PSPGenesis_SaveState)))
		{
			YPSP_fclose(iStateFile);
			YPSPVideo_ScreenError("Save State file wrong length");
			return 0;
		}


		YPSP_fseek(iStateFile, 0, YPSPFILE_SEEK_SET);

		YPSP_fread(iStateFile, a_pState, sizeof(PSPGenesis_SaveState));
		YPSP_fclose(iStateFile);

		if (strnicmp(a_pState->statehdr, "YPSPSAVE", 8) != 0)
		{
			YPSPVideo_ScreenError("Invalid Save State Header");
			return -1;
		}
	}

	if (!LoadStateFromMemory(a_pState))
	{
		return 0;
	}
	return 1;
}

int SaveStateToDisk(PSPGenesis_SaveState *a_pState, int a_iSlot)
{
	if (g_szCurRom[0] == 0)
	{
		return 0;
	}

	char szBaseName[YPSPFILE_MAX_PATH];
	char szFileName[YPSPFILE_MAX_PATH] = "Saves/";

	// see if dir exists, if not create
	YINT32	iWorkLen = 0;
	strcpy(szBaseName, YPSPFile_GetWorkingDir());
	iWorkLen = strlen(szBaseName);
	strcpy(&szBaseName[iWorkLen], "Saves");
	YINT32 iDirHandle = YPSP_DirOpen(szBaseName);
	if (iDirHandle < 0)
	{
		YPSPVideo_ScreenError(szBaseName);
		if (YPSP_fmkdir(szBaseName, 777) < 0)
		{
			YPSPVideo_ScreenError("Failed to create save directory");
			return 0;
		}
	}
	else
	{
		YPSP_DirClose(iDirHandle);
	}

	// now make save
	if (YPSPFile_GetFileBaseName(g_szCurRom, szBaseName, YPSPFILE_MAX_PATH) == YFALSE)
	{
		return 0;
	}
	YINT32 i, iBaseLen = strlen(szBaseName);
	for (i = 0; i < iBaseLen; ++i)
	{
		szFileName[i+6] = szBaseName[i];
		if (szFileName[i+6] == '.')
		{
			++i;
			break;
		}
	}
	sprintf(&szFileName[i+6], "s%02d", a_iSlot);


	if (!SaveStateToMemory(a_pState))
	{
		return 0;
	}
	memcpy(a_pState->statehdr, "YPSPSAVE", 8);

	YINT32 iStateFile = YPSP_fopen(szFileName, YPSPFILE_O_RDWR|YPSPFILE_O_CREAT|YPSPFILE_O_TRUNC);
	if (iStateFile >= 0)
	{
		YPSP_fwrite(iStateFile, a_pState, sizeof(PSPGenesis_SaveState));

		YPSP_fclose(iStateFile);
	}
	else
	{
		YPSPVideo_ScreenError("Failed to create Save State file");
	}

	return 1;
}

/*
 * NB:
 * states are only loaded/saved at the end of the frame
 */

/*** state_date - return the modification date or 0 for non-existant ***/

time_t state_date(const int slot)
{
/*
	char filename[256];
  struct stat statbuf;

  snprintf(filename, sizeof(filename), "%s.gt%d", gen_leafname, slot);

  if (stat(filename, &statbuf) != 0)
    return 0;
  return statbuf.st_mtime;
*/
	return 0;
}

/*** state_load - load the given slot ***/

int state_load(const int slot)
{
/*
char filename[256];

  snprintf(filename, sizeof(filename), "%s.gt%d", gen_leafname, slot);
  return state_loadfile(filename);
  */
	return 0;
}

/*** state_save - save to the given slot ***/

int state_save(const int slot)
{
/*
char filename[256];

  snprintf(filename, sizeof(filename), "%s.gt%d", gen_leafname, slot);
  return state_savefile(filename);
*/
	return 0;
}

void state_transfer8(const char *mod, const char *name, uint8 instance,
                     uint8 *data, uint32 size)
{
	return;
	/*
  t_statelist *l;
  uint8 buf[4];
  uint32 i;

  if (state_transfermode == 0) {
    fwrite(mod, strlen(mod)+1, 1, state_outputfile);
    fwrite(name, strlen(name)+1, 1, state_outputfile);
    buf[0] = instance;
    buf[1] = 1;
    fwrite(buf, 2, 1, state_outputfile);
    buf[0] = (size >> 24) & 0xff;
    buf[1] = (size >> 16) & 0xff;
    buf[2] = (size >> 8) & 0xff;
    buf[3] = size & 0xff;
    fwrite(buf, 4, 1, state_outputfile);
    fwrite(data, size, 1, state_outputfile);
  } else {
    for (l = state_statelist; l; l = l->next) {
      if (!strcasecmp(l->mod, mod) && !strcasecmp(l->name, name) &&
          l->instance == instance && l->size == size && l->bytes == 1) {
        for (i = 0; i < size; i++)
          data[i] = l->data[i];
        LOG_VERBOSE(("Loaded %s %s (%d)", mod, name, instance));
        break;
      }
    }
    if (l == NULL) {
      LOG_CRITICAL(("bad %s/%s\n", mod, name));
      memset(data, 0, size);
    }
  }
  */
}

void state_transfer16(const char *mod, const char *name, uint8 instance,
                      uint16 *data, uint32 size)
{
/*
t_statelist *l;
  uint8 buf[4];
  uint32 i;

  if (state_transfermode == 0) {
    fwrite(mod, strlen(mod)+1, 1, state_outputfile);
    fwrite(name, strlen(name)+1, 1, state_outputfile);
    buf[0] = instance;
    buf[1] = 2;
    fwrite(buf, 2, 1, state_outputfile);
    buf[0] = (size >> 24) & 0xff;
    buf[1] = (size >> 16) & 0xff;
    buf[2] = (size >> 8) & 0xff;
    buf[3] = size & 0xff;
    fwrite(buf, 4, 1, state_outputfile);
    for (i = 0; i < size; i++) {
      buf[0] = (data[i] >> 8) & 0xff;
      buf[1] = data[i] & 0xff;
      fwrite(buf, 2, 1, state_outputfile);
    }
  } else {
    for (l = state_statelist; l; l = l->next) {
      if (!strcasecmp(l->mod, mod) && !strcasecmp(l->name, name) &&
          l->instance == instance && l->size == size && l->bytes == 2) {
        for (i = 0; i < size; i++)
          data[i] = ((((uint8 *)l->data)[(i << 1)] << 8) |
                     (((uint8 *)l->data)[(i << 1) + 1]));
        LOG_VERBOSE(("Loaded %s %s (%d)", mod, name, instance));
        break;
      }
    }
    if (l == NULL) {
      LOG_CRITICAL(("bad %s/%s\n", mod, name));
      memset(data, 0, size * 2);
    }
  }
  */
}

void state_transfer32(const char *mod, const char *name, uint8 instance,
                      uint32 *data, uint32 size)
{
/*  t_statelist *l;
  uint8 buf[4];
  uint32 i;

  if (state_transfermode == 0) {
    fwrite(mod, strlen(mod)+1, 1, state_outputfile);
    fwrite(name, strlen(name)+1, 1, state_outputfile);
    buf[0] = instance;
    buf[1] = 4;
    fwrite(buf, 2, 1, state_outputfile);
    buf[0] = (size >> 24) & 0xff;
    buf[1] = (size >> 16) & 0xff;
    buf[2] = (size >> 8) & 0xff;
    buf[3] = size & 0xff;
    fwrite(buf, 4, 1, state_outputfile);
    for (i = 0; i < size; i++) {
      buf[0] = (data[i] >> 24) & 0xff;
      buf[1] = (data[i] >> 16) & 0xff;
      buf[2] = (data[i] >> 8) & 0xff;
      buf[3] = data[i] & 0xff;
      fwrite(buf, 4, 1, state_outputfile);
    }
  } else {
    for (l = state_statelist; l; l = l->next) {
      if (!strcasecmp(l->mod, mod) && !strcasecmp(l->name, name) &&
          l->instance == instance && l->size == size && l->bytes == 4) {
        for (i = 0; i < size; i++)
          data[i] = ((((uint8 *)l->data)[(i << 2)] << 24) |
                     (((uint8 *)l->data)[(i << 2) + 1] << 16) |
                     (((uint8 *)l->data)[(i << 2) + 2] << 8) |
                     (((uint8 *)l->data)[(i << 2) + 3]));
        LOG_VERBOSE(("Loaded %s %s (%d)", mod, name, instance));
        break;
      }
    }
    if (l == NULL) {
      LOG_CRITICAL(("bad %s/%s\n", mod, name));
      memset(data, 0, size * 4);
    }
  }
  */
}

/*** state_dotransfer - do transfer of data, either save or load ***/

int state_dotransfer(unsigned int mode)
{
/*  uint8 i8, i8b;
  uint16 i16;

  (void)i8b;
  state_transfermode = mode;
  state_transfer8("ver", "major", 0, &state_major, 1);
  state_transfer8("ver" ,"minor", 0, &state_minor, 1);
  state_transfer8("vdp", "vram", 0, vdp_vram, LEN_VRAM);
  state_transfer8("vdp", "cram", 0, vdp_cram, LEN_CRAM);
  state_transfer8("vdp", "vsram", 0, vdp_vsram, LEN_VSRAM);
  state_transfer8("vdp", "regs", 0, vdp_reg, 25);
  state_transfer8("vdp", "pal", 0, &vdp_pal, 1);
  state_transfer8("vdp", "overseas", 0, &vdp_overseas, 1);
  state_transfer8("vdp", "ctrlflag", 0, &vdp_ctrlflag, 1);
  state_transfer8("vdp", "code", 0, (uint8 *)&vdp_code, 1);
  state_transfer16("vdp", "first", 0, &vdp_first, 1);
  state_transfer16("vdp", "second", 0, &vdp_second, 1);
  state_transfer32("vdp", "dmabytes", 0, &vdp_dmabytes, 1);
  state_transfer16("vdp", "address", 0, &vdp_address, 1);
  state_transfer8("68k", "ram", 0, cpu68k_ram, 0x10000);
  state_transfer32("68k", "regs", 0, regs.regs, 16);
  state_transfer32("68k", "pc", 0, &regs.pc, 1);
  state_transfer32("68k", "sp", 0, &regs.sp, 1);
  state_transfer16("68k", "sr", 0, &regs.sr.sr_int, 1);
  state_transfer16("68k", "stop", 0, &regs.stop, 1);
  state_transfer16("68k", "pending", 0, &regs.pending, 1);
  state_transfer8("z80", "ram", 0, cpuz80_ram, LEN_SRAM);
  state_transfer8("z80", "active", 0, &cpuz80_active, 1);
  state_transfer8("z80", "resetting", 0, &cpuz80_resetting, 1);
  state_transfer32("z80", "bank", 0, &cpuz80_bank, 1);
#ifdef RAZE
  if (state_transfermode == 0) {
    i16 = z80_get_reg(Z80_REG_AF); state_transfer16("z80", "af", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_BC); state_transfer16("z80", "bc", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_DE); state_transfer16("z80", "de", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_HL); state_transfer16("z80", "hl", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_AF2); state_transfer16("z80", "af2", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_BC2); state_transfer16("z80", "bc2", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_DE2); state_transfer16("z80", "de2", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_HL2); state_transfer16("z80", "hl2", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_IX); state_transfer16("z80", "ix", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_IY); state_transfer16("z80", "iy", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_SP); state_transfer16("z80", "sp", 0, &i16, 1);
    i16 = z80_get_reg(Z80_REG_PC); state_transfer16("z80", "pc", 0, &i16, 1);
    i8 = (z80_get_reg(Z80_REG_IR) >> 8) & 0xff;
    state_transfer8("z80", "i", 0, &i8, 1);
    i8 = z80_get_reg(Z80_REG_IR) & 0xff;
    state_transfer8("z80", "r", 0, &i8, 1);
    i8 = z80_get_reg(Z80_REG_IFF1); state_transfer8("z80", "iff1", 0, &i8, 1);
    i8 = z80_get_reg(Z80_REG_IFF2); state_transfer8("z80", "iff2", 0, &i8, 1);
    i8 = z80_get_reg(Z80_REG_IM); state_transfer8("z80", "im", 0, &i8, 1);
    i8 = z80_get_reg(Z80_REG_Halted);
    state_transfer8("z80", "halted", 0, &i8, 1);
  } else {
    state_transfer16("z80", "af", 0, &i16, 1); z80_set_reg(Z80_REG_AF, i16);
    state_transfer16("z80", "bc", 0, &i16, 1); z80_set_reg(Z80_REG_BC, i16);
    state_transfer16("z80", "de", 0, &i16, 1); z80_set_reg(Z80_REG_DE, i16);
    state_transfer16("z80", "hl", 0, &i16, 1); z80_set_reg(Z80_REG_HL, i16);
    state_transfer16("z80", "af2", 0, &i16, 1); z80_set_reg(Z80_REG_AF2, i16);
    state_transfer16("z80", "bc2", 0, &i16, 1); z80_set_reg(Z80_REG_BC2, i16);
    state_transfer16("z80", "de2", 0, &i16, 1); z80_set_reg(Z80_REG_DE2, i16);
    state_transfer16("z80", "hl2", 0, &i16, 1); z80_set_reg(Z80_REG_HL2, i16);
    state_transfer16("z80", "ix", 0, &i16, 1); z80_set_reg(Z80_REG_IX, i16);
    state_transfer16("z80", "iy", 0, &i16, 1); z80_set_reg(Z80_REG_IY, i16);
    state_transfer16("z80", "sp", 0, &i16, 1); z80_set_reg(Z80_REG_SP, i16);
    state_transfer16("z80", "pc", 0, &i16, 1); z80_set_reg(Z80_REG_PC, i16);
    state_transfer8("z80", "i", 0, &i8, 1);
    state_transfer8("z80", "r", 0, &i8b, 1);
    z80_set_reg(Z80_REG_IR, ((uint16)i8 << 8) | i8b);
    state_transfer8("z80", "iff1", 0, &i8, 1); z80_set_reg(Z80_REG_IFF1, i8);
    state_transfer8("z80", "iff2", 0, &i8, 1); z80_set_reg(Z80_REG_IFF2, i8);
    state_transfer8("z80", "im", 0, &i8, 1); z80_set_reg(Z80_REG_IM, i8);
    state_transfer8("z80", "halted", 0, &i8, 1);
    z80_set_reg(Z80_REG_Halted, i8);
  }
#else
  state_transfer16("z80", "af", 0, &cpuz80_z80.z80af, 1);
  state_transfer16("z80", "bc", 0, &cpuz80_z80.z80bc, 1);
  state_transfer16("z80", "de", 0, &cpuz80_z80.z80de, 1);
  state_transfer16("z80", "hl", 0, &cpuz80_z80.z80hl, 1);
  state_transfer16("z80", "af2", 0, &cpuz80_z80.z80afprime, 1);
  state_transfer16("z80", "bc2", 0, &cpuz80_z80.z80bcprime, 1);
  state_transfer16("z80", "de2", 0, &cpuz80_z80.z80deprime, 1);
  state_transfer16("z80", "hl2", 0, &cpuz80_z80.z80hlprime, 1);
  state_transfer16("z80", "ix", 0, &cpuz80_z80.z80ix, 1);
  state_transfer16("z80", "iy", 0, &cpuz80_z80.z80iy, 1);
  state_transfer16("z80", "sp", 0, &cpuz80_z80.z80sp, 1);
  state_transfer16("z80", "pc", 0, &cpuz80_z80.z80pc, 1);
  state_transfer8("z80", "i", 0, &cpuz80_z80.z80i, 1);
  state_transfer8("z80", "r", 0, &cpuz80_z80.z80r, 1);
  if (state_transfermode == 0) {
    i8 = cpuz80_z80.z80inInterrupt;
    state_transfer8("z80", "iff1", 0, &i8, 1);
    i8 = cpuz80_z80.z80interruptState;
    state_transfer8("z80", "iff2", 0, &i8, 1);
    i8 = cpuz80_z80.z80interruptMode;
    state_transfer8("z80", "im", 0, &i8, 1);
    i8 = cpuz80_z80.z80halted;
    state_transfer8("z80", "halted", 0, &i8, 1);
  } else {
    state_transfer8("z80", "iff1", 0, &i8, 1);
    cpuz80_z80.z80inInterrupt = i8;
    state_transfer8("z80", "iff2", 0, &i8, 1);
    cpuz80_z80.z80interruptState = i8;
    state_transfer8("z80", "im", 0, &i8, 1);
    cpuz80_z80.z80interruptMode = i8;
    state_transfer8("z80", "halted", 0, &i8, 1);
    cpuz80_z80.z80halted = i8;
  }
#endif
  YM2612_save_state();
*/

  /* XXX: FIX ME!
     Z80_REG_IRQVector,   0x00 to 0xff
     Z80_REG_IRQLine,     boolean - 1 or 0
   */

  /* XXX: FIX ME!
     c -> z80intAddr = z80intAddr;
     c -> z80nmiAddr = z80nmiAddr;
   */

return 0;
}  

/*** state_savefile - save to the given filename */

int state_savefile(const char *filename)
{
/*
	if ((state_outputfile = fopen(filename, "wb")) == NULL) {
    LOG_CRITICAL(("Failed to open '%s' for writing: %s",
                  filename, strerror(errno)));
    return -1;
  }
//  fprintf(state_outputfile, "Generator " VERSION " saved state\n");
  state_major = 2;
  state_minor = 0;
  state_dotransfer(0);
  fclose(state_outputfile);
*/
	return 0;
}

/*** state_loadfile - load the given filename ***/

int state_loadfile(const char *filename)
{
/*  char *blk;
  uint8 *p, *e;
  struct stat statbuf;
  FILE *f;
  t_statelist *ent;

  if (stat(filename, &statbuf) != 0) {
    errno = ENOENT;
    return -1;
  }

  if ((blk = malloc(statbuf.st_size)) == NULL) {
    LOG_CRITICAL(("Failed to allocate memory whilst loading '%s'", filename));
    return -1;
  }
  if ((f = fopen(filename, "rb")) == NULL) {
    LOG_CRITICAL(("Failed to open '%s': %s", filename, strerror(errno)));
    return -1;
  }
  if (fread(blk, statbuf.st_size, 1, f) != 1) {
    if (feof(f)) {
      LOG_CRITICAL(("EOF whilst reading save state file '%s'", filename));
      return -1;
    }
    LOG_CRITICAL(("Error whilst reading save state file '%s': %s", filename,
                  strerror(errno)));
    return -1;
  }
  fclose(f);

  p = blk;
  e = blk + statbuf.st_size;

  while (p < e && *p++ != '\n') ;
  if (p >= e)
    goto OVERRUN;

  state_statelist = NULL;
  for (;;) {
    if (e == p)
      break;
    if ((ent = malloc(sizeof(t_statelist))) == NULL)
      ui_err("out of memory");
    if ((e-p) < 8)
      goto OVERRUN;
    ent->mod = p;
    while (p < e && *p++) ;
    if ((e-p) < 7)
      goto OVERRUN;
    ent->name = p;
    while (p < e && *p++) ;
    if ((e-p) < 6)
      goto OVERRUN;
    ent->instance = p[0];
    ent->bytes = p[1];
    ent->size = (p[2] << 24) | (p[3] << 16) | (p[4] << 8) | p[5];
    if ((e-p) < (int)(ent->bytes * ent->size))
      goto OVERRUN;
    p+= 6;
    ent->data = p;
    p+= ent->bytes * ent->size;
    ent->next = state_statelist;
    state_statelist = ent;
  }

  gen_reset();

  state_dotransfer(1);

  if (state_major != 2) {
    LOG_CRITICAL(("Save state file '%s' is version %d, and we're version 2",
                  filename, state_major));
    errno = EINVAL;
    return -1;
  }

  vdp_setupvideo();
  vdp_dmabusy = vdp_dmabytes > 0 ? 1 : 0;
  cpuz80_updatecontext();
  return 0;
OVERRUN:
  LOG_CRITICAL(("Invalid state file '%s': overrun encountered", filename));
  errno = EINVAL;
  return -1;
  */
	return 0;
}

int SaveState()
{
	return 0;
}

int LoadState()
{
	return 0;
}

int SaveSRAM()
{
	// Only attempt to save if ROM uses SRAM
	if(!cpu68k_sramactive)
		return 0;

	if (cpu68k_sramlen > 64*1024)
	{
		// something wrong
		return -1;
	}

	if (g_szCurRom[0] == 0)
	{
		return 0;
	}

	{
		YPSPVideo_ScreenDebug("Saving SRAM");
	}

	char szBaseName[YPSPFILE_MAX_PATH];
	char szFileName[YPSPFILE_MAX_PATH] = "Saves/";

	// see if dir exists, if not create
	YINT32	iWorkLen = 0;
	strcpy(szBaseName, YPSPFile_GetWorkingDir());
	iWorkLen = strlen(szBaseName);
	strcpy(&szBaseName[iWorkLen], "Saves");
	YINT32 iDirHandle = YPSP_DirOpen(szBaseName);
	if (iDirHandle < 0)
	{
		YPSPVideo_ScreenError(szBaseName);
		if (YPSP_fmkdir(szBaseName, 777) < 0)
		{
			YPSPVideo_ScreenError("Failed to create save directory");
			return -1;
		}
	}
	else
	{
		YPSP_DirClose(iDirHandle);
	}

	// now make save
	if (YPSPFile_GetFileBaseName(g_szCurRom, szBaseName, YPSPFILE_MAX_PATH) == YFALSE)
	{
		return -1;
	}
	YINT32 i, iBaseLen = strlen(szBaseName);
	for (i = 0; i < iBaseLen; ++i)
	{
		szFileName[i+6] = szBaseName[i];
		if (szFileName[i+6] == '.')
		{
			++i;
			break;
		}
	}
	sprintf(&szFileName[i+6], "srm");

	YINT32 iSRAMFile = YPSP_fopen(szFileName, YPSPFILE_O_RDWR|YPSPFILE_O_CREAT|YPSPFILE_O_TRUNC);
	if (iSRAMFile >= 0)
	{
		YUINT8	aHeader[8] = "YPSPSRAM";
		YPSP_fwrite(iSRAMFile, aHeader, 8);
		YPSP_fwrite(iSRAMFile, cpu68k_srambuff, cpu68k_sramlen);

		YPSP_fclose(iSRAMFile);
	}
	else
	{
		YPSPVideo_ScreenError("Failed to create SRAM file");
	}

	return 0;
}

int LoadSRAM()
{
	// Only attempt to load if ROM uses SRAM
	if(!cpu68k_sramactive)
		return 0;

	if (cpu68k_sramlen > 128*1024)
	{
		char szSRAMError[256];
		sprintf(szSRAMError, "SRAM Len: %d exceeds 128k", cpu68k_sramlen);
		YPSPVideo_ScreenError(szSRAMError);
		// something wrong
		return -1;
	}

	if (g_szCurRom[0] == 0)
	{
		return 0;
	}

	char szBaseName[YPSPFILE_MAX_PATH];
	char szFileName[YPSPFILE_MAX_PATH] = "Saves/";
	if (YPSPFile_GetFileBaseName(g_szCurRom, szBaseName, YPSPFILE_MAX_PATH) == YFALSE)
	{
		return 0;
	}
	YINT32 i, iBaseLen = strlen(szBaseName);
	for (i = 0; i < iBaseLen; ++i)
	{
		szFileName[i+6] = szBaseName[i];
		if (szFileName[i+6] == '.')
		{
			++i;
			break;
		}
	}
	sprintf(&szFileName[i+6], "srm");

	YINT32 iSRAMFile = YPSP_fopen(szFileName, YPSPFILE_O_RDONLY);
	if (iSRAMFile >= 0)
	{
		YINT64 iLen = YPSP_fseek(iSRAMFile, 0, YPSPFILE_SEEK_END);
		if (iLen != (cpu68k_sramlen+8))
		{
			YPSP_fclose(iSRAMFile);
			YPSPVideo_ScreenError("SRAM file wrong length");
			return -1;
		}


		YPSP_fseek(iSRAMFile, 0, YPSPFILE_SEEK_SET);

		YUINT8	aHeader[9];
		YPSP_fread(iSRAMFile, aHeader, 8);
		aHeader[8] = 0;

		if (strcmp(aHeader, "YPSPSRAM") != 0)
		{
			YPSP_fclose(iSRAMFile);
			YPSPVideo_ScreenError("Invalid SRAM Header");
			return -1;
		}

		YPSP_fread(iSRAMFile, cpu68k_srambuff, cpu68k_sramlen);
		YPSP_fclose(iSRAMFile);

		{
			char szStatus[256];
			sprintf(szStatus, "Loaded %s", szFileName);
			ui_addstatustext(szStatus, 3.0f);
		}
	}
	else
	{
		//YPSPVideo_ScreenError("Failed to open SRAM file");
	}

	return 0;
}
