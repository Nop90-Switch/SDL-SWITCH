ifeq ($(strip $(DEVKITA64)),)
$(error "Please set DEVKITA64 in your environment. export DEVKITA64=<path to>devkitA64")
endif

include $(DEVKITA64)/base_tools

TARGET_LIB = libSDL_ttf.a

OBJS =	SDL_ttf.o 

LIBNX	:= $(DEVKITPRO)/libnx
INCLUDES = -I./include -I$(LIBNX)/include 

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv8-a -mtp=soft -fPIE
CFLAGS		:=	-g -Wall -O2 -ffast-math $(ARCH)
CFLAGS		+=	$(INCLUDES) -DARM11 -DSWITCH -D__SWITCH__  \
				-I$(LIBNX)/../portlibs/armv8-a/include -I$(LIBNX)/../portlibs/SWITCH/include/SDL \
				-I$(LIBNX)/../portlibs/armv8-a/include/freetype2
CXXFLAGS	:=  $(CFLAGS) -fno-rtti -std=gnu++11
ASFLAGS		:=	-g $(ARCH)
LDFLAGS		:=	-specs=switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

CFLAGS		+= -Wno-unused-variable

LIBS	:= -freetype -lSDL -lnx

all: $(OBJS) $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(AR) -rc $@ $^

install: $(TARGET_LIB)
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/lib"
	@cp  $(TARGET_LIB) "$(DEVKITPRO)/portlibs/switch/lib"
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/include"
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@cp SDL_ttf.h "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@echo "Installed!"

clean:
	@rm -rf $(OBJS) $(TARGET_LIB)