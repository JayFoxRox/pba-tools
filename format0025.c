// Converts 0025 files to WAV
//
// (C) 2017 Jannik Vogel
//
// Licensed under GPLv2 or later.
// See LICENSE.txt for more information.

//FIXME: What's different with unk0 == 1 ?!

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// WAVE file header format

typedef struct {
    unsigned char riff[4];                      // RIFF string
    unsigned int overall_size;                  // overall size of file in bytes
    unsigned char wave[4];                      // WAVE string
} RiffHeader;

typedef struct {
    unsigned char fmt_chunk_marker[4];
    unsigned int length_of_fmt;                 // length of the format data
    uint16_t format_type;                   // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    uint16_t channels;                      // no.of channels
    unsigned int sample_rate;                   // sampling rate (blocks per second)
    unsigned int byterate;                      // SampleRate * NumChannels * BitsPerSample/8
    uint16_t block_align;                   // NumChannels * BitsPerSample/8
    uint16_t bits_per_sample;               // bits per sample, 8- 8bits, 16- 16 bits etc
} WavHeader;

typedef struct {
    unsigned char data_chunk_header[4];         // DATA string or FLLR string
    unsigned int data_size;                     // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} WavDataHeader;


int main(int argc, char* argv[]) {
  FILE* in = fopen(argv[1], "rb");
  FILE* out = fopen(argv[2], "wb");

  uint32_t unk0;
  fread(&unk0, 4, 1, in);
  printf("unk0: 0x%08X\n", unk0);
  assert(unk0 == 0 || unk0 == 1);

  uint8_t inWavHeader[16];
  fread(&inWavHeader, sizeof(inWavHeader), 1, in);

  RiffHeader riffHeader;
  memcpy(riffHeader.riff, "RIFF", 4);
  riffHeader.overall_size = 100000; //FIXME
  memcpy(riffHeader.wave, "WAVE", 4);
  fwrite(&riffHeader, sizeof(riffHeader), 1, out);

  WavHeader wavHeader;
  memcpy(wavHeader.fmt_chunk_marker, "fmt ", 4);
  wavHeader.length_of_fmt = 16;
  memcpy(&wavHeader.format_type, inWavHeader, sizeof(inWavHeader));
  fwrite(&wavHeader, sizeof(wavHeader), 1, out);

  WavDataHeader wavDataHeader;
  fread(&wavDataHeader.data_chunk_header, 4, 1, in);
  //FIXME: Assert that this header reads "wave" now

  uint32_t unk1;
  fread(&unk1, 4, 1, in);
  printf("unk1: 0x%08X (%d)\n", unk1, unk1);
  assert(unk1 == 1);

  // FIXME: Assert zero
  fseek(in, 28, SEEK_CUR);

  uint32_t unk2;
  fread(&unk2, 4, 1, in);
  printf("unk2: 0x%08X (%d)\n", unk2, unk2);

  fread(&wavDataHeader.data_size, 4, 1, in);
  printf("data size: %d bytes\n", wavDataHeader.data_size);

  fwrite(&wavDataHeader, sizeof(wavDataHeader), 1, out);

  uint32_t unk3;
  fread(&unk3, 4, 1, in);
  printf("unk3: 0x%08X (%d)\n", unk3, unk3);

  fseek(in, 1980, SEEK_CUR);
  assert(ftell(in) == 2048);

  //FIXME: Loop until dataLength instead
  while(!feof(in)) {
    uint8_t b;
    fread(&b, 1, 1, in);
    fwrite(&b, 1, 1, out);
  }

  fclose(in);
  fclose(out);

  return 0;
}
