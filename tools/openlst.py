import serial
from dataclasses import dataclass


@dataclass
class OpenLSTParams:
    # Configuration
    f_ref = 27e6

    # Register values
    FSCTRL0: int
    FSCTRL1: int
    FREQ: int
    MDMCFG4: int
    MDMCFG3: int
    MDMCFG2: int
    DEVIATN: int
    PA_CONFIG: int

    def set_if(self, f_if) -> float:
        FREQ_IF = int(2**10 * f_if / self.f_ref)
        f_if_actual = self.f_ref * FREQ_IF / (2**10)

        self.FSCTRL1 = FREQ_IF

        return f_if_actual

    def set_freqoff(self):
        "TODO"
        pass

    def set_freq(self, f_carrier) -> float:
        FREQ = int(2**16 * f_carrier / self.f_ref)
        f_carrier_actual = self.f_ref * FREQ / (2**16)

        self.FREQ = FREQ

        return f_carrier_actual

    def set_datarate(self, baud) -> bool:
        pass



class OpenLST:
    def __init__(self, port: str, ser: serial.Serial = None, baud=115200) -> None:
        if ser:
            self.ser = ser
        else:
            self.ser = serial.Serial(port, baud)

    def _send(self, cmd: int, msg: bytes):
        pass

    def transmit():
        pass

    def receive():
        pass

    def reboot():
        pass

    def get_time():
        pass

    def set_time():
        pass

    def get_telem(self):
        pass

    def set_rf_params(self, params: OpenLSTParams):
        pass
