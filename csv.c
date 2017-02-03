// This converts a PBA CSV file to JSON (which is far easier to parse)

// Assumptions:
// * Comma at end of line is implied (sometimes given, sometimes not)
// * "//" Starts a comment
// * Strings are trimmed
// * Strings can contain spaces or comma by quoting them
// * In quoted strings you can escape using "\". Known:
//   * \"

// Issues:
// * Fields may appear twice, it is not yet known which field will be used
// * @<Platform> tags are not implemented yet [are those per line or command?]
// FIXME ^

//FIXME: This deserves a full rewrite..

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* out = NULL;

#include "json.h"

//FIXME: Move these into some state struct?!
const char* tableType = NULL;
const char* tableName = NULL;
FILE* f;

bool isSpace(char c) {
  switch(c) {
    case '\t':
    case ' ':
      return true;
    default:
      break;
  }
  return false;
}

// This is very hacky.. If this wasn't a weekend project I'd probably have
// written a proper parser (Contributions welcome!)
// Also: Caller has to free this later!
char* parseString() {
  static char* cursor = "";

  if (strlen(cursor) == 0) {
    static char lineBuffer[2048];
    if (feof(f)) {
      return NULL;
    }
    fgets(lineBuffer, sizeof(lineBuffer), f);
    cursor = lineBuffer;
  }

  //printf("Data '%s'\n", cursor);

  // Check if this might be the end of the table!
  if (!strncmp(cursor, "TableEnd", 8)) {
    //FIXME: Do same stuff as with "Table" start [Check if LHS of = is correct]
    //FIXME: Match if the correct "type:name" pair was closed as was opened
    printf("End of table?! '%s'\n", cursor);
    cursor = ""; // FIXME: Is it possible to have more on this line?!
    return NULL;
  }

  char* buffer = malloc(2048);
  char* bufferCursor = buffer;

  // Skip leading spaces
  while(isSpace(*cursor++));
  cursor--;

#if 0
  printf("\n\n");
#endif

  bool escape = false;
  while(*cursor != '\0') {

#if 0
    printf("escape + buffer: %d '%s<>%s'\n", escape, buffer, cursor);
#endif

    // Toggle escape state
    if (cursor[0] == '"') {
      escape = !escape;
      cursor++;
      continue;
    }

    if (escape) {
      // Do the escape
      if (cursor[0] == '\\') {
        cursor++;
      }
    } else {
      // End due to comment
      if (cursor[0] == '/' && cursor[1] == '/') {
#if 0
        printf("Found comment '%s'\n", cursor + 2);
#endif
        cursor = "";
        break;
      }

      // End due to next line
      if ((cursor[0] == '\r') || (cursor[0] == '\n')) {
        cursor = "";
        break;
      }

      // End due to next field
      if (cursor[0] == ',') {
        cursor++;
        break;
      }
    }

    *bufferCursor++ = *cursor++;
  }

  // Don't allow empty output
  if (bufferCursor == buffer) {
    return parseString();
  }

  *bufferCursor = '\0';
  return buffer;
}

long long parseInteger() {
  char* buffer = parseString();
  long long value = atoll(buffer);
  free(buffer);
  return value;
}

float parseFloat() {
  //FIXME: Handle "f" suffix!
  char* buffer = parseString();
  float value = atof(buffer);
  free(buffer);
  return value;
}

// Helper routines to move from CSV to JSON
//FIXME: These should always be array copies so my types don't get mixed..
void copyInteger() {
  long long int value = parseInteger();
  putInteger(value);
}
void copyFloat() {
  float value = parseFloat();
  putFloat(value);
}
void copyString() {
  char* value = parseString();
  putString(value);
  free(value);
}
void copyBoolean() {
  copyString(); //FIXME!
}

void _copy(void(*callback)(), unsigned int count) {
  startArray();
  for(unsigned int i = 0; i < count; i++) {
    callback();
  }
  endArray();
}

void copyIntegers(unsigned int count) {
  _copy(copyInteger, count);
}
void copyFloats(unsigned int count) {
  _copy(copyFloat, count);
}
void copyStrings(unsigned int count) {
  _copy(copyString, count);
}

