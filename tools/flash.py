#!/usr/bin/env python

import binascii
import click
import struct
import subprocess

from typing import List

import intel_hex


class FlashConstants:
    FLASH_BOOTLOADER_START = 0x0000
    FLASH_BOOTLOADER_STORAGE = 0x03CC
    FLASH_SIGNATURE_KEYS = 0x03CC
    FLASH_RESERVED = 0x03FC
    FLASH_HWID = 0x03FE
    FLASH_SIGNATURE_START = 0x6BF0
    FLASH_SIGNATURE_LEN = 16
    FLASH_APP_START = 0x0400
    FLASH_APP_END = 0x6BFF
    FLASH_PAGE_SIZE = 128
    FLASH_APP_SIGNATURE = 0x6BF0
    FLASH_UPDATER_START = 0x7000
    FLASH_STORAGE_START = 0x6C00
    FLASH_STORAGE_END = 0x6FFF


def process(image: bytes, hwid: int, keys: List[bytes]) -> bytes:
    FC = FlashConstants
    # Insert HWID
    hwid_bytes = struct.pack("<H", hwid)
    assert len(hwid_bytes) < 3, "Invalid HWID"

    if (
        any(b != 0 for b in image[FC.FLASH_HWID : FC.FLASH_APP_START])
        and image[FC.FLASH_HWID : FC.FLASH_APP_START] != hwid_bytes
    ):
        raise ValueError("hardware ID area is not empty")
    image[FC.FLASH_HWID : FC.FLASH_APP_START] = hwid_bytes

    # Insert keys
    for key_index, key in enumerate(keys):
        key_start = FC.FLASH_SIGNATURE_KEYS + key_index * 16
        key_end = key_start + 16
        if key_end > FC.FLASH_RESERVED:
            raise ValueError("too many keys (would overwrite reserved space)")
        if (
            any(b != 0 for b in image[key_start:key_end])
            and image[key_start:key_end] != key
        ):
            raise ValueError("key area is not empty for key %d" % key_index)
        image[key_start:key_end] = key

    # Insert application
    app = bytearray([0xFF] * (FC.FLASH_APP_SIGNATURE - FC.FLASH_APP_START))
    if (
        any(b != 0 for b in image[FC.FLASH_APP_START : FC.FLASH_APP_SIGNATURE])
        and image[FC.FLASH_APP_START : FC.FLASH_APP_SIGNATURE] != app
    ):
        raise ValueError("application area is not empty")
    image[FC.FLASH_APP_START : FC.FLASH_APP_SIGNATURE] = app

    # Insert signature
    sig = bytearray([0xFF] * (FC.FLASH_STORAGE_START - FC.FLASH_APP_SIGNATURE))
    if (
        any(b != 0 for b in image[FC.FLASH_APP_SIGNATURE : FC.FLASH_STORAGE_START])
        and image[FC.FLASH_APP_SIGNATURE : FC.FLASH_STORAGE_START] != sig
    ):
        raise ValueError("signature area is not empty")
    image[FC.FLASH_APP_SIGNATURE : FC.FLASH_STORAGE_START] = sig

    # Insert storage
    storage = bytearray([0xFF] * (FC.FLASH_UPDATER_START - FC.FLASH_STORAGE_START))
    if (
        any(b != 0 for b in image[FC.FLASH_STORAGE_START : FC.FLASH_UPDATER_START])
        and image[FC.FLASH_STORAGE_START : FC.FLASH_UPDATER_START] != storage
    ):
        raise ValueError("storage area is not empty")
    image[FC.FLASH_STORAGE_START : FC.FLASH_UPDATER_START] = storage

    return image


@click.command()
@click.argument(
    "build_dir", type=click.Path(exists=True, file_okay=False, writable=True)
)
@click.argument("hwid")
@click.option(
    "--key_file",
    type=click.Path(exists=True, dir_okay=False),
    help="File containing comma separated keys. If not specified, keys will be all 1s.",
)
@click.option("-n", "dry_run", is_flag=True, default=False, help="Don't flash binary")
def main(build_dir, hwid, key_file, dry_run):
    hwid = binascii.unhexlify(hwid)

    with open(build_dir + "bootloader.hex", "r") as f:
        image = intel_hex.parse_hex_file(f.read())

    if key_file:
        with open(key_file, "r") as f:
            keys = f.read().strip().split(",")
            keys = [binascii.unhexlify(k.strip()) for k in keys]
            for k in keys:
                assert len(k) == 16, "Invalid key length"
    else:
        keys = [bytes([0xFF] * 16)] * 3

    image = process(image, hwid, keys)

    with open(build_dir + "bootloader_processed.hex", 'w') as f:
        f.write(intel_hex.dump_hex_file(image))

    LOCK_BITS = "{:02X}".format(
        0b000 << 5 |  # Write to 0 per datasheet (not used)
        0 << 4 |  # Lock the boot block (flash block 0)
        0b100 << 1 | # Lock the top 4KB of flash
        0 << 0  # Enable debug commands TODO: disable these?
    )

    cmd = [
        "cc-tool",
        "-f",  # Fast mode
        "-e",  # Erase
        "-v",  # Verify after write
        "-l", LOCK_BITS,  # set lock bits
        "-w", build_dir + "bootloader_processed.hex"  # Write processed file
    ]

    print(" ".join(cmd))

    if not dry_run:
        subprocess.check_call(cmd)


if __name__ == "__main__":
    main()
