SDL              	:= SDL
SDL_VERSION      	:= $(SDL)-1.2.15

SDL_TTF             := SDL_ttf
SDL_TTF_VERSION     := $(SDL_TTF)-2.0.11
SDL_TTF_SRC         := $(SDL_TTF_VERSION).tar.gz
SDL_TTF_DOWNLOAD    := http://www.libsdl.org/projects/SDL_ttf/release/SDL_ttf-2.0.11.tar.gz

SDL_IMAGE           := SDL_image
SDL_IMAGE_VERSION   := $(SDL_IMAGE)-1.2.12
SDL_IMAGE_SRC       := $(SDL_IMAGE_VERSION).tar.gz
SDL_IMAGE_DOWNLOAD  := "http://www.libsdl.org/projects/SDL_image/release/SDL_image-1.2.12.tar.gz"

SDL_MIXER           := SDL_mixer
SDL_MIXER_VERSION   := $(SDL_MIXER)-1.2.12
SDL_MIXER_SRC       := $(SDL_MIXER_VERSION).tar.gz
SDL_MIXER_DOWNLOAD  := "http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-1.2.12.tar.gz"

SDL_GFX           := SDL_gfx
SDL_GFX_VERSION   := $(SDL_GFX)-2.0.25
SDL_GFX_SRC       := $(SDL_GFX_VERSION).tar.gz
SDL_GFX_DOWNLOAD  := "http://netix.dl.sourceforge.net/project/sdlgfx/SDL_gfx-2.0.25.tar.gz"

SDL_SOUND           := SDL_sound
SDL_SOUND_VERSION   := $(SDL_SOUND)-1.0.3
SDL_SOUND_SRC     := $(SDL_SOUND_VERSION).tar.gz
SDL_SOUND_DOWNLOAD  := "https://www.icculus.org/SDL_sound/downloads/SDL_sound-1.0.3.tar.gz"

SDL_NET           := SDL_net
SDL_NET_VERSION   := $(SDL_NET)-1.2.8
SDL_NET_SRC       := $(SDL_NET_VERSION).tar.gz
SDL_NET_DOWNLOAD  := "http://www.libsdl.org/projects/SDL_net/release/SDL_net-1.2.8.tar.gz"

.PHONY: all old_all install clean download \
        $(SDL) \
        $(SDL_TTF) \
        $(SDL_MIXER) \
        $(SDL_IMAGE) \
        $(SDL_GFX) 
		
all: SDL SDL_ttf SDL_mixer SDL_image SDL_gfx install
	@echo "Finished!"

old_all:
	@echo "Please choose one of the following targets:"
	@echo "  $(SDL)"
	@echo "  $(SDL_TTF)"
	@echo "  $(SDL_MIXER)"
	@echo "  $(SDL_IMAGE)"
	@echo "  $(SDL_GFX)"

DOWNLOAD = wget --no-check-certificate -O "$(1)" "$(2)" || curl -Lo "$(1)" "$(2)"

$(SDL_TTF_SRC):
	$(call DOWNLOAD,$@,$(SDL_TTF_DOWNLOAD))
	
$(SDL_IMAGE_SRC):
	$(call DOWNLOAD,$@,$(SDL_IMAGE_DOWNLOAD))
    
$(SDL_MIXER_SRC):
	$(call DOWNLOAD,$@,$(SDL_MIXER_DOWNLOAD))

$(SDL_GFX_SRC):
	$(call DOWNLOAD,$@,$(SDL_GFX_DOWNLOAD))

$(SDL):
	@cd $(SDL_VERSION) 
	@$(MAKE) -C $(SDL_VERSION) 
	
$(SDL_TTF): $(SDL_TTF_SRC)
	@[ -d $(SDL_TTF_VERSION) ] || tar -xzf $<
	@cd $(SDL_TTF_VERSION) && \
	cp ../Makefiles/SDL_ttf.Makefile Makefile
	@$(MAKE) -C $(SDL_TTF_VERSION)

$(SDL_IMAGE): $(SDL_IMAGE_SRC)
	@[ -d $(SDL_IMAGE_VERSION) ] || tar -xzf $<
	@cd $(SDL_IMAGE_VERSION) && \
	cp ../Makefiles/SDL_image.Makefile Makefile
	@$(MAKE) -C $(SDL_IMAGE_VERSION)
        
$(SDL_MIXER): $(SDL_MIXER_SRC)
	@[ -d $(SDL_MIXER_VERSION) ] || tar --exclude=SDL_mixer-1.2.12/Xcode -xzf $< 
	@cd $(SDL_MIXER_VERSION) && \
	cp ../Makefiles/SDL_mixer.Makefile Makefile
	@$(MAKE) -C $(SDL_MIXER_VERSION)

$(SDL_GFX): $(SDL_GFX_SRC)
	@[ -d $(SDL_GFX_VERSION) ] || tar -xzf $<
	@cd $(SDL_GFX_VERSION) && \
	cp ../Makefiles/SDL_gfx.Makefile Makefile
	@$(MAKE) -C $(SDL_GFX_VERSION)

install:
	@[ ! -d $(SDL_VERSION) ] || $(MAKE) -C $(SDL_VERSION) install
	@[ ! -d $(SDL_TTF_VERSION) ] || $(MAKE) -C $(SDL_TTF_VERSION) install
	@[ ! -d $(SDL_IMAGE_VERSION) ] || $(MAKE) -C $(SDL_IMAGE_VERSION) install
	@[ ! -d $(SDL_MIXER_VERSION) ] || $(MAKE) -C $(SDL_MIXER_VERSION) install
	@[ ! -d $(SDL_GFX_VERSION) ] || $(MAKE) -C $(SDL_GFX_VERSION) install

clean:
	@$(RM) -r $(SDL_TTF_VERSION)
	@$(RM) -r $(SDL_IMAGE_VERSION)
	@$(RM) -r $(SDL_MIXER_VERSION)
	@$(RM) -r $(SDL_GFX_VERSION)
