#!/bin/sh

case $1 in
    pre) killall dwmst ;;
    post) dwmst    ;;
esac
exit
