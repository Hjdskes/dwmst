#define WIRED_DEVICE    "enp3s0"
#define WIRELESS_DEVICE "wlp5s0"
#define BAT_LOW_P       11
#define BAT_LOW_T       3
#define INTERVAL        1
#define VOL_CH          "Master"

#define SKYPE_LOCK      "/home/jente/.Skype/jente_etnej/main.lock"
#define BATT_NOW        "/sys/class/power_supply/BAT0/charge_now"
#define BATT_FULL       "/sys/class/power_supply/BAT0/charge_full"
#define BATT_STAT       "/sys/class/power_supply/BAT0/status"
#define BATT_VOLT       "/sys/class/power_supply/BAT0/voltage_now"
#define BATT_CNOW       "/sys/class/power_supply/BAT0/current_now"

#ifdef MPD
#define MPD_STR	        "\x02%s \x01 - \x02 %s"
#define MPD_P_STR       "\x01Paused: \x02 %s \x01 - \x02 %s"
#define MPD_S_STR       ""
#define NO_MPD_STR      "\x01Geen verbinding"
#endif
#ifdef AUD
#define AUD_STR         "\x02%s"
#define AUD_P_STR       "\x01%s"
#define AUD_S_STR       ""
#endif
#define SKYPE_STR       "\x02Skype"
#define NO_SKYPE_STR    ""
#define LAN_STR         "\x01Verbonden"
#define WLAN_STR        "\x01%s \x02%d%%"
#define NO_CON_STR      "\x01Geen verbinding"
#define VOL_STR         "\x01Volume \x02%d%%"
#define VOL_MUTE_STR    "\x01Volume \x02M"
#define BAT_FULL_STR    "\x01 Batterij\x02 F"
#define BAT_STR         "\x01 Batterij\x02 D %d%%\x01, \x02%02d:%02d resterend"
#define BAT_LOW_STR     "\x01 Batterij\x02 D %d%%\x01, \x02%02d:%02d resterend"
#define BAT_CHRG_STR    "\x01 Batterij\x02 C %d%%\x01, \x02%02d:%02d tot opgeladen"
#define BAT_UNK_STR     "\x01 Batterij\x02 U"
