PROG     =  dwmst
PREFIX  ?= /usr/local

# MPD, comment if you don't want it
#MPDLIB   =  -lmpdclient
#MPDFLAG  =  -DMPD

# AUDACIOUS, comment if you don't want it
AUDLIB	 =  `pkg-config --cflags --libs glib-2.0 dbus-glib-1 audclient`
AUDFLAG  =  -DAUD

# Clock, comment if you don't want it
#CLKFLAG  =  -DCLK

LIBS     =  -liw -lasound -lX11 ${MPDLIB} ${AUDLIB}
CPPFLAGS =  ${MPDFLAG} ${AUDFLAG} ${CLKFLAG}
CFLAGS   =  -Os -Wall -Wextra -pedantic -Wno-format-zero-length -Wno-unused-parameter -Wno-unused-result ${CPPFLAGS}

${PROG}: ${PROG}.c ${PROG}.h
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

clean:
	rm -f ${PROG}

install:
	install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
	install -Dm644 ${PROG}.service ${DESTDIR}/usr/lib/systemd/system/${PROG}.service

uninstall:
	rm -f ${PREFIX}/bin/${PROG}
	rm -f /usr/lib/systemd/system/${PROG}.service
