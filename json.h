// JSON helper routines
//
// (C) 2017 Jannik Vogel
//
// Licensed under GPLv2 or later.
// See LICENSE.txt for more information.

static unsigned int nesting = 0;
static bool isFirst = true; // Wether an element is the first within the object / array

void putComma() {
  if (!isFirst) {
    fprintf(out, ", ");
  } else {
    isFirst = false;
  }
}

// Internal use only!
void _putEscapedString(const char* value) {
  const char* cursor = value;
  fputc('"', out);
  while(*cursor != '\0') {
    // Escape quotes (end of string) and backslashes (escape symbol)
    if((*cursor == '"') || (*cursor == '\\')) {
      fputc('\\', out);
    }
    fputc(*cursor++, out);
  }
  fputc('"', out);
}

void putNull() {
  putComma();
  fprintf(out, "null");
}

void putString(const char* value) {
  putComma();
  _putEscapedString(value);
}

void putInteger(long long value) {
  putComma();
  fprintf(out, "%lld", value);
}

void putFloat(float value) {
  putComma();
  fprintf(out, "%f", value);
}

void putNesting() {
  for(unsigned int i = 0; i < nesting; i++) {
    fprintf(out, "    "); // Let the "spaces vs. tabs" var begin!
  }
}

void putKey(const char* key) {
  // This is generated errornously once as we didn't have a key to close
  if (!isFirst) {
    fprintf(out, ",\n");
  }
  putNesting();
  _putEscapedString(key);
  fprintf(out, ": ");
  isFirst = true;
}

//FIXME: Rename to "Table" to stay in PBA lingo here?!
void startObject() {
  putComma();
  fprintf(out, "{\n");
  nesting++;
  isFirst = true;
}
void endObject() {
  nesting--;
  fprintf(out, "\n");
  putNesting();
  fprintf(out, "}");
}

//FIXME: Make it possible to have this either inline or downwards
void startArray() {
  putComma();
  fprintf(out, "[ ");
  isFirst = true;
}
void endArray() {
  fprintf(out, " ]");
}
