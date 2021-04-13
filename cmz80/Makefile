
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
OUTPUTLIBDIR	= ../Lib
TARGETDIRS		= $(OUTPUTLIBDIR) $(BUILDDIR)

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
LD          = $(PREFIX)-ld
OBJDUMP     = $(PREFIX)-objdump
RM          = $(TOOLSBINDIR)/rm
MKDIR		= $(TOOLSBINDIR)/mkdir
ECHO		= $(TOOLSBINDIR)/echo
TOUCH		= $(TOOLSBINDIR)/touch
SED			= $(TOOLSBINDIR)/sed

#-------------

CFLAGS		= -O3 -G0 -Wall -fomit-frame-pointer
CPPFLAGS    = -O3 -G0 -Wall -Wa,-al -fno-exceptions -fno-rtti -fno-commona -fomit-frame-pointer
ASFLAGS		= -c -G0 -xassembler -O -fomit-frame-pointer
ARFLAGS		= -cru
DEFS		= -DHAVE_CONFIG_H
LDFLAGS		=
#LDFLAGS		= -Map out.map -Ttext 8900000 -q -O3 -nostdlib

#-----------------------------------------------------------------------------------
YUMEPSPLIBDIR			=	D:/Development/PSP/YumePSPLib
YUMEPSPLIBNAMES			=	YumePSP$(BUILDPOSTFIX)

YUMEPSPLIBS				=	$(addprefix -l,$(YUMEPSPLIBNAMES))
YUMEPSPLIBDEPS			=	$(foreach lib, $(YUMEPSPLIBNAMES), $(YUMEPSPLIBDIR)/Lib/lib$(lib).a)

INCDIRS					+=	$(YUMEPSPLIBDIR)/Source
LIBDIRS					+=	$(YUMEPSPLIBDIR)/Lib

#------------------------------------------------------------------------------------
#-------------

TARGET      = libcmz80

SOURCES		= z80.c
HEADERS		= z80.h
INCDIRS		+= ../hdr ../

OBJS        = $(patsubst %.c, %.o, $(SOURCES))
LIBS		=

#-------------

INCDIRS		+= $(SOURCEDIR)

CFLAGS		+= $(addprefix -I,$(INCDIRS))
#CPPFLAGS	+= $(addprefix -I,$(INCDIRS))
ASFLAGS		+= $(addprefix "-I ",$(INCDIRS))

LDFLAGS		+= $(addprefix -L,$(LIBDIRS))

#------------------------------------------------------------------------------------

.PHONY: all
all:	$(TARGET).a
	@echo Made $(TARGET).a

$(TARGET).a: $(HEADERS) $(OBJS) $(LIBS)
	@echo Archiving $@
	@$(AR) $(ARFLAGS) $@ $(OBJS)
	@$(RANLIB) $@

%.o: %.c $(HEADERS)
	@echo Compiling $<
	@$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

%.o: %.s
	@echo Compiling $<
	@$(AS) $(ASFLAGS) -o $@ $<

.PHONY: clean
clean:
	@echo Cleaning z80 files...
	@$(RM) -rf $(TARGET).a $(OBJS)
	@echo Cleaning done.

.PHONY: rebuildall
rebuildall: clean all

