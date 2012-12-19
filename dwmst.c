#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <iwlib.h>
#include <alsa/asoundlib.h>
#ifdef MPD
#include <mpd/client.h>
#endif
#ifdef AUD
#include <audacious/dbus.h>
#include <audacious/audctrl.h>
#endif

#define WIFI			"wlan0"		// Wireless interface
#define BATT_LOW		11			// Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL		1			// Sleeps for INTERVAL seconds between updates
#define VOL_CH			"Master"	// Channel to watch for volume
// Files read for system info:
#define SKYPE_FILE		"/home/jente/.Skype/jente_etnej/main.lock"
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
// Display format strings. Defaults make extensive use of escape characters for colors which require colorstatus patch.
#ifdef MPD
#define MPD_STR         "%s \x02-\x01 %s   \x02•\x01   "                    // MPD, playing
#define MPD_P_STR       "Paused\x02:\x01 %s \x02-\x01 %s   \x02•\x01   "    // MPD, paused
#define MPD_S_STR       " "                                                 // MPD, stopped
#define NO_MPD_STR      "Geen verbinding   \x02•\x01   "					// MPD, can't connect
#endif
#ifdef AUD
#define MUSIC_STR       "%s   \x02•\x01   "                         // Music, playing
#define MUSIC_P_STR     "Paused\x02:\x01 %s   \x02•\x01   "         // Music, paused
#define MUSIC_S_STR     " "                                         // Music, stopped
#define NO_MUSIC_STR    "Geen verbinding    \x02•\x01   "           // Music, can't connect
#endif
#define SKYPE_STR		"Skype   \x02•\x01   "						// Skype is running
#define NO_SKYPE_STR	" "											// Skype is not running
#define WIFI_STR		" %s %d%%   "								// WIFI
#define NO_WIFI_STR		"  Geen verbinding   "						// WIFI, no connection
#define VOL_STR			"\x02•\x01   %d%%   "						// Volume
#define VOL_MUTE_STR	"\x02•\x01       ×       "					// Volume, muted
#define BAT_STR			"\x02•\x01   D  %d%%   "					// Battery, BAT, above BATT_LOW percentage
#define BAT_LOW_STR		"\x02•\x03   D  %d%%   "					// Battery, BAT, below BATT_LOW percentage
#define BAT_FULL_STR	"\x02•\x04   F  \x01%d%%   "				// Battery, full
#define BAT_CHRG_STR	"\x02•\x01   C  %d%%   "					// Battery, AC
#define DATE_TIME_STR	"\x02•\x01   %a %d %b\x02,\x01 %H:%M   "	// This is a strftime format string which is passed localtime

