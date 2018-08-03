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

import argparse
import binascii


def hwid_type(hwid_str):
    try:
        hwid = int(hwid_str, 16)
    except TypeError:
        raise argparse.ArgumentTypeError("Invalid HWID")
    if hwid <= 0x0000 or hwid >= 0xFFFF:
        raise argparse.ArgumentTypeError("Invalid HWID")
    return hwid


def aes_key_option(hex_key, length=16):
    """Convert an AES-128 key hex string to a byte string from argparse args"""
    try:
        key = binascii.unhexlify(hex_key)
    except TypeError:
        raise argparse.ArgumentTypeError(
            "AES keys must be %d byte hex strings" % length)
    if len(key) != length:
        raise argparse.ArgumentTypeError(
            "AES keys must be %d byte hex strings" % length)
    return key
