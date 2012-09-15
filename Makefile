dwmStatus: status.c
	gcc -g -o dwmStatus status.c -lX11 -liw `pkg-config --libs libmpdclient`

clean:
	rm dwmStatus
