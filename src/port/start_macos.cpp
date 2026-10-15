#define GL_SILENCE_DEPRECATION
#include "_types.hpp"
#include "_start.hpp"
#include "_startdx.hpp"
#include "material11.hpp"
#include "d3dshader.hpp"

#include <SDL.h>
#include <OpenGL/gl3.h>
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>

#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

sSystem_ *sSystem;
class sBroker_ *sBroker;
sInt sFatality;
sInt IntroLoop;
sInt IntroTargetAspect;
sBool IntroHighTexRes = sTRUE;
sInt DebugTextMem;
sU8 DebugData[4];

extern sBool sAppHandler(sInt code, sDInt value);

static void pump();
static void drawLoading(sInt done, sInt max);

sBool ConfigDialog(sInt, const sChar *) { return sTRUE; }

extern "C" void GetSystemTime(void *st) { memset(st, 0, 16); }
extern "C" sBool SystemTimeToFileTime(const void *, void *ft) { memset(ft, 0, 8); return 1; }
extern "C" void OutputDebugStringA(const char *s) { if(s) fputs(s, stderr); }

/****************************************************************************/
/***   Direct3D 9 object stand-ins                                        ***/
/****************************************************************************/

struct IDirect3DVertexShader9
{
  D3DShaderInfo Info;
  GLuint Obj = 0;
  void Release();
};

struct IDirect3DPixelShader9
{
  D3DShaderInfo Info;
  std::map<sU32,GLuint> Objs;     // per sampler kind / projection variant
  void Release();
};

struct IDirect3DTexture9
{
  GLuint Tex = 0;
  sInt XS = 0, YS = 0;
  sBool RT = 0;
  void Release();
};

/****************************************************************************/
/***   globals                                                            ***/
/****************************************************************************/

static SDL_Window *gWin;
static SDL_GLContext gGL;
static sChar gCmdLine[1024];
static sBool gQuit;
static sU32 gStartMs;
static sInt gMouseX, gMouseY;
static sU32 gMouseBtns;
static sSoundHandler gSndH;
static void *gSndUser;
static std::mutex gSndMu;
static sU32 *gFontMem;
static sInt gFontW, gFontH;
static CTFontRef gFont;

static const sInt gFvfSize[] = {0,32,32,64,16,16,32,48};

static GLuint gVAO;
static GLuint gFBO;                   // all rendering goes through this one
static GLuint gBlitFBO[2];
static GLuint gScreenTex;             // stands in for the d3d back buffer
static GLuint gDepthRB;               // shared depth/stencil, like the d3d z buffer
static sInt gDepthW, gDepthH;
static GLuint gBoundColor;            // color attachment of gFBO
static GLuint gWhiteTex, gCubeTex, gVolumeTex;
static GLuint gSamplers[8];
static GLuint gLoadProg, gLoadVBO;
static GLint gLoadColorLoc;

static sU32 gRS[0x310];               // d3d render/texture/sampler state values
static sInt gStageTex[MAX_TEXSTAGE];
static GLfloat gVSConst[256*4];
static GLfloat gPSConst[32*4];
static sInt gVSConstUsed = 32;
static sInt gScissor[4];              // left, top, right, bottom in target pixels

static GLuint gGeoGL[MAX_GEOBUFFER];
static sBool gGeoDyn[MAX_GEOBUFFER];

// locked geometry is uploaded when it is drawn: some callers keep writing
// through the lock pointer after GeoEnd()
struct GeoStage
{
  std::vector<sU8> Data;
  sInt Buffer, Start, Count, ElemSize;
  sBool Active;
};

static sBool gTrace;
static sInt gTraceSeq;
static void dumpTexture(GLuint tex,sInt w,sInt h,const char *name);
static GeoStage gStage[MAX_GEOHANDLE][2];
static GeoStage *gAllocStage;

/****************************************************************************/
/***   program cache                                                      ***/
/****************************************************************************/

struct GLProg
{
  GLuint Prog;
  GLint VC, PC, PosFix, FogColor, AlphaTest, Bump, BumpL, COp, AOp, TFactor;
};

struct ProgKey
{
  void *VS, *PS;
  sU32 Kinds, Proj;
  bool operator<(const ProgKey &o) const
  {
    if(VS!=o.VS) return VS<o.VS;
    if(PS!=o.PS) return PS<o.PS;
    if(Kinds!=o.Kinds) return Kinds<o.Kinds;
    return Proj<o.Proj;
  }
};

static std::map<ProgKey,GLProg> gProgs;
static std::map<sU32,GLuint> gFFObjs;

static GLuint compileShader(GLenum type,const std::string &src)
{
  GLuint sh = glCreateShader(type);
  const char *s = src.c_str();
  glShaderSource(sh,1,&s,0);
  glCompileShader(sh);
  GLint ok = 0;
  glGetShaderiv(sh,GL_COMPILE_STATUS,&ok);
  if(!ok)
  {
    char log[4096];
    glGetShaderInfoLog(sh,sizeof(log),0,log);
    fprintf(stderr,"glsl compile error:\n%s\n--- source ---\n%s\n",log,s);
  }
  return sh;
}

static void dropPrograms(void *vs,void *ps)
{
  for(auto it=gProgs.begin();it!=gProgs.end();)
  {
    if((vs && it->first.VS==vs) || (ps && it->first.PS==ps))
    {
      glDeleteProgram(it->second.Prog);
      it = gProgs.erase(it);
    }
    else
      ++it;
  }
}

void IDirect3DVertexShader9::Release()
{
  dropPrograms(this,0);
  if(Obj) glDeleteShader(Obj);
  delete this;
}

void IDirect3DPixelShader9::Release()
{
  dropPrograms(0,this);
  for(auto &o : Objs) glDeleteShader(o.second);
  delete this;
}

void IDirect3DTexture9::Release()
{
  if(Tex)
  {
    if(gBoundColor==Tex)
    {
      glBindFramebuffer(GL_FRAMEBUFFER,gFBO);
      glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,gScreenTex,0);
      gBoundColor = gScreenTex;
    }
    glDeleteTextures(1,&Tex);
  }
  delete this;
}

static GLProg *getProgram(IDirect3DVertexShader9 *vs,IDirect3DPixelShader9 *ps,sU32 kinds,sU32 proj)
{
  ProgKey key = { vs,ps,kinds,proj };
  auto it = gProgs.find(key);
  if(it!=gProgs.end())
    return &it->second;

  GLuint fs;
  sU32 variant = kinds | (proj<<16);
  if(ps)
  {
    auto o = ps->Objs.find(variant);
    if(o==ps->Objs.end())
    {
      fs = compileShader(GL_FRAGMENT_SHADER,D3DBuildPS(&ps->Info,kinds,proj));
      ps->Objs[variant] = fs;
    }
    else
      fs = o->second;
  }
  else
  {
    auto o = gFFObjs.find(variant);
    if(o==gFFObjs.end())
    {
      fs = compileShader(GL_FRAGMENT_SHADER,D3DBuildPS(0,kinds,proj));
      gFFObjs[variant] = fs;
    }
    else
      fs = o->second;
  }

  GLProg p;
  p.Prog = glCreateProgram();
  glAttachShader(p.Prog,vs->Obj);
  glAttachShader(p.Prog,fs);
  for(sInt i=0;i<16;i++)
  {
    sInt slot = vs->Info.InputSlot[i];
    if((vs->Info.InputsUsed & (1u<<i)) && slot>=0 && slot!=D3DSLOT_UNUSED)
    {
      char name[8];
      snprintf(name,sizeof(name),"v%d",i);
      glBindAttribLocation(p.Prog,slot,name);
    }
  }
  glBindFragDataLocation(p.Prog,0,"fragColor");
  glLinkProgram(p.Prog);
  GLint ok = 0;
  glGetProgramiv(p.Prog,GL_LINK_STATUS,&ok);
  if(!ok)
  {
    char log[4096];
    glGetProgramInfoLog(p.Prog,sizeof(log),0,log);
    fprintf(stderr,"glsl link error:\n%s\n",log);
  }
  glUseProgram(p.Prog);
  for(sInt i=0;i<8;i++)
  {
    char name[8];
    snprintf(name,sizeof(name),"s%d",i);
    GLint l = glGetUniformLocation(p.Prog,name);
    if(l>=0) glUniform1i(l,i);
  }
  p.VC        = glGetUniformLocation(p.Prog,"vc");
  p.PC        = glGetUniformLocation(p.Prog,"pc");
  p.PosFix    = glGetUniformLocation(p.Prog,"uPosFix");
  p.FogColor  = glGetUniformLocation(p.Prog,"uFogColor");
  p.AlphaTest = glGetUniformLocation(p.Prog,"uAlphaTest");
  p.Bump      = glGetUniformLocation(p.Prog,"uBump");
  p.BumpL     = glGetUniformLocation(p.Prog,"uBumpL");
  p.COp       = glGetUniformLocation(p.Prog,"uCOp");
  p.AOp       = glGetUniformLocation(p.Prog,"uAOp");
  p.TFactor   = glGetUniformLocation(p.Prog,"uTFactor");
  return &(gProgs[key] = p);
}

/****************************************************************************/
/***   d3d state emulation                                                ***/
/****************************************************************************/

static void initDefaultStates()
{
  sSetMem(gRS,0,sizeof(gRS));
  gRS[sD3DRS_ZENABLE] = 1;
  gRS[sD3DRS_ZWRITEENABLE] = 1;
  gRS[sD3DRS_SRCBLEND] = sD3DBLEND_ONE;
  gRS[sD3DRS_DESTBLEND] = sD3DBLEND_ZERO;
  gRS[sD3DRS_CULLMODE] = sD3DCULL_CCW;
  gRS[sD3DRS_ZFUNC] = sD3DCMP_LESSEQUAL;
  gRS[sD3DRS_ALPHAFUNC] = sD3DCMP_ALWAYS;
  gRS[sD3DRS_STENCILFAIL] = sD3DSTENCILOP_KEEP;
  gRS[sD3DRS_STENCILZFAIL] = sD3DSTENCILOP_KEEP;
  gRS[sD3DRS_STENCILPASS] = sD3DSTENCILOP_KEEP;
  gRS[sD3DRS_STENCILFUNC] = sD3DCMP_ALWAYS;
  gRS[sD3DRS_STENCILMASK] = 0xffffffff;
  gRS[sD3DRS_STENCILWRITEMASK] = 0xffffffff;
  gRS[sD3DRS_TEXTUREFACTOR] = 0xffffffff;
  gRS[sD3DRS_COLORWRITEENABLE] = 15;
  gRS[sD3DRS_BLENDOP] = sD3DBLENDOP_ADD;
  gRS[sD3DRS_CCW_STENCILFAIL] = sD3DSTENCILOP_KEEP;
  gRS[sD3DRS_CCW_STENCILZFAIL] = sD3DSTENCILOP_KEEP;
  gRS[sD3DRS_CCW_STENCILPASS] = sD3DSTENCILOP_KEEP;
  gRS[sD3DRS_CCW_STENCILFUNC] = sD3DCMP_ALWAYS;
  for(sInt i=0;i<8;i++)
  {
    sU32 *t = &gRS[0x100+i*0x20];
    t[sD3DTSS_COLOROP&31] = i ? sD3DTOP_DISABLE : sD3DTOP_MODULATE;
    t[sD3DTSS_COLORARG1&31] = sD3DTA_TEXTURE;
    t[sD3DTSS_COLORARG2&31] = sD3DTA_CURRENT;
    t[sD3DTSS_ALPHAOP&31] = i ? sD3DTOP_DISABLE : sD3DTOP_SELECTARG1;
    t[sD3DTSS_ALPHAARG1&31] = sD3DTA_TEXTURE;
    t[sD3DTSS_ALPHAARG2&31] = sD3DTA_CURRENT;
    t[sD3DTSS_TEXCOORDINDEX&31] = i;
  }
  for(sInt i=0;i<16;i++)
  {
    sU32 *s = &gRS[0x200+i*0x10];
    s[sD3DSAMP_ADDRESSU&15] = sD3DTADDRESS_WRAP;
    s[sD3DSAMP_ADDRESSV&15] = sD3DTADDRESS_WRAP;
    s[sD3DSAMP_ADDRESSW&15] = sD3DTADDRESS_WRAP;
    s[sD3DSAMP_MAGFILTER&15] = sD3DTEXF_POINT;
    s[sD3DSAMP_MINFILTER&15] = sD3DTEXF_POINT;
    s[sD3DSAMP_MIPFILTER&15] = sD3DTEXF_NONE;
    s[sD3DSAMP_MAXANISOTROPY&15] = 1;
  }
  // like the original device init
  gRS[0x100+(sD3DTSS_COLOROP&31)] = sD3DTOP_SELECTARG1;
  gRS[0x100+(sD3DTSS_COLORARG1&31)] = sD3DTA_TEXTURE;
}

static GLenum glCmp(sU32 v)
{
  static const GLenum t[] = { GL_ALWAYS,GL_NEVER,GL_LESS,GL_EQUAL,GL_LEQUAL,GL_GREATER,GL_NOTEQUAL,GL_GEQUAL,GL_ALWAYS };
  return v<=8 ? t[v] : GL_ALWAYS;
}

