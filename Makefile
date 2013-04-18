PROG      =  dwmst
CC        = gcc
PREFIX   ?= /usr/local
BINPREFIX = ${PREFIX}/bin

# MPD, comment if you don't want it
#MPDLIB   =  -lmpdclient
#MPDFLAG  =  -DMPD

# AUDACIOUS, comment if you don't want it
AUDLIB	 =  `pkg-config --cflags --libs glib-2.0 dbus-glib-1 audclient`
AUDFLAG  =  -DAUD

LIBS     =  -liw -lasound -lX11 -lcanberra ${MPDLIB} ${AUDLIB}
CPPFLAGS =  ${MPDFLAG} ${AUDFLAG} ${CLKFLAG}
CFLAGS   =  -Os -Wall -Wextra ${CPPFLAGS}

${PROG}: ${PROG}.c ${PROG}.h
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

debug: CFLAGS += -O0 -g -pedantic
debug: ${PROG}

install:
	install -Dm755 ${PROG} ${DESTDIR}${BINPREFIX}/${PROG}
	install -Dm644 ${PROG}.service ${DESTDIR}/usr/lib/systemd/system/${PROG}.service

uninstall:
	rm -f ${BINPREFIX}/${PROG}
	rm -f /usr/lib/systemd/system/${PROG}.service

clean:
	rm -f ${PROG}

.PHONY: all debug clean install uninstall
