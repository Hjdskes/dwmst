#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <iwlib.h>
#include <alsa/asoundlib.h>
#ifdef CLK
#include <time.h>
#endif
#ifdef MPD
#include <mpd/client.h>
#endif
#ifdef AUD
#include <audacious/dbus.h>
#include <audacious/audctrl.h>
#endif

#define WIFI			"wlan0"		/* Wireless interface */
#define BATT_LOW_P		11			/* Below BATT_LOW percentage left on battery, the battery display turns red */
#define BATT_LOW_T		3			/* Same as above, but now minutes instead of percentage */
#define INTERVAL		1			/* Sleeps for INTERVAL seconds between updates */
#define VOL_CH			"Master"	/* Channel to watch for volume */
/* Files read for system info: */
#define SKYPE_FILE		"/home/jente/.Skype/jente_etnej/main.lock"
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
#define BATT_VOLT		"/sys/class/power_supply/BAT0/voltage_now"
#define BATT_CNOW		"/sys/class/power_supply/BAT0/current_now"
/* Display format strings. Defaults make extensive use of escape characters for colors which require colorstatus patch. */
#ifdef MPD
#define MPD_STR			"\x02%s \x01 - \x02 %s  "			/* MPD, playing */
#define MPD_P_STR		"Paused: \x02 %s \x01 - \x02 %s  "	/* MPD, paused */
#define MPD_S_STR		""									/* MPD, stopped */
#define NO_MPD_STR		"Geen verbinding  "					/* MPD, can't connect */
#endif
#ifdef AUD
#define MUSIC_STR		"\x02%s  "						/* Music, playing */
#define MUSIC_P_STR		"P: \x02 %s  "					/* Music, paused */
#define MUSIC_S_STR		""								/* Music, stopped */
#endif
#define SKYPE_STR		"\x02Skype "					/*Skype is running */
#define NO_SKYPE_STR	""								/* Skype is not running */
#define WIFI_STR		"\x01 %s \x02 %d%% "			/* WIFI */
#define NO_WIFI_STR		"\x01 Geen verbinding "			/* WIFI, no connection */
#define VOL_STR			"\x01 Volume \x02 %d%% "		/* Volume */
#define VOL_MUTE_STR	"\x01 Volume \x02 × "			/* Volume, muted */
#define BAT_FULL_STR	"\x01 Batterij \x02 F %d%%"		/* Battery, full */
#define BAT_STR			"\x01 Batterij \x02 D %d%%, %02d:%02d resterend"		/* Battery, discharging, above BATT_LOW percentage */
#define BAT_LOW_STR		"\x01 Batterij \x02 D %d%%, %02d:%02d resterend"		/* Battery, discharging, below BATT_LOW percentage */
#define BAT_CHRG_STR	"\x01 Batterij \x02 C %d%%, %02d:%02d tot opgeladen"	/* Battery, AC */
#ifdef CLK
#define DATE_TIME_STR	"\x01 %a \x02%d \x01%b,\x02 %H:%M"	/* This is a strftime format string which is passed localtime */
#endif

