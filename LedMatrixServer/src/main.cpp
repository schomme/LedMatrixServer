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

#include <../lib/EndPoint/src/EndPoint.h>

/*
======================================
         Constants and Variables
======================================
*/

//Wifi
ESP8266WebServer server(80);
WiFiManager wifiManager;

#define HTML_HEAD "<!DOCTYPE html><html lang=\"en\"><head><title>LEDMATRIX</title><style> .c{text-align:center;}div,input{padding:5px;font-size:1em;}input{width:95%;}body{text-align:center;font-family:verdana;}button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float:right;width:64px;text-align:right;} .l{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\")no-repeat left center;background-size:1em;}</style></head><body><div style=\"text-align:left;display:inline-block;min-width:260px;\">"
#define HTML_BOTTOM "</div></body></html>"
#define HTML_INPUT "<input id=\"{id}\" name=\"{id}\" type=\"{type}\" min=\"{min}\" max=\"{max}\" value=\"{value}\" />"
#define HTML_LABEL "<label for=\"{id}\">{name}</label>"
#define HTML_BR "<br>"
#define WIFISSID "LEDSETUP"
#define MDNSNAME "LEDMATRIX"

//HTTP ARGS
#define ARG_TEXT "text"
String TEXT_VALUE = "";
#define ARG_SCROLL "scroll"
bool SCROLL_VALUE = false;
#define ARG_COLOR_R "color_R"
uint8_t  COLOR_R_VALUE = 255;
#define ARG_COLOR_G "color_G"
uint8_t COLOR_G_VALUE = 0;
#define ARG_COLOR_B "color_B"
uint8_t COLOR_B_VALUE = 0;
#define ARG_XPOS "xpos"
int XPOS_VALUE = 0;
#define ARG_YPOS "ypos"
uint8_t YPOS_VALUE = 0;
#define ARG_TEXT_SIZE "size"
uint8_t SIZE_VALUE = 1;
#define ARG_WRAP "wrap"
bool WRAP_VALUE = true;
#define ARG_BRIGHTNESS "brightness"
uint8_t BRIGHTNESS_VALUE = 255;

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
const int16_t ButtonPressedResetTime = 10000; //10sec

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
String GetArgValue(String key){
  for (uint8_t i = 0; i < server.args(); i++) {
    if(server.argName(i) == key){
      return server.arg(i);
    }
  }
  return "";
}

