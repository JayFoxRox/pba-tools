// Prints 002A files
//FIXME: Add some kind of conversion

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  FILE* in = fopen(argv[1], "rb");
  typedef struct {
    char sourceFile[32];
    char sourcePath[256];
    uint32_t placementCount;
    uint8_t unk[272];
  } Header;
  typedef struct {
    char model[40];
    uint32_t unk0;
    struct {
      float x;
      float y;
      float z;
    } position;
//    uint8_t imp[16]; // 16 bytes [quaternion rotation?]
    uint8_t tmp[8]; // uninteresting? [scale of some sorts?]
    uint32_t unk1;
    struct {
      float x;
      float y;
      float z;
    } scale;
  } __attribute__((packed)) Placement;

  Header header;
  fread(&header, sizeof(header), 1, in);
  printf("Source file: '%.32s'\n", header.sourceFile);
  printf("Source path: '%.256s'\n", header.sourcePath);

  for(unsigned int i = 0; i < header.placementCount; i++) {
    Placement placement;
    fread(&placement, sizeof(placement), 1, in);

    printf("[%d] = Placing '%.40s' 0x%08X position: { %f %f %f } 0x%08X scale: { %f %f %f }\n",
           i,
           placement.model,
           placement.unk0,
           placement.position.x, placement.position.y, placement.position.z,
           placement.unk1,
           placement.scale.x, placement.scale.y, placement.scale.z);
  }

  fclose(in);

  return 0;
}
