#include <SPI.h>
#include <Ethernet.h>

#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

#include <Servo.h>

const int sensorPin = A0;
const int servoPin = 9;
const int addr = 177;
const int sensivity = 4;//порог фиксации удара
const int defaultPosition = 90;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 42, addr);//указываем ip контроллера

EthernetServer ethServer(502);//создаем веб-сервер на 502 порту

ModbusTCPServer modbusTCPServer;

Servo servo;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);//defaults
  Serial.begin(115200);
  servo.attach(servoPin);

  Ethernet.begin(mac, ip);//включаем ethernet-контроллер
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {//чек наличия шилда
    Serial.println("Шилд не обнаружен");
    while (true) {
      digitalWrite(LED_BUILTIN, 1);
      delay(100);
      digitalWrite(LED_BUILTIN, 0);
      delay(300);
    }
  }

  ethServer.begin();//запуск веб-сервера

  if (!modbusTCPServer.begin()) {//чек запуска утилиты модбаса
    Serial.println("Невозможно запустить modbus сервер");
    while (true) {
      digitalWrite(LED_BUILTIN, 1);
      delay(200);
      digitalWrite(LED_BUILTIN, 0);
      delay(600);
    }

  }

  digitalWrite(LED_BUILTIN, 1);//все стартануло-врубаем светодиод

  modbusTCPServer.configureHoldingRegisters(0x00, 6);
  /*
     пояснение к регистрам
     0x00 - id контроллера (чтение)
     0x01 - тип контроллера: 00 - мишень; (чтение)
     0x02 - значение сервы 0-180 (запись)
     0x03 - значение датчика удара (чтение)
     0x04 - состояние мишени: был удар/не было (чтение)
     0x05 - сброс удара (запись)
  */
  //дефолтные значения регистров
  modbusTCPServer.holdingRegisterWrite(0x00, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x01, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x02, defaultPosition);
  modbusTCPServer.holdingRegisterWrite(0x03, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x04, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x05, 0x00);

  servo.write(defaultPosition);
}

void loop() {
  EthernetClient client = ethServer.available();

  if (client) {
    Serial.println("Новое подключение");
    resetController();
    modbusTCPServer.accept(client);

    while (client.connected()) {
      modbusTCPServer.poll();
      setServo(modbusTCPServer.holdingRegisterRead(0x02));
      updateSensor();
      if (modbusTCPServer.holdingRegisterRead(0x05) == 0x01) {//если надо сбросить удар
        modbusTCPServer.holdingRegisterWrite(0x04, 0x00);
        modbusTCPServer.holdingRegisterWrite(0x05, 0x00);
      }
    }

    Serial.println("Отключен клиент");
  }
}

void resetController() {//сброс до дефолтной конфигурации
  modbusTCPServer.holdingRegisterWrite(0x00, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x01, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x02, defaultPosition);
  modbusTCPServer.holdingRegisterWrite(0x03, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x04, 0x00);
  modbusTCPServer.holdingRegisterWrite(0x05, 0x00);
  
  servo.write(defaultPosition);
}

void setServo(int value) {//перемещение сервы
  servo.write(value);
}

void updateSensor() {//чтение значения с датчика удара
  uint16_t value = analogRead(sensorPin);
  modbusTCPServer.holdingRegisterWrite(0x03, value);
  if (value > sensivity) {
    modbusTCPServer.holdingRegisterWrite(0x04, 0x01);//произошел удар
  }
}

void reset() {//сброс удара
  modbusTCPServer.holdingRegisterWrite(0x04, 0x00);
}
