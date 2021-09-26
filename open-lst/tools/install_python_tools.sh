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
sudo apt-get install --assume-yes python-dev python-pip
sudo python -m pip install -e /home/vagrant/project/open-lst/tools

# Install the radio services
sudo cp /home/vagrant/project/open-lst/tools/radio@.service /etc/systemd/system/radio@.service
sudo systemctl daemon-reload
sudo systemctl enable radio@0 radio@1