static GLenum glBlendFactor(sU32 v)
{
  switch(v)
  {
  case sD3DBLEND_ZERO:          return GL_ZERO;
  case sD3DBLEND_ONE:           return GL_ONE;
  case sD3DBLEND_SRCCOLOR:      return GL_SRC_COLOR;
  case sD3DBLEND_INVSRCCOLOR:   return GL_ONE_MINUS_SRC_COLOR;
  case sD3DBLEND_SRCALPHA:      return GL_SRC_ALPHA;
  case sD3DBLEND_INVSRCALPHA:   return GL_ONE_MINUS_SRC_ALPHA;
  case sD3DBLEND_DESTALPHA:     return GL_DST_ALPHA;
  case sD3DBLEND_INVDESTALPHA:  return GL_ONE_MINUS_DST_ALPHA;
  case sD3DBLEND_DESTCOLOR:     return GL_DST_COLOR;
  case sD3DBLEND_INVDESTCOLOR:  return GL_ONE_MINUS_DST_COLOR;
  case sD3DBLEND_SRCALPHASAT:   return GL_SRC_ALPHA_SATURATE;
  }
  return GL_ONE;
}

static GLenum glBlendEq(sU32 v)
{
  switch(v)
  {
  case sD3DBLENDOP_SUBTRACT:    return GL_FUNC_SUBTRACT;
  case sD3DBLENDOP_REVSUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
  case sD3DBLENDOP_MIN:         return GL_MIN;
  case sD3DBLENDOP_MAX:         return GL_MAX;
  }
  return GL_FUNC_ADD;
}

static GLenum glStencilOpD3D(sU32 v)
{
  static const GLenum t[] = { GL_KEEP,GL_KEEP,GL_ZERO,GL_REPLACE,GL_INCR,GL_DECR,GL_INVERT,GL_INCR_WRAP,GL_DECR_WRAP };
  return v<=8 ? t[v] : GL_KEEP;
}

static GLenum glAddress(sU32 v)
{
  switch(v)
  {
  case sD3DTADDRESS_MIRROR: return GL_MIRRORED_REPEAT;
  case sD3DTADDRESS_CLAMP:  return GL_CLAMP_TO_EDGE;
  case sD3DTADDRESS_BORDER: return GL_CLAMP_TO_BORDER;
  case sD3DTADDRESS_MIRRORONCE: return GL_CLAMP_TO_EDGE;
  }
  return GL_REPEAT;
}

static sF32 asFloat(sU32 v) { sF32 f; memcpy(&f,&v,4); return f; }

static void applyRenderStates()
{
  if(gRS[sD3DRS_ALPHABLENDENABLE])
  {
    glEnable(GL_BLEND);
    sU32 src = gRS[sD3DRS_SRCBLEND], dst = gRS[sD3DRS_DESTBLEND];
    if(src==12) { src = sD3DBLEND_SRCALPHA; dst = sD3DBLEND_INVSRCALPHA; }       // BOTHSRCALPHA
    if(src==13) { src = sD3DBLEND_INVSRCALPHA; dst = sD3DBLEND_SRCALPHA; }       // BOTHINVSRCALPHA
    glBlendFunc(glBlendFactor(src),glBlendFactor(dst));
    glBlendEquation(glBlendEq(gRS[sD3DRS_BLENDOP]));
  }
  else
    glDisable(GL_BLEND);

  static sBool noz = getenv("KK_NOZ")!=0, nocull = getenv("KK_NOCULL")!=0;
  if(getenv("KK_DCLAMP")) glEnable(GL_DEPTH_CLAMP);
  if(gRS[sD3DRS_ZENABLE] && !noz)
  {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(glCmp(gRS[sD3DRS_ZFUNC]));
  }
  else
    glDisable(GL_DEPTH_TEST);
  glDepthMask(gRS[sD3DRS_ZWRITEENABLE] ? GL_TRUE : GL_FALSE);

  // d3d front faces are clockwise; rendering into the y-flipped fbo turns them ccw
  switch(nocull ? sD3DCULL_NONE : gRS[sD3DRS_CULLMODE])
  {
  case sD3DCULL_NONE: glDisable(GL_CULL_FACE); break;
  case sD3DCULL_CW:   glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); break;
  default:            glEnable(GL_CULL_FACE); glCullFace(GL_BACK); break;
  }

  sU32 cm = gRS[sD3DRS_COLORWRITEENABLE];
  glColorMask((cm&1)!=0,(cm&2)!=0,(cm&4)!=0,(cm&8)!=0);

  if(gRS[sD3DRS_STENCILENABLE])
  {
    glEnable(GL_STENCIL_TEST);
    GLint ref = gRS[sD3DRS_STENCILREF];
    GLuint mask = gRS[sD3DRS_STENCILMASK];
    glStencilFuncSeparate(GL_FRONT,glCmp(gRS[sD3DRS_STENCILFUNC]),ref,mask);
    glStencilOpSeparate(GL_FRONT,glStencilOpD3D(gRS[sD3DRS_STENCILFAIL]),
      glStencilOpD3D(gRS[sD3DRS_STENCILZFAIL]),glStencilOpD3D(gRS[sD3DRS_STENCILPASS]));
    if(gRS[sD3DRS_TWOSIDEDSTENCILMODE])
    {
      glStencilFuncSeparate(GL_BACK,glCmp(gRS[sD3DRS_CCW_STENCILFUNC]),ref,mask);
      glStencilOpSeparate(GL_BACK,glStencilOpD3D(gRS[sD3DRS_CCW_STENCILFAIL]),
        glStencilOpD3D(gRS[sD3DRS_CCW_STENCILZFAIL]),glStencilOpD3D(gRS[sD3DRS_CCW_STENCILPASS]));
    }
    else
    {
      glStencilFuncSeparate(GL_BACK,glCmp(gRS[sD3DRS_STENCILFUNC]),ref,mask);
      glStencilOpSeparate(GL_BACK,glStencilOpD3D(gRS[sD3DRS_STENCILFAIL]),
        glStencilOpD3D(gRS[sD3DRS_STENCILZFAIL]),glStencilOpD3D(gRS[sD3DRS_STENCILPASS]));
    }
    glStencilMask(gRS[sD3DRS_STENCILWRITEMASK]);
  }
  else
    glDisable(GL_STENCIL_TEST);

  sF32 bias = asFloat(gRS[sD3DRS_DEPTHBIAS]);
  sF32 slope = asFloat(gRS[sD3DRS_SLOPESCALEDEPTHBIAS]);
  if(bias!=0 || slope!=0)
  {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(slope,bias*16777216.0f);
  }
  else
    glDisable(GL_POLYGON_OFFSET_FILL);

  if(gRS[sD3DRS_SCISSORTESTENABLE])
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor(gScissor[0],gScissor[1],sMax(gScissor[2]-gScissor[0],0),sMax(gScissor[3]-gScissor[1],0));
  }
  else
    glDisable(GL_SCISSOR_TEST);
}

static void applySampler(sInt i)
{
  const sU32 *s = &gRS[0x200+i*0x10];
  GLuint so = gSamplers[i];
  glSamplerParameteri(so,GL_TEXTURE_WRAP_S,glAddress(s[sD3DSAMP_ADDRESSU&15]));
  glSamplerParameteri(so,GL_TEXTURE_WRAP_T,glAddress(s[sD3DSAMP_ADDRESSV&15]));
  glSamplerParameteri(so,GL_TEXTURE_WRAP_R,glAddress(s[sD3DSAMP_ADDRESSW&15]));
  sU32 mag = s[sD3DSAMP_MAGFILTER&15], min = s[sD3DSAMP_MINFILTER&15], mip = s[sD3DSAMP_MIPFILTER&15];
  glSamplerParameteri(so,GL_TEXTURE_MAG_FILTER,mag>=sD3DTEXF_LINEAR ? GL_LINEAR : GL_NEAREST);
  GLenum mf;
  sBool lin = min>=sD3DTEXF_LINEAR;
  if(mip==sD3DTEXF_NONE)       mf = lin ? GL_LINEAR : GL_NEAREST;
  else if(mip==sD3DTEXF_POINT) mf = lin ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
  else                         mf = lin ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
  glSamplerParameteri(so,GL_TEXTURE_MIN_FILTER,mf);
  sF32 aniso = 1.0f;
  if(min==sD3DTEXF_ANISOTROPIC)
    aniso = (sF32)sMax<sInt>(1,sMin<sInt>(16,s[sD3DSAMP_MAXANISOTROPY&15]));
  glSamplerParameterf(so,GL_TEXTURE_MAX_ANISOTROPY_EXT,aniso);
  glBindSampler(i,so);
}

// binds stage textures and returns sampler kinds; proj receives projected stages
static sU32 bindStageTextures(sU32 used,sU32 &proj)
{
  sU32 kinds = 0;
  proj = 0;
  for(sInt i=0;i<8;i++)
  {
    if(!(used & (1u<<i))) continue;
    sInt ti = gStageTex[i];
    GLenum target = GL_TEXTURE_2D;
    GLuint tex = gWhiteTex;
    sU32 kind = D3DTEXKIND_2D;
    if(ti>=0 && ti<MAX_TEXTURE && sSystem->Textures[ti].Tex)
      tex = sSystem->Textures[ti].Tex->Tex;
    else if(ti==-2) { target = GL_TEXTURE_CUBE_MAP; tex = gCubeTex; kind = D3DTEXKIND_CUBE; }
    else if(ti==-3) { target = GL_TEXTURE_3D; tex = gVolumeTex; kind = D3DTEXKIND_3D; }
    glActiveTexture(GL_TEXTURE0+i);
    glBindTexture(target,tex);
    kinds |= kind<<(2*i);
    if(kind==D3DTEXKIND_2D && (gRS[0x100+i*0x20+(sD3DTSS_TEXTURETRANSFORMFLAGS&31)] & sD3DTTFF_PROJECTED))
      proj |= 1u<<i;
    applySampler(i);
  }
  glActiveTexture(GL_TEXTURE0);
  return kinds;
}

static void setColorUniform(GLint loc,sU32 c,sF32 a)
{
  glUniform4f(loc,((c>>16)&255)/255.0f,((c>>8)&255)/255.0f,(c&255)/255.0f,a);
}

static void setProgramUniforms(GLProg *p)
{
  if(p->VC>=0) glUniform4fv(p->VC,gVSConstUsed,gVSConst);
  if(p->PC>=0) glUniform4fv(p->PC,32,gPSConst);
  if(p->PosFix>=0)
    glUniform4f(p->PosFix,1.0f/sMax(sSystem->ViewportX,1),1.0f/sMax(sSystem->ViewportY,1),-1.0f,0.0f);
  if(p->FogColor>=0)
    setColorUniform(p->FogColor,gRS[sD3DRS_FOGCOLOR],gRS[sD3DRS_FOGENABLE] ? 1.0f : 0.0f);
  if(p->AlphaTest>=0)
    glUniform4f(p->AlphaTest,gRS[sD3DRS_ALPHATESTENABLE] ? 1.0f : 0.0f,
      (sF32)(gRS[sD3DRS_ALPHAREF]&255),(sF32)gRS[sD3DRS_ALPHAFUNC],0.0f);
  if(p->Bump>=0 || p->BumpL>=0)
  {
    GLfloat b[32],l[32];
    for(sInt i=0;i<8;i++)
    {
      const sU32 *t = &gRS[0x100+i*0x20];
      b[i*4+0] = asFloat(t[sD3DTSS_BUMPENVMAT00&31]);
      b[i*4+1] = asFloat(t[sD3DTSS_BUMPENVMAT01&31]);
      b[i*4+2] = asFloat(t[sD3DTSS_BUMPENVMAT10&31]);
      b[i*4+3] = asFloat(t[sD3DTSS_BUMPENVMAT11&31]);
      l[i*4+0] = asFloat(t[sD3DTSS_BUMPENVLSCALE&31]);
      l[i*4+1] = asFloat(t[sD3DTSS_BUMPENVLOFFSET&31]);
      l[i*4+2] = l[i*4+3] = 0;
    }
    if(p->Bump>=0) glUniform4fv(p->Bump,8,b);
    if(p->BumpL>=0) glUniform4fv(p->BumpL,8,l);
  }
  if(p->COp>=0)
  {
    GLint c[8],a[8];
    for(sInt i=0;i<2;i++)
    {
      const sU32 *t = &gRS[0x100+i*0x20];
      c[i*4+0] = t[sD3DTSS_COLOROP&31];
      c[i*4+1] = t[sD3DTSS_COLORARG1&31];
      c[i*4+2] = t[sD3DTSS_COLORARG2&31];
      c[i*4+3] = t[sD3DTSS_ALPHAOP&31];
      a[i*4+0] = t[sD3DTSS_ALPHAARG1&31];
      a[i*4+1] = t[sD3DTSS_ALPHAARG2&31];
      a[i*4+2] = a[i*4+3] = 0;
    }
    glUniform4iv(p->COp,2,c);
    if(p->AOp>=0) glUniform4iv(p->AOp,2,a);
    if(p->TFactor>=0)
    {
      sU32 tf = gRS[sD3DRS_TEXTUREFACTOR];
      setColorUniform(p->TFactor,tf,((tf>>24)&255)/255.0f);
    }
  }
}

