#include "d3dshader.hpp"
#include <cstdio>
#include <cmath>
#include <map>
#include <vector>

namespace {

enum
{
  OP_NOP=0, OP_MOV, OP_ADD, OP_SUB, OP_MAD, OP_MUL, OP_RCP, OP_RSQ, OP_DP3, OP_DP4,
  OP_MIN, OP_MAX, OP_SLT, OP_SGE, OP_EXP, OP_LOG, OP_LIT, OP_DST, OP_LRP, OP_FRC,
  OP_M4x4, OP_M4x3, OP_M3x4, OP_M3x3, OP_M3x2,
  OP_DCL=31, OP_POW, OP_CRS, OP_SGN, OP_ABS, OP_NRM, OP_SINCOS,
  OP_MOVA=46,
  OP_TEXCOORD=64, OP_TEXKILL, OP_TEX, OP_TEXBEM, OP_TEXBEML, OP_TEXREG2AR, OP_TEXREG2GB,
  OP_TEXM3x2PAD, OP_TEXM3x2TEX, OP_TEXM3x3PAD, OP_TEXM3x3TEX, OP_TEXM3x3DIFF,
  OP_TEXM3x3SPEC, OP_TEXM3x3VSPEC, OP_EXPP, OP_LOGP, OP_CND, OP_DEF, OP_TEXREG2RGB,
  OP_TEXDP3TEX, OP_TEXM3x2DEPTH, OP_TEXDP3, OP_TEXM3x3, OP_TEXDEPTH, OP_CMP, OP_BEM,
  OP_DP2ADD,
  OP_PHASE=0xfffd, OP_COMMENT=0xfffe, OP_END=0xffff,
};

// operand count for shaders whose opcode length field was stripped
static sInt opLength(sU32 op,sInt ver)
{
  switch(op)
  {
  case OP_NOP: return 0;
  case OP_MOV: case OP_RCP: case OP_RSQ: case OP_EXP: case OP_LOG: case OP_LIT:
  case OP_FRC: case OP_ABS: case OP_NRM: case OP_MOVA: case OP_DCL:
  case OP_EXPP: case OP_LOGP:
    return 2;
  case OP_ADD: case OP_SUB: case OP_MUL: case OP_DP3: case OP_DP4: case OP_MIN:
  case OP_MAX: case OP_SLT: case OP_SGE: case OP_DST: case OP_M4x4: case OP_M4x3:
  case OP_M3x4: case OP_M3x3: case OP_M3x2: case OP_POW: case OP_CRS: case OP_BEM:
    return 3;
  case OP_MAD: case OP_LRP: case OP_CND: case OP_CMP: case OP_DP2ADD: case OP_SGN:
    return 4;
  case OP_SINCOS: return ver>=0x300 ? 2 : 4;
  case OP_DEF: return 5;
  case OP_TEXCOORD: return ver>=0x104 ? 2 : 1;
  case OP_TEX: return ver>=0x200 ? 3 : (ver>=0x104 ? 2 : 1);
  case OP_TEXKILL: case OP_TEXDEPTH: return 1;
  case OP_TEXBEM: case OP_TEXBEML: case OP_TEXREG2AR: case OP_TEXREG2GB:
  case OP_TEXM3x2PAD: case OP_TEXM3x2TEX: case OP_TEXM3x3PAD: case OP_TEXM3x3TEX:
  case OP_TEXM3x3VSPEC: case OP_TEXREG2RGB: case OP_TEXDP3TEX: case OP_TEXM3x2DEPTH:
  case OP_TEXDP3: case OP_TEXM3x3:
    return 2;
  case OP_TEXM3x3DIFF: case OP_TEXM3x3SPEC: return 3;
  }
  return -1;
}

static sInt regType(sU32 t) { return ((t>>28)&7) | ((t>>8)&0x18); }
static sInt regNum(sU32 t)  { return t & 0x7ff; }

static std::string num(sInt i) { char b[16]; snprintf(b,sizeof(b),"%d",i); return b; }
static std::string flt(sF32 f)
{
  if(!std::isfinite(f)) f = f>0 ? 3.0e38f : -3.0e38f;
  char b[32]; snprintf(b,sizeof(b),"%.9e",f); return b;
}

static const char *kComp = "xyzw";

static std::string maskStr(sU32 m)
{
  std::string s;
  for(sInt i=0;i<4;i++) if(m&(1<<i)) s+=kComp[i];
  return s;
}

struct Ctx
{
  sBool vs;
  sInt ver;
  sF32 range;                    // ps register clamp, 0 = none
  std::map<sInt,std::string> defs;
  sU32 inputs;
  sInt slot[16];
  sU32 samplers;
  sInt pad;                      // texm3xN pad count
  sInt tmp;
  sBool ok;
  std::string code;
};

struct Dst
{
  sInt type,num;
  sU32 mask;
  sInt shift;
  sBool sat;
  std::string name;
};

static std::string regName(Ctx &c,sU32 t,sInt off)
{
  sInt type = regType(t), n = regNum(t)+off;
  if(c.vs)
  {
    switch(type)
    {
    case 0: return "r"+num(n);
    case 1: c.inputs |= 1u<<(n&15); return "v"+num(n);
    case 2:
      if(t & 0x2000) return "vc[clamp(a0.x+"+num(n)+",0,255)]";
      if(c.defs.count(n)) return "k"+num(n);
      return "vc["+num(n)+"]";
    case 3: return "vec4(a0)";
    }
  }
  else
  {
    switch(type)
    {
    case 0: return "r"+num(n);
    case 1: return "v"+num(n&1);
    case 2:
      if(c.defs.count(n)) return "k"+num(n);
      if(c.ver<0x200) return "clamp(pc["+num(n&31)+"],-1.0,1.0)";
      return "pc["+num(n&31)+"]";
    case 3: return (c.ver>=0x104 ? "xT" : "t")+num(n&7);
    }
  }
  fprintf(stderr,"d3dshader: unsupported source register type %d\n",type);
  c.ok = sFALSE;
  return "vec4(0.0)";
}

static std::string srcExpr(Ctx &c,const sU32 *&p,sInt off=0)
{
  sU32 t = *p++;
  if(c.vs && c.ver>=0x200 && (t&0x2000)) p++;   // relative address token
  std::string r = regName(c,t,off);
  sU32 swz = (t>>16)&0xff;
  if(swz!=0xe4)
  {
    r += '.';
    for(sInt i=0;i<4;i++) r += kComp[(swz>>(2*i))&3];
  }
  switch((t>>24)&15)
  {
  case 0:  break;
  case 1:  r = "(-"+r+")"; break;
  case 2:  r = "("+r+"-0.5)"; break;
  case 3:  r = "(0.5-"+r+")"; break;
  case 4:  r = "("+r+"*2.0-1.0)"; break;
  case 5:  r = "(1.0-"+r+"*2.0)"; break;
  case 6:  r = "(1.0-"+r+")"; break;
  case 7:  r = "("+r+"*2.0)"; break;
  case 8:  r = "("+r+"*-2.0)"; break;
  case 9:  r = "dz("+r+")"; break;
  case 10: r = "dw("+r+")"; break;
  case 11: r = "abs("+r+")"; break;
  case 12: r = "(-abs("+r+"))"; break;
  default:
    fprintf(stderr,"d3dshader: unsupported source modifier %d\n",(t>>24)&15);
    break;
  }
  return r;
}

static Dst parseDst(Ctx &c,const sU32 *&p)
{
  sU32 t = *p++;
  if(c.vs && c.ver>=0x200 && (t&0x2000)) p++;
  Dst d;
  d.type = regType(t);
  d.num = regNum(t);
  d.mask = (t>>16)&15;
  d.sat = ((t>>20)&1) ? sTRUE : sFALSE;
  d.shift = (t>>24)&15;
  if(d.shift>=8) d.shift -= 16;
  if(c.vs)
  {
    switch(d.type)
    {
    case 0: d.name = "r"+num(d.num); break;
    case 3: d.name = "a0"; break;
    case 4: d.name = d.num==0 ? "oPos" : (d.num==1 ? "oFog" : "oPts"); break;
    case 5: d.name = "oD"+num(d.num&1); break;
    case 6: d.name = "oT"+num(d.num&7); break;
    default:
      fprintf(stderr,"d3dshader: unsupported vs dest type %d\n",d.type);
      c.ok = sFALSE;
      d.name = "r31";
      break;
    }
  }
  else
  {
    switch(d.type)
    {
    case 0: d.name = "r"+num(d.num); break;
    case 3: d.name = "t"+num(d.num&7); break;
    default:
      fprintf(stderr,"d3dshader: unsupported ps dest type %d\n",d.type);
      c.ok = sFALSE;
      d.name = "r31";
      break;
    }
  }
  return d;
}

static std::string writeStmt(Ctx &c,const Dst &d,const std::string &val)
{
  std::string v = val;
  static const char *scale[] = { "0.125","0.25","0.5","","2.0","4.0","8.0" };
  if(d.shift>=-3 && d.shift<=3 && d.shift!=0)
    v = "("+v+"*"+scale[d.shift+3]+")";
  if(d.sat)
    v = "clamp("+v+",0.0,1.0)";
  if(!c.vs && c.range>0)
    v = "clamp("+v+",-"+flt(c.range)+","+flt(c.range)+")";

  if(c.vs && d.type==3)
  {
    std::string s;
    for(sInt i=0;i<4;i++)
      if(d.mask & (1<<i))
        s += std::string("a0.")+kComp[i]+" = int(floor(("+v+")."+kComp[i]+"+0.5)); ";
    return s;
  }
  if(d.mask==15)
    return d.name+" = "+v+";";
  std::string m = maskStr(d.mask);
  return d.name+"."+m+" = ("+v+")."+m+";";
}

struct Ins
{
  std::string pre;      // compute
  std::string post;     // write back
  sBool coissue;
};

static std::string sampleCall(Ctx &c,sInt stage,const std::string &coord,sBool proj)
{
  stage &= 7;
  c.samplers |= 1u<<stage;
  return std::string(proj ? "SAMPLEP" : "SAMPLE")+num(stage)+"("+coord+")";
}

static sInt attribSlot(sU32 usage,sU32 index)
{
  switch(usage)
  {
  case 0:  return index==0 ? D3DSLOT_POSITION : D3DSLOT_UNUSED;
  case 1:  return D3DSLOT_BLENDW;
  case 2:  return D3DSLOT_BLENDI;
  case 3:  return index==0 ? D3DSLOT_NORMAL : D3DSLOT_UNUSED;
  case 4:  return D3DSLOT_PSIZE;
  case 5:  return index<4 ? D3DSLOT_TEXCOORD0+index : D3DSLOT_UNUSED;
  case 6:  return D3DSLOT_TANGENT;
  case 7:  return D3DSLOT_BINORMAL;
  case 10: return index==0 ? D3DSLOT_COLOR0 : (index==1 ? D3DSLOT_COLOR1 : D3DSLOT_UNUSED);
  }
  return D3DSLOT_UNUSED;
}

} // namespace

