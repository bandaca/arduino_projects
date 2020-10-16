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
#define WLAN_PASS       "***"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "***"
#define AIO_KEY         "***"


// Assign output variables to GPIO pins
const int LINES[] = {4, 5, 16};  
const int SWITCH[] = {14, 13, 12};
String switchStatus[] = {"off", "off", "off"};
int swBtnPushed = -1;
int btnDown = -1;


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Subscribe subs[3] = {
  Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/product-1.sw1"),
  Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/product-1.sw2"),
  Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/product-1.sw3")
};

Adafruit_MQTT_Publish pubs[3] = { 
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/product-1.sw1"),
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/product-1.sw2"),
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/product-1.sw3")
};

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {

  for(byte i = 0; i < 3; i = i + 1) {
    pinMode(LINES[i], OUTPUT);
    digitalWrite(LINES[i], LOW);
    pinMode(SWITCH[i], INPUT);
  }
  
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
  for(byte i = 0; i < 3; i = i + 1) {
      mqtt.subscribe(&subs[i]);
  }

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
    handleSubscriptionMessage(subscription);
  }

  checkManualSwitch();

  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

void publishValue(char* value, byte index) {
  if (! pubs[index].publish(value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

void checkManualSwitch() {

  for(byte i = 0; i < 3; i += 1){
    if(LOW == digitalRead(SWITCH[i])) {
      btnDown = i; 
    } else if(btnDown == i) {
      swBtnPushed = i;
      btnDown = -1;
    } 
  }


  if(swBtnPushed >= 0) {
    for(byte i = 0; i < 3; i += 1) {
      if(swBtnPushed == i) {
        Serial.println("MANUAL SWITCH PUSHED");
        if(switchStatus[i].indexOf("off") >= 0) {
          switchStatus[i] = "on";
          digitalWrite(LINES[i], HIGH);
          publishValue("ON", i);
        } else {
          switchStatus[i] = "off";
          digitalWrite(LINES[i], LOW);
          publishValue("OFF", i);
        }
      }
    }
    swBtnPushed = -1;
  }
  
}

void handleSubscriptionMessage(Adafruit_MQTT_Subscribe *subscription) {
  byte subIndex = -1;
  for(byte i = 0; i < 3; i += 1) {
    if(subscription == &subs[i]) {
      subIndex = i;
      break;
    }
  }

  if(subIndex < 0) return;
  
  String value = (char *)subs[subIndex].lastread;
  Serial.print(F("Got: "));
  Serial.println(value);

  // turns the GPIOs on and off
  if (value.indexOf("ON") >= 0) {
    Serial.println("SW on");
    switchStatus[subIndex] = "on";
    digitalWrite(LINES[subIndex], HIGH);
  } else {
    Serial.println("SW off");
    switchStatus[subIndex] = "off";
    digitalWrite(LINES[subIndex], LOW);
  }
  
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
