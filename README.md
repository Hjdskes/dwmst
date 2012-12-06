FORKED FROM TRILBY WHITE'S DWMSTATUS: https://github.com/TrilbyWhite/dwmStatus

This is a hardcoded statusbar for my system and, like Trilby's dwmStatus from which this is forked, it was not meant to be flexible.  Flexibility costs memory and processor time. This can, however, be used as a template for your own status bar app.

Note that this was written to work with the statuscolors patch. The colors are int format strings of the sprintf commands. However, as they are mostly nonprintable they can show up oddly, or not at all, depending on your editor. To remove the dependecy on statuscolors simply remove these characters from the format strings.

Also note that from now on there's two versions: one that works with Audacious and one that works with MPD. All you have to do is adjust the Makefile.
[*]For Audacious, the libs are: glib-2.0 dbus-glib-1 audclient
[*]For MPD, the libs are: libmpdclient

ToDo:
[*]Make a header file for cleanliness and practice;
[*]Get make to recognise whether we want MPD or Audacious and have only one dwmst.c file.
