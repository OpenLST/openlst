#!/usr/bin/env python

from cryptography.hazmat.primitives.ciphers.aead import AESCCM
from binascii import unhexlify

import click
import time
import logging

from flash import FlashConstants
from openlst import OpenLst

import intel_hex

def sign_image(image: bytes, key: bytes):
    aes = AESCCM(key)

    # Not very secure but this is how the firmware does it
    nonce = '\0' * 16

    cbc = aes.encrypt(nonce, image)

    return cbc[-16:]

@click.command()
@click.option('--port', default=None, help="Serial port")
@click.option('--key', default="FF"*16, help="Image signing key, default is all 1s")
@click.argument('hwid')
@click.argument('image', type=click.Path(exists=True, dir_okay=False))
def main(port, key, hwid, image):
    logging.basicConfig(level="INFO")

    hwid = unhexlify(hwid)
    key = unhexlify(key)

    openlst = OpenLst(port, hwid=hwid)

    # Get image
    with open(image, 'r') as f:
        app_section = intel_hex.parse_hex_file(f.read())
    
    logging.info("Loaded image")

    # Sign image
    signature = sign_image(app_section, key)
    assert len(signature) == FlashConstants.FLASH_SIGNATURE_LEN

    app_section = (
        app_section[:FlashConstants.FLASH_SIGNATURE_START] + 
        signature + 
        app_section[FlashConstants.FLASH_SIGNATURE_START + len(signature):]
    )

    logging.info("Signed image")

    with openlst:
        # Get into bootloader
        while True:
            logging.info("Attempting to enter bootloader...")
            openlst.reboot()
            time.sleep(0.2)
            resp = openlst.flash_erase()

            if resp is not None:
                logging.info("Entered bootloader")
                break

        logging.info("Flashing application image")

        # Flash each page
        for page in range(FlashConstants.FLASH_APP_START, FlashConstants.FLASH_APP_END+1, FlashConstants.FLASH_PAGE_SIZE):
            page_num = page // FlashConstants.FLASH_PAGE_SIZE
            page_data = app_section[page:page + FlashConstants.FLASH_PAGE_SIZE]

            # Skip empty pages
            if all(b == 0xFF for b in page_data):
                continue
            
            openlst.flash_program_page(page_data, page_num)

if __name__ == "__main__":
    main()
