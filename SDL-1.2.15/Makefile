ifeq ($(strip $(DEVKITA64)),)
$(error "Please set DEVKITA64 in your environment. export DEVKITA84=<path to>devkitA64")
endif

include $(DEVKITA64)/base_tools

TARGET_LIB = libSDL.a

OBJS =	src/SDL.o \
	src/SDL_error.o \
	src/SDL_fatal.o \
	src/audio/SDL_audio.o \
	src/audio/SDL_audiocvt.o \
	src/audio/SDL_audiodev.o \
	src/audio/SDL_mixer.o \
	src/audio/SDL_wave.o \
	src/audio/dummy/SDL_dummyaudio.o \
	src/cdrom/SDL_cdrom.o \
	src/cdrom/dummy/SDL_syscdrom.o \
	src/cpuinfo/SDL_cpuinfo.o \
	src/events/SDL_active.o \
	src/events/SDL_events.o \
	src/events/SDL_expose.o \
	src/events/SDL_keyboard.o \
	src/events/SDL_mouse.o \
	src/events/SDL_quit.o \
	src/events/SDL_resize.o \
	src/file/SDL_rwops.o \
	src/joystick/SDL_joystick.o \
	src/joystick/dummy/SDL_sysjoystick.o \
	src/loadso/dummy/SDL_sysloadso.o \
	src/stdlib/SDL_getenv.o \
	src/stdlib/SDL_iconv.o \
	src/stdlib/SDL_malloc.o \
	src/stdlib/SDL_qsort.o \
	src/stdlib/SDL_stdlib.o \
	src/stdlib/SDL_string.o \
	src/thread/SDL_thread.o \
	src/thread/generic/SDL_syssem.o \
	src/thread/generic/SDL_systhread.o \
	src/thread/generic/SDL_sysmutex.o \
	src/thread/generic/SDL_syscond.o \
	src/timer/SDL_timer.o \
	src/timer/switch/SDL_systimer.o \
	src/video/SDL_blit.o \
	src/video/SDL_blit_0.o \
	src/video/SDL_blit_1.o \
	src/video/SDL_blit_A.o \
	src/video/SDL_blit_N.o \
	src/video/SDL_bmp.o \
	src/video/SDL_cursor.o \
	src/video/SDL_gamma.o \
	src/video/SDL_pixels.o \
	src/video/SDL_RLEaccel.o \
	src/video/SDL_stretch.o \
	src/video/SDL_surface.o \
	src/video/SDL_video.o \
	src/video/SDL_yuv.o \
	src/video/SDL_yuv_sw.o \
	src/video/switch/SDL_switchevents.o \
	src/video/switch/SDL_switchvideo.o \
	src/video/switch/SDL_switchmouse.o \

NXLIB	:= $(DEVKITPRO)/libnx
INCLUDES = -I./include -I$(NXLIB)/include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv8-a -mtp=soft -fPIE
CFLAGS		:=	-g -Wall -O2 -ffast-math $(ARCH)
CFLAGS		+=	$(INCLUDES) -DARM11 -D_SWITCH -D__SWITCH__
CXXFLAGS	:=      $(CFLAGS) -fno-rtti -std=gnu++11
ASFLAGS		:=	-g $(ARCH)
LDFLAGS		:=	-specs=switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) -L$(NXLIB)/lib

CFLAGS		+= -Wno-unused-variable

LIBS	:= -lnx

all: $(OBJS) $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(AR) -rc $@ $^

install: $(TARGET_LIB)
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/lib"
	@cp  $(TARGET_LIB) "$(DEVKITPRO)/portlibs/switch/lib"
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@cp include/*.h "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@echo "Installed!"

clean:
	@rm -rf $(OBJS) $(TARGET_LIB)
	
