#include <stdint.h>

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
} __attribute__((packed)) SurfaceHeader;

static void printSurfaceHeader(SurfaceHeader* header) {
  printf("%d x %d (Stride: %d)\n", header->width_a, header->height_a, header->stride);
  printf("format: 0x%04X\n", header->format);
  printf("unk0: 0x%08X\n", header->unk0);
  printf("mipmapCount: 0x%04X\n", header->mipmapCount);
  printf("widthShift: 0x%02X (%d)\n", header->widthShift, 1 << header->widthShift);
  printf("heightShift: 0x%02X (%d)\n", header->heightShift, 1 << header->heightShift);
  printf("pixelWidth: %f\n", header->pixelWidth);
  printf("pixelHeight: %f\n", header->pixelHeight);

  printf("usedSize: %d x %d\n", header->width_b, header->height_b);
//  assert(header->width_a == header->width_b);
//  assert(header->height_a == header->height_b);

  uint32_t o = sizeof(SurfaceHeader);
  uint32_t s = header->stride;
  uint32_t w = header->width_a;
  uint32_t h = header->height_a;
  for(unsigned int i = 0; i <= header->mipmapCount; i++) {
    printf("Level %d: %d x %d at 0x%X\n", i, w, h, o);
    o += s * h;
    s /= 2;
    w /= 2;
    h /= 2;
  }
  printf("EOF at 0x%X\n", o);
}
