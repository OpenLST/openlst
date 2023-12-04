import binascii
import logging
import numpy as np
import random
import serial
import serial.threaded
import serial.tools.list_ports
import struct
import time

from dataclasses import dataclass

from commands import OpenLstCmds
from handler import LstProtocol

MAX_DATA_LEN = 251 - 6


def unpack_cint(data: bytes, size: int, signed: bool) -> int:
    if size == 1:
        fmt = "<b"
    elif size == 2:
        fmt = "<h"
    elif size == 4:
        fmt = "<i"

    if not signed:
        fmt = fmt.upper()

    return struct.unpack(fmt, data)[0]


def pack_cint(data: int, size: int, signed: bool) -> bytes:
    if size == 1:
        fmt = "<b"
    elif size == 2:
        fmt = "<h"
    elif size == 4:
        fmt = "<i"

    if not signed:
        fmt = fmt.upper()

    return struct.pack(fmt, data)


class OpenLst:
    def __init__(
        self,
        port: str,
        hwid: int,
        id: str = None,
        baud: int = 115200,
        rtscts: bool = False,
        timeout: float = 1,
        f_ref: float = 27e6,
    ) -> None:
        """Object for communicating with OpenLST.

        If `id` is given, all ports will be checked for one with that ID. This
        is useful when more than one serial interface is used at the same time.

        Args:
            port (str): Serial port location. Looks like /dev/tty123 on Linux and COM123 on windows.
            hwid (int): HWID of connected OpenLST.
            id (str, optional): Serial port ID.
            baud (int, optional): Serial baud rate. Defaults to 115200.
            rtscts (bool, optional): Enable flow control with RTS/CTS. Defaults to False.
            timeout (int, optional): Command timeout in seconds. Defaults to 1.
        """

        if port:
            self.ser = serial.Serial(port, baud, rtscts=rtscts)
        else:
            ports = serial.tools.list_ports.comports()

            for ser_port in ports:
                if ser_port.serial_number == id:
                    self.ser = serial.Serial(ser_port.device, baud, rtscts=rtscts)

        self.timeout = timeout
        self.hwid = hwid
        self.f_ref = f_ref

        # Create thread but don't start it yet
        self.thread = serial.threaded.ReaderThread(self.ser, LstProtocol)

        self.open = False
        self.protocol: LstProtocol = None

        # Initialize sequence number
        self.seq = random.randint(0, 65536)

        self.packets = []

    def __enter__(self):
        """Enter context"""
        self.thread.start()

        _, self.protocol = self.thread.connect()
        self.open = True

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Exit context"""
        self.open = False
        self.thread.close()

    def packets_available(self):
        """Returns number of packets in queue."""

        while self.protocol.packet_queue.qsize() > 0:
            self.packets.append(self.protocol.packet_queue.get_nowait())

        return len(self.packets)

    def get_packet(self, cmd=None, seqnum=None):
        if self.packets_available() == 0:
            return None
        elif seqnum:
            for i, packet in enumerate(self.packets):
                if packet["seq"] == seqnum:
                    return self.packets.pop(i)

            return None
        elif cmd:
            for i, packet in enumerate(self.packets):
                if packet["command"] == cmd:
                    return self.packets.pop(i)

            return None
        else:
            return self.packets.pop(0)

    def get_packet_timeout(self, cmd=None, seqnum=None, timeout=None):
        start = time.time()

        if timeout == None:
            timeout = self.timeout

        while time.time() - start < timeout:
            resp = self.get_packet(cmd, seqnum)

            if resp is not None:
                return resp

    def clean_packets(self, cmd=None):
        if self.packets_available() == 0:
            return

        if cmd:
            self.packets = [x for x in self.packets if x["command"] != cmd]
        else:
            del self.packets[:]

    def _send(self, hwid: int, cmd: int, msg: bytes = bytes()):
        packet = bytearray()

        packet.extend(b"\x22\x69")
        packet.append(6 + len(msg))
        packet.extend(struct.pack(">H", hwid))
        packet.extend(struct.pack(">H", self.seq))
        packet.append(0x01)  # TODO: figure this out
        packet.append(cmd)
        packet.extend(msg)

        self.thread.write(packet)

        seq = self.seq

        self.seq += 1
        self.seq %= 2**16

        return seq

    def receive(self) -> bytes:
        return self.get_packet(OpenLstCmds.ASCII)

    def transmit(self, msg: bytes, dest_hwid=0x0000):
        assert len(msg) <= MAX_DATA_LEN

        self._send(dest_hwid, OpenLstCmds.ASCII, msg)

    def reboot(self):
        self._send(self.hwid, OpenLstCmds.REBOOT)

    def bootloader_ping(self):
        seq = self._send(self.hwid, OpenLstCmds.BOOTLOADER_PING)

        return self.get_packet_timeout(seqnum=seq)

    def flash_erase(self):
        seq = self._send(self.hwid, OpenLstCmds.BOOTLOADER_ERASE)

        return self.get_packet_timeout(seqnum=seq)

    def flash_program_page(self, data: bytes, addr: int, check_resp: bool = True):
        assert len(data) == 128 or len(data) == 0

        msg = bytearray()
        msg.append(addr)
        msg.extend(data)

        seq = self._send(self.hwid, OpenLstCmds.BOOTLOADER_WRITE_PAGE, msg)
        resp = self.get_packet_timeout(seqnum=seq)

        if check_resp:
            assert resp is not None
            assert resp["command"] == OpenLstCmds.BOOTLOADER_ACK, hex(resp["command"])

        return resp

    def get_time(self):
        seq = self._send(self.hwid, OpenLstCmds.GET_TIME)
        resp = self.get_packet_timeout(seqnum=seq)

        assert resp is not None

        if resp["command"] == OpenLstCmds.NACK:
            return None

        t = {}

        t["s"] = unpack_cint(resp["data"][0:4], 4, False)
        t["ns"] = unpack_cint(resp["data"][4:8], 4, False)

        return t

    def set_time(self, seconds: int = None, nanoseconds: int = None):
        if seconds == None or nanoseconds == None:
            pass  # TODO: get computer time

        msg = bytearray()

        msg.extend(pack_cint(seconds, 4, False))
        msg.extend(pack_cint(nanoseconds, 4, False))

        seq = self._send(self.hwid, OpenLstCmds.SET_TIME, msg)
        resp = self.get_packet_timeout(seqnum=seq)

        assert resp is not None
        assert resp["command"] == OpenLstCmds.ACK

    def get_telem(self):
        seq = self._send(self.hwid, OpenLstCmds.GET_TELEM)
        resp = self.get_packet_timeout(seqnum=seq)

        assert resp is not None
        assert resp["command"] == OpenLstCmds.TELEM

        data = resp["data"]
        telem = {}

        telem["uptime"] = unpack_cint(data[1:5], 4, False)
        telem["uart0_rx_count"] = unpack_cint(data[5:9], 4, False)
        telem["uart1_rx_count"] = unpack_cint(data[9:13], 4, False)
        telem["rx_mode"] = unpack_cint(data[13:14], 1, False)
        telem["tx_mode"] = unpack_cint(data[14:15], 1, False)
        telem["adc"] = [unpack_cint(data[i : i + 2], 2, True) for i in range(15, 35, 2)]
        telem["last_rssi"] = unpack_cint(data[35:36], 1, True)
        telem["last_lqi"] = unpack_cint(data[36:37], 1, False)
        telem["last_freqest"] = unpack_cint(data[37:38], 1, True)
        telem["packets_sent"] = unpack_cint(data[38:42], 4, False)
        telem["cs_count"] = unpack_cint(data[42:46], 4, False)
        telem["packets_good"] = unpack_cint(data[46:50], 4, False)
        telem["packets_rejected_checksum"] = unpack_cint(data[50:54], 4, False)
        telem["packets_rejected_reserved"] = unpack_cint(data[54:58], 4, False)
        telem["packets_rejected_other"] = unpack_cint(data[58:62], 4, False)

        # TODO: reserved, custom fields

        return telem

    def set_rf_params(
        self,
        frequency: float = 437e6,
        chan_bw: float = 60268,
        drate: float = 7416,
        deviation: float = 3707,
        power: int = 0x12,
    ):
        """Sets CC1110 RF parameters.

        Output power setting currently controls the raw register value.
        Calibration is needed to correlate to actual output power.

        Args:
            frequency (float, optional): Carrier frequency (Hz). Defaults to 437e6.
            chan_bw (float, optional): Channel bandwidth (Hz). Defaults to 60268.
            drate (float, optional): Data rate (bps). Defaults to 7416.
            deviation (float, optional): FSK frequency deviation. Defaults to 3707.
            power (int, optional): Output power settings. Defaults to 0x12.

        Returns:
            Tuple of actual values of carrier frequency, channel bandwidth,
            data rate and deviation.
        """

        # f_carrier = (f_ref / 2^16) * FREQ
        FREQ = int(2**16 * frequency / self.f_ref)
        f_actual = FREQ * self.f_ref / 2**16

        # Keep offset and IF as defaults
        FREQOFF = 0
        FREQ_IF = 6

        FSCTRL0 = FREQOFF
        FSCTRL1 = FREQ_IF

        # Channel bandwidth
        if chan_bw < 60268:
            chan_bw = 60268
        elif chan_bw > 843750:
            chan_bw = 843750

        # BW_channel = f_ref / (8 * 2^CHANBW_E * (4 + CHANBW_M))
        CHANBW_E = int(19 - np.floor(np.log2(chan_bw) + 0.25))
        CHANBW_M = int(np.round(self.f_ref / (chan_bw * 8 * 2**CHANBW_E) - 4))

        assert CHANBW_E >= 0 and CHANBW_E < 4, CHANBW_E
        assert CHANBW_M >= 0 and CHANBW_M < 4, CHANBW_M

        chanbw_actual = self.f_ref / (8 * (4 + CHANBW_M) * 2**CHANBW_E)

        # Data rate
        # R_DATA = f_ref * 2^DRATE_E * (256 + DRATE_M) / 2^28
        DRATE_E = int(np.floor(np.log2(drate * 2**20 / self.f_ref)))
        DRATE_M = int(np.round(drate * 2**28 / (self.f_ref * 2**DRATE_E) - 256))

        assert DRATE_E >= 0 and DRATE_E < 16, DRATE_E
        assert DRATE_M >= 0 and DRATE_M < 256, DRATE_M

        drate_actual = self.f_ref * 2**DRATE_E * (256 + DRATE_M) / 2**28

        # Deviation
        # f_dev = f_ref * 2^DEVIATN_E * (8 + DEVIATN_M) / 2^17
        DEVIATN_E = int(np.floor(np.log2(deviation * 2**14 / self.f_ref)))
        DEVIATN_M = int(
            np.round(deviation * 2**17 / (self.f_ref * 2**DEVIATN_E) - 8)
        )

        assert DEVIATN_E >= 0 and DEVIATN_E < 7, DEVIATN_E
        assert DEVIATN_M >= 0 and DEVIATN_M < 7, DEVIATN_M

        dev_act = self.f_ref * 2**DEVIATN_E * (8 + DEVIATN_M) / 2**17

        msg = bytearray()
        msg.extend(pack_cint(FREQ, 4, False))
        msg.append(FSCTRL0)
        msg.append(FSCTRL1)
        msg.append(CHANBW_M | (CHANBW_E << 2))
        msg.append(DRATE_E)
        msg.append(DRATE_M)
        msg.append(DEVIATN_M | (DEVIATN_E << 4))
        msg.append(power & 0xFF)

        seq = self._send(self.hwid, OpenLstCmds.RF_PARAMS, msg)
        resp = self.get_packet_timeout(seqnum=seq)

        assert resp is not None
        assert resp["command"] == OpenLstCmds.ACK, hex(resp["command"])

        return f_actual, chanbw_actual, drate_actual, dev_act


if __name__ == "__main__":
    import click
    import IPython

    @click.command()
    @click.option("--port", default=None, help="Serial port")
    @click.option("--id", default=None, help="Serial interface ID")
    @click.option(
        "--rtscts", is_flag=True, default=False, help="Use RTS/CTS flow control"
    )
    @click.argument("hwid")
    def main(hwid, port, id, rtscts):
        logging.basicConfig(level="INFO")

        hwid = binascii.unhexlify(hwid)
        hwid = struct.unpack(">H", hwid)[0]

        openlst = OpenLst(port, hwid, id, rtscts=rtscts)

        with openlst:
            IPython.embed()

    main()