static void bindFVF(sInt fvf)
{
  for(sInt i=0;i<D3DSLOT_MAX;i++)
    glDisableVertexAttribArray(i);
  sInt st = gFvfSize[fvf];
  auto f = [&](GLuint slot,GLint n,sInt ofs)
  {
    glEnableVertexAttribArray(slot);
    glVertexAttribPointer(slot,n,GL_FLOAT,GL_FALSE,st,(const void *)(sDInt)ofs);
  };
  auto c = [&](GLuint slot,sInt ofs)
  {
    glEnableVertexAttribArray(slot);
    glVertexAttribPointer(slot,GL_BGRA,GL_UNSIGNED_BYTE,GL_TRUE,st,(const void *)(sDInt)ofs);
  };
  switch(fvf)
  {
  case sFVF_STANDARD:
    f(D3DSLOT_POSITION,3,0); f(D3DSLOT_NORMAL,3,12); f(D3DSLOT_TEXCOORD0,2,24);
    break;
  case sFVF_DOUBLE:
    f(D3DSLOT_POSITION,3,0); c(D3DSLOT_COLOR0,12); f(D3DSLOT_TEXCOORD0,2,16); f(D3DSLOT_TEXCOORD0+1,2,24);
    break;
  case sFVF_TSPACE:
    f(D3DSLOT_POSITION,3,0); f(D3DSLOT_NORMAL,3,12); c(D3DSLOT_COLOR0,24); c(D3DSLOT_COLOR1,28);
    f(D3DSLOT_TEXCOORD0,2,32); f(D3DSLOT_TANGENT,3,40); f(D3DSLOT_BINORMAL,3,52);
    break;
  case sFVF_COMPACT:
    f(D3DSLOT_POSITION,3,0); c(D3DSLOT_COLOR0,12);
    break;
  case sFVF_XYZW:
    f(D3DSLOT_POSITION,4,0);
    break;
  case sFVF_TSPACE3:
    f(D3DSLOT_POSITION,3,0); c(D3DSLOT_NORMAL,12); c(D3DSLOT_TANGENT,16); c(D3DSLOT_COLOR0,20);
    f(D3DSLOT_TEXCOORD0,2,24);
    break;
  case sFVF_TSPACE3BIG:
    f(D3DSLOT_POSITION,3,0); f(D3DSLOT_NORMAL,3,12); f(D3DSLOT_TANGENT,3,24); c(D3DSLOT_COLOR0,36);
    f(D3DSLOT_TEXCOORD0,2,40);
    break;
  }
}

static void attachTarget(GLuint tex)
{
  glBindFramebuffer(GL_FRAMEBUFFER,gFBO);
  if(gBoundColor!=tex)
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex,0);
    gBoundColor = tex;
  }
}

static void ensureDepth(sInt w,sInt h)
{
  if(w<=gDepthW && h<=gDepthH && gDepthRB)
    return;
  gDepthW = sMax(w,gDepthW);
  gDepthH = sMax(h,gDepthH);
  if(!gDepthRB) glGenRenderbuffers(1,&gDepthRB);
  glBindRenderbuffer(GL_RENDERBUFFER,gDepthRB);
  glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,gDepthW,gDepthH);
  glBindFramebuffer(GL_FRAMEBUFFER,gFBO);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,gDepthRB);
}

/****************************************************************************/
/***   engine glue                                                        ***/
/****************************************************************************/

static sU32 sdlKey(SDL_Keycode k, sBool up)
{
  sU32 c=0;
  if(k>='a' && k<='z') c=(sU32)k;
  else if(k>='0' && k<='9') c=(sU32)k;
  else {
    switch(k){
    case SDLK_SPACE: c=' '; break;
    case SDLK_ESCAPE: c=sKEY_ESCAPE; break;
    case SDLK_RETURN: c=sKEY_ENTER; break;
    case SDLK_KP_ENTER: c=sKEY_ENTER; break;
    case SDLK_BACKSPACE: c=sKEY_BACKSPACE; break;
    case SDLK_TAB: c=sKEY_TAB; break;
    case SDLK_UP: c=sKEY_UP; break;
    case SDLK_DOWN: c=sKEY_DOWN; break;
    case SDLK_LEFT: c=sKEY_LEFT; break;
    case SDLK_RIGHT: c=sKEY_RIGHT; break;
    case SDLK_LSHIFT: c=sKEY_SHIFTL; break;
    case SDLK_RSHIFT: c=sKEY_SHIFTR; break;
    case SDLK_LCTRL: c=sKEY_CTRLL; break;
    case SDLK_RCTRL: c=sKEY_CTRLR; break;
    default: c=0; break;
    }
  }
  if(up) c|=sKEYQ_BREAK;
  return c;
}

#if sLINK_KKRIEGER
static void mixSamples(sS16 *out,sInt frames);
#endif

static void audioCB(void *, Uint8 *stream, int len)
{
  sInt samples = len / 4;
  std::lock_guard<std::mutex> lock(gSndMu);
  if(gSndH) gSndH((sS16*)stream, samples, gSndUser);
  else memset(stream,0,len);
#if sLINK_KKRIEGER
  mixSamples((sS16*)stream, samples);
#endif
  { static FILE *wf=fopen("/tmp/kk_audio.raw","wb"); fwrite(stream,1,len,wf); fflush(wf); }
}

void sSetConfig(sU32 flags, sInt xs, sInt ys)
{
  if(sSystem){ sSystem->ConfigFlags=flags; if(xs>0) sSystem->ConfigX=xs; if(ys>0) sSystem->ConfigY=ys; }
}

void sMaterialEnv::Init()
{
  sSetMem(this,0,sizeof(*this));
  ModelSpace.Init();
  CameraSpace.Init();
  NearClip = 0.125f;
  FarClip = 4096.0f;
  ZoomX = 1.0f;
  ZoomY = 1.0f;
  CenterX = 0.0f;
  CenterY = 0.0f;
  FogStart = 0;
  FogEnd = FarClip;
  FogColor = 0xff808080;
}

void sMaterialEnv::MakeProjectionMatrix(sMatrix &pmat) const
{
  sF32 q;
  sF32 shiftX = 1.0f / sSystem->ViewportX;
  sF32 shiftY = 1.0f / sSystem->ViewportY;

  switch(Orthogonal)
  {
  case sMEO_PIXELS:
    pmat.i.Init(2.0f/sSystem->ViewportX,0,0,0);
    pmat.j.Init(0,-2.0f/sSystem->ViewportY,0,0);
    pmat.k.Init(0,0,1,0);
    pmat.l.Init(-1 - (2*CenterX+1)*shiftX,1 + (2*CenterY+1)*shiftY,0,1);
    break;

  case sMEO_NORMALISED:
    pmat.i.Init(ZoomX,0,0,0);
    pmat.j.Init(0,ZoomY,0,0);
    pmat.k.Init(0,0,1.0f/FarClip,0);
    pmat.l.Init(-shiftX,shiftY,0,1);
    break;

  case sMEO_PERSPECTIVE:
    q = 1.0f;
    pmat.i.Init(ZoomX,0,0,0);
    pmat.j.Init(0,ZoomY,0,0);
    pmat.k.Init(CenterX,CenterY,q,1);
    pmat.l.Init(-shiftX,shiftY,-q*NearClip,0);
    break;
  }
}

void sViewport::Init(sInt screen){
  Screen=screen; RenderTarget=sINVALID;
  if(sSystem && screen>=0)
    Window.Init(0,0,sSystem->Screen[screen].XSize,sSystem->Screen[screen].YSize);
  else
    Window.Init(0,0,0,0);
}
void sViewport::InitTex(sInt handle){ Screen=-1; RenderTarget=handle; Window.Init(0,0,0,0); }
void sTexInfo::Init(sBitmap *bm,sInt format,sU32 flags){ XSize=bm->XSize; YSize=bm->YSize; Bitmap=bm; Flags=flags; Format=format; }
void sTexInfo::InitRT(sInt xs,sInt ys){ XSize=xs; YSize=ys; Bitmap=0; Flags=sTIF_RENDERTARGET; Format=0; }
sMaterial::~sMaterial(){}
void sMaterialSetup::Cleanup()
{
  delete[] States; delete[] VSCode; delete[] PSCode;
  sRelease(VS); sRelease(PS);
  States=0; VSCode=0; PSCode=0;
}
sMaterialInstance sMaterialInstance::Null = {0};

void sGeoBuffer::Init(){ Type=0; Size=0; Used=0; VB=0; UserCount=0; }
sBool sGeoBuffer::Alloc(sInt count,sInt size,sInt &firstp,sInt type)
{
  if(Type!=type) return sFALSE;
  sInt first=(Used+size-1)/size;
  sInt pos=(first+count)*size;
  if(pos>Size) return sFALSE;
  Used=pos; firstp=first; UserCount++; return sTRUE;
}
void sGeoBuffer::Free(){ if(UserCount>0 && --UserCount==0) Used=0; }

sNORETURN void sSystem_::Abort(sChar *msg)
{
  if(msg) fprintf(stderr,"fatal: %s\n", msg);
  exit(1);
}
void sSystem_::Log(sChar *s){ if(s) fputs(s,stderr); }
void sSystem_::Exit(){ gQuit=1; }
void sSystem_::Tag(){}
sInt sSystem_::MemoryUsed(){ return 0; }
void sSystem_::CheckMem(){}
sChar *sSystem_::GetCmdLine(){ return gCmdLine; }
void sSystem_::Progress(sInt done,sInt max)
{
  static sInt lastDraw, lastPrint;
  sInt now=GetTime();
  if(max>0 && (done==0 || done==max || now-lastPrint>250)){
    lastPrint=now;
    float pct = max>0 ? (100.f*done)/max : 0;
    fprintf(stderr,"loading %d/%d (%.0f percent)\n",done,max,pct);
    fflush(stderr);
  }
  if(gWin && gLoadProg && (now-lastDraw>40 || done==max || done==0)){
    lastDraw=now;
    pump();
    drawLoading(done,max);
  }
}
void sSystem_::WaitForKey(){}
sBool sSystem_::FileRequester(sChar *,sInt,sU32){ return 0; }
void sSystem_::GetInput(sInt,sInputData &data)
{
  sSetMem(&data,0,sizeof(data));
  data.Type=sIDT_MOUSE; data.AnalogCount=2; data.DigitalCount=3;
  data.Analog[0]=gMouseX; data.Analog[1]=gMouseY; data.Digital=gMouseBtns;
}
sU32 sSystem_::GetKeyboardShiftState(){ return 0; }
void sSystem_::GetKeyName(sChar *b,sInt s,sU32){ if(s>0) b[0]=0; }
sBool sSystem_::GetAbortKey(){ return 0; }
void sSystem_::ClearAbortKey(){}
static sInt gFrameNo;
// KK_FIXEDDT=1 runs the clock at exactly 60 frames per second, for reproducible debug shots
sInt sSystem_::GetTime()
{
  static sBool fixed = getenv("KK_FIXEDDT")!=0;
  if(fixed) return gFrameNo*1000/60;
  return (sInt)(SDL_GetTicks()-gStartMs);
}
void sSystem_::GetPerf(sPerfInfo &i,sInt){ sSetMem(&i,0,sizeof(i)); }
sInt sSystem_::GetTimeOfDay(){ return (sInt)time(0); }
void sSystem_::PerfKalib(){}
sU32 sSystem_::PerfTime(){ return GetTime()*1000; }
sBool sSystem_::GetWinMouse(sInt &x,sInt &y){ x=gMouseX; y=gMouseY; return 1; }
sBool sSystem_::GetWinMouseAbs(sInt &x,sInt &y){ x=gMouseX; y=gMouseY; return 1; }
void sSystem_::SetWinMouse(sInt x,sInt y){ SDL_WarpMouseInWindow(gWin,x,y); }
void sSystem_::SetWinTitle(sChar *n){ if(gWin) SDL_SetWindowTitle(gWin,n); }
void sSystem_::MoveWindow(sInt,sInt){}
void sSystem_::SetWinMode(sInt){}
sInt sSystem_::GetWinMode(){ return 0; }
void sSystem_::HideWinMouse(sBool hide){ SDL_ShowCursor(hide?SDL_DISABLE:SDL_ENABLE); }

void sSystem_::SetSoundHandler(sSoundHandler hnd,sInt,void *user)
{
  std::lock_guard<std::mutex> lock(gSndMu);
  gSndH=hnd; gSndUser=user;
}
sInt sSystem_::GetCurrentSample(){ return GetTime()*44; }

#if sLINK_KKRIEGER
// software replacement for the DirectSound buffers: every sample owns a few
// voices that are reused round-robin, like the duplicated d3d sound buffers.
struct SampleVoice
{
  sInt Pos;               // -1 = stopped
  sF32 Volume,Pan;
  sBool Has3D;
  sVector Pos3D;
  sF32 MinDist,MaxDist;
};
struct SampleSlot
{
  std::vector<sS16> PCM;  // stereo
  sInt Length;            // in sample frames
  sBool Is3D;
  sInt LRU,Uses;
  std::vector<SampleVoice> Voices;
};
static SampleSlot gSamples[sMAXSAMPLEHANDLE];
static sVector gListenerPos,gListenerRight;

static void sampleVoiceGains(const SampleVoice &v,sF32 &gl,sF32 &gr)
{
  sF32 vol = v.Volume;
  sF32 pan = v.Pan;

  if(v.Has3D)
  {
    sVector d;
    d.Sub3(v.Pos3D,gListenerPos);
    sF32 dist = sFSqrt(d.Dot3(d));
    sF32 eff = sRange(dist,v.MaxDist,v.MinDist);
    vol *= v.MinDist / eff;
    if(dist>1e-4f)
      pan = d.Dot3(gListenerRight) / dist;
  }

  pan = sRange(pan,1.0f,-1.0f);
  gl = vol * sFSqrt(0.5f*(1.0f-pan));
  gr = vol * sFSqrt(0.5f*(1.0f+pan));
}

