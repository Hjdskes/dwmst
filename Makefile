dwmStatus: status.c
	gcc -g -o dwmStatus status.c -lX11

clean:
	rm dwmStatus
