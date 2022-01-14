import serial, random, struct, time

seq_num = random.randint(0, 65535)
hwid = 2

packet = bytearray(struct.pack('<HH', hwid, seq_num))
# packet.extend([0x11] + [12]*200)
packet.extend([0x01, 0x11])
packet.extend(bytearray(b'test'))

packet_len = len(packet)

packet = bytearray([0x22, 0x69, packet_len]) + packet

ser = serial.Serial('/dev/lst_uart1', 115200)

while True:
    ser.write(packet)
    time.sleep(0.5)

packet_hex = [hex(b) for b in packet]

# print(packet.hex())
# print(packet_hex)
