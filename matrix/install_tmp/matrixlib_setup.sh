#!/bin/bash
#
# Set ldconfig path to library folder.
# get list of search directories: ldconfig -v 2>/dev/null | grep -v ^$'\t'
sudo sh -c "echo '/opt/Kemek/lib' > /etc/ld.so.conf.d/matrix.ld.so.conf"
# update ldconfig cache:
sudo rm /etc/ld.so.cache
sudo ldconfig
# print current paths:
ldconfig -v 2>/dev/null | grep -v ^$'\t'
