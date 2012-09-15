	#include <mpd/client.h>
//#include <mpd/status.h>
//#include <mpd/entity.h>
//#include <mpd/search.h>
//#include <mpd/tag.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <iwlib.h>

#define WIFI		"wlan0"	// Wireless interface
#define BATT_LOW	11		// Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL	1		// Sleeps for INTERVAL seconds between updates
// Files read for system info:
#define AUD_FILE		"/home/jente/.audio_volume"
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
// Display format strings:
// Defaults make extensive use of escape characters for colors which require colorstatus patch.
	#define MPD_STR			"<b>%s</b>- <i>%s</i> \x02•\x01 "	// MPD
#define WIFI_STR		"%s %d%% "								// WIFI
#define NO_WIFI_STR		"Geen verbinding "						// WIFI, no connection
#define VOL_STR			"\x02•\x01 %d%% "						// Volume
#define VOL_MUTE_STR	"\x02•\x01       ×      "				// Volume, muted
#define BAT_STR			"\x02•\x01 D %d%% "						// Battery, BAT, above BATT_LOW percentage
#define BAT_FULL_STR	"\x02•\x04 F \x01%d%% "					// Battery, full
#define BAT_LOW_STR		"\x02•\x03 D %d%% "						// Battery, BAT, below BATT_LOW percentage
#define BAT_CHRG_STR	"\x02•\x01 C %d%% "						// Battery, AC
#define DATE_TIME_STR	"\x02•\x01 %a %b %d\x02,\x01 %H:%M "	// This is a strftime format string which is passed localtime

int main() {
	Display *dpy;
	Window root;
	int num;
	unsigned int i = 0;
	int have_song = 0;
	long lnum1,lnum2;
	char statnext[30], status[100];
	/*const struct mpd_song *song;
	const char *artist; 
	const char *title;
	struct mpd_connection *conn;*/
	int skfd, has_bitrate = 0;
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
// MAIN LOOP STARTS HERE
	for (;;) {
		status[0]='\0';
	// MPD
		/*while ((song = mpd_recv_song(conn)) != NULL ) {
			have_song = 1;
			artist = mpd_song_get_tag(song,MPD_TAG_ARTIST,i++);
			i = 0;
			title = mpd_song_get_tag(song,MPD_TAG_TITLE,i++);
		};
		mpd_connection_free(conn);
		if (have_song == 1) {
		sprintf(statnext, MPD_STR, title, artist);
		};
		strcat(status,statnext);*/
	// WIFI
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
					sprintf(statnext,WIFI_STR,winfo->b.essid,(winfo->stats.qual.qual*100)/winfo->range.max_qual.qual);
				} else {
					sprintf(statnext,NO_WIFI_STR);
				}
			}
		}
		iw_sockets_close(skfd);
		fflush(stdout);
		memset(winfo, 0, sizeof(struct wireless_info));
		strcat(status,statnext);
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
			if (num < BATT_LOW)
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

