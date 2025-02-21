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
    Api(Telemetry* tl, Camera* camera) : tl(tl), camera(camera), server(80) {};

    void init(); // Start the webserver

    CameraView camera_view(); // Get line data from the camera

    TelemetryData telemetry(); // Get telemetry data from the vehicle
private:
    AsyncWebServer server;
    Telemetry* tl;
    Camera* camera;
};
