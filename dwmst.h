/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * dwmst.h
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

#define WIRED_DEVICE    "enp9s0"
#define WIRELESS_DEVICE "wlp3s0"
#define INTERVAL        1
#define VOL_CH          "Master"
#define BAT_NOW         "/sys/class/power_supply/BAT1/energy_now"
#define BAT_FULLL        "/sys/class/power_supply/BAT1/energy_full"
#define BAT_STATE       "/sys/class/power_supply/BAT1/status"
#define BAT_VOLTAGE     "/sys/class/power_supply/BAT1/voltage_now"
#define BAT_CURRENT     "/sys/class/power_supply/BAT1/power_now"

#define MPD         "%s: %s - %s"
#define NO_MPD      ""
#define LAN         "Connected"
#define NO_CON      "No connection"
#define VOL_MUTE    "M %d%%"
#define VOL         "V %d%%"
#define BAT_FULL    "F"
#define BAT_DIS     "D %d%%, %02d:%02d"
#define BAT_CHRG    "C %d%%, %02d:%02d"
#define BAT_UNK     "U"
#define NO_BAT      "N"
#define CLK         "%a %d %b, %R"
#define NO_CLK      "Unable"
