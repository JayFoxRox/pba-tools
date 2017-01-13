// Converts 002C files to a obj (which contains the collision)

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <libgen.h>

typedef struct {
  float x;
  float y;
  float z;
  float radius; // ?!
  uint32_t unkCount;
  uint8_t always0[8]
} Header;

typedef struct {
  uint32_t always0;
  float x;
  float y;
  float z;
  float unk0; // 1.0 or some huge value?!
} Unk;

typedef struct {
  float x;
  float y;
  float z;

//  float unk[5];
} Point;

#if 0
0.000000 -0.000000 -1.000000 1
0.828400 -0.560138 -0.000000 ~1
-0.221253 0.149604  0.963673 <1
#endif

int main(int argc, char* argv[]) {
  char* outPath = argv[2];
  FILE* in = fopen(argv[1], "rb");
  FILE* out = fopen(outPath, "w");

  Header header;
  fread(&header, sizeof(header), 1, in);
  printf("unk: %f %f %f %f\n", header.x, header.y, header.z, header.radius);
  printf("unkCount: %d\n", header.unkCount);

  while(!feof(in)) {
    Point point;
    size_t x = ftell(in);
    fread(&point, sizeof(point), 1, in);
    printf("%d point: { %f %f %f }\n", x, point.x, point.y, point.z);
    fprintf(out, "v %f %f %f\n", point.x, point.y, point.z);
  }

  fclose(in);
  fclose(out);
  
  return 0;
}
