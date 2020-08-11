/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "INFINITUM149C_2.4"
#define WLAN_PASS       "*****"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "****"
#define AIO_KEY         "****"


// Assign output variables to GPIO pins
const int L1 = 4;
const int L2 = 5;
const int L3 = 16;
const int SW1 = 14;
const int SW2 = 13;
const int SW3 = 12;
String sw1State = "off";
String sw2State = "off";
String sw3State = "off";
int swBtnPushed = 0;
int btnDown = 0;
//const int output2 = 14;             // D5
//const int inputSwitch = 12;         // D6
//String output2State = "off";
//bool buttonPushed = false;


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

//Adafruit_MQTT_Publish sub_sw1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "feeds/product-1.sw1");
Adafruit_MQTT_Subscribe sub_sw1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/product-1.sw1");
Adafruit_MQTT_Subscribe sub_sw2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/product-1.sw2");
Adafruit_MQTT_Subscribe sub_sw3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/product-1.sw3");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(L3, LOW);

  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&sub_sw1);
  mqtt.subscribe(&sub_sw2);
  mqtt.subscribe(&sub_sw3);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if(subscription == &sub_sw1) {
      String value = (char *)sub_sw1.lastread;
      Serial.print(F("Got: "));
      Serial.println(value);

      // turns the GPIOs on and off
      if (value.indexOf("ON")>=0) {
        Serial.println("SW1 on");
        sw1State = "on";
        digitalWrite(L1, HIGH);
      } else {
        Serial.println("SW1 off");
        sw1State = "off";
        digitalWrite(L1, LOW);
      }
    } else if(subscription == &sub_sw2) {
      String value = (char *)sub_sw2.lastread;
      if (value.indexOf("ON")>=0) {
        Serial.println("SW2 on");
        sw1State = "on";
        digitalWrite(L2, HIGH);
      } else {
        Serial.println("SW2 off");
        sw1State = "off";
        digitalWrite(L2, LOW);
      }
    } else if(subscription == &sub_sw3) {
      String value = (char *)sub_sw3.lastread;
      if (value.indexOf("ON")>=0) {
        Serial.println("SW3 on");
        sw1State = "on";
        digitalWrite(L3, HIGH);
      } else {
        Serial.println("SW3 off");
        sw1State = "off";
        digitalWrite(L3, LOW);
      }
    }
  }

    if(LOW == digitalRead(SW1)) {
      btnDown = 1; 
    } else if(btnDown == 1) {
      swBtnPushed = 1;
      btnDown = 0;
    }

    if(LOW == digitalRead(SW2)) {
      btnDown = 2; 
    } else if(btnDown == 2) {
      swBtnPushed = 2;
      btnDown = 0;
    }

    if(LOW == digitalRead(SW3)) {
      btnDown = 3; 
    } else if(btnDown == 3) {
      swBtnPushed = 3;
      btnDown = 0;
    }

    if(swBtnPushed > 0) {
      switch (swBtnPushed) {
         case 1: {
          Serial.println("MANUAL SWITCH 1 PUSHED");
          if(sw1State.indexOf("off") >= 0) {
            Serial.println("MANUAL SWITCH 1 " + sw1State);
            sw1State = "on";
            digitalWrite(L1, HIGH);
          } else {
            Serial.println("MANUAL SWITCH 1 " + sw1State);
            sw1State = "off";
            digitalWrite(L1, LOW);
          }
          break; 
         }
         case 2: {
          if(sw2State.indexOf("off") >= 0) {
            Serial.println("MANUAL SWITCH 2 " + sw2State);
            sw2State = "on";
            digitalWrite(L2, HIGH);
          } else {
            Serial.println("MANUAL SWITCH 2 " + sw2State);
            sw2State = "off";
            digitalWrite(L2, LOW);
          }
          break; 
         }
         case 3: {
          if(sw3State.indexOf("off") >= 0) {
            Serial.println("MANUAL SWITCH 3 " + sw3State);
            sw3State = "on";
            digitalWrite(L3, HIGH);
          } else {
            Serial.println("MANUAL SWITCH 3 " + sw3State);
            sw3State = "off";
            digitalWrite(L3, LOW);
          }
          break;
         }
          
      }
      swBtnPushed = 0;
   }
  
  // Now we can publish stuff!
  /*
   * 
  Serial.print(F("\nSending photocell val "));
  Serial.print(x);
  Serial.print("...");
  if (! photocell.publish(x++)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  */

  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
