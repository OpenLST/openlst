#!/bin/bash -ex
# OpenLST
# Copyright (C) 2018 Planet Labs Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

sudo apt-get update
# Linux extra virtual includes the FTDI drivers, typically left out of VM images
sudo apt-get install --assume-yes linux-image-extra-virtual
# Git is required to clone and build cctool
sudo apt-get install --assume-yes git
# cctool build dependencies
sudo apt-get install --assume-yes build-essential pkg-config libusb-1.0-0-dev libboost-all-dev
# SDCC is luckily already bundled and distributed for Debian/Ubuntu
sudo apt-get install --assume-yes sdcc

# This mirror has version 0.26
# Bash 'if' to only pull if we're reprovisioning otherwise we fail
CCTOOL_REPO="https://github.com/dashesy/cc-tool.git"
if [ -d "cc-tool" ]; then
    cd cc-tool
    git pull ${CCTOOL_REPO}
else
    git clone ${CCTOOL_REPO}
    cd cc-tool
fi
# Version 0.26
git checkout d5bb566d7ed49e7f7ce7f80015c2ead17de77d48
# Workaround for Xenial which uses GCC 5:
# GCC 5+ adds in extra comment lines which breaks autoconf's ability to read the libboost version
CPPFLAGS="-P" ./configure

make
sudo make install

# Let the vagrant user access the USB devices (requires a vagrant reload)
sudo cp /home/vagrant/project/open-lst/tools/90-radio.rules /etc/udev/rules.d/90-radio.rules

