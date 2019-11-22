#include <Arduino.h>
#line 1 "c:\\Users\\Jonathan\\Documents\\GitHub\\SmartSwitch\\code\\code.ino"
#line 1 "c:\\Users\\Jonathan\\Documents\\GitHub\\SmartSwitch\\code\\code.ino"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include "FS.h"   // Include the SPIFFS library

// Assign output variables to GPIO pins
const int pin_set = 13; // D7
const int pin_reset = 15; // D8

const int pin_set_led = 5; // D1
const int pin_reset_led = 2; // D4

bool master_state = false; // State of SmartSwitch

bool set_state = false;
int set_timeout = 99999; // Initialized to not time out initially
bool reset_state = false;
int reset_timeout = 99999; // Initialized to not time out initially

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)

#line 29 "c:\\Users\\Jonathan\\Documents\\GitHub\\SmartSwitch\\code\\code.ino"
void setup();
#line 73 "c:\\Users\\Jonathan\\Documents\\GitHub\\SmartSwitch\\code\\code.ino"
void loop(void);
#line 29 "c:\\Users\\Jonathan\\Documents\\GitHub\\SmartSwitch\\code\\code.ino"
void setup() {

  Serial.begin(115200);
  while(!Serial) { delay(100); }

  Serial.println("test");

  pinMode(pin_set, OUTPUT);
  pinMode(pin_reset, OUTPUT);
  digitalWrite(pin_set, LOW);
  digitalWrite(pin_reset, LOW);

  wifiMulti.addAP("ATTW6YS4I2", "c%qi=ijne=t+");   // add Wi-Fi networks you want to connect to

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("SmartSwitch")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  SPIFFS.begin();                           // Start the SPI Flash Files System
  
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
  
}

void loop(void) {
  
  ESP.wdtFeed();
  
  server.handleClient();

  if (set_state && millis() > set_timeout)
  {
    digitalWrite(pin_set, LOW);

    set_state = false;
  }

  if (reset_state && millis() > reset_timeout)
  {
    digitalWrite(pin_reset, LOW);

    reset_state = false;
  }
  
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)

  Serial.println("handleFileRead: " + path);
  
  if (path.endsWith("/"))
  {
    if (master_state)
      path += "button_on.html";
    else
      path += "button_off.html";
  }
  else if (path.endsWith("/button_on.html")) 
  {
    digitalWrite(pin_set, HIGH);
    digitalWrite(pin_set_led, HIGH);
    digitalWrite(pin_reset_led, LOW);

    set_state = true;
    set_timeout = millis() + 500;
    master_state = true;

    Serial.println(path);
  }
  else if (path.endsWith("/button_off.html"))
  {
    digitalWrite(pin_reset, HIGH);
    digitalWrite(pin_set_led, LOW);
    digitalWrite(pin_reset_led, HIGH);

    reset_state = true;
    reset_timeout = millis() + 500;
    master_state = false;
    
    Serial.println(path);
  }
  
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

