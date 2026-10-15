CXX ?= clang++
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS := $(shell sdl2-config --libs)

VENDOR := src/vendor
PORT := src/port

CXXFLAGS := -std=c++17 -O2 \
  -include $(VENDOR)/kkrieger_config.hpp \
  -include $(PORT)/compat.hpp \
  -I$(VENDOR) -I$(VENDOR)/materials -I$(PORT) \
  $(SDL_CFLAGS) \
  -fno-strict-aliasing -fno-exceptions -fno-rtti \
  -Wno-everything -DGL_SILENCE_DEPRECATION

LDFLAGS := $(SDL_LIBS) \
  -framework OpenGL -framework CoreText -framework CoreGraphics \
  -framework Cocoa -framework Foundation

VENDOR_SRC := \
  $(VENDOR)/_types.cpp \
  $(VENDOR)/_viruz2.cpp \
  $(VENDOR)/engine.cpp \
  $(VENDOR)/genbitmap.cpp \
  $(VENDOR)/genblobspline.cpp \
  $(VENDOR)/geneffect.cpp \
  $(VENDOR)/genmaterial.cpp \
  $(VENDOR)/genmesh.cpp \
  $(VENDOR)/genoverlay.cpp \
  $(VENDOR)/genscene.cpp \
  $(VENDOR)/kdoc.cpp \
  $(VENDOR)/kkriegergame.cpp \
  $(VENDOR)/shadercodegen.cpp \
  $(VENDOR)/mainplayer.cpp \
  $(VENDOR)/materials/material11.cpp \
  $(VENDOR)/materials/materialdirect.cpp \
  $(VENDOR)/materials/rtmanager.cpp

PORT_SRC := \
  $(PORT)/start_macos.cpp \
  $(PORT)/d3dshader.cpp \
  $(PORT)/v2_bridge.cpp \
  $(PORT)/callwrap.cpp \
  $(PORT)/stubs.cpp

SYNTH_SRC := $(VENDOR)/v2/synth_core.cpp
V2MCONV_SRC := $(VENDOR)/v2/v2mconv.cpp

BUILD := build
OBJS := $(patsubst %.cpp,$(BUILD)/%.o,$(VENDOR_SRC) $(PORT_SRC))
SYNTH_OBJ := $(BUILD)/synth_core.o
V2MCONV_OBJ := $(BUILD)/v2mconv.o

.PHONY: all clean
all: $(BUILD)/kkrieger

$(BUILD)/kkrieger: $(OBJS) $(SYNTH_OBJ) $(V2MCONV_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SYNTH_OBJ): $(SYNTH_SRC)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++17 -O2 -include $(PORT)/compat.hpp -I$(VENDOR)/v2 -I$(PORT) \
	  -fno-exceptions -Wno-everything \
	  -DsynthGetSize=v2synthGetSize -DsynthInit=v2synthInit \
	  -DsynthRender=v2synthRender -DsynthProcessMIDI=v2synthProcessMIDI \
	  -DsynthSetGlobals=v2synthSetGlobals -DsynthGetPoly=v2synthGetPoly \
	  -DsynthGetPgm=v2synthGetPgm -DsynthGetFrameSize=v2synthGetFrameSize \
	  -c $< -o $@

$(V2MCONV_OBJ): $(V2MCONV_SRC)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++17 -O2 -include $(PORT)/compat.hpp -I$(VENDOR)/v2 -I$(PORT) \
	  -fno-exceptions -Wno-everything -c $< -o $@

clean:
	rm -rf $(BUILD)
