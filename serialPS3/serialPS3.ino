// ESP32
#include <Ps3Controller.h>


bool joystickMovedRight = false;
bool joystickMovedLeft = false;
bool joystickMovedUp = false;
bool joystickMovedDown = false;
bool movedUp = false;
bool movedDown = false;
bool movedLeft = false;
bool movedRight = false;

void setup() {
  Serial.begin(9600);    // USB: comunicación con PC
  Serial2.begin(9600, SERIAL_8N1, 19, 21);  // UART: con ATmega (RX, TX)
  controller_setup();
}

void controller_setup() {
  Ps3.attach(controller_callback);
  Ps3.attachOnConnect(controller_connect);
  Ps3.begin("60:f4:94:5c:be:bb");
  Serial.println("Ready.");
}

void controller_callback() {
  int rx = Ps3.data.analog.stick.rx;
  int ry = Ps3.data.analog.stick.ry;
  int lx = Ps3.data.analog.stick.lx;
  int ly = Ps3.data.analog.stick.ly;

  // Arriba (ry negativo)
  if (ry < -120 && !movedUp) {
    Serial2.write('a');
    Serial.print('a');
    movedUp = true;
    movedDown = movedLeft = movedRight = false;
  }
  // Derecha (rx positivo)
  else if (rx > 120 && !movedRight) {
    Serial2.write('b');
    Serial.print('b');
    movedRight = true;
    movedUp = movedDown = movedLeft = false;
  }
  // Abajo (ry positivo)
  else if (ry > 120 && !movedDown) {
    Serial2.write('c');
    Serial.print('c');
    movedDown = true;
    movedUp = movedLeft = movedRight = false;
  }
  // Izquierda (rx negativo)
  else if (rx < -120 && !movedLeft) {
    Serial2.write('d');
    Serial.print('d');
    movedLeft = true;
    movedUp = movedDown = movedRight = false;
  }


  if (ly > 120 && !joystickMovedDown) {
    Serial2.write('e');
    Serial.print('e');
    joystickMovedUp = false;
    joystickMovedDown = true;
  }
  if (ly < -120 && !joystickMovedUp) {
    Serial2.write('e');
    Serial.print('e');
    joystickMovedUp = true;
    joystickMovedDown = false;
  }


  if (lx > 120 && !joystickMovedRight) {
    Serial2.write('f');
    Serial.print('f');
    joystickMovedLeft = false;
    joystickMovedRight = true;  // Resetea el otro lado
  }
  if (lx < -120 && !joystickMovedLeft) {
    Serial2.write('f');
    Serial.print('f');
    joystickMovedLeft = true;
    joystickMovedRight = false;  // Resetea el otro lado
  }
}

// Funcion que se ejecuta cuando se conecta el mando
void controller_connect() {
  Serial.println("Connected.");
}


void loop() {
  // Del USB al ATmega
  /*if (Serial.available()) {
    char byteFromUSB = Serial.read();
    Serial2.write(byteFromUSB);
    Serial.println("Send " + String(byteFromUSB));
    }*/

}
