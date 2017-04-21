This is a set of tools to operate on The Pinball Arcade (PBA) files.
PBA is a video-game developed by [FarSight Studios](http://farsightstudios.com/).

![A PBA table loaded in Blender](http://i.imgur.com/5tLnvJG.png)

Most of the tools in this repository convert to [JSON](http://json.org), TGA or OBJ/MTL.

This tool is not able to *create* any kind of PBA files yet!
It merely loads / converts PBA files to more usable file formats at the moment.

# Compilation / Running

First install all dependencies:

* libtxc_dxtn

Then run the following commands to clone and compile

```
git clone https://github.com/JayFoxRox/pba-tools.git
cd pba-tools
mkdir build
cd build
cmake ..
make -j 4
```

The tools will be available in the build folder.

# License

License is TBD.
Consider it (c) 2017 Jannik Vogel (JayFoxRox) until then.

(I hereby explicitly allow you to look at it, use it but not re-use code for other projects)
If you have license recommendations, tell me about it in GitHub Issue #1
