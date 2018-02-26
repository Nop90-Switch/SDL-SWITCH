ifeq ($(strip $(DEVKITA64)),)
$(error "Please set DEVKITA64 in your environment. export DEVKITA64=<path to>devkitA64")
endif

include $(DEVKITA64)/base_tools

TARGET_LIB = libSDL_mixer.a

OBJS =	dynamic_flac.o \
	dynamic_fluidsynth.o \
	dynamic_mod.o \
	dynamic_mp3.o \
	dynamic_ogg.o \
	effect_position.o \
	effect_stereoreverse.o \
	effects_internal.o \
	fluidsynth.o \
	load_aiff.o \
	load_flac.o \
	load_ogg.o \
	load_voc.o \
	mixer.o \
	music.o \
	music_cmd.o \
	music_flac.o \
	music_mad.o \
	music_mod.o \
	music_modplug.o \
	music_ogg.o \
	wavestream.o 

LIBNX	:= $(DEVKITPRO)/libnx
INCLUDES = -I./include -I$(LIBNX)/include 

SUPPORT_TREMOR = false
SUPPORT_VORBIS = false
SUPPORT_MAD = false

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv8-a -mtp=soft -fPIE
CFLAGS		:=	-g -Wall -O2 -ffast-math $(ARCH)
CFLAGS		+=	$(INCLUDES) -DARM11 -DSWITCH -D__SWITCH__  \
				-I$(LIBNX)/../portlibs/armv8-a/include -I$(LIBNX)/../portlibs/SWITCH/include/SDL

ifeq ($(SUPPORT_TREMOR), true)
CFLAGS += -DWAV_MUSIC -DMOD_MUSIC -DLIBMIKMOD_MUSIC -DMP3_MAD_MUSIC -DOGG_MUSIC -DOGG_USE_TREMOR 
endif

ifeq ($(SUPPORT_VORBIS), true)
CFLAGS += -DOGG_MUSIC
endif

ifeq ($(SUPPORT_MAD), true)
CFLAGS += -DMP3_MAD_MUSIC
endif

CXXFLAGS	:=      $(CFLAGS) -fno-rtti -std=gnu++11
ASFLAGS		:=	-g $(ARCH)
LDFLAGS		:=	-specs=switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

CFLAGS		+= -Wno-unused-variable

LIBS	:= -lSDL -lnx

all: $(OBJS) $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(AR) -rc $@ $^

install: $(TARGET_LIB)
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/lib"
	@cp  $(TARGET_LIB) "$(DEVKITPRO)/portlibs/switch/lib"
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/include"
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@cp SDL_mixer.h "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@echo "Installed!"

clean:
	@rm -rf $(OBJS) $(TARGET_LIB)