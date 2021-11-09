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
#include <WiFiManager.h>


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
WiFiManager wifiManager;

#define WIFISSID "LEDSETUP"
#define MDNSNAME "LEDMATRIX"

//HTTP ARGS
#define ARG_TEXT "text"
String TEXT_VALUE = "";
#define ARG_SCROLL "scroll"
bool SCROLL_VALUE = false;
#define ARG_COLOR_R "color_R"
uint8_t COLOR_R_VALUE = 255;
#define ARG_COLOR_G "color_G"
uint8_t COLOR_G_VALUE = 0;
#define ARG_COLOR_B "color_B"
uint8_t COLOR_B_VALUE = 0;
#define ARG_XPOS "xpos"
uint16_t XPOS_VALUE = 0;
#define ARG_YPOS "ypos"
uint16_t YPOS_VALUE = 0;
#define ARG_TEXT_SIZE "size"
uint8_t SIZE_VALUE = 1;
#define ARG_WRAP "wrap"
bool WRAP_VALUE = true;

String parameters[9] = {ARG_TEXT, ARG_SCROLL, ARG_COLOR_R, ARG_COLOR_G, ARG_COLOR_B, ARG_XPOS, ARG_YPOS, ARG_TEXT_SIZE, ARG_WRAP};

//LED Matrix
#include <Ticker.h>
Ticker ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32

PxMATRIX display(MATRIX_WIDTH,MATRIX_HEIGHT,P_LAT, P_OE,P_A,P_B,P_C,P_D);


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
  root += MDNSNAME;

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
void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t color)
{
    uint16_t text_length = text.length();
    uint8_t characterwidth = 5;
    for (int xpos=MATRIX_WIDTH; xpos>-(MATRIX_WIDTH+text_length*characterwidth); xpos--)
    {
      display.setTextColor(display.color565(COLOR_R_VALUE,COLOR_G_VALUE, COLOR_B_VALUE));
      display.clearDisplay();
      display.setCursor(xpos,ypos);
      display.println(text);
      delay(scroll_delay);
      yield();
      delay(scroll_delay/5);
      yield();

    }
}
void display_text(){

  display.clearDisplay();
  display.setTextWrap(WRAP_VALUE);
  display.setCursor(XPOS_VALUE,YPOS_VALUE);
  display.setTextColor(display.color565(COLOR_R_VALUE, COLOR_G_VALUE, COLOR_B_VALUE));
  display.setTextSize(SIZE_VALUE);

  if(SCROLL_VALUE){
    scroll_text(YPOS_VALUE, 50, TEXT_VALUE, 0);
  }else{
    display.print(TEXT_VALUE);
  }
}
void display_ticker()
{
  display.display(70);
}
bool isNumeric(String value){
  for (size_t i = 0; i < value.length(); i++)
  {
    if(!isDigit(value[i])) return false;
  }
  return true;
}
String CreateNotSetError(String key, String value){
      String errMsg = "Could net set parameter '";
      errMsg += key;
      errMsg += "' to '";
      errMsg += value;
      errMsg += "'.\n";
      return errMsg;
}
uint8_t MapColor(int c){
  return c >= 255 ? 255 : (c <= 0 ? 0 : c);
}
/*
================================
         ENDPOINTS
================================
*/

