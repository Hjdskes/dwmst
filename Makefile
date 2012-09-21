status: status.c
	gcc -g -o status status.c `pkg-config --cflags --libs x11 libmpdclient` -liw

clean:
	rm status
