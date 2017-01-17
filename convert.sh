#!/bin/bash


# Exit in case of failure
set -e

# Handle empty lists
shopt -s nullglob


BIN_DIR=.

inPath=./unpacked-pc11
outPath=./converted-pc11

mkdir -p $outPath

# WIP Stuff:

for filename in $inPath/*.002C; do
  echo "$filename"
  $BIN_DIR/format002C "$filename" "$outPath/$(basename "$filename" .002C).obj"
done

#exit

# FIXME: Only using -0.002A because -1.002A might be broken PS4 DILs
for filename in $inPath/*-0.002A; do
  echo "$filename"
  $BIN_DIR/format002A "$filename"  "$outPath/$(basename "$filename" .002A).json"
done


# Mostly working:

for filename in $inPath/*.0001; do
  echo "$filename"
  $BIN_DIR/format0001 "$filename"  "$outPath/$(basename "$filename" .0001).csv"
done

for filename in $inPath/*.0022; do
  echo "$filename"
  $BIN_DIR/format0022 "$filename"  "$outPath/$(basename "$filename" .0022).tga"
done

for filename in $inPath/*.0024; do
  echo "$filename"
  $BIN_DIR/format0024 "$filename"  "$outPath/$(basename "$filename" .0024).obj"
done

# DX11 models?!
for filename in $inPath/*.008E; do
  echo "$filename"
done

# DX11 textures?!
for filename in $inPath/*.008D; do
  echo "$filename"
  $BIN_DIR/format008D "$filename"  "$outPath/$(basename "$filename" .008D).jpg"
done

echo "Success!"