unsigned int copyArray() {
  unsigned int count = parseInteger();
  startArray();
  return count;
}



void handleTable() {
  if (!strcmp(tableType, "PBTable")) {
    putKey("data"); startArray();
    while(1) {
      const char* type = parseString();
      if (type == NULL) {
        break;
      }
      startObject();
      putKey("type"); putString(type);
      putKey("data");
      bool isHDR_ps3 = !strcmp(type, "HDR_ps3");
      if (!strcmp(type, "HDR") || isHDR_ps3) {
        //FIXME: This is a temporary measure because I don't care about HDR
        while(1) {
          const char* unk = parseString();
          if (!strcmp(unk, "End")) {
            break;
          }
        }
        putNull();
      } else if (!strcmp(type, "Resources")) {
        startObject();
        putKey("start"); copyString();
        putKey("end"); copyString();
        putKey("models"); copyString();
        endObject();
      } else if (!strcmp(type, "BallTrough")) {
        startObject();
        putKey("switches");
        unsigned int count = copyArray();
        for(unsigned int i = 0; i < count; i++) {
          copyInteger();
        }
        endArray();
        endObject();
      } else if (!strcmp(type, "TiltSwitch")) {
        copyInteger(); // Switch index
      } else if (!strcmp(type, "Dil")) {
        startObject();
        putKey("resource"); copyString();
        putKey("index"); copyInteger();
        endObject();
      } else if (!strcmp(type, "Controls")) {
        putNull();
      } else if (!strcmp(type, "Gravity")) {
        startObject();
        // Yup.. "Unrivaled and critically acclaimed physics"
        putKey("slow"); copyFloats(3);
        putKey("angle"); copyFloat(); // Angle of table
        putKey("fast"); copyFloats(3);
        putKey("slowSpeed"); copyFloat();
        endObject();
      } else if (!strcmp(type, "SPU_FX")) {
        //FIXME: This is a temporary measure because I don't care about SPUs
        while(1) {
          const char* unk = parseString();
          if (!strcmp(unk, "End")) {
            break;
          }
        }
        putNull();
      } else if (!strcmp(type, "Display")) {
        const char* displayType = parseString();
        const char* resourceName = parseString();
        unsigned int symbolCount = parseInteger();
        for(unsigned int i = 0; i < symbolCount; i++) {
          const char* symbolType = parseString();
          if (!strcmp(symbolType, "Alpha")) {
            unsigned int x = parseInteger();
            unsigned int y = parseInteger();
          } else if (!strcmp(symbolType, "None")) {
            // NOP
          } else {
            printf("Unknown symbol type: '%s'\n", symbolType);
          } 
        }
        putNull();
      } else if (!strcmp(type, "LightingData")) {
        startObject();
        putKey("resource"); copyString();
        putKey("index"); copyInteger();
        putKey("lights"); startArray();
        while(1) {
          const char* lightType = parseString();
          if(!strcmp(lightType, "EndLightingData")) {
            break;
          }
          startObject();
          if(!strcmp(lightType, "LightON")) {
          } else if(!strcmp(lightType, "LightLamp")) {
          } else if(!strcmp(lightType, "LightFlasher")) {
          } else if(!strcmp(lightType, "LightGI1")) {
          } else if(!strcmp(lightType, "LightGI2")) {
          } else if(!strcmp(lightType, "LightGI3")) {
          } else if(!strcmp(lightType, "LightGI4")) {
          } else if(!strcmp(lightType, "LightGI5")) {
          } else {
            printf("Unknown light type '%s'\n", lightType);
          }
          putKey("type"); putString(lightType);
          putKey("index"); copyInteger(); // Called "num" in PBA files
          putKey("dilName"); copyString();
          putKey("dilIndex"); copyInteger();
          putKey("color"); copyFloats(3);
          putKey("unk0"); copyFloat();
          putKey("brightness"); copyFloat();
          endObject();
        }
        endArray();
        endObject();
      } else {
        printf("Unknown PBTable type: '%s'\n", type);
        putNull();
      }
      endObject();
    }
    endArray();
  }

  if (!strcmp(tableType, "PBObject")) {
    putKey("data"); startArray();
    while(1) {
      const char* type = parseString();
      if (type == NULL) {
        break;
      }
      startObject();
      putKey("type"); putString(type);
      putKey("data");
      if (!strcmp(type, "Object")) {
        //, PB_OBJECTTYPE_LAMPSET, 2589,
        startObject();
        putKey("type"); copyString();
        putKey("unk0"); copyInteger();
        endObject();
      } else if (!strcmp(type, "DilPos")) {
        startObject();
        putKey("dilName"); copyString();
        putKey("dilIndex"); copyInteger();
        endObject();
      } else if (!strcmp(type, "Solenoid")) {
        copyInteger();
      } else if (!strcmp(type, "Switch")) {
        copyInteger();
      } else if (!strcmp(type, "AffectPhysics")) {
        copyBoolean();
      } else if (!strcmp(type, "Pos")) {
        startObject();
        putKey("position"); copyFloats(3);
        putKey("scale"); copyFloats(3);
        // Assumptions:
        putKey("angle"); copyFloats(3);
        endObject();
      } else if (!strcmp(type, "Models")) {
        // 1, RSID_TBLACKKNIGHT2000_MODELS, 39,
        unsigned int count = copyArray();
        for(unsigned int i = 0; i < count; i++) {
          startObject();
          putKey("resource"); copyString();
          putKey("index"); copyInteger();
          endObject();
        }
        endArray();
      } else if (!strcmp(type, "BackglassRender")) {
        copyBoolean();
      } else if (!strcmp(type, "EnablePhysics")) {
        while(1) {
          const char* unk = parseString();
          if (!strcmp(unk, "End")) {
            break;
          }
        }
        putNull();
      } else if (!strcmp(type, "DeflectionModPhysics")) {
        copyFloats(11);
      } else if (!strcmp(type, "DownAccelPhysics")) {
        copyFloats(3);
      } else if (!strcmp(type, "LiveCatchPhysics")) {
        copyFloats(3);
      } else if (!strcmp(type, "UseSpecialRotation")) {
        copyBoolean();
      } else if (!strcmp(type, "SwitchEOS")) {
        copyInteger();
      } else if (!strcmp(type, "PlungerWire")) {
        copyInteger();
      } else if (!strcmp(type, "SolenoidSound")) {
        copyStrings(5);
      } else if (!strcmp(type, "EnvironmentModel")) {
        copyInteger();
      } else if (!strcmp(type, "Func")) {
        copyString();
      } else if (!strcmp(type, "BallStack")) {
        unsigned int count = copyArray();
        for(unsigned int i = 0; i < count; i++) {
          copyInteger();
        }
        endArray();
      } else if (!strcmp(type, "Link")) {
        copyString();
      } else if (!strcmp(type, "Links")) {
        unsigned int count = copyArray();
        for(unsigned int i = 0; i < count; i++) {
          copyString();
        }
        endArray();
      } else if (!strcmp(type, "Sound")) {
        startObject();
        putKey("type"); copyString();
        putKey("name"); copyString();
        endObject();
      } else if (!strcmp(type, "Lights")) {
        //, Def_LightCutouts,
        const char* variableName = parseString();
        putNull();
      } else if (!strcmp(type, "EnvMapReflection")) {
        copyBoolean();
      } else if (!strcmp(type, "Vibrate")) {
        //FIXME: This is a temporary measure because vibrate has a mode called "Collision"
        //FIXME: Implement!
        while(1) {
          const char* unk = parseString();
          if (!strcmp(unk, "End")) {
            break;
          }
        }
        putNull();
      } else if (!strcmp(type, "Flags")) {
        while(1) {
          const char* flag = parseString(); // Known: zWrite, zTest, End
          if (!strcmp(flag, "End")) {
            break;
          }
          const char* value = parseString();
          printf("Setting flag: '%s' = '%s'\n", flag, value);
        }
        putNull();
      } else if (!strcmp(type, "LampCombo")) {
        //Comment: cmd lampSet	lampNum	 onOffset		offOffset	RSID
        //, 1,	17,24,	 35,134,		34,			RSID_TBLACKKNIGHT2000_LAMP_TEXTURES,	EmuCombo,
        unsigned int set = parseInteger();
        unsigned int lampIndexA = parseInteger();
        unsigned int lampIndexB = parseInteger();
        unsigned int lampTextureOnA = parseInteger();
        unsigned int lampTextureOnB = parseInteger();
        unsigned int lampTextureOff = parseInteger();
        unsigned int resource = parseInteger();
        const char* type = parseString();
        putNull();
      } else if (!strcmp(type, "Lamp")) {
        //FIXME: This is kind of bad.. this can be repeating

        //, 1,		24,		 49,			48,			RSID_TBLACKKNIGHT2000_LAMP_TEXTURES,	EmuLamp, Smoothing,2,
        //, 6,		15,		31,				30,				RSID_TABLE_RIPLEYS_LAMP_TEXTURES,		EmuLamp,
        startObject();
        putKey("set"); copyInteger();
        putKey("index"); copyInteger();
        putKey("textureOn"); copyInteger();
        putKey("textureOff"); copyInteger();
        putKey("resource"); copyString();
        putKey("type"); copyString();
        endObject();
        //const char* unk0 = parseString(); // Known: "Smoothing"
        //unsigned int unk1 = parseInteger();
      } else if (!strcmp(type, "Collision")) {
        startObject();
        char* collisionType = parseString();
        putKey("type"); putString(collisionType);
        if (!strcmp(collisionType, "Mesh")) {
          putKey("data");
          unsigned int count = copyArray();
          for(unsigned int i = 0; i < count; i++) {
            startObject();
            putKey("resource"); copyString();
            putKey("index"); copyInteger();
            endObject();
          }
          endArray();
        } else if (!strcmp(collisionType, "Sphere")) {
          char* mode = parseString();
          putKey("mode"); putString(mode);
          if (!strcmp(mode, "Manual")) {
            putKey("position"); copyFloats(3);
            putKey("radius"); copyFloat();
          } else {
            printf("Unknown sphere mode: '%s'\n", mode);
            assert(false);
          }
          free(mode);
        } else {
          printf("Unknown collision type: '%s'\n", collisionType);
          assert(false);
        }
        free(collisionType);
        endObject();
      } else if (!strcmp(type, "Physics")) {
        while(1) {
          const char* field = parseString();
          if (!strcmp(field, "End")) {
            break;
          }

          bool isLeft = !strcmp(field, "Left");
          bool isRight = !strcmp(field, "Right");
          bool isUpperRight = !strcmp(field, "UpperRight");
          if (isLeft || isRight || isUpperRight) {
            float length = parseFloat();
            float angleSpeedDown = parseFloat();
            float staticElasticity = parseFloat();
            float dropElasticity = parseFloat();
            float bottomSpeedMult = parseFloat();
            unsigned int transferCount = parseInteger();
            for(unsigned int i = 0; i < transferCount; i++) {
              const char* type = parseString();
              assert(!strcmp(type, "TransferPoint"));
              float unk0 = parseFloat();
              float unk1 = parseFloat();
              float unk2 = parseFloat();
              float unk3 = parseFloat();
              float unk4 = parseFloat();
              float unk5 = parseFloat();
            }
            // This type always seems to stop parsing the physics?!
            break;
          } else if (!strcmp(field, "TrueElasticity")) {
            float value = parseFloat();
          } else if (!strcmp(field, "Elasticity")) {
            float value = parseFloat();
          } else if (!strcmp(field, "Friction")) {
            float value = parseFloat();
          } else if (!strcmp(field, "RollNormalVel")) {
            float value = parseFloat();
          } else if (!strcmp(field, "RollTransferAmt")) {
            float value = parseFloat();
          } else if (!strcmp(field, "MaxTransferVel")) {
            float value = parseFloat();
          } else if (!strcmp(field, "RollNormal")) {
            float value = parseFloat();
          } else if (!strcmp(field, "Gravity")) {
            // BK2K (and others?!) has a bug in its csv where it says:
            //   "Gravity,0,0,0,0 Dampening"
            // Other locations use the correct
            //   "Gravity,0,0,0,0, Dampening"
            // Obviously this will confuse the parser (mine at least)
            float x = parseFloat();          
            float y = parseFloat();          
            float z = parseFloat();          
            float w = parseFloat();    
          } else if (!strcmp(field, "Dampening")) {
            float x = parseFloat();          
            float y = parseFloat();          
            float z = parseFloat();          
          } else {
            printf("Unknown physics '%s'\n", field);
          }
          printf("Setting physics: '%s'\n", field);
        }
        putNull();
      } else if (!strcmp(type, "Vars")) {
        // First [out of 2] variable for Flipper fingers seems to be angle in up position
        unsigned int count = copyArray();
        for(unsigned int i = 0; i < count; i++) {
          copyFloat();
        }
        endArray();
      } else {
        printf("Unknown PBObject type: '%s'\n", type);
        putNull();
      }
      endObject();
    }
    endArray();
  }
}

