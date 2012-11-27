dwm-status: dwm-status.c
	gcc -o dwm-status dwm-status.c `pkg-config --cflags --libs x11 libmpdclient` -liw -lasound

clean:
	rm dwm-status