int main() {
	Display *dpy;
	Window root;
	int num, skfd, wifiloops=60, skypeloops=20, musicloops=10;
	long lnum1, lnum2;
	char statnext[100], status[100], wifistring[30], skypestring[30], musicstring[100];
#ifdef MPD
    struct mpd_song * song = NULL;
    const char * title = NULL;
    const char * artist = NULL;
#endif
#ifdef AUD
    gint playpos;
    gchar *psong;
    DBusGProxy *session = NULL;
    DBusGConnection *connection = NULL;
    session = 0;
    psong = NULL;
#endif
	struct wireless_info *winfo;
	winfo = (struct wireless_info *) malloc(sizeof(struct wireless_info));
	memset(winfo, 0, sizeof(struct wireless_info));
	long vol = 0, max = 0, min = 0;
	int mute = 0, realvol = 0;
	time_t current;
	FILE *infile;
	// Setup X display and root window id:
	dpy=XOpenDisplay(NULL);
	if ( dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	root = XRootWindow(dpy,DefaultScreen(dpy));
	setlocale(LC_ALL, "");
#ifdef AUD
	g_type_init();
#endif
// MAIN LOOP STARTS HERE
	for (;;) {
		status[0]='\0';
	// MUSIC
		//if (++musicloops > 10) {
		//	musicloops=0;
#ifdef MPD
			struct mpd_connection * conn = mpd_connection_new(NULL, 0, 30000);
    	    if (mpd_connection_get_error(conn))
        	    sprintf(statnext,NO_MPD_STR);

			mpd_command_list_begin(conn, true);
            mpd_send_status(conn);
            mpd_send_current_song(conn);
            mpd_command_list_end(conn);

			struct mpd_status* theStatus = mpd_recv_status(conn);
                if (!theStatus)
                    sprintf(statnext,NO_MPD_STR);
                else
                    if (mpd_status_get_state(theStatus) == MPD_STATE_PLAY) {
                        mpd_response_next(conn);
                        song = mpd_recv_song(conn);
                        title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
                        artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
                        sprintf(statnext,MPD_STR,title,artist);
                        mpd_song_free(song);
                    }
                    else if (mpd_status_get_state(theStatus) == MPD_STATE_PAUSE) {
                        mpd_response_next(conn);
						song = mpd_recv_song(conn);
                        title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
                        artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
                        sprintf(statnext,MPD_P_STR,title,artist);
                        mpd_song_free(song);
                    }
                    else if (mpd_status_get_state(theStatus) == MPD_STATE_STOP) {
                        sprintf(statnext,MPD_S_STR);
                    }
            mpd_response_finish(conn);
            mpd_connection_free(conn);
			strcat(status,statnext);
#endif
#ifdef AUD
        	connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
        	session = dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE, AUDACIOUS_DBUS_PATH, AUDACIOUS_DBUS_INTERFACE);
        	playpos = audacious_remote_get_playlist_pos(session);
        	psong = audacious_remote_get_playlist_title(session, playpos);
        	if (psong) {
	            if (audacious_remote_is_paused(session)) {
    	            sprintf(statnext,MUSIC_P_STR,psong);
        	        g_free(psong);
            	    psong = NULL;
	            } else if (audacious_remote_is_playing(session)) {
    	            sprintf(statnext,MUSIC_STR,psong);
        	        g_free(psong);
            	    psong = NULL;
	            } else {
    	            sprintf(statnext,MUSIC_S_STR);
        	    }
	        } else {
    	        sprintf(statnext,MUSIC_S_STR);
        	}
	        g_object_unref(session);
    	    session = NULL;
			strcat(status,statnext);
#endif
		//}
	// SKYPE
		if (++skypeloops > 20) {
			skypeloops=0;
			infile = fopen(SKYPE_FILE,"r");
			if (infile) {
				sprintf(skypestring,SKYPE_STR);
				fclose(infile);
			}
			else
				sprintf(skypestring,NO_SKYPE_STR);
		}
		strcat(status,skypestring);
	// WIFI
		if (++wifiloops > 60) {
            wifiloops=0;
			skfd = iw_sockets_open();
			if (iw_get_basic_config(skfd, WIFI, &(winfo->b)) > -1) {
				if (iw_get_stats(skfd, WIFI, &(winfo->stats), // set present winfo variables
					&winfo->range, winfo->has_range) >= 0) {
					winfo->has_stats = 1;
				}
				if (iw_get_range_info(skfd, WIFI, &(winfo->range)) >= 0) { // set present winfo variables
					winfo->has_range = 1;
				}
				if (winfo->b.has_essid) {
					if (winfo->b.essid_on) {
						sprintf(wifistring,WIFI_STR,winfo->b.essid,(winfo->stats.qual.qual*100)/winfo->range.max_qual.qual);
					} else {
						sprintf(wifistring,NO_WIFI_STR);
					}
				}
			}
			iw_sockets_close(skfd);
			fflush(stdout);
			memset(winfo, 0, sizeof(struct wireless_info));
		}
		strcat(status,wifistring);
	// Audio volume
		snd_mixer_t *handle; // init alsa
		snd_mixer_open(&handle, 0);
		snd_mixer_attach(handle, "default");
		snd_mixer_selem_register(handle, NULL, NULL);
		snd_mixer_load(handle);
		snd_mixer_selem_id_t *vol_info; // init channel with volume info
		snd_mixer_selem_id_malloc(&vol_info);
		snd_mixer_selem_id_set_name(vol_info, VOL_CH);
		snd_mixer_elem_t* pcm_mixer = snd_mixer_find_selem(handle, vol_info);
		snd_mixer_selem_get_playback_volume_range(pcm_mixer, &min, &max); // get volume
		snd_mixer_selem_get_playback_volume(pcm_mixer, SND_MIXER_SCHN_MONO, &vol);
		snd_mixer_selem_id_t *mute_info; // init channel with mute info
		snd_mixer_selem_id_malloc(&mute_info);
		snd_mixer_selem_id_set_name(mute_info, VOL_CH);
		snd_mixer_elem_t* mas_mixer = snd_mixer_find_selem(handle, mute_info);
		snd_mixer_selem_get_playback_switch(mas_mixer, SND_MIXER_SCHN_MONO, &mute); // get mute state

		if(mute == 0)
			sprintf(statnext, VOL_MUTE_STR);
		else
			realvol = (vol*100)/max;
			sprintf(statnext, VOL_STR, realvol);

		if(vol_info)
			snd_mixer_selem_id_free(vol_info);
		if (mute_info)
			snd_mixer_selem_id_free(mute_info);
		if (handle)
			snd_mixer_close(handle);
		strcat(status,statnext);
	// Power / Battery
		infile = fopen(BATT_NOW,"r");
			fscanf(infile,"%ld\n",&lnum1); fclose(infile);
		infile = fopen(BATT_FULL,"r");
			fscanf(infile,"%ld\n",&lnum2); fclose(infile);
		infile = fopen(BATT_STAT,"r");
			fscanf(infile,"%s\n",statnext); fclose(infile);
		num = lnum1*100/lnum2;
		if (strncmp(statnext,"Charging",8) == 0) {
			sprintf(statnext,BAT_CHRG_STR,num);
		}
		else if (strncmp(statnext,"Full",8) == 0) {
				sprintf(statnext,BAT_FULL_STR,num);
		}
		else {
			if (num <  BATT_LOW)
				sprintf(statnext,BAT_LOW_STR,num);
			else
				sprintf(statnext,BAT_STR,num);
		}
		strcat(status,statnext);
	// Time
		time(&current);
		strftime(statnext,38,DATE_TIME_STR,localtime(&current));
		strcat(status,statnext);
	// Set root name
		XStoreName(dpy,root,status);
		XFlush(dpy);
		sleep(INTERVAL);
	}
// NEXT LINES SHOULD NEVER EXECUTE, only here to satisfy Trilby's O.C.D. ;)
	XCloseDisplay(dpy);
	return 0;
}

