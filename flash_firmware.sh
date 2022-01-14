sign_radio -k FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF satfab_telem_radio.hex satfab_telem_radio.sig
bootload_radio --signature-file satfab_telem_radio.sig -i 0001 satfab_telem_radio.hex --tx-path ipc:///tmp/radiomux$1_tx --rx-path ipc:///tmp/radiomux$1_rx
