#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

// Below BATT_LOW% left on battery, the battery display turns red
#define BATT_LOW	11

// Sleeps for INTERVAL seconds between updates
#define INTERVAL	1

// Files read for system info:
#define AUD_FILE		"/home/jmcclure/.status_info"
#define BATT_NOW		"/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT1/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT1/status"
// Display format strings:
//  Defaults make extensive use of escape characters for colors which require
//  colorstatus patch.  There are also "extended" characters selected to work
//  with terminus2 font for symbols and trianlge "flags".
#define VOL_STR			"Ü	 Ô %d% Ü"				// volume when not muted  IMPORTANT! SEE NOTE IN README FOR AUDO INFO
#define VOL_MUTE_STR	"Ü	 Ô × Ü"					// volume when muted
#define BAT_STR			"Ü	 %d%% Ü"				// Battery, unplugged, above BATT_LOW%
#define BAT_LOW_STR		"Ü %d%% Ü"					// Battery, unplugged, below BATT_LOW% remaining
#define BAT_CHRG_STR	"Ü %d%% Ü"					// Battery, when charging (plugged into AC)
#define DATE_TIME_STR	"Ü %a %b %d ÜÜ Õ %H:%M "	// This is a strftime format string which is passed localtime

int main() {
	Display *dpy;
	Window root;
	int num;
	long lnum1,lnum2,lnum3,lnum4;
	char statnext[30], status[100];
	time_t current;
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
	// Audio volume:
	/*	infile = fopen(AUD_FILE,"r");
		fscanf(infile,"%d",&num);
		fclose(infile);
		if (num == -1)
			sprintf(statnext,VOL_MUTE_STR,num);
		else
			sprintf(statnext,VOL_STR,num);
		strcat(status,statnext);*/
	// Power / Battery:
		infile = fopen(BATT_NOW,"r");
			fscanf(infile,"%ld\n",&lnum1);fclose(infile);
		infile = fopen(BATT_FULL,"r");
			fscanf(infile,"%ld\n",&lnum2);fclose(infile);
		infile = fopen(BATT_STAT,"r");
			fscanf(infile,"%s\n",statnext);fclose(infile);
		num = lnum1*100/lnum2;
		if (strncmp(statnext,"Charging",8) == 0) {
			sprintf(statnext,BAT_CHRG_STR,num);
		}
		else {
			if (num < BATT_LOW)
				sprintf(statnext,BAT_LOW_STR,num);
			else
				sprintf(statnext,BAT_STR,num);
		}
		strcat(status,statnext);
	// Date & Time:
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

