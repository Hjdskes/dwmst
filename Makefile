PROG      = dwmst
CC        = gcc
PREFIX   ?= /usr/local
BINPREFIX = ${PREFIX}/bin

LIBS      = -lasound -lX11 `pkg-config --libs libmpdclient`
CFLAGS   += -std=c99 -pedantic -Wall -Wextra

debug: CFLAGS += -O0 -g
debug: ${PROG}

${PROG}: ${PROG}.c ${PROG}.h
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

install:
	install -Dm755 ${PROG} ${DESTDIR}${BINPREFIX}/${PROG}
	install -Dm644 ${PROG}@.service ${DESTDIR}/usr/lib/systemd/system/${PROG}@.service

uninstall:
	rm -f ${BINPREFIX}/${PROG}
	rm -f /usr/lib/systemd/system/${PROG}@.service

clean:
	rm -f ${PROG}

.PHONY: all debug clean install uninstall