sInt D3DShaderLength(const sU32 *tokens)
{
  const sU32 *p = tokens;
  sInt ver = (*p++)&0xffff;
  for(;;)
  {
    sU32 tok = *p;
    sU32 op = tok & 0xffff;
    if(tok==0x0000ffff) { p++; break; }
    if(op==OP_COMMENT) { p += 1+((tok>>16)&0x7fff); continue; }
    if(op==OP_PHASE) { p++; continue; }
    sInt len = (tok>>24)&15;
    if(len==0 && ver<0x200) len = opLength(op,ver);
    if(len<0) { p++; break; }
    p += 1+len;
  }
  return (sInt)(p-tokens);
}

sBool D3DTranslate(const sU32 *tokens,D3DShaderInfo &out)
{
  Ctx c;
  const sU32 *p = tokens;
  sU32 vt = *p++;
  c.vs = (vt>>16)==0xfffe;
  c.ver = vt & 0xffff;
  c.range = 0;
  if(!c.vs)
    c.range = c.ver<0x104 ? 1.0f : (c.ver<0x200 ? 8.0f : 0.0f);
  c.inputs = 0;
  for(sInt i=0;i<16;i++) c.slot[i] = -1;
  c.samplers = 0;
  c.pad = 0;
  c.tmp = 0;
  c.ok = sTRUE;

  // first pass: def constants
  {
    const sU32 *q = p;
    for(;;)
    {
      sU32 tok = *q;
      sU32 op = tok & 0xffff;
      if(tok==0x0000ffff) break;
      if(op==OP_COMMENT) { q += 1+((tok>>16)&0x7fff); continue; }
      if(op==OP_PHASE) { q++; continue; }
      sInt len = (tok>>24)&15;
      if(len==0 && c.ver<0x200) len = opLength(op,c.ver);
      if(len<0) break;
      if(op==OP_DEF)
      {
        sInt n = regNum(q[1]);
        sF32 f[4];
        for(sInt i=0;i<4;i++)
        {
          sU32 u = q[2+i];
          memcpy(&f[i],&u,4);
          if(!c.vs && c.range>0)
            f[i] = f[i]<-c.range ? -c.range : (f[i]>c.range ? c.range : f[i]);
        }
        c.defs[n] = "vec4("+flt(f[0])+","+flt(f[1])+","+flt(f[2])+","+flt(f[3])+")";
      }
      q += 1+len;
    }
  }

  std::vector<Ins> ins;
  std::string padU,padV;

  for(;;)
  {
    sU32 tok = *p;
    sU32 op = tok & 0xffff;
    if(tok==0x0000ffff) break;
    if(op==OP_COMMENT) { p += 1+((tok>>16)&0x7fff); continue; }
    if(op==OP_PHASE) { p++; continue; }
    sInt len = (tok>>24)&15;
    if(len==0 && c.ver<0x200) len = opLength(op,c.ver);
    if(len<0)
    {
      fprintf(stderr,"d3dshader: unknown opcode %u\n",op);
      c.ok = sFALSE;
      break;
    }
    const sU32 *next = p+1+len;
    const sU32 *a = p+1;
    Ins in;
    in.coissue = (tok & 0x40000000) ? sTRUE : sFALSE;
    std::string t = "_t"+num(c.tmp++);

    auto arith = [&](sInt nsrc,auto fn)
    {
      Dst d = parseDst(c,a);
      std::string s[4];
      for(sInt i=0;i<nsrc;i++) s[i] = srcExpr(c,a);
      in.pre = "vec4 "+t+" = "+fn(s)+";";
      in.post = writeStmt(c,d,t);
    };
    auto matrix = [&](sInt cols,sInt rows)
    {
      Dst d = parseDst(c,a);
      std::string v = srcExpr(c,a);
      std::string r[4];
      for(sInt i=0;i<rows;i++) { const sU32 *b = a; r[i] = srcExpr(c,b,i); }
      std::string e = "vec4(";
      for(sInt i=0;i<4;i++)
      {
        if(i) e += ",";
        if(i<rows)
          e += cols==4 ? "dot("+v+","+r[i]+")" : "dot(("+v+").xyz,("+r[i]+").xyz)";
        else
          e += "0.0";
      }
      e += ")";
      in.pre = "vec4 "+t+" = "+e+";";
      in.post = writeStmt(c,d,t);
    };

    switch(op)
    {
    case OP_NOP:
      break;
    case OP_DCL:
      if(c.vs)
      {
        sU32 u = a[0];
        sU32 r = a[1];
        if(regType(r)==1)
          c.slot[regNum(r)&15] = attribSlot(u&0x1f,(u>>16)&15);
      }
      break;
    case OP_DEF:
      break;
    case OP_MOV:  arith(1,[](std::string *s){ return s[0]; }); break;
    case OP_MOVA: arith(1,[](std::string *s){ return s[0]; }); break;
    case OP_ADD:  arith(2,[](std::string *s){ return "("+s[0]+"+"+s[1]+")"; }); break;
    case OP_SUB:  arith(2,[](std::string *s){ return "("+s[0]+"-"+s[1]+")"; }); break;
    case OP_MUL:  arith(2,[](std::string *s){ return "("+s[0]+"*"+s[1]+")"; }); break;
    case OP_MAD:  arith(3,[](std::string *s){ return "("+s[0]+"*"+s[1]+"+"+s[2]+")"; }); break;
    case OP_RCP:  arith(1,[](std::string *s){ return "vec4(rcpf(("+s[0]+").w))"; }); break;
    case OP_RSQ:  arith(1,[](std::string *s){ return "vec4(rsqf(("+s[0]+").w))"; }); break;
    case OP_DP3:  arith(2,[](std::string *s){ return "vec4(dot(("+s[0]+").xyz,("+s[1]+").xyz))"; }); break;
    case OP_DP4:  arith(2,[](std::string *s){ return "vec4(dot("+s[0]+","+s[1]+"))"; }); break;
    case OP_MIN:  arith(2,[](std::string *s){ return "min("+s[0]+","+s[1]+")"; }); break;
    case OP_MAX:  arith(2,[](std::string *s){ return "max("+s[0]+","+s[1]+")"; }); break;
    case OP_SLT:  arith(2,[](std::string *s){ return "vec4(lessThan("+s[0]+","+s[1]+"))"; }); break;
    case OP_SGE:  arith(2,[](std::string *s){ return "vec4(greaterThanEqual("+s[0]+","+s[1]+"))"; }); break;
    case OP_EXP:  arith(1,[](std::string *s){ return "vec4(exp2(("+s[0]+").w))"; }); break;
    case OP_LOG:  arith(1,[](std::string *s){ return "vec4(logf(("+s[0]+").w))"; }); break;
    case OP_EXPP:
      if(c.ver<0x200) arith(1,[](std::string *s){ return "expp1(("+s[0]+").w)"; });
      else arith(1,[](std::string *s){ return "vec4(exp2(("+s[0]+").w))"; });
      break;
    case OP_LOGP:
      if(c.ver<0x200) arith(1,[](std::string *s){ return "logp1(("+s[0]+").w)"; });
      else arith(1,[](std::string *s){ return "vec4(logf(("+s[0]+").w))"; });
      break;
    case OP_LIT:  arith(1,[](std::string *s){ return "litf("+s[0]+")"; }); break;
    case OP_DST:  arith(2,[](std::string *s){ return "vec4(1.0,("+s[0]+").y*("+s[1]+").y,("+s[0]+").z,("+s[1]+").w)"; }); break;
    case OP_LRP:  arith(3,[](std::string *s){ return "mix("+s[2]+","+s[1]+","+s[0]+")"; }); break;
    case OP_FRC:  arith(1,[](std::string *s){ return "fract("+s[0]+")"; }); break;
    case OP_POW:  arith(2,[](std::string *s){ return "vec4(pow(abs(("+s[0]+").w),("+s[1]+").w))"; }); break;
    case OP_CRS:  arith(2,[](std::string *s){ return "vec4(cross(("+s[0]+").xyz,("+s[1]+").xyz),0.0)"; }); break;
    case OP_ABS:  arith(1,[](std::string *s){ return "abs("+s[0]+")"; }); break;
    case OP_SGN:  arith(1,[](std::string *s){ return "sign("+s[0]+")"; }); break;
    case OP_NRM:  arith(1,[](std::string *s){ return "("+s[0]+"*rsqf(dot(("+s[0]+").xyz,("+s[0]+").xyz)))"; }); break;
    case OP_SINCOS: arith(1,[](std::string *s){ return "vec4(cos(("+s[0]+").w),sin(("+s[0]+").w),0.0,0.0)"; }); break;
    case OP_CND:
      arith(3,[](std::string *s){ return "mix("+s[2]+","+s[1]+",vec4(greaterThan("+s[0]+",vec4(0.5))))"; });
      break;
    case OP_CMP:
      arith(3,[](std::string *s){ return "mix("+s[2]+","+s[1]+",vec4(greaterThanEqual("+s[0]+",vec4(0.0))))"; });
      break;
    case OP_DP2ADD:
      arith(3,[](std::string *s){ return "vec4(dot(("+s[0]+").xy,("+s[1]+").xy)+("+s[2]+").x)"; });
      break;
    case OP_M4x4: matrix(4,4); break;
    case OP_M4x3: matrix(4,3); break;
    case OP_M3x4: matrix(3,4); break;
    case OP_M3x3: matrix(3,3); break;
    case OP_M3x2: matrix(3,2); break;

    case OP_BEM:
      {
        Dst d = parseDst(c,a);
        std::string s0 = srcExpr(c,a), s1 = srcExpr(c,a);
        sInt n = d.num&7;
        in.pre = "vec4 "+t+" = vec4(("+s0+").x+uBump["+num(n)+"].x*("+s1+").x+uBump["+num(n)+"].z*("+s1+").y,"
                 "("+s0+").y+uBump["+num(n)+"].y*("+s1+").x+uBump["+num(n)+"].w*("+s1+").y,0.0,0.0);";
        in.post = writeStmt(c,d,t);
      }
      break;

    case OP_TEXCOORD:
      if(c.ver>=0x104)
      {
        Dst d = parseDst(c,a);
        std::string s = srcExpr(c,a);
        in.pre = "vec4 "+t+" = vec4(("+s+").xyz,1.0);";
        in.post = writeStmt(c,d,t);
      }
      else
      {
        Dst d = parseDst(c,a);
        sInt n = d.num&7;
        in.pre = "vec4 "+t+" = vec4(clamp(xT"+num(n)+".xyz,0.0,1.0),1.0);";
        in.post = writeStmt(c,d,t);
      }
      break;

    case OP_TEXKILL:
      {
        sU32 r = a[0];
        sInt n = regNum(r)&7;
        std::string v = (c.ver>=0x104 && regType(r)==0) ? "r"+num(n) : "xT"+num(n);
        in.pre = "if(any(lessThan("+v+".xyz,vec3(0.0)))) discard;";
      }
      break;

    case OP_TEX:
      if(c.ver>=0x104)
      {
        Dst d = parseDst(c,a);
        std::string s = srcExpr(c,a);
        sInt stage = d.num;
        if(c.ver>=0x200) stage = regNum(*a++);
        in.pre = "vec4 "+t+" = "+sampleCall(c,stage,s,sFALSE)+";";
        in.post = writeStmt(c,d,t);
      }
      else
      {
        Dst d = parseDst(c,a);
        sInt n = d.num&7;
        in.pre = "vec4 "+t+" = "+sampleCall(c,n,"xT"+num(n),sTRUE)+";";
        in.post = writeStmt(c,d,t);
      }
      break;

    case OP_TEXBEM:
    case OP_TEXBEML:
      {
        Dst d = parseDst(c,a);
        std::string s = srcExpr(c,a);
        sInt n = d.num&7;
        std::string b = "uBump["+num(n)+"]";
        std::string uv = "vec4(xT"+num(n)+".x+"+b+".x*("+s+").x+"+b+".z*("+s+").y,"
                         "xT"+num(n)+".y+"+b+".y*("+s+").x+"+b+".w*("+s+").y,0.0,1.0)";
        in.pre = "vec4 "+t+" = "+sampleCall(c,n,uv,sFALSE)+";";
        if(op==OP_TEXBEML)
          in.pre += " "+t+".xyz *= clamp(("+s+").z*uBumpL["+num(n)+"].x+uBumpL["+num(n)+"].y,0.0,1.0);";
        in.post = writeStmt(c,d,t);
      }
      break;

    case OP_TEXREG2AR:
    case OP_TEXREG2GB:
    case OP_TEXREG2RGB:
      {
        Dst d = parseDst(c,a);
        std::string s = srcExpr(c,a);
        sInt n = d.num&7;
        std::string uv = op==OP_TEXREG2AR ? "vec4(("+s+").w,("+s+").x,0.0,1.0)"
                       : op==OP_TEXREG2GB ? "vec4(("+s+").y,("+s+").z,0.0,1.0)"
                       : "vec4(("+s+").xyz,1.0)";
        in.pre = "vec4 "+t+" = "+sampleCall(c,n,uv,sFALSE)+";";
        in.post = writeStmt(c,d,t);
      }
      break;

    case OP_TEXM3x2PAD:
    case OP_TEXM3x3PAD:
      {
        Dst d = parseDst(c,a);
        std::string s = srcExpr(c,a);
        sInt n = d.num&7;
        std::string v = "_p"+num(c.tmp++);
        in.pre = "float "+v+" = dot(xT"+num(n)+".xyz,("+s+").xyz);";
        if(c.pad==0) padU = v; else padV = v;
        c.pad++;
      }
      break;

    case OP_TEXM3x2TEX:
    case OP_TEXM3x2DEPTH:
    case OP_TEXDP3TEX:
    case OP_TEXDP3:
    case OP_TEXM3x3TEX:
    case OP_TEXM3x3:
    case OP_TEXM3x3SPEC:
    case OP_TEXM3x3VSPEC:
    case OP_TEXM3x3DIFF:
      {
        Dst d = parseDst(c,a);
        std::string s = srcExpr(c,a);
        sInt n = d.num&7;
        std::string w = "dot(xT"+num(n)+".xyz,("+s+").xyz)";
        std::string e;
        switch(op)
        {
        case OP_TEXM3x2TEX:
          e = sampleCall(c,n,"vec4("+padU+","+w+",0.0,1.0)",sFALSE);
          break;
        case OP_TEXM3x2DEPTH:
          e = "vec4("+padU+"/max("+w+",1e-6))";
          break;
        case OP_TEXDP3TEX:
          e = sampleCall(c,n,"vec4("+w+",0.0,0.0,1.0)",sFALSE);
          break;
        case OP_TEXDP3:
          e = "vec4("+w+")";
          break;
        case OP_TEXM3x3TEX:
        case OP_TEXM3x3DIFF:
          e = sampleCall(c,n,"vec4("+padU+","+padV+","+w+",1.0)",sFALSE);
          break;
        case OP_TEXM3x3:
          e = "vec4("+padU+","+padV+","+w+",1.0)";
          break;
        case OP_TEXM3x3SPEC:
        case OP_TEXM3x3VSPEC:
          {
            std::string eye = op==OP_TEXM3x3SPEC ? "("+srcExpr(c,a)+").xyz"
              : "vec3(xT"+num((n-2)&7)+".w,xT"+num((n-1)&7)+".w,xT"+num(n)+".w)";
            e = sampleCall(c,n,"vec4(reflspec(vec3("+padU+","+padV+","+w+"),"+eye+"),1.0)",sFALSE);
          }
          break;
        }
        in.pre = "vec4 "+t+" = "+e+";";
        in.post = writeStmt(c,d,t);
        c.pad = 0;
      }
      break;

    case OP_TEXDEPTH:
      break;

    default:
      fprintf(stderr,"d3dshader: unsupported opcode %u\n",op);
      c.ok = sFALSE;
      break;
    }
    if(!in.pre.empty() || !in.post.empty())
      ins.push_back(in);
    p = next;
  }

  // emit, keeping co-issued pairs reading the same inputs
  std::string body;
  for(size_t i=0;i<ins.size();i++)
  {
    if(i+1<ins.size() && ins[i+1].coissue)
    {
      body += "  "+ins[i].pre+"\n  "+ins[i+1].pre+"\n  "+ins[i].post+"\n  "+ins[i+1].post+"\n";
      i++;
    }
    else
      body += "  "+ins[i].pre+"\n  "+ins[i].post+"\n";
  }

  std::string defs;
  for(auto &d : c.defs)
    defs += "  const vec4 k"+num(d.first)+" = "+d.second+";\n";

  out.Vertex = c.vs;
  out.Version = c.ver;
  out.Ok = c.ok;
  out.Body = body;
  out.Defs = defs;
  out.InputsUsed = c.inputs;
  for(sInt i=0;i<16;i++) out.InputSlot[i] = c.slot[i];
  out.SamplersUsed = c.samplers;
  return c.ok;
}

