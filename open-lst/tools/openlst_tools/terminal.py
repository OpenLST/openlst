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

from __future__ import print_function
import argparse
import zmq
from blessed import Terminal
from .translator import Translator
from .translator import CMD_TREE
import sys
import threading
import readline
import time
import random


class RadioTerminal(object):
    def __init__(self, rx_socket, tx_socket, echo_socket, **kwargs):
        self.rx_socket = rx_socket
        self.tx_socket = tx_socket
        self.echo_socket = echo_socket
        self.term = Terminal()
        self.user_buffer = ''
        self.lock = threading.Lock()
        self.running = True  # Boolean flag used to signal loops to end
        self.trans = Translator()
        self.hwid = kwargs.get('hwid')  # Hardware ID for identifying the radio
        self.seqnum = kwargs.get('seqnum')  # Sequence number for cmd/response
        self.raw = kwargs.get('raw')  # Flag for displaying hex bytes vs words

    def _connect_zmq(self):
        self.context = zmq.Context()
        # Setup the ZMQ Rx socket (from radiomux to terminal); PUB/SUB model
        self.rx = self.context.socket(zmq.SUB)
        self.rx.setsockopt(zmq.SUBSCRIBE, b"")
        self.rx.connect(self.rx_socket)
        # Setup the echo socket to view commands going to radiomux
        self.echo = self.context.socket(zmq.SUB)
        self.echo.setsockopt(zmq.SUBSCRIBE, b"")
        self.echo.connect(self.echo_socket)
        # Setup the Tx socket (from terminal to radiomux); PUSH/PULL model
        self.tx = self.context.socket(zmq.PUSH)
        self.tx.connect(self.tx_socket)

        self.poller = zmq.Poller()
        self.poller.register(self.rx, zmq.POLLIN)
        self.poller.register(self.echo, zmq.POLLIN)

    def _disconnect_zmq(self):
        ''' Try to close sockets when we're finished '''
        self.rx.close()
        self.tx.close()
        print("{:^80}".format("ZMQ sockets closed."))
        print("{:^80}".format("goodbye."))

    def run(self):
        ''' Startup the ZMQ watcher thread, then run the user input loop '''
        # add readline tab completion
        readline.parse_and_bind("tab: complete")
        readline.set_completer(self.tab_completer)
        # create and start a ZMQ thread to monitor and display the ZMQ sockets
        zmq_thread = threading.Thread(
            target=self._zmq_watcher_thread, name='zmq_thread')
        zmq_thread.daemon = True
        zmq_thread.start()
        # Otherwise enter a loop to collect and process user input
        while self.running:
            self._input_loop()

        while zmq_thread.is_alive():
            time.sleep(.1)

    def _zmq_watcher_thread(self):
        ''' Monitor and print from ZMQ sockets '''
        self._connect_zmq()
        while self.running:
            msgs = dict(self.poller.poll(50))
            for sock, msg in msgs.iteritems():
                if sock == self.rx:
                    msg = self.rx.recv()
                    self.insert_msg("<", self._process_zmq_msg(msg))
                elif sock == self.echo:
                    msg = self.echo.recv()
                    # Suppress echos if seqnum matches; most likely from us
                    if self._seqnum_matches(msg):
                        continue
                    self.insert_msg(">", self._process_zmq_msg(msg))
        self._disconnect_zmq()

    def _process_zmq_msg(self, msg):
        ''' Deal with the raw data vs translated data '''
        if self.raw:
            return bytearray(msg)
        return self.trans.string_from_bytes(msg)

    def insert_msg(self, prompt, msg):
        ''' Clear the current line, print some text, then reprint the line '''
        if type(msg) == str:
            msg = bytearray(msg)
        if self.raw:
            msg = ' '.join('{:02x}'.format(x) for x in msg)
        self.clear_line()
        with self.term.location(x=0):
            print("{} {}".format(prompt, msg))
        self.print_user_buffer()

    def print_user_buffer(self):
        ''' Print the current contents of the user buffer '''
        with self.term.location(x=0):
            print("> {}".format(self.user_buffer), end='')
        print(self.term.move_x(len(self.user_buffer) + 2), end='')
        sys.stdout.flush()

    def send_user_command(self, cmd):
        try:
            cmd = self.trans.bytes_from_string(
                self.hwid, self.seqnum, cmd)
        except Exception:
            print("[ERROR] Invalid command: {}".format(self.user_buffer))
            return
        if self.raw:
            self.insert_msg(">", cmd)
        self.seqnum = (self.seqnum + 1) % (16**4)
        self.tx.send(cmd)

    def clear_line(self):
        self.lock.acquire()
        self.user_buffer = readline.get_line_buffer()
        with self.term.location(x=0):
            print(self.term.clear_eol(), end='')
        self.lock.release()
        return

    def _input_loop(self):
        try:
            self.user_buffer = raw_input('> ')
            self._command_preprocessor()
        except (KeyboardInterrupt, EOFError):
            if readline.get_line_buffer() != '':
                return
            print("\n\n{:^80}".format("Caught signal; closing ZMQ..."))
            self.running = False

    def tab_completer(self, text, index):
        self.user_buffer = readline.get_line_buffer()
        cmd_parts = self.user_buffer.split(' ')
        options = _navigate_tree(cmd_parts, CMD_TREE)
        _pretty_list_print(options)
        self.print_user_buffer()

    def _seqnum_matches(self, msg):
        seqnum_bytes = bytearray(msg)[2:4]
        seqnum = self.trans.int_from_bytes(seqnum_bytes)
        return ((seqnum + 1) % (16 ** 4)) == self.seqnum

    def _command_preprocessor(self):
        ''' Preprocess input to catch a few useful commands and avoid
             errors on whitespace                                     '''
        cmd_parts = self.user_buffer.split()  # split on whitespace
        # Don't error out if it was all whitespace
        if len(cmd_parts) == 0:
            return
        # Allow a few useful commands to do things in the terminal
        command_selector = {
            'h': self._hwid,
            'r': self._toggle_raw,
            'q': self._quit
        }
        try:
            func = command_selector[cmd_parts[0]]
            func(*cmd_parts[1:])
        # If we're still around, pass the command on to the ZMQ controls
        except KeyError:
            self.send_user_command(' '.join(cmd_parts))

    def _hwid(self, new_hwid=None, *args):
        try:
            assert len(args) == 0
        except AssertionError:
            print("[ERROR] Unexpected args")
            return
        if new_hwid is None:  # No HWID was given, return the current one
            print("HWID: {:04X}\n".format(self.hwid))
            return
        try:
            new_hwid = int(new_hwid, 16)
            assert 0 <= new_hwid <= 0xFFFF
            self.hwid = new_hwid
            print("[OK] Accepted new HWID: {:04X}\n".format(new_hwid))
        except (AssertionError, ValueError, TypeError):
            print("[ERROR] Invalid HWID: {}\n".format(new_hwid))

    def _toggle_raw(self, *args):
        try:
            assert len(args) == 0
        except AssertionError:
            print("[ERROR] Unexpected args")
            return
        self.raw = not self.raw

    def _quit(self, *args):
        try:
            assert len(args) == 0
        except AssertionError:
            print("[ERROR] Unexpected args")
            return
        self.running = False


