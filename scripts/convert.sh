#!/bin/bash

inPath=$1
outPath=$2

BIN_DIR=.

# Exit in case of failure
set -e

# Handle empty lists
shopt -s nullglob


#mkdir -p $outPath

# WIP Stuff:

# DX11 models?!
for filename in $inPath/*.008E; do
  echo "$filename"
  $BIN_DIR/format0024 "$filename"  "$outPath/$(basename "$filename" .008E).obj"
done

# PC11 Sound
for filename in $inPath/*.008F; do
  echo "$filename"
  #$BIN_DIR/format008F "$filename"  "$outPath/$(basename "$filename" .008F).wav"
done

# Mostly working:

# Data [CSV or roms]
for filename in $inPath/*.0001; do
  echo "$filename"
  $BIN_DIR/format0001 "$filename"  "$outPath/$(basename "$filename" .0001).csv"
done

# Textures
for filename in $inPath/*.0022; do
  echo "$filename"
  $BIN_DIR/format0022 "$filename"  "$outPath/$(basename "$filename" .0022).tga"
done

# Models
for filename in $inPath/*.0024; do
  echo "$filename"
  $BIN_DIR/format0024 "$filename"  "$outPath/$(basename "$filename" .0024).obj"
done

# Sounds
for filename in $inPath/*.0025; do
  echo "$filename"
  $BIN_DIR/format0025 "$filename"  "$outPath/$(basename "$filename" .0025).wav"
done

for filename in $inPath/*.002A; do
  echo "$filename"
  $BIN_DIR/format002A "$filename"  "$outPath/$(basename "$filename" .002A).json"
done

# Collisions
for filename in $inPath/*.002C; do
  echo "$filename"
  $BIN_DIR/format002C "$filename" "$outPath/$(basename "$filename" .002C).obj"
done

# PC11 Textures
for filename in $inPath/*.008D; do
  echo "$filename"
  $BIN_DIR/format008D "$filename"  "$outPath/$(basename "$filename" .008D).jpg"
done

echo "Success!"
