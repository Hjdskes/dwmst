#include "dwmst.h"

int main() {
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	root = XRootWindow(dpy, DefaultScreen(dpy));
	ca_context_create(&sound);
#ifdef MPD
	conn = mpd_connection_new(NULL, 0, 30000);
#endif
#ifdef AUD
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	session = dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE, AUDACIOUS_DBUS_PATH, AUDACIOUS_DBUS_INTERFACE);
#endif
	winfo = (struct wireless_info *) calloc(1, sizeof(struct wireless_info));
	memset(winfo, 0, sizeof(struct wireless_info));
	skfd = iw_sockets_open();
/* MAIN LOOP STARTS HERE */
	for (;;) {
		status[0]='\0';
	/* MUSIC */
#ifdef MPD
		if (++musicloops > 10) {
			musicloops = 0;
			if (mpd_connection_get_error(conn))
				sprintf(musicstring, NO_MPD_STR);
			mpd_command_list_begin(conn, true);
			mpd_send_status(conn);
			mpd_send_current_song(conn);
			mpd_command_list_end(conn);
			mpd_status = mpd_recv_status(conn);
			if (!mpd_status)
				sprintf(musicstring, NO_MPD_STR);
			else {
				if (mpd_status_get_state(mpd_status) == MPD_STATE_PLAY) {
					mpd_response_next(conn);
					song = mpd_recv_song(conn);
					title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
					artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
					sprintf(musicstring, MPD_STR, title, artist);
					mpd_song_free(song);
					free(title);
					free(artist);
				} else if (mpd_status_get_state(mpd_status) == MPD_STATE_PAUSE) {
					mpd_response_next(conn);
					song = mpd_recv_song(conn);
					title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
					artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
					sprintf(musicstring, MPD_P_STR, title, artist);
					mpd_song_free(song);
					free(title);
					free(artist);
				} else if (mpd_status_get_state(mpd_status) == MPD_STATE_STOP)
					sprintf(musicstring, MPD_S_STR);
			}
			mpd_response_finish(conn);
		}
		strcat(status, musicstring);
#endif
#ifdef AUD
		if (++musicloops > 10) {
			musicloops = 0;
			psong = audacious_remote_get_playlist_title(session, audacious_remote_get_playlist_pos(session));
			if (psong) {
				if (audacious_remote_is_paused(session)) {
					sprintf(musicstring, MUSIC_P_STR, psong);
					free(psong);
				} else if (audacious_remote_is_playing(session)) {
					sprintf(musicstring, MUSIC_STR, psong);
					free(psong);
				}
			}
		}
		strcat(status, musicstring);
#endif
	/* SKYPE */
		infile = fopen(SKYPE_FILE, "r");
		if (infile) {
			sprintf(statnext, SKYPE_STR);
			fclose(infile);
			strcat(status, statnext);
		}
	/* NET */
		if (++netloops > 60) {
			netloops = 0;
			if (fopen(NET_FILE, "r")) {
				fclose(infile);
				sprintf(netstring, LAN_STR);
			} else {
				if (iw_get_basic_config(skfd, WLAN, &(winfo->b)) > -1) {
					if (iw_get_stats(skfd, WLAN, &(winfo->stats), &winfo->range, winfo->has_range) >= 0)
						winfo->has_stats = 1;
					if (iw_get_range_info(skfd, WLAN, &(winfo->range)) >= 0)
						winfo->has_range = 1;
					if (winfo->b.has_essid) {
						if (winfo->b.essid_on)
							sprintf(netstring, WLAN_STR, winfo->b.essid, (winfo->stats.qual.qual * 100) / winfo->range.max_qual.qual);
						else
							sprintf(netstring, NO_CON_STR);
					}
				}
			}
		}
		strcat(status, netstring);
	/* Audio volume */
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
		snd_mixer_selem_get_playback_volume(pcm_mixer, SND_MIXER_SCHN_MONO, &vol);
		snd_mixer_selem_get_playback_switch(mas_mixer, SND_MIXER_SCHN_MONO, &mute);
		if (!mute)
			sprintf(statnext, VOL_MUTE_STR);
		else {
			realvol = (vol * 100) / max;
			sprintf(statnext, VOL_STR, realvol);
			if (oldvol > 0 && oldvol != realvol)
				ca_context_play(sound, 0, CA_PROP_APPLICATION_NAME, "Dwmst", CA_PROP_EVENT_ID, "audio-volume-change", CA_PROP_CANBERRA_CACHE_CONTROL, "never", NULL);
			oldvol = realvol;
		}
		if (vol_info)
			snd_mixer_selem_id_free(vol_info);
		if (mute_info)
			snd_mixer_selem_id_free(mute_info);
		if (handle)
			snd_mixer_close(handle);
		strcat(status,statnext);
	/* Power / Battery */
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
			sprintf(statnext, BAT_FULL_STR);
		else if (strncmp(state, "Unknown", 8) == 0)
			sprintf(statnext, BAT_UNK_STR);
		else {
			if (strncmp(state, "Charging", 8) == 0)
				seconds = 3600 * (((float)full - (float)now) / (float)rate);
			else
				seconds = 3600 * ((float)now / (float)rate);
			hours = seconds / 3600;
			seconds -= 3600 * hours;
			minutes = seconds / 60;
			seconds -= 60 * minutes;
			if (strncmp(state, "Charging", 8) == 0) {
				sprintf(statnext, BAT_CHRG_STR, perc, hours, minutes);
				warning = 0;
			} else {
				if (perc < BAT_LOW_P || minutes < BAT_LOW_T) {
					sprintf(statnext, BAT_LOW_STR, perc, hours, minutes);
					if (!warning) {
						ca_context_play(sound, 0, CA_PROP_APPLICATION_NAME, "Dwmst", CA_PROP_EVENT_ID, "suspend-error", CA_PROP_CANBERRA_CACHE_CONTROL, "never", NULL);
						warning = 1;
					}
				} else {
					sprintf(statnext, BAT_STR, perc, hours, minutes);
					warning = 0;
				}
			}
		}
		strcat(status, statnext);
	/* Set root name */
		XStoreName(dpy, root, status);
		XFlush(dpy);
		sleep(INTERVAL);
	}
/* NEXT LINES SHOULD NEVER EXECUTE, only here to satisfy Trilby's O.C.D. ;) */
	XCloseDisplay(dpy);
	ca_context_destroy(sound);
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
