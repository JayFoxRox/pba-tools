// Converts 0022 files to tga
//FIXME: Somehow store used-size + mipmaps

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "txc_dxtn.h"

#include "tga.h"
#include "surface.h"

int main(int argc, char* argv[]) {
  FILE* in = fopen(argv[1], "rb");
  FILE* out = fopen(argv[2], "wb"); 

  SurfaceHeader header;
  fread(&header, sizeof(header), 1, in);
  printSurfaceHeader(&header);

  TgaHeader tgaHeader;
  setupTgaHeader(&tgaHeader, UncompressedRGB, header.width_a, header.height_a);
  fwrite(&tgaHeader, sizeof(tgaHeader), 1, out);

  size_t bufferSize = header.stride * header.height_a;
  uint8_t* buffer = malloc(bufferSize);
  fread(buffer, bufferSize, 1, in);

  for(unsigned int y = 0; y < header.height_a; y++) {
    for(unsigned int x = 0; x < header.width_a; x++) {
      switch(header.format) {
      case 0x0001:
        fwrite(&buffer[y * header.stride + x * 4], 4, 1, out);
        break;
      case 0x0006: {
        // This is a weird format..
        // It can be found in ToTAN 2004-0 (0006 no-mipmaps) where the same
        // texture is also in 2004-5 (0009 mipmapped).
        // The stride suggests it's 16bpp, however, you'll get garbage then
        // as pixels seem to be 32bpp.
        // Yet, that means only half the image fits into the file..
        // 
        uint32_t pixel;
        if (x >= header.width_a / 2) {
          pixel = 0x00FFFFFF;
        } else {
          pixel = *(uint32_t*)&buffer[y * header.stride + x * 4];
        }
        uint8_t texel[4];

        // Attempts at other 16bpp formats
#if 0
        // 565
        texel[0] = ((pixel >> 0) & ((1 << 5) - 1)) << 3;
        texel[1] = ((pixel >> 5) & ((1 << 6) - 1)) << 2;
        texel[2] = ((pixel >> 11) & ((1 << 5) - 1)) << 3;
        texel[3] = 0xFF;

        // 1555
        texel[0] = ((pixel >> 0) & ((1 << 5) - 1)) << 3;
        texel[1] = ((pixel >> 5) & ((1 << 5) - 1)) << 3;
        texel[2] = ((pixel >> 10) & ((1 << 5) - 1)) << 3;
        texel[3] = ((pixel >> 15) & ((1 << 1) - 1)) << 7;

        // 4444
        texel[0] = ((pixel >> 0) & ((1 << 4) - 1)) << 4;
        texel[1] = ((pixel >> 4) & ((1 << 4) - 1)) << 4;
        texel[2] = ((pixel >> 8) & ((1 << 4) - 1)) << 4;
        texel[3] = ((pixel >> 12) & ((1 << 4) - 1)) << 4;

        // 5551
        texel[0] = ((pixel >> 11) & ((1 << 5) - 1)) << 3;
        texel[1] = ((pixel >> 6) & ((1 << 5) - 1)) << 3;
        texel[2] = ((pixel >> 1) & ((1 << 5) - 1)) << 3;
        texel[3] = ((pixel >> 0) & ((1 << 1) - 1)) << 7;
#endif

        texel[0] = (pixel >> 0) & 0xFF;
        texel[1] = (pixel >> 8) & 0xFF;
        texel[2] = (pixel >> 16) & 0xFF;
        texel[3] = (pixel >> 24) & 0xFF;
        fwrite(&texel[0], 1, 1, out);
        fwrite(&texel[1], 1, 1, out);
        fwrite(&texel[2], 1, 1, out);
        fwrite(&texel[3], 1, 1, out);

        break;
      }
      case 0x0009: {
        uint8_t texel[4];
        fetch_2d_texel_rgba_dxt1(header.width_a, buffer, x, y, texel);
        fwrite(&texel[2], 1, 1, out);
        fwrite(&texel[1], 1, 1, out);
        fwrite(&texel[0], 1, 1, out);
        fwrite(&texel[3], 1, 1, out);
        break;
      }
      case 0x000D: {
        uint8_t texel[4];
        fetch_2d_texel_rgba_dxt5(header.width_a, buffer, x, y, texel);
        fwrite(&texel[2], 1, 1, out);
        fwrite(&texel[1], 1, 1, out);
        fwrite(&texel[0], 1, 1, out);
        fwrite(&texel[3], 1, 1, out);
        break;
      }
      default:
        assert(false);
        break;
      }
    }
  }

  free(buffer);
 
  fclose(out);
  fclose(in);

  return 0;
}
