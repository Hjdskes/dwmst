#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <mpd/client.h>
#include <X11/Xlib.h>
#include <iwlib.h>

#define WIFI			"wlan0"		// Wireless interface
#define BATT_LOW		11			// Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL		1			// Sleeps for INTERVAL seconds between updates
// Files read for system info:
#define AUD_FILE		"/home/jente/.audio_volume"
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
// Display format strings. Defaults make extensive use of escape characters for colors which require colorstatus patch.
#define MPD_STR			"%s \x02-\x01 %s \x02•\x01   "					// MPD, playing
#define MPD_P_STR		"Paused\x02:\x01 %s \x02-\x01 %s \x02•\x01   "	// MPD, paused
#define MPD_S_STR		" "												// MPD, stopped
#define NO_MPD_STR		"Geen verbinding \x02•\x01   "					// MPD, can't connect
#define WIFI_STR		"  %s %d%%   "									// WIFI
#define NO_WIFI_STR		"  Geen verbinding   "							// WIFI, no connection
#define VOL_STR			"\x02•\x01   %d%%   "							// Volume
#define VOL_MUTE_STR	"\x02•\x01       ×       "						// Volume, muted
#define BAT_STR			"\x02•\x01   D %d%%   "							// Battery, BAT, above BATT_LOW percentage
#define BAT_LOW_STR		"\x02•\x03   D %d%%   "							// Battery, BAT, below BATT_LOW percentage
#define BAT_FULL_STR	"\x02•\x04   F \x01%d%%   "						// Battery, full
#define BAT_CHRG_STR	"\x02•\x01   C %d%%   "							// Battery, AC
#define DATE_TIME_STR	"\x02•\x01   %a %d %b\x02,\x01 %H:%M   "		// This is a strftime format string which is passed localtime

int main() {
	Display *dpy;
	Window root;
	int num, skfd, has_bitrate=0, loops=60;
	long lnum1,lnum2;
	char statnext[30], status[100], wifiString[30];
	struct mpd_song * song = NULL;
	char * title = NULL;
	char * artist = NULL;
	struct wireless_info *winfo;
	winfo = (struct wireless_info *) malloc(sizeof(struct wireless_info));
	memset(winfo, 0, sizeof(struct wireless_info));
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
// MAIN LOOP STARTS HERE
	for (;;) {
		status[0]='\0';
	// MPD
		struct mpd_connection * conn = mpd_connection_new(NULL, 0, 30000);
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
	// WIFI
		if (++loops > 60) {
			loops=0;
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
						sprintf(wifiString,WIFI_STR,winfo->b.essid,(winfo->stats.qual.qual*100)/winfo->range.max_qual.qual);
					} else {
						sprintf(wifiString,NO_WIFI_STR);
					}
				}
			}
			iw_sockets_close(skfd);
			fflush(stdout);
			memset(winfo, 0, sizeof(struct wireless_info));
		}
		strcat(status,wifiString);
	// Audio volume
		infile = fopen(AUD_FILE,"r");
		fscanf(infile,"%d",&num);
		fclose(infile);
		if (num == -1)
			sprintf(statnext,VOL_MUTE_STR,num);
		else
			sprintf(statnext,VOL_STR,num);
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

