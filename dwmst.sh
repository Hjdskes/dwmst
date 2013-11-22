#!/bin/sh
case $1/$2 in
	pre/*)
		echo "Going to $2..."
		/usr/bin/killall dwmst
	;;
	post/*)
		echo "Waking up from $2..."
		/usr/bin/sleep 3
		/usr/bin/dwmst
	;;
esac