void handleLightSet() {
  if (!strcmp(tableType, "LightSet")) {
    putKey("ambientColor"); copyFloats(4);

    putKey("lightColors");
    startArray();
    for(unsigned int i = 0; i < 3; i++) {
      copyFloats(4); // light i, rgba
    }
    endArray();

    putKey("specularColor"); copyFloats(4); // rgb, shininess

    putKey("lightPositions");
    startArray();
    for(unsigned int i = 0; i < 3; i++) {
      copyFloats(4); // light i, xyzw
    }
    endArray();

    putKey("specularPosition"); copyFloats(4);

    //FIXME: List remaining fields [expecting that this was all!]
  }
}

void handleCamera() {
  if (!strcmp(tableType, "PBCameras")) {
    putKey("data"); startArray();
    while(1) {
      const char* type = parseString();
      if (type == NULL) {
        break;
      }
      startObject();
      putKey("type"); putString(type);
      if (!strcmp(type, "camera")) {
        putKey("mode"); copyString();

        putKey("ratio"); copyString();

        // minX, maxX, minY, maxY, minZ, maxZ
        putKey("positionBounds"); copyFloats(6);

        // Yeah.. WTF: Weird order and no range for Y apparently:
        // xAngleMin, xAngleMax, yAngle, zAngle, zAngleMin, zAngleMax
        putKey("angleBounds"); copyFloats(6);

        putKey("zoom"); copyFloat();
        putKey("vd"); copyFloat(); // Unknown what this means?!

        // Not sure how these work
        putKey("ballOffset"); copyFloats(2); // YZ only?!

        // Not sure how these work
        putKey("spring"); startObject();
        putKey("force"); copyFloat();
        putKey("dampening"); copyFloat();
        putKey("length"); copyFloat();
        endObject();

      } else if (!strcmp(type, "TRANSITION_VALUES")) {
        // minX, maxX, minY, maxY
        copyFloats(4);
      } else {
        printf("Unknown PBCameras type: '%s'\n", type);
      }
      endObject();
    }
    endArray();
  }

  if (!strcmp(tableType, "PBFlyByCamera")) {
    putKey("data"); startArray();
    while(1) {
      const char* type = parseString();
      if (type == NULL) {
        break;
      }
      startObject();
      putKey("type"); putString(type);
      bool isStart = !strcmp(type, "Start");
      if (isStart || !strcmp(type, "Waypoint")) {
        putKey("position"); copyFloats(3);
        putKey("angle"); copyFloats(3);
        putKey("moveRate"); copyFloat();
      } else {
        printf("Unknown PBFlyByCamera type: '%s'\n", type);
      }
      endObject();
    }
    endArray();
  }
}

