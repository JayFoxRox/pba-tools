This is a set of tools to operate on The Pinball Arcade (PBA) files.
PBA is a video-game developed by [FarSight Studios](http://farsightstudios.com/).

Most of the tools in this repository convert to [JSON](http://json.org), TGA or OBJ/MTL.

This tool is not able to create any kind of PBA files yet!

# PBA Coordinate System

Their coordinate system seems to be (from a players perspective):

  +X = Right
  +Y = Forward
  +Z = Up

The playfield center (including vertical elements) is usually located at 0.0, 0.0, 0.0.
Based on dimensions of Medieval madness the PBA coordinate system is roughly:

1 unit = 0.01905cm = 0.0075inch

Human-readable angles are usually in degree, machine readable are usually in radians.

# PBA bugs / issues / facts

## General
  - Tables contain wav versions of the emulation sounds at varying quality
  - Reflections are hardcoded onto textures, sometimes in image-space
  - Texture resolution is absolutely horrible
  - Lamps and Flashers are simulated by switching between on/off textures
  - Some Flashers have a mesh for their light-shine around them
  - Apron missing hole for ball trough which is not simulated
  - Subways and other "hidden" objects not modelled / simulated
  - Plunger lane is not carved
  - Missing playfield glass
  - Meshes are already instanced and embedded
  - Physics are actually arcade physics, not simulated physics
  - Lots of redundancy in each file and across files
  - Cameras are hardcoded for each device instead of defining a frustum

## BK2K
  - Playfield texture contains lamps, walls and wires
  - Main resource has typos / bug: "Gravity,0,0,0,0 Dampening" (missing comma)
  - Spring (slows ball down) on wire after W-A-R not simulated

## Ripley's
  - 2704/29: Missing alpha

## Medieval Madness
  - Right Ramp contains a wire piece from the left ramp (vertical connection at the very bottom)