static void mixSamples(sS16 *out,sInt frames)
{
  for(sInt h=0;h<sMAXSAMPLEHANDLE;h++)
  {
    SampleSlot &s = gSamples[h];
    for(auto &v : s.Voices)
    {
      if(v.Pos<0) continue;
      sF32 gl,gr;
      sampleVoiceGains(v,gl,gr);
      const sS16 *src = s.PCM.data();
      sInt n = sMin(frames,s.Length-v.Pos);
      for(sInt i=0;i<n;i++)
      {
        const sS16 *p = src + (v.Pos+i)*2;
        sF32 l = p[0], r = p[1];
        if(s.Is3D) l = r = (l+r)*0.5f;
        out[i*2+0] = sRange<sInt>(out[i*2+0] + (sInt)(l*gl),0x7fff,-0x7fff);
        out[i*2+1] = sRange<sInt>(out[i*2+1] + (sInt)(r*gr),0x7fff,-0x7fff);
      }
      v.Pos += n;
      if(v.Pos>=s.Length) v.Pos = -1;
    }
  }
}

sInt sSystem_::SampleAdd(sS16 *data,sInt size,sInt buffers,sInt handle,sBool is3d)
{
  std::lock_guard<std::mutex> lock(gSndMu);
  if(handle==sINVALID)
  {
    for(sInt i=0;i<sMAXSAMPLEHANDLE;i++)
    {
      if(gSamples[i].Voices.empty())
      {
        handle = i;
        break;
      }
    }
  }
  else if(handle<0 || handle>=sMAXSAMPLEHANDLE || !gSamples[handle].Voices.empty())
    handle = sINVALID;

  if(handle!=sINVALID)
  {
    SampleSlot &s = gSamples[handle];
    s.PCM.assign(data,data+size*2);
    s.Length = size;
    s.Is3D = is3d;
    s.LRU = 0;
    s.Uses = 0;
    s.Voices.assign(sMax(buffers,1),SampleVoice());
    for(auto &v : s.Voices)
      v.Pos = -1;
  }
  return handle;
}

void sSystem_::SampleRem(sInt h)
{
  std::lock_guard<std::mutex> lock(gSndMu);
  if(h>=0 && h<sMAXSAMPLEHANDLE)
  {
    gSamples[h].Voices.clear();
    gSamples[h].PCM.clear();
  }
}

void sSystem_::SampleRemAll()
{
  for(sInt i=0;i<sMAXSAMPLEHANDLE;i++)
    SampleRem(i);
}

sInt sSystem_::SamplePlay(sInt handle,sF32 volume,sF32 pan,sInt)
{
  std::lock_guard<std::mutex> lock(gSndMu);
  if(handle<0 || handle>=sMAXSAMPLEHANDLE || gSamples[handle].Voices.empty())
    return -1;

  SampleSlot &s = gSamples[handle];
  s.LRU = (s.LRU+1) % (sInt)s.Voices.size();
  SampleVoice &v = s.Voices[s.LRU];
  v.Pos = 0;
  v.Volume = sRange(volume,1.0f,0.0f);
  v.Pan = s.Is3D ? 0.0f : pan;
  v.Has3D = sFALSE;
  return ++s.Uses;
}

void sSystem_::Sample3DParam(sInt handle,sInt bufnum,const sVector &pos,const sVector &,sF32 minDist,sF32 maxDist)
{
  std::lock_guard<std::mutex> lock(gSndMu);
  if(handle<0 || handle>=sMAXSAMPLEHANDLE) return;

  SampleSlot &s = gSamples[handle];
  sInt count = (sInt)s.Voices.size();
  if(!s.Is3D || bufnum<0 || s.Uses-bufnum>=count) return;

  SampleVoice &v = s.Voices[bufnum%count];
  if(v.Pos<0) return;
  v.Has3D = sTRUE;
  v.Pos3D = pos;
  v.MinDist = sMax(minDist,1e-3f);
  v.MaxDist = sMax(maxDist,v.MinDist);
}

void sSystem_::Sample3DListener(const sVector &pos,const sVector &,const sVector &up,const sVector &fwd,sF32,sF32)
{
  std::lock_guard<std::mutex> lock(gSndMu);
  gListenerPos = pos;
  gListenerRight.Cross3(up,fwd);
  gListenerRight.Unit3();
}

void sSystem_::Sample3DCommit(){}
#endif

sU8 *sSystem_::LoadFile(const sChar *name,sInt &size)
{
  FILE *f=fopen(name,"rb"); if(!f) return 0;
  fseek(f,0,SEEK_END); size=(sInt)ftell(f); fseek(f,0,SEEK_SET);
  sU8 *p=new sU8[size+16]; memset(p,0,size+16);
  fread(p,1,size,f); fclose(f); return p;
}
sU8 *sSystem_::LoadFile(const sChar *name){ sInt sz; return LoadFile(name,sz); }
sU8 *sSystem_::LoadFileIfNewerThan(const sChar *name,const sChar *,sInt &size){ return LoadFile(name,size); }
sChar *sSystem_::LoadText(const sChar *name)
{
  sInt sz; sU8 *p=LoadFile(name,sz); return (sChar*)p;
}
sBool sSystem_::SaveFile(const sChar *name,const sU8 *data,sInt size)
{
  FILE *f=fopen(name,"wb"); if(!f) return 0; fwrite(data,1,size,f); fclose(f); return 1;
}
sU64 sSystem_::GetFileStamp(const sChar *name){ struct stat st; return stat(name,&st)?0:(sU64)st.st_mtime; }
sDirEntry *sSystem_::LoadDir(const sChar *){ return 0; }
sBool sSystem_::MakeDir(const sChar *){ return 0; }
sBool sSystem_::CheckDir(const sChar *n){ struct stat st; return stat(n,&st)==0 && S_ISDIR(st.st_mode); }
sBool sSystem_::CheckFile(const sChar *n){ struct stat st; return stat(n,&st)==0; }
sBool sSystem_::RenameFile(const sChar *a,const sChar *b){ return rename(a,b)==0; }
sBool sSystem_::DeleteFile(const sChar *n){ return unlink(n)==0; }
void sSystem_::GetCurrentDir(sChar *buffer,sInt size){ getcwd(buffer,size); }
sU32 sSystem_::GetDriveMask(){ return 1; }
sBitmap *sSystem_::LoadBitmap(const sU8 *,sInt){ return 0; }
sBool sSystem_::LoadBitmapCore(const sU8 *,sInt,sInt &,sInt &,sU8 *&){ return 0; }

static CTFontRef createFont(const sChar *name,CGFloat size,const CGAffineTransform *mat,sInt style)
{
  CFStringRef nm=CFStringCreateWithCString(0,name,kCFStringEncodingWindowsLatin1);
  CTFontRef f=CTFontCreateWithName(nm,size,mat);
  CFRelease(nm);
  CTFontSymbolicTraits traits = ((style&2) ? kCTFontBoldTrait : 0) | ((style&1) ? kCTFontItalicTrait : 0);
  if(traits)
  {
    CTFontRef t=CTFontCreateCopyWithSymbolicTraits(f,size,mat,traits,traits);
    if(t) { CFRelease(f); f=t; }
  }
  return f;
}

// mirrors the win32 version: CreateFont(-ys,xs,...), i.e. ys is the em size and
// xs (if nonzero) the average character width.
sInt sSystem_::FontBegin(sInt pagex,sInt pagey,const sChar *name,sInt xs,sInt ys,sInt style)
{
  gFontW=pagex; gFontH=pagey;
  delete[] gFontMem; gFontMem=new sU32[pagex*pagey];
  sSetMem(gFontMem,0,pagex*pagey*4);
  if(gFont) CFRelease(gFont);
  if(!name || !name[0]) name = "Arial";
  CGFloat em = ys ? sAbs(ys) : 16;
  gFont=createFont(name,em,0,style);
  if(xs>0)
  {
    UniChar uc[26]; CGGlyph g[26]; CGSize adv[26];
    for(sInt i=0;i<26;i++) uc[i]='a'+i;
    CTFontGetGlyphsForCharacters(gFont,uc,g,26);
    CTFontGetAdvancesForGlyphs(gFont,kCTFontOrientationDefault,g,adv,26);
    CGFloat avg=0; for(sInt i=0;i<26;i++) avg+=adv[i].width; avg/=26;
    if(avg>0)
    {
      CGAffineTransform m=CGAffineTransformMakeScale(xs/avg,1);
      CFRelease(gFont); gFont=createFont(name,em,&m,style);
    }
  }
  FontMem=gFontMem;
  return (sInt)ceil(CTFontGetAscent(gFont)+CTFontGetDescent(gFont));
}
sInt sSystem_::FontWidth(const sChar *string,sInt count)
{
  if(count<0) count=sGetStringLen(string);
  CFStringRef s=CFStringCreateWithBytes(0,(const UInt8*)string,count,kCFStringEncodingWindowsLatin1,false);
  CFIndex n=CFStringGetLength(s);
  UniChar *uc=new UniChar[n]; CGGlyph *g=new CGGlyph[n]; CGSize *adv=new CGSize[n];
  CFStringGetCharacters(s,CFRangeMake(0,n),uc);
  CTFontGetGlyphsForCharacters(gFont,uc,g,n);
  CTFontGetAdvancesForGlyphs(gFont,kCTFontOrientationDefault,g,adv,n);
  sF32 sum=0; for(CFIndex i=0;i<n;i++) sum+=adv[i].width;
  delete[] uc; delete[] g; delete[] adv; CFRelease(s);
  return (sInt)sum;
}
void sSystem_::FontCharWidth(sInt ch,sInt *widths)
{
  sU8 c8=(sU8)ch; UniChar u=c8;
  CFStringRef cs=CFStringCreateWithBytes(0,&c8,1,kCFStringEncodingWindowsLatin1,false);
  if(cs) { if(CFStringGetLength(cs)==1) u=CFStringGetCharacterAtIndex(cs,0); CFRelease(cs); }
  CGGlyph g=0; CGSize adv;
  CTFontGetGlyphsForCharacters(gFont,&u,&g,1);
  CTFontGetAdvancesForGlyphs(gFont,kCTFontOrientationDefault,&g,&adv,1);
  CGRect bb = CTFontGetBoundingRectsForGlyphs(gFont,kCTFontOrientationDefault,&g,0,1);
  sInt advance = (sInt)ceil(adv.width);
  sInt a = (sInt)floor(bb.origin.x);
  sInt b = (sInt)ceil(bb.origin.x+bb.size.width) - a;
  if(b<=0) { a = 0; b = advance; }
  widths[0]=a; widths[1]=b; widths[2]=advance-a-b;
}
void sSystem_::FontPrint(sInt x,sInt y,const sChar *string,sInt count)
{
  if(count<0) count=sGetStringLen(string);
  CGColorSpaceRef cs=CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx=CGBitmapContextCreate(gFontMem,gFontW,gFontH,8,gFontW*4,cs,kCGImageAlphaPremultipliedLast);
  CGContextSetRGBFillColor(ctx,1,1,1,1);
  CFStringRef s=CFStringCreateWithBytes(0,(const UInt8*)string,count,kCFStringEncodingWindowsLatin1,false);
  CFIndex n=CFStringGetLength(s);
  UniChar *uc=new UniChar[n]; CGGlyph *gl=new CGGlyph[n];
  CFStringGetCharacters(s,CFRangeMake(0,n),uc);
  CTFontGetGlyphsForCharacters(gFont,uc,gl,n);
  CGFloat xp=x;
  for(CFIndex i=0;i<n;i++){
    // CG bitmap y runs bottom-up; memory row 0 is the top of the page
    CGPoint pt=CGPointMake(xp, gFontH-(y+CTFontGetAscent(gFont)));
    CTFontDrawGlyphs(gFont,gl+i,&pt,1,ctx);
    CGSize adv; CTFontGetAdvancesForGlyphs(gFont,kCTFontOrientationDefault,gl+i,&adv,1);
    xp+=adv.width;
  }
  delete[] uc; delete[] gl; CFRelease(s); CGContextRelease(ctx); CGColorSpaceRelease(cs);
}
void sSystem_::FontPrint(sInt x,sInt y,const sU16 *string,sInt count)
{
  sChar tmp[256]; sInt n=0;
  if(count<0){ while(string[n]&&n<255){ tmp[n]=(sChar)string[n]; n++; } }
  else { n=sMin(count,255); for(sInt i=0;i<n;i++) tmp[i]=(sChar)string[i]; }
  tmp[n]=0; FontPrint(x,y,tmp,n);
}
void sSystem_::FontEnd(){}

sBool sSystem_::GetScreenInfo(sInt i,sScreenInfo &info)
{
  if(i!=0) return 0;
  info.XSize=ConfigX; info.YSize=ConfigY; info.FullScreen=0; info.LowQuality=0;
  info.ShaderLevel=sPS_14; info.PixelRatio=1; return 1;
}
sInt sSystem_::GetScreenCount(){ return 1; }
sBool sSystem_::GetFullscreen(){ return 0; }
void sSystem_::Reset(sU32,sInt,sInt,sInt,sInt){}
void sSystem_::SetGamma(sF32){}

/****************************************************************************/
/***   render targets, viewport, clear                                    ***/
/****************************************************************************/

