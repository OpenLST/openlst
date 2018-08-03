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
from .commands import get_handler
from .arguments import hwid_type
from .radio_mux import UART1_RX_SOCKET, UART1_TX_SOCKET

TELEM_FIELDS = (
    "reserved",
    "uptime",
    "uart0_rx_count",
    "uart1_rx_count",
    "rx_mode",
    "tx_mode",
    "adc0",
    "adc1",
    "adc2",
    "adc3",
    "adc4",
    "adc5",
    "adc6",
    "adc7",
    "adc8",
    "adc9",
    "last_rssi",
    "last_lqi",
    "last_freqest",
    "packets_sent",
    "cs_count",
    "packets_good",
    "packets_rejected_checksum",
    "packets_rejected_reserved",
    "packets_rejected_other",
    "reserved0",
    "reserved1",
    "custom0",
    "custom1",
)


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

    args = parser.parse_args()

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)

    con = get_handler(args.hwid, args.rx_path, args.tx_path)
    resp = con.send_cmd("lst get_telem")
    print resp
    for field, val in zip(TELEM_FIELDS, resp.split()[2:]):
        print field, val

if __name__ == '__main__':
    main()
