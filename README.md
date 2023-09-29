# UMD OpenLST Software

This is a modified version of the software for [Planet Labs' OpenLST](https://github.com/OpenLST/openlst).

Changes include:
* Restructured build process to avoid using Vagrant and simplify makefiles
* TODO: Rewrite python interface to be simpler
* TODO: Add commands to change modulation, data rate, output power
* TODO: Code for SDR to talk to OpenLST

## Setup

Install required packages

```bash
sudo apt install build-essential pkg-config libusb-1.0-0-dev libboost-all-dev sdcc cc-tool
```

Set up Python environment (must have Python 3.6+ installed)

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

Build bootloader and application

```bash
cd openlst
make all
```

Flash bootloader and application

```bash
TODO
```

## File Structure

* `ground` - Python interface for controlling OpenLST over UART
* `openlst` - C firmware for CC1110
    * `board` - Board specific files
    * `bootloader` - Bootloader code
    * `build` - Directory created during build process to hold all build artifacts
    * `common` - Files used by both bootloader and application
    * `radio` - Main application

## RF Parameters

### Data Rate

Page 191, Section 13.5

### Receiver Channel Filter Bandwidth

Page 191, Section 13.6

### Modulation Formats

Page 196, Section 13.9

DEVIATION_M, DEVIATION_E

### Frequency

Page 205, Section 13.3

* Base frequency set using FREQ2, FREQ1, FREQ0
* 8 bit channel selector and CHANSPC settings are used to choose specific channel relative to base frequency
* Must be changed while radio is in idle state

### Output Power

Page 207, Section 13.15

* Table with list of settings for given frequencies and power settings
* TODO: Test these values and other values and see what happens

## UART Protocol

Multi-byte fields are all least significant byte first

| Field       | Length | Purpose                                         |
| ----------- | ------ | ----------------------------------------------- |
| Start bytes | 2      | Denotes start of frame using bytes [0x22, 0x69] |
| Length | 1 | Number of bytes in packet, not including start bytes and length byte. Must be greater than 0 and less than 251. |
| HWID | 2 | ID of destination node |
| Sequence number | 2 | Increments by one after each command |
| System | 1 | Must be the same as `MSG_TYPE_RADIO_IN` |
| Command | 1 | Determine how the rest of the packet is interpreted |
| Data | N | Data bytes |

### Default Commands

#### 0x10 - ACK

#### 0xFF - NACK

#### 0x11 - ASCII

#### 0x12 - Reboot

Reboot OpenLST. Data field is either empty for immediate reboot or `uint32_t` representing number of seconds until reboot.

#### 0x13 - Get Time

Replies with 0x14 Set Time containing current internal time, or NACK if no time is set.

#### 0x14 - Set Time

Sets internal time. First 4 bytes are seconds, second 4 bytes are nanoseconds. Replies with ACK.

#### 0x15 - Ranging

TBD

#### 0x16 - Ranging ACK

TBD

#### 0x17 - Get Telem

Requests OpenLST telemetry. Reply is 0x18 Telem.

#### 0x18 - Telem

Struct containing:
* `uint8_t reserved` - Always 0
* `uint32_t uptime` - Uptime in ms
* `uint32_t uart0_rx_count` - Number of packets received on UART0
* `uint32_t uart1_rx_count` - Number of packets received on UART1
* `uint8_t rx_mode` - RX radio mode
* `uint8_t tx_mode` - TX radio mode
* `int16_t adc[ADC_NUM_CHANNELS]` - 8 ADC channels, temperature sensor, VDD/3
* `int8_t last_rssi` - RSSI of last packet. See page 198 of CC1110 datasheet to convert to dBm.
* `uint8_t last_lqi` - Link quality indicator of last packet. Relative indicator of how easily the signal was demodulated. High value means better link. Only comparable if RF settings are identical.
* `int8_t last_freqest` - Estimated frequency offset of carrier.
* `uint32_t packets_sent` - Number of packets sent.
* `uint32_t cs_count` - Carrier sense?
* `uint32_t packets_good` - Number of successfully received packets.
* `uint32_t packets_rejected_checksum` - Number of packets rejected due to CRC failure.
* `uint32_t packets_rejected_reserved` - Always 0.
* `uint32_t packets_rejected_other` - Number of packets rejected due to missing data in header.
* `uint32_t reserved0`
* `uint32_t reserved1`
* `uint32_t custom0`
* `uint32_t custom1`

#### 0x19 - Get Callsign

#### 0x1A - Set Callsign

#### 0x1B - Callsign

### Custom Commands

Planned commands to be added.

#### 0x80 - Set RF Parameters

Struct containing:
* `uint8_t FSCTRL1`
* `uint8_t FSCTRL0`
* `uint32_t FREQ`
* `uint8_t CHAN_BW_E`
* `uint8_t CHAN_BW_M`
* `uint8_t DRATE_E`
* `uint8_t DRATE_M`
* `uint8_t DEVIATN_E`
* `uint8_t DEVIATN_M`
* `uint8_t PA_CONFIG`