void sSystem_::Clear(sU32 flags,sU32 color)
{
  GLbitfield m = 0;
  if(flags & sVCF_COLOR) m |= GL_COLOR_BUFFER_BIT;
  if(flags & sVCF_Z) m |= GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT;
  if(flags & sVCF_STENCIL) m |= GL_STENCIL_BUFFER_BIT;
  if(!m) return;

  // d3d clears the viewport (intersected with the scissor rect) and ignores write masks
  sInt x0 = CurrentViewport.Window.x0, y0 = CurrentViewport.Window.y0;
  sInt x1 = CurrentViewport.Window.x1, y1 = CurrentViewport.Window.y1;
  if(gRS[sD3DRS_SCISSORTESTENABLE])
  {
    x0 = sMax(x0,gScissor[0]); y0 = sMax(y0,gScissor[1]);
    x1 = sMin(x1,gScissor[2]); y1 = sMin(y1,gScissor[3]);
  }
  if(gTrace) fprintf(stderr,"  clear %x %08x %d,%d-%d,%d\n",flags,color,x0,y0,x1,y1);
  if(x1<=x0 || y1<=y0) return;
  glEnable(GL_SCISSOR_TEST);
  glScissor(x0,y0,x1-x0,y1-y0);
  glColorMask(1,1,1,1);
  glDepthMask(GL_TRUE);
  glStencilMask(0xff);
  glClearColor(((color>>16)&255)/255.0f,((color>>8)&255)/255.0f,(color&255)/255.0f,((color>>24)&255)/255.0f);
  glClearDepth(1.0);
  glClearStencil(0);
  glClear(m);
}

void sSystem_::SetViewport(const sViewport &vp)
{
  sInt txs,tys;
  GLuint color;

  CurrentViewport = vp;
  if(vp.RenderTarget==sINVALID)
  {
    txs = Screen[0].XSize;
    tys = Screen[0].YSize;
    color = gScreenTex;
  }
  else
  {
    IDirect3DTexture9 *t = Textures[vp.RenderTarget].Tex;
    txs = Textures[vp.RenderTarget].XSize;
    tys = Textures[vp.RenderTarget].YSize;
    color = t ? t->Tex : gScreenTex;
  }

  if(!vp.Window.XSize() || !vp.Window.YSize())
  {
    CurrentViewport.Window.x0 = 0;
    CurrentViewport.Window.y0 = 0;
    CurrentViewport.Window.x1 = txs;
    CurrentViewport.Window.y1 = tys;
  }

  ViewportX = CurrentViewport.Window.XSize();
  ViewportY = CurrentViewport.Window.YSize();
  if(gTrace)
  {
    static GLuint lastTex;
    static sInt lastW,lastH;
    if(lastTex && gTraceSeq)
    {
      char name[64];
      snprintf(name,sizeof(name),"/tmp/kkrt_%02d.bmp",gTraceSeq);
      dumpTexture(lastTex,lastW,lastH,name);
    }
    gTraceSeq++;
    lastTex = color; lastW = txs; lastH = tys;
    fprintf(stderr,"viewport #%d rt%d (%dx%d) win %d,%d-%d,%d\n",gTraceSeq,vp.RenderTarget,txs,tys,
      CurrentViewport.Window.x0,CurrentViewport.Window.y0,CurrentViewport.Window.x1,CurrentViewport.Window.y1);
  }

  attachTarget(color);
  // fbo rows are stored top-down like d3d surfaces, so the window maps directly
  glViewport(CurrentViewport.Window.x0,CurrentViewport.Window.y0,ViewportX,ViewportY);
}

void sSystem_::GrabScreen(sInt sourceRT,const sRect &win,sInt tex,sBool stretch)
{
  if(tex<0 || tex>=MAX_TEXTURE || !Textures[tex].Tex) return;
  GLuint src = gScreenTex;
  if(sourceRT>=0 && sourceRT<MAX_TEXTURE && Textures[sourceRT].Tex)
    src = Textures[sourceRT].Tex->Tex;
  IDirect3DTexture9 *dst = Textures[tex].Tex;
  if(src==dst->Tex) return;

  sInt dw = stretch ? dst->XS : win.XSize();
  sInt dh = stretch ? dst->YS : win.YSize();
  if(gTrace) fprintf(stderr,"  grab rt%d %d,%d-%d,%d -> tex%d %dx%d\n",sourceRT,win.x0,win.y0,win.x1,win.y1,tex,dw,dh);

  glBindFramebuffer(GL_READ_FRAMEBUFFER,gBlitFBO[0]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,src,0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,gBlitFBO[1]);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,dst->Tex,0);
  glDisable(GL_SCISSOR_TEST);
  glColorMask(1,1,1,1);
  glBlitFramebuffer(win.x0,win.y0,win.x1,win.y1,0,0,dw,dh,GL_COLOR_BUFFER_BIT,
    (dw==win.XSize() && dh==win.YSize()) ? GL_NEAREST : GL_LINEAR);
  glBindFramebuffer(GL_FRAMEBUFFER,gFBO);
}

void sSystem_::Begin2D(sU32 **,sInt &){}
void sSystem_::End2D(){}
void sSystem_::GetTransform(sInt mode,sMatrix &mat)
{
  switch(mode){
  case sGT_VIEW: mat=LastCamera; break;
  case sGT_MODEL: mat=LastMatrix; break;
  case sGT_PROJECT: mat=LastProjection; break;
  default: mat=LastViewProject; break;
  }
}

/****************************************************************************/
/***   textures                                                           ***/
/****************************************************************************/

static void uploadTexture(IDirect3DTexture9 *t,sInt format,const sU16 *source,sInt mipcount,sInt miptresh)
{
  sInt xs = t->XS, ys = t->YS, oxs = xs, level = 0;
  sInt mipdir = miptresh & 16;
  miptresh &= 15;
  sBool snorm = (format==sTF_Q8W8V8U8);

  std::vector<sU16> data(source,source+xs*ys*4);
  std::vector<sU8> out(xs*ys*4);

  glBindTexture(GL_TEXTURE_2D,t->Tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  for(;;)
  {
    for(sInt y=0;y<ys;y++)
    {
      const sU16 *s = &data[y*oxs*4];
      sU8 *d = &out[y*xs*4];
      for(sInt x=0;x<xs;x++,s+=4,d+=4)
      {
        if(snorm)
        {
          sVector v;
          v.x = s[2]-0x4000;
          v.y = s[1]-0x4000;
          v.z = s[0]-0x4000;
          v.UnitSafe3();
          d[0] = sInt(v.x*127.0f)&0xff;
          d[1] = sInt(v.y*127.0f)&0xff;
          d[2] = sInt(v.z*127.0f)&0xff;
          d[3] = (((sInt)s[3]-0x4000)>>7)&0xff;
        }
        else
        {
          d[0] = (s[0]>>7)&0xff;
          d[1] = (s[1]>>7)&0xff;
          d[2] = (s[2]>>7)&0xff;
          d[3] = (s[3]>>7)&0xff;
        }
      }
    }
    if(snorm)
      glTexImage2D(GL_TEXTURE_2D,level,GL_RGBA8_SNORM,xs,ys,0,GL_RGBA,GL_BYTE,out.data());
    else
      glTexImage2D(GL_TEXTURE_2D,level,GL_RGBA8,xs,ys,0,GL_BGRA,GL_UNSIGNED_BYTE,out.data());

    if(xs<=1 || ys<=1 || (mipcount && level+1>=mipcount))
      break;

    sBool filter = (miptresh <= level);
    if(mipdir) filter = !filter;
    for(sInt y=0;y<ys/2;y++)
    {
      const sU16 *s = &data[(2*y)*oxs*4];
      sU16 *d = &data[y*oxs*4];
      for(sInt x=0;x<xs/2;x++,s+=8,d+=4)
      {
        for(sInt c=0;c<4;c++)
          d[c] = filter ? (sU16)((s[c]+s[c+4]+s[oxs*4+c]+s[oxs*4+c+4]+2)>>2) : s[c];
      }
    }
    xs >>= 1;
    ys >>= 1;
    level++;
  }
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,level);
}

static void bitmapTo16(const sBitmap *bm,std::vector<sU16> &tmp)
{
  sInt n = bm->XSize*bm->YSize;
  tmp.resize(n*4);
  const sU8 *s = (const sU8 *)bm->Data;
  for(sInt i=0;i<n*4;i++)
    tmp[i] = s[i]<<7;
}

static sInt allocTexture(sSystem_ *sys,sInt xs,sInt ys,sInt format,sBool rt)
{
  for(sInt i=0;i<MAX_TEXTURE;i++)
  {
    sHardTex *tex = &sys->Textures[i];
    if(tex->Flags) continue;
    if(rt && xs==0) { xs = sys->Screen[0].XSize; ys = sys->Screen[0].YSize; }
    tex->RefCount = 1;
    tex->XSize = xs;
    tex->YSize = ys;
    tex->Flags = sTIF_ALLOCATED | (rt ? sTIF_RENDERTARGET : 0);
    tex->Format = format;
    tex->MipLevels = 1;
    IDirect3DTexture9 *t = new IDirect3DTexture9;
    glGenTextures(1,&t->Tex);
    t->XS = xs;
    t->YS = ys;
    t->RT = rt;
    tex->Tex = t;
    if(rt)
    {
      std::vector<sU8> zero(xs*ys*4,0);
      glBindTexture(GL_TEXTURE_2D,t->Tex);
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,xs,ys,0,GL_BGRA,GL_UNSIGNED_BYTE,zero.data());
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
      ensureDepth(xs,ys);
    }
    return i;
  }
  return sINVALID;
}

sInt sSystem_::AddTexture(const sTexInfo &ti)
{
  if(ti.Flags & sTIF_RENDERTARGET)
    return allocTexture(this,ti.XSize,ti.YSize,sTF_A8R8G8B8,sTRUE);
  sInt h = allocTexture(this,ti.XSize,ti.YSize,sTF_A8R8G8B8,sFALSE);
  if(h!=sINVALID && ti.Bitmap)
    UpdateTexture(h,ti.Bitmap);
  return h;
}

sInt sSystem_::AddTexture(sInt xs,sInt ys,sInt format,sU16 *data,sInt mipcount,sInt miptresh)
{
  if(!data)
    return allocTexture(this,xs,ys,format,sTRUE);

  mipcount = sMin(mipcount,sGetPower2(xs));
  mipcount = sMin(mipcount,sGetPower2(ys));
  if((xs & (xs-1)) || (ys & (ys-1)))
    mipcount = 1;
  sInt h = allocTexture(this,xs,ys,format,sFALSE);
  if(h==sINVALID) return h;
  Textures[h].MipLevels = mipcount;
  UpdateTexture(h,data,miptresh);
  return h;
}

void sSystem_::AddRefTexture(sInt tex){ if(tex>=0) Textures[tex].RefCount++; }

void sSystem_::RemTexture(sInt handle)
{
  if(handle<0 || !Textures[handle].Flags || Textures[handle].RefCount<=0) return;
  if(--Textures[handle].RefCount<=0)
  {
    for(sInt i=0;i<MAX_TEXSTAGE;i++)
    {
      if(gStageTex[i]==handle) gStageTex[i] = sINVALID;
      if(CurrentTextures[i]==handle) CurrentTextures[i] = sINVALID;
    }
    sRelease(Textures[handle].Tex);
    Textures[handle].Flags = 0;
  }
}

void sSystem_::GetTextureSize(sInt tex,sInt &w,sInt &h){ w=Textures[tex].XSize; h=Textures[tex].YSize; }

void sSystem_::UpdateTexture(sInt handle,sBitmap *bm)
{
  sHardTex *tex = &Textures[handle];
  if(tex->Flags & sTIF_RENDERTARGET) return;
  std::vector<sU16> tmp;
  bitmapTo16(bm,tmp);
  uploadTexture(tex->Tex,sTF_A8R8G8B8,tmp.data(),0,0);
}

void sSystem_::UpdateTexture(sInt handle,sU16 *source,sInt miptresh)
{
  sHardTex *tex = &Textures[handle];
  if(tex->Flags & sTIF_RENDERTARGET) return;
  uploadTexture(tex->Tex,tex->Format,source,tex->MipLevels,miptresh);
}

void sSystem_::ReadTexture(sInt handle,sU16 *dest)
{
  sHardTex *tex = &Textures[handle];
  std::vector<sU8> buf(tex->XSize*tex->YSize*4);
  glBindTexture(GL_TEXTURE_2D,tex->Tex->Tex);
  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glGetTexImage(GL_TEXTURE_2D,0,GL_BGRA,GL_UNSIGNED_BYTE,buf.data());
  for(size_t i=0;i<buf.size();i++)
    dest[i] = (buf[i]<<7) | (buf[i]>>1);
}

sBool sSystem_::StreamTextureStart(sInt,sInt,sBitmapLock &){ return 0; }
void sSystem_::StreamTextureEnd(){}
void sSystem_::FlushTexture(sInt){}