def _pretty_list_print(l, width=80):
    ''' Prints into rows and columns based on the max character length
        assumes 80 character wide terminal                              '''
    nchar = max([len(i) for i in l]) + 2
    assert nchar < width  # things will break if columns are wider than term
    ncol = width / nchar
    nrow = len(l) / ncol
    lrow = len(l) - (nrow * ncol)
    pstr = "{{:<{nchar}}}".format(nchar=nchar) * ncol
    pstr = "\n".join([pstr for i in xrange(nrow)])
    if lrow != 0:
        pstr += "\n" + "{{:<{nchar}}}".format(nchar=nchar) * lrow
    print(pstr.format(*l))


def _navigate_tree(cmd_list, tree):
    ''' Navigates down a nested dict using space-separated keywords
        Last level should be a list                                 '''
    cmd = cmd_list.pop(0)
    if len(cmd_list) == 0:
        flat_level = tree if type(tree) == list else tree.keys()
        if cmd in flat_level:
            return [cmd]
        else:
            return filter(lambda k: k.startswith(cmd), flat_level)
    else:
        return _navigate_tree(cmd_list, tree[cmd])


def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-u', '--uart',
        choices=(0, 1),
        type=int, default=1,
        help="Select the radio UART to watch (0 or 1). The default is UART 1.")
    parser.add_argument(
        '--rx-socket',
        help="override the receive socket "
        "(takes precedence over the UART setting)")
    parser.add_argument(
        '--tx-socket',
        help="override the transmit socket "
        "(takes precedence over the UART setting)")
    parser.add_argument(
        '--echo-socket',
        help="override the echo socket "
        "(takes precedence over the UART setting)")
    parser.add_argument(
        '--hwid',
        type=lambda x: int(x, 16),
        default=0xFFFF,
        help="4-digit hexadecimal hardware ID of the radio to command")
    parser.add_argument(
        '--seqnum',
        type=lambda x: int(x, 16),
        default=random.randrange(16**4),
        help=("4-digit hexadecimal sequence number to start at. Sequence "
              "numbers are increased incrementally and used to pair commands "
              "and responses"))
    parser.add_argument(
        '--raw',
        action='store_true',
        help="Output raw hexadecimal with very limited formatting")
    return parser


def main():
    parser = get_parser()
    args = parser.parse_args()
    rx_socket = args.rx_socket or "ipc:///tmp/radiomux{}_rx".format(args.uart)
    tx_socket = args.tx_socket or "ipc:///tmp/radiomux{}_tx".format(args.uart)
    echo_socket = (
        args.echo_socket or "ipc:///tmp/radiomux{}_echo".format(args.uart))
    del args.rx_socket
    del args.tx_socket
    del args.echo_socket
    rt = RadioTerminal(
        rx_socket=rx_socket,
        tx_socket=tx_socket,
        echo_socket=echo_socket,
        **vars(args))
    rt.run()
