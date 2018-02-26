ifeq ($(strip $(DEVKITA64)),)
$(error "Please set DEVKITA64 in your environment. export DEVKITA64=<path to>devkitA64")
endif

include $(DEVKITA64)/base_tools

TARGET_LIB = libSDL_image.a

OBJS =	IMG.o \
	IMG_bmp.o \
	IMG_gif.o \
	IMG_jpg.o \
	IMG_lbm.o \
	IMG_pcx.o \
	IMG_png.o \
	IMG_pnm.o \
	IMG_tga.o \
	IMG_tif.o \
	IMG_webp.o \
	IMG_xcf.o \
	IMG_xpm.o \
	IMG_xv.o \
	IMG_xxx.o 

LIBNX	:= $(DEVKITPRO)/libnx
INCLUDES = -I./include -I$(LIBNX)/include 

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv8-a -mtp=soft -fPIE
CFLAGS		:=	-g -Wall -O2 -ffast-math $(ARCH)
CFLAGS		+=	$(INCLUDES) -DARM11 -DSWITCH -D__SWITCH__ -DLOAD_BMP \
				-DLOAD_GIF -DLOAD_LBM -DLOAD_PCX -DLOAD_PNM -DLOAD_TGA -DLOAD_XCF \
				-DLOAD_XPM -DLOAD_XV -DLOAD_JPG -DLOAD_PNG \
				-I$(LIBNX)/../portlibs/armv8-a/include -I$(LIBNX)/../portlibs/switch/include/SDL
CXXFLAGS	:=  $(CFLAGS) -fno-rtti -std=gnu++11
ASFLAGS		:=	-g $(ARCH)
LDFLAGS		:=	-specs=switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

CFLAGS		+= -Wno-unused-variable

LIBS	:= -ljpeg -lSDL -lnx

all: $(OBJS) $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(AR) -rc $@ $^

install: $(TARGET_LIB)
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/lib"
	@cp  $(TARGET_LIB) "$(DEVKITPRO)/portlibs/switch/lib"
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/include"
	@mkdir -p "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@cp SDL_image.h "$(DEVKITPRO)/portlibs/switch/include/SDL"
	@echo "Installed!"

clean:
	@rm -rf $(OBJS) $(TARGET_LIB)