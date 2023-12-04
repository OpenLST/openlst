#!/usr/bin/env python

from cryptography.hazmat.primitives.ciphers.aead import AESCCM
from Crypto.Cipher import AES
from binascii import unhexlify

import click
import logging
import struct
import time

from flash import FlashConstants
from openlst import OpenLst

import intel_hex


def sign_image(image: bytes, key: bytes):
    iv = b"\0" * 16

    cipher = AES.new(key, AES.MODE_CBC, iv)
    cbc = cipher.encrypt(image)

    return cbc[-16:]


def load_image(openlst: OpenLst, key: bytes, image: bytes):
    FC = FlashConstants

    # Sign image
    signature = sign_image(image[FC.FLASH_APP_START : FC.FLASH_SIGNATURE_START], key)
    assert len(signature) == FC.FLASH_SIGNATURE_LEN

    image = (
        image[: FC.FLASH_SIGNATURE_START]
        + signature
        + image[FC.FLASH_SIGNATURE_START + len(signature) :]
    )

    logging.info(f"Signed image with signature {signature}")

    # Get into bootloader
    while True:
        logging.info("Attempting to enter bootloader...")
        openlst.reboot()
        time.sleep(0.3)
        resp = openlst.flash_erase()

        if resp is not None:
            logging.info("Entered bootloader")
            logging.info("Erased flash")
            break

    logging.info("Flashing application image")

    # Flash each page
    for page in range(
        FC.FLASH_APP_START,
        FC.FLASH_APP_END + 1,
        FC.FLASH_PAGE_SIZE,
    ):
        page_num = page // FC.FLASH_PAGE_SIZE
        page_data = image[page : page + FC.FLASH_PAGE_SIZE]

        # Skip empty pages
        if all(b == 0xFF for b in page_data):
            continue

        logging.info(f"Writing page {page_num} with {len(page_data)} bytes")
        openlst.flash_program_page(page_data, page_num)

    openlst.flash_program_page(bytes(), 255, False)

    # Delay so we can see boot message
    time.sleep(2)


@click.command()
@click.option("--port", default=None, help="Serial port")
@click.option("--key", default="FF" * 16, help="Image signing key, default is all 1s")
@click.argument("hwid")
@click.argument("image", type=click.Path(exists=True, dir_okay=False))
def main(port, key, hwid, image):
    logging.basicConfig(level="INFO")

    hwid = unhexlify(hwid)
    hwid = struct.unpack(">H", hwid)[0]
    key = unhexlify(key)

    openlst = OpenLst(port, hwid=hwid)

    # Get image
    with open(image, "r") as f:
        image_bytes = intel_hex.parse_hex_file(f.read())

    logging.info("Loaded image")

    with openlst:
        load_image(openlst, key, image_bytes)


if __name__ == "__main__":
    main()
