// This file is distributed under a BSD license. See LICENSE.txt for details.

#include "kdoc.hpp"
#include "genbitmap.hpp"
#include "genmaterial.hpp"
#include "genmesh.hpp"
#include "genoverlay.hpp"
#include "genscene.hpp"
#include "geneffect.hpp"
#include "kkriegergame.hpp"

 
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