void sSystem_::MakeCubeNormalizer()
{
  const sInt size = 64;
  static sVector faces[6][2] =
  {
    {{ 0, 0,-1},{ 0, 1, 0}},
    {{ 0, 0, 1},{ 0, 1, 0}},
    {{ 1, 0, 0},{ 0, 0,-1}},
    {{ 1, 0, 0},{ 0, 0, 1}},
    {{ 1, 0, 0},{ 0, 1, 0}},
    {{-1, 0, 0},{ 0, 1, 0}}
  };
  std::vector<sU8> img(size*size*4);
  sVector v;

  glGenTextures(1,&gCubeTex);
  glBindTexture(GL_TEXTURE_CUBE_MAP,gCubeTex);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  for(sInt i=0;i<6;i++)
  {
    sU8 *p = img.data();
    for(sInt y=0;y<size;y++)
    {
      for(sInt x=0;x<size;x++)
      {
        v.Cross3(faces[i][0],faces[i][1]);
        v.Scale3((size-1)*0.5f);
        v.AddScale3(faces[i][0],(x-(size-1)*0.5f));
        v.AddScale3(faces[i][1],(-y+(size-1)*0.5f));
        v.Unit3();
        p[0] = sFtol(128.0f+v.z*127);
        p[1] = sFtol(128.0f+v.y*127);
        p[2] = sFtol(128.0f+v.x*127);
        p[3] = 0;
        p += 4;
      }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGBA8,size,size,0,GL_BGRA,GL_UNSIGNED_BYTE,img.data());
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_BASE_LEVEL,0);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAX_LEVEL,0);
}

void sSystem_::MakeAttenuationVolume()
{
  const sInt size = 32;
  const sF32 scale = 2.0f / (size - 2.0f);
  const sF32 mid = size / 2.0f;
  std::vector<sU8> img(size*size*size*4);
  sU8 *p = img.data();
  sVector v;

  for(sInt z=0;z<size;z++)
  {
    v.z = (z - mid) * scale;
    for(sInt y=0;y<size;y++)
    {
      v.y = (y - mid) * scale;
      for(sInt x=0;x<size;x++)
      {
        v.x = (x - mid) * scale;
        sF32 attn = sMax(1.0f - v.Dot3(v),0.0f);
        p[0] = p[1] = p[2] = p[3] = sFtol(attn * 255);
        p += 4;
      }
    }
  }
  glGenTextures(1,&gVolumeTex);
  glBindTexture(GL_TEXTURE_3D,gVolumeTex);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA8,size,size,size,0,GL_BGRA,GL_UNSIGNED_BYTE,img.data());
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_BASE_LEVEL,0);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAX_LEVEL,0);
}

void sSystem_::SetScissor(const sFRect *scissor)
{
  if(!scissor)
    SetState(sD3DRS_SCISSORTESTENABLE,0);
  else
  {
    const sRect &w = CurrentViewport.Window;
    gScissor[0] = w.x0 + (sInt)((w.x1 - w.x0) * (1.0f + scissor->x0) * 0.5f);
    gScissor[2] = w.x0 + (sInt)((w.x1 - w.x0) * (1.0f + scissor->x1) * 0.5f);
    gScissor[1] = w.y0 + (sInt)((w.y1 - w.y0) * (1.0f - scissor->y1) * 0.5f);
    gScissor[3] = w.y0 + (sInt)((w.y1 - w.y0) * (1.0f - scissor->y0) * 0.5f);
    SetState(sD3DRS_SCISSORTESTENABLE,1);
  }
}

/****************************************************************************/
/***   geometry                                                           ***/
/****************************************************************************/

void sSystem_::CreateGeoBuffer(sInt i,sInt dyn,sInt index,sInt size)
{
  GeoBuffer[i].Type = 1+index;
  GeoBuffer[i].Size = size;
  GeoBuffer[i].Used = 0;
  GeoBuffer[i].UserCount = 0;
  GeoBuffer[i].VB = 0;
  if(!gGeoGL[i]) glGenBuffers(1,&gGeoGL[i]);
  glBindBuffer(GL_ARRAY_BUFFER,gGeoGL[i]);
  glBufferData(GL_ARRAY_BUFFER,size,0,dyn ? GL_STREAM_DRAW : GL_STATIC_DRAW);
  gGeoDyn[i] = dyn;
}

sInt sSystem_::GeoAdd(sInt fvf,sInt prim)
{
  for(sInt i=1;i<MAX_GEOHANDLE;i++)
  {
    sGeoHandle *gh = &GeoHandle[i];
    if(gh->Mode==0)
    {
      sSetMem(gh,0,sizeof(*gh));
      gh->VertexSize = gFvfSize[fvf];
      gh->FVF = fvf;
      gh->Mode = prim;
      gh->Locked = 0;
      return i;
    }
  }
  sFatal("GeoAdd() ran out of handles");
  return 0;
}

void sSystem_::GeoRem(sInt handle)
{
  if(GeoHandle[handle].Vertex.Buffer>=4)
    GeoBuffer[GeoHandle[handle].Vertex.Buffer].Free();
  if(GeoHandle[handle].Index.Buffer>=4)
    GeoBuffer[GeoHandle[handle].Index.Buffer].Free();
  GeoHandle[handle].Mode = 0;
}

void sSystem_::GeoFlush()
{
  for(sInt i=1;i<MAX_GEOHANDLE;i++)
    GeoHandle[i].Vertex.Count = 0;
  for(sInt i=4;i<MAX_GEOBUFFER;i++)
  {
    if(gGeoGL[i]) { glDeleteBuffers(1,&gGeoGL[i]); gGeoGL[i] = 0; }
    GeoBuffer[i].Init();
  }
}

void sSystem_::GeoFlush(sInt handle,sInt what)
{
  if(what & sGEO_VERTEX) GeoHandle[handle].Vertex.DiscardCount = 0;
  if(what & sGEO_INDEX)  GeoHandle[handle].Index.DiscardCount = 0;
}

void sSystem_::AllocBufferInternal(sGeoBufferRef &ref,sInt size,sBool stat,sInt index,void **ptr)
{
  sInt i;
  sBool orphan = sFALSE;

  if(stat)
  {
    for(i=4;i<MAX_GEOBUFFER;i++)
      if(GeoBuffer[i].Alloc(ref.Count,size,ref.Start,index+1))
        break;

    if(i==MAX_GEOBUFFER)
    {
      for(i=4;i<MAX_GEOBUFFER;i++)
      {
        if(GeoBuffer[i].Type==0)
        {
          sInt bufSize = index ? MAX_DYNIBSIZE*index : MAX_DYNVBSIZE;
          if(bufSize < ref.Count*size)
            bufSize = ref.Count*size;
          CreateGeoBuffer(i,0,index,bufSize);
          GeoBuffer[i].Alloc(ref.Count,size,ref.Start,index+1);
          break;
        }
      }
      if(i==MAX_GEOBUFFER)
        sFatal("AllocInternal(): out of static geobuffers");
    }
    ref.Buffer = i;
  }
  else
  {
    sBool ok = GeoBuffer[index].Alloc(ref.Count,size,ref.Start,index+1);
    ref.Buffer = index;
    if(!ok)
    {
      GeoBuffer[index].Used = 0;
      DiscardCount[index]++;
      GeoBuffer[index].Alloc(ref.Count,size,ref.Start,index+1);
    }
    ref.DiscardCount = DiscardCount[index];
    orphan = (ref.Start==0);
  }

  if(orphan)
  {
    glBindBuffer(GL_ARRAY_BUFFER,gGeoGL[ref.Buffer]);
    glBufferData(GL_ARRAY_BUFFER,GeoBuffer[ref.Buffer].Size,0,GL_STREAM_DRAW);
  }

  GeoStage *st = gAllocStage;
  st->Buffer = ref.Buffer;
  st->Start = ref.Start;
  st->Count = ref.Count;
  st->ElemSize = size;
  st->Active = sTRUE;
  if((sInt)st->Data.size() < ref.Count*size+16)
    st->Data.resize(ref.Count*size+16);
  *ptr = st->Data.data();
}

void sSystem_::GeoBegin(sInt handle,sInt vc,sInt ic,sF32 **fp,void **ip,sInt upd)
{
  sGeoHandle *gh = &GeoHandle[handle];

  if(fp) *fp = 0;
  if(ip) *ip = 0;

  if(upd & sGEO_VERTEX)
  {
    if(gh->Vertex.Buffer>=4)
      GeoBuffer[gh->Vertex.Buffer].Free();
    gh->Vertex.Buffer = 0;
    gh->Vertex.Start = 0;
    gh->Vertex.Count = vc;
    gAllocStage = &gStage[handle][0];
    void *p = 0;
    AllocBufferInternal(gh->Vertex,gh->VertexSize,gh->Mode & sGEO_STATVB,0,&p);
    if(fp) *fp = (sF32 *)p;
    gh->Locked |= sGEO_VERTEX;
  }

  if(upd & sGEO_INDEX)
  {
    if(gh->Index.Buffer>=4)
      GeoBuffer[gh->Index.Buffer].Free();
    gh->Index.Buffer = 0;
    gh->Index.Start = 0;
    gh->Index.Count = ic;
    if(ic)
    {
      sBool is32b = (gh->Mode & sGEO_IND32B) ? sTRUE : sFALSE;
      gAllocStage = &gStage[handle][1];
      void *p = 0;
      AllocBufferInternal(gh->Index,is32b ? 4 : 2,gh->Mode & sGEO_STATIB,is32b ? 2 : 1,&p);
      if(ip) *ip = p;
      gh->Locked |= sGEO_INDEX;
    }
  }
}

static void flushStage(GeoStage &st)
{
  if(!st.Active) return;
  st.Active = sFALSE;
  if(st.Count>0)
  {
    glBindBuffer(GL_ARRAY_BUFFER,gGeoGL[st.Buffer]);
    glBufferSubData(GL_ARRAY_BUFFER,(GLintptr)st.Start*st.ElemSize,(GLsizeiptr)st.Count*st.ElemSize,st.Data.data());
  }
  if(!gGeoDyn[st.Buffer])
    std::vector<sU8>().swap(st.Data);
}

void sSystem_::GeoEnd(sInt handle,sInt vc,sInt ic)
{
  sGeoHandle *gh = &GeoHandle[handle];

  if(gh->Locked & sGEO_VERTEX)
  {
    GeoStage &st = gStage[handle][0];
    if(vc>=0 && vc<st.Count) st.Count = vc;
    gh->Locked &= ~sGEO_VERTEX;
  }
  if(gh->Locked & sGEO_INDEX)
  {
    GeoStage &st = gStage[handle][1];
    if(ic>=0 && ic<st.Count) st.Count = ic;
    gh->Locked &= ~sGEO_INDEX;
  }

  if(vc!=-1) gh->Vertex.Count = vc;
  if(ic!=-1) gh->Index.Count = ic;
}

sInt sSystem_::GeoDraw(sInt &handle)
{
  sGeoHandle *gh = &GeoHandle[handle];
  sInt update = 0;

  if(gh->Vertex.Count==0)
    update |= sGEO_VERTEX|sGEO_INDEX;
  if(gh->Vertex.Buffer==0 && gh->Vertex.DiscardCount!=DiscardCount[0])
    update |= sGEO_VERTEX;
  if(gh->Index.Buffer==1 && gh->Index.DiscardCount!=DiscardCount[1])
    update |= sGEO_INDEX;
  if(gh->Index.Buffer==2 && gh->Index.DiscardCount!=DiscardCount[2])
    update |= sGEO_INDEX;
  if(update)
  {
    gStage[handle][0].Active = sFALSE;
    gStage[handle][1].Active = sFALSE;
    return update;
  }
  flushStage(gStage[handle][0]);
  flushStage(gStage[handle][1]);

  if(gTrace)
  {
    fprintf(stderr,"  draw h%d fvf%d mode%d v%d/%d i%d setup%d tex",handle,gh->FVF,gh->Mode&7,
      gh->Vertex.Count,gh->Vertex.Buffer,gh->Index.Count,CurrentSetupId);
    for(sInt i=0;i<4;i++) fprintf(stderr," %d",gStageTex[i]);
    fprintf(stderr," vp%dx%d blend%d z%d/%d st%d cw%d\n",ViewportX,ViewportY,gRS[sD3DRS_ALPHABLENDENABLE],
      gRS[sD3DRS_ZFUNC],gRS[sD3DRS_ZWRITEENABLE],gRS[sD3DRS_STENCILENABLE],gRS[sD3DRS_COLORWRITEENABLE]);
    if(getenv("KK_DUMPTEX") && gStageTex[0]>=0 && Textures[gStageTex[0]].Tex)
    {
      char name[64];
      IDirect3DTexture9 *t = Textures[gStageTex[0]].Tex;
      snprintf(name,sizeof(name),"/tmp/kktex_%d.bmp",gStageTex[0]);
      dumpTexture(t->Tex,t->XS,t->YS,name);
    }
  }

  IDirect3DVertexShader9 *vs = CurrentVS;
  IDirect3DPixelShader9 *ps = CurrentPS;
  if(!vs || !vs->Obj)
    return sFALSE;

  sU32 proj;
  sU32 used = ps ? ps->Info.SamplersUsed : 3;
  sU32 kinds = bindStageTextures(used,proj);
  GLProg *prog = getProgram(vs,ps,kinds,proj);
  glUseProgram(prog->Prog);
  setProgramUniforms(prog);
  applyRenderStates();

  glBindVertexArray(gVAO);
  glBindBuffer(GL_ARRAY_BUFFER,gGeoGL[gh->Vertex.Buffer]);
  bindFVF(gh->FVF);

  sInt count = gh->Index.Count ? gh->Index.Count : gh->Vertex.Count;
  GLenum mode = GL_TRIANGLES;
  switch(gh->Mode&7)
  {
  case sGEO_POINT:     mode = GL_POINTS; break;
  case sGEO_LINE:      mode = GL_LINES; count = count/2*2; break;
  case sGEO_LINESTRIP: mode = GL_LINE_STRIP; break;
  case sGEO_TRI:       mode = GL_TRIANGLES; count = count/3*3; break;
  case sGEO_TRISTRIP:  mode = GL_TRIANGLE_STRIP; break;
  case sGEO_QUAD:
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gGeoGL[3]);
    glDrawElementsBaseVertex(GL_TRIANGLES,gh->Vertex.Count/4*6,GL_UNSIGNED_SHORT,0,gh->Vertex.Start);
    return sFALSE;
  default:
    return sFALSE;
  }

  if(gh->Index.Count)
  {
    sBool is32b = (gh->Mode & sGEO_IND32B) ? sTRUE : sFALSE;
    sInt isz = is32b ? 4 : 2;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gGeoGL[gh->Index.Buffer]);
    glDrawElementsBaseVertex(mode,count,is32b ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
      (const void *)(sDInt)(gh->Index.Start*isz),gh->Vertex.Start);
  }
  else if(count)
    glDrawArrays(mode,gh->Vertex.Start,count);

  return sFALSE;
}

