from pymodbus.exceptions import ModbusIOException
from pymodbus.client.sync import ModbusTcpClient
import time

host = '192.168.42.177'
port = 502 



class device():
    '''
    пояснение к регистрам контроллера мишени
    0x00 - id контроллера (чтение)
    0x01 - тип контроллера: 00 - мишень; (чтение)
    0x02 - значение сервы 0-180 (запись)
    0x03 - значение датчика удара (чтение)
    0x04 - состояние мишени: был удар/не было (чтение)
    0x05 - сброс удара (запись)
    '''
    def __init__(self, host, port):
        self.host = host
        self.port = port
        
        self.client = ModbusTcpClient(host, port)
        self.client.connect()

        while(not self.client.is_socket_open()):
            pass
        print("Connected to device: %s\n\r device type: %s" % (str(hex(self.read(0x00))), str(hex(self.read(0x01)))))
        
    def write(self, addr, value):
        self.client.write_register(addr, value, unit=1)

    def read(self, addr):
        rr = self.client.read_holding_registers(addr, 1, unit=1)
        if type(rr) != ModbusIOException:
            return (rr.registers)[0]
        else:
            return -1

    def disconnect(self):
        self.client.close()

hitController = device(host, port)
#hitController.write(0x05, 1)
#hitController.write(0x02, 0)
#time.sleep(1)
#hitController.write(0x02, 180)
#time.sleep(1)
print("instant status")
try:
    while True:
        print("%d %d" % (hitController.read(0x03), hitController.read(0x04)))
        time.sleep(0.1)
except KeyboardInterrupt:
    print("Ctrl+C pressed")
    hitController.disconnect()
    
