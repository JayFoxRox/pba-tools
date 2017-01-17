This is a set of tools to operate on The Pinball Arcade (PBA) files.
PBA is a video-game developed by [FarSight Studios](http://www.farsightstudios.com/).

# PBA Coordinate System

Their coordinate system seems to be (from a players perspective):

  +X = Right
  +Y = Forward
  +Z = Up

The playfield center (including vertical elements) is usually located at 0.0, 0.0, 0.0.

Each unit appears to be roughly 400/3 inch (guessed).
So a 20.5" x 42.0" playfield would be 2733 x 5600 in game units.

Human-readable angles are usually in degree, machine readable are usually in radians.

# PBA Bugs

2704/29: Missing alpha
