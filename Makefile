dwm-status: dwmst.c
	gcc -o dwmst dwmst.c `pkg-config --cflags --libs x11 alsa glib-2.0 dbus-glib-1 audclient` -liw

clean:
	rm dwmst
