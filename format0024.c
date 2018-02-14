// Converts 0024 files to obj / mtl

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgen.h>

typedef struct {
  float x;
  float y;
  float z;
} Vector;

typedef struct {
  uint32_t unk0; // 4163 in most files, 4161 in BK2k main-playfield model
// unk0: 4099   DX11 [vertex, uv]
// unk0: 4163   DX9 [vertex, normal, uv]
//       0x40 = normal ??


  uint32_t textureCount;
  uint32_t always0_a;
  uint32_t vertexCount;
  uint32_t always0_b;
  uint32_t unk1; // No idea.. flags of some sorts? [0x00002001]
// 0x64001402   DX11 [vertex, uv]
// 0x00002001   DX9 [vertex, normal, uv]
//       XX << stride / size in bytes?!
//         YY << ??
// 


  uint32_t faceCount; // triangle count without degenerates ?!
  uint32_t always0_c[4];
  uint32_t unk[16+38-7-2];
  uint32_t drawCount;
  uint32_t unk2;
  uint32_t indexCount;
  uint32_t always0_d[7];
} Header;

int main(int argc, char* argv[]) {
  char* outPath = argv[2];
  FILE* in = fopen(argv[1], "rb");
  FILE* out = fopen(outPath, "wb");

  char* mtlPath = alloca(strlen(outPath) + 4);
  sprintf(mtlPath, "%s.mtl", outPath);
  char* mtlFile = basename(mtlPath);

  fprintf(out, "mtllib ./%s\n", mtlFile);

  Header header;
  fread(&header, sizeof(header), 1, in);
  printf("%d vertices\n", header.vertexCount);
  printf("drawCount: %d\n", header.drawCount);
  printf("faces: %d\n", header.faceCount);
  printf("0[a]: %d\n", header.always0_a);
  printf("0[b]: %d\n", header.always0_b);
  for(unsigned int i = 0; i < 4; i++) {
    printf("0[c%d]: %d\n", i, header.always0_c[i]);
  }
  for(unsigned int i = 0; i < 7; i++) {
    printf("0[d%d]: %d\n", i, header.always0_d[i]);
  }
  printf("%d indices\n", header.indexCount);

  uint32_t d;

  for(unsigned int i = 0; i < sizeof(header.unk) / 4; i++) {
    d = header.unk[i];
    printf("[%d] = 0x%08X [%d]\t\t%f\n", i, d, d, *(float*)&d); // Unknown
  }
//  assert(header.unk[45] == header.drawCount);

  bool hasNormal = false; // atoi(argv[3]);

  // FIXME: Figure out a pattern here...
  hasNormal |= (header.unk0 == 4161 && header.unk1 == 0x00002001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x00002001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x00642001);
  hasNormal |= (header.unk0 == 4161 && header.unk1 == 0x1E192001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x1E3C2001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x0F192001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x0F142001);
  hasNormal |= (header.unk0 == 4291 && header.unk1 == 0x00002001);
  hasNormal |= (header.unk0 ==69699 && header.unk1 == 0x00002001);
  hasNormal |= (header.unk0 == 4162 && header.unk1 == 0x00002001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x460A2001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x0F0F2001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x0F232001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x00142001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x05232001);
  hasNormal |= (header.unk0 == 4161 && header.unk1 == 0x1E0F2001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x321E2001);
  hasNormal |= (header.unk0 == 4163 && header.unk1 == 0x0F4B2001);

  for(unsigned int i = 0; i < header.vertexCount; i++) {
    Vector position;
    fread(&position, sizeof(position), 1, in);
    printf("[%d] =", i);
    printf(" position: { %f %f %f }",
           position.x, position.y, position.z);
    fprintf(out, "v %f %f %f\n", position.x, position.y, position.z);

    if (hasNormal) {
      Vector normal;
      fread(&normal, sizeof(normal), 1, in);
#if 0
      printf(", normal: { %f %f %f }",
             normal.x, normal.y, normal.z);
#endif
      fprintf(out, "vn %f %f %f\n", normal.x, normal.y, normal.z);
    }

    struct {
      float u;
      float v;
    } uv;
    fread(&uv, sizeof(uv), 1, in);
    printf(", uv: { %f %f }",
           uv.u, uv.v);
    fprintf(out, "vt %f %f\n", uv.u, -uv.v); // Note the flipped V axis!

    printf("\n");
    fprintf(out, "\n");
  }

  uint16_t* indexBuffer = malloc(2 * header.indexCount);
  fread(indexBuffer, 2, header.indexCount, in);

  uint32_t totalDrawIndexCount = 0;
  unsigned int totalFaceCount = 0;

  printf("unk0: 0x%08X (%d)\n", header.unk0, header.unk0); 
  printf("unk1: 0x%08X\n", header.unk1);

  unsigned int indexOffset = 0;
  for(unsigned int i = 0; i < header.drawCount; i++) {

    typedef struct {
      uint16_t unk0a; // Texture index?
      uint16_t unk0b;
      uint32_t unk1;
      uint32_t unk2; // Sometimes matches vertexCount ?!
      uint32_t indexCount;
    } Draw;

    Draw draw;
    fread(&draw, sizeof(draw), 1, in);

    printf("[%d] = 0x%04X 0x%04X 0x%08X 0x%08X, indexCount: %d\n",
           i,
           draw.unk0a,
           draw.unk0b,
           draw.unk1,
           draw.unk2,
           draw.indexCount);

    fprintf(out, "g %s/draw%d\n", basename(outPath), i);
    fprintf(out, "usemtl %s/texture%d\n", mtlFile, draw.unk0a);

    for(unsigned int j = 0; j < draw.indexCount; j++) {

      // Keep track of the last 3 vertex indices
      static unsigned int a = 0;
      static unsigned int b = 0;
      static unsigned int c = 0;

      a = b;
      b = c;
      c = indexBuffer[indexOffset++] + 1; // Wavefront has indices starting at 1

      // Let the buffer fill up first
      if (j < 2) {
        continue;
      }

      // Skip degenerates
      if (a == b || a == c || b == c) {
        continue;      
      }

      // Output face indices
      if (j % 2 == 1) {
        fprintf(out, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", a, a, a, b, b, b, c, c, c);
      } else {
        fprintf(out, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", b, b, b, a, a, a, c, c, c);
      }
      totalFaceCount++;

    }

    totalDrawIndexCount += draw.indexCount;
  }

  free(indexBuffer);

  printf("Total indexCount: %d / %d\n", totalDrawIndexCount, header.indexCount);
  printf("Total faces: %d / %d\n", totalFaceCount, header.faceCount);
  assert(totalDrawIndexCount == header.indexCount);
//  assert(totalFaceCount == header.faceCount); // Sometimes off by one?!

  // Switch to the material output file
  fclose(out);
  out = fopen(mtlPath, "wb");

  for(unsigned int i = 0; i < header.textureCount; i++) {
    typedef struct {
      uint16_t file;
      uint16_t resource;
      uint8_t always0[20];
    } TextureReference;

    printf("Reading tex at %d\n", ftell(in));

    TextureReference texture;
    fread(&texture, sizeof(texture), 1, in);
    printf("[%d] = resource: %d, index: %d\n", i, texture.resource, texture.file);

    fprintf(out, "newmtl %s/texture%d\n", mtlFile, i);
    fprintf(out, "map_Kd -s 1 1 %d-%d.tga\n", texture.resource, texture.file);
  }

  while(!feof(in)) {
    fread(&d, 4, 1, in);
    printf("0x%08X [%d]\n", d, d);
  }

/*
  printf("unk0: 0x%08X (%d)\n", header.unk0, header.unk0); 
  printf("unk1: 0x%08X\n", header.unk1);

switch(header.unk0) {
  case 4099:
    switch(header.unk1) {
      case 0x64001402: break; // vertex, uv
      default: assert(false); break;
    }
    break;
  case 4161:
    switch(header.unk1) {
      case 0x00002001: break; // vertex, uv
      default: assert(false); break;
    }
    break;
  case 4163:
    switch(header.unk1) {
      case 0x00002001: break; // vertex, normal, uv
      case 0x00642001: break; // vertex, uv
      default: assert(false); break;
    }
    break;
  default: assert(false); break;
}
*/

  fclose(in);
  fclose(out);

  return 0;
}
