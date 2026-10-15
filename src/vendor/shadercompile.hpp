#ifndef __SHADERCOMPILE_HPP__
#define __SHADERCOMPILE_HPP__

class sShaderCompiler
{
public:
  sBool Compile(sChar *, const sChar *, class sText *) { return sFALSE; }
  const sU32 *GetShaderCopy() { return 0; }
};

#endif
