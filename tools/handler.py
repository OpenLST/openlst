import logging
import serial
import serial.threaded
import queue

from commands import OpenLstCmds

class LstProtocol(serial.threaded.Protocol):
    START = b'\x22\x69'

    def __init__(self):
        self.packet = bytearray()
        self.in_packet = False
        self.transport = None

        self.byte_idx = 0
        self.packet_len = 0

        self.packet_queue = queue.Queue()
    
    def connection_mode(self, transport):
        self.transport = transport
    
    def connection_lost(self, exc):
        self.transport = None
        self.in_packet = False
        del self.packet[:]
        self.byte_idx = 0
        super(LstProtocol, self).connection_lost(exc)

    def data_received(self, data):
        # for byte in data:
        for byte in serial.iterbytes(data):
            # Handle start bytes
            if int(byte[0]) == self.START[0] and self.byte_idx == 0:
                self.byte_idx += 1

            elif int(byte[0]) == self.START[1] and self.byte_idx == 1:
                self.byte_idx += 1
                self.in_packet = True
                self.packet_len = 0

            elif self.in_packet:
                # Add all bytes to packet except start bytes
                self.packet.extend(byte)

                # Store length byte
                if self.byte_idx == 2:
                    self.packet_len = int(byte[0])

                # Finish packet
                if self.byte_idx > 2 and self.byte_idx-2 == self.packet_len:
                    self.in_packet = False
                    self.packet_len = 0
                    self.byte_idx = 0

                    self.handle_packet(bytes(self.packet))
                    del self.packet[:]
                else:
                    self.byte_idx += 1

            else:
                self.handle_out_of_packet_data(byte)

    def handle_packet(self, packet_raw: bytes):
        packet = {}

        packet['len'] = packet_raw[0]
        packet['hwid'] = int.from_bytes(packet_raw[1:3], 'big')
        packet['seq'] = int.from_bytes(packet_raw[3:5], 'big')
        packet['system'] = packet_raw[5]
        packet['command'] = packet_raw[6]
        packet['data'] = packet_raw[7:]

        self.packet_queue.put_nowait(packet)

        # Print boot messages
        msg = packet['data'].decode()

        print(f"Packet type {packet['command']}")
        # if packet['command'] == OpenLstCmds.ASCII and "OpenLST" in msg:
            # print(f"Boot message: {msg}") # TODO: figure out how to use logging without breaking ipython

    def handle_out_of_packet_data(self, data):
        print(f"Unexpected bytes: {data}")

if __name__ == "__main__":
    import click
    import IPython

    @click.command()
    @click.option('--port', default=None, help="Serial port")
    @click.option('--rtscts', is_flag=True, default=False, help="Use RTS/CTS flow control")
    def main(port, rtscts):
        logging.basicConfig(level="INFO")

        if port:
            ser = serial.Serial(port, baudrate=115200, rtscts=rtscts)
        else:
            ser = serial.serial_for_url('loop://', baudrate=115200, timeout=1)
        
        thread = serial.threaded.ReaderThread(ser, LstProtocol)
        with thread as protocol:
            IPython.embed()

    main()
