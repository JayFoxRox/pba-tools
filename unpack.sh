#!/bin/bash


# Exit in case of failure
set -e

# Handle empty lists
shopt -s nullglob

# Case insensitive matching (some files are *.Rez, others are *.rez)
shopt -s nocaseglob

BIN_DIR=.

# Mostly working:

mkdir -p unpacked-pc11
for filename in resources-pc11/*.rez; do
  $BIN_DIR/pba-rez "$filename" unpacked-pc11
done

exit

mkdir -p unpacked
for filename in resources/*.rez; do
  echo "$filename"
  $BIN_DIR/pba-rez "$filename" unpacked
done

echo "Success!"
