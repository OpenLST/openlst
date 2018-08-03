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

from binascii import hexlify


def parse_hex_file(hex_data, mem_size=0x8000):
    """Reads an Intel .hex file

    See https://en.wikipedia.org/wiki/Intel_HEX or
    http://www.piclist.com/techref/fileext/hex/intel.htm

    This is a partial implementation good enough for the CC1110 + SDCC

    It returns a 32KB bytearray with empty/unused bytes set to 0xFF
    (flash empty)
    """
    outbuff = bytearray(0xff for _ in xrange(mem_size))
    line_count = 0
    total_data = 0
    for line in hex_data.splitlines():
        line_count += 1
        if not line.startswith(':'):
            raise ValueError(
                ("Can't read payload file - "
                 "line %d doesn't start with ':'") % line_count)
        exp_checksum = 0
        byte_count = int(line[1:3], 16)
        exp_checksum += byte_count
        addrb1 = int(line[3:5], 16)
        addrb2 = int(line[5:7], 16)
        address = addrb1 << 8 | addrb2
        exp_checksum += addrb1 + addrb2
        record_type = int(line[7:9], 16)
        exp_checksum += record_type
        if record_type == 0:  # Data
            data = [
                int(line[(9 + 2 * i):(9 + 2 * i + 2)], 16)
                for i in xrange(byte_count)]
            checksum = int(
                line[(9 + 2 * byte_count):(9 + 2 * byte_count + 2)],
                16)
            if len(line.rstrip()) > 9 + 2 * byte_count + 2:
                raise ValueError("extra bytes on line %d" % line_count)
            exp_checksum += sum(data)
            if exp_checksum & 0xff != (~checksum + 1) & 0xff:
                raise ValueError("Bad checksum on line %d" % line_count)
            outbuff[address:address + len(data)] = data
            total_data += len(data)
        elif record_type == 1:  # End of File
            return outbuff
        else:
            raise ValueError(
                "Unknown record type '%d' on line %d" %
                (record_type, line_count))
    raise ValueError("Expected end of file")


def dump_hex_file(data, line_size=32, skip_lines=True,
                  truncate_lines=False):
    lines = []
    for addr in xrange(0, len(data), line_size):
        line_data = data[addr:addr + line_size]
        if truncate_lines:
            line_data = line_data.rstrip('\xff')
        if skip_lines and all(c == 255 for c in line_data):
            continue
        length = len(line_data)
        checksum = length
        checksum += addr >> 8
        checksum += addr & 0xff
        checksum += 0  # Data row
        for b in line_data:
            checksum += b
        checksum = ((checksum ^ 0xff) + 1) & 0xff  # Two's complement
        lines.append(
            ":{length:02X}{address:04X}{cmd:02X}{data}{checksum:02X}".format(
                length=length,
                address=addr,
                cmd=0,  # Data row
                data=hexlify(line_data).upper(),
                checksum=checksum))
    # EOF marker
    lines.append(":00000001FF")

    return "\n".join(lines)
