import logging
import random
import serial
import serial.threaded
import serial.tools.list_ports
import struct
import time

from dataclasses import dataclass

from commands import OpenLstCmds
from handler import LstProtocol

@dataclass
class OpenLstParams:
    # Configuration
    f_ref = 27e6

    # Register values
    FSCTRL0: int
    FSCTRL1: int
    FREQ: int
    MDMCFG4: int
    MDMCFG3: int
    MDMCFG2: int
    DEVIATN: int
    PA_CONFIG: int

    def set_if(self, f_if) -> float:
        FREQ_IF = int(2**10 * f_if / self.f_ref)
        f_if_actual = self.f_ref * FREQ_IF / (2**10)

        self.FSCTRL1 = FREQ_IF

        return f_if_actual

    def set_freqoff(self):
        "TODO"
        pass

    def set_freq(self, f_carrier) -> float:
        FREQ = int(2**16 * f_carrier / self.f_ref)
        f_carrier_actual = self.f_ref * FREQ / (2**16)

        self.FREQ = FREQ

        return f_carrier_actual

    def set_datarate(self, baud) -> bool:
        pass


class OpenLst:
    def __init__(self, port: str, id: str, hwid: int, baud=115200, rtscts=False, timeout=1) -> None:
        """Object for communicating with OpenLST.

        If `id` is given, all ports will be checked for one with that ID. This
        is useful when more than one serial interface is used at the same time.

        Args:
            port (str): Serial port location. Looks like /dev/tty123 on Linux and COM123 on windows.
            id (str): Serial port ID.
            hwid (int): HWID of connected OpenLST.
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
        self.hwid = int(hwid)

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
                if packet['seq'] == seqnum:
                    return self.packets.pop(i)

            return None
        elif cmd:
            for i, packet in enumerate(self.packets):
                if packet['command'] == cmd:
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
            self.packets = [x for x in self.packets if x['command'] != cmd]
        else:
            del self.packets[:]

    def _send(self, hwid: int, cmd: int, msg: bytes = bytes()):
        packet = bytearray()

        packet.extend(b'\x22\x69')
        packet.append(6 + len(msg))
        packet.extend(struct.pack(">H", hwid))
        packet.extend(struct.pack(">H", self.seq))
        packet.append(0x01) # TODO: figure this out
        packet.append(cmd)
        packet.extend(msg)

        self.thread.write(packet)

        seq = self.seq

        self.seq += 1
        self.seq %= 2**16

        return seq

    def receive():
        pass

    def transmit():
        pass

    def reboot(self):
        self._send(self.hwid, OpenLstCmds.REBOOT)

    def bootloader_ping(self):
        seq = self._send(self.hwid, OpenLstCmds.BOOTLOADER_PING)

        return self.get_packet_timeout(seqnum=seq)

    def flash_erase(self):
        seq = self._send(self.hwid, OpenLstCmds.BOOTLOADER_ERASE)

        return self.get_packet_timeout(seqnum=seq)

    def flash_program_page(self, data: bytes, addr: int):
        assert len(data) == 128

        msg = bytearray()
        msg.append(addr)
        msg.extend(data)

        seq = self._send(self.hwid, OpenLstCmds.BOOTLOADER_WRITE_PAGE)
        resp = self.get_packet_timeout(seqnum=seq)
        assert resp is not None
        assert resp['command'] == OpenLstCmds.BOOTLOADER_ACK

        return resp

    def get_time():
        pass

    def set_time():
        pass

    def get_telem(self):
        pass

    def set_rf_params(self, params: OpenLstParams):
        pass

if __name__ == "__main__":
    import click
    import IPython

    @click.command()
    @click.option('--port', default=None, help="Serial port")
    @click.option('--id', default=None, help="Serial interface ID")
    @click.option('--rtscts', is_flag=True, default=False, help="Use RTS/CTS flow control")
    @click.argument('hwid')
    def main(hwid, port, id, rtscts):
        logging.basicConfig(level="INFO")

        openlst = OpenLst(port, id, hwid, rtscts=rtscts)

        with openlst:
            IPython.embed()

    main()
