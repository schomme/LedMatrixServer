
#ifndef EndPoint_h
#define EndPoint_h

    #include "Arduino.h"

    class EndPoint{
        public:
            EndPoint(String inPath, String inName, std::function<void(void)> inHandler, String inHelp);
            String path;
            String name;
            String help;
            std::function<void(void)> handler;
            String CreateLink();
    };

#endif