/****************************************************************************/
/***   materials                                                          ***/
/****************************************************************************/

void sSystem_::SetViewProject(const sMaterialEnv *env)
{
  env->MakeProjectionMatrix(LastProjection);
  LastCamera = env->CameraSpace;
  LastCamera.TransR();
  LastViewProject.Mul4(LastCamera,LastProjection);
}

sInt sSystem_::MtrlAddSetup(const sU32 *states,const sU32 *vs,const sU32 *ps)
{
  sInt stateLen;
  for(stateLen=0;states[stateLen]!=~0U;stateLen+=2);
  stateLen += 2;

  sInt slot = -1;
  for(sInt i=0;i<MAX_SETUPS;i++)
    if(!Setups[i].RefCount) { slot = i; break; }
  if(slot<0)
  {
    fprintf(stderr,"out of material setup slots!\n");
    return sINVALID;
  }

  IDirect3DVertexShader9 *vso = 0;
  IDirect3DPixelShader9 *pso = 0;
  if(vs)
  {
    vso = new IDirect3DVertexShader9;
    if(!D3DTranslate(vs,vso->Info))
    {
      fprintf(stderr,"vertex shader translation failed\n");
      delete vso;
      return sINVALID;
    }
    vso->Obj = compileShader(GL_VERTEX_SHADER,D3DBuildVS(vso->Info));
  }
  if(ps)
  {
    pso = new IDirect3DPixelShader9;
    if(!D3DTranslate(ps,pso->Info))
    {
      fprintf(stderr,"pixel shader translation failed\n");
      delete pso;
      sRelease(vso);
      return sINVALID;
    }
  }

  sMaterialSetup *setup = &Setups[slot];
  setup->RefCount = 1;
  setup->States = new sU32[stateLen];
  sCopyMem4(setup->States,states,stateLen);
  setup->VSCode = 0;
  setup->PSCode = 0;
  setup->VS = vso;
  setup->PS = pso;
  return slot;
}

void sSystem_::MtrlAddRefSetup(sInt sid){ Setups[sid].AddRef(); }
void sSystem_::MtrlRemSetup(sInt sid){ Setups[sid].Release(); }

void sSystem_::MtrlClearCaches()
{
  CurrentSetupId = sINVALID;
  CurrentVS = 0;
  CurrentPS = 0;
  for(sInt i=0;i<MAX_TEXSTAGE;i++)
    CurrentTextures[i] = sINVALID;
  for(sInt i=0;i<0x310;i++)
    CurrentStates[i] = 0xfefefefe;
}

void sSystem_::MtrlSetSetup(sInt sid)
{
  if(sid==CurrentSetupId)
    return;
  sMaterialSetup *setup = &Setups[sid];
  setup->AddRef();
  if(CurrentSetupId!=sINVALID)
    Setups[CurrentSetupId].Release();

  for(sU32 *st=setup->States;st[0]!=~0U;st+=2)
    SetState(st[0],st[1]);

  CurrentVS = setup->VS;
  CurrentPS = setup->PS;
  CurrentSetupId = sid;
}

void sSystem_::MtrlSetInstance(const sMaterialInstance &inst)
{
  for(sInt i=0;i<inst.NumTextures && i<MAX_TEXSTAGE;i++)
  {
    gStageTex[i] = inst.Textures[i];
    CurrentTextures[i] = inst.Textures[i];
  }
  if(inst.NumVSConstants)
  {
    sInt n = sMin(inst.NumVSConstants,256);
    sCopyMem(gVSConst,inst.VSConstants,n*16);
    gVSConstUsed = sMax(gVSConstUsed,n);
  }
  if(inst.NumPSConstants)
    sCopyMem(gPSConst,inst.PSConstants,sMin(inst.NumPSConstants,32)*16);
}

void sSystem_::SetStates(sU32 *stream)
{
  while(*stream!=0xffffffff)
  {
    SetState(stream[0],stream[1]);
    stream += 2;
  }
}

void sSystem_::SetState(sU32 token,sU32 value)
{
  if(token<0x310)
  {
    gRS[token] = value;
    CurrentStates[token] = value;
  }
}

void sSystem_::InitScreens()
{
  if(!gScreenTex) return;
  if(Screen[0].XSize==ConfigX && Screen[0].YSize==ConfigY) return;

  Screen[0].XSize = ConfigX;
  Screen[0].YSize = ConfigY;
  glBindTexture(GL_TEXTURE_2D,gScreenTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,ConfigX,ConfigY,0,GL_BGRA,GL_UNSIGNED_BYTE,0);
  ensureDepth(sMax(ConfigX,1024),sMax(ConfigY,512));
  if(gWin)
  {
    SDL_SetWindowSize(gWin,ConfigX,ConfigY);
    SDL_SetWindowPosition(gWin,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);
  }
  attachTarget(gScreenTex);
}
void sSystem_::ExitScreens(){}
void sSystem_::ReCreateZBuffer(){}
void sSystem_::Render(){}
sInt sSystem_::Msg(sU32,sU32,sU32){ return 0; }

/****************************************************************************/
/***   main loop                                                          ***/
/****************************************************************************/

// KK_SHOT=frame[,frame...] writes /tmp/kk_<frame>.bmp; KK_QUIT=frame exits
static void debugShot(sInt frame)
{
  static const char *shots = getenv("KK_SHOT");
  if(!shots) return;
  char key[16];
  snprintf(key,sizeof(key),"%d",frame);
  const char *p = shots;
  sBool hit = sFALSE;
  while(*p)
  {
    sInt n = (sInt)strtol(p,(char **)&p,10);
    if(n==frame) hit = sTRUE;
    while(*p==',') p++;
    if(!n && *p && (*p<'0' || *p>'9')) break;
  }
  if(!hit) return;
  char name[64];
  snprintf(name,sizeof(name),"/tmp/kk_%d.bmp",frame);
  dumpTexture(gScreenTex,sSystem->Screen[0].XSize,sSystem->Screen[0].YSize,name);
}

static void dumpTexture(GLuint tex,sInt w,sInt h,const char *name)
{
  std::vector<sU8> px(w*h*4);
  glBindFramebuffer(GL_READ_FRAMEBUFFER,gBlitFBO[0]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex,0);
  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glReadPixels(0,0,w,h,GL_BGRA,GL_UNSIGNED_BYTE,px.data());
  glBindFramebuffer(GL_READ_FRAMEBUFFER,gFBO);
  FILE *f = fopen(name,"wb");
  if(!f) return;
  sU8 hdr[54] = { 'B','M' };
  sU32 size = 54+w*h*4;
  memcpy(hdr+2,&size,4);
  sU32 v = 54; memcpy(hdr+10,&v,4);
  v = 40; memcpy(hdr+14,&v,4);
  memcpy(hdr+18,&w,4);
  sInt nh = -h; memcpy(hdr+22,&nh,4);
  hdr[26] = 1; hdr[28] = 32;
  fwrite(hdr,1,54,f);
  fwrite(px.data(),1,px.size(),f);
  fclose(f);
  fprintf(stderr,"wrote %s\n",name);
}

static void presentScreen()
{
  sInt frame = ++gFrameNo;
  debugShot(frame);
  if(getenv("KK_QUIT") && frame>=atoi(getenv("KK_QUIT"))) gQuit = 1;
  // KK_KEYS=frame:code[,frame:^code...] queues a key press (^ = release); code is a char
  // or a decimal sKEY_ value with at least two digits
  if(const char *keys = getenv("KK_KEYS"))
  {
    const char *p = keys;
    while(*p)
    {
      sInt f = (sInt)strtol(p,(char **)&p,10);
      if(*p!=':') break;
      p++;
      sU32 brk = 0;
      if(*p=='^') { brk = sKEYQ_BREAK; p++; }
      sU32 code;
      if(*p>='0' && *p<='9' && p[1]>='0' && p[1]<='9') code = (sU32)strtoul(p,(char **)&p,10);
      else code = (sU8)*p++;
      if(f==frame && sSystem->KeyIndex<MAX_KEYBUFFER-1)
        sSystem->KeyBuffer[sSystem->KeyIndex++] = code|brk;
      while(*p==',') p++;
    }
  }
  gTrace = getenv("KK_TRACE") && atoi(getenv("KK_TRACE"))==frame+1;

  int dw = sSystem->ConfigX, dh = sSystem->ConfigY;
  if(gWin) SDL_GL_GetDrawableSize(gWin,&dw,&dh);
  glBindFramebuffer(GL_READ_FRAMEBUFFER,gBlitFBO[0]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,gScreenTex,0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
  glDisable(GL_SCISSOR_TEST);
  glColorMask(1,1,1,1);
  // the screen texture is stored top-down, the window bottom-up
  glBlitFramebuffer(0,0,sSystem->Screen[0].XSize,sSystem->Screen[0].YSize,0,dh,dw,0,
    GL_COLOR_BUFFER_BIT,(dw==sSystem->Screen[0].XSize && dh==sSystem->Screen[0].YSize) ? GL_NEAREST : GL_LINEAR);
  SDL_GL_SwapWindow(gWin);
  glBindFramebuffer(GL_FRAMEBUFFER,gFBO);
}

static void drawLoading(sInt done, sInt max)
{
  if(!gWin || !gLoadProg) return;
  float t = (max>0) ? (float)done / (float)max : 0.f;
  if(t<0) t=0;
  if(t>1) t=1;

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT,vp);
  int dw = sSystem->ConfigX, dh = sSystem->ConfigY;
  SDL_GL_GetDrawableSize(gWin,&dw,&dh);

  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glViewport(0,0,dw,dh);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glColorMask(1,1,1,1);
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gLoadProg);
  glBindVertexArray(gVAO);
  for(sInt i=0;i<D3DSLOT_MAX;i++) glDisableVertexAttribArray(i);
  glBindBuffer(GL_ARRAY_BUFFER,gLoadVBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,8,0);

  auto bar = [&](float x0,float y0,float x1,float y1,float r,float g,float b)
  {
    float v[] = { x0,y0, x1,y0, x0,y1, x1,y1 };
    glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STREAM_DRAW);
    glUniform4f(gLoadColorLoc,r,g,b,1);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  };
  bar(-0.6f,-0.03f,0.6f,0.03f,0.15f,0.15f,0.15f);
  bar(-0.6f,-0.03f,-0.6f+1.2f*t,0.03f,0.8f,0.8f,0.8f);
  SDL_GL_SwapWindow(gWin);

  glBindFramebuffer(GL_FRAMEBUFFER,gFBO);
  glViewport(vp[0],vp[1],vp[2],vp[3]);
}

static void pump()
{
  SDL_Event e;
  while(SDL_PollEvent(&e)){
    if(e.type==SDL_QUIT) gQuit=1;
    if(e.type==SDL_KEYDOWN || e.type==SDL_KEYUP){
      sU32 k=sdlKey(e.key.keysym.sym, e.type==SDL_KEYUP);
      if(k && sSystem->KeyIndex<MAX_KEYBUFFER-1) sSystem->KeyBuffer[sSystem->KeyIndex++]=k;
    }
    if(e.type==SDL_MOUSEMOTION){ gMouseX+=e.motion.xrel; gMouseY+=e.motion.yrel; }
    if(e.type==SDL_MOUSEBUTTONDOWN || e.type==SDL_MOUSEBUTTONUP){
      sU32 k=sKEY_MOUSEL;
      if(e.button.button==SDL_BUTTON_RIGHT) k=sKEY_MOUSER;
      if(e.type==SDL_MOUSEBUTTONUP) k|=sKEYQ_BREAK;
      else gMouseBtns|=1;
      if(e.type==SDL_MOUSEBUTTONUP) gMouseBtns&=~1;
      if(sSystem->KeyIndex<MAX_KEYBUFFER-1) sSystem->KeyBuffer[sSystem->KeyIndex++]=k;
    }
  }
}

