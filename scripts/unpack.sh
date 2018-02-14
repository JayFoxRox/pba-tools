#!/bin/bash

inPath=$1
outPath=$2

BIN_DIR=.

# Exit in case of failure
set -e

# Handle empty lists
shopt -s nullglob

# Case insensitive matching (some files are *.Rez, others are *.rez)
shopt -s nocaseglob

# Mostly working:

mkdir -p $outPath
for filename in $inPath/*.rez; do
  echo "$filename"
  $BIN_DIR/pba-rez "$filename" unpacked-pc11
done

echo "Success!"
