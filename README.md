# pimpmypcbnewsvg
simple SVG merger for KiCad/pcbnew

want to print your PCB layout on paper but the colors are nasty?
what if you could have most layers almost white, while the F.SilkS or F.Fab layer black?

this tool can help
first, in pcbnew:
- export your PCB layout as SVG
- in the export options choose "one file per layer" and "black and white"
- it might be a good idea to create and use a subfolder /svg/ to not polute your project main folder
pcbnew will generate files for each layer, with the layer name appended to the filename
- pass any one of them (e.g. "/home/user/somewhere/myLEDblinker/svg/myLEDblinker-F.Cu.svg") as commandline argument to the tool (or drag and drop over it (this works on windows)) and the tool will figure out the basename "/home/user/somewhere/myLEDblinker/svg/myLEDblinker-" and try to load the svg files of the other layers too
the colors will be changed, and the layers will be aranged in a specific order, configurable from the pimpmypcbnewsvg.xml file, and the merged result will be saved into the same folder

this sort of works with pcbnew from the stable kicad version right now (4.0.something)
the pcb layers graphics in the exported SVGs are not as accurate as the gerbers!

--------

pimpmypcbnewsvg requires tinyxml2 and a C++11 compiler, i think it should build easily on linux, windows, etc..
pimpmypcbnewsvg comes with a Code::Blocks project with a linux build target (so far)
tinyxml2 must be placed in the project folder under its own folder /tinyxml2/

the executable looks for "pimpmypcbnewsvg.xml" which is used as a configuration file
it should contain information about two things:
- what are the exact names of the layers (so that it can substitute them in the file names) and their default colors
- one or more "output file" setups, each describing the output file suffix (including the svg extension) which layers it should contain, in what order and with what colors

---------

it seems pcbnew "kind of" exports the drill holes inside the copper layer SVG files as white color (while the copper itself is black) so pimpmypcbnewsvg has the functionality to try and extract it sepparately as a virtual layer
