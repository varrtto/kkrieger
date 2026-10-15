cmake_minimum_required(VERSION 3.16)
project(kkrieger_macos CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
  list(APPEND CMAKE_PREFIX_PATH /opt/homebrew /usr/local)
endif()

find_package(SDL2 REQUIRED)
find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
find_library(CORETEXT_FRAMEWORK CoreText REQUIRED)
find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)

set(VENDOR ${CMAKE_SOURCE_DIR}/src/vendor)
set(PORT ${CMAKE_SOURCE_DIR}/src/port)

set(VENDOR_SRC
  ${VENDOR}/_types.cpp
  ${VENDOR}/_viruz2.cpp
  ${VENDOR}/engine.cpp
  ${VENDOR}/genbitmap.cpp
  ${VENDOR}/genblobspline.cpp
  ${VENDOR}/geneffect.cpp
  ${VENDOR}/genmaterial.cpp
  ${VENDOR}/genmesh.cpp
  ${VENDOR}/genoverlay.cpp
  ${VENDOR}/genscene.cpp
  ${VENDOR}/kdoc.cpp
  ${VENDOR}/kkriegergame.cpp
  ${VENDOR}/shadercodegen.cpp
  ${VENDOR}/mainplayer.cpp
  ${VENDOR}/materials/material11.cpp
  ${VENDOR}/materials/materialdirect.cpp
  ${VENDOR}/materials/rtmanager.cpp
)

set(PORT_SRC
  ${PORT}/start_macos.cpp
  ${PORT}/d3dshader.cpp
  ${PORT}/v2_bridge.cpp
  ${PORT}/callwrap.cpp
  ${PORT}/stubs.cpp
)

add_library(v2synth OBJECT ${VENDOR}/v2/synth_core.cpp)
target_include_directories(v2synth PRIVATE ${VENDOR}/v2 ${PORT})
target_compile_options(v2synth PRIVATE
  -include ${PORT}/compat.hpp
  -Wno-everything
  -fno-exceptions
)
target_compile_definitions(v2synth PRIVATE
  synthGetSize=v2synthGetSize
  synthInit=v2synthInit
  synthRender=v2synthRender
  synthProcessMIDI=v2synthProcessMIDI
  synthSetGlobals=v2synthSetGlobals
  synthGetPoly=v2synthGetPoly
  synthGetPgm=v2synthGetPgm
  synthGetFrameSize=v2synthGetFrameSize
)

add_executable(kkrieger ${VENDOR_SRC} ${PORT_SRC} $<TARGET_OBJECTS:v2synth>)

target_include_directories(kkrieger PRIVATE
  ${VENDOR}
  ${VENDOR}/materials
  ${PORT}
  ${SDL2_INCLUDE_DIRS}
)

if(TARGET SDL2::SDL2)
  target_link_libraries(kkrieger PRIVATE SDL2::SDL2)
elseif(SDL2_LIBRARIES)
  target_link_libraries(kkrieger PRIVATE ${SDL2_LIBRARIES})
endif()

target_compile_options(kkrieger PRIVATE
  -include ${VENDOR}/kkrieger_config.hpp
  -include ${PORT}/compat.hpp
  -fno-strict-aliasing
  -fno-exceptions
  -Wno-everything
)

target_link_libraries(kkrieger PRIVATE
  ${OPENGL_FRAMEWORK}
  ${CORETEXT_FRAMEWORK}
  ${COREGRAPHICS_FRAMEWORK}
  ${COCOA_FRAMEWORK}
  ${FOUNDATION_FRAMEWORK}
)

install(TARGETS kkrieger DESTINATION bin)
install(FILES data/kkrieger.kx DESTINATION share/kkrieger)
