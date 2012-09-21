dwm-status: dwm-status.c
	gcc -g -o dwm-status dwm-status.c `pkg-config --cflags --libs x11 libmpdclient` -liw

clean:
	rm dwm-status
