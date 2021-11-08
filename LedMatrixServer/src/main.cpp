/*
================================
         Libraries
================================
*/

//  Wifi Libraries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WifiManager.h>

//  LED Matrix Libraries
#include <PxMatrix.h>
#include <Wire.h>

/*
======================================
         Constants and Variables
======================================
*/

//Wifi
ESP8266WebServer server(80);
const char* mdnsName = "Ledmatrix";
String argName = "text";
String displayText = "Hello World";

//LED Matrix
#ifdef ESP8266
  #include <Ticker.h>
  Ticker display_ticker;
  #define P_LAT 16
  #define P_A 5
  #define P_B 4
  #define P_C 15
  #define P_D 12
  #define P_E 0
  #define P_OE 2
#endif
PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);



/*
================================
         Helper Methods
================================
*/

String ArgsToString(bool linebreak = true){
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i);
    message += linebreak ? "\n" : " ";
  }
  return message;
}
String CreateLink(String endPoint, String linkText){

  String root = "http://";
  root += mdnsName;

  String s = "<a href=\"";
  s += root;
  s += endPoint;
  s += "\">";
  s += linkText;
  s += "</a>";
  return s;
}
String AddLineBreak(){
  return "<br>";
}
String GetArgValue(String key){
  for (uint8_t i = 0; i < server.args(); i++) {
    if(server.argName(i) == key){
      return server.arg(i);
    }
  }
  return "";
}

#ifdef ESP8266
  void display_updater()
  {
    display.display(70);
  }
#endif
/*
================================
         ENDPOINTS
================================
*/


void handleRoot() {
  digitalWrite(LED_BUILTIN, 1);

  String message = "Current Endpoints";
  message += AddLineBreak();
  message += CreateLink("/", "This Page");
  message += AddLineBreak();
  message += CreateLink("/display", "Sets the display variable");
  message += AddLineBreak();
  message += CreateLink("/input", "Input Form");

  server.send(200, "text/html", message);
  digitalWrite(LED_BUILTIN, 0);
}
void handleNotFound() {
  digitalWrite(LED_BUILTIN, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  message += ArgsToString();

  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, 0);
}
void handleDisplay(){
  if(server.args() <= 0 || !server.hasArg(argName)){
    server.send(400,"text/plain" ,"Missing Argument \"text\"\n" + server.responseCodeToString(400));
    return;
  }
  displayText = GetArgValue(argName);

  server.send(200, "text/plain", "Value is set to \""+displayText+"\"\n");
}
void handleInput(){
  server.send(200,"text/html","<form action=\"/display\"><label for=\"fname\">Text</label><br><input type=\"text\" id=\"text\" name=\"text\" value=\"Hello World\"><br><input type=\"submit\" value=\"Submit\"></form>");
}



/*
================================
         SETUP METHODS
================================
*/


void setupWifi(){

  WiFiManager wifiManager;
  wifiManager.autoConnect("LedMatrixSetup");

  return;
}

void setupMDNS(){
  if (MDNS.begin(mdnsName)) {
    Serial.println("MDNS responder started");
    Serial.print("Go to http://");
    Serial.print(mdnsName);
    Serial.println("/");
  }
}

void setupServer(){
  server.on("/", handleRoot);
  server.on("/display", handleDisplay);
  server.on("/input", handleInput);
  server.onNotFound(handleNotFound);

  Serial.println("HTTP server started");
  server.begin();
}


void setupDisplay(){

  uint16_t RED = display.color565(255,0,0);

  //Serial.print("Setting up display...");
  display.begin(16);
  display.flushDisplay();
  display.setTextColor(RED);
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("###hello world####");
  display_ticker.attach(0.004, display_updater);
  //Serial.print("[OK]");
  delay(1000);
}
/*
================================
         MAIN CODE
================================
*/

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  Serial.begin(115200);
  setupWifi();
  setupMDNS();
  setupServer();
  setupDisplay();
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  delay(100);
}