PROG     =  dwmst
PREFIX   =  /usr/local

# MPD, comment if you don't want it
#MPDLIB   =  -lmpdclient
#MPDFLAG  =  -DMPD

# AUDACIOUS, comment if you don't want it
AUDLIB	 =  `pkg-config --cflags --libs glib-2.0 dbus-glib-1 audclient`
AUDFLAG  =  -DAUD

LIBS     =  -liw -lasound -lX11 ${MPDLIB} ${AUDLIB}
CPPFLAGS =  ${MPDFLAG} ${AUDFLAG}
CFLAGS   =  -Os -Wall -Wno-unused-parameter -Wno-unused-result ${CPPFLAGS}

$(PROG): $(PROG).c
	@$(CC) $(CFLAGS) $(LIBS) -o $(PROG) $(PROG).c
	@strip $(PROG)

clean:
	rm -f $(PROG)

