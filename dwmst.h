#define WIRED_DEVICE    "enp9s0"
#define WIRELESS_DEVICE "wlp3s0"
#define BAT_LOW_P       11
#define BAT_LOW_T       3
#define INTERVAL        1
#define VOL_CH          "Master"

#define SKYPE_LOCK      "/home/jente/.Skype/jente_etnej/main.lock"
#define BATT_NOW        "/sys/class/power_supply/BAT1/energy_now"
#define BATT_FULL       "/sys/class/power_supply/BAT1/energy_full"
#define BATT_STAT       "/sys/class/power_supply/BAT1/status"
#define BATT_VOLT       "/sys/class/power_supply/BAT1/voltage_now"
#define BATT_CNOW       "/sys/class/power_supply/BAT1/power_now"

#ifdef MPD
#define MPD_STR	        "%s - %s"
#define MPD_P_STR       "Paused: %s - %s"
#define MPD_S_STR       ""
#define NO_MPD_STR      "Geen verbinding"
#endif
#ifdef AUD
#define AUD_STR         "%s"
#define AUD_P_STR       "%s"
#define AUD_S_STR       ""
#endif
#define SKYPE_STR       "Skype"
#define NO_SKYPE_STR    ""
#define LAN_STR         "Verbonden"
#define WLAN_STR        "%s %d%%"
#define NO_CON_STR      "Geen verbinding"
#define VOL_STR         "Volume %d%%"
#define VOL_MUTE_STR    "Volume M"
#define BAT_FULL_STR    "Batterij F"
#define BAT_STR         "Batterij D%d%%, %02d:%02d"
#define BAT_LOW_STR     "Batterij D%d%%, %02d:%02d"
#define BAT_CHRG_STR    "Batterij C%d%%, %02d:%02d"
#define BAT_UNK_STR     "Batterij U"
#define NO_BAT_STR      "Geen batterij"
