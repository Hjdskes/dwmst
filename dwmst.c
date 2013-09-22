#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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

#include "dwmst.h"

FILE *infile;
int skfd;
struct wireless_info *winfo;
#ifdef AUD
DBusGProxy *session = NULL;
DBusGConnection *connection = NULL;
#endif

#ifdef MPD
char *get_mpd(char *buf) {
	struct mpd_song *song = NULL;
	struct mpd_connection *conn = NULL;
	struct mpd_status *mpd_status = NULL;
	const char *title = NULL, *artist = NULL;

	if (mpd_connection_get_error(conn))
		sprintf(buf, NO_MPD_STR);
	mpd_command_list_begin(conn, true);
	mpd_send_status(conn);
	mpd_send_current_song(conn);
	mpd_command_list_end(conn);
	mpd_status = mpd_recv_status(conn);
	if (!mpd_status)
		sprintf(buf, NO_MPD_STR);
	else {
		if (mpd_status_get_state(mpd_status) == MPD_STATE_PLAY) {
			mpd_response_next(conn);
			song = mpd_recv_song(conn);
			title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
			artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
			sprintf(buf, MPD_STR, title, artist);
			mpd_song_free(song);
			free(title);
			free(artist);
		} else if (mpd_status_get_state(mpd_status) == MPD_STATE_PAUSE) {
			mpd_response_next(conn);
			song = mpd_recv_song(conn);
			title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
			artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
			sprintf(buf, MPD_P_STR, title, artist);
			mpd_song_free(song);
			free(title);
			free(artist);
		} else if (mpd_status_get_state(mpd_status) == MPD_STATE_STOP)
			sprintf(buf, MPD_S_STR);
	}
	mpd_response_finish(conn);
	return buf;
}
#endif
#ifdef AUD
char *get_aud(char *buf) {
	char *psong = NULL;

	psong = audacious_remote_get_playlist_title(session, audacious_remote_get_playlist_pos(session));
	if (psong) {
		if (audacious_remote_is_paused(session)) {
			sprintf(buf, AUD_P_STR, psong);
			free(psong);
		} else if (audacious_remote_is_playing(session)) {
			sprintf(buf, AUD_STR, psong);
			free(psong);
		}
	} else
			sprintf(buf, AUD_S_STR);
	return buf;
}
#endif

char *get_skype(char *buf) {
	if(access(SKYPE_LOCK, F_OK) == 0)
		sprintf(buf, SKYPE_STR);
	else
		sprintf(buf, NO_SKYPE_STR);
	return buf;
}

int is_up(char *device) {
	char devpath[35], state[5];

	sprintf(devpath, "/sys/class/net/%s/operstate", device);
	infile = fopen(devpath, "r");
	if(infile != NULL) {
		fscanf(infile, "%s", state);
		fclose(infile);
		if(strcmp(state, "up") == 0)
			return 1;
	}
	return 0;
}

char *get_net(char *buf) {
	if (is_up(WIRED_DEVICE))
		sprintf(buf, LAN_STR);
	else if (is_up(WIRELESS_DEVICE)) {
		if (iw_get_basic_config(skfd, WIRELESS_DEVICE, &(winfo->b)) > -1) {
			if (iw_get_stats(skfd, WIRELESS_DEVICE, &(winfo->stats), &winfo->range, winfo->has_range) >= 0)
				winfo->has_stats = 1;
			if (iw_get_range_info(skfd, WIRELESS_DEVICE, &(winfo->range)) >= 0)
				winfo->has_range = 1;
			if (winfo->b.has_essid && winfo->b.essid_on) {
					sprintf(buf, WLAN_STR, winfo->b.essid, (winfo->stats.qual.qual * 100) / winfo->range.max_qual.qual);
			}
		}
	} else
		sprintf(buf, NO_CON_STR);
	return buf;
}

