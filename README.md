FORKED FROM TRILBY WHITE'S DWMSTATUS: https://github.com/TrilbyWhite/dwmStatus

This is a hardcoded statusbar for my system and, like Trilby's dwmStatus from which this is forked, it was not meant to be flexible.  Flexibility costs memory and processor time. This can, however, be used as a template for your own status bar app.

Note that this was written to work with the statuscolors patch. The colors are int format strings of the sprintf commands. However, as they are mostly nonprintable they can show up oddly, or not at all, depending on your editor. To remove the dependecy on statuscolors simply remove these characters from the format strings.

The program supports two music players (MPD and Audacious). You can choose which to support at compile time. For example, if you want MPD built it as follows:
	`make MPD=1`
Or, if you want Audacious support:
	`make AUD=1`

ToDo:
* Make a header file for cleanliness and practice;
* Get another string for Audacious/MPD; we don't need polling every second.