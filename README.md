# pimpmypcbnewsvg
Simple SVG merger for KiCad/pcbnew

Want to print your PCB layout on paper but the colors are nasty?
What if you could have most layers almost white, while the F.SilkS or F.Fab layer black?

This tool can help
First, in pcbnew:
- export your PCB layout as SVG
- in the export options choose "one file per layer" and "black and white"
- it might be a good idea to create and use a subfolder /svg/ to not polute your project main folder
pcbnew will generate files for each layer, with the layer name appended to the filename
- pass any one of them (e.g. "/home/user/somewhere/myLEDblinker/svg/myLEDblinker-F.Cu.svg") as commandline argument to the tool (or drag and drop over it (this works on windows)) and the tool will figure out the basename "/home/user/somewhere/myLEDblinker/svg/myLEDblinker-" and try to load the svg files of the other layers too
the colors will be changed, and the layers will be aranged in a specific order, configurable from the pimpmypcbnewsvg.xml file, and the merged result will be saved into the same folder

This sort of works with pcbnew from the stable kicad version right now (4.0.something)
The pcb layers graphics in the exported SVGs are not as accurate as the gerbers!

--------

pimpmypcbnewsvg has the following dependencies: tinyxml2, cmake, and a  C++11 capable compiler (recent versions of clang, gcc, etc.).

To build, perform:

```bash
mkdir build
cd build
cmake ..
make install
```

The pimpmypcbnewsvg executable will installed into the root of the source directory, where you can run it via `./pimpmypcbnewsvg`.

The executable looks for "pimpmypcbnewsvg.xml" which is used as a configuration file
It should contain information about two things:
- what are the exact names of the layers (so that it can substitute them in the file names) and their default colors
- one or more "output file" setups, each describing the output file suffix (including the svg extension) which layers it should contain, in what order and with what colors

---------

It seems pcbnew "kind of" exports the drill holes inside the copper layer SVG files as white color (while the copper itself is black) so pimpmypcbnewsvg has the functionality to try and extract it sepparately as a virtual layer