void handleRoot() {
  String message = "Current Endpoints";
  message += AddLineBreak();
  message += CreateLink("/", "This Page");
  message += AddLineBreak();
  message += CreateLink("/input", "Input Form");

  server.send(200, "text/html", message);
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
void handleInput(){
  server.send(200,"text/html","<form action=\"/display\"><label for=\"fname\">Text</label><br><input type=\"text\" id=\"text\" name=\"text\" value=\"Hello World\"><br><input type=\"submit\" value=\"Submit\"></form>");
}
void handleSet(){
    if(server.args() <= 0){
    server.send(400,"text/plain" ,"Missing Argument \n See /help for more." + server.responseCodeToString(400));
    return;
  }

  String errMsg = "";
  if(server.hasArg(ARG_TEXT)){
    TEXT_VALUE = GetArgValue(ARG_TEXT);
  }
  if(server.hasArg(ARG_SCROLL)){
    String value = GetArgValue(ARG_SCROLL); 
    if(isNumeric(value)){
      int number = value.toInt();
      SCROLL_VALUE = number >= 1 ? 1 : 0;
      
    }else{
      errMsg = CreateNotSetError(ARG_SCROLL, value);
    }
  }
  if(server.hasArg(ARG_TEXT_SIZE)){
    String value = GetArgValue(ARG_TEXT_SIZE); 
    if(isNumeric(value)){
      int number = value.toInt();
      SIZE_VALUE = number <= 1 ? 1 : number;
    }else{
      errMsg += CreateNotSetError(ARG_TEXT_SIZE, value);
    }
  }
  if(server.hasArg(ARG_COLOR_R)){
    String value = GetArgValue(ARG_COLOR_R); 
    if(isNumeric(value)){
      int number = value.toInt();
      COLOR_R_VALUE = MapColor(number);
    }else{
      errMsg += CreateNotSetError(ARG_COLOR_R, value);
    }
  }
  if(server.hasArg(ARG_COLOR_G)){
    String value = GetArgValue(ARG_COLOR_G); 
    if(isNumeric(value)){
      int number = value.toInt();
      COLOR_G_VALUE = MapColor(number);
    }else{
      errMsg += CreateNotSetError(ARG_COLOR_G, value);
    }
  }
  if(server.hasArg(ARG_COLOR_B)){
    String value = GetArgValue(ARG_COLOR_B); 
    if(isNumeric(value)){
      int number = value.toInt();
      COLOR_B_VALUE = MapColor(number);
    }else{
      errMsg += CreateNotSetError(ARG_COLOR_B, value);
    }
  }
  if(server.hasArg(ARG_XPOS)){
    String value = GetArgValue(ARG_XPOS); 
    if(isNumeric(value)){
      int number = value.toInt();
      XPOS_VALUE = number;
    }else{
      errMsg += CreateNotSetError(ARG_XPOS, value);
    }
  }
  if(server.hasArg(ARG_YPOS)){
    String value = GetArgValue(ARG_YPOS); 
    if(isNumeric(value)){
      int number = value.toInt();
      YPOS_VALUE = number;
    }else{
      errMsg += CreateNotSetError(ARG_YPOS, value);
    }
  }
  if(server.hasArg(ARG_WRAP)){
    String value = GetArgValue(ARG_WRAP); 
    if(isNumeric(value)){
      int number = value.toInt();
      WRAP_VALUE = number >= 1;
    }else{
      errMsg += CreateNotSetError(ARG_WRAP, value);
    }
  }

  if(errMsg.length() > 0){
    server.send(400, "text/plain", errMsg + "\n" + server.responseCodeToString(400));
    return;
  }
  
  server.send(200, "text/plain", "All values has successfully been set!");
}
void handleHelp(){
  server.send(200, "text/plain", "this is the help page.\n comming soon...");
}
void handleResetWifi(){
  server.send(200, "text/plain", "Restarting...");
  wifiManager.resetSettings();
  delay(3000);
  ESP.restart();
}

/*
================================
         SETUP METHODS
================================
*/


void setupWifi(){
  wifiManager.setClass("invert"); // dark theme
  wifiManager.autoConnect(WIFISSID);
}

void setupMDNS(){
  if (MDNS.begin(MDNSNAME)) {
    Serial.println("MDNS responder started");
    Serial.print("Go to http://");
    Serial.print(MDNSNAME);
    Serial.println("/");

    String msg = "Go to http://";
    msg += MDNSNAME;
    msg += "/";
    TEXT_VALUE = msg;
    display_text();
  }
}

void setupServer(){
  server.on("/", handleRoot);
  server.on("/input", handleInput);
  server.on("/set", handleSet);
  server.on("/help", handleHelp);
  server.on("/resetwifi", handleResetWifi);
  server.onNotFound(handleNotFound);

  Serial.println("HTTP server started");
  server.begin();
}

void setupDisplay(){

  display.begin(16);
  display.clearDisplay();
  display.setTextColor(display.color565(COLOR_R_VALUE,COLOR_G_VALUE, COLOR_B_VALUE));
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Setup...");
  display.print("Connect to Wifi: ");
  display.println(WIFISSID);

  ticker.attach(0.004, display_ticker);
  delay(1000);
}
/*
================================
         MAIN CODE
================================
*/

void setup(void) {
  Serial.begin(115200); 
  setupDisplay();
  setupWifi();
  setupMDNS();
  setupServer();
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  display_text();

}