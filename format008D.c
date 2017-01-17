// Converts 008D files to jpg

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "tga.h"
#include "surface.h"

int main(int argc, char* argv[]) {
  FILE* in = fopen(argv[1], "rb");
  const char* outPath = argv[2];
  FILE* out = fopen(outPath, "wb"); 

  SurfaceHeader header;
  fread(&header, sizeof(header), 1, in);
  printSurfaceHeader(&header);

  //FIXME: Read and print unknown value
  fseek(in, 4, SEEK_CUR);


  //FIXME: Obviously not correct for JFIF / JPEG..

  size_t bufferSize = header.stride * header.height_a * 4;
  uint8_t* buffer = malloc(bufferSize);

  switch(header.format) {
  case 0x000E: {
    // JFIF / JPEG
    //FIXME: Needs to read more data!
    fread(buffer, bufferSize, 1, in);
    fwrite(buffer, bufferSize, 1, out);
    break;
  }
  case 0x0010: {
    // RGBA?!

    char alphaPath[1024];
    sprintf(alphaPath, "%s-alpha.tga", outPath);

    FILE* alpha = fopen(alphaPath, "wb");
  
    TgaHeader tgaHeader;
    setupTgaHeader(&tgaHeader, UncompressedGrayscale, header.width_a, header.height_a);
    fwrite(&tgaHeader, sizeof(tgaHeader), 1, alpha);
 
    fread(buffer, bufferSize / 4, 1, in);
    fwrite(buffer, bufferSize / 4, 1, alpha);

    fclose(alpha);

    //FIXME: if unk is 0x1004506C there might not be a color image?!

    fread(buffer, bufferSize * 10, 1, in);
    fwrite(buffer, bufferSize * 10, 1, out);

    break;
  }
  default:
    assert(false);
    break;
  }

  free(buffer);
 
  fclose(out);
  fclose(in);

  return 0;
}
