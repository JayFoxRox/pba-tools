// TGA helpers
//
// (C) 2017 Jannik Vogel
//
// Licensed under GPLv2 or later.
// See LICENSE.txt for more information.

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

typedef enum {
  UncompressedRGB = 2,
  UncompressedGrayscale = 3
} TgaType;

void setupTgaHeader(TgaHeader* header, TgaType type, unsigned short width, unsigned short height) {
  header->idlength = 0;
  header->colourmaptype = 0;
  header->datatypecode = type;
  header->colourmaporigin = 0;
  header->colourmaplength = 0;
  header->colourmapdepth = 0;
  header->x_origin = 0;
  header->y_origin = 0;
  header->width = width;
  header->height = height;
  header->bitsperpixel = (type == UncompressedRGB ? 32 : 8);
  header->imagedescriptor = 1 << 5;
}