static std::string varyings(const char *qual)
{
  static const char *names[] = { "vec4 xD0","vec4 xD1","vec4 xT0","vec4 xT1","vec4 xT2","vec4 xT3",
                                 "vec4 xT4","vec4 xT5","vec4 xT6","vec4 xT7","float xFog" };
  std::string s;
  for(auto n : names) s += std::string(qual)+" "+n+";\n";
  return s;
}

static const char *kHelpers =
  "float rcpf(float x){ return x==0.0 ? 3.0e38 : 1.0/x; }\n"
  "float rsqf(float x){ x=abs(x); return x==0.0 ? 3.0e38 : inversesqrt(x); }\n"
  "float logf(float x){ x=abs(x); return x==0.0 ? -3.0e38 : log2(x); }\n"
  "vec4 expp1(float x){ float f=floor(x); return vec4(exp2(f),x-f,exp2(x),1.0); }\n"
  "vec4 logp1(float x){ x=abs(x); if(x==0.0) return vec4(-3.0e38,1.0,-3.0e38,1.0);\n"
  "  float e=floor(log2(x)); return vec4(e,x/exp2(e),log2(x),1.0); }\n"
  "vec4 litf(vec4 s){ vec4 r=vec4(1.0,0.0,0.0,1.0);\n"
  "  if(s.x>0.0){ r.y=s.x; float p=clamp(s.w,-127.9961,127.9961);\n"
  "    if(s.y>0.0) r.z=exp2(p*log2(s.y)); else if(s.y==0.0 && p==0.0) r.z=1.0; }\n"
  "  return r; }\n";

