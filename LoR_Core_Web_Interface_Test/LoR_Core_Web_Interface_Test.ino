#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

/* Put your SSID & Password */
const char* ssid = "MiniBot";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

#define LED_DataPin 12
#define LED_COUNT 36
bool LEDstatus = LOW;

// Set a specific color for the entire NeoPixel strip
// NeoPixel Configurations
Adafruit_NeoPixel strip(LED_COUNT, LED_DataPin, NEO_GRB + NEO_KHZ800);
const uint32_t RED = strip.Color(255, 0, 0, 0);
const uint32_t GREEN = strip.Color(0, 255, 0, 0);
const uint32_t BLUE = strip.Color(0, 0, 255, 0);
const uint32_t WHITE = strip.Color(0, 0, 0, 255);
const uint32_t PURPLE = strip.Color(255, 0, 255, 0);
const uint32_t CYAN = strip.Color(0, 255, 255, 0);
const uint32_t YELLOW = strip.Color(255, 255, 0, 0);
void NeoPixel_SetColour(uint32_t color) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                // Update strip with new contents
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_DataPin, OUTPUT);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/LEDon", handle_LEDon);
  server.on("/LEDoff", handle_LEDoff);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  if(LEDstatus)
  {NeoPixel_SetColour(GREEN);}
  else
  {NeoPixel_SetColour(RED);}

}

void handle_OnConnect() {
  LEDstatus = LOW;
  Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LEDstatus)); 
}

void handle_LEDon() {
  LEDstatus = HIGH;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true)); 
}

void handle_LEDoff() {
  LEDstatus = LOW;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t LEDstat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
   if(LEDstat)
  {ptr +="<p>LED Status: ON</p><a class=\"button button-off\" href=\"/LEDoff\">OFF</a>\n";}
  else
  {ptr +="<p>LED Status: OFF</p><a class=\"button button-on\" href=\"/LEDon\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}