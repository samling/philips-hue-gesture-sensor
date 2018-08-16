#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#define DESK        1
#define CEILING     2
#define NIGHTSTAND1 4
#define NIGHTSTAND2 3

int trigPin = D6;
int echoPin = D7;
long duration, cm, inches;
boolean obstructed;

const char* ssid     = wifiName;
const char* password = wifiPass;
const char* API      = apiEndpoint;

HTTPClient http;

IPAddress ip(192, 168, 86, 169);
IPAddress gateway(192, 168, 86, 1); 
IPAddress subnet(255, 255, 255, 0);

void setup() {
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  //Serial Port begin
  Serial.begin(9600);
  
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  obstructed = false;

  //pulse(1, 125);
}
 
void loop()
{
  while(obstructed == false) {
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
   
    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);
   
    // Convert the time into a distance
    cm = (duration/2) / 29.1;
    inches = (duration/2) / 74; 

    // Print readings to serial monitor
    Serial.print(inches);
    Serial.print("in, ");
    Serial.print(cm);
    Serial.print("cm");
    Serial.println();
    delay(250);

    // If something is obstructing the sensor within 10cm [todo: for longer than some period of time],
    // interrupt our loop and give us a signal that our obstruction has been detected (pulse twice)
    if (cm < 10) {
      obstructed = true;
      pulse(1, 125);
    }
  }
}

void pulse(int light, int brightVal) {
  // Generate request endpoint
  char host[100];
  sprintf(host, "%s/lights/%i/state", API, light);
  Serial.println(host);
  
  // Generate request payload
  char brightness[100];
  sprintf(brightness, "{\"bri\": %i, \"transitiontime\": 1}", brightVal);

  // Open connection and set payload type to json
  http.begin(host);
  http.addHeader("Content-Type", "application-json");

  // Pulse lights twice between brightVal (out of 254) and max brightness (254)
  hueRequest(brightness, light);
  delay(150);
  hueRequest("{\"bri\": 254, \"transitiontime\": 1}", light);
  delay(150);
  hueRequest(brightness, light);
  delay(150);
  hueRequest("{\"bri\": 254, \"transitiontime\": 1}", light);
  delay(150);

  // Close connection
  http.end();

  // Reset interrupt
  obstructed = false;
}

String hueRequest(char* data, int light) {
  int httpCode = http.sendRequest("PUT", data);
  String payload = http.getString();
  Serial.println(httpCode);
  Serial.println(payload);
}

