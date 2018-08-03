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
import pwd
import grp
import os
import logging
import serial
import signal
import zmq
from binascii import hexlify
from threading import Thread, Event, Lock
from Queue import Queue

ESP_START_BYTE_0 = '\x22'
ESP_START_BYTE_1 = '\x69'

DEFAULT_RX_SOCKET = 'ipc:///tmp/radiomux_rx'
DEFAULT_TX_SOCKET = 'ipc:///tmp/radiomux_tx'
DEFAULT_ECHO_SOCKET = 'ipc:///tmp/radiomux_echo'
UART1_RX_SOCKET = 'ipc:///tmp/radiomux1_rx'
UART1_TX_SOCKET = 'ipc:///tmp/radiomux1_tx'
UART1_ECHO_SOCKET = 'ipc:///tmp/radiomux1_echo'

log = logging.getLogger(__name__)


def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('stty')
    parser.add_argument('-v', '--verbose', action='store_true')
    parser.add_argument('--tx-socket',
                        default=DEFAULT_TX_SOCKET)
    parser.add_argument('--rx-socket',
                        default=DEFAULT_RX_SOCKET)
    parser.add_argument('--echo-socket',
                        default=DEFAULT_ECHO_SOCKET)
    parser.add_argument('--baud', default=115200)
    parser.add_argument('--user')
    parser.add_argument('--group')
    parser.add_argument('--mode')
    return parser


class ZMQPoller(Thread):
    def __init__(self, tx_socket, rx_socket, echo_socket, serial_port,
                 user=None, group=None, mode=None):
        self.stop = Event()
        self.tx_socket = tx_socket
        self.rx_socket = rx_socket
        self.echo_socket = echo_socket
        self.serial_port = serial_port
        self.serial_tx = SerialTx(serial_port, self)
        self.serial_rx = SerialRx(serial_port, self)
        self.rx_ready = Event()
        self.rx_lock = Lock()
        self.user = user
        self.group = group
        self.mode = mode
        super(ZMQPoller, self).__init__()

    def run(self):
        self.serial_tx.start()
        self.serial_rx.start()
        context = zmq.Context()
        self.tx = context.socket(zmq.PULL)
        self.tx.bind(self.tx_socket)
        log.debug("Transmitting from %s", self.tx_socket)
        self.rx = context.socket(zmq.PUB)
        self.rx.bind(self.rx_socket)
        log.debug("Receiving to %s", self.rx_socket)
        self.rx_ready.set()
        self.echo = context.socket(zmq.PUB)
        self.echo.bind(self.echo_socket)
        for socket in (self.tx_socket, self.rx_socket, self.echo_socket):
            filename = socket.replace("ipc://", "")
            if self.user and self.group:
                uid = pwd.getpwnam(self.user).pw_uid
                gid = grp.getgrnam(self.group).gr_gid
                os.chown(filename, uid, gid)
            if self.mode is not None:
                os.chmod(filename, self.mode)
        log.debug("Transmit echo on %s", self.echo_socket)
        poller = zmq.Poller()
        poller.register(self.tx, zmq.POLLIN)
        log.debug("ZMQ setup complete")
        while not self.stop.is_set():
            for socket, event in poller.poll(500):
                if socket == self.tx and event == zmq.POLLIN:
                    msg_parts = self.tx.recv_multipart()
                    full_msg = ''.join(msg_parts)
                    log.debug("Received message of %d parts length %d on %s",
                              len(msg_parts), len(full_msg), self.tx_socket)
                    log.debug("> %s", ''.join(hexlify(p) for p in msg_parts))
                    self.echo.send(full_msg)
                    self.serial_tx.queue.put(full_msg)

    def rx_packet(self, full_msg):
        msg_parts = [full_msg]
        self.rx_ready.wait()
        with self.rx_lock:
            self.rx.send_multipart(msg_parts)
            log.debug("Forwarded message of %d parts length %d to %s",
                      len(msg_parts), len(full_msg), self.rx_socket)
            log.debug("> %s", ''.join(hexlify(p) for p in msg_parts))

    def stop_now(self, *args, **kwargs):
        log.debug("Stop signal received")
        self.stop.set()


class SerialTx(Thread):
    daemon = True

    def __init__(self, serial_port, zmq_poller):
        self.serial_port = serial_port
        self.zmq_poller = zmq_poller
        self.queue = Queue()
        super(SerialTx, self).__init__()

    def run(self):
        log.debug("Waiting for serial messages to transmit")
        while True:
            msg = self.queue.get()[:250]
            log.debug("Sending serial message %s",
                      ''.join(hex(ord(b)) for b in msg))
            header = ESP_START_BYTE_0 + ESP_START_BYTE_1 + chr(len(msg))
            self.serial_port.write(header + msg)


class SerialRx(Thread):
    daemon = True

    def __init__(self, serial_port, zmq_poller):
        self.serial_port = serial_port
        self.zmq_poller = zmq_poller
        self.queue = Queue()
        super(SerialRx, self).__init__()

    def read_messages(self):
        while True:
            # wait for ESP sync
            b = ""
            log.debug("Waiting for start byte 1")
            while b != ESP_START_BYTE_0:
                b = self.serial_port.read(1)
            log.debug("Waiting for start byte 2")
            while b == ESP_START_BYTE_0:
                b = self.serial_port.read(1)
            if b != ESP_START_BYTE_1:
                continue
            length = ord(self.serial_port.read(1))
            log.debug("Length is %d", length)
            packet = self.serial_port.read(length)
            log.debug("Got message")
            yield packet

    def run(self):
        log.debug("Listening for serial messages")
        for msg in self.read_messages():
            self.zmq_poller.rx_packet(msg)


def main():
    parser = get_parser()
    args = parser.parse_args()
    logging.basicConfig()
    if args.verbose:
        log.setLevel(logging.DEBUG)

    serial_port = serial.Serial(
        port=args.stty,
        baudrate=args.baud,
        rtscts=False,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE)
    print serial_port
    zmq_poller = ZMQPoller(
        tx_socket=args.tx_socket,
        rx_socket=args.rx_socket,
        echo_socket=args.echo_socket,
        serial_port=serial_port,
        user=args.user,
        group=args.group,
        mode=None if args.mode is None else int(args.mode, 8))

    signal.signal(signal.SIGTERM, zmq_poller.stop_now)
    signal.signal(signal.SIGINT, zmq_poller.stop_now)
    zmq_poller.run()