char *get_volume(char *buf) {
	long vol = 0, max = 0, min = 0;
	int mute = 0, realvol = 0;

	snd_mixer_t *handle;
	snd_mixer_elem_t *pcm_mixer, *mas_mixer;
	snd_mixer_selem_id_t *vol_info, *mute_info;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, "default");
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);
	snd_mixer_selem_id_malloc(&vol_info);
	snd_mixer_selem_id_malloc(&mute_info);
	snd_mixer_selem_id_set_name(vol_info, VOL_CH);
	snd_mixer_selem_id_set_name(mute_info, VOL_CH);
	pcm_mixer = snd_mixer_find_selem(handle, vol_info);
	mas_mixer = snd_mixer_find_selem(handle, mute_info);
	snd_mixer_selem_get_playback_volume_range((snd_mixer_elem_t *)pcm_mixer, &min, &max);
	snd_mixer_selem_get_playback_volume((snd_mixer_elem_t *)pcm_mixer, SND_MIXER_SCHN_MONO, &vol);
	snd_mixer_selem_get_playback_switch(mas_mixer, SND_MIXER_SCHN_MONO, &mute);
	if (!mute)
		sprintf(buf, VOL_MUTE_STR);
	else {
		realvol = (vol * 100) / max;
		sprintf(buf, VOL_STR, realvol);
	}
	if (vol_info)
		snd_mixer_selem_id_free(vol_info);
	if (mute_info)
		snd_mixer_selem_id_free(mute_info);
	if (handle)
		snd_mixer_close(handle);
	return buf;
}

char *get_battery(char *buf) {
	char state[8];
	long now = -1, full = -1, voltage = -1, rate = -1;
	int perc, hours, minutes, seconds = -1;

	infile = fopen(BATT_NOW, "r"); fscanf(infile, "%ld\n", &now); fclose(infile);
	infile = fopen(BATT_FULL, "r"); fscanf(infile, "%ld\n", &full); fclose(infile);
	infile = fopen(BATT_STAT, "r"); fscanf(infile, "%s\n", state); fclose(infile);
	infile = fopen(BATT_VOLT, "r"); fscanf(infile, "%ld\n", &voltage); fclose(infile);
	infile = fopen(BATT_CNOW, "r"); fscanf(infile, "%ld\n", &rate); fclose(infile);
	now = ((float)voltage * (float)now);
	full = ((float)voltage * (float)full);
	rate = ((float)voltage * (float)rate);
	perc = (now * 100) / full;
	if (strncmp(state, "Full", 8) == 0)
		sprintf(buf, BAT_FULL_STR);
	else if (strncmp(state, "Unknown", 8) == 0)
		sprintf(buf, BAT_UNK_STR);
	else {
		if (strncmp(state, "Charging", 8) == 0)
			seconds = 3600 * (((float)full - (float)now) / (float)rate);
		else
			seconds = 3600 * ((float)now / (float)rate);
		hours = seconds / 3600;
		seconds -= 3600 * hours;
		minutes = seconds / 60;
		seconds -= 60 * minutes;
		if (strncmp(state, "Charging", 8) == 0)
			sprintf(buf, BAT_CHRG_STR, perc, hours, minutes);
		else {
			if (perc < BAT_LOW_P || minutes < BAT_LOW_T)
				sprintf(buf, BAT_LOW_STR, perc, hours, minutes);
			else
				sprintf(buf, BAT_STR, perc, hours, minutes);
		}
	}
	return buf;
}

int main(void) {
	Display *dpy;
	Window root;
	char status[201], music[100], skype[7], net[30], volume[14], battery[35];
	int netloops = 60, musicloops = 10;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		return 1;
	}
	root = XRootWindow(dpy, DefaultScreen(dpy));
	winfo = malloc(sizeof(struct wireless_info));
	memset(winfo, 0, sizeof(struct wireless_info));

#ifdef MPD
	conn = mpd_connection_new(NULL, 0, 30000);
#endif
#ifdef AUD
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	session = dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE, AUDACIOUS_DBUS_PATH, AUDACIOUS_DBUS_INTERFACE);
#endif
	skfd = iw_sockets_open();

	while(1) {
#ifdef MPD
		if (++musicloops > 10) {
			musicloops = 0;
			get_mpd(music);
		}
#endif
#ifdef AUD
		if (++musicloops > 10) {
			musicloops = 0;
			get_aud(music);
		}
#endif
		get_skype(skype);
		if (++netloops > 60) {
			netloops = 0;
			get_net(net);
		}
		get_volume(volume);
		get_battery(battery);

		sprintf(status, "%s %s %s %s %s", music, skype, net, volume, battery);

		XStoreName(dpy, root, status);
		XFlush(dpy);
		sleep(INTERVAL);
	}

	/* NEXT LINES SHOULD NEVER EXECUTE, only here to satisfy Trilby's O.C.D. ;) */
	XCloseDisplay(dpy);
#ifdef MPD
	mpd_connection_free(conn);
#endif
#ifdef AUD
	dbus_g_connection_unref(connection);
	g_object_unref(session);
#endif
	iw_sockets_close(skfd);
	return 0;
}
