#include <unistd.h>
#include <X11/Xlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <alsa/asoundlib.h>
#include <linux/wireless.h>
#include <audacious/dbus.h>
#include <audacious/audctrl.h>

#include "dwmstatus.h"

static Display *dpy;

char *
smprintf(char *fmt, ...) {
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

int
is_up (char *device) {
	char devpath[32], state[5];
	FILE *infile;

	snprintf(devpath, sizeof(devpath), "/sys/class/net/%s/operstate", device);
	infile = fopen(devpath, "r");
	if(infile != NULL) {
		fscanf(infile, "%s", state);
		fclose(infile);
		if (strncmp (state, "up", 2) == 0)
			return 1;
	}
	return 0;
}

char *
get_aud(DBusGProxy *session) {
	char *music = NULL; /*ToDo: free?*/
	int pos;

	pos = audacious_remote_get_playlist_pos(session);
	music = audacious_remote_get_playlist_title(session, pos);
	if(music != NULL)
		return smprintf(AUD_STR, music);
	else
		return NULL; /*ToDo: fix this*/
}

char *
get_skype(void) {
	if(access (SKYPE_LOCK, F_OK) == 0)
		return smprintf("%s", SKYPE_STR);
	else
		return NULL; /*ToDo: fix this*/
}

char *
get_net(struct iwreq wreq, int socket) {
	if(is_up (WIRELESS_DEVICE)) {
		char essid[30];

		memset(essid, 0, sizeof(essid));
		wreq.u.essid.pointer = essid;
		wreq.u.essid.length = sizeof(essid);
		ioctl(socket, SIOCGIWESSID, &wreq);
		return smprintf(WLAN_STR, essid);
	} else if(is_up (WIRED_DEVICE))
		return smprintf("%s", LAN_STR);
	else
		return smprintf("%s", NO_CON_STR);
}

char *
get_vol(snd_mixer_t *handle) {
	int mute = 0, realvol = 0;
	long vol = 0, max = 0, min = 0;
	snd_mixer_elem_t *pcm_mixer, *max_mixer;
	snd_mixer_selem_id_t *vol_info, *mute_info;

	/*ToDo: maybe move all this to main?*/
	snd_mixer_handle_events(handle);
	snd_mixer_selem_id_malloc(&vol_info);
	snd_mixer_selem_id_malloc(&mute_info);
	snd_mixer_selem_id_set_name(vol_info, VOL_CH);
	snd_mixer_selem_id_set_name(mute_info, VOL_CH);
	pcm_mixer = snd_mixer_find_selem(handle, vol_info);
	max_mixer = snd_mixer_find_selem(handle, mute_info);
	snd_mixer_selem_get_playback_volume_range(pcm_mixer, &min, &max);
	snd_mixer_selem_get_playback_volume(pcm_mixer, 0, &vol);
	snd_mixer_selem_get_playback_switch(max_mixer, 0, &mute);
	snd_mixer_selem_id_free(vol_info);
	snd_mixer_selem_id_free(mute_info);

	realvol = (vol * 100) / max;
	if(mute == 0)
		return smprintf(VOL_MUTE_STR, realvol);
	else {
		if (realvol < 20)
			return smprintf(VOL_LOW_STR, realvol);
		else if (realvol > 60)
			return smprintf(VOL_HIGH_STR, realvol);
		else
			return smprintf(VOL_MID_STR, realvol);
	}
}

char *
get_batt(void) { /*ToDo: fix many strncmps*/
	FILE *f1, *f2, *f3, *f4;
	char state[11];

	if(access("/sys/class/power_supply/BAT1/", F_OK) != 0)
		return smprintf("%s", NO_BAT_STR);

	memset(state, 0, sizeof(state));
	f1 = fopen(BATT_STAT, "r");
	if(f1 != NULL) {
		fscanf(f1, "%s\n", state);
		fclose(f1);
	} else
		return NULL; /*ToDo: fix this*/

	if(strncmp(state, "Full", 4) == 0)
		return smprintf("%s", BAT_FULL_STR);
	else if(strncmp(state, "Unknown", 7) == 0)
		return smprintf("%s", BAT_UNK_STR);
	else {
		long now = -1, full = -1, voltage = -1, rate = -1;
		int perc = -1, seconds = -1, minutes, hours;

		f1 = fopen(BATT_NOW, "r");
		f2 = fopen(BATT_FULL, "r");
		f3 = fopen(BATT_VOLT, "r");
		f4 = fopen(BATT_CNOW, "r");
		if(f1 != NULL && f2 != NULL && f3 != NULL && f4 != NULL) {
			fscanf(f1, "%ld\n", &now); fclose(f1);
			fscanf(f2, "%ld\n", &full); fclose(f2);
			fscanf(f3, "%ld\n", &voltage); fclose(f3);
			fscanf(f4, "%ld\n", &rate); fclose(f4);
		}

		now = voltage * now;
		full = voltage * full;
		rate = voltage * rate;
		perc = ((float)now * 100) / (float)full;

		if(perc > 100) perc = 100;
		if(perc < 0) perc = 0;

		if(strncmp(state, "Charging", 8) == 0)
			seconds = 3600 * ((full - now) / rate);
		else
			seconds = 3600 * (now / rate);
		hours = seconds / 3600;
		seconds -= 3600 * hours;
		minutes = seconds / 60;
		seconds -= 60 * minutes;

		if(strncmp(state, "Charging", 8) == 0)
			return smprintf(BAT_CHRG_STR, perc, hours, minutes);
		else {
			if(perc < 5)
				return smprintf(BAT_EMPTY_STR, perc, hours, minutes);
				/*notify*/
			else if(perc > 5 && perc < 50)
				return smprintf(BAT_LOW_STR, perc, hours, minutes);
			else if(perc > 50 && perc < 95)
				return smprintf(BAT_HIGH_STR, perc, hours, minutes);
			else
				return smprintf(BAT_FULL_STR, perc, hours, minutes);
		}
	}
}

void
setstatus(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}


/*char **/
void
read_stdin(void) {
	char inbuf[256];
	char *status;

	memset(inbuf, 0, sizeof(inbuf));
	if(fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
		inbuf[strlen(inbuf) - 1] = '\0';
		status = smprintf("%s", inbuf);
		setstatus(status);
		free(status);
		/*return smprintf("%s", inbuf);*/
		sleep(3);
	}
	/*return NULL;*/
}

int
main(void) {
	int sockfd, netloops = 60, musicloops = 60;
	struct iwreq wreq;
	snd_mixer_t *handle;
	char *status, *pipe, *aud, *skype, *net, *vol, *batt;
	DBusGProxy *session = NULL;
	DBusGConnection *conn = NULL;

	if(!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmst: cannot open display.\n");
		return 1;
	}

	conn = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	session = dbus_g_proxy_new_for_name(conn, AUDACIOUS_DBUS_SERVICE, AUDACIOUS_DBUS_PATH, AUDACIOUS_DBUS_INTERFACE);

	memset(&wreq, 0, sizeof(struct iwreq));
	snprintf(wreq.ifr_name, sizeof(WIRELESS_DEVICE), WIRELESS_DEVICE); /*ToDo: what with multiple devices?*/
	sockfd = socket (AF_INET, SOCK_DGRAM, 0); /*ToDo: what do these mean?*/

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, "default");
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	for(;;sleep(INTERVAL)) {
		/*pipe = read_stdin();
		if(strlen(pipe) > 2) {
			status = smprintf("%s", pipe);
			setstatus(status);
			free(pipe);
			free(status);
			sleep(3);
		}*/
		/*read_stdin();*/
		if(++musicloops > 60 && session != NULL)
			aud = get_aud(session);
		skype = get_skype();
		if(++netloops > 60 && sockfd != -1)
			net = get_net(wreq, sockfd);
		vol = get_vol(handle);
		batt = get_batt();

		status = smprintf("%s      %s      %s      %s      %s", aud, skype, net, vol, batt);
		setstatus(status);
		if(++musicloops > 60 && session != NULL)
			free(aud);
		free(skype);
		if(++netloops > 60 && sockfd != -1)
			free(net);
		free(vol);
		free(batt);
		free(status);
	}

	XCloseDisplay(dpy);
	close(sockfd);
	snd_mixer_close(handle);
	return 0;
}
