PROG      = dwmst
CC        = gcc
PREFIX   ?= /usr/local
BINPREFIX = ${PREFIX}/bin

LIBS     =  -liw -lasound -lX11 `pkg-config --cflags --libs glib-2.0 dbus-glib-1 audclient`
CFLAGS   =  -O3 -pedantic -Wall -Wextra -Wno-format-zero-length

${PROG}: ${PROG}.c ${PROG}.h
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

debug: CFLAGS += -O0 -g
debug: ${PROG}

install:
	install -Dm755 ${PROG} ${DESTDIR}${BINPREFIX}/${PROG}
	install -Dm644 ${PROG}@.service ${DESTDIR}/usr/lib/systemd/system/${PROG}@.service

uninstall:
	rm -f ${BINPREFIX}/${PROG}
	rm -f /usr/lib/systemd/system/${PROG}@.service

clean:
	rm -f ${PROG}

.PHONY: all debug clean install uninstall
