#include <stdlib.h>
#include <string.h>

extern "C" unsigned int v2synthGetSize();
extern "C" void v2synthInit(void *pthis, const void *patchmap, int samplerate);
extern "C" void v2synthRender(void *pthis, void *buf, int smp, void *buf2, int add);
extern "C" void v2synthProcessMIDI(void *pthis, const void *ptr);
extern "C" void v2synthSetGlobals(void *pthis, const void *ptr);
extern "C" long v2synthGetFrameSize(void *pthis);

static void *g_synth;

extern "C" void synthInit(const void *patchmap, int samplerate)
{
  if(!g_synth)
    g_synth = calloc(1, v2synthGetSize());
  v2synthInit(g_synth, patchmap, samplerate);
}

extern "C" void synthRender(void *buf, int smp, void *buf2, int add)
{
  if(g_synth)
    v2synthRender(g_synth, buf, smp, buf2, add);
  else if(buf)
    memset(buf, 0, smp * 2 * sizeof(float));
  (void)buf2; (void)add;
}

extern "C" void synthProcessMIDI(const void *ptr)
{
  if(g_synth) v2synthProcessMIDI(g_synth, ptr);}

extern "C" void synthSetGlobals(const void *ptr)
{
  if(g_synth) v2synthSetGlobals(g_synth, ptr);
}

extern "C" long synthGetFrameSize()
{
  return g_synth ? v2synthGetFrameSize(g_synth) : 128;
}

extern "C" void synthGetPoly(void *dest) { if(dest) memset(dest, 0, 64); }
extern "C" void synthGetPgm(void *dest) { if(dest) memset(dest, 0, 64); }
