#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "FRITZ!Box 7590 DH"
#define STAPSK  "79420784303209085505"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* mdnsName = "Ledmatrix";

char* argName = "text";
char* displayText = "Hello World";

ESP8266WebServer server(80);

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
  if(server.args() <= 0){
    server.send(400,"text/plain" ,"Missing Argument\n" + server.responseCodeToString(400));
    return;
  }
  
  server.send(200, "text/plain", ArgsToString());
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
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
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

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}