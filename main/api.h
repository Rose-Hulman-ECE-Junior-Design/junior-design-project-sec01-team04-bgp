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

  CurveData* get_curve(int curve);

  void set_curve(CurveData* curve_data, double x_start, double x_end, double y_start, double y_end);

  CameraView camera_view();  // Get line data from the camera

  TelemetryData telemetry();  // Get telemetry data from the vehicle
private:
  AsyncWebServer server;
  VehicleState* state;
  fs::FS& fs;
  Telemetry* tl;
  Camera* camera;
};
