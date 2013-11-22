PROG      = dwmst
CC        = gcc
PREFIX   ?= /usr/local
BINPREFIX = ${PREFIX}/bin

LIBS      = -lasound -lX11 `pkg-config --cflags --libs glib-2.0 dbus-glib-1 audclient`
CFLAGS   += -pedantic -Wall -Wextra

debug: CFLAGS += -O0 -g
debug: ${PROG}

${PROG}: ${PROG}.c ${PROG}.h
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

install:
	install -Dm755 ${PROG} ${DESTDIR}${BINPREFIX}/${PROG}
	install -Dm644 ${PROG}.sh ${DESTDIR}/usr/lib/systemd/system-sleep/${PROG}.sh

uninstall:
	rm -f ${BINPREFIX}/${PROG}
	rm -f /usr/lib/systemd/system-sleep/${PROG}.sh

clean:
	rm -f ${PROG}

.PHONY: all debug clean install uninstall
