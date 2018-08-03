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
import time
from functools import partial
from binascii import hexlify
from .intel_hex import parse_hex_file
from .commands import get_handler, ResponseError
from .arguments import hwid_type, aes_key_option
from .flash_constants import (
    FLASH_SIGNATURE_START,
    FLASH_SIGNATURE_LEN,
    FLASH_APP_START,
    FLASH_APP_END,
    FLASH_PAGE_SIZE)
from .radio_mux import UART1_RX_SOCKET, UART1_TX_SOCKET


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-r', '--rx-path',
        type=str,
        default=os.environ.get('RX_PATH', UART1_RX_SOCKET),
        help="The receive socket or serial device")
    parser.add_argument(
        '-t', '--tx-path',
        type=str,
        default=os.environ.get('TX_PATH', UART1_TX_SOCKET),
        help="The transmit socket or serial device")
    parser.add_argument(
        '-i', '--hwid',
        type=hwid_type,
        help="The HWID of the satellite or ground radio")
    sig_opt = parser.add_mutually_exclusive_group(required=True)
    sig_opt.add_argument(
        '--signature',
        # TODO clean up this
        type=partial(aes_key_option, length=FLASH_SIGNATURE_LEN),
        help="The AES CBC-MAC signature of this image (8 byte hex)")
    sig_opt.add_argument(
        '--signature-file',
        type=argparse.FileType('r'),
        help="A file with an 8 byte AES CBC-MAC hex signature")
    parser.add_argument(
        'firmware_image',
        type=argparse.FileType('r'),
        help="The firmware .hex file")

    args = parser.parse_args()

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)

    app_section = parse_hex_file(args.firmware_image.read())
    log.info("Read firmware image")
    # Insert signature
    if args.signature:
        signature = args.signature
    elif args.signature_file:
        signature = aes_key_option(
            args.signature_file.read().strip(),
            length=FLASH_SIGNATURE_LEN)
    if len(signature) != FLASH_SIGNATURE_LEN:
        raise ValueError("signature length is incorrect")
    app_section = (
        app_section[:FLASH_SIGNATURE_START] +
        signature +
        app_section[FLASH_SIGNATURE_START + FLASH_SIGNATURE_LEN:]
    )
    log.info("Inserted signature")

    con = get_handler(args.hwid, args.rx_path, args.tx_path)

    # Get in the bootloader
    in_bootloader = False
    while not in_bootloader:
        # Drop to the bootloader
        con.send_cmd_once('lst reboot', timeout=0.1)
        time.sleep(0.2)
        # Erase application
        try:
            in_bootloader = con.send_cmd_resp(
                "lst bootloader_erase",
                reply="lst bootloader_ack 1",
                timeout=2.0,
                retries=1)
        except ResponseError:
            pass
        time.sleep(1.5)
    # Load pages
    for page in xrange(FLASH_APP_START, FLASH_APP_END + 1, FLASH_PAGE_SIZE):
        page_number = page // FLASH_PAGE_SIZE
        page_data = app_section[page:page + FLASH_PAGE_SIZE]
        if all(b == 0xff for b in page_data):
            log.info("Page %d is empty, skipping" % page_number)
            continue
        con.send_cmd_resp(
            "lst bootloader_write_page %d %s" % (
                page_number,
                hexlify(page_data)),
            reply="lst bootloader_ack %d" % page_number)
    con.send_cmd("lst bootloader_write_page 255")
    # Get telem?

if __name__ == '__main__':
    main()
