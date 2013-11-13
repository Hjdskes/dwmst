#include <X11/Xlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <alsa/asoundlib.h>
#include <audacious/dbus.h>
#include <audacious/audctrl.h>

#include "dwmst.h"

char *get_aud (char *buf, DBusGProxy *session) {
	char *psong = NULL;

	psong = audacious_remote_get_playlist_title (session, audacious_remote_get_playlist_pos (session));
	if (psong) {
		snprintf (buf, 160, AUD_STR, psong);
		free (psong);
	} else
		buf[0] = '\0';
	return buf;
}

char *get_skype (char *buf) {
	if (access (SKYPE_LOCK, F_OK) == 0)
		snprintf (buf, 6, SKYPE_STR);
	else
		buf[0] = '\0';
	return buf;
}

int is_up (char *device) {
	char devpath[32], state[5];
	FILE *infile;

	snprintf (devpath, 32, "/sys/class/net/%s/operstate", device);
	if ((infile = fopen (devpath, "r")) != NULL) {
		fscanf (infile, "%s", state); fclose (infile);
		if (strncmp (state, "up", 2) == 0)
			return 1;
	}
	return 0;
}

char *get_net (char *buf, struct iwreq wreq, int socket) {
	char ssid[30] = "";

	if (is_up (WIRELESS_DEVICE)) {
		wreq.u.essid.pointer = ssid;
		wreq.u.essid.length = sizeof (ssid);
		ioctl (socket, SIOCGIWESSID, &wreq);
		snprintf (buf, 30, WLAN_STR, ssid);
	} else if (is_up (WIRED_DEVICE))
		snprintf (buf, 10, LAN_STR);
	else
		snprintf (buf, 16, NO_CON_STR);
	return buf;
}

char *get_volume (char *buf, snd_mixer_t *handle) {
	snd_mixer_elem_t *pcm_mixer, *mas_mixer;
	snd_mixer_selem_id_t *vol_info, *mute_info;
	long vol = 0, max = 0, min = 0;
	int mute = 0, realvol = 0;

	snd_mixer_handle_events (handle);
	snd_mixer_selem_id_malloc (&vol_info);
	snd_mixer_selem_id_malloc (&mute_info);
	snd_mixer_selem_id_set_name (vol_info, VOL_CH);
	snd_mixer_selem_id_set_name (mute_info, VOL_CH);
	pcm_mixer = snd_mixer_find_selem (handle, vol_info);
	mas_mixer = snd_mixer_find_selem (handle, mute_info);
	snd_mixer_selem_get_playback_volume_range ((snd_mixer_elem_t *)pcm_mixer, &min, &max);
	snd_mixer_selem_get_playback_volume ((snd_mixer_elem_t *)pcm_mixer, 0, &vol);
	snd_mixer_selem_get_playback_switch (mas_mixer, 0, &mute);
	if (!mute)
		snprintf (buf, 9, VOL_MUTE_STR);
	else {
		realvol = (vol * 100) / max;
		snprintf (buf, 12, VOL_STR, realvol);
	}
	snd_mixer_selem_id_free (vol_info);
	snd_mixer_selem_id_free (mute_info);
	return buf;
}

char *get_battery (char *buf) {
	FILE *infile;
	char state[11];
	long now = -1, full = -1, voltage = -1, rate = -1;
	int perc, hours, minutes, seconds = -1;

	if (access ("/sys/class/power_supply/BAT1/", F_OK) == 0) {
		infile = fopen (BATT_STAT, "r"); fscanf (infile, "%s\n", state); fclose (infile);
		if (strncmp (state, "Full", 4) == 0)
			snprintf (buf, 11, BAT_FULL_STR);
		else if (strncmp (state, "Unknown", 7) == 0)
			snprintf (buf, 11, BAT_UNK_STR);
		else {
			infile = fopen (BATT_NOW, "r"); fscanf (infile, "%ld\n", &now); fclose (infile);
			infile = fopen (BATT_FULL, "r"); fscanf (infile, "%ld\n", &full); fclose (infile);
			infile = fopen (BATT_VOLT, "r"); fscanf (infile, "%ld\n", &voltage); fclose (infile);
			infile = fopen (BATT_CNOW, "r"); fscanf (infile, "%ld\n", &rate); fclose (infile);
			now = ((float)voltage * (float)now);
			full = ((float)voltage * (float)full);
			rate = ((float)voltage * (float)rate);
			perc = (now * 100) / full;

			if (strncmp (state, "Charging", 8) == 0)
				seconds = 3600 * (((float)full - (float)now) / (float)rate);
			else
				seconds = 3600 * ((float)now / (float)rate);
			hours = seconds / 3600;
			seconds -= 3600 * hours;
			minutes = seconds / 60;
			seconds -= 60 * minutes;
			if (strncmp (state, "Charging", 8) == 0)
				snprintf (buf, 24, BAT_CHRG_STR, perc, hours, minutes);
			else {
				/*if (perc < BAT_LOW_P || minutes < BAT_LOW_T)
					notify*/
				snprintf (buf, 24, BAT_STR, perc, hours, minutes);
			}
		}
	} else
		snprintf (buf, 14, NO_BAT_STR);

	return buf;
}

int main(void) {
	Display *dpy;
	Window root;
	char status[256], music[160], skype[6], net[30], volume[12], battery[24];
	int netloops = 60, musicloops = 10;
	DBusGProxy *session = NULL;
	DBusGConnection *connection = NULL;
	int sockfd;
	struct iwreq wreq;
	snd_mixer_t *handle;

	if (!(dpy = XOpenDisplay (NULL))) {
		fprintf (stderr, "Error: could not open display.\n");
		return 1;
	}
	root = XRootWindow (dpy, DefaultScreen (dpy));

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, NULL);
	session = dbus_g_proxy_new_for_name (connection, AUDACIOUS_DBUS_SERVICE, AUDACIOUS_DBUS_PATH, AUDACIOUS_DBUS_INTERFACE);

	memset (&wreq, 0, sizeof (struct iwreq));
	snprintf (wreq.ifr_name, 7, WIRELESS_DEVICE);
	sockfd = socket (AF_INET, SOCK_DGRAM, 0);

	snd_mixer_open (&handle, 0);
	snd_mixer_attach (handle, "default");
	snd_mixer_selem_register (handle, NULL, NULL);
	snd_mixer_load (handle);

	while (1) {
		if (++musicloops > 10) {
			musicloops = 0;
			get_aud (music, session);
		}
		get_skype (skype);
		if (++netloops > 60 && sockfd != -1) {
			netloops = 0;
			get_net (net, wreq, sockfd);
		}
		get_volume (volume, handle);
		get_battery (battery);

		snprintf (status, 256, "%s      %s      %s      %s      %s", music, skype, net, volume, battery);

		XStoreName (dpy, root, status);
		XFlush (dpy);
		sleep (INTERVAL);
	}

	XCloseDisplay (dpy);
	dbus_g_connection_unref (connection);
	g_object_unref (session);
	close (sockfd);
	snd_mixer_close (handle);
	return 0;
}
