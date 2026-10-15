#pragma once

#ifndef _MSC_VER

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>
#include <new>

#define __forceinline inline
#define __stdcall
#define _stdcall
#define __cdecl
#define __w64
#define __int64 long long
#define _alloca alloca

#ifdef __declspec
#undef __declspec
#endif
#define __declspec(x)

#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-value"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Wparentheses"
#pragma clang diagnostic ignored "-Wcomment"
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#pragma clang diagnostic ignored "-Wvoid-pointer-to-int-cast"
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wwritable-strings"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#pragma clang diagnostic ignored "-Wreorder"
#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#pragma clang diagnostic ignored "-Wnull-conversion"
#pragma clang diagnostic ignored "-Wextra-tokens"

struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;
struct IDirect3DTexture9;
struct IDirect3DSurface9;
struct IDirect3DCubeTexture9;
struct IDirect3DVolumeTexture9;
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirectSound3DListener;
struct IDirectSoundBuffer;
struct IDirectSound3DBuffer;

#endif
