
#ifndef EndPoint_h
#define EndPoint_h

    #include "Arduino.h"

    class EndPoint{
        public:
            EndPoint(String inPath, String inName, std::function<void(void)> inHandler);
            String path;
            String name;
            std::function<void(void)> handler;
            String CreateLink();
    };

#endif