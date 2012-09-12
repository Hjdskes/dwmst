dwmStatus: status.c
	gcc -o dwmStatus status.c -lX11

install: 
	install -D dwmStatus $(DESTDIR)/usr/bin/dwmStatus

clean:
	rm dwmStatus
