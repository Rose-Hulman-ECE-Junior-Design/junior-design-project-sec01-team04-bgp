#pragma once

#include <ESPAsyncWebServer.h>
#include "telemetry.h"
#include "state.h"
#include "camera.h"

// WiFi network ssid and password
const char* ssid = "ECE362TeamBGP";
const char* password = nullptr;

// This class is responsible for providing the webserver and JSON-RPC based API.
// It also starts the WiFi network based on the ssid and password defined above.
class Api {
public:
    Api(Telemetry* tl, VehicleState* st, int* speed, Camera* camera) : tl(tl), st(st), speed(speed), camera(camera), server(80) {};

    void init(); // Start the webserver
    
    void start(); // Start the vehicle

    void stop(); // Stop the vehicle

    void set_speed(int speed); // Set the vehicle's speed

    CameraView camera_view();

    TelemetryData telemetry(); // Get telemetry data from the vehicle
private:
    AsyncWebServer server;
    Telemetry* tl;
    VehicleState* st;
    Camera* camera;
    int* speed;
};
