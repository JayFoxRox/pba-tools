// Prints 002A files
//FIXME: Add support for light-placements

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* out = NULL;

#include "json.h"

typedef struct {
  char sourceFile[32];
  char sourcePath[256];
  uint32_t objectCount;
  uint32_t unk0;
/*
union {
  float f[272 / 4];
  struct {
    float x;
    float y;
    float z;

    float x2;
    float y2;
    float z2;

    float a;
    float b;
  } unk2[1];
}
*/
    // objectCount = sizeof(unk)
    // 41 = 168 (not sure which game, but 2007-0 / totan?!)
    // 120 = ~~168~~ 484 (not sure which game, but 2007-1 / totan?!) [168 is actually garbage which was probably left in mem during resource creation?!]
    // 67 = 272 BK2K
    // 89 = 360 TAF PC
} Header;

typedef struct {
  uint32_t placementCount;
  uint32_t unk0;
  uint32_t unk1; // unk1 of previous object incremented by the size of this one
} Object;

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
  } angle; // In radians!
  struct {
    float x;
    float y;
    float z;
  } scale;
} __attribute__((packed)) Placement;

int main(int argc, char* argv[]) {
  FILE* in = fopen(argv[1], "rb");
  out = fopen(argv[2], "wb");
  startObject();

  Header header;
  fread(&header, sizeof(header), 1, in);
  printf("Source file: '%.32s'\n", header.sourceFile);
  printf("Source path: '%.256s'\n", header.sourcePath);

  putKey("sourceFile"); putString(header.sourceFile);
  putKey("sourcePath"); putString(header.sourcePath);

  printf("Type: 0x%08X, Object count: %d\n", header.unk0, header.objectCount);

  // FIXME: Pretty sure that all of this data is garbage. no idea why it is allocated..
  for(unsigned int i = 0; i < header.objectCount; i++) {
    uint32_t u;
    fread(&u,4,1,in);
    printf("[%d] = 0x%08X (%f)\n", i, u, *(float*)&u);
  }

  putKey("objects");
  startArray();
  for(unsigned int i = 0; i < header.objectCount; i++) {

    Object object;
    char objectName[256];
    fread(objectName, 32, 1, in);
    objectName[32] = '\0';
    size_t length = strlen(objectName);
    // Read name in chunks [first chunk is 32 byte each chunk after that is 8 byte]
    if(length == 32) {
      //FIXME: Do this in a loop?!
      fread(&objectName[length], 8, 1, in);
      objectName[length + 8] = '\0';
    }
    fread(&object, sizeof(object), 1, in);

    startObject();

    putKey("name"); putString(objectName);
    putKey("unk0"); putInteger(object.unk0);
    putKey("unk1"); putInteger(object.unk1);

    putKey("placements");
    startArray();

    printf("[%d] = Placing '%s' 0x%08X 0x%08X, %d placement(s)",
           i,
           objectName,
           object.unk0,
           object.unk1,
           object.placementCount);

    for(unsigned int j = 0; j < object.placementCount; j++) {
      Placement placement;
      fread(&placement, sizeof(placement), 1, in);

      printf(";; position: { %f %f %f } angle: { %f %f %f } scale: { %f %f %f }",
             placement.position.x, placement.position.y, placement.position.z,
             placement.angle.x, placement.angle.y, placement.angle.z,
             placement.scale.x, placement.scale.y, placement.scale.z);

      //FIXME: Export unknown data?!

      startObject();

      putKey("position");
      startArray();
      putFloat(placement.position.x);
      putFloat(placement.position.y);
      putFloat(placement.position.z);
      endArray();

      putKey("angle");
      startArray();
      putFloat(placement.angle.x);
      putFloat(placement.angle.y);
      putFloat(placement.angle.z);
      endArray();

      putKey("scale");
      startArray();
      putFloat(placement.scale.x);
      putFloat(placement.scale.y);
      putFloat(placement.scale.z);
      endArray();

      endObject();
    }
    printf("\n");
    endArray();
  
    endObject();
  }
  endArray();

  endObject();

  fclose(in);
  fclose(out);

  return 0;
}
