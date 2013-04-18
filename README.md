DWMST
=====

**Forked from TrilbyWhite's dwmStatus:** https://github.com/TrilbyWhite/dwmStatus

This is a hardcoded statusbar for my system and, like Trilby's dwmStatus from which this is forked, it was not meant to be flexible.  Flexibility costs memory and processor time. This can, however, be used as a template for your own status bar app.

Note that this was written to work with the statuscolors patch. The colors are int format strings of the sprintf commands. However, as they are mostly nonprintable they can show up oddly, or not at all, depending on your editor. To remove the dependecy on statuscolors simply remove these characters from the format strings.

Installation
------------

In order to succesfully compile this, you need to have the following dependencies installed:
* `libx11`
* `wireless_tools`
* `alsa-lib`

The program supports two music players (MPD and Audacious). You can choose which to support by editing the Makefile before compiling. Just comment out the LIB and FLAG lines of the music player you don't want. You will also need certain dependencies to support this output:
* `libmpdclient` for MPD;
* `audacious` for Audacious.

To built and install, simply run

	$ make
	# make clean install

To (re)launch dwmst after suspending your computer, you have to enable the systemd service file:
`systemctl enable dwmst.service`

ToDo
----
* See if there's a better way to implement wired connection checks
* Re-check event sounds
* Code left to clean up:
	* MPD;
	* Alsa;	
