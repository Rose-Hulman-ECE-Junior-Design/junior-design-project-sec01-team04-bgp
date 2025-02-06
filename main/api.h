#pragma once

#include <ESPAsyncWebServer.h>
#include "telemetry.h"
#include "state.h"

const char* ssid = "ECE362TeamBGP";
const char* password = nullptr;

class Api {
public:
    Api(Telemetry* tl, VehicleState* st, int* speed) : tl(tl), st(st), speed(speed), server(80) {};

    void init();
    
    void start();

    void stop();

    void set_speed(int speed);

    TelemetryData telemetry();
private:
    AsyncWebServer server;
    Telemetry* tl;
    VehicleState* st;
    int* speed;
};
