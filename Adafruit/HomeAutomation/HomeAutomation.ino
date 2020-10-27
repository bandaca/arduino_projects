#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "cbandam"
#define AIO_KEY         ""


/************************* WiFi Access Point *********************************/

String WLAN_SSID = "";
String WLAN_PASS = "";

// Assign output variables to GPIO pins
const int LINES[] = {4, 5, 16};  
const int SWITCH[] = {14, 13, 12};
String switchStatus[] = {"off", "off", "off"};
int swBtnPushed = -1;
int btnDown = -1;
int btnDownCounter = 0;

// Set web server port number to 80
ESP8266WebServer server(80);
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
String header;


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
  delay(100);

  ConnectWiFi_STA(WLAN_SSID, WLAN_PASS);

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

  if (WiFi.getMode() == WIFI_AP) server.handleClient();

  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

void handleRoot() {
  const String content = "<!DOCTYPE html><html>\
  <head>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <link rel=\"icon\" href=\"data:,\">\
    <style>\
      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\
      .button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;}\
      text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\
      .button2 {background-color: #77878A;}\
     </style>\
   </head>\
  <body>\
    <h1>ESP8266 Web Server</h1>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      <div><label>SSID</label><input type='text' name='ssid' id='ssid' value=\"\" /></div>\
      <div><label>Password</label><input type='password' name='password' id='password' value=\"\"/></div>\
      <div><input type='submit' id='submit' value=\"submit\"/></div>\
    </form>\
  </body>\
 </html>";
 
  server.send(200, "text/html", content);
}

void handleForm() {
  String ssid = "";
  String password = "";
  if (server.method() != HTTP_POST) {
    Serial.println("Method Not Allowed");
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      if (server.argName(i) == "ssid") ssid = server.arg(i);
      if (server.argName(i) == "password") password = server.arg(i);
    }
    Serial.print("RESPONSE: ");
    Serial.println(message);
    server.send(200, "text/plain", message);
    delay(5000);
    
    ConnectWiFi_STA(ssid, password);
  }
}



void ConnectWiFi_STA(String ssid, String password)
{
   // Connect to WiFi access point.
  Serial.print("Connecting WiFi: ");
  Serial.println(ssid);
  Serial.println(password);

  WiFi.mode(WIFI_STA);
  int count = 0;
  if ( ssid.length() > 0 ) {
    WiFi.begin(ssid, password);
  } else {
    WiFi.begin();
  }
  
  Serial.println();
  
  while (WiFi.status() != WL_CONNECTED && count < 50) {
    delay(500);
    Serial.print(".");
    count++;
  }

  Serial.println();
  
  if (WiFi.status() != WL_CONNECTED && ssid.length() > 0) {
    Serial.println("Unable to connect to the new credentials. Using previous credentials.");
    Serial.println();
    WiFi.begin();
    count=0;
    while (WiFi.status() != WL_CONNECTED && count < 50) {
      delay(500);
      Serial.print(".");
      count++;
    }
  }
  
  Serial.println();

  if (WiFi.status()!= WL_CONNECTED) {
    Serial.print("Unable to connect to WiFi.");
    ConnectWiFi_AP();
    return;
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());


   
}

void ConnectWiFi_AP()
{ 
   String ssid = "WifiSwitch-" + String(ESP.getChipId(), HEX);
   String password = "Admin-123";

   
   IPAddress local_IP(192,168,250,1);
   IPAddress gateway(192,168,250,254);
   IPAddress subnet(255,255,255,0);


   Serial.println("Setting soft-AP configuration ... ");
   Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
   
   Serial.println("");
   WiFi.mode(WIFI_AP);
   while(!WiFi.softAP(ssid, password))
   {
     Serial.println(".");
     delay(100);
   }

   if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

   Serial.println("Starting web server on port 80...");
   server.on("/", handleRoot);
   server.on("/postform/", handleForm);
   server.begin();
   Serial.println("HTTP server started");
   
   Serial.println("");
   Serial.print("Iniciado AP:\t");
   Serial.println(ssid);
   Serial.print("IP address:\t");
   Serial.println(WiFi.softAPIP());
}

void publishValue(char* value, byte index) {
  if (! pubs[index].publish(value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

void checkManualSwitch() {

  for (byte i = 0; i < 3; i += 1){
    if (HIGH == digitalRead(SWITCH[i])) {
      //Serial.println("MANUAL SWITCH DOWN");
      btnDown = i;
      if (WiFi.getMode() == WIFI_STA) {
        btnDownCounter++;
        if (btnDownCounter >= 50) {
          btnDownCounter = 0;
          btnDown = -1;
          ConnectWiFi_AP();
        }
      }      
    } else if (btnDown == i) {
      swBtnPushed = i;
      btnDown = -1;
      btnDownCounter = 0;
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

  if (WiFi.status() != WL_CONNECTED) {
    //Serial.println("No WiFi Connected...");
    return;
  }
  
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
