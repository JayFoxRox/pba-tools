#!/bin/bash


# Exit in case of failure
set -e


BIN_DIR=build

mkdir -p converted

# Mostly working:

for filename in unpacked/*.0001; do
  echo "$filename"
  $BIN_DIR/format0001 "$filename" "converted/$(basename "$filename" .0001).csv"
done

for filename in unpacked/*.0022; do
  echo "$filename"
  $BIN_DIR/format0022 "$filename" "converted/$(basename "$filename" .0022).tga"
done

for filename in unpacked/*.0024; do
  echo "$filename"
  $BIN_DIR/format0024 "$filename" "converted/$(basename "$filename" .0024).obj"
done

# No conversion yet:

for filename in unpacked/*.002A; do
  echo "$filename"
  $BIN_DIR/format002A "$filename"
done

# WIP Stuff:

mkdir -p tmp

for filename in unpacked/*.002C; do
  echo "$filename"
  $BIN_DIR/format002C "$filename" "tmp/$(basename "$filename" .002C).obj"
done

echo "Success!"
