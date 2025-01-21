#pragma once

#include <ESPAsyncWebServer.h>
#include "telemetry.h"
#include "state.h"

const char* ssid = "ECE362CarTeam04";
const char* password = nullptr;

class Api {
public:
    Api(Telemetry* tl, VehicleState* st) : tl(tl), st(st) {};

    void init();
private:
    AsyncWebServer server(80);
    Telemetry* tl;
    VehicleState* st;

    void start();

    void stop();

    TelemetryData telemetry();
};
