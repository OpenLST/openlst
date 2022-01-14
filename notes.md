
            USE GOOGLE DOC NOW

# Hardware

OpenLST max input voltage - 16 V

Does TR pin actually do anything on TX?
* With TR set high, -40 dBFS dBFS on RTL-SDR
* With TR set low, -33 dBFS dBFS on RTL-SDR

## OpenLST Pinouts

### Port 0

1. GND
2. UART0_RTS - P1_3 - USART0 (SPI C, UART RT)
3. NC
4. UART0_RX - P1_4 - USART0 (SPI MISO, **UART RX**), USART1 (SPI SS, UART CT)
5. UART0_TX - P1_5 - USART0 (SPI MOSI, **UART TX**), USART1 (SPI C, UART RT)
6. UART0_CTS - P1_2 - USART0 (SPI SS, UART CT)

### Port 1

1. GND
2. CAN_CS - P0_2 - USART0 (SPI MISO, UART RX), USART1 (SPI SS, UART CT)
3. NC
4. CAN_SCK - P0_3 - USART0 (SPI MOSI, UART TX), USART1 (SPI C, UART RT)
5. CAN_MOSI - P0_4 - USART0 (SPI SS, UART CT), USART1 (SPI MOSI, **UART TX**)
6. CAN_MISO - P0_5 - USART0 (SPI C, UART RT), USART1 (SPI MISO, **UART RX**)

### Leds

* LED0 - P0_6
    * Controlled by board_led0_set() function
* LED1 - P0_7
    * Controlled by board_led_set() function

### Frontend Controls

* P1_7
    * ~LST_RX_MODE
    * Connected to PE4250 switch
    * 1k pulldown
    * High for TX, low for RX
* P1_6
    * LST_TX_MODE
    * Connected to RFFM6404 TR pin
    * 1k pullup
    * High for TX, low for RX
* P1_0
    * RF_EN
    * Connected to RFFM6404 EN pin
    * 1k pullup
    * High for enable, low for shutdown
* P1_1
    * RF_BYP
    * Connected to RFFM6404 BYP pin
    * 1k pulldown
    * High for LNA bypass, low for LNA normal

# Software

## Demodulation

https://edamworkshop.blogspot.com/2020/04/2fsk-demodulation-with-rtl-sdr-and.html
https://edamworkshop.blogspot.com/2020/04/closer-look-at-2fsk-demodulated-message.html

## ADC Measurements

* 1.25V reference
* 12 bits


Inputs
* AN0 - VAA
* AN1 - NC
* Internal temperature sensor
* CC1110 VDD/3