std::string D3DBuildVS(const D3DShaderInfo &vs)
{
  std::string s = "#version 150\n";
  s += "uniform vec4 vc[256];\n";
  s += "uniform vec4 uPosFix;\n";
  for(sInt i=0;i<16;i++)
    if((vs.InputsUsed & (1u<<i)) && vs.InputSlot[i]>=0 && vs.InputSlot[i]!=D3DSLOT_UNUSED)
      s += "in vec4 v"+num(i)+";\n";
  s += varyings("out");
  s += kHelpers;
  s += "void main(){\n";
  for(sInt i=0;i<16;i++)
    if((vs.InputsUsed & (1u<<i)) && !(vs.InputSlot[i]>=0 && vs.InputSlot[i]!=D3DSLOT_UNUSED))
      s += "  vec4 v"+num(i)+" = vec4(0.0,0.0,0.0,1.0);\n";
  s += "  vec4 r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15;\n";
  s += "  vec4 r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31;\n";
  s += "  r0=r1=r2=r3=r4=r5=r6=r7=r8=r9=r10=r11=r12=r13=r14=r15=vec4(0.0);\n";
  s += "  r16=r17=r18=r19=r20=r21=r22=r23=r24=r25=r26=r27=r28=r29=r30=r31=vec4(0.0);\n";
  s += "  ivec4 a0 = ivec4(0);\n";
  s += "  vec4 oPos=vec4(0.0,0.0,0.0,1.0), oFog=vec4(1.0), oPts=vec4(1.0);\n";
  s += "  vec4 oD0=vec4(1.0), oD1=vec4(0.0);\n";
  s += "  vec4 oT0,oT1,oT2,oT3,oT4,oT5,oT6,oT7;\n";
  s += "  oT0=oT1=oT2=oT3=oT4=oT5=oT6=oT7=vec4(0.0,0.0,0.0,1.0);\n";
  s += vs.Defs;
  s += vs.Body;
  s += "  xD0=oD0; xD1=oD1; xT0=oT0; xT1=oT1; xT2=oT2; xT3=oT3;\n";
  s += "  xT4=oT4; xT5=oT5; xT6=oT6; xT7=oT7; xFog=oFog.x;\n";
  s += "  vec4 p = oPos;\n";
  s += "  p.x += uPosFix.x*p.w;\n";
  s += "  p.y -= uPosFix.y*p.w;\n";
  s += "  p.y *= uPosFix.z;\n";
  s += "  p.z = 2.0*p.z - p.w;\n";
  s += "  gl_Position = p;\n";
  s += "}\n";
  return s;
}

