#include "Arduino.h"
#include "EndPoint.h"

EndPoint::EndPoint(String inPath, String inName, std::function<void(void)> inHandler, String inHelp){
    path = inPath;
    name = inName;
    handler = inHandler;
    help = inHelp;  
}
String EndPoint::CreateLink(){
  String form1 = "<form action=\"";
  String form2 = "\" method=\"get\"><button>";
  String form3 = "</button></form>";
  return form1 + path + form2 + name + form3;
}