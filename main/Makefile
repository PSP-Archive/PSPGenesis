
#------------- define the build type
ifndef $(BUILD)
	BUILD = Release
endif

ifneq ($(BUILD), Debug)
ifneq ($(BUILD), Release)
	override BUILD = Release
endif
endif

#------------- directories
ifeq ($(BUILD), Debug)
BUILDDIR		= Debug
endif
ifeq ($(BUILD), Release)
BUILDDIR		= Release
endif

SOURCEDIR		= ../Source
OUTPUTDIR		= ../CD
TARGETDIRS		= $(OUTPUTDIR) $(BUILDDIR)

#------------- build specific defs
ifeq ($(BUILD), Debug)
BUILDPOSTFIX	= D
endif
ifeq ($(BUILD), Release)
BUILDPOSTFIX	=
endif

#-------------

TOP         = D:/Development/Devkits/psp
PSPGCCBINDIR= $(TOP)/bin
LIBDIRS		= $(TOP)/lib $(TOP)/psp/lib $(TOP)/lib/gcc/psp/4.0.0 $(TOP)/psp/sdk/lib
INCDIRS		= $(TOP)/psp/include $(TOP)/psp/sdk/include

TOOLSBINDIR	= D:/Development/Bin

PREFIX      = $(PSPGCCBINDIR)/psp
AS          = $(PREFIX)-gcc
AR			= $(PREFIX)-ar
RANLIB		= $(AR) -s
CC          = $(PREFIX)-gcc
LD          = $(PREFIX)-gcc
OBJDUMP     = $(PREFIX)-objdump
STRIP		= $(PREFIX)-strip
ELF2PBP		= $(PSPGCCBINDIR)/elf2pbp
OUTPATCH	= $(PSPGCCBINDIR)/outpatchn

RM          = $(TOOLSBINDIR)/rm
MKDIR		= $(TOOLSBINDIR)/mkdir
ECHO		= $(TOOLSBINDIR)/echo
TOUCH		= $(TOOLSBINDIR)/touch
SED			= $(TOOLSBINDIR)/sed
CP			= $(TOOLSBINDIR)/cp

#-------------

CFLAGS		= -O3 -G0 -Wall -fomit-frame-pointer
CPPFLAGS    = -O3 -G0 -Wall -Wa,-al -fno-exceptions -fno-rtti -fno-commona -fomit-frame-pointer
ASFLAGS		= -c -G0 -xassembler -O -fomit-frame-pointer
ARFLAGS		= -cru
DEFS		= -DHAVE_CONFIG_H
LDFLAGS		=
#LDFLAGS		= -Map out.map -Ttext 8900000 -q -O3 -nostdlib

#------------------------------------------------------------------------------------
SNSND		= psg
YMSND		= ym2612

INCDIRS	+=	../hdr ../
INCDIRS += ../cpu68k ../$(YMSND) ../cmz80 ../$(SNSND)

#-----------------------------------------------------------------------------------
PSPSDKLIBNAMES			=	pspglue pspdisplay pspge pspctrl pspuser pspkernel psppower pspaudio pspgu
PSPSDKLIBS				=	$(addprefix -l,$(PSPSDKLIBNAMES))

#-----------------------------------------------------------------------------------
YUMEPSPLIBDIR			=	D:/Development/PSP/YumePSPLib
YUMEPSPLIBNAMES			=	YumePSP$(BUILDPOSTFIX)

YUMEPSPLIBS				=	$(addprefix -l,$(YUMEPSPLIBNAMES))
YUMEPSPLIBDEPS			=	$(foreach lib, $(YUMEPSPLIBNAMES), $(YUMEPSPLIBDIR)/Lib/lib$(lib).a)

INCDIRS					+=	$(YUMEPSPLIBDIR)/Source
LIBDIRS					+=	$(YUMEPSPLIBDIR)/Lib

#------------------------------------------------------------------------------------

#-------------

INCDIRS		+= $(SOURCEDIR)

CFLAGS		+= $(addprefix -I,$(INCDIRS))
CPPFLAGS	+= $(addprefix -I,$(INCDIRS))
ASFLAGS		+= $(addprefix "-I ",$(INCDIRS))

LDFLAGS		+= $(addprefix -L,$(LIBDIRS))

#-----------------------------------------------------------------------------------
			  
TARGETS					=	$(OUTPUTDIR)/EBOOT.PBP

PSPGENESIS_SOURCEFILES	= cpu68k.c mem68k.c \
							reg68k.c event.c \
							generator.c state.c cpuz80.c \
							vdp.c gensound.c memz80.c \
							snprintf.c ui-psp.c uip-psp.c \
							uiplot.c gensoundp-psp.c \
							render.c
			  
PSPGENESIS_TARGETOBJS	=	$(patsubst %.c, %.o, $(PSPGENESIS_SOURCEFILES))
PSPGENESIS_TARGETDEPS	=	$(patsubst %.c, %.d, $(PSPGENESIS_SOURCEFILES))

PSPGENESIS_LIBDIRS		=	-L../cpu68k -L../cmz80 -L../$(YMSND) -L../$(SNSND)
PSPGENESIS_LIBS			=	-l68k -lcmz80 \
							-lym2612_gens -lpsg_gens
PSPGENESIS_LIBSDEPS		=	../cpu68k/lib68k.a ../cmz80/libcmz80.a  \
							../$(YMSND)/libym2612_gens.a ../$(SNSND)/libpsg_gens.a

#------------------------------------------------------------------------------------

.PHONY: all
all:	maketree $(TARGETS)
	@echo All Done.

$(sort $(TARGETDIRS)):
	@$(ECHO) mkdir $@
	@$(MKDIR) $@

maketree: $(sort $(TARGETDIRS))

$(OUTPUTDIR)/EBOOT.PBP : $(BUILDDIR)/PSPGenesis.elf
	@echo Generating PBP $@
	@$(ELF2PBP) $< PSPGenesis
	@$(CP) EBOOT.PBP $@
	@$(RM) -rf EBOOT.PBP

$(BUILDDIR)/PSPGenesis.elf : $(PSPGENESIS_TARGETOBJS) $(PSPGENESIS_LIBSDEPS)
	@echo Linking $@
	@$(LD) $(LDFLAGS) $(PSPGENESIS_TARGETOBJS) -o $@ $(PSPGENESIS_LIBDIRS) $(PSPGENESIS_LIBS) $(YUMEPSPLIBS) -lpspdebug -lm -lc -lgcc $(PSPSDKLIBS)

%.o: %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

%.o: %.s
	@echo Compiling $<
	@$(AS) $(ASFLAGS) -o $@ $<

.PHONY: clean
clean:
	@echo Cleaning PSPGenesis files...
	@$(RM) -rf $(PSPGENESIS_TARGETOBJS)
	@$(RM) -rf $(PSPGENESIS_TARGETDEPS)
	@$(RM) -rf $(BUILDDIR)/PSPGenesis.elf
	@$(RM) -rf $(BUILDDIR)/PSPGenesisPatched.elf
	@$(RM) -rf $(OUTPUTDIR)/EBOOT.PBP
	@$(RM) -rf EBOOT.PBP
	@echo Cleaning done.

.PHONY: rebuildall
rebuildall: clean all

#------------------------------------------------------------------------------------