static const char *kPSCommon =
  "uniform vec4 uFogColor;\n"     // rgb, a=enable
  "uniform vec4 uAlphaTest;\n"    // x=enable y=ref z=func
  "out vec4 fragColor;\n"
  "bool alphaPass(float a){\n"
  "  float v=floor(clamp(a,0.0,1.0)*255.0+0.5); float r=uAlphaTest.y; int f=int(uAlphaTest.z);\n"
  "  if(f==1) return false; if(f==2) return v<r; if(f==3) return v==r; if(f==4) return v<=r;\n"
  "  if(f==5) return v>r; if(f==6) return v!=r; if(f==7) return v>=r; return true; }\n"
  "void finish(vec4 c){\n"
  "  c=clamp(c,0.0,1.0);\n"
  "  if(uAlphaTest.x>0.5 && !alphaPass(c.a)) discard;\n"
  "  if(uFogColor.a>0.5) c.rgb=mix(uFogColor.rgb,c.rgb,clamp(xFog,0.0,1.0));\n"
  "#ifdef KK_FLAT\n"
  "  c=vec4(0.25);\n"
  "#endif\n"
  "  fragColor=c; }\n";

static std::string samplerDecls(sU32 used,sU32 kinds,sU32 proj)
{
  std::string s;
  for(sInt i=0;i<8;i++)
  {
    if(!(used & (1u<<i))) continue;
    sU32 k = (kinds>>(2*i))&3;
    std::string n = num(i);
    if(k==D3DTEXKIND_CUBE)
    {
      s += "uniform samplerCube s"+n+";\n";
      s += "#define SAMPLE"+n+"(c) texture(s"+n+",(c).xyz)\n";
      s += "#define SAMPLEP"+n+"(c) texture(s"+n+",(c).xyz)\n";
    }
    else if(k==D3DTEXKIND_3D)
    {
      s += "uniform sampler3D s"+n+";\n";
      s += "#define SAMPLE"+n+"(c) texture(s"+n+",(c).xyz)\n";
      s += "#define SAMPLEP"+n+"(c) texture(s"+n+",(c).xyz)\n";
    }
    else
    {
      s += "uniform sampler2D s"+n+";\n";
      s += "#define SAMPLE"+n+"(c) texture(s"+n+",(c).xy)\n";
      if(proj & (1u<<i))
        s += "#define SAMPLEP"+n+"(c) texture(s"+n+",(c).xy/(c).w)\n";
      else
        s += "#define SAMPLEP"+n+"(c) texture(s"+n+",(c).xy)\n";
    }
  }
  return s;
}

