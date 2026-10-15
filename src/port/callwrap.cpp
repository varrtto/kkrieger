// Auto-generated ABI wrappers for macOS/ARM64
#include <string.h>
#include "_types.hpp"
#include "kdoc.hpp"
#include "genbitmap.hpp"
#include "genmaterial.hpp"
#include "genmesh.hpp"
#include "genoverlay.hpp"
#include "genscene.hpp"
#include "geneffect.hpp"
#include "kkriegergame.hpp"
static inline sF32 sBitFloat(sDInt v){ sU32 u=(sU32)v; sF32 f; memcpy(&f,&u,4); return f; }
static inline sF322 sBitF322(sDInt *a){ sF322 r; r.x=sBitFloat(a[0]); r.y=sBitFloat(a[1]); return r; }
static inline sF323 sBitF323(sDInt *a){ sF323 r; r.x=sBitFloat(a[0]); r.y=sBitFloat(a[1]); r.z=sBitFloat(a[2]); return r; }
static inline sF324 sBitF324(sDInt *a){ sF324 r; r.x=sBitFloat(a[0]); r.y=sBitFloat(a[1]); r.z=sBitFloat(a[2]); r.w=sBitFloat(a[3]); return r; }
static inline sInt2 sBitInt2(sDInt *a){ sInt2 r; r.x=(sInt)a[0]; r.y=(sInt)a[1]; return r; }
static inline sFSRT sBitFSRT(sDInt *a){ sFSRT r; for(int i=0;i<9;i++) r.v[i]=sBitFloat(a[i]); return r; }
static inline void *AsPtr(sDInt v){ return ((unsigned long long)v>0x10000ull)?(void*)v:0; }
sDInt W_Bitmap_Blur(sDInt *a) { return (sDInt)Bitmap_Blur(((GenBitmap *)AsPtr(a[0])), ((sInt)a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4])); }
sDInt W_Bitmap_Bump(sDInt *a) { return (sDInt)Bitmap_Bump(((GenBitmap *)AsPtr(a[0])), ((GenBitmap *)AsPtr(a[1])), ((sInt)a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), (sF32)sBitFloat(a[7]), ((sU32)a[8]), ((sU32)a[9]), (sF32)sBitFloat(a[10]), (sF32)sBitFloat(a[11]), (sF32)sBitFloat(a[12]), ((sU32)a[13]), (sF32)sBitFloat(a[14]), (sF32)sBitFloat(a[15])); }
sDInt W_Bitmap_Cell(sDInt *a) { return (sDInt)Bitmap_Cell(((sInt)a[0]), ((sInt)a[1]), ((sU32)a[2]), ((sU32)a[3]), ((sU32)a[4]), ((sInt)a[5]), ((sInt)a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8]), ((sInt)a[9]), (sF32)sBitFloat(a[10]), ((sInt)a[11]), (sF32)sBitFloat(a[12])); }
sDInt W_Bitmap_Color(sDInt *a) { return (sDInt)Bitmap_Color(((GenBitmap *)AsPtr(a[0])), ((sInt)a[1]), ((sU32)a[2])); }
sDInt W_Bitmap_ColorBalance(sDInt *a) { return (sDInt)Bitmap_ColorBalance(((GenBitmap *)AsPtr(a[0])), sBitF323(a+1), sBitF323(a+4), sBitF323(a+7)); }
sDInt W_Bitmap_Distort(sDInt *a) { return (sDInt)Bitmap_Distort(((GenBitmap *)AsPtr(a[0])), ((GenBitmap *)AsPtr(a[1])), (sF32)sBitFloat(a[2]), ((sInt)a[3])); }
sDInt W_Bitmap_Dots(sDInt *a) { return (sDInt)Bitmap_Dots(((GenBitmap *)AsPtr(a[0])), ((sU32)a[1]), ((sU32)a[2]), ((sInt)a[3]), ((sInt)a[4])); }
sDInt W_Bitmap_Flat(sDInt *a) { return (sDInt)Bitmap_Flat(((sInt)a[0]), ((sInt)a[1]), ((sU32)a[2])); }
sDInt W_Bitmap_Format(sDInt *a) { return (sDInt)Bitmap_Format(((GenBitmap *)AsPtr(a[0])), ((sInt)a[1]), ((sInt)a[2]), ((sInt)a[3])); }
sDInt W_Bitmap_GlowRect(sDInt *a) { return (sDInt)Bitmap_GlowRect(((GenBitmap *)AsPtr(a[0])), (sF32)sBitFloat(a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), ((sU32)a[7]), (sF32)sBitFloat(a[8]), (sF32)sBitFloat(a[9]), ((sU32)a[10]), ((sU32)a[11])); }
sDInt W_Bitmap_Gradient(sDInt *a) { return (sDInt)Bitmap_Gradient(((sInt)a[0]), ((sInt)a[1]), ((sU32)a[2]), ((sU32)a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), ((sInt)a[7])); }
sDInt W_Bitmap_HSCB(sDInt *a) { return (sDInt)Bitmap_HSCB(((GenBitmap *)AsPtr(a[0])), (sF32)sBitFloat(a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4])); }
sDInt W_Bitmap_Light(sDInt *a) { return (sDInt)Bitmap_Light(((GenBitmap *)AsPtr(a[0])), ((sInt)a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), ((sU32)a[7]), ((sU32)a[8]), (sF32)sBitFloat(a[9]), (sF32)sBitFloat(a[10]), (sF32)sBitFloat(a[11])); }
sDInt W_Bitmap_Mask(sDInt *a) { return (sDInt)Bitmap_Mask(((GenBitmap *)AsPtr(a[0])), ((GenBitmap *)AsPtr(a[1])), ((GenBitmap *)AsPtr(a[2])), ((sInt)a[3])); }
sDInt W_Bitmap_Merge(sDInt *a) {
  sInt mode=(sInt)a[0]; sInt count=(sInt)a[1];
  GenBitmap *b0=((GenBitmap *)AsPtr(a[2]));
  if(count<=1) return (sDInt)Bitmap_Merge(mode,count,b0);
  // Bitmap_Merge() takes at most 32 inputs
  GenBitmap *in[32];
  for(sInt i=1;i<32;i++)
    in[i] = i<count ? (GenBitmap *)AsPtr(a[2+i]) : 0;
  return (sDInt)Bitmap_Merge(mode,count,b0,
    in[1],in[2],in[3],in[4],in[5],in[6],in[7],in[8],in[9],in[10],in[11],in[12],in[13],in[14],in[15],
    in[16],in[17],in[18],in[19],in[20],in[21],in[22],in[23],in[24],in[25],in[26],in[27],in[28],in[29],
    in[30],in[31],(GenBitmap*)0);
}
sDInt W_Bitmap_Normals(sDInt *a) { return (sDInt)Bitmap_Normals(((GenBitmap *)AsPtr(a[0])), (sF32)sBitFloat(a[1]), ((sInt)a[2])); }
sDInt W_Bitmap_Perlin(sDInt *a) { return (sDInt)Bitmap_Perlin(((sInt)a[0]), ((sInt)a[1]), ((sInt)a[2]), ((sInt)a[3]), (sF32)sBitFloat(a[4]), ((sInt)a[5]), ((sInt)a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8]), ((sU32)a[9]), ((sU32)a[10])); }
sDInt W_Bitmap_Range(sDInt *a) { return (sDInt)Bitmap_Range(((GenBitmap *)AsPtr(a[0])), ((sInt)a[1]), ((sU32)a[2]), ((sU32)a[3])); }
sDInt W_Bitmap_RenderTarget(sDInt *a) { return (sDInt)Bitmap_RenderTarget(((sInt)a[0]), ((sInt)a[1]), ((sU32)a[2])); }
sDInt W_Bitmap_Rotate(sDInt *a) { return (sDInt)Bitmap_Rotate(((GenBitmap *)AsPtr(a[0])), (sF32)sBitFloat(a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), ((sInt)a[6]), ((sInt)a[7]), ((sInt)a[8])); }
sDInt W_Bitmap_RotateMul(sDInt *a) { return (sDInt)Bitmap_RotateMul(((GenBitmap *)AsPtr(a[0])), (sF32)sBitFloat(a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), ((sInt)a[6]), ((sU32)a[7]), ((sInt)a[8]), ((sInt)a[9]), ((sU32)a[10])); }
sDInt W_Bitmap_Sharpen(sDInt *a) { return (sDInt)Bitmap_Sharpen(((GenBitmap *)AsPtr(a[0])), ((sInt)a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4])); }
sDInt W_Bitmap_Text(sDInt *a) { return (sDInt)Bitmap_Text(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((GenBitmap *)AsPtr(a[2])), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), ((sU32)a[7]), ((sU32)a[8]), ((sInt)a[9]), (sF32)sBitFloat(a[10]), (sF32)sBitFloat(a[11]), ((sChar *)a[12]), ((sChar *)a[13])); }
sDInt W_Bitmap_Twirl(sDInt *a) { return (sDInt)Bitmap_Twirl(((GenBitmap *)AsPtr(a[0])), (sF32)sBitFloat(a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), ((sInt)a[7])); }
sDInt W_Exec_Effect_PartEmitter(sDInt *a) { Exec_Effect_PartEmitter(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), sBitF324(a+7), sBitF324(a+11), sBitF324(a+15)); return 0; }
sDInt W_Exec_Effect_PartSystem(sDInt *a) { Exec_Effect_PartSystem(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sU32)a[4]), ((sU32)a[5]), ((sU32)a[6]), sBitF323(a+7), (sF32)sBitFloat(a[10]), (sF32)sBitFloat(a[11]), (sF32)sBitFloat(a[12]), ((sInt)a[13])); return 0; }
sDInt W_Exec_Effect_Print(sDInt *a) { Exec_Effect_Print(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), (sF32)sBitFloat(a[7]), ((sChar *)a[8])); return 0; }
sDInt W_Exec_IPP_Blur(sDInt *a) { Exec_IPP_Blur(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), ((sInt)a[5]), ((sInt)a[6]), ((sInt)a[7])); return 0; }
sDInt W_Exec_IPP_Color(sDInt *a) { Exec_IPP_Color(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sU32)a[3]), ((sInt)a[4]), ((sU32)a[5]), ((sInt)a[6])); return 0; }
sDInt W_Exec_IPP_Copy(sDInt *a) { Exec_IPP_Copy(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sU32)a[3]), (sF32)sBitFloat(a[4]), ((sInt)a[5])); return 0; }
sDInt W_Exec_IPP_Mask(sDInt *a) { Exec_IPP_Mask(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3])); return 0; }
sDInt W_Exec_IPP_Merge(sDInt *a) { Exec_IPP_Merge(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), ((sU32)a[5]), ((sInt)a[6])); return 0; }
sDInt W_Exec_IPP_RenderTarget(sDInt *a) { Exec_IPP_RenderTarget(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_IPP_Select(sDInt *a) { Exec_IPP_Select(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2])); return 0; }
sDInt W_Exec_IPP_Viewport(sDInt *a) {
  Exec_IPP_Viewport(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])),
    ((sInt)a[2]), ((sInt)a[3]), ((sU32)a[4]),
    sBitF323(a+5), sBitF323(a+8),
    (sF32)sBitFloat(a[11]), (sF32)sBitFloat(a[12]), (sF32)sBitFloat(a[13]),
    (sF32)sBitFloat(a[14]), (sF32)sBitFloat(a[15]), (sF32)sBitFloat(a[16]),
    ((sU32)a[17]), (sF32)sBitFloat(a[18]), (sF32)sBitFloat(a[19]),
    0,0,0,0,0,0, ((sInt)a[20]));
  return 0;
}
sDInt W_Exec_KKrieger_Events(sDInt *a) { Exec_KKrieger_Events(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_KKrieger_Monster(sDInt *a) { Exec_KKrieger_Monster(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_KKrieger_MonsterFlags(sDInt *a) { Exec_KKrieger_MonsterFlags(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4])); return 0; }
sDInt W_Exec_KKrieger_Para(sDInt *a) { Exec_KKrieger_Para(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_KKrieger_Respawn(sDInt *a) { Exec_KKrieger_Respawn(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), ((sInt)a[5]), (sF32)sBitFloat(a[6])); return 0; }
sDInt W_Exec_KKrieger_SplashDamage(sDInt *a) { Exec_KKrieger_SplashDamage(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4])); return 0; }
sDInt W_Exec_Material_Material(sDInt *a) { Exec_Material_Material(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Misc_Delay(sDInt *a) { Exec_Misc_Delay(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8])); return 0; }
sDInt W_Exec_Misc_Demo(sDInt *a) { Exec_Misc_Demo(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Misc_Event(sDInt *a) { Exec_Misc_Event(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), (sF32)sBitFloat(a[2])); return 0; }
sDInt W_Exec_Misc_If(sDInt *a) { Exec_Misc_If(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3])); return 0; }
sDInt W_Exec_Misc_Load(sDInt *a) { Exec_Misc_Load(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Misc_Menu(sDInt *a) { Exec_Misc_Menu(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4])); return 0; }
sDInt W_Exec_Misc_Nop(sDInt *a) { Exec_Misc_Nop(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Misc_PlaySample(sDInt *a) { Exec_Misc_PlaySample(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), (sF32)sBitFloat(a[4]), ((sInt)a[5]), (sF32)sBitFloat(a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8])); return 0; }
sDInt W_Exec_Misc_SetIf(sDInt *a) { Exec_Misc_SetIf(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), ((sInt)a[5])); return 0; }
sDInt W_Exec_Misc_State(sDInt *a) { Exec_Misc_State(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), ((sInt)a[5]), ((sInt)a[6])); return 0; }
sDInt W_Exec_Misc_Time(sDInt *a) { Exec_Misc_Time(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Misc_Trigger(sDInt *a) { Exec_Misc_Trigger(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), ((sInt)a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8]), ((sInt)a[9]), sBitF323(a+10), sBitF323(a+13), sBitF323(a+16), ((sU32)a[19]), ((KSpline *)AsPtr(a[20]))); return 0; }
sDInt W_Exec_Scene_Add(sDInt *a) { Exec_Scene_Add(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Scene_Camera(sDInt *a) { Exec_Scene_Camera(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), sBitF323(a+5), sBitF323(a+8)); return 0; }
sDInt W_Exec_Scene_Light(sDInt *a) { Exec_Scene_Light(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), sBitF323(a+5), ((sU32)a[8]), (sF32)sBitFloat(a[9]), ((sU32)a[10]), (sF32)sBitFloat(a[11]), (sF32)sBitFloat(a[12])); return 0; }
sDInt W_Exec_Scene_Limb(sDInt *a) { Exec_Scene_Limb(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), sBitF323(a+5), (sF32)sBitFloat(a[8]), (sF32)sBitFloat(a[9]), ((sU32)a[10])); return 0; }
sDInt W_Exec_Scene_MatHack(sDInt *a) { Exec_Scene_MatHack(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Scene_Multiply(sDInt *a) { Exec_Scene_Multiply(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), sBitF323(a+5), sBitF323(a+8), ((sInt)a[11])); return 0; }
sDInt W_Exec_Scene_Particles(sDInt *a) { Exec_Scene_Particles(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), sBitF323(a+5), sBitF323(a+8), sBitF323(a+11), (sF32)sBitFloat(a[14]), sBitF323(a+15), ((KSpline *)AsPtr(a[18]))); return 0; }
sDInt W_Exec_Scene_Physic(sDInt *a) { Exec_Scene_Physic(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sInt)a[2]), sBitF323(a+3), sBitF323(a+6), sBitF323(a+9), (sF32)sBitFloat(a[12]), ((sInt)a[13])); return 0; }
sDInt W_Exec_Scene_Portal(sDInt *a) { Exec_Scene_Portal(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF322(a+2), sBitF322(a+4), sBitF322(a+6), ((sInt)a[8]), (sF32)sBitFloat(a[9])); return 0; }
sDInt W_Exec_Scene_Rotate(sDInt *a) { Exec_Scene_Rotate(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), ((sInt)a[5])); return 0; }
sDInt W_Exec_Scene_Scene(sDInt *a) { Exec_Scene_Scene(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), sBitF323(a+5), sBitF323(a+8), ((sBool)a[11])); return 0; }
sDInt W_Exec_Scene_Sector(sDInt *a) { Exec_Scene_Sector(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); return 0; }
sDInt W_Exec_Scene_Transform(sDInt *a) { Exec_Scene_Transform(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), sBitF323(a+2), sBitF323(a+5), sBitF323(a+8)); return 0; }
sDInt W_Exec_Scene_Walk(sDInt *a) { Exec_Scene_Walk(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1])), ((sU32)a[2]), ((sInt)a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), sBitInt2(a+7), sBitF322(a+9), sBitF322(a+11), sBitF322(a+13), sBitF323(a+15), sBitF323(a+18), sBitF323(a+21), sBitF323(a+24), sBitF323(a+27), sBitF323(a+30), sBitF323(a+33), sBitF323(a+36), (sF32)sBitFloat(a[39]), (sF32)sBitFloat(a[40]), ((KSpline *)AsPtr(a[41]))); return 0; }
sDInt W_Init_Effect_PartEmitter(sDInt *a) { return (sDInt)Init_Effect_PartEmitter(((sInt)a[0]), ((sInt)a[1]), ((sInt)a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), sBitF324(a+5), sBitF324(a+9), sBitF324(a+13)); }
sDInt W_Init_Effect_PartSystem(sDInt *a) { return (sDInt)Init_Effect_PartSystem(((class GenMaterial *)a[0]), ((sInt)a[1]), ((sInt)a[2]), ((sU32)a[3]), ((sU32)a[4]), ((sU32)a[5]), sBitF323(a+6), (sF32)sBitFloat(a[9]), (sF32)sBitFloat(a[10]), (sF32)sBitFloat(a[11]), ((sInt)a[12])); }
sDInt W_Init_Effect_Print(sDInt *a) { return (sDInt)Init_Effect_Print(((class GenMaterial *)a[0]), ((sInt)a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), ((sChar *)a[7])); }
sDInt W_Init_IPP_Blur(sDInt *a) { return (sDInt)Init_IPP_Blur(((GenIPP *)AsPtr(a[0])), ((sInt)a[1]), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), ((sInt)a[4]), ((sInt)a[5]), ((sInt)a[6])); }
sDInt W_Init_IPP_Color(sDInt *a) { return (sDInt)Init_IPP_Color(((GenIPP *)AsPtr(a[0])), ((sInt)a[1]), ((sU32)a[2]), ((sInt)a[3]), ((sU32)a[4]), ((sInt)a[5])); }
sDInt W_Init_IPP_Copy(sDInt *a) { return (sDInt)Init_IPP_Copy(((GenIPP *)AsPtr(a[0])), ((sInt)a[1]), ((sU32)a[2]), (sF32)sBitFloat(a[3]), ((sInt)a[4])); }
sDInt W_Init_IPP_Mask(sDInt *a) { return (sDInt)Init_IPP_Mask(((GenIPP *)AsPtr(a[0])), ((GenIPP *)AsPtr(a[1])), ((GenIPP *)AsPtr(a[2])), ((sInt)a[3]), ((sInt)a[4])); }
sDInt W_Init_IPP_Merge(sDInt *a) { return (sDInt)Init_IPP_Merge(((GenIPP *)AsPtr(a[0])), ((GenIPP *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), ((sU32)a[5]), ((sInt)a[6])); }
sDInt W_Init_IPP_RenderTarget(sDInt *a) { return (sDInt)Init_IPP_RenderTarget(((GenIPP *)AsPtr(a[0])), ((GenBitmap *)AsPtr(a[1]))); }
sDInt W_Init_IPP_Select(sDInt *a) {
  sInt count=(sInt)a[0];
  for(sInt i=0;i<count;i++)
  {
    GenIPP *in = (GenIPP *)AsPtr(a[1+i]);
    if(in) in->Release();
  }
  return (sDInt)new GenIPP;
}
sDInt W_Init_IPP_Viewport(sDInt *a) {
  // kx packing is 1 input + bbcggggggffffffcff + outputcount; no spline pointer
  return (sDInt)Init_IPP_Viewport(((GenScene *)AsPtr(a[0])), 0,
    ((sInt)a[1]), ((sInt)a[2]), ((sU32)a[3]),
    sBitF323(a+4), sBitF323(a+7),
    (sF32)sBitFloat(a[10]), (sF32)sBitFloat(a[11]), (sF32)sBitFloat(a[12]),
    (sF32)sBitFloat(a[13]), (sF32)sBitFloat(a[14]), (sF32)sBitFloat(a[15]),
    ((sU32)a[16]), (sF32)sBitFloat(a[17]), (sF32)sBitFloat(a[18]),
    0,0,0,0,0,0, ((sInt)a[19]));
}
sDInt W_Init_KKrieger_Events(sDInt *a) { return (sDInt)Init_KKrieger_Events(((KOp *)AsPtr(a[0]))); }
sDInt W_Init_KKrieger_Monster(sDInt *a) { return (sDInt)Init_KKrieger_Monster(((KOp *)AsPtr(a[0]))); }
sDInt W_Init_KKrieger_MonsterFlags(sDInt *a) { return (sDInt)Init_KKrieger_MonsterFlags(((KOp *)AsPtr(a[0]))); }
sDInt W_Init_KKrieger_Para(sDInt *a) { return (sDInt)Init_KKrieger_Para(((KOp *)AsPtr(a[0])), ((KEnvironment *)AsPtr(a[1]))); }
sDInt W_Init_KKrieger_Respawn(sDInt *a) { return (sDInt)Init_KKrieger_Respawn(sBitF323(a+0), ((sInt)a[3]), (sF32)sBitFloat(a[4])); }
sDInt W_Init_KKrieger_SplashDamage(sDInt *a) { return (sDInt)Init_KKrieger_SplashDamage(((sInt)a[0]), (sF32)sBitFloat(a[1]), (sF32)sBitFloat(a[2])); }
sDInt W_Init_Material_Material(sDInt *a) { return (sDInt)Init_Material_Material(((KOp *)AsPtr(a[0]))); }
sDInt W_Init_Misc_Demo(sDInt *a) { return (sDInt)Init_Misc_Demo(((KOp *)AsPtr(a[0]))); }
sDInt W_Init_Misc_Event(sDInt *a) { return (sDInt)Init_Misc_Event(((KObject *)AsPtr(a[0])), (sF32)sBitFloat(a[1])); }
sDInt W_Init_Misc_Load(sDInt *a) { return (sDInt)Init_Misc_Load(((KObject *)AsPtr(a[0]))); }
sDInt W_Init_Misc_Nop(sDInt *a) { return (sDInt)Init_Misc_Nop(((KOp *)AsPtr(a[0]))); }
sDInt W_Init_Misc_Trigger(sDInt *a) { return (sDInt)Init_Misc_Trigger(((KObject *)AsPtr(a[0])), ((KOp *)AsPtr(a[1])), (sF32)sBitFloat(a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), ((sInt)a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8]), ((sInt)a[9]), sBitF323(a+10), sBitF323(a+13), sBitF323(a+16), ((sU32)a[19]), ((KSpline *)AsPtr(a[20]))); }
// variable-input ops (OPC_VARIABLEINPUT: a[0]=count, a[1..count]=inputs) can have up to
// KK_MAXINPUT inputs, so they are implemented here without going through varargs
GenScene *MakeScene(KObject *in);
sDInt W_Init_Scene_Add(sDInt *a) {
  sInt count=(sInt)a[0];
  GenScene *scene = new GenScene;
  for(sInt i=0;i<count;i++)
  {
    GenScene *add = MakeScene((KObject *)AsPtr(a[1+i]));
    if(add)
      *scene->Childs.Add() = add;
  }
  return (sDInt)scene;
}
sDInt W_Init_Scene_Camera(sDInt *a) { return (sDInt)Init_Scene_Camera(sBitF323(a+0), sBitF323(a+3), sBitF323(a+6)); }
sDInt W_Init_Scene_Light(sDInt *a) { return (sDInt)Init_Scene_Light(sBitF323(a+0), sBitF323(a+3), ((sU32)a[6]), (sF32)sBitFloat(a[7]), ((sU32)a[8]), (sF32)sBitFloat(a[9]), (sF32)sBitFloat(a[10])); }
sDInt W_Init_Scene_Limb(sDInt *a) { return (sDInt)Init_Scene_Limb(((GenScene *)AsPtr(a[0])), ((GenScene *)AsPtr(a[1])), sBitF323(a+2), sBitF323(a+5), (sF32)sBitFloat(a[8]), (sF32)sBitFloat(a[9]), ((sU32)a[10])); }
sDInt W_Init_Scene_MatHack(sDInt *a) { return (sDInt)Init_Scene_MatHack(((class GenMaterial *)a[0])); }
sDInt W_Init_Scene_Multiply(sDInt *a) { return (sDInt)Init_Scene_Multiply(((GenScene *)AsPtr(a[0])), sBitF323(a+1), sBitF323(a+4), sBitF323(a+7), ((sInt)a[10])); }
sDInt W_Init_Scene_Particles(sDInt *a) { return (sDInt)Init_Scene_Particles(((GenScene *)AsPtr(a[0])), ((sInt)a[1]), ((sInt)a[2]), ((sInt)a[3]), sBitF323(a+4), sBitF323(a+7), sBitF323(a+10), (sF32)sBitFloat(a[13]), sBitF323(a+14), ((KSpline *)AsPtr(a[17]))); }
sDInt W_Init_Scene_Physic(sDInt *a) { return (sDInt)Init_Scene_Physic(((GenScene *)AsPtr(a[0])), ((sInt)a[1]), sBitF323(a+2), sBitF323(a+5), sBitF323(a+8), (sF32)sBitFloat(a[11]), ((sInt)a[12])); }
sDInt W_Init_Scene_Portal(sDInt *a) { return (sDInt)Init_Scene_Portal(((GenScene *)AsPtr(a[0])), ((GenScene *)AsPtr(a[1])), ((GenScene *)AsPtr(a[2])), ((GenScene *)AsPtr(a[3])), sBitF322(a+4), sBitF322(a+6), sBitF322(a+8), ((sInt)a[10]), (sF32)sBitFloat(a[11])); }
sDInt W_Init_Scene_Rotate(sDInt *a) { return (sDInt)Init_Scene_Rotate(((GenScene *)AsPtr(a[0])), sBitF323(a+1), ((sInt)a[4])); }
sDInt W_Init_Scene_Scene(sDInt *a) { return (sDInt)Init_Scene_Scene(((GenMesh *)AsPtr(a[0])), sBitF323(a+1), sBitF323(a+4), sBitF323(a+7), ((sBool)a[10])); }
sDInt W_Init_Scene_Sector(sDInt *a) { return (sDInt)Init_Scene_Sector(((GenScene *)AsPtr(a[0]))); }
sDInt W_Init_Scene_Transform(sDInt *a) { return (sDInt)Init_Scene_Transform(((GenScene *)AsPtr(a[0])), sBitF323(a+1), sBitF323(a+4), sBitF323(a+7)); }
sDInt W_Init_Scene_Walk(sDInt *a) { return (sDInt)Init_Scene_Walk(((GenScene *)AsPtr(a[0])), ((sU32)a[1]), ((sInt)a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), sBitInt2(a+6), sBitF322(a+8), sBitF322(a+10), sBitF322(a+12), sBitF323(a+14), sBitF323(a+17), sBitF323(a+20), sBitF323(a+23), sBitF323(a+26), sBitF323(a+29), sBitF323(a+32), sBitF323(a+35), (sF32)sBitFloat(a[38]), (sF32)sBitFloat(a[39]), ((KSpline *)AsPtr(a[40]))); }
sDInt W_Material_Add(sDInt *a) {
  sInt count=(sInt)a[0];
  GenMaterial *mtrl = new GenMaterial;
  for(sInt i=0;i<count;i++)
  {
    GenMaterial *madd = (GenMaterial *)AsPtr(a[1+i]);
    if(madd)
    {
      for(sInt j=0;j<madd->Passes.Count;j++)
      {
        GenMaterialPass *pc = mtrl->Passes.Add();
        *pc = madd->Passes[j];
        pc->Mtrl->AddRef();
      }
      madd->Release();
    }
  }
  return (sDInt)mtrl;
}
sDInt W_Mesh_Add(sDInt *a) {
  sInt count=(sInt)a[0];
  GenMesh *mesh = (GenMesh *)AsPtr(a[1]);
  if(CheckMesh(mesh)) return 0;
  for(sInt i=1;i<count;i++)
  {
    GenMesh *other = (GenMesh *)AsPtr(a[1+i]);
    if(other)
    {
      mesh->Add(other);
      other->Release();
    }
  }
  mesh->GotNormals = sFALSE;
  return (sDInt)mesh;
}
sDInt W_Mesh_AutoCollision(sDInt *a) { return (sDInt)Mesh_AutoCollision(((GenMesh *)AsPtr(a[0])), (sF32)sBitFloat(a[1]), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), ((sInt)a[5]), ((sInt)a[6])); }
sDInt W_Mesh_Bend(sDInt *a) { return (sDInt)Mesh_Bend(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((sInt)a[2]), sBitFSRT(a+3), sBitFSRT(a+12), sBitF322(a+21), sBitF322(a+23), ((sInt)a[25])); }
sDInt W_Mesh_Bend2(sDInt *a) { return (sDInt)Mesh_Bend2(((GenMesh *)AsPtr(a[0])), sBitF323(a+1), sBitF323(a+4), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8])); }
sDInt W_Mesh_Bevel(sDInt *a) { return (sDInt)Mesh_Bevel(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((sInt)a[2]), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), ((sInt)a[5])); }
sDInt W_Mesh_Center(sDInt *a) { return (sDInt)Mesh_Center(((GenMesh *)AsPtr(a[0])), ((sInt)a[1]), ((sInt)a[2])); }
sDInt W_Mesh_CollisionCube(sDInt *a) { return (sDInt)Mesh_CollisionCube(((GenMesh *)AsPtr(a[0])), ((KOp *)AsPtr(a[1])), ((KOp *)AsPtr(a[2])), (sF32)sBitFloat(a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5]), (sF32)sBitFloat(a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8]), ((sInt)a[9]), ((sInt)a[10]), ((sInt)a[11]), ((sInt)a[12]), ((sInt)a[13]), ((sInt)a[14]), ((sInt)a[15]), ((sInt)a[16])); }
sDInt W_Mesh_Color(sDInt *a) { return (sDInt)Mesh_Color(((GenMesh *)AsPtr(a[0])), sBitF323(a+1), sBitF322(a+4), ((sU32)a[6]), (sF32)sBitFloat(a[7]), (sF32)sBitFloat(a[8]), ((sInt)a[9])); }
sDInt W_Mesh_Crease(sDInt *a) { return (sDInt)Mesh_Crease(((GenMesh *)AsPtr(a[0])), ((sInt)a[1]), ((sInt)a[2]), ((sInt)a[3])); }
sDInt W_Mesh_Cube(sDInt *a) { return (sDInt)Mesh_Cube(((sInt)a[0]), ((sInt)a[1]), ((sInt)a[2]), ((sInt)a[3]), sBitFSRT(a+4)); }
sDInt W_Mesh_Cylinder(sDInt *a) { return (sDInt)Mesh_Cylinder(((sInt)a[0]), ((sInt)a[1]), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4])); }
sDInt W_Mesh_DeleteFaces(sDInt *a) { return (sDInt)Mesh_DeleteFaces(((GenMesh *)AsPtr(a[0])), ((sInt)a[1])); }
sDInt W_Mesh_Displace(sDInt *a) { return (sDInt)Mesh_Displace(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((GenBitmap *)AsPtr(a[2])), ((sInt)a[3]), sBitF323(a+4)); }
sDInt W_Mesh_Extrude(sDInt *a) { return (sDInt)Mesh_Extrude(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3]), ((sInt)a[4]), ((sInt)a[5]), sBitF323(a+6), sBitF323(a+9), sBitF323(a+12)); }
sDInt W_Mesh_Grid(sDInt *a) { return (sDInt)Mesh_Grid(((sInt)a[0]), ((sInt)a[1]), ((sInt)a[2])); }
sDInt W_Mesh_LightSlot(sDInt *a) { return (sDInt)Mesh_LightSlot(sBitF323(a+0), ((sU32)a[3]), (sF32)sBitFloat(a[4]), (sF32)sBitFloat(a[5])); }
sDInt W_Mesh_MatLink(sDInt *a) { return (sDInt)Mesh_MatLink(((GenMesh *)AsPtr(a[0])), ((GenMaterial *)AsPtr(a[1])), ((sInt)a[2]), ((sInt)a[3])); }
sDInt W_Mesh_Multiply(sDInt *a) { return (sDInt)Mesh_Multiply(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), sBitFSRT(a+2), ((sInt)a[11]), ((sInt)a[12]), (sF32)sBitFloat(a[13]), (sF32)sBitFloat(a[14]), sBitF323(a+15), (sF32)sBitFloat(a[18])); }
sDInt W_Mesh_Perlin(sDInt *a) { return (sDInt)Mesh_Perlin(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((sInt)a[2]), sBitFSRT(a+3), sBitF323(a+12)); }
sDInt W_Mesh_SelectCube(sDInt *a) { return (sDInt)Mesh_SelectCube(((GenMesh *)AsPtr(a[0])), ((sU32)a[1]), ((sInt)a[2]), sBitF323(a+3), sBitF323(a+6)); }
sDInt W_Mesh_SelectSphere(sDInt *a) { return (sDInt)Mesh_SelectSphere(((GenMesh *)AsPtr(a[0])), ((sU32)a[1]), ((sInt)a[2]), sBitF323(a+3), sBitF323(a+6)); }
sDInt W_Mesh_ShadowEnable(sDInt *a) { return (sDInt)Mesh_ShadowEnable(((GenMesh *)AsPtr(a[0])), ((sBool)a[1])); }
sDInt W_Mesh_SingleVert(sDInt *a) { return (sDInt)Mesh_SingleVert(); }
sDInt W_Mesh_Subdivide(sDInt *a) { return (sDInt)Mesh_Subdivide(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((sInt)a[2]), (sF32)sBitFloat(a[3]), ((sInt)a[4])); }
sDInt W_Mesh_Transform(sDInt *a) { return (sDInt)Mesh_Transform(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((sInt)a[2]), sBitFSRT(a+3)); }
sDInt W_Mesh_TransformEx(sDInt *a) { return (sDInt)Mesh_TransformEx(((KOp *)AsPtr(a[0])), ((GenMesh *)AsPtr(a[1])), ((sInt)a[2]), sBitFSRT(a+3), ((sInt)a[12]), ((sInt)a[13])); }
sDInt W_Mesh_Triangulate(sDInt *a) { return (sDInt)Mesh_Triangulate(((GenMesh *)AsPtr(a[0])), ((sInt)a[1]), ((sInt)a[2]), ((sInt)a[3])); }
sDInt W_Mesh_UVProjection(sDInt *a) { return (sDInt)Mesh_UVProjection(((GenMesh *)AsPtr(a[0])), ((sInt)a[1]), sBitFSRT(a+2), ((sInt)a[11])); }

