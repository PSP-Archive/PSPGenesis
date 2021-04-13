#ifndef __PSPGENESIS_STATE_H__
#define __PSPGENESIS_STATE_H__

#include "cpu68k.h"
#include "cpuz80.h"
#include "../cmz80/z80.h"

typedef struct
{
	uint8		statehdr[8];
	uint8 		vdp_vram[64*1024];
	uint8 		vdp_cram[128];
	uint8 		vdp_vsram[80];
	uint8 		vdp_reg[25];
	uint8		gen_region;
	uint8 		vdp_ctrlflag;
	uint8 		vdp_code;
	uint16		vdp_first;
	uint16		vdp_second;
	uint32		vdp_dmabytes;
	uint16		vdp_address;
	uint8		cpu68k_ram[0x10000];
	t_regs		regs;
	uint8		cpuz80_ram[0x2000];
	uint8		cpuz80_active;
	uint8		cpuz80_resetting;
	uint32		cpuz80_bank;
	Z80_Regs	cpuz80_regs;
	uint32		cpuz80_after_EI;
	uint8		YM2612_Regs[0x200];
	uint32		PSG_Regs[8];
	uint32		uValid;
} PSPGenesis_SaveState;


time_t state_date(const int slot);
int state_load(const int slot);
int state_save(const int slot);
int state_loadfile(const char *filename);
int state_savefile(const char *filename);

void state_write8(const char *mod, const char *name, uint8 instance,
                              
				  uint8 *data, uint32 size);
void state_write16(const char *mod, const char *name, uint8 instance,
                                     uint16 *data, uint32 size);
void state_write32(const char *mod, const char *name, uint8 instance,
                                     uint32 *data, uint32 size);

int LoadStateFromMemory(PSPGenesis_SaveState *a_pState);
int SaveStateToMemory(PSPGenesis_SaveState *a_pState);

int LoadStateFromDisk(PSPGenesis_SaveState *a_pState, int a_iSlot);
int SaveStateToDisk(PSPGenesis_SaveState *a_pState, int a_iSlot);

#endif //__PSPGENESIS_STATE_H__

