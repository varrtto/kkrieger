#include "genminmesh.hpp"
#include <stdlib.h>

extern "C" char *_fcvt(double value,int count,int *dec,int *sign) { return fcvt(value,count,dec,sign); }
extern "C" char *_ecvt(double value,int count,int *dec,int *sign) { return ecvt(value,count,dec,sign); }

sBool IntroStereo3D;

void GenMinMesh::CalcBBox(sAABox &box) const
{
  box.Min.Init(0,0,0,1);
  box.Max.Init(0,0,0,1);
}

void GenMinMesh::Prepare()
{
}

class KOp;
KOp *gDbgOpStack[64];
sInt gDbgOpDepth;

#include <map>
#include <vector>
#include <stdio.h>
#include <math.h>
static std::map<void*,std::vector<sVector> > gDbgColl,gDbgExec;
void DbgCollScenePos(void *s,const sVector &p){ if(gDbgColl[s].size()<64) gDbgColl[s].push_back(p); }
void DbgExecScenePos(void *s,const sVector &p)
{
  auto &v=gDbgExec[s]; if(v.size()>=64) return;
  for(auto &q:v) if(fabsf(q.x-p.x)<1e-3f&&fabsf(q.y-p.y)<1e-3f&&fabsf(q.z-p.z)<1e-3f) return;
  v.push_back(p);
}
void DbgDumpScenePos()
{
  FILE *f=fopen("/tmp/kk_scenepos.txt","w");
  for(auto &c:gDbgColl)
  {
    auto &e=gDbgExec[c.first];
    fprintf(f,"scene %p coll=%zu exec=%zu\n",c.first,c.second.size(),e.size());
    for(auto &p:c.second) fprintf(f,"  C %.2f %.2f %.2f\n",p.x,p.y,p.z);
    for(auto &p:e) fprintf(f,"  E %.2f %.2f %.2f\n",p.x,p.y,p.z);
  }
  fclose(f);
}
