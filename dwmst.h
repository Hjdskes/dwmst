#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <iwlib.h>
#include <alsa/asoundlib.h>
#include <canberra.h>
#ifdef MPD
#include <mpd/client.h>
#endif
#ifdef AUD
#include <audacious/dbus.h>
#include <audacious/audctrl.h>
#endif

#define LAN             "enp3s0"    /* Wired interface */
#define WLAN            "wlp5s0"    /* Wireless interface */
#define BAT_LOW_P       11          /* Below BAT_LOW percentage left on battery, the battery display turns red */
#define BAT_LOW_T       3           /* Same as above, but now minutes instead of percentage */
#define INTERVAL        1           /* Sleeps for INTERVAL seconds between updates */
#define VOL_CH          "Master"    /* Channel to watch for volume */
/* Files read for system info: */
#define SKYPE_FILE      "/home/jente/.Skype/jente_etnej/main.lock"
#define NET_FILE        "/sys/class/net/LAN/carrier"
#define BATT_NOW        "/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL       "/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT       "/sys/class/power_supply/BAT0/status"
#define BATT_VOLT       "/sys/class/power_supply/BAT0/voltage_now"
#define BATT_CNOW       "/sys/class/power_supply/BAT0/current_now"
/* Display format strings. Defaults make extensive use of escape characters for colors which require colorstatus patch. */
#ifdef MPD
#define MPD_STR	        "\x02%s \x01 - \x02 %s "                               /* MPD, playing */
#define MPD_P_STR       "Paused: \x02 %s \x01 - \x02 %s "                      /* MPD, paused */
#define MPD_S_STR       ""                                                     /* MPD, stopped */
#define NO_MPD_STR      "Geen verbinding "                                     /* MPD, can't connect */
#endif
#ifdef AUD
#define MUSIC_STR       "\x02%s "                                              /* Music, playing */
#define MUSIC_P_STR     "P: \x02%s "                                           /* Music, paused */
#define MUSIC_S_STR     ""                                                     /* Music, stopped */
#endif
#define SKYPE_STR       "\x02Skype \x01"                                       /* Skype is running */
#define LAN_STR         "Verbonden \x01"                                       /* LAN */
#define WLAN_STR        "%s \x02%d%% \x01"                                     /* WLAN */
#define NO_CON_STR      "Geen verbinding \x01"                                 /* WLAN, no connection */
#define VOL_STR         "Volume \x02%d%% \x01"                                 /* Volume */
#define VOL_MUTE_STR    "Volume \x02M \x01"                                    /* Volume, muted */
#define BAT_FULL_STR    "Batterij\x02 F "                                      /* Battery, full */
#define BAT_STR         "Batterij\x02 D %d%%\x01, \x02%02d:%02d resterend "        /* Battery, discharging, above BATT_LOW percentage */
#define BAT_LOW_STR     "Batterij\x02 D %d%%\x01, \x02%02d:%02d resterend "        /* Battery, discharging, below BATT_LOW percentage */
#define BAT_CHRG_STR    "Batterij\x02 C %d%%\x01, \x02%02d:%02d tot opgeladen "    /* Battery, AC */
#define BAT_UNK_STR     "Batterij\x02 U "                                      /* Battery, unknown */

Display *dpy;
Window root;
FILE *infile;
ca_context *sound;
char state[8], statnext[100], status[200], netstring[30], musicstring[100];
long now = -1, full = -1, voltage = -1, rate = -1, vol = 0, max = 0, min = 0;
int warning, perc, hours, minutes, seconds = -1, skfd, mute = 0, oldvol = -1, realvol = 0, netloops = 60, musicloops = 10;
struct wireless_info *winfo;
snd_mixer_t *handle;
snd_mixer_elem_t *pcm_mixer, *mas_mixer;
snd_mixer_selem_id_t *vol_info, *mute_info;
#ifdef MPD
struct mpd_song *song = NULL;
struct mpd_connection *conn = NULL;
struct mpd_status *mpd_status = NULL;
const char *title = NULL, *artist = NULL;
#endif
#ifdef AUD
char *psong = NULL;
DBusGProxy *session = NULL;
DBusGConnection *connection = NULL;
#endif
