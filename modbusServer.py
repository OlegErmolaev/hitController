from pymodbus.client.sync import ModbusTcpClient

host = '192.168.42.177'
port = 502 

client = ModbusTcpClient(host, port)
client.connect()
while(not client.is_socket_open()):
    pass
rr = client.read_holding_registers(0x00,6,unit=1)
print(rr.registers)