void handleInstructions() {
  if (!strcmp(tableType, "ChapterText")) {
    while(1) {
      const char* type = parseString();
#if 0
      printf("Reading type: '%s'\n", type);
#endif
      if (type == NULL) {
        break;
      }
      putKey(type);
      if (!strcmp(type, "Chapter")) {
        startObject();
        putKey("index"); copyInteger();
        putKey("title"); copyString();
        endObject();
      }
    }
  } else if (!strcmp(tableType, "Instruction")) {
    while(1) {
      const char* type = parseString();
      if (type == NULL) {
        break;
      }
      putKey(type);
      if (!strcmp(type, "ChapterNum")) {
        copyInteger();
      } else if (!strcmp(type, "Camera")) {
        startObject();
        putKey("position"); copyFloats(3);
        putKey("angle"); copyFloats(3);
        putKey("zoom"); copyFloat();
        putKey("vd"); copyFloat(); // Unknown what this means?!
        endObject();
      } else if (!strcmp(type, "Arrow")) {
        startObject();
        putKey("position"); copyFloats(3);
        putKey("angle"); copyFloats(3);
        putKey("scale"); copyFloats(3);
        endObject();
      } else if (!strcmp(type, "Text")) {
        copyString();
      }  else {
        printf("Unknown Instruction type: '%s'\n", type);
      }
    }
  }
}

