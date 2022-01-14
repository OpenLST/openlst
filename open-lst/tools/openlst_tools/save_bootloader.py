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

import os
import argparse
import logging
import tempfile
import subprocess
import struct
from .intel_hex import parse_hex_file, dump_hex_file
from .arguments import hwid_type, aes_key_option
from .flash_constants import (
    FLASH_SIGNATURE_KEYS, FLASH_RESERVED, FLASH_HWID, FLASH_APP_START,
    FLASH_BOOTLOADER_STORAGE, FLASH_APP_END, FLASH_APP_SIGNATURE,
    FLASH_STORAGE_START, FLASH_UPDATER_START)

LOCK_BITS = "{:02X}".format(
    0b000 << 5 |  # Write to 0 per datasheet (not used)
    0 << 4 |  # Lock the boot block (flash block 0)
    0b100 << 1 | # Lock the top 4KB of flash
    0 << 0  # Enable debug commands TODO: disable these?
)


def insert_keys(image, keys):
    for key_index, key in enumerate(keys):
        key_start = FLASH_SIGNATURE_KEYS + key_index * 16
        key_end = key_start + 16
        if key_end > FLASH_RESERVED:
            raise ValueError("too many keys (would overwrite reserved space)")
        if any(b != 0 for b in image[key_start:key_end]) and \
           image[key_start:key_end] != key:
            raise ValueError("key area is not empty for key %d" % key_index)
        image[key_start:key_end] = key


def insert_reserved(image, reserved):
    reserved_bytes = struct.pack('<H', reserved)
    if any(b != 0 for b in image[FLASH_RESERVED:FLASH_HWID]) and \
       image[FLASH_RESERVED:FLASH_HWID] != reserved_bytes:
        raise ValueError("reserved area is not empty")

    image[FLASH_RESERVED:FLASH_HWID] = struct.pack('<H', reserved)


def insert_hwid(image, hwid):
    hwid_bytes = struct.pack('<H', hwid)
    if any(b != 0 for b in image[FLASH_HWID:FLASH_APP_START]) and \
       image[FLASH_HWID:FLASH_APP_START] != hwid_bytes:
        raise ValueError("hardware ID area is not empty")
    image[FLASH_HWID:FLASH_APP_START] = hwid_bytes


def insert_application(image, app=None):
    if app is None:
        app = bytearray([0xff] * (FLASH_APP_SIGNATURE - FLASH_APP_START))
    if any(b != 0 for b in image[FLASH_APP_START:FLASH_APP_SIGNATURE]) and \
       image[FLASH_APP_START:FLASH_APP_SIGNATURE] != app:
        raise ValueError("application area is not empty")
    image[FLASH_APP_START:FLASH_APP_SIGNATURE] = app


def insert_signature(image, sig=None):
    if sig is None:
        sig = bytearray([0xff] * (FLASH_STORAGE_START - FLASH_APP_SIGNATURE))
    if any(b != 0 for b in
           image[FLASH_APP_SIGNATURE:FLASH_STORAGE_START]) and \
       image[FLASH_APP_SIGNATURE:FLASH_STORAGE_START] != sig:
        raise ValueError("signature area is not empty")
    image[FLASH_APP_SIGNATURE:FLASH_STORAGE_START] = sig


def insert_storage(image, storage=None):
    if storage is None:
        storage = bytearray([0xff] * (FLASH_UPDATER_START -
                                      FLASH_STORAGE_START))
    if any(b != 0 for b in
           image[FLASH_STORAGE_START:FLASH_UPDATER_START]) and \
       image[FLASH_STORAGE_START:FLASH_UPDATER_START] != storage:
        raise ValueError("storage area is not empty")
    image[FLASH_STORAGE_START:FLASH_UPDATER_START] = storage


def main():
    parser = argparse.ArgumentParser()
    key_source = parser.add_mutually_exclusive_group(required=True)
    key_source.add_argument(
        '--keys',
        type=aes_key_option,
        nargs=3,
        help="AES signing keys to load (in hex format)")
    key_source.add_argument(
        '--key-file',
        type=argparse.FileType('r'),
        help="A comma separated file with AES signing keys "
        "to load (in hex format)")
    key_source.add_argument(
        '--reserved',
        type=int,
        default=0,
        help="A uint16 (in hex) for the reserved bytes in the "
        "bootloader storage. Default is 0.")
    parser.add_argument(
        '-i', '--hwid',
        required=True,
        type=hwid_type,
        help="The HWID of the satellite or ground radio")
    parser.add_argument(
        'bootloader_hex',
        type=argparse.FileType('r'),
        help="The .hex radio bootloader file")
    parser.add_argument(
        '-o', '--out',
        required=True,
        type=argparse.FileType('x'),
        help="The .hex file to write to")
    args = parser.parse_args()

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)
    if args.key_file:
        keys = [
            aes_key_option(k.strip())
            for k in args.key_file.read().strip().split(',')]
    else:
        keys = args.keys
    bootloader = parse_hex_file(args.bootloader_hex.read())
    insert_hwid(bootloader, args.hwid)
    insert_keys(bootloader, keys)
    insert_application(bootloader)
    insert_signature(bootloader)
    insert_storage(bootloader)
    # with open(delete=False, suffix='.hex') as tf:
    #     tf.write(dump_hex_file(bootloader))
    args.out.write(dump_hex_file(bootloader))

if __name__ == '__main__':
    main()
