#!/bin/bash

inRezPath=$1
inEnumPath=$2
outPath=$3

tmpHack=$4 # Id of the resource which contains the table in slot 1 to use

BIN_DIR=.
SCRIPT_DIR=$BIN_DIR/scripts

# FIXME: Convert enum to json
mkdir -p $outPath
$BIN_DIR/pba-csv $inEnumPath $outPath/enums.json

# FIXME: Unpack table
mkdir -p $outPath/unpacked
$BIN_DIR/pba-rez $inRezPath $outPath/unpacked

# FIXME: Convert table
mkdir -p $outPath/converted
echo "$SCRIPT_DIR/convert.sh $outPath/unpacked $outPath/converted"
bash $SCRIPT_DIR/convert.sh $outPath/unpacked $outPath/converted

# Extra copy of the placement and table file [a bit hacky..]
$BIN_DIR/format002A $outPath/unpacked/*-0.002A $outPath/dil.json
$BIN_DIR/pba-csv $outPath/converted/$tmpHack-1.csv $outPath/table.json

echo "blender --python $SCRIPT_DIR/blender.py -- $outPath"
