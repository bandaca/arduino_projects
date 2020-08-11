
/*
 * ESP8266 E-12   SIM800L
 * D7 -13         TX
 * D8 -15         RX
 * RESET          RST
 * GND            GND
 */

#include <SoftwareSerial.h>

const int SIM800_TX = 13;
const int SIM800_RX = 15;
const int OUTPUT_PIN = 14;

SoftwareSerial MOD_SIM800L(SIM800_RX, SIM800_TX);

void setup() {
  // put your setup code here, to run once:

  pinMode(OUTPUT_PIN, OUTPUT);
  
  Serial.begin(115200);

  MOD_SIM800L.begin(115200);

  digitalWrite(OUTPUT_PIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(OUTPUT_PIN, HIGH);  
  
  if (MOD_SIM800L.available()) {
    Serial.write(MOD_SIM800L.read());
  }

  if(Serial.available()) {
    while(Serial.available()) {
      MOD_SIM800L.write(Serial.read());
    }
    MOD_SIM800L.println();
  }
  
 }
