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

Vagrant.configure("2") do |config|
  config.ssh.username = "vagrant"
  config.vm.box = "ubuntu/xenial64"
  config.vm.provision "shell", path: "open-lst/tools/install_cctool.sh"
  config.vm.provision "shell", path: "open-lst/tools/install_python_tools.sh"
  config.vm.synced_folder ".", "/home/vagrant/project"
  config.vm.provider "virtualbox" do |vb|
    vb.memory = 2048  # building cctool requires 2GB of RAM
    vb.customize ["modifyvm", :id, "--usb", "on"]
    vb.customize ["modifyvm", :id, "--usbehci", "on"]
    vb.customize ["usbfilter", "add", "0", 
      "--target", :id,
      "--name", "CC Debugger",
      "--manufacturer", "Texas Instruments",
      "--product", "CC Debugger"]
    vb.customize ["usbfilter", "add", "0",
      "--target", :id,
      "--name", "TTL232R-3V3",
      "--manufacturer", "FTDI",
      "--product", "TTL232R-3V3"]
  end
end
