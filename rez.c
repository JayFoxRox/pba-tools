#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

typedef struct {
  uint32_t always0;
  uint32_t resourceIndex;
  uint32_t resourceCount;
  uint32_t fileCount;
} Header;

typedef struct {
  char name[96];
  uint32_t size;

  // Maybe a timestamp?!
  uint64_t unk0; // FILETIME?!

  // Maybe attributes?
  uint32_t unk1;
    // 0x00000012 for TGA files?!

  uint32_t always0[2];
  uint32_t always1;
} __attribute__((packed)) OriginalFileHeader;

// Describes which files are stored in a resource
typedef struct {
    uint32_t offset;
    uint32_t packedLength;
    uint32_t unpackedLength;
    uint16_t type; // Maybe file format?!
      // DX9:
      // 0x0000 = Marker / No Data
      // 0x0001 = CSV or ROM
      // 0x0013 = Resource
      // 0x0018 = Font metrics? [.bin]
      // 0x0022 = Texture
      // 0x0024 = DIM (model file)
      // 0x0025 = WAV or ROM
      // 0x002A = DIL (placement file)
      // 0x002C = DIM (collision file)
      // 0x002F = Samples (What is this??)
      // 0x005F = Version

      // DX11 (also supports DX9 types):
      // 0x008D = Texture [extension of 0022 but also JPEG etc?]
      // 0x008E = DIM (model file) [Similar to 0024 but without normal?!]
      // 0x008F = WAV stuff [Same as 0025 ?!]
    union {
      uint16_t compression;
        // DX9:
        // 0x0000 = Raw data
        // 0x0001 = LZSS
        
        // DX11 (also supports DX9 compressions):
        // 0x0009 = LZSS
      uint16_t tableOffset; // from end
    };
    uint32_t flags; // Alignment?!
        // 0x00000000 for resources
        // 0x00000010 for files
    uint32_t always0;
} FileHeader;

const uint64_t filetimeOffset = 11644473600ULL * 10000000ULL; // number of 100ns intervals from 1 Jan. 1601 00:00 to 1 Jan 1970 00:00 UTC
void filetimeToTimeval(uint64_t filetime, struct timeval* tv) {
  // We only handle times AFTER 1 Jan 1970 00:00 UTC
  if (filetime <= filetimeOffset) {
    tv->tv_sec = 0;
    tv->tv_usec = 0;
    return;
  }
  filetime -= filetimeOffset;
  filetime /= 10ULL; // Convert to us
  tv->tv_sec = filetime / 1000000ULL;
  tv->tv_usec = filetime % 1000000ULL;
}

unsigned int alignUp(unsigned int value, unsigned int alignment) {
  return value + (alignment - value % alignment) % alignment;
}

void unpackRaw(FILE* in, FILE* out, size_t length) {
  while(length > 0) {
    uint8_t b;
    fread(&b, 1, 1, in);
    fwrite(&b, 1, 1, out);
    length--;
  }
}

void unpackLZSS(FILE* in, FILE* out, size_t length) {
  uint32_t outputLength;
  uint32_t processedLength = 0;
  uint16_t c = 0; // Cursor for the window (automaticly wraps!)
  uint8_t window[0x10000]; // Bomb da stack!
  memset(window, 0x00, sizeof(window));

  fread(&outputLength, 4, 1, in);

  // In some cases the outputLength is aligned to 16 ?!
  unsigned int padding = 0;
  if (outputLength < length) {
    padding = alignUp(outputLength, 16) - outputLength;
    printf("File was stored without padding!\n");
  }

//FIXME: Dirty hack for DX11 files which don't seem to store a size for packed blocks in resource tables!
length = outputLength;

  //assert((outputLength + padding) == length);
  while(length > 0) {
    uint8_t b;
    fread(&b, 1, 1, in);
    processedLength++;

    uint8_t chunkLength = b & 0x7F;
    if (chunkLength >= length) {
      printf("Shortening chunk due to end of output.\n");
      chunkLength = length;
    }

    if (b & 0x80) {
      while(chunkLength-- > 0) {
        fread(&b, 1, 1, in);
        processedLength++;
        window[c++] = b;
        fwrite(&b, 1, 1, out);
        length--;
      }
    } else {
      uint16_t offset;
      fread(&offset, 2, 1, in);
      processedLength += 2;
      while(chunkLength-- > 0) {
        b = window[(uint16_t)(c - offset)];
        fwrite(&b, 1, 1, out);
        length--;
        window[c++] = b;
      }
    }
  }
  printf("Processed %d\n", processedLength);
  printf("       of %d\n", outputLength);

  for(unsigned int i = 0; i < padding; i++) {
    uint8_t b = 0x00;
    fwrite(&b, 1, 1, out);
  }
}

void export(FILE* f, FileHeader* fileHeader, const char* path, bool compressed) {
  size_t cursor = ftell(f);
  fseek(f, fileHeader->offset, SEEK_SET);
  printf("Exporting '%s' (Compressed: %d)\n", path, compressed);
  FILE* out = fopen(path, "wb");
  assert(out != NULL);
  if (compressed) {
    unpackLZSS(f, out, fileHeader->unpackedLength);
  } else {
    unpackRaw(f, out, fileHeader->packedLength);
  }
  fclose(out);
  fseek(f, cursor, SEEK_SET);
}

