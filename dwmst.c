/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * dwmst.c
 * Copyright (C) 2014 Jente Hidskes <hjdskes@gmail.com>
 *
 * dwmst is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dwmst is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <time.h>
#include <X11/Xlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <mpd/client.h>
#include <alsa/asoundlib.h>
#include <linux/wireless.h>

#include "dwmst.h"

char *
smprintf(char *fmt, ...) {
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if(ret == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

int
is_up(char *device) {
	char devpath[32], state[5];
	FILE *infile;

	snprintf(devpath, sizeof(devpath), "/sys/class/net/%s/operstate", device);
	infile = fopen(devpath, "r");
	if(infile != NULL) {
		fscanf(infile, "%s", state);
		fclose(infile);
		if(strncmp(state, "up", 2) == 0)
			return 1;
	}
	return 0;
}

char *
get_net(struct iwreq wreq, int sockfd) {
	if(is_up(WIRED_DEVICE))
		return LAN;
	else if(is_up(WIRELESS_DEVICE) && sockfd != -1) {
		char essid[IW_ESSID_MAX_SIZE + 1];

		wreq.u.essid.pointer = essid;
		wreq.u.essid.length = sizeof(essid);
		ioctl(sockfd, SIOCGIWESSID, &wreq);
		return smprintf(essid);
	} else
		return NO_CON;
}

char *
get_mpd(void) {
	struct mpd_connection *con;
	struct mpd_status *status;
	struct mpd_song *song;
	int status_type;
	char *res;
	const char *artist = NULL, *title = NULL;

	con = mpd_connection_new(NULL, 0, 30000);
	if(mpd_connection_get_error(con)) {
		mpd_connection_free(con);
		return NO_MPD;
	}

	mpd_command_list_begin(con, true);
	mpd_send_status(con);
	mpd_send_current_song(con);
	mpd_command_list_end(con);

	status = mpd_recv_status(con);
	if(!status) {
		mpd_connection_free(con);
		return NO_MPD;
	}
	mpd_response_next(con);
	song = mpd_recv_song(con);
	title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	if(!title)
		title = mpd_song_get_uri(song);
	artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

	status_type = mpd_status_get_state(status);
	switch(status_type) {
		case(MPD_STATE_PLAY):
			res = smprintf(MPD, "Playing", artist, title);
			break;
		case(MPD_STATE_PAUSE):
			res = smprintf(MPD, "Paused", artist, title);
			break;
		case(MPD_STATE_STOP):
			res = smprintf(MPD, "Stopped", artist, title);
			break;
		default:
			res = NO_MPD;
			break;
	}
	mpd_song_free(song);
	mpd_response_finish(con);
	mpd_status_free(status);
	mpd_connection_free(con);
	return res;
}

char *
get_vol(snd_mixer_t *handle, snd_mixer_elem_t *elem) {
	int mute = 0;
	long vol, max, min;

	snd_mixer_handle_events(handle);
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_get_playback_volume(elem, 0, &vol);
	snd_mixer_selem_get_playback_switch(elem, 0, &mute);

	return smprintf(mute == 0 ? VOL_MUTE : VOL, (vol * 100) / max);
}

char *
get_bat(void) {
	FILE *f;
	char state;
	float now, full, voltage, rate;
	int perc, minutes, hours;

	if(access("/sys/class/power_supply/BAT1/", F_OK) != 0)
		return NO_BAT;

	f = fopen(BAT_STATE, "r"); if(f == NULL) return BAT_UNK;
			state = (char)fgetc(f); fclose(f);

	if(state == 'F')
		return BAT_FULL;
	if(state == 'U')
		return BAT_UNK;

	f = fopen(BAT_FULLL, "r"); if(f == NULL) return BAT_UNK;
			fscanf(f, "%f", &now); fclose(f);
	f = fopen(BAT_NOW, "r"); if(f == NULL) return BAT_UNK;
			fscanf(f, "%f", &full); fclose(f);
	f = fopen(BAT_VOLTAGE, "r"); if(f == NULL) return BAT_UNK;
			fscanf(f, "%f", &voltage); fclose(f);
	f = fopen(BAT_CURRENT, "r"); if(f == NULL) return BAT_UNK;
			fscanf(f, "%f", &rate); fclose(f);

	now *= voltage;
	full *= voltage;
	rate *= voltage;
	perc = (now * 100) / full;

	if(state == 'C') {
		minutes = 60 * ((full - now) / rate);
		hours = minutes / 60;
		minutes -= 60 * hours;
		return smprintf(BAT_CHRG, perc > 100 ? 100 : perc, hours > 24 ?
				24 : hours, minutes);
	} else {
		minutes = 60 * (now / rate);
		hours = minutes / 60;
		minutes -= 60 * hours;
		return smprintf(BAT_DIS, perc > 100 ? 100 : perc, hours > 24 ? 
				24 : hours, minutes);
	}
}

char *
get_time(void) {
	char clock[38];
	time_t current;

	time(&current);
	if(!strftime(clock, sizeof(clock) - 1, CLK, localtime(&current)))
		return NO_CLK;
	return smprintf(clock);
}

void
setstatus(Display *dpy, char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

void
cleanup(Display *dpy, int sockfd, snd_mixer_t *handle,
		snd_mixer_selem_id_t *vol_info)
{
	if(sockfd != -1)
		close(sockfd);
	XCloseDisplay(dpy);
	snd_mixer_selem_id_free(vol_info);
	snd_mixer_close(handle);
}

int
main(void) {
	Display *dpy;
	struct iwreq wreq;
	snd_mixer_t *handle;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *vol_info;
	int sockfd, loops = 60;
	char *status, *mpd, *net, *vol, *bat, *clk;

	if(!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmst: cannot open display.\n");
		exit(EXIT_FAILURE);
	}

	memset(&wreq, 0, sizeof(struct iwreq));
	snprintf(wreq.ifr_name, sizeof(WIRELESS_DEVICE), WIRELESS_DEVICE);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, "default");
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);
	snd_mixer_selem_id_malloc(&vol_info);
	snd_mixer_selem_id_set_name(vol_info, VOL_CH);
	elem = snd_mixer_find_selem(handle, vol_info);
	if(elem == NULL) {
		fprintf(stderr, "dwmst: can not open device.\n");
		cleanup(dpy, sockfd, handle, vol_info);
		exit(EXIT_FAILURE);
	}

	for(;;sleep(INTERVAL)) {
		if(++loops > 60) {
			loops = 0;
			mpd = get_mpd();
			net = get_net(wreq, sockfd);
			bat = get_bat();
			clk = get_time();
		}
		vol = get_vol(handle, elem);
		status = smprintf("%s  %s  %s  %s  %s", mpd, net, vol, bat, clk);
		setstatus(dpy, status);
		free(vol);
		free(status);
	}

	free(mpd);
	free(net);
	free(bat);
	free(clk);
	cleanup(dpy, sockfd, handle, vol_info);
	exit(EXIT_SUCCESS);
}
