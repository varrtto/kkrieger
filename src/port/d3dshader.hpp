#pragma once
#include <string>
#include "_types.hpp"

// Translation of Direct3D 9 shader bytecode (vs_1_1..vs_2_0, ps_1_1..ps_1_4)
// into GLSL 1.50.
//
// Vertex shader inputs v# are bound to attribute slots by their dcl usage
// (see D3DAttribSlot). Interpolants between stages are named xD0, xD1,
// xT0..xT7 and xFog.

enum
{
  D3DSLOT_POSITION  = 0,
  D3DSLOT_NORMAL    = 1,
  D3DSLOT_COLOR0    = 2,
  D3DSLOT_COLOR1    = 3,
  D3DSLOT_TEXCOORD0 = 4,   // 4..7
  D3DSLOT_TANGENT   = 8,
  D3DSLOT_BINORMAL  = 9,
  D3DSLOT_BLENDW    = 10,
  D3DSLOT_BLENDI    = 11,
  D3DSLOT_PSIZE     = 12,
  D3DSLOT_UNUSED    = 15,
  D3DSLOT_MAX       = 16,
};

enum
{
  D3DTEXKIND_2D   = 0,
  D3DTEXKIND_CUBE = 1,
  D3DTEXKIND_3D   = 2,
};

struct D3DShaderInfo
{
  sBool Vertex;
  sU32 Version;           // major<<8 | minor
  sBool Ok;
  std::string Body;       // statements of main()
  std::string Defs;       // constants from def instructions
  sU32 InputsUsed;        // vs: v# referenced
  sInt InputSlot[16];     // vs: attribute slot per v#
  sU32 SamplersUsed;      // ps: texture stages sampled
};

sBool D3DTranslate(const sU32 *tokens,D3DShaderInfo &out);

// full GLSL sources
std::string D3DBuildVS(const D3DShaderInfo &vs);
// ps==0 builds a fixed function texture stage emulation.
// kinds: 2 bits per stage (D3DTEXKIND_*), proj: 1 bit per stage
std::string D3DBuildPS(const D3DShaderInfo *ps,sU32 kinds,sU32 proj);

sInt D3DShaderLength(const sU32 *tokens);   // in dwords, including end token
