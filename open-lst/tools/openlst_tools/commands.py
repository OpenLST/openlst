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

import abc
import time
import logging
from threading import Thread, Lock
from Queue import Queue, Empty
from .translator import Translator
from .radio_mux import DEFAULT_RX_SOCKET, DEFAULT_TX_SOCKET

SEQNUM_MIN = 16
SEQNUM_MAX = 64000

ESP_START_BYTE_0 = '\x22'
ESP_START_BYTE_1 = '\x69'
ESP_HEADER = ESP_START_BYTE_0 + ESP_START_BYTE_1

log = logging.getLogger(__name__)


def get_handler(hwid, rx_path, tx_path, **kw):
    if rx_path is None:
        rx_path = DEFAULT_RX_SOCKET
    if tx_path is None:
        tx_path = DEFAULT_TX_SOCKET

    if rx_path.startswith('ipc:'):
        return ZMQCommandHandler(hwid, rx_path, tx_path)
    else:
        return SerialCommandHandler(hwid, rx_path, **kw)


class ResponseError(Exception):
    pass


class CommandHandler(object):
    __metaclass__ = abc.ABCMeta

    def __init__(self, hwid):
        if isinstance(hwid, basestring):
            self.hwid = int(hwid, 16)
        else:
            self.hwid = hwid
        self.trans = Translator()
        self.seqnum = SEQNUM_MIN

    def _inc_seqnum(self):
        self.seqnum = max((self.seqnum + 1) % SEQNUM_MAX, SEQNUM_MIN)

    def _is_reply(self, m1, m2):
        if len(m1) < 4 or len(m2) < 4:
            return False
        return m1[0:4] == m2[0:4]

    @abc.abstractmethod
    def send_message(self, msg):
        pass

    @abc.abstractmethod
    def poll_message(self, timeout):
        pass

    @abc.abstractmethod
    def flush(self):
        pass

    def _send_cmd_once(self, cmd, timeout):
        t = time.time()
        if timeout is not None:
            expires = t + timeout
        msg = self.trans.bytes_from_string(
            hwid=self.hwid, seqnum=self.seqnum,
            s=cmd)
        self.flush()
        self.send_message(msg)

        while timeout is None or time.time() <= expires:
            if timeout is None:
                poll_timeout = 1.
            else:
                poll_timeout = max(expires - time.time(), 0.)
            reply_msg = self.poll_message(timeout=poll_timeout)
            if reply_msg and self._is_reply(msg, reply_msg):
                return self.trans.string_from_bytes(reply_msg)
        return None

    def send_cmd_once(self, cmd, timeout):
        self._inc_seqnum()
        log.debug("Sending (%04X): %s", self.hwid, cmd)
        resp = self._send_cmd_once(cmd, timeout)
        if resp:
            log.debug("Response: %s", resp)
        else:
            log.debug("No response")
        return resp

    def send_cmd(self, cmd, timeout=1.2, retries=None):
        tries = 0
        log.debug("Sending (%04X): %s", self.hwid, cmd)
        self._inc_seqnum()
        while retries is None or tries <= retries:
            resp = self._send_cmd_once(cmd, timeout=timeout)
            if resp:
                log.debug("Response: %s", resp)
                return resp
            else:
                tries += 1
        log.debug("No response")
        return None

    def send_cmd_resp(self, cmd, reply="lst ack", **kwargs):
        """Send a command and expect and expect a reply"""
        resp = self.send_cmd(cmd, **kwargs)
        if reply != resp:
            raise ResponseError(
                "Reply of '%s' did not match the expected reply of '%s'" %
                (resp, reply))
        else:
            return resp


def esp_parser():
    buf = bytearray()
    while True:
        # see if there's an ESP header
        while ESP_HEADER not in buf:
            buf += yield
        packet = buf.split(ESP_HEADER, 1)[1]
        while len(packet) < 1:
            packet += yield
        length = packet[0]
        if len(packet) > 1:
            data = packet[1:]
        else:
            data = bytearray()
        while len(data) < length:
            data += yield
        data += yield data[:length]
        buf = data[length:]


_serial_connections = {}


class SerialListener(Thread):
    name = 'SerialListener'
    daemon = True

    def __init__(self, rx_socket, baud):
        import serial
        super(SerialListener, self).__init__()
        self.serial = serial.Serial(rx_socket, baudrate=baud, timeout=0.)
        self.serial_lock = Lock()
        self.messages = Queue()
        self.parser = esp_parser()
        self.parser.next()

    def update(self):
        with self.serial_lock:
            msg = self.parser.send(self.serial.read(4096))
        if msg:
            self.messages.put(msg)

    def run(self):
        while True:
            self.update()
            time.sleep(0.1)

    def write(self, *args, **kwargs):
        with self.serial_lock:
            self.serial.write(*args, **kwargs)


class SerialCommandHandler(CommandHandler):
    def __init__(self, hwid, rx_socket, baud=115200, **kw):
        global _serial_connections
        super(SerialCommandHandler, self).__init__(hwid)
        if rx_socket not in _serial_connections:
            _serial_connections[rx_socket] = SerialListener(rx_socket, baud)
            _serial_connections[rx_socket].start()
        self.listener = _serial_connections[rx_socket]

    def send_message(self, msg):
        outbuf = bytearray()
        outbuf += ESP_START_BYTE_0
        outbuf += ESP_START_BYTE_1
        outbuf += chr(len(msg))
        outbuf += msg
        self.listener.write(outbuf)

    def flush(self):
        self.listener.update()
        with self.listener.messages.mutex:
            self.listener.messages.queue.clear()

    def poll_message(self, timeout):
        try:
            return self.listener.messages.get(timeout=timeout)
        except Empty:
            return None


class ZMQCommandHandler(CommandHandler):
    def __init__(self, hwid, rx_socket, tx_socket, **kw):
        import zmq
        super(ZMQCommandHandler, self).__init__(hwid)
        self.context = zmq.Context()

        self.rx = self.context.socket(zmq.SUB)
        self.rx.setsockopt(zmq.SUBSCRIBE, b"")
        self.rx.connect(rx_socket)
        self.poller = zmq.Poller()
        self.poller.register(self.rx, zmq.POLLIN)

        self.tx = self.context.socket(zmq.PUSH)
        self.tx.connect(tx_socket)

    def send_message(self, msg):
        self.tx.send(msg)

    def flush(self):
        self.poll_message(timeout=0)

    def poll_message(self, timeout):
        msgs = dict(self.poller.poll(timeout * 1000))
        for sock, msg in msgs.iteritems():
            if sock == self.rx:
                return self.rx.recv()
        return None
