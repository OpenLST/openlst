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
from binascii import hexlify
from Crypto.Cipher import AES
from .intel_hex import parse_hex_file
from .arguments import aes_key_option
from .flash_constants import FLASH_APP_START, FLASH_SIGNATURE_START


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-k', '--signing-key',
        type=aes_key_option,
        default=os.environ.get('SIGNING_KEY'),
        help="The AES-128 signing key (as a hex string). This can also be "
        "specified via the SIGNING_KEY environment variable")
    parser.add_argument(
        'radio_hex_in',
        type=argparse.FileType('r'),
        help="The unsigned .hex radio application file")
    parser.add_argument(
        'signature_file',
        type=argparse.FileType('w'),
        help="The signature output file")
    args = parser.parse_args()

    if args.signing_key is None:
        exit(parser.print_usage())

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)
    radio_app = parse_hex_file(args.radio_hex_in.read())
    iv = '\0' * 16
    cipher = AES.new(args.signing_key, AES.MODE_CBC, iv)
    cbc = cipher.encrypt(bytes(
        radio_app[FLASH_APP_START:FLASH_SIGNATURE_START]))
    cbcmac = cbc[-16:]
    args.signature_file.write(hexlify(cbcmac))


if __name__ == '__main__':
    main()
