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

from setuptools import setup


setup(name='openlst_tools',
      version="1.0.0",
      author='Planet Labs Inc.',
      author_email='hello@planet.com',
      description='Tools for communicating with OpenLST',
      classifiers=[
          "Development Status :: 4 - Beta",
          "Environment :: Console",
          "Intended Audience :: Developers",
          "Intended Audience :: Education",
          "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
          "Natural Language :: English",
          "Operating System :: POSIX :: Linux",
          "Programming Language :: Python :: 2.7",
          "Programming Language :: Python :: 3",
          "Programming Language :: Python :: 3.0",
          "Programming Language :: Python :: 3.1",
          "Programming Language :: Python :: 3.2",
          "Programming Language :: Python :: 3.3",
          "Programming Language :: Python :: 3.4",
          "Programming Language :: Python :: 3.5",
          "Programming Language :: Python :: 3.6",
          "Programming Language :: Python :: 3.7",
          "Topic :: Communications :: Ham Radio",
          "Topic :: Education",
          "Topic :: Scientific/Engineering",
          ],
      license="GNU General Public License v3 (GPLv3)",
      entry_points={
          'console_scripts': [
              'flash_bootloader=openlst_tools.flash_bootloader:main',
              'bootload_radio=openlst_tools.bootload_radio:main',
              'sign_radio=openlst_tools.sign_radio:main',
              'get_telem=openlst_tools.get_telem:main',
              'radio_mux=openlst_tools.radio_mux:main',
              'radio_terminal=openlst_tools.terminal:main',
              'radio_cmd=openlst_tools.radio_cmd:main',
              'radio_time_sync=openlst_tools.time_sync:main',
              'save_bootloader=openlst_tools.save_bootloader:main',
          ]
      },
      packages=['openlst_tools'],
      install_requires=[
          "blessed>=1.15.0,<2.0.0",
          "pyzmq>=13.1.0",
          "pycrypto>=2.6",
          "pyserial",
          "six"])
