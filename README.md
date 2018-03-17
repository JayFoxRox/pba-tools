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

# Donate

If you like my work, a donation would be nice:

* [Patreon](https://www.patreon.com/jayfoxrox)
* [PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=x1f3o3x7x%40googlemail%2ecom&lc=GB&item_name=Jannik%20Vogel%20%28JayFoxRox%29&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHostedGuest)

Even a small amount does help me and shows appreciation. Thank you!

# License

(c) 2017 Jannik Vogel (JayFoxRox)
All rights reserved.

* I hereby explicitly allow you to look at the source-code of pba-tools.
* You are also allowed to download my code, compile my code and execute the generated binary.
* For scripts you are allowed to interpret them using the respective scripting languages interpreter.
* You are *explicitly* **not allowed to modify** any of my code.
* You are *explicitly* **not allowed to link** (in any possible way) my code (or any derivative, such as the binary) against other software (except system libraries) either.
* You are *explicitly* **not allowed to redistribute** my code (or any derivative, such as the binary) anywhere.

This means you are not allowed to re-use this code (or any derivative work) for any other projects.


Want this changed? License is TBD.
If you have license recommendations, tell me about it in GitHub Issue #1