int main() {
	Display *dpy;
	Window root;
	FILE *infile;
	int perc, skfd, mute = 0, realvol = 0, wifiloops = 60, musicloops = 10;
	int hours, minutes, seconds = -1;
	long now = -1, full = -1, voltage = -1, rate = -1, vol = 0, max = 0, min = 0;
	char statnext[100], status[200], wifistring[30], musicstring[100];
	struct wireless_info *winfo;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *vol_info, *mute_info;
	snd_mixer_elem_t *pcm_mixer;
	snd_mixer_elem_t *mas_mixer;
#ifdef CLK
	time_t current;
#endif
#ifdef MPD
	struct mpd_song *song = NULL;
	const char *title = NULL;
	const char *artist = NULL;
#endif
#ifdef AUD
	int playpos;
	char *psong;
	DBusGProxy *session;
	DBusGConnection *connection;
#endif
	/* Setup X display and root window id */
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	root = XRootWindow(dpy,DefaultScreen(dpy));
	winfo = (struct wireless_info *) calloc(1, sizeof(struct wireless_info));
#ifdef CLK
	setlocale(LC_ALL, "");
#endif
#ifdef AUD
	g_type_init();
	session = NULL;
	connection = NULL;
	psong = NULL;
#endif
/* MAIN LOOP STARTS HERE */
	for (;;) {
		status[0]='\0';
	/* MUSIC */
#ifdef MPD
		if (++musicloops > 10) {
			musicloops = 0;
			struct mpd_connection * conn = mpd_connection_new(NULL, 0, 30000);
			if (mpd_connection_get_error(conn))
				sprintf(musicstring, NO_MPD_STR);

			mpd_command_list_begin(conn, true);
			mpd_send_status(conn);
			mpd_send_current_song(conn);
			mpd_command_list_end(conn);

			struct mpd_status* theStatus = mpd_recv_status(conn);
				if (!theStatus)
					sprintf(musicstring, NO_MPD_STR);
				else
					if (mpd_status_get_state(theStatus) == MPD_STATE_PLAY) {
						mpd_response_next(conn);
						song = mpd_recv_song(conn);
						title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
						artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
						sprintf(musicstring, MPD_STR, title, artist);
						mpd_song_free(song);
					}
					else if (mpd_status_get_state(theStatus) == MPD_STATE_PAUSE) {
						mpd_response_next(conn);
						song = mpd_recv_song(conn);
						title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
						artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
						sprintf(musicstring, MPD_P_STR, title, artist);
						mpd_song_free(song);
					}
					else if (mpd_status_get_state(theStatus) == MPD_STATE_STOP) {
						sprintf(musicstring, MPD_S_STR);
					}
			mpd_response_finish(conn);
			mpd_connection_free(conn);
		}
		strcat(status, musicstring);
#endif
#ifdef AUD
		if (++musicloops > 10) {
			musicloops = 0;
			connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
			session = dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE, AUDACIOUS_DBUS_PATH, AUDACIOUS_DBUS_INTERFACE);
			playpos = audacious_remote_get_playlist_pos(session);
			psong = audacious_remote_get_playlist_title(session, playpos);
			if (psong) {
				if (audacious_remote_is_paused(session)) {
					sprintf(musicstring, MUSIC_P_STR, psong);
					free(psong);
					psong = NULL;
				} else if (audacious_remote_is_playing(session)) {
					sprintf(musicstring, MUSIC_STR, psong);
					free(psong);
					psong = NULL;
				} else {
					sprintf(musicstring, MUSIC_S_STR);
				}
			} else {
				sprintf(musicstring, MUSIC_S_STR);
			}
			g_object_unref(session);
			session = NULL;
		}
		strcat(status, musicstring);
#endif
	/* SKYPE */
		infile = fopen(SKYPE_FILE, "r");
		if (infile) {
			sprintf(statnext, SKYPE_STR);
			fclose(infile);
		}
		else
			sprintf(statnext, NO_SKYPE_STR);
		strcat(status, statnext);
	/* WIFI */
		if (++wifiloops > 60) {
			wifiloops = 0;
			skfd = iw_sockets_open();
			if (iw_get_basic_config(skfd, WIFI, &(winfo->b)) > -1) {
				if (iw_get_stats(skfd, WIFI, &(winfo->stats), /* set present winfo variables */
					&winfo->range, winfo->has_range) >= 0) {
					winfo->has_stats = 1;
				}
				if (iw_get_range_info(skfd, WIFI, &(winfo->range)) >= 0) { /* set present winfo variables */
					winfo->has_range = 1;
				}
				if (winfo->b.has_essid) {
					if (winfo->b.essid_on) {
						sprintf(wifistring, WIFI_STR, winfo->b.essid, (winfo->stats.qual.qual * 100) / winfo->range.max_qual.qual);
					} else {
						sprintf(wifistring, NO_WIFI_STR);
					}
				}
			}
			iw_sockets_close(skfd);
			fflush(stdout);
			memset(winfo, 0, sizeof(struct wireless_info));
		}
		strcat(status,wifistring);
	/* Audio volume */
		snd_mixer_open(&handle, 0);
		snd_mixer_attach(handle, "default");
		snd_mixer_selem_register(handle, NULL, NULL);
		snd_mixer_load(handle);
		snd_mixer_selem_id_malloc(&vol_info);
		snd_mixer_selem_id_set_name(vol_info, VOL_CH);
		pcm_mixer = snd_mixer_find_selem(handle, vol_info);
		snd_mixer_selem_get_playback_volume_range(pcm_mixer, &min, &max); /* get volume */
		snd_mixer_selem_get_playback_volume(pcm_mixer, SND_MIXER_SCHN_MONO, &vol);
		snd_mixer_selem_id_malloc(&mute_info);
		snd_mixer_selem_id_set_name(mute_info, VOL_CH);
		mas_mixer = snd_mixer_find_selem(handle, mute_info);
		snd_mixer_selem_get_playback_switch(mas_mixer, SND_MIXER_SCHN_MONO, &mute); /* get mute state */

		if(mute == 0) {
			sprintf(statnext, VOL_MUTE_STR);
		} else {
			realvol = (vol * 100) / max;
			sprintf(statnext, VOL_STR, realvol);
		}
		if(vol_info)
			snd_mixer_selem_id_free(vol_info);
		if (mute_info)
			snd_mixer_selem_id_free(mute_info);
		if (handle)
			snd_mixer_close(handle);
		strcat(status,statnext);
	/* Power / Battery */
		infile = fopen(BATT_NOW, "r");
			fscanf(infile, "%ld\n", &now); fclose(infile);
		infile = fopen(BATT_FULL, "r");
			fscanf(infile, "%ld\n", &full); fclose(infile);
		infile = fopen(BATT_STAT, "r");
			fscanf(infile, "%s\n", statnext); fclose(infile);
		infile = fopen(BATT_VOLT, "r");
			fscanf(infile, "%ld\n", &voltage); fclose(infile);
		infile = fopen(BATT_CNOW, "r");
			fscanf(infile, "%ld\n", &rate); fclose(infile);
		now = ((float)voltage / 1000) * ((float)now / 1000);
		full = ((float)voltage / 1000) * ((float)full / 1000);
		rate = ((float)voltage / 1000) * ((float)rate / 1000);
		perc = (now * 100) / full;

		if (strncmp(statnext, "Full", 8) == 0) {
			sprintf(statnext, BAT_FULL_STR, perc);
		} else if (strncmp(statnext, "Charging", 8) == 0) {
			seconds = 3600 * (((float)full - (float)now) / (float)rate);
			hours = seconds / 3600;
			seconds -= 3600 * hours;
			minutes = seconds / 60;
			seconds -= 60 * minutes;
			sprintf(statnext, BAT_CHRG_STR, perc, hours, minutes); 
		} else {
			seconds = 3600 * ((float)now / (float)rate);
            hours = seconds / 3600;
			seconds -= 3600 * hours;
			minutes = seconds / 60;
			seconds -= 60 * minutes;
			if (perc <  BATT_LOW_P || minutes < BATT_LOW_T) {				
				sprintf(statnext, BAT_LOW_STR, perc, hours, minutes);
			} else {
				sprintf(statnext, BAT_STR, perc, hours, minutes);
			}
		}
		hours = minutes = seconds = 0;
		strcat(status, statnext);
	/* Time */
#ifdef CLK
		time(&current);
		strftime(statnext, 38, DATE_TIME_STR, localtime(&current));
		strcat(status, statnext);
#endif
	/* Set root name */
		XStoreName(dpy, root, status);
		XFlush(dpy);
		sleep(INTERVAL);
	}
/* NEXT LINES SHOULD NEVER EXECUTE, only here to satisfy Trilby's O.C.D. ;) */
	XCloseDisplay(dpy);
	return 0;
}
