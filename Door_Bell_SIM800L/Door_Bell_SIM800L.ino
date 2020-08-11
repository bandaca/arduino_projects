
/*
 * ESP8266 E-12   SIM800L
 * D7 -13         TX
 * D8 -15         RX
 * RESET          RST
 * GND            GND
 */

#include <SoftwareSerial.h>

const int SIM800_TX = 13;         // D7
const int SIM800_RX = 15;         // D8
const int SIGNAL_IN = 12;         // D6
const int SIGNAL_STATUS_OUT = 14; // D5
const String PHONE_NO_1 = "+520000000000;";
const String PHONE_NO_2 = "+520000000000;";

int signalState = LOW;
bool buttonPushed = false;

SoftwareSerial MOD_SIM800L(SIM800_RX, SIM800_TX);

void setup() {
  // put your setup code here, to run once:

  pinMode(SIGNAL_IN, INPUT);
  pinMode(SIGNAL_STATUS_OUT, OUTPUT);

  digitalWrite(SIGNAL_STATUS_OUT, signalState);
  
  Serial.begin(115200);

  MOD_SIM800L.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:  
  int retryCounter = 0;
  
  if(HIGH == digitalRead(SIGNAL_IN)) {
      buttonPushed = true;
  } else {
    
    if (buttonPushed) {
      
      if(signalState == HIGH)
         signalState = LOW;
      else
         signalState = HIGH;
         
      Serial.printf("INPUT %d DETECTED\r\n", signalState);
      
      digitalWrite(SIGNAL_STATUS_OUT, signalState);
      buttonPushed = false;
    }
  
  }

  if (HIGH == signalState) {
   
    signalState = LOW;
    
    // CHECK SIM800 STATUS
    if (sendCommand("AT").indexOf("OK") > 0) {
      if (sendCommand("AT+CCALR?").indexOf("1") > 0) {
        // IF STATUS IS AVAILABLE FOR CALL THEN MAKE CALL NUMBER 1
        sendCommand("ATD" + PHONE_NO_1);
        delay(15000);
        sendCommand("ATH");

        retryCounter = 0;
        {
          Serial.println("Intentando llamada: " + PHONE_NO_1);
          retryCounter++;
          delay(1000);
        } while(sendCommand("AT+CCALR?").indexOf("1") <= 0 && retryCounter < 10)
        
        sendCommand("ATD" + PHONE_NO_1);
        
        delay(15000);
        sendCommand("ATH");
      }
    }

    digitalWrite(SIGNAL_STATUS_OUT, signalState);
    
    // DELAY(15000) // WAIT 15 SECONDS
    // IF CALL IN PROGRESS THEN HANG UP
    // DELAY(5000)  // WAIT 5 SECONDS
    // IF STATUS IS AVAILABLE FOR CALL THEN MAKE CALL NUMBER 2
  }

  
 }

String sendCommand(String command)
{ 
  String Sim800Output = "";

  Serial.println(command);
  MOD_SIM800L.println(command);

  delay(500);
  /*
  while(Serial.available()) {
    Sim800Output = Serial.readString();
    MOD_SIM800L.write(Sim800Output);
  }
  */
  
  while (MOD_SIM800L.available()) {
    char c = MOD_SIM800L.read();
    Sim800Output += c;
    //Serial.write(c);
  }
  Serial.println(Sim800Output);


  return Sim800Output;
}
