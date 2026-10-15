# .kkrieger for macOS

Native source port of [.kkrieger](https://en.wikipedia.org/wiki/.kkrieger) (Farbrausch / .theprodukkt, Breakpoint 2004).

The 96 KB Windows `pno0001.exe` is a packed Direct3D 9 player. This tree rebuilds the game from Farbrausch's **BSD-licensed** player (`werkkzeug3_kkrieger`) plus the exported level `data/kkrieger.kx`, with Win32/D3D9 replaced by **SDL2 + OpenGL 3.2** on macOS.

## Build

```bash
brew install sdl2
make -j
./build/kkrieger data/kkrieger.kx
```

## Controls

Same as the original: WASD, mouse look, left mouse shoot, space jump, 1–5 weapons, Shift use, Esc quit.

## Status

Wait until the console prints `ready` (intro precalc is a few seconds). The window shows a loading bar, then the 3D view. `loading 152/160` is a second, shorter precalc (gameplay root), not a hang. The renderer is an OpenGL stand-in for D3D9 (not pixel-accurate SM 1.4).

Original code: [farbrausch/fr_public](https://github.com/farbrausch/fr_public) (BSD).


[You have received this identical output 3 times. Re-reading '/Users/alfredo.gore/Downloads/kkrieger/README.md:raw' will not change it — use a narrower selector (path:A-B), or proceed with the edit.]

[You have received this identical output 3 times. Re-reading '/Users/alfredo.gore/Downloads/kkrieger/README.md:raw' will not change it — use a narrower selector (path:A-B), or proceed with the edit.]