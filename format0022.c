// Converts 0022 files to tga
//FIXME: Somehow store used-size + mipmaps

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "txc_dxtn.h"

typedef struct {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   char  colourmapdepth;
   short int x_origin;
   short int y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
} __attribute__((packed)) TgaHeader;


int main(int argc, char* argv[]) {
  FILE* out = fopen(argv[2], "wb"); 
  TgaHeader header;
  header.idlength = 0;
  header.colourmaptype = 0;
  header.datatypecode = 2;
  header.colourmaporigin = 0;
  header.colourmaplength = 0;
  header.colourmapdepth = 0;
  header.x_origin = 0;
  header.y_origin = 0;

typedef struct {
  uint16_t width_a; // +0
  uint16_t height_a; // +2
  uint16_t stride; // +4 ?
  uint16_t format; // +6
  uint32_t unk0; // +8
  uint8_t always0_a[16]; // +12
  uint16_t mipmapCount; // +30
  uint8_t widthShift; // +28
  uint8_t heightShift; // +29
  float pixelWidth; // +32
  float pixelHeight; // +36
  uint8_t always0_b[20]; // +40
  uint16_t width_b; // +60
  uint16_t height_b; // +62
  uint8_t always0_c[8]; // +64
  // 72
} __attribute__((packed)) Format0022;

  FILE* in = fopen(argv[1], "rb");

  Format0022 fmt;
  fread(&fmt, sizeof(fmt), 1, in);

  printf("%d x %d (Stride: %d)\n", fmt.width_a, fmt.height_a, fmt.stride);
  printf("format: 0x%04X\n", fmt.format);
  printf("unk0: 0x%08X\n", fmt.unk0);
  printf("mipmapCount: 0x%04X\n", fmt.mipmapCount);
  printf("widthShift: 0x%02X (%d)\n", fmt.widthShift, 1 << fmt.widthShift);
  printf("heightShift: 0x%02X (%d)\n", fmt.heightShift, 1 << fmt.heightShift);
  printf("pixelWidth: %f\n", fmt.pixelWidth);
  printf("pixelHeight: %f\n", fmt.pixelHeight);

  printf("usedSize: %d x %d\n", fmt.width_b, fmt.height_b);
//  assert(fmt.width_a == fmt.width_b);
//  assert(fmt.height_a == fmt.height_b);


  uint32_t o = 72;
  uint32_t s = fmt.stride;
  uint32_t w = fmt.width_a;
  uint32_t h = fmt.height_a;
  for(unsigned int i = 0; i <= fmt.mipmapCount; i++) {
    printf("Level %d: %d x %d at 0x%X\n", i, w, h, o);
    o += s * h;
    s /= 2;
    w /= 2;
    h /= 2;
  }
  printf("EOF at 0x%X\n", o);

  // Padding might follow..

  header.width = fmt.width_a;
  header.height = fmt.height_a;
  header.bitsperpixel = 32;
  header.imagedescriptor = 1 << 5;

  fwrite(&header, sizeof(header), 1, out);
  fseek(in, 72, SEEK_SET);

  size_t bufferSize = fmt.stride * fmt.height_a;
  uint8_t* buffer = malloc(bufferSize);
  fread(buffer, bufferSize, 1, in);

  for(unsigned int y = 0; y < fmt.height_a; y++) {
    for(unsigned int x = 0; x < fmt.width_a; x++) {
      switch(fmt.format) {
      case 0x0001:
        fwrite(&buffer[y * fmt.stride + x * 4], 4, 1, out);
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
        if (x >= fmt.width_a / 2) {
          pixel = 0x00FFFFFF;
        } else {
          pixel = *(uint32_t*)&buffer[y * fmt.stride + x * 4];
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
        fetch_2d_texel_rgba_dxt1(fmt.width_a, buffer, x, y, texel);
        fwrite(&texel[2], 1, 1, out);
        fwrite(&texel[1], 1, 1, out);
        fwrite(&texel[0], 1, 1, out);
        fwrite(&texel[3], 1, 1, out);
        break;
      }
      case 0x000D: {
        uint8_t texel[4];
        fetch_2d_texel_rgba_dxt5(fmt.width_a, buffer, x, y, texel);
        fwrite(&texel[2], 1, 1, out);
        fwrite(&texel[1], 1, 1, out);
        fwrite(&texel[0], 1, 1, out);
        fwrite(&texel[3], 1, 1, out);
        break;
      }
      case 0x000E: {
        // 4 bytes unknown
        // Then JPEG follows
      }
      default:
        assert(false);
        break;
      }
    }
  }

  free(buffer);
 
  fclose(in);
  fclose(out);

  return 0;
}
