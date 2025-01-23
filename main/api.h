#pragma once

#include <ESPAsyncWebServer.h>
#include "telemetry.h"
#include "state.h"

const char* ssid = "ECE362TeamBGP";
const char* password = nullptr;

class Api {
public:
    Api(Telemetry* tl, VehicleState* st) : tl(tl), st(st), server(80) {};

    void init();
    
    void start();

    void stop();

    TelemetryData telemetry();
private:
    AsyncWebServer server;
    Telemetry* tl;
    VehicleState* st;
};
