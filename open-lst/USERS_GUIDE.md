# OpenLST User's Guide

### Table of Contents

1. [OpenLST User's Guide](#openlst-users-guide)
   * [Table of Contents](#table-of-contents)
1. [Introduction](#introduction)
   * [Project Goals](#project-goals)
   * [Project History](#project-history)
   * [Contents and Features](#contents-and-features)
   * [Non-Features](#non-features)
   * [Customizability](#customizability)
1. [Getting Started](#getting-started)
   * [What You'll Need](#what-youll-need)
   * [System Requirements and Setup](#system-requirements-and-setup)
   * [Using Vagrant/VirtualBox](#using-vagrantvirtualbox)
   * [Other Setup Options](#other-setup-options)
1. [Connecting to the Demo Board](#connecting-to-the-demo-board)
   * [Cleaning the Build Environment (Optional)](#cleaning-the-build-environment-optional)
1. [Building and Loading the Bootloader](#building-and-loading-the-bootloader)
1. [Building and Loading the Application](#building-and-loading-the-application)
1. [Basic Local Commands](#basic-local-commands)
   * [Packet Structure](#packet-structure)
   * [Commands](#commands)
1. [Creating a Custom Firmware Project](#creating-a-custom-firmware-project)
   * [Repository Setup and Layout](#repository-setup-and-layout)
   * [Common Customizations](#common-customizations)
   * [Bootloader Features](#bootloader-features)
   * [Application Features](#application-features)
   * [Adding a Custom Command](#adding-a-custom-command)
1. [Creating a Custom Hardware Project](#creating-a-custom-hardware-project)
   * [Frequency Coverage](#frequency-coverage)
   * [Design Files](#design-files)
   * [Fabrication and Prototyping](#fabrication-and-prototyping)
   * [Specifications](#specifications)
1. [Frequency Selection](#frequency-selection)
   * [Amateur Radio Frequencies](#amateur-radio-frequencies)
   * [Commercial Frequencies](#commercial-frequencies)
   * [Hardware changes](#hardware-changes)
1. [Troubleshooting](#troubleshooting)
   * [General Linux Resources](#general-linux-resources)
   * [ZMQ](#zmq)
   * [Systemd / Upstart / initd](#systemd-upstart-initd)
   * [Udev Rules](#udev-rules)
1. [Acknowledgements](#acknowledgements)
1. [License Info](#license-info)
1. [Disclaimers](#disclaimers)

## Introduction

### Project Goals

OpenLST is an open-source, proven radio design for communicating with remote
instruments, vehicles, and stations using low-cost commercial components.

### Project History

OpenLST uses components that build on [Planet's](http://planet.com/) experience
with the Dove LST (Low-Speed Transceiver) radio. Planet's Dove satellite radio,
which uses similar components and software, has an extensive history of
on-orbit success and reliability with over 150 satellites contributing to over
200 cumulative years of on-orbit data.

The firmware included in this release supports basic operation on the amateur
70cm band for use between OpenLST radios or similarly programed packet radios.

### Contents and Features

OpenLST is radio system that enables packet-based communication with a remote
terminal using low-cost commercial parts. "LST" stands for Low-Speed
Transceiver. The reference design included in this release supports
packet-based communication 437MHz at 3kbps and includes forward error
correction (FEC), CRC-based integrity checks, basic radio telemetry, as well as
support for in-situ or over-the-air firmware updates. This reference design
also supports time-of-light ranging for coarse location and trajectory
estimation.

The OpenLST project consists of:

1. A firmware (embedded C) repository with the source for the bootloader and
   application sections of the radio
2. A small set of Python tools for programming and communicating with the
   OpenLST radio
3. A reference hardware design including schematics, part specs, layout, and
   Gerber files using [KiCad](http://kicad-pcb.org/)
4. This user's guide

### Non-Features

OpenLST may not be suitable on its own for all environments or use cases. The
following features are not included:

- Message encryption or authentication; users who need this will need to
  implement it at the application or protocol layer.
- Beaconing or unrequested transmissions; users that require a beacon feature
  will need to implement that separately.
- Full duplex communication; while OpenLST can support separate channels for
  uplink and downlink, the design does not support receiving while
  transmitting.

### Customizability

OpenLST is specifically designed to make certain common customizations easy:

1. Changing the uplink/downlink frequencies
2. Altering the bitrate or modulation settings
3. Adding additional custom command opcodes
4. Channels for custom analog-digital conversion setups (custom sensors and
   telemetry)

See [Creating a Custom Firmware Project](#creating-a-custom-firmware-project)
for instructions on making these common customizations.

#### A note about Bash code blocks

In common parlance, bash lines will start with either a `#` or `$` indicating
privilege level. The octothorpe (`#`) indicates a line should be executed with
administrator privileges (for example, with `sudo`), while the dollar sign
(`$`) indicates a line should be executed with standard user privileges.
Octothorpes in the line indicate a comment and should not be executed in the
shell.

Lines that terminate in a backslash (`\`) indicate that the next line is a
continuation of the same command. Commands are sometimes split across lines as
an attempt to make them more readable.

## Getting Started

This section describes the steps necessary to set up a development environment
using an evaluation sample board.

### What You'll Need

To get started and use the sample project, you will need:

1. A license to operate in the 70cm band, depending on your jurisdiction
2. An OpenLST evaluation board
3. A DC power supply for the evaluation board
4. The Texas Instruments CC Debugger tool
5. A 3.3V FTDI serial/TTL adapter
5. A PC running Windows or Linux or a Mac running macOS (any OS that supports
   VirtualBox on a x64 host)
6. An installed copy of git to clone the repository

These items are described in the sections below.

#### Licensing Requirements for Sample Hardware

Depending on your legal jurisdiction, developers and users may be required to
obtain a license for operating radio hardware in the 70cm band or any frequency
bands utilized by a custom radio project. The default sample project operates
at 437MHz which is in the amateur 70cm band in the United States and may
require an amateur radio license to operate over the air. Licensing is outside
the scope of this guide - it is up to the user to make sure they comply with
all applicable regulations.

#### Evaluation Board

The KiCad project, Gerber files, and bill of materials for the OpenLST evaluation
board are available as a git repostory:

```bash
$ git clone --recursive https://www.planet.com/open/openlst-hw.git
```

#### Programming Hardware and Recommended Tools

The [Texas Instruments CC Debugger](http://www.ti.com/tool/CC-DEBUGGER) tool is
used to load bootloader flash images onto the CC1110 microcontroller. This tool
is not required to load new application flash binaries, however it is required
for the initial bootloader load and for bootloader development.

The FTDI USB to serial cable is also recommended. The
[TTL 3.3V version](http://www.ftdichip.com/Products/Cables/USBTTLSerial.htm) is
used for this design.

#### Power Supply

Additionally, the OpenLST development board requires a 5V 1.5A DC power supply.
An oscilloscope and spectrum analyzer are also recommended for system
development.

#### Cloning and Building the Repository

The example firmware project is located on Github at
[https://www.planet.com/open/openlst.git](
https://www.planet.com/open/openlst.git)

To clone the example project, use git. For example:

```bash
$ git clone --recursive https://www.planet.com/open/openlst.git
```

### System Requirements and Setup

This guide assumes you are using the included Vagrantfile/virtual machine (VM)
for development and programming. It is also possible to install the toolchain
and programmer tools directly if you prefer.


### Using Vagrant/VirtualBox

There is a Vagrantfile included in the sample project that creates a VM that
includes:

1. cctool (TI CC Debugger support software for programming)
2. SDCC (the compiler toolchain)
3. The included python tools for programming, signing, and testing the
   application
4. USB passthrough support for the TI CC Debugger and evaluation board serial
   ports
5. A shared folder with the host machine so you can use development tools on
   your host while running the toolchain and programmer tools in the VM

#### To set up the VM:

1. Install [VirtualBox and the Guest Extensions](
   https://www.virtualbox.org/wiki/Downloads). Guest extensions are required to
   share the USB device with the host.
   * For macOS, follow the instructions on the link above
   * For Ubuntu Linux, this [linked guide may be useful](
    https://askubuntu.com/questions/41478/how-do-i-install-the-virtualbox-version-from-oracle-to-install-an-extension-pack/)
2. Install [Vagrant](https://www.vagrantup.com/). Version 2.0 or later is
   required.
3. If you have not cloned the example project yet, clone the repository (see
   above). There is a Vagrantfile in the example project used to set up the VM.
   You will need at least 2GB of RAM available to provision the VM.
4. Run:
   ```bash
   $ cd openlst
   $ vagrant up
   $ vagrant reload
   ```
   This will create and provision an Ubuntu 16.04 VM with cctool, the SDCC
   toolchain, and the python utilities installed. The "reload" step is required
   to reboot the VM and allow the vagrant user access to the USB devices.
5. Log into the VM using:
   ```
   $ vagrant ssh
   ```
6. Plugging in the CC Debugger tool and running "cc-tool" should produce the
   output:
   ```
   vagrant@vagrant-ubuntu-trusty-64:~$ cc-tool
     Programmer: CC Debugger
     Target: CC1110
     No actions specified
   ```
   If you see an error like:
   ```
   Error occured: libusb_open failed, Access denied (insufficient permissions)
   ```
   You may need to reboot the VM to make sure the udev rules are applied.

### Other Setup Options

This section is mainly targeting Linux users that want to run the toolchain on
their own system. If you are using the Vagrant VM you can skip these steps.

#### cctool

In order to operate the CC Debugger programming device, you will need to
install the cctool software. The source is available from
[sourceforge](https://sourceforge.net/projects/cctool/) and from GitHub mirrors
like [https://github.com/dashesy/cc-tool](https://github.com/dashesy/cc-tool)
for Linux systems (used in this guide). Building on macOS may be possible using
macports.

For Windows systems, Texas Instruments provides software for using the
[debugger tool](http://www.ti.com/tool/flash-programmer). Use of this tool is
not covered in this guide.

#### SDCC

OpenLST uses the [Small Device C Compiler](http://sdcc.sourceforge.net/) to
generate bootloader and application flash images. SDCC binary images are
available directly [via sourceforge](
https://sourceforge.net/projects/sdcc/files/). Version 3.5 is recommended.
Additionally some distributions (like Debian/Ubuntu) provide sdcc binaries
through their package manager, installable using:

```bash
# apt-get install sdcc
```

On Debian Jessie or later and Ubuntu 14.04 or later. Again, SDCC version 3.5 or
later is recommended. Early versions are not well tested with this codebase.

Alternatively, you can use a docker image from the Dockerfile provided in the
build directory of the OpenLST repository:

```bash
$ cd build
$ docker build -t sdcc .
```

Then you can build using:

```bash
$ docker run --rm -v $PWD:/project sdcc bash -c "cd project && make"
```

## Connecting to the Demo Board

### Cleaning the Build Environment (Optional)

If you have previously built the firmware on the same computer, and
subsequently modified the firmware, clean your build environment by running:

```bash
vagrant@ubuntu-xenial:~/project$ make clean
```

## Building and Loading the Bootloader

Note: this section assumes you logged in to the Vagrant VM (via vagrant ssh)

Loading the bootloader requires the CC Debugger tool. This must be plugged
in to the programming port on the Open LST board.

Run make to build the bootloader binary image for the OpenLST 437Mhz sample
project:

```bash
vagrant@ubuntu-xenial:~$ cd project/
vagrant@ubuntu-xenial:~/project$ make openlst_437_bootloader

Memory usage summary for openlst_437_bootloader.hex:
   Name             Start    End      Size     Max
   ---------------- -------- -------- -------- --------
   PAGED EXT. RAM   0xf000   0xf036      55      256
   EXTERNAL RAM     0x0000   0xf96d    2363     3328
   ROM/EPROM/FLASH  0x0000   0x7c99   31823    32768
Stack starts at: 0x2e (sp set to 0x2d) with 210 bytes available.
```

The SDCC output shows a summary of the bootloader binary (Intel `.hex`  file).
In the example above, the bootloader's code segment starts at address 0x0000
(the bottom of flash) and extends to 0x7FFF (32768 bytes). The bootloader
actually only occupies the first page and 0x7000-0x7FFF and reserves the rest
of flash for the application.

To load the bootloader onto the development board, first make sure the CC
Debugger is connected to the development board's programming header and that
the board is powered off an external supply. You may need to press the reset
button on the CC Debugger after first connecting (if your CC Debugger has a
green/red LED, it should show green).

Run `cc-tool` to make sure you are connected and the board is working:

```bash
vagrant@ubuntu-xenial:~/project$ cc-tool
  Programmer: CC Debugger
  Target: CC1110
  No actions specified
```

Load the flash image using the `flash_bootloader` Python utility. This script
is included in the `openlst_tools` Python library in the repository. It is
installed as part of the vagrant up provisioning step if you are using the
Vagrant VM (if you're not using the Vagrant VM, you will need to install the
python package under `tools/openlst_tools` locally with
`pip install -e open-lst/tools`).

You will need to specify two parameters when loading the bootloader:

1. A hardware identifier (HWID): This is a two-byte identifier, typically
   expressed as a four-digit hex number like "01AB". Some important rules and
   guidelines apply:
   * Hardware IDs 8000 and above are reserved for ground/base station radios
   * Hardware IDs 7FFF and below are reserved for remote radios.
   * When deployed in the field, it is important that your HWIDs do not overlap
     with other users of the same frequency band and modulation settings. If
     you share allocation and are worried about overlap in HWIDs, it is
     recommended that you coordinate with your issuing authority and contact
     Planet.
   * HWID 0000 is reserved for broadcast messages and should not be used
   * HWID FFFF is reserved for local messages sent over the serial port to the
     ground/base station radio and should not be used
   * For testing purposes HWIDs 0001-00FF are recommended. For this tutorial you
     can just use 0001.
2. Signature keys for application payloads: The bootloader requires three
   AES-128 (16 byte) keys. These are used to verify the authenticity of
   application images. Applications must be cryptographically signed with one of
   these three keys to run. Keys can either be specified at the command line (as
   32 character hex strings) or from a key file (as three comma-separated 32
   character hex strings).
   * In production, all three keys should be set to non-trivial values (not all
     zeros or all ones).
   * It is okay to set two or three of the keys to the same value if you don't
     need three distinct keys. In production it is recommended that you have at
     least two unique keys in order to have a backup in case the primary key is
     lost.
   * For lab testing you can set all three keys to all ones (a value of
     `FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF`)

```bash
ubuntu@ubuntu-xenial:~/project$ flash_bootloader --keys \
FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF \
FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF --hwid 0001 openlst_437_bootloader.hex
```

This will erase the flash on the CC1110 and load the bootloader binary,
encryption keys, and HWID (0001) into flash. This will also set CC1110
lock bits to prevent accidental erasure of the bootloader.

If the bootload fails with a stacktrace, make sure the CC Debugger is plugged
in, press the "Reset" button, and retry.

Once the bootloader is loaded it should be running on the device. The
bootloader will reset itself roughly every second from the CC1110's internal
watchdog timer. After a reset it prints its version over UART1. You can observe
this by connecting to UART1 and using the `radio_terminal` command:

```bash
vagrant@ubuntu-xenial:~$ radio_terminal
< lst ascii "OpenLST BL 1874e4b"
< lst ascii "OpenLST BL 1874e4b"
< lst ascii "OpenLST BL 1874e4b"
...
(press Ctrl+c to exit)
```

The message will include the git SHA of the current bootloader. In this
example the SHA is `1874e4b`, but it may be different on your terminal
depending on your version of the repo.

## Building and Loading the Application

The application is loaded over the serial connection rather than
through the CC Debugger. To program the application, the FTDI USB-serial
cable must be connected to the UART1 on the board.

To build the application, run:
```bash
vagrant@ubuntu-xenial:~/project$ make openlst_437_radio

Memory usage summary for openlst_437_radio.hex:
   Name             Start    End      Size     Max
   ---------------- -------- -------- -------- --------
   PAGED EXT. RAM   0xf000   0xf066     103      256
   EXTERNAL RAM     0xf067   0xf9d3    2413     3328
   ROM/EPROM/FLASH  0x0400   0x1abd    5790    26592
Stack starts at: 0x48 (sp set to 0x47) with 184 bytes available.
```

Again, this prints out some important statistics for the radio application
image. In this case the code area starts at 0x0400 (after the first page of the
bootloader). It uses 5790 bytes of roughly 26KB available in the flash.

In order to program the application, it first needs to be signed using one of
the keys loaded into the bootloader. Assuming you used all 1s (using the
section above) for the keys, you would run:

```bash
$ sign_radio --signing-key FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF \
openlst_437_radio.hex openlst_437_radio.sig
```

This writes the 16 byte signature to the `openlst_437_radio.sig`.

Once signed, the application can be loaded using:

```bash
$ bootload_radio --signature-file openlst_437_radio.sig \
-i 0001 openlst_437_radio.hex

INFO:root:Read firmware image
INFO:root:Inserted signature
DEBUG:openlst_tools.commands:Sending (0001): lst reboot
DEBUG:openlst_tools.commands:No response
DEBUG:openlst_tools.commands:Sending (0001): lst bootloader_erase
DEBUG:openlst_tools.commands:Response: lst bootloader_ack 1
DEBUG:openlst_tools.commands:Sending (0001): lst bootloader_write_page 8 ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
...
...
DEBUG:openlst_tools.commands:Response: lst bootloader_ack 215
DEBUG:openlst_tools.commands:Sending (0001): lst bootloader_write_page 255
DEBUG:openlst_tools.commands:Response: lst bootloader_nack
```

The firmware image should boot immediately. If it fails to load the firmware
and continues to loop through reboot / no response messages, then check your
connection. Remember, the bootloader is loaded over the CC Debugger, but the
radio firmware is flashed over the UART. If you are watching with an instance
of `radio_terminal`, the firmware will print its version:

```bash
< lst ascii "OpenLST 1874e4b"
```

As before, the firmware will print a git SHA as it's version number. That SHA
may be different than the example here based on your version of the repository
and firmware.

You can try querying telemetry:

```bash
$ get_telem -i 0001

DEBUG:openlst_tools.commands:Sending (0001): lst get_telem
DEBUG:openlst_tools.commands:Response: lst telem 0 179 0 0 0 0 2047 2047 2047 4092 2047 2047 2047 2047 1410 1839 -128 0 0 0 0 0 0 0 0 0 0 0 0
lst telem 0 179 0 0 0 0 2047 2047 2047 4092 2047 2047 2047 2047 1410 1839 -128 0 0 0 0 0 0 0 0 0 0 0 0
reserved 0
uptime 179
uart0_rx_count 0
uart1_rx_count 0
rx_mode 0
tx_mode 0
adc0 2047
adc1 2047
adc2 2047
adc3 4092
adc4 2047
adc5 2047
adc6 2047
adc7 2047
adc8 1410
adc9 1839
last_rssi -128
last_lqi 0
last_freqest 0
packets_sent 0
cs_count 0
packets_good 0
packets_rejected_checksum 0
packets_rejected_reserved 0
packets_rejected_other 0
reserved0 0
reserved1 0
custom0 0
custom1 0
```

## Basic Local Commands

The translator is a good resource to view possible commands available to the
user. While the radio may accept commands outside of the translator, the
translator is designed to interpret, validate, and parse commands and their
arguments into the hexadecimal bytearrays accepted by the radio.

The terminal program provided with the radio firmware will try to parse the
translator's command structure to build a list of suggested ways the user can
complete the command (use the `<TAB>` key to view it).

When using the radio terminal, commands begin with `lst` and are then followed
by the commands listed in the "Commands" section within this guide. An example
of a command used in the radio terminal is `lst get_telem` which will output
`telem` followed by the telemetry information of the radio.

Radio commands can be given in the command line of the Linux environment. To do
so, enter the following: `radio_terminal -c [COMMAND]`. Here, [COMMAND] should
be replaced by any command that can be entered into the radio terminal. Once
entered, `Waiting for response...` will be displayed to signify that the command
was valid and is waiting for a response. After the response is received,
`Exiting - Message Received` will output and the program will end. This is an
example of a `ACK` being succesfully sent and received:

```
$ radio_terminal -c lst ack
Waiting for response...
lst ack
Exiting - Message Received
```

If no response is received, `Exiting - Timeout` will display. If the command is
invalid, `[ERROR] Invalid Command:` will be displayed.

### Packet Structure

Packets include a few parts to help the radios direct and respond to commands
appropriately:

#### HWID

The hardware ID (HWID) is a 2-byte serial number assigned to the radio and used
to identify the unit to which commands are addressed. Usually this is expressed
as a four-digit hexadecimal value, for instance you may have assigned the radio
the HWID `0x0001` in the examples above. Each unit you use should have a unique
HWID.

#### SEQNUM

The sequence number (SEQNUM) is a 2-byte number used to match commands and
replies. Typically the sequence number will increment by one for each command
sent (and overflow without issue: `..., 0xFFFE, 0xFFFF, 0x0000, 0x0001, ...`).
The included `radio_terminal` program will select a random SEQNUM at startup,
and increment it after each command. The radio will automatically send any
response with the same seqnum as the originating command.

#### Destination

The destination is a 1-byte number used to specify what the radio should do
with a command. Most user commands will go to `lst` (the translator will map
the string to the proper byte code). The OpenLST will forward any commands
not targeted to the `lst` destination.

#### Payload
    
The payload of the bytearray contains up to N-bytes that are parsed by the
radio to activate different functions. The Payload may also include arguments
and optional arguments depending on which command is being given.

### Commands

#### `ACK`

Acknowledge (ACK) is a basic command or response used to "ping" the radio or to
acknowledge certain other commands. An ack command received by the radio will
trigger an ack reply.

#### `NACK`

The opposite of an ACK: this is a non/negative/no-acknowledge command. It is
used by the radio to signal that while it has received a command, it cannot
execute that command.

#### `REBOOT [DELAY]`

The reboot command tells the radio to reboot or schedules it for some `DELAY`
seconds in the future. DELAY is an optional argument: without it, the radio
will reboot immediately. By default the maximum delay duration is about one
week.

#### `GET_TELEM`

The radio collects some health telemetry that can be queried using `GET_TELEM`.

#### `TELEM`

The radio will reply to a `GET_TELEM` command with a `TELEM` reply. Fields
include:

```bash
Reserved
Uptime
Uart0_rx_count
Uart1_rx_count
Rx_mode
Tx_mode
Adc0
Acd1
...
Acd9
Last_rssi
Last_lqi
Last_freqest
Packets_sent
Cs_count
Packets_good
Packets_rejected_checksum
packets_rejected_reserved
Packets_rejected_other
Reserved0
Reserved1
Custom0
Custom1
```

#### `GET_TIME`

The radio has an onboard clock in the CC1110 chip. This clock is reset upon
reboot and will start counting once time is initially set. The command returns
seconds and nanoseconds. It will return a `NACK` if the time has not been set.

#### `SET_TIME SECONDS NANOSECONDS`

The radio's time can be set using `SET_TIME`, then queried using `GET_TIME`.
This command has two required arguments (one for seconds, one for nanoseconds).
The epoch used is seconds since J2000.

#### `SET_CALLSIGN CALLSIGN`

The radio can store a callsign in the persistent memory using the
`set_callsign` command. The callsign is meant to ease compliance with US CFR
Part 97 (or compliance with similar laws in other countries). It is up to the
user to ensure compliance with local laws.

#### `GET_CALLSIGN`

`get_callsign` will request the callsign from the persistent memory.

#### `CALLSIGN`

`callsign` is the command that the radio will use to reply to `get_callsign`
requests.

#### `ASCII STRING`

The radio is capable of sending basic ASCII text. It takes a string argument.

#### `BOOTLOADER_PING`

The bootloader accepts a basic ping with an `ack` reply. This ping will also
reset the watchdog timeout, and can be used to deploy a reboot of the
bootloader.

#### `BOOTLOADER_ERASE`

The `BOOTLOADER_ERASE` command is used to erase pages of the CC1110's flash
memory

#### `BOOTLOADER_WRITE_PAGE`

The `BOOTLOADER_WRITE_PAGE` command is used by the bootloader to write to the
CC1110's flash memory.

#### `BOOTLOADER_ACK`

The bootloader can send acks similar to the default radio. See also:
[`ACK`](#ack)

#### `BOOTLOADER_NACK`

The bootloader can send nacks similar to the default radio. See also:
[`NACK`](#nack)

## Creating a Custom Firmware Project

The example project is a good learning resource and a great way to get started.
However, any serious application is going to require frequency selection as
well as some custom commands and board settings. The recommended approach is to
fork the OpenLST repository and make your changes.

### Repository Setup and Layout

The sample project is laid out as follows:

`openlst` - git repository, root

1. `Vagrantfile` - file, template for a Vagrant/VirtualBox VM that includes the
   toolchain
2. `build` - directory, contains a Dockerfile useful for the SDCC toolchain
3. `open-lst` - submodule containing the open-lst core repository
4. `radios` - directory containing build target makefiles and custom code
   * `openlst_437` - directory containing custom makefiles and code for the
     sample amateur radio
   * `board.h` - configuration options for the `openlst_437` example project
   * `board.c` - custom initialization code for the sample board
   * `Board.mk` - a make include file used to describe the build targets
5. `README.md` - general repository information
6. `Makefile` - this makefile sources the Board.mk files from all the radio
   definitions

The easiest way to make your own project is to fork this repository by pushing
it to your own remote, like:

```bash
$ git clone https://www.planet.com/open/openlst.git
$ cd openlst
$ git remote rename origin openlst
$ git remote add origin MY_GIT_HOST
$ git push origin master
```

To add a new radio target (in this example called `my_radio`):

1. Create a directory under "radios" for the new target:
   ```bash
   $ mkdir radios/my_radio
   ```
2. Create a `board.c` file under `radios/my_radio`. To get started, it is
   recommended that you copy the `board.c` file from `openlst_437` as it
   contains PA control and pin setup logic for the sample board:
   ```bash
   cp radios/openlst_437/board.c radios/my_radio/board.c
   ```
3. Create a `board.h` file under `radios/my_radio`. Again, to get started, just
   copy the file from `openlst_437` as it supports the sample board:
   ```bash
   cp radios/openlst_437/board.h radios/my_radio/board.h
   ```
4. Create a `Board.mk` file under `radios/my_radio`. Its contents should be as
   follows:
   ```make
   RADIOS += my_radio
   BOOTLOADERS += my_radio
   my_radio_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

   # Source files
   my_radio_SRCS := \
       $(my_radio_DIR)/board.c

   my_radio_CFLAGS := -DCUSTOM_BOARD_INIT -I$(my_radio_DIR)

   # Disable UART0 in the bootloader to save space
   my_radio_BOOTLOADER_CFLAGS := -DUART0_ENABLED=0
   ```
   You can adjust these settings later (like modifying the UART enable
   setting), but these match well with the sample board.
5. Add an include directive to the top level (repository root) Makefile. This
   should go before any other include directives as the first line of the file:
   ```make
   include radios/my_radio/Board.mk
   ```
   This will add your new target to the make radios and make bootloaders
   targets as well as adding individual targets like `make my_radio_bootloader`
   and `make my_radio_radio`.

Now test your changes by running make from the project root. Both the radio and
bootloader images should compile successfully and produce `my_radio_radio.hex`
and `my_radio_bootloader.hex` files in the project root.

### Common Customizations

#### Crystal Oscillator

The reference design uses a 27MHz clock for the CC1110. Modify this value if
you are using a different oscillator. You must also update the flash write
(FWT) frequency per page 88 of the datasheet.

```cpp
#define F_CLK 27000000
#define FWT_FREQUENCY 36
```

#### Radio Message Type

The default message type for OpenLST is 0x05. This value is arbitrary and can
be modified if necessary:

```cpp
#define MSG_TYPE_RADIO_OUT  5
#define MSG_TYPE_RADIO_IN   5
```

Messages with other type codes are forwarded rather than processed.

#### Transceiver vs Rx or Tx Only

You can set one of these values to 0 to disable the transmit or receive
functionality:

```cpp
#define CONFIG_CAPABLE_RF_RX 1
#define CONFIG_CAPABLE_RF_TX 1
```

#### UARTs

OpenLST supports two independent UARTs. You can disable one or both of these by
setting the values below to 0:

```cpp
#define UART0_ENABLED 1
#define UART1_ENABLED 1
```

#### Baud Rates

By default both UARTs operate at 115200 baud. You can reconfigure the baud rate
here. Keep in mind this depends on the system clock rate. See page 155 of the
datasheet for details. Both the BAUD and GCR settings contribute to define the
baud rate.

```cpp
#define CONFIG_UART0_BAUD 24
#define CONFIG_UART1_BAUD 24
#define CONFIG_UART0_GCR 12
#define CONFIG_UART1_GCR 12
```

#### Flow Control

UART1 supports flow control. By default it is enabled here:

```cpp
#define CONFIG_UART1_USE_FLOW_CTRL 1
#define CONFIG_UART1_FLOW_PIN P0_3
```

#### General UART Configuration

Default UART configurations can be overridden here if you need to make hardware
customizations or pin changes:

```cpp
#define CONFIG_UART0_UCR ((1<<7) | (1<<1))
#define CONFIG_UART1_UCR ((1<<7) | (1<<1))
```

#### Default Radio Modes

The default transmit and receive modes can be overridden. Keep in mind any
custom modes will need to be defined in board.h (you'll need to override the
`lst_rf_mode_e enum`)

```cpp
#define RADIO_MODE_DEFAULT_RX amateur_rf_mode_437_7k_FEC
#define RADIO_MODE_DEFAULT_TX amateur_rf_mode_437_7k_FEC
```

Ranging response can also be overridden:

```cpp
#define RADIO_MODE_RANGING_RX amateur_rf_mode_437_10k_ranging
#define RADIO_MODE_RANGING_TX amateur_rf_mode_437_10k_ranging
```

#### Ranging Options

The number of timer cycles between a ranging request and a ranging response can
be modified here:

```cpp
#define RF_PRECISE_TIMING_DELAY 3
```

#### Reboots

Unless commanded otherwise, the radio defaults to rebooting every 10 minutes.
That value can be adjusted here:

```cpp
#define AUTO_REBOOT_SECONDS 600
```

The radio will reject requests to delay the reboot by more than
`AUTO_REBOOT_MAX`, which is normally 1 week:

```cpp
#define AUTO_REBOOT_MAX 604800
```

#### Message Forwarding

By default, the bootloader does not forward messages at all while the
application does. Message forwarding can be enabled in the bootloader by
setting these to 1 or disabled in the application by setting them to 0.

```cpp
#define FORWARD_MESSAGES_UART1 1
#define FORWARD_MESSAGES_UART0 1
#define FORWARD_MESSAGES_RF 1
```

#### Optimizations

By default all utility and library functions are included. Code size can be
reduced by disabling some of these functions:

```cpp
#define KEEP_CODE_SMALL 0
```

#### Analog to Digital Conversion

By default all ADC channels are disabled (input disabled). You can enabled some
or all of these channels using the bitmask. Keep in mind this overrides the
PDIR setting for the pin.

```cpp
#define ADCCFG_CONFIG 0b00000000
```

#### Watchdog Reset

The bootloader enables the CC1110's hardware watchdog timer. You can adjust
this timing if necessary from its default (about 1s). Be careful not to set it
too short (application never boots) or too high (outside the limits of the
timer).

```cpp
#define COMMAND_WATCHDOG_DELAY 45000
```

#### Simple RF Customization

If you just want to change the center frequency or some of the basic modulation
parameters, redefining these constants may be sufficient:

For frequency customization, see the CC1110 datasheet on how to set these
registers, or use SmartRF studio to derive them:

```cpp
#define RF_FSCTRL1 0x06
#define RF_FSCTRL0 0x00
#define RF_FREQ2 0x10
#define RF_FREQ1 0x2F
#define RF_FREQ0 0x69
```

Channel bandwidth, data rates, and sync words can also be overridden here:

```cpp
#define RF_SYNC_WORD1 0xD3
#define RF_SYNC_WORD0 0x91
#define RF_CHAN_BW_E 3
#define RF_CHAN_BW_M 3
#define RF_CHAN_BW_RANGING_E 3
#define RF_CHAN_BW_RANGING_M 3
#define RF_DRATE_E   8
#define RF_DRATE_M  32
#define RF_DRATE_RANGING_E   8
#define RF_DRATE_RANGING_M  132
#define RF_CHANSPC_E 3
#define RF_CHANSPC_M 185
#define RF_CHANSPC_RANGING_E 3
#define RF_CHANSPC_RANGING_M 185
#define RF_DEVIATN_E 1
#define RF_DEVIATN_M 1
#define RF_DEVIATN_RANGING_E 2
#define RF_DEVIATN_RANGING_M 4
#define RF_FSCAL3_CONFIG 201
#define RF_FSCAL2_CONFIG 10
#define RF_FSCAL1_CONFIG 0
#define RF_FSCAL0_CONFIG 31
#define RF_TEST0_CONFIG  9
```

The power setting can be overridden if you are using a different power
amplifier or no power amplifier:

```cpp
#define RF_PA_CONFIG     192
```

#### Advanced RF Customization
To define custom RF modes for the CC1110 set `BOARD_RF_SETTINGS` to 1 in
`board.h`:

```cpp
#define BOARD_RF_SETTINGS 1
```

Also in `board.h`, you will need to define an enum for each supported radio
mode you wish to create:

```cpp
typedef enum {
  my_radio_rf_mode_430MHz   = 0,
  my_radio_rf_mode_432MHz   = 1
} lst_rf_mode_e;
```

And decide which modes are the default for receive and transmit:

```cpp
#define RADIO_MODE_DEFAULT_RX my_radio_rf_mode_430MHz
#define RADIO_MODE_DEFAULT_TX my_radio_rf_mode_432MHz
```

These modes must then be defined in `board.c` by defining a
`board_apply_radio_settings` function:

```cpp
uint8_t board_apply_radio_settings(uint8_t mode) {
	switch (mode) {
		case my_radio_rf_mode_430MHz:
		...
		break;
		case my_radio_rf_mode_432MHz:
		...
		break;
		default:
			return RADIO_MODE_INVALID;
}
return RADIO_MODE_OK;
}
```

Inside each case statement, the following registers need to be set (described
in the CC1110 datasheet):

- `PKTLEN`
- `PKTCTRL1`
- `PKTCTRL0`
- `CHANNR`
- `SYNC0`
- `SYNC1`
- `MDMCFG4`
- `MDMCFG3`
- `MDMCFG2`
- `MDMCFG1`
- `MDMCFG0`
- `DEVIATN`
- `MCSM2`
- `MCSM1`
- `MCSM0`
- `FOCCFG`
- `BSCFG`
- `AGCCTRL2`
- `AGCCTRL1`
- `AGCCTRL0`
- `FREND1`
- `FREND0`
- `FSCAL3`
- `FSCAL2`
- `FSCAL1`
- `FSCAL0`
- `TEST2`
- `TEST1`
- `TEST0`
- `PA_TABLE0`
- `FSCTRL1`
- `FSCTRL0`
- `FREQ2`
- `FREQ1`
- `FREQ0`

The OpenLST core repository has good example settings in the `board_defaults.c`
file. Values for these parameters can be derived using Texas Instruments
SmartRF Studio. 

### Bootloader Features

 * Signature verification of application binaries
 * Reprogramming via UART
 * Reprogramming over the air
 * Locked code pages for extra safety assurance

### Application Features

 * Real-time clock
 * Time of light ranging
 * Basic callsign support
 * Command/message forwarding from UART to RF and vice versa
 * Settable reboot timer

### Adding a Custom Command

Custom command handlers can be added by defining `CUSTOM_COMMANDS` in `board.h`
and providing a `custom_commands` function in `board.c` (with a forward definition
in `board.h`), like:

```cpp
uint8_t custom_commands(const __xdata command_t *cmd, uint8_t len, __xdata command_t *reply) {
  switch (cmd->header.command) {
    ...
  }
}
```

## Creating a Custom Hardware Project

The OpenLST project includes an example hardware implementation. This design is
largely based on the Texas Instruments reference design for the CC1110
microcontroller and is targeted for operation at 437 MHz. The design also
incorporates an external power amplifier module (Qorvo RFFM6403) which which
boosts transmit power to 1 W (+30 dBm). The user is of course responsible for
obtaining a suitable license for operating the transmitter.

### Frequency Coverage

Although the reference design is intended for use at 437 MHz, it is possible to
operate at other frequencies. The transmit power amplifier module is rated for
operation from 405 MHz to 475 MHz. It may be necessary to adjust the balun /
matching network between the CC1110 and the PA if operating more than a few
megahertz away from 437 MHz. The CC1110 itself is capable of operating on a
much wider span of frequencies.

### Design Files

The hardware reference design is implemented in the populate open-source PCB
tool, KiCAD (v4.0.7). A schematic, bill of materials, component footprints, PCB
layout and ready-to-fab Gerber files are included in the package. The design
can also serve as a jumping off point for integrating the transceiver with
other circuitry.

### Fabrication and Prototyping

Care has been taken to ensure that the design can be easily replicated. A
four-layer stackup has been selected that is available from many of the
low-cost hobbyist board houses (e.g., OSH Park). The design has not been
optimized for size, but rather to be easy to assemble. The use of surface mount
components is necessary for this project, however, hand assembly is certainly
feasible.

### Specifications

| Parameter         | Value         | Notes                             |
| ----------------- | ------------- | --------------------------------- |
| Frequency         | 437 MHz       | 405-475 MHz covered by power amplifier module, Split frequency operation also available with firmware mods |
| RF Transmit Power | 1 W (+30 dBm) | Lower power possible by putting PA into bypass |
| Modulation format | 2FSK          | Variable (2FSK example project)   |
| User data rate    | 7.4 kBaud     | Variable (See CC1110 Manual and board.c file) |
| Signal Bandwidth  | 60 kHz        | Variable (60 kHz used by example) |
| Receiver Sensitivity | -112 dBm   | +/- 3 dB                          |
| Transmit harmonics | -47 dBc      |                                   |
| Input Power Supply | +5 VDC       |                                   |
| Size              | 5x6x0.5 cm    | Excludes antenna                  |
| Mass              | 20g (approx)  | Excludes antenna                  |
| Power Consumption | TX: 8.0 W     | 5V at 160 mA                      |
|                   | RX: 800 mW    | 5V at 1.6 A                       |
| Interface         | 3.3V TTL UART (x2) | 115,200 baud, 8N1            |


## Frequency Selection

Choosing a frequency can be complicated and depends on your use case. This
section is not meant to be exhaustive or offer advice, but some of these links
may be useful.

### Amateur Radio Frequencies

University and hobbyist projects may be able to use amateur radio frequencies
for communications. Users of amateur frequencies cannot be paid for their time,
be sure to read [Part 97](http://www.arrl.org/part-97-amateur-radio),
expecially Section 113. While OpenLST supports the entire 420-450 MHz Amateur
radio band, the digital modulation generated by OpenLST is only allowed in a a
small slice of this spectrum. Use 432.1-433.0 MHz for mixed-mode, weak signal,
and propogation beacons, and 435-438 MHz for satellites. See the
[ARRL Band Plan](http://www.arrl.org/band-plan) for more information.

For more information about Amateur radio licensing for CubeSats, check out
[FCC Part 97 Amateur Radio Licensing for CubeSats](
https://www.amsat.org/wordpress/wp-content/uploads/2018/04/AMSAST_CubeSat_Licensing.pdf)
document from AMSAT, and the
[Amateur Radio Satellite Frequency Coordination](
http://www.iaru.org/amateur-radio-satellite-frequency-coordination.html)
document from the IARU.


### Commercial Frequencies

Commercial companies, whether they are privately funded, venture backed, or
public, must use frequencies allocated for commercial services. For United
States teams, the best way to determine which service your project fits under
is to look at the [existing service definitions](
https://www.fcc.gov/wireless/bureau-divisions/technologies-systems-and-innovation-division/rules-regulations-title-47)
in the FCC rules.

For this OpenLST radio implementation, commercial frequencies that are in this
range include the Space Research downlink from 400.15-401 MHz, Space Operation
downlink from 401-402 MHz, Earth Exploration uplink from 402-403 MHz, and Fixed
and Land Mobile from 450-470 MHz. See Chapter 4b of the [NTIA Red Book](
https://www.ntia.doc.gov/page/2011/manual-regulations-and-procedures-federal-radio-frequency-management-redbook)
for the frequency table. Don't forget to read all the footnotes in Chapter 4c
(such as US87 for 450 MHz uplink in the United States).

To change the transmit and/or receive frequencies, see the "Simple RF
Customization" section above.

For those United States teams whose project is expected to last for two years
or less, consider a Part 5 Experimental License. These types of licenses are
very cheap and easier to get, provided they follow the existing frequency
allocations. More information in this [FCC Notice](
https://www.fcc.gov/document/guidance-obtaining-licenses-small-satellites).

### Hardware changes

The OpenLST RF design works across all the frequency ranges described above,
from 400 to 470 MHz. The only part that will need to change is the custom SAW
filter (U8, STA1120A). This filter is centered at 446 MHz with a bandwidth of
20 MHz. For those teams wanting to use lower or higher frequencies, a different
SAW filter must be procured. If you want to use the existing layout, Sawtron
makes a wide selection of filters in the same package. Free samples are
available, but lead times can be lengthy. For operation outside 400-470 MHz, a
standard mixer may be placed on the output of the CC1110, or use a different
frequency range in the CC1110.

## Troubleshooting

There are a few common issues you might hit, and while we can't predict all of
them, this section aims to give a broad background on some of the common
issues. We recommend you take a look at the dedicated resources created by the
specific projects' maintainers.

### General Linux Resources

#### Man pages and help

Many standard Linux programs will have a "man" page or manual page associated
with them. To read the page, try:

```bash
$ man make
$ # Opens man page for make
```

Even more ubiquitous are help queries, usually accessed by `-h` or `--help`.
Many of the python helper scripts included with OpenLST will include help text,
for example:

```bash
$ radio_terminal --help
usage: radio_terminal [-h] [-u {0,1}] [--rx-socket RX_SOCKET]
                      [--tx-socket TX_SOCKET] [--echo-socket ECHO_SOCKET]
                      [--hwid HWID] [--seqnum SEQNUM] [--raw]

optional arguments:
  -h, --help            show this help message and exit
  -u {0,1}, --uart {0,1}
                        Select the radio UART to watch (0 or 1). The default
                        is UART 1.
...
```

### ZMQ

The bulk of back-end message handling is done by ZMQ / 0MQ / ZeroMQ. ZMQ
creates sockets (the default setup by OpenLST is for IPC sockets in Linux's
`/tmp/` directory (which is commonly mounted as a tmpfs and is usually wiped on
reboot).

ZMQ allows us to "mux" or multiplex messaging so that multiple processes or
users can communicate simultaneously to the radio. In the default example,
users will communicate with the `radio_mux`, which opens the UART and handles
the actual communication with the radio.

ZMQ has a few different models, and we recommend you read the ZMQ guide if you
are unfamiliar ([zmq manual](http://zguide.zeromq.org/page:all)). The default
project uses the PUB/SUB and PUSH/PULL models.

ZMQ will have issues if the proper sockets don't exist. Check for the presence
of the sockets described in `radio_mux.py`. If these files don't exist, there's
most likely an issue with either the device or the `radio_mux`.

### Systemd / Upstart / initd

Communication with the radio in the demo project relies on the `radio_mux`
running to handle messages. Usually it should run as a daemon handled by the
underlying operating system's process supervisor. In the demo project,
`radio_mux` is handled by systemd, but you can change it or run the mux by hand
if needed.

If the ZMQ sockets don't exist, check your process supervisor and make sure the
`radio_mux` process is running.

### Udev Rules

The Linux kernel relies on a "userspace device" handler (udev) to interface
with devices like the OpenLST's UART. The default project includes a udev
configuration to create a named device for the OpenLST.

In general, you can ask the kernel to reexamine devices and rules by running
commands like:

```bash
# udevadm control --reload-rules
# udevadm trigger
```

## Acknowledgements

This project would not have been possible without the hard work of many people
at Planet over the course of many years. We would especially like to thank:

- Henry Hallam
- Alex Ray
- Rob Zimmerman
- Matt Peddie
- Helen Lurie
- Matt Ligon
- Bryan Klofas
- Ryan Kingsbury
- Rob Harvey
- Lucas Eznarriaga
- Kiruthika Devaraj
- Kyle Colton
- Steve Burt
- Joseph Breu
- Tymm Zerr

## License Info

OpenLST
Copyright (C) 2018 Planet Labs Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see
[https://www.gnu.org/licenses/](https://www.gnu.org/licenses/)

## Disclaimers

This document may contain technology or software the export of which is or may
be restricted by the Export Administration Act and the Export Administration
Regulations (EAR), 15 C.F.R. parts 730-774. Diversion contrary to U.S. law is
prohibited.
