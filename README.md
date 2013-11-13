DWMST
=====

**Forked from TrilbyWhite's dwmStatus:** https://github.com/TrilbyWhite/dwmStatus

A great inspiration for many code optimizations has been OK100. Thanks!

This is a hardcoded statusbar for my system and, like Trilby's dwmStatus from which this is forked, it was not meant to be flexible.  Flexibility costs memory and processor time. This can, however, be used as a template for your own status bar app.

Installation
------------

In order to succesfully compile this, you need to have the following dependencies installed:
* `libx11`
* `alsa-lib`
* `audacious`

To built and install, simply run

	$ make
	# make clean install

To (re)launch dwmst after suspending your computer, you have to enable the systemd service file:
`systemctl enable dwmst@<yourusername>.service`

ToDo
----
* Code left to clean up:
	* Alsa;	
* Improve handling when no battery is present
* Look into empty spaces when Skype/Network is not present