int main(int argc, char* argv[]) {
  FILE* f = fopen(argv[1], "rb");
  const char* outPath = argv[2];

  // This is the actual header

  fseek(f, -2048, SEEK_END);
  Header header;
  fread(&header, sizeof(header), 1, f);
  printf("Resource %d - %d\n", header.resourceIndex, header.resourceIndex + header.resourceCount - 1);
  printf("%d files total\n", header.fileCount);

  unsigned int j;
  for(j = 0; j < 30; j++) {
    uint32_t d;
    fread(&d, 4, 1, f);
    printf("  unknown[%d] = 0x%X [%d]\n", j, d, d);
  }

  // This is a header which describes where data resides
  
  fseek(f, -4096, SEEK_END);

  FileHeader resourceHeader;

  unsigned int i;
  for(i = 0; i < header.resourceCount; i++) {
    fread(&resourceHeader, sizeof(resourceHeader), 1, f);

    uint32_t tableAddress = resourceHeader.offset + resourceHeader.packedLength - resourceHeader.tableOffset;
    printf("[%d] = offset: 0x%08X, length: %d (%d unpacked), type: 0x%04X, table: %d [0x%08X], flags: 0x%08X, always0: %d\n",
           header.resourceIndex + i,
           resourceHeader.offset,
           resourceHeader.packedLength,
           resourceHeader.unpackedLength,
           resourceHeader.type,
           resourceHeader.tableOffset, tableAddress,
           resourceHeader.flags,
           resourceHeader.always0);

    // Dump file table for resources
    if (resourceHeader.type == 0x0013) {
      size_t cursor = ftell(f);
      fseek(f, tableAddress, SEEK_SET);
      
      uint32_t fileCount;
      fread(&fileCount, 4, 1, f);
      printf("  %d files in resource\n", fileCount);

      unsigned int j;
      for(j = 0; j < 3; j++) {
        uint32_t d;
        fread(&d, 4, 1, f);
        printf("  unknown[%d] = 0x%X [%d]\n", j, d, d);
      }

      for(j = 0; j < fileCount; j++) {
        FileHeader fileHeader;
        fread(&fileHeader, sizeof(fileHeader), 1, f);
        printf("  [%d] = offset: 0x%X, length: %d (%d unpacked), type: 0x%04X, compression: 0x%04X, flags: 0x%08X, always0: %d \n",
               j,
               fileHeader.offset,
               fileHeader.packedLength,
               fileHeader.unpackedLength,
               fileHeader.type,
               fileHeader.compression,
               fileHeader.flags,
               fileHeader.always0);

        char buf[1024];
        sprintf(buf, "%s/%d-%d.%04X", outPath, header.resourceIndex + i, j, fileHeader.type);
        export(f, &fileHeader, buf,
#if 0
          // Old version
          fileHeader.compression != 0
#else
          // PC11
          fileHeader.compression > 1
#endif
        );
      }

      fseek(f, cursor, SEEK_SET);
    } else {
      //FIXME: Dump resource directly..
      char buf[1024];
      sprintf(buf, "%s/%d.%04X", outPath, header.resourceIndex + i, resourceHeader.type);
      export(f, &resourceHeader, buf, false);
    }
  }


  for(i = 0; i < 100; i++) {
#if 0
    if (i % 6 == 0) {
      printf("\n");
    }
#endif
    uint32_t d;
    fread(&d, 4, 1, f);
    printf("[%d] = 0x%X [%d]\n", i, d, d);
  }
  
  fseek(f, alignUp(resourceHeader.offset, 4096), SEEK_SET);

  //fseek(f, 0xF9F8B8, SEEK_SET); // TEldorado
  while(!feof(f)) {

    // First a table for each resource
    for(i = 0; i < header.resourceCount; i++) {

      OriginalFileHeader fileHeader;
      fread(&fileHeader, sizeof(fileHeader), 1, f);

      
      struct timeval tv;
      time_t nowtime;
      struct tm* nowtm;
      char tmbuf[64];

      filetimeToTimeval(fileHeader.unk0, &tv);
      nowtime = tv.tv_sec;
      nowtm = localtime(&nowtime);
      strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);

      printf("original name '%.96s', original size %d, 0x%" PRIX64 " (%s) 0x%08X 0x%08X%08X %d\n",
             fileHeader.name,
             fileHeader.size,
             fileHeader.unk0, tmbuf,
             fileHeader.unk1,
             fileHeader.always0[0],
             fileHeader.always0[1],
             fileHeader.always1);
    }
    printf("\n\n");

    //FIXME: Remove.. this is a stupid hack
    if (header.resourceCount == 0) {
      break;
    }

  }

  while(!feof(f)) {
    uint32_t d;
    fread(&d, 4, 1, f);
    printf("Read 0x%08X [%d]\n", d, d);
  }

  fclose(f);
  return 0;
}