KHandler KHandlers[] =
{
  // misc
  0x01, W_Init_Misc_Load,         W_Exec_Misc_Load,             // load
  0x02, W_Init_Misc_Nop,          W_Exec_Misc_Nop,              // store
  0x03, W_Init_Misc_Nop,          W_Exec_Misc_Nop,              // nop
  0x05, W_Init_Misc_Nop,          W_Exec_Misc_Time,             // time
  0x06, W_Init_Misc_Event,        W_Exec_Misc_Event,            // event
  0x07, W_Init_Misc_Trigger,      W_Exec_Misc_Trigger,          // trigger
  0x08, W_Init_Misc_Nop,          W_Exec_Misc_Delay,            // delay
  0x09, W_Init_Misc_Nop,          W_Exec_Misc_If,               // if
  0x0a, W_Init_Misc_Nop,          W_Exec_Misc_SetIf,            // setif
  0x0b, W_Init_Misc_Nop,          W_Exec_Misc_State,            // state
  0x0c, W_Init_Misc_Nop,          W_Exec_Misc_PlaySample,       // play sample
  0x0d, W_Init_Misc_Demo,         W_Exec_Misc_Demo,             // demo 
  0x0e, W_Init_Misc_Nop,          W_Exec_Misc_Menu,             // menu

  // kkrieger
  0x10, W_Init_KKrieger_Para,     W_Exec_KKrieger_Para,         // KKriegerPara
  0x11, W_Init_KKrieger_Monster,  W_Exec_KKrieger_Monster,      // KKriegerMonster
  0x12, W_Init_KKrieger_Events,   W_Exec_KKrieger_Events,       // KKriegerEvents
  0x13, W_Init_KKrieger_Respawn,  W_Exec_KKrieger_Respawn,
  0x14, W_Init_KKrieger_SplashDamage,  W_Exec_KKrieger_SplashDamage,
  0x15, W_Init_KKrieger_MonsterFlags,  W_Exec_KKrieger_MonsterFlags,

  // bitmap
  0x21, W_Bitmap_Flat,            W_Exec_Misc_Nop,              // flat
  0x22, W_Bitmap_Perlin,          W_Exec_Misc_Nop,              // perlin
  0x23, W_Bitmap_Color,           W_Exec_Misc_Nop,              // color
  0x24, W_Bitmap_Merge,           W_Exec_Misc_Nop,              // merge
  0x25, W_Bitmap_Format,          W_Exec_Misc_Nop,              // format
  0x26, W_Bitmap_RenderTarget,    W_Exec_Misc_Nop,              // render target
  0x27, W_Bitmap_GlowRect,        W_Exec_Misc_Nop,              // glowrect (old)
  0x28, W_Bitmap_Dots,            W_Exec_Misc_Nop,              // dots
  0x29, W_Bitmap_Blur,            W_Exec_Misc_Nop,              // blur
  0x2a, W_Bitmap_Mask,            W_Exec_Misc_Nop,              // mask
  0x2b, W_Bitmap_HSCB,            W_Exec_Misc_Nop,              // hscb
  0x2c, W_Bitmap_Rotate,          W_Exec_Misc_Nop,              // rotate
  0x2d, W_Bitmap_Distort,         W_Exec_Misc_Nop,              // distort
  0x2e, W_Bitmap_Normals,         W_Exec_Misc_Nop,              // normals
  0x2f, W_Bitmap_Light,           W_Exec_Misc_Nop,              // light
  0x30, W_Bitmap_Bump,            W_Exec_Misc_Nop,              // bump
  0x31, W_Bitmap_Text,            W_Exec_Misc_Nop,              // text
  0x32, W_Bitmap_Cell,            W_Exec_Misc_Nop,              // cell
  0x34, W_Bitmap_Gradient,        W_Exec_Misc_Nop,              // gradient
  0x35, W_Bitmap_Range,           W_Exec_Misc_Nop,              // range
  0x36, W_Bitmap_RotateMul,       W_Exec_Misc_Nop,              // rotatemul
  0x37, W_Bitmap_Twirl,           W_Exec_Misc_Nop,              // twirl
  0x38, W_Bitmap_Sharpen,         W_Exec_Misc_Nop,              // sharpen
  0x39, W_Bitmap_GlowRect,        W_Exec_Misc_Nop,              // glowrect
  0x3b, W_Bitmap_ColorBalance,    W_Exec_Misc_Nop,              // color balance

  // effecte
//  0x60, Init_Effect_Dummy,      Exec_Effect_Dummy,          // dummy-effect
  0x61, W_Init_Effect_Print,      W_Exec_Effect_Print,          // print text/menu
  0x63, W_Init_Effect_PartEmitter,W_Exec_Effect_PartEmitter,    // particles
  0x64, W_Init_Effect_PartSystem, W_Exec_Effect_PartSystem,     // particles

  // mesh
  0x81, W_Mesh_Cube,              W_Exec_Misc_Nop,              // cube
  0x82, W_Mesh_Cylinder,          W_Exec_Misc_Nop,              // cylinder
//  0x83, Mesh_Torus,             W_Exec_Misc_Nop,              // torus
//  0x84, Mesh_Sphere,            W_Exec_Misc_Nop,              // sphere
//  0x85, Mesh_SelectAll,         W_Exec_Misc_Nop,              // select all
  0x86, W_Mesh_SelectCube,        W_Exec_Misc_Nop,              // select cube
  0x87, W_Mesh_Subdivide,         W_Exec_Misc_Nop,              // subdivide
  0x88, W_Mesh_Transform,         W_Exec_Misc_Nop,              // transform
  0x89, W_Mesh_TransformEx,       W_Exec_Misc_Nop,              // transformex
  0x8a, W_Mesh_Crease,            W_Exec_Misc_Nop,              // crease
//  0x8b, Mesh_UnCrease,          W_Exec_Misc_Nop,              // uncrease
  0x8c, W_Mesh_Triangulate,       W_Exec_Misc_Nop,              // triangulate
//  0x8d, Mesh_Cut,               W_Exec_Misc_Nop,              // cut
//  0x8e, Mesh_ExtrudeNormal,     W_Exec_Misc_Nop,              // extrudenormal
  0x8f, W_Mesh_Displace,          W_Exec_Misc_Nop,              // displace
// bevel ONLY used in full level!
  0x90, W_Mesh_Bevel,             W_Exec_Misc_Nop,              // bevel
  0x91, W_Mesh_Perlin,            W_Exec_Misc_Nop,              // perlin
  0x92, W_Mesh_Add,               W_Exec_Misc_Nop,              // add
// deletefaces ONLY used in full level! (and kkrieger-logo)
  0x93, W_Mesh_DeleteFaces,       W_Exec_Misc_Nop,              // delete faces
//  0x94, Mesh_SelectRandom,      W_Exec_Misc_Nop,              // select random
  0x95, W_Mesh_Multiply,          W_Exec_Misc_Nop,              // multiply
  0x96, W_Mesh_MatLink,           W_Exec_Misc_Nop,              // matlink
//  0x99, Mesh_BeginRecord,       W_Exec_Misc_Nop,              // beginrec
  0x9a, W_Mesh_Extrude,           W_Exec_Misc_Nop,              // extrude
  0x9c, W_Mesh_CollisionCube,     W_Exec_Misc_Nop,              // collcube
// grid ONLY used in full level! (and kkrieger-logo)
  0x9d, W_Mesh_Grid,              W_Exec_Misc_Nop,              // grid
  0xa0, W_Mesh_Bend,              W_Exec_Misc_Nop,              // bend
//  0xa1, Mesh_SelectLogic,       W_Exec_Misc_Nop,              // select logic
//  0xa2, Mesh_SelectGrow,        W_Exec_Misc_Nop,              // select grow
//  0xa3, Mesh_Invert,            W_Exec_Misc_Nop,              // invert
//  0xa4, Mesh_SetPivot,          W_Exec_Misc_Nop,              // set pivot
  0xa5, W_Mesh_UVProjection,      W_Exec_Misc_Nop,              // uvproject
  0xa6, W_Mesh_Center,            W_Exec_Misc_Nop,              // center
  0xa7, W_Mesh_AutoCollision,     W_Exec_Misc_Nop,              // autocoll
  0xa8, W_Mesh_SelectSphere,      W_Exec_Misc_Nop,              // selectsphere
//  0xa9, Mesh_SelectFace,        W_Exec_Misc_Nop,              // selectface
//  0xaa, Mesh_SelectAngle,       W_Exec_Misc_Nop,              // selectangle
  0xab, W_Mesh_Bend2,             W_Exec_Misc_Nop,              // bend 2
//  0xac, Mesh_SmoothAngle,       W_Exec_Misc_Nop,              // smangle
  0xad, W_Mesh_Color,             W_Exec_Misc_Nop,              // color
//  0xae, Mesh_BendS,             W_Exec_Misc_Nop,              // bend spline
  0xaf, W_Mesh_LightSlot,         W_Exec_Misc_Nop,              // light slot
  0xb0, W_Mesh_ShadowEnable,      W_Exec_Misc_Nop,              // shadowenable
  0xb3, W_Mesh_SingleVert,        W_Exec_Misc_Nop,              // single vertex

  // scene
  0xc0, W_Init_Scene_Scene,       W_Exec_Scene_Scene,           // scene
  0xc1, W_Init_Scene_Add,         W_Exec_Scene_Add,             // add
  0xc2, W_Init_Scene_Multiply,    W_Exec_Scene_Multiply,        // multiply
  0xc3, W_Init_Scene_Transform,   W_Exec_Scene_Transform,       // transform
  0xc4, W_Init_Scene_Light,       W_Exec_Scene_Light,           // light
  0xc5, W_Init_Scene_Particles,   W_Exec_Scene_Particles,       // scene multply particles
  0xc6, W_Init_Scene_Camera,      W_Exec_Scene_Camera,          // camera

  0xc7, W_Init_Scene_Limb,        W_Exec_Scene_Limb,            // limb
  0xc8, W_Init_Scene_Walk,        W_Exec_Scene_Walk,            // walk
  0xc9, W_Init_Scene_Rotate,      W_Exec_Scene_Rotate,          // rotate
//  0xca, Init_Scene_Forward,     Exec_Scene_Forward,         // forward
  0xcb, W_Init_Scene_Sector,      W_Exec_Scene_Sector,          // sector
  0xcd, W_Init_Scene_Portal,      W_Exec_Scene_Portal,          // portal
  0xce, W_Init_Scene_Physic,      W_Exec_Scene_Physic,          // physic
  0xce, W_Init_Scene_Physic,      W_Exec_Scene_Physic,          // physic

  // material
  0xd0, W_Init_Material_Material, W_Exec_Material_Material,     // material
  0xd1, W_Material_Add,           W_Exec_Misc_Nop,              // add material
  0xd2, W_Init_Scene_MatHack,     W_Exec_Scene_MatHack,         // link material for animation

  // ipp
  0xe2, W_Init_IPP_Copy,          W_Exec_IPP_Copy,              // copy
  0xe3, W_Init_IPP_Blur,          W_Exec_IPP_Blur,              // blur
//  0xe4, Init_IPP_Crashzoom,     Exec_IPP_Crashzoom,         // crashzoom
//  0xe5, Init_IPP_Sharpen,       Exec_IPP_Sharpen,           // sharpen
  0xe6, W_Init_IPP_Color,         W_Exec_IPP_Color,             // color
  0xe7, W_Init_IPP_Merge,         W_Exec_IPP_Merge,             // merge
  0xe8, W_Init_IPP_Mask,          W_Exec_IPP_Mask,              // mask
//  0xe9, Init_IPP_Layer2D,       Exec_IPP_Layer2D,           // layer2d
  0xea, W_Init_IPP_Select,        W_Exec_IPP_Select,            // select
  0xeb, W_Init_IPP_RenderTarget,  W_Exec_IPP_RenderTarget,      // render target
  0xf0, W_Init_IPP_Viewport,      W_Exec_IPP_Viewport,          // viewport

  // minmesh
//  0x100, MinMesh_SingleVert,    W_Exec_Misc_Nop,              // single vert
//  0x101, MinMesh_Grid,          W_Exec_Misc_Nop,              // grid
//  0x102, MinMesh_Cube,          W_Exec_Misc_Nop,              // cube
//  0x103, MinMesh_Torus,         W_Exec_Misc_Nop,              // torus
//  0x104, MinMesh_Sphere,        W_Exec_Misc_Nop,              // sphere
//  0x105, MinMesh_Cylinder,      W_Exec_Misc_Nop,              // cylinder
//  0x106, MinMesh_XSI,           W_Exec_Misc_Nop,              // xsi

//  0x110, MinMesh_MatLink,       W_Exec_Misc_Nop,              // matlink
//  0x111, MinMesh_Add,           W_Exec_Misc_Nop,              // add
//  0x112, MinMesh_SelectAll,     W_Exec_Misc_Nop,              // select all
//  0x113, MinMesh_SelectCube,    W_Exec_Misc_Nop,              // select cube
//  0x114, MinMesh_DeleteFaces,   W_Exec_Misc_Nop,              // delete faces
//  0x115, MinMesh_Invert,        W_Exec_Misc_Nop,              // invert

//  0x120, MinMesh_TransformEx,   W_Exec_Misc_Nop,              // transform
//  0x121, MinMesh_TransformEx,   W_Exec_Misc_Nop,              // transform ex
//  0x122, MinMesh_ExtrudeNormal, W_Exec_Misc_Nop,              // extrude normal
//  0x123, MinMesh_Displace,      W_Exec_Misc_Nop,              // displace
//  0x124, MinMesh_Perlin,        W_Exec_Misc_Nop,              // perlin
//  0x125, MinMesh_Bend2,         W_Exec_Misc_Nop,              // bend 2

  // end!
  0,0,0,
};