void display_text(){

  display.clearDisplay();
  display.setTextWrap(WRAP_VALUE);
  display.setCursor(XPOS_VALUE,YPOS_VALUE);
  display.setTextColor(display.color565(COLOR_R_VALUE, COLOR_G_VALUE, COLOR_B_VALUE));
  display.setTextSize(SIZE_VALUE);
  display.setBrightness(BRIGHTNESS_VALUE);

  if(SCROLL_VALUE){
    int characterwidth = 6*SIZE_VALUE;
    int text_length = TEXT_VALUE.length();
    int step_width = 1;

    String DEBUGMSG = "TextLength: ";
    DEBUGMSG += String(text_length);
    DEBUGMSG += "   XPOS: ";
    DEBUGMSG += String(XPOS_VALUE);
    DEBUGMSG += "   Max Pixel width: ";
    DEBUGMSG += String(text_length*characterwidth);
    DEBUGMSG += "   MatrixWidth: ";
    DEBUGMSG += String(MATRIX_WIDTH);
    Serial.println(DEBUGMSG);





    if(XPOS_VALUE + text_length*characterwidth <= 0){
      XPOS_VALUE = MATRIX_WIDTH;
      Serial.println("Resetting XPOS to " + String(XPOS_VALUE) );

    }

    XPOS_VALUE -= step_width;
  }
  display.print(TEXT_VALUE);

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
String CreateSetMsg(String key, String value){
  String msg = "Parameter '";
  msg += key;
  msg += "' is set to '";
  msg += value;
  msg += "'.\n";
  return msg;
}
uint8_t MapColor(int c){
  return c >= 255 ? 255 : (c <= 0 ? 0 : c);
}
String CreateFormElement(String name,String id, String type, String value, String min, String max){
  String result = HTML_LABEL;
  result.replace("{name}",name);
  result += HTML_INPUT;
  result.replace("{id}",id);
  result.replace("{type}",type);
  result.replace("{value}", value);
  result.replace("{min}",min);
  result.replace("{max}",max);
  result += HTML_BR;
  return result;
}
void ResetWifi(){
  wifiManager.resetSettings();
  wifiManager.erase();
  delay(3000);
  ESP.restart();
}
void CheckResetButton(){
  long start = millis();
  long end = start;
  while(analogRead(A0) > 1000){
    Serial.println("button is down");
    end = millis();
    if(end - start > ButtonPressedResetTime){
      ResetWifi();
    }  
  }
}

/*
================================
         ENDPOINTS
================================
*/
void handleRoot(); //declared to use in EndPoints. Definition below
void handleSet();
void handleInput();
void handleResetWifi();

EndPoint ep_root("/", "Home", handleRoot);
EndPoint ep_set("/set", "Setter", handleSet);
EndPoint ep_input("/input", "Input", handleInput);
EndPoint ep_help("https://github.com/schomme/LedMatrixServer", "Help", NULL);
EndPoint ep_wifi("/resetwifi", "ResetWifi", handleResetWifi);

/*
================================
         EndPoints
================================
*/

void handleRoot() {
  String m = "";
  m += HTML_HEAD;
  m += ep_root.CreateLink() + HTML_BR;
  m += ep_input.CreateLink() + HTML_BR;
  m += ep_help.CreateLink() + HTML_BR;
  m += HTML_BR + ep_wifi.CreateLink();

  m += HTML_BOTTOM;
  server.send(200, "text/html", m);
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
  String msg = HTML_HEAD;
  msg += "<form action=\"/set\" method=\"get\">";
  msg += CreateFormElement("Text", ARG_TEXT, "text", TEXT_VALUE,"","");
  msg += CreateFormElement("Size", ARG_TEXT_SIZE, "number", String(SIZE_VALUE), "1", "");
  msg += CreateFormElement("Wrap", ARG_WRAP, "checkbox","1","","");
  msg += CreateFormElement("Scroll", ARG_SCROLL, "checkbox","1","","");
  msg += CreateFormElement("Red", ARG_COLOR_R, "number",String(COLOR_R_VALUE),"0","255");
  msg += CreateFormElement("Green", ARG_COLOR_G, "number",String(COLOR_G_VALUE),"0","255");
  msg += CreateFormElement("Blue", ARG_COLOR_B, "number",String(COLOR_B_VALUE),"0","255");
  msg += CreateFormElement("X Position", ARG_XPOS, "number",String(XPOS_VALUE),"","");
  msg += CreateFormElement("Y Position", ARG_YPOS, "number",String(YPOS_VALUE),"","");
  msg += CreateFormElement("Brightness", ARG_BRIGHTNESS, "number", String(BRIGHTNESS_VALUE),"0","255");

  msg += HTML_BR;
  msg += CreateFormElement("Submit", "submit", "submit" , "submit", "", "");
  msg += "</form>";
  msg += HTML_BOTTOM;
  server.send(200,"text/html", msg);
}
void handleSet(){
    if(server.args() <= 0){
    server.send(400,"text/plain" ,"Missing Argument \n See /help for more." + server.responseCodeToString(400));
    return;
  }

  String result = "Result:\n===========================================================\n";
  
  if(server.hasArg(ARG_TEXT)){
    TEXT_VALUE = GetArgValue(ARG_TEXT);
    result += CreateSetMsg(ARG_TEXT, TEXT_VALUE); 
  }
  if(server.hasArg(ARG_TEXT_SIZE)){
    String value = GetArgValue(ARG_TEXT_SIZE); 
    if(isNumeric(value)){
      int number = value.toInt();
      SIZE_VALUE = number <= 1 ? 1 : number;
      result += CreateSetMsg(ARG_TEXT_SIZE, String(SIZE_VALUE)); 

    }else{
      result += CreateNotSetError(ARG_TEXT_SIZE, value);
    }
  }
  if(server.hasArg(ARG_COLOR_R)){
    String value = GetArgValue(ARG_COLOR_R); 
    if(isNumeric(value)){
      int number = value.toInt();
      COLOR_R_VALUE = MapColor(number);
      result += CreateSetMsg(ARG_COLOR_R, String(COLOR_R_VALUE)); 

    }else{
      result += CreateNotSetError(ARG_COLOR_R, value);
    }
  }
  if(server.hasArg(ARG_COLOR_G)){
    String value = GetArgValue(ARG_COLOR_G); 
    if(isNumeric(value)){
      int number = value.toInt();
      COLOR_G_VALUE = MapColor(number);
      result += CreateSetMsg(ARG_COLOR_G, String(COLOR_G_VALUE)); 
    }else{
      result += CreateNotSetError(ARG_COLOR_G, value);
    }
  }
  if(server.hasArg(ARG_COLOR_B)){
    String value = GetArgValue(ARG_COLOR_B); 
    if(isNumeric(value)){
      int number = value.toInt();
      COLOR_B_VALUE = MapColor(number);
      result += CreateSetMsg(ARG_COLOR_B, String(COLOR_B_VALUE)); 

    }else{
      result += CreateNotSetError(ARG_COLOR_B, value);
    }
  }
  if(server.hasArg(ARG_XPOS)){
    String value = GetArgValue(ARG_XPOS); 
    if(isNumeric(value)){
      int number = value.toInt();
      XPOS_VALUE = number;
      result += CreateSetMsg(ARG_XPOS, String(XPOS_VALUE)); 
    }else{
      result += CreateNotSetError(ARG_XPOS, value);
    }
  }
  if(server.hasArg(ARG_YPOS)){
    String value = GetArgValue(ARG_YPOS); 
    if(isNumeric(value)){
      int number = value.toInt();
      YPOS_VALUE = number;
      result += CreateSetMsg(ARG_YPOS, String(YPOS_VALUE)); 

    }else{
      result += CreateNotSetError(ARG_YPOS, value);
    }
  }

  if(server.hasArg(ARG_BRIGHTNESS)){
    String value = GetArgValue(ARG_BRIGHTNESS); 
    if(isNumeric(value)){
      int number = value.toInt();
      BRIGHTNESS_VALUE = MapColor(number);
      result += CreateSetMsg(ARG_BRIGHTNESS, String(BRIGHTNESS_VALUE)); 
    }else{
      result += CreateNotSetError(ARG_BRIGHTNESS, value);
    }
  }

  WRAP_VALUE = server.hasArg(ARG_WRAP);
  result += CreateSetMsg(ARG_WRAP, String(WRAP_VALUE)); 
  SCROLL_VALUE = server.hasArg(ARG_SCROLL);
  result += CreateSetMsg(ARG_SCROLL, String(SCROLL_VALUE)); 
  server.send(200, "text/plain", result);
}

void handleResetWifi(){
  server.send(200, "text/plain", "Restarting...");
  ResetWifi();
}

/*
================================
         SETUP METHODS
================================
*/

void setupWifi(){
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
  
  server.onNotFound(handleNotFound);
  server.on(ep_root.path, ep_root.handler);
  server.on(ep_set.path, ep_set.handler);
  server.on(ep_input.path, ep_input.handler);
  server.on(ep_wifi.path, ep_wifi.handler);

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
  CheckResetButton();
  delay(100);
}