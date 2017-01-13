// Converts 0024 files to obj / mtl

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <libgen.h>

typedef struct {
  struct {
    float x;
    float y;
    float z;
  } position;
  struct {
    float x;
    float y;
    float z;
  } normal;
  float u;
  float v;
} Vertex;

typedef struct {
  uint32_t unk0; // 4163 in most files, 4161 in BK2k main-playfield model
  uint32_t drawCount;
  uint32_t always0_a;
  uint32_t vertexCount;
  uint32_t always0_b;
  uint32_t unk1; // No idea.. flags of some sorts? [0x00002001]
  uint32_t faceCount; // triangle count without degenerates ?!
  uint32_t always0_c[4];
  uint32_t unk[16+38-7];
  uint32_t indexCount;
  uint32_t always0_d[7]
} Header;

int main(int argc, char* argv[]) {
  char* outPath = argv[2];
  FILE* in = fopen(argv[1], "rb");
  FILE* out = fopen(outPath, "w");

  char* mtlPath = alloca(strlen(outPath) + 4);
  sprintf(mtlPath, "%s.mtl", outPath);

  fprintf(out, "mtllib ./%s\n", basename(mtlPath));

  Header header;
  fread(&header, sizeof(header), 1, in);
  printf("unk0: %d\n", header.unk0); 
  printf("%d vertices\n", header.vertexCount);
  printf("drawCount: %d\n", header.drawCount);
  printf("unk1: 0x%08X\n", header.unk1);
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
    printf("[%d] = 0x%08X [%d]\n", i, d, d); // Unknown
  }

  for(unsigned int i = 0; i < header.vertexCount; i++) {
    Vertex v;
    fread(&v, sizeof(v), 1, in);
    printf("[%d] =", i);
    printf(" position: { %f %f %f }",
           v.position.x, v.position.y, v.position.z);
    printf(", normal: { %f %f %f }",
           v.normal.x, v.normal.y, v.normal.z);
    printf(", uv: { %f %f }",
           v.u, v.v);
    printf("\n");

    fprintf(out, "v %f %f %f\n", v.position.x, v.position.y, v.position.z);
    fprintf(out, "vn %f %f %f\n", v.position.x, v.position.y, v.position.z);
    fprintf(out, "vt %f %f\n", v.u, -v.v); // Note the flipped V axis!
    fprintf(out, "\n");
  }

  uint16_t* indexBuffer = malloc(2 * header.indexCount);
  fread(indexBuffer, 2, header.indexCount, in);

  uint32_t totalDrawIndexCount = 0;
  unsigned int totalFaceCount = 0;

  unsigned int indexOffset = 0;
  for(unsigned int i = 0; i < header.drawCount; i++) {

    typedef struct {
      uint32_t unk0;
      uint32_t unk1;
      uint32_t unk2; // Sometimes matches vertexCount ?!
      uint32_t indexCount;
    } Draw;

    Draw draw;
    fread(&draw, sizeof(draw), 1, in);

    printf("[%d] = 0x%08X 0x%08X 0x%08X, indexCount: %d\n",
           i,
           draw.unk0,
           draw.unk1,
           draw.unk2,
           draw.indexCount);

    fprintf(out, "g draw%d\n", i);
    fprintf(out, "usemtl draw%d\n", i);

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

  // Switch to the material output file
  fclose(out);
  out = fopen(mtlPath, "w");

  for(unsigned int i = 0; i < header.drawCount; i++) {
    typedef struct {
      uint16_t file;
      uint16_t resource;
      uint8_t always0[20]
    } TextureReference;

    TextureReference texture;
    fread(&texture, sizeof(texture), 1, in);
    printf("[%d] = resource: %d, index: %d\n", i, texture.resource, texture.file);

    fprintf(out, "newmtl draw%d\n", i);
    fprintf(out, "map_Kd -s 1 1 %d-%d.tga\n", texture.resource, texture.file);

  }

  while(!feof(in)) {
    fread(&d, 4, 1, in);
    printf("0x%08X [%d]\n", d, d);
  }


  fclose(in);
  fclose(out);

  return 0;
}
