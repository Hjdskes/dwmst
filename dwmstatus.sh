#!/bin/sh

case $1 in
    pre) killall dwm-status ;;
    post) dwm-status    ;;
esac
exit
