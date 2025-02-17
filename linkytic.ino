#include "serial.h"
#include "secret.h"
#include "tic.h"


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define WIFI_ENABLE  //enable wifi
#define SERVER_NAME "ticweb"
#ifdef WIFI_ENABLE
ESP8266WebServer server(80);
#endif

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  DebugPort.println();
  DebugPort.print("Connecting to ");
  DebugPort.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passPhrase);

  int c = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DebugPort.print(".");
  }
  DebugPort.println("");
  DebugPort.println("WiFi connected");
  DebugPort.println("IP address: ");
  DebugPort.println(WiFi.localIP());

  if (MDNS.begin(SERVER_NAME)) {
    DebugPort.println(SERVER_NAME);
  }
}

void getSettings() {
  String response = "{";

  response += "\"ip\": \"" + WiFi.localIP().toString() + "\"";
  response += ",\"gw\": \"" + WiFi.gatewayIP().toString() + "\"";
  response += ",\"nm\": \"" + WiFi.subnetMask().toString() + "\"";

  if (server.arg("signalStrength") == "true") {
    response += ",\"signalStrengh\": \"" + String(WiFi.RSSI()) + "\"";
  }

  if (server.arg("chipInfo") == "true") {
    response += ",\"chipId\": \"" + String(ESP.getChipId()) + "\"";
    response += ",\"flashChipId\": \"" + String(ESP.getFlashChipId()) + "\"";
    response += ",\"flashChipSize\": \"" + String(ESP.getFlashChipSize()) + "\"";
    response += ",\"flashChipRealSize\": \"" + String(ESP.getFlashChipRealSize()) + "\"";
  }
  if (server.arg("freeHeap") == "true") {
    response += ",\"freeHeap\": \"" + String(ESP.getFreeHeap()) + "\"";
  }
  response += "}";

  server.send(200, "text/json", response);
}

void getTicData() {
  String response = ticValuesAsJson();

  server.send(200, "text/json", response.c_str());
}

// Define routing
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, F("text/html"),
                F("Welcome to the REST Web Server"));
  });
  server.on(F("/settings"), HTTP_GET, getSettings);
  server.on(F("/ticdata"), HTTP_GET, getTicData);
}

void setup_serial() {
  //debug interface
  //there is only RX on Serial 0 interface
  //As the port is shared with usb debug, need to readress it
  //thanks to swap.
  //The speed is not appropriate for debug.
  //Debug serial port is on Serial 1, TX only
  DebugPort.begin(9600, SERIAL_7E1);

//data acquisition interface
#ifdef TIC
  TicPort.begin(9600, SERIAL_7E1);
  Serial.swap();
#else
  TicPort.begin(115200);
#endif
}

void setup() {

  setup_serial();

#ifdef WIFI_ENABLE
  setup_wifi();
  // Set server routing
  restServerRouting();

  // Start server
  DebugPort.println("Start HTTP server");
  server.begin();
  DebugPort.println("HTTP server started");
#endif
}

void loop() {

  server.handleClient();
  MDNS.update();
  readTicPort();

}
