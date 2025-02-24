#pragma once

#include <ESPAsyncWebServer.h>
#include "telemetry.h"
#include "camera.h"
#include "state.h"

// WiFi network ssid and password
const char* ssid = "ECE362TeamBGP";
const char* password = nullptr;

// This class is responsible for providing the webserver and JSON-RPC based API.
// It also starts the WiFi network based on the ssid and password defined above.
class Api {
public:
  Api(VehicleState* state, fs::FS& fs, Telemetry* tl, Camera* camera)
    : state(state), fs(fs), tl(tl), camera(camera), server(80){};

  void init();  // Start the webserver

  void start();

  void stop();

  void set_speed(int);

  void set_lookahead_distance(double);

  void set_forward_offset(double);

  Data get_defaults();

  CameraView camera_view();  // Get line data from the camera

  TelemetryData telemetry();  // Get telemetry data from the vehicle
private:
  AsyncWebServer server;
  VehicleState* state;
  fs::FS& fs;
  Telemetry* tl;
  Camera* camera;
};