static void initGL(sSystem_ *sys)
{
  glGenVertexArrays(1,&gVAO);
  glBindVertexArray(gVAO);
  for(sInt i=0;i<D3DSLOT_MAX;i++)
    glVertexAttrib4f(i,0,0,0,1);

  glGenFramebuffers(1,&gFBO);
  glGenFramebuffers(2,gBlitFBO);
  glGenSamplers(8,gSamplers);

  glGenTextures(1,&gScreenTex);
  glBindTexture(GL_TEXTURE_2D,gScreenTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,sys->Screen[0].XSize,sys->Screen[0].YSize,0,GL_BGRA,GL_UNSIGNED_BYTE,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  attachTarget(gScreenTex);
  ensureDepth(sys->Screen[0].XSize,sys->Screen[0].YSize);
  GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(st!=GL_FRAMEBUFFER_COMPLETE) fprintf(stderr,"framebuffer incomplete: %x\n",st);

  glGenTextures(1,&gWhiteTex);
  sU8 w[4] = { 255,255,255,255 };
  glBindTexture(GL_TEXTURE_2D,gWhiteTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,w);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);

  glFrontFace(GL_CCW);

  static const char *lvs =
    "#version 150\nin vec2 pos;\nvoid main(){ gl_Position=vec4(pos,0.0,1.0); }\n";
  static const char *lps =
    "#version 150\nuniform vec4 color;\nout vec4 frag;\nvoid main(){ frag=color; }\n";
  gLoadProg = glCreateProgram();
  GLuint a = compileShader(GL_VERTEX_SHADER,lvs), b = compileShader(GL_FRAGMENT_SHADER,lps);
  glAttachShader(gLoadProg,a);
  glAttachShader(gLoadProg,b);
  glBindAttribLocation(gLoadProg,0,"pos");
  glBindFragDataLocation(gLoadProg,0,"frag");
  glLinkProgram(gLoadProg);
  gLoadColorLoc = glGetUniformLocation(gLoadProg,"color");
  glGenBuffers(1,&gLoadVBO);

  // buffer management, like the original InitScreens()
  sys->CreateGeoBuffer(0,1,0,MAX_DYNVBSIZE);
  sys->CreateGeoBuffer(1,1,1,MAX_DYNIBSIZE);
  sys->CreateGeoBuffer(2,1,2,MAX_DYNIBSIZE*2);
  sys->CreateGeoBuffer(3,0,1,2*0x8000/4*6);
  {
    std::vector<sU16> quads(0x8000/4*6,0);
    sU16 *ip = quads.data();
    for(sInt i=0;i<0x8000/6;i++)
      sQuad(ip,i*4+0,i*4+1,i*4+2,i*4+3);
    glBindBuffer(GL_ARRAY_BUFFER,gGeoGL[3]);
    glBufferSubData(GL_ARRAY_BUFFER,0,quads.size()*2,quads.data());
  }

  sys->MakeCubeNormalizer();
  sys->MakeAttenuationVolume();

  // the engine uses handle 0 as a placeholder (SetTex(i,0)) and releases it
  // again later, so slot 0 must stay allocated for the whole run.
  sInt reserved = allocTexture(sys,1,1,sTF_A8R8G8B8,sTRUE);
  sys->Textures[reserved].RefCount = 0x40000000;

  for(sInt i=0;i<MAX_TEXSTAGE;i++)
    gStageTex[i] = sINVALID;
  initDefaultStates();
}

void sSystem_::InitX()
{
  ConfigX = ConfigX?ConfigX:1280;
  ConfigY = ConfigY?ConfigY:720;
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,0);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,0);
  gWin=SDL_CreateWindow(".kkrieger",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,ConfigX,ConfigY,SDL_WINDOW_OPENGL);
  if(!gWin){ fprintf(stderr,"SDL_CreateWindow: %s\n", SDL_GetError()); exit(1); }
  gGL=SDL_GL_CreateContext(gWin);
  if(!gGL){ fprintf(stderr,"SDL_GL_CreateContext: %s\n", SDL_GetError()); exit(1); }
  SDL_GL_SetSwapInterval(1);
  SDL_SetRelativeMouseMode(SDL_TRUE);
  fprintf(stderr,"gl %s\n", (const char*)glGetString(GL_VERSION));

  Screen[0].XSize=ConfigX; Screen[0].YSize=ConfigY; WScreenCount=1;
  GpuMask=sGPU_TWOSIDESTENCIL; CpuMask=sCPU_SSE2;
  gStartMs=SDL_GetTicks();
  MtrlClearCaches();
  initGL(this);

  SDL_AudioSpec want={}, got{};
  want.freq=44100; want.format=AUDIO_S16SYS; want.channels=2; want.samples=1024; want.callback=audioCB;
  SDL_OpenAudio(&want,&got); SDL_PauseAudio(0);

  sAppHandler(sAPPCODE_INIT,0);
  KeyIndex=0;
  fprintf(stderr,"ready - click the window, Esc quits.\n");
  fflush(stderr);

  while(!gQuit)
  {
    pump();
    for(sInt i=0;i<KeyIndex;i++) sAppHandler(sAPPCODE_KEY,KeyBuffer[i]);
    KeyIndex=0;

    LastCamera.Init();
    LastMatrix.Init();
    LastViewProject.Init();

    sViewport vp;
    vp.Init();
    SetViewport(vp);
    Clear(sVCF_ALL,0xff000000);

    sAppHandler(sAPPCODE_PAINT,0);
    presentScreen();
  }
  sAppHandler(sAPPCODE_EXIT,0);
  SDL_Quit();
}

/****************************************************************************/

sSimpleMaterial::sSimpleMaterial(sInt tex,sU32 flags,sU32 flags2,sU32 color)
{
  sU32 states[64],*st = states;
  static const sU32 vShaderTex[] =
  {
    0xfffe0101,                                         // vs.1.1
    0x0000001f, 0x80000000, 0x900f0000,                 // dcl_position v0
    0x0000001f, 0x80000005, 0x900f0001,                 // dcl_texcoord v1
    0x0000001f, 0x8000000a, 0x900f0002,                 // dcl_color v2
    0x00000014, 0xc00f0000, 0x90e40000, 0xa0e40000,     // m4x4 oPos,v0,c0
    0x00000001, 0xe00f0000, 0x90e40001,                 // mov oT0,v1
    0x00000001, 0xd00f0000, 0x90e40002,                 // mov oD0,v2
    0x0000ffff,                                         // end
  };
  static const sU32 vShaderNoTex[] =
  {
    0xfffe0101,                                         // vs.1.1
    0x0000001f, 0x80000000, 0x900f0000,                 // dcl_position v0
    0x0000001f, 0x8000000a, 0x900f0001,                 // dcl_color v1
    0x00000014, 0xc00f0000, 0x90e40000, 0xa0e40000,     // m4x4 oPos,v0,c0
    0x00000001, 0xd00f0000, 0x90e40001,                 // mov oD0,v1
    0x0000ffff,                                         // end
  };

  sF32 zBias = 0.0f;
  if(flags & sMBF_ZBIASBACK) zBias =  1.0f / 65536.0f;
  if(flags & sMBF_ZBIASFORE) zBias = -1.0f / 65536.0f;

  *st++ = sD3DRS_ALPHATESTENABLE;     *st++ = 0;
  *st++ = sD3DRS_ZENABLE;             *st++ = sD3DZB_TRUE;
  *st++ = sD3DRS_ZWRITEENABLE;        *st++ = (flags & sMBF_ZWRITE) ? 1 : 0;
  *st++ = sD3DRS_ZFUNC;               *st++ = (flags & sMBF_ZREAD) ? sD3DCMP_LESS : sD3DCMP_ALWAYS;
  *st++ = sD3DRS_CULLMODE;            *st++ = (flags & sMBF_DOUBLESIDED) ? sD3DCULL_NONE : (flags & sMBF_INVERTCULL ? sD3DCULL_CW : sD3DCULL_CCW);
  *st++ = sD3DRS_COLORWRITEENABLE;    *st++ = 15;
  *st++ = sD3DRS_SLOPESCALEDEPTHBIAS; *st++ = *(sU32 *) &zBias;
  *st++ = sD3DRS_DEPTHBIAS;           *st++ = *(sU32 *) &zBias;
  *st++ = sD3DRS_FOGENABLE;           *st++ = 0;
  *st++ = sD3DRS_STENCILENABLE;       *st++ = 0;

  switch(flags & sMBF_BLENDMASK)
  {
  case sMBF_BLENDADD:
    *st++ = sD3DRS_ALPHABLENDENABLE;  *st++ = 1;
    *st++ = sD3DRS_SRCBLEND;          *st++ = sD3DBLEND_ONE;
    *st++ = sD3DRS_DESTBLEND;         *st++ = sD3DBLEND_ONE;
    *st++ = sD3DRS_BLENDOP;           *st++ = sD3DBLENDOP_ADD;
    break;
  case sMBF_BLENDALPHA:
    *st++ = sD3DRS_ALPHABLENDENABLE;  *st++ = 1;
    *st++ = sD3DRS_SRCBLEND;          *st++ = sD3DBLEND_SRCALPHA;
    *st++ = sD3DRS_DESTBLEND;         *st++ = sD3DBLEND_INVSRCALPHA;
    *st++ = sD3DRS_BLENDOP;           *st++ = sD3DBLENDOP_ADD;
    break;
  default:
    *st++ = sD3DRS_ALPHABLENDENABLE;  *st++ = 0;
    break;
  }

  *st++ = sD3DTSS_0|sD3DTSS_TEXCOORDINDEX;          *st++ = 0;
  *st++ = sD3DTSS_0|sD3DTSS_TEXTURETRANSFORMFLAGS;  *st++ = 0;

  sU32 colorSource = (flags2 & 1) ? sD3DTA_TFACTOR : sD3DTA_DIFFUSE;
  if(tex != sINVALID)
  {
    *st++ = sD3DTSS_0|sD3DTSS_COLORARG1;            *st++ = sD3DTA_TEXTURE;
    *st++ = sD3DTSS_0|sD3DTSS_COLORARG2;            *st++ = colorSource;
    *st++ = sD3DTSS_0|sD3DTSS_COLOROP;              *st++ = sD3DTOP_MODULATE;
    *st++ = sD3DTSS_0|sD3DTSS_ALPHAARG1;            *st++ = sD3DTA_TEXTURE;
    *st++ = sD3DTSS_0|sD3DTSS_ALPHAOP;              *st++ = sD3DTOP_SELECTARG1;
  }
  else
  {
    *st++ = sD3DTSS_0|sD3DTSS_COLORARG2;            *st++ = colorSource;
    *st++ = sD3DTSS_0|sD3DTSS_COLOROP;              *st++ = sD3DTOP_SELECTARG2;
    *st++ = sD3DTSS_0|sD3DTSS_ALPHAARG2;            *st++ = colorSource;
    *st++ = sD3DTSS_0|sD3DTSS_ALPHAOP;              *st++ = sD3DTOP_SELECTARG2;
  }
  *st++ = sD3DTSS_1|sD3DTSS_COLOROP;                *st++ = sD3DTOP_DISABLE;
  *st++ = sD3DTSS_1|sD3DTSS_ALPHAOP;                *st++ = sD3DTOP_DISABLE;

  sU32 addressMode = (flags2 & 2) ? sD3DTADDRESS_CLAMP : sD3DTADDRESS_WRAP;
  *st++ = sD3DSAMP_0|sD3DSAMP_MAGFILTER;            *st++ = sD3DTEXF_LINEAR;
  *st++ = sD3DSAMP_0|sD3DSAMP_MINFILTER;            *st++ = sD3DTEXF_LINEAR;
  *st++ = sD3DSAMP_0|sD3DSAMP_MIPFILTER;            *st++ = (flags2 & 4) ? sD3DTEXF_NONE : sD3DTEXF_LINEAR;
  *st++ = sD3DSAMP_0|sD3DSAMP_ADDRESSU;             *st++ = addressMode;
  *st++ = sD3DSAMP_0|sD3DSAMP_ADDRESSV;             *st++ = addressMode;

  *st++ = ~0U;                                      *st++ = ~0U;

  Color = color;
  Tex = sINVALID;
  SetTex(tex);
  Setup = sSystem->MtrlAddSetup(states,(tex == sINVALID) ? vShaderNoTex : vShaderTex,0);
}

sSimpleMaterial::~sSimpleMaterial()
{
  SetTex(sINVALID);
  sSystem->MtrlRemSetup(Setup);
}

void sSimpleMaterial::SetTex(sInt tex)
{
  if(tex != sINVALID)
    sSystem->AddRefTexture(tex);
  if(Tex != sINVALID)
    sSystem->RemTexture(Tex);
  Tex = tex;
}

void sSimpleMaterial::Set(const sMaterialEnv &env)
{
  sMaterialInstance inst;
  sMatrix mat;

  inst.NumTextures = 1;
  inst.Textures[0] = Tex;
  inst.NumVSConstants = 4;
  inst.VSConstants = &mat.i;
  inst.NumPSConstants = 0;

  mat.Mul4(env.ModelSpace,sSystem->LastViewProject);
  mat.Trans4();

  sSystem->MtrlSetSetup(Setup);
  sSystem->MtrlSetInstance(inst);
  sSystem->SetState(sD3DRS_TEXTUREFACTOR,Color);
}

int main(int argc, char **argv)
{
  if(argc>1) snprintf(gCmdLine,sizeof(gCmdLine),"%s",argv[1]);
  else {
    const char *cands[]={"data/kkrieger.kx","./kkrieger.kx","../data/kkrieger.kx",0};
    gCmdLine[0]=0;
    for(sInt i=0;cands[i];i++) if(access(cands[i],R_OK)==0){ snprintf(gCmdLine,sizeof(gCmdLine),"%s",cands[i]); break; }
    if(!gCmdLine[0]) snprintf(gCmdLine,sizeof(gCmdLine),"data/kkrieger.kx");
  }
  sBroker=new sBroker_;
  sSystem=new sSystem_;
  sSetMem(sSystem,0,sizeof(sSystem_));
  sSystem->ConfigX=1280; sSystem->ConfigY=720; sSystem->ConfigFlags=sSF_DIRECT3D;
  sAppHandler(sAPPCODE_CONFIG,0);
  sSystem->InitX();
  return 0;
}