char* cut(const char* from, const char* to, bool trim) {
  // Skip bytes on the left
  if (trim) {
    while(isSpace(*from++));
    from--;
  }

  // Go to end of `to` if we don't specify an absolute end
  if (to == NULL) {
    to = &from[strlen(from) - 1];
  }

  // Skip bytes on the right
  if (trim) {
    while(isSpace(*to--));
    to++;
  }
  
  size_t length = to - from + 1;
  char* buffer = malloc(length + 1);
  memcpy(buffer, from, length);
  buffer[length] = '\0';
  return buffer;
}

int main(int argc, char* argv[]) {
  FILE* in = fopen(argv[1], "rb");
  out = fopen(argv[2], "wb");

  f = in;
  startObject(); // JSON always starts with an object
  while(!feof(in)) {

    const char* identifier = parseString();
repeat:
    if (identifier == NULL) {
      break;
    }

    printf("Got '%s'\n", identifier);
    if (!strncmp(identifier, "Table", 5)) {
      // Split into LHS and RHS of "="
      const char* assign = strchr(identifier, '=');
      assert(assign != NULL);
      const char* lhs = cut(identifier, assign - 1, true);
      assert(!strcmp(lhs, "Table")); // Make sure there is no other garbage here
      const char* rhs = cut(assign + 1, NULL, true);
      // Split the RHS at the ":"
      const char* colon = strchr(rhs, ':');
      assert(colon != NULL);
      tableType = cut(rhs, colon - 1, true);
      tableName = cut(colon + 1, NULL, true);

      printf("Starting Table '%s' type '%s'\n", tableName, tableType);
      putKey(tableName); startObject();
      putKey("type");
      putString(tableType);
    }

    if (tableType != NULL) {
      handleTable();
      handleInstructions();
      handleLightSet();
      handleCamera();

#if 0
      while((identifier = parseString()) != NULL) {
        printf("Unknown identifier: '%s'\n", identifier);
      }
#endif

      printf("Table over!\n");
      endObject();
      tableType = NULL;
    } else {
      // This is the mode in which this tool parses resource files..
      // This should probably be in a different file

      //FIXME: There is a bug where the last line is duplicated..

      printf("without Table: '%s'\n", identifier);
      
      putKey(identifier);
      startObject();
      putKey("index"); copyInteger();
#if 0
      // This is usually ",,," in the file but this parser currently skips empty fields!
      putKey("unk0"); copyString();
      putKey("unk1"); copyString();
      putKey("unk2"); copyString();
#endif
      putKey("files");
      startObject();
      while(1) {
        identifier = parseString();
        if (identifier == NULL) {
          break;
        }
        if (!strcmp(identifier, "[Offset]")) {
#if 0
          startObject();
          putKey("name"); copyString();
          putKey("index"); copyInteger();
          endObject();
#else
          identifier = parseString();
          putKey(identifier); copyInteger();
#endif
        } else {
          break;
        }
      }
      putKey("null"); putNull(); // FIXME: Get rid of this..
      endObject();

      endObject();

      goto repeat; // Continue with the chunk we just read
    }
  }

  endObject();
  fprintf(out, "\n");

  fclose(in);
  fclose(out);

  return 0;
}
