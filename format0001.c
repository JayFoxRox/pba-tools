// Zero terminates 0001 files
//
// (C) 2017 Jannik Vogel
//
// Licensed under GPLv2 or later.
// See LICENSE.txt for more information.

//FIXME: Some files are actually ROMs so the tool won't work?!

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  FILE* in = fopen(argv[1], "rb");
  FILE* out = fopen(argv[2], "wb");

  while(!feof(in)) {
    uint8_t b;
    fread(&b, 1, 1, in);
    // Abort on zero termination
    if (b == 0x00) {
      break;
    }
    fwrite(&b, 1, 1, out);
  }

  fclose(in);
  fclose(out);

  return 0;
}
