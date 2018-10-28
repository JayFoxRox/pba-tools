// Converts 002C files to a obj (which contains the collision)
//
// (C) 2017 Jannik Vogel
//
// Licensed under GPLv2 or later.
// See LICENSE.txt for more information.

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <libgen.h>

typedef struct {
  float x;
  float y;
  float z;
  float radius; // ?!
  uint32_t unkCount;
  uint32_t always0[2];
} Header;

typedef struct {
  uint32_t unk0; // anything from zero, nan, huge and small values, flags or 0xFFFFFFFF

  struct {
    float x;
    float y;
    float z;
  } face0Center; // Vertex coords
  float unk1; // Large values?!  [Squared radius of bounding sphere?!]

  struct {
    struct {
      float x;
      float y;
      float z;
    } normal;
    float distance; // Distance along normal from origin
  } plane;

  // Probably some rotation matrix of some kind?!
  float unk4[3*3];

  // 6 Vertices which make up 2 faces (3 for each)
  // Only the first of these faces is used though?!
  struct {
    struct {
      float x;
      float y;
      float z;
    } vertex[3];
  } faces[2];

  uint32_t always0;
} Unk1;

#if 0
0.000000 -0.000000 -1.000000 1
0.828400 -0.560138 -0.000000 ~1
-0.221253 0.149604  0.963673 <1
#endif

float f = 0.001;

int main(int argc, char* argv[]) {
  char* outPath = argv[2];
  FILE* in = fopen(argv[1], "rb");
  FILE* out = fopen(outPath, "w");

  Header header;
  fread(&header, sizeof(header), 1, in);
  printf("unk: %f %f %f %f\n", header.x, header.y, header.z, header.radius);
  printf("unkCount: %d\n", header.unkCount);

  printf("bpy.ops.object.empty_add(type='SPHERE', radius=%f, view_align=False, location=(%f, %f, %f), layers=(True, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False))\n",
         sqrtf(header.radius) * f,
         header.x * f, header.y * f, header.z * f);


  assert(header.always0[0] == 0);
  assert(header.always0[1] == 0);
/*
    Point point;
    fread(&point, sizeof(point), 1, in);
    printf("%d point: { %f %f %f }\n", x, point.x, point.y, point.z);
    fprintf(out, "v %f %f %f\n", point.x, point.y, point.z);
*/

  static int v = 1;
  for(unsigned int i = 0; i < header.unkCount; i++) {
    Unk1 unk;
    fread(&unk, sizeof(unk), 1, in);
    printf("[%d] =\n", i);
    printf("\tunk0: 0x%08X (%f)\n", unk.unk0, *(float*)&unk.unk0);

    // Bounding sphere [for faces[0] only]?!
    printf("\tface0Center: { %f %f %f }\n", unk.face0Center.x, unk.face0Center.y, unk.face0Center.z);
    fprintf(out, "v %f %f %f\n", unk.face0Center.x, unk.face0Center.y, unk.face0Center.z); // Face center?!
    v++;
    printf("\tunk1: %f\n", unk.unk1);

    //FIXME: Not sure about this interpretation yet.. it's slightly too high..
    printf("bpy.ops.object.empty_add(type='SPHERE', radius=%f, view_align=False, location=(%f, %f, %f), layers=(True, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False))\n",
           sqrtf(unk.unk1) * f,
           unk.face0Center.x * f, unk.face0Center.y * f, unk.face0Center.z * f);

    printf("\tplane: { %f %f %f } %f\n", unk.plane.normal.x, unk.plane.normal.y, unk.plane.normal.z, unk.plane.distance);

    //FIXME: k and j inverted!
    for(unsigned int k = 0; k < 2; k++) {
      printf("\tfaces[%d]", k);
      for(unsigned int j = 0; j < 3; j++) {
        printf(" { %f %f %f }", unk.faces[k].vertex[j].x, unk.faces[k].vertex[j].y, unk.faces[k].vertex[j].z);
        fprintf(out, "v %f %f %f\n", unk.faces[k].vertex[j].x, unk.faces[k].vertex[j].y, unk.faces[k].vertex[j].z);
        v++;
      }
      printf("\n");
    }
    // There is another valid face but we don't use it at the moment
    fprintf(out, "f %d %d %d\n", v - 4, v - 5, v - 6);

    for(unsigned int j = 0; j < 3; j++) {
      printf("\t\tunk4[%d]: %f %f %f\n", j, unk.unk4[j*3+0], unk.unk4[j*3+1], unk.unk4[j*3+2]);
    }
//    fprintf(out, "f %d %d %d\n", v - 1, v - 2, v - 3);

    fprintf(out, "\n");
    printf("\n");

    //assert(unk.unk0 == 0 || unk.unk0 == 1);
    assert(unk.always0 == 0);
  }

  while(!feof(in)) {
    size_t x = ftell(in);
    uint32_t u;
    fread(&u,4,1,in);

    printf("%d:\t0x%08X (%f)\n",
           x,
           u, *(float*)&u);
  }

  fclose(in);
  fclose(out);
  
  return 0;
}