std::string D3DBuildPS(const D3DShaderInfo *ps,sU32 kinds,sU32 proj)
{
  std::string s = "#version 150\n";
  if(getenv("KK_FLAT")) s += "#define KK_FLAT 1\n";
  s += varyings("in");
  s += kPSCommon;

  if(!ps)
  {
    // fixed function texture stages 0 and 1
    s += samplerDecls(3,kinds&15,proj&3);
    s += "uniform ivec4 uCOp[2];\n";   // colorop, colorarg1, colorarg2, alphaop
    s += "uniform ivec4 uAOp[2];\n";   // alphaarg1, alphaarg2, has texture
    s += "uniform vec4 uTFactor;\n";
    s += "vec4 v0, v1;\n";
    s += "vec4 targ(int a,vec4 cur,vec4 tex){ int s=a&15; vec4 v=cur;\n"
         "  if(s==0) v=v0; else if(s==2) v=tex; else if(s==3) v=uTFactor; else if(s==4) v=v1;\n"
         "  if((a&16)!=0) v=1.0-v; if((a&32)!=0) v=vec4(v.a); return v; }\n";
    s += "vec4 top(int o,vec4 a1,vec4 a2,vec4 cur,vec4 tex){\n"
         "  if(o==2) return a1; if(o==3) return a2; if(o==4) return a1*a2;\n"
         "  if(o==5) return 2.0*a1*a2; if(o==6) return 4.0*a1*a2; if(o==7) return a1+a2;\n"
         "  if(o==8) return a1+a2-0.5; if(o==9) return 2.0*(a1+a2-0.5); if(o==10) return a1-a2;\n"
         "  if(o==11) return a1+a2-a1*a2; if(o==12) return mix(a2,a1,v0.a);\n"
         "  if(o==13) return mix(a2,a1,tex.a); if(o==14) return mix(a2,a1,uTFactor.a);\n"
         "  if(o==15) return a1+a2*(1.0-tex.a); if(o==16) return mix(a2,a1,cur.a);\n"
         "  if(o==24) return vec4(clamp(4.0*dot(a1.rgb-0.5,a2.rgb-0.5),0.0,1.0));\n"
         "  return a1; }\n";
    s += "void main(){\n";
    s += "  v0=clamp(xD0,0.0,1.0); v1=clamp(xD1,0.0,1.0);\n";
    s += "  vec4 cur=v0;\n";
    s += "  for(int i=0;i<2;i++){\n";
    s += "    if(uCOp[i].x==1) break;\n";
    s += "    vec4 tex = i==0 ? SAMPLEP0(xT0) : SAMPLEP1(xT1);\n";
    s += "    vec4 c=top(uCOp[i].x,targ(uCOp[i].y,cur,tex),targ(uCOp[i].z,cur,tex),cur,tex);\n";
    s += "    float a=cur.a;\n";
    s += "    if(uCOp[i].w!=1) a=top(uCOp[i].w,targ(uAOp[i].x,cur,tex),targ(uAOp[i].y,cur,tex),cur,tex).a;\n";
    s += "    cur=clamp(vec4(c.rgb,a),0.0,1.0);\n";
    s += "  }\n";
    s += "  finish(cur);\n";
    s += "}\n";
    return s;
  }

  s += "uniform vec4 pc[32];\n";
  s += "uniform vec4 uBump[8];\n";
  s += "uniform vec4 uBumpL[8];\n";
  s += samplerDecls(ps->SamplersUsed,kinds,proj);
  s += kHelpers;
  s += "vec4 dz(vec4 v){ return vec4(v.xy/v.z,v.z,v.w); }\n";
  s += "vec4 dw(vec4 v){ return vec4(v.xyz/v.w,v.w); }\n";
  s += "vec3 reflspec(vec3 n,vec3 e){ return 2.0*n*dot(n,e)/dot(n,n)-e; }\n";
  s += "void main(){\n";
  s += "  vec4 v0=clamp(xD0,0.0,1.0), v1=clamp(xD1,0.0,1.0);\n";
  s += "  vec4 r0,r1,r2,r3,r4,r5,r6,r7;\n";
  s += "  r0=r1=r2=r3=r4=r5=r6=r7=vec4(0.0);\n";
  s += "  vec4 t0,t1,t2,t3,t4,t5,t6,t7;\n";
  s += "  t0=t1=t2=t3=t4=t5=t6=t7=vec4(0.0);\n";
  s += ps->Defs;
  s += ps->Body;
  s += "  finish(r0);\n";
  s += "}\n";
  return s;
}
