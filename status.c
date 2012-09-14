#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

#define BATT_LOW	11 // Below BATT_LOW percentage left on battery, the battery display turns red
#define INTERVAL	1  // Sleeps for INTERVAL seconds between updates

// Files read for system info:
#define AUD_FILE		"/home/jente/.audio_volume"
#define BATT_NOW		"/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT0/status"
// Display format strings:
// Defaults make extensive use of escape characters for colors which require colorstatus patch.
#define VOL_STR			"\x02•\x01 %d%% "						// volume when not muted
#define VOL_MUTE_STR	"\x02•\x01       ×      "				// volume when muted
#define BAT_STR			"\x02•\x01 D %d%% "						// Battery, BAT, above BATT_LOW percentage
#define BAT_FULL_STR	"\x02•\x04 F \x01%d%% "						// Battery, full
#define BAT_LOW_STR		"\x02•\x03 D %d%% "						// Battery, BAT, below BATT_LOW percentage
#define BAT_CHRG_STR	"\x02•\x01 C %d%% "						// Battery, AC
#define DATE_TIME_STR	"\x02•\x01 %a %b %d\x02,\x01 %H:%M "	// This is a strftime format string which is passed localtime

int main() {
	Display *dpy;
	Window root;
	int num;
	long lnum1,lnum2,lnum3,lnum4;
	char statnext[30], status[100];
	time_t current;
	FILE *infile;
	// Setup X display and root window id:
	dpy=XOpenDisplay(NULL);
	if ( dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	root = XRootWindow(dpy,DefaultScreen(dpy));
// MAIN LOOP STARTS HERE:
	for (;;) {
		status[0]='\0';
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
// NEXT LINES SHOULD NEVER EXECUTE, only here to satisfy my O.C.D.
	XCloseDisplay(dpy);
	return 0;
}

