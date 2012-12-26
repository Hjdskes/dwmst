**Forked from TrilbyWhite's dwmStatus:** https://github.com/TrilbyWhite/dwmStatus

This is a hardcoded statusbar for my system and, like Trilby's dwmStatus from which this is forked, it was not meant to be flexible.  Flexibility costs memory and processor time. This can, however, be used as a template for your own status bar app.

Note that this was written to work with the statuscolors patch. The colors are int format strings of the sprintf commands. However, as they are mostly nonprintable they can show up oddly, or not at all, depending on your editor. To remove the dependecy on statuscolors simply remove these characters from the format strings.

The program supports two music players (MPD and Audacious). You can choose which to support by editing the Makefile before compiling. Just comment out the LIB and FLAG lines of the music player you don't want.

To (re)launch dwmst after suspending your computer, you have to enable the systemd service file:  
`systemctl enable dwmst.service`

In order to succesfully compile this, you need to have the following dependencies installed:
* libx11
* wireless_tools
* alsa-lib
* Optional dependencies are, obviously, MPD and Audacious.

ToDo:
* Make a header file for cleanliness and practice;